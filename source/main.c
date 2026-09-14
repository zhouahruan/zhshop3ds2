/**
 * 3DS App Store - main entry point
 *
 * Initializes graphics, audio, network, scene manager; runs the main loop;
 * cleans up on exit. Real product runs on Nintendo 3DS hardware via CIA
 * install. Source builds under devkitPro/devkitARM toolchain.
 */

#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "core/app.h"
#include "core/render.h"
#include "core/input.h"
#include "core/scene_manager.h"
#include "core/transition.h"
#include "core/utils.h"
#include "data/store.h"
#include "scenes/scene_splash.h"

/* Frame timing: 3DS runs the main loop targeting 60 fps. */
#define TARGET_FPS 60
#define FRAME_NS   (1000000000ULL / TARGET_FPS)

/* SOC (network) buffer. 1 MiB aligned to a page boundary. httpc needs a
 * reasonably large SOC heap; 256 KiB is too small and causes socInit to
 * silently fail on some firmware versions. */
#define SOC_BUFFER_SIZE  0x100000
#define SOC_BUFFER_ALIGN 0x1000
static u32 s_soc_buffer[SOC_BUFFER_SIZE / sizeof(u32)] __attribute__((aligned(SOC_BUFFER_ALIGN)));
static int s_romfs_ok = 0;
static int s_cfgu_ok = 0;
static int s_ac_ok = 0;

/* Simple SD-card log so we can locate crash points on hardware. The log
 * file is written to "sdmc:/3ds/appstore/boot.log" (truncated each boot). */
static FILE* s_log = NULL;

static void log_open(void) {
    /* Best-effort: ensure parent dirs exist before opening the log. */
    mkdir("sdmc:/3ds",            0777);
    mkdir("sdmc:/3ds/appstore",   0777);
    s_log = fopen("sdmc:/3ds/appstore/boot.log", "wb");
}

static void log_step(const char* msg) {
    if (!s_log) return;
    fputs(msg, s_log);
    fputc('\n', s_log);
    fflush(s_log);
}

static void log_close(void) {
    if (s_log) { fclose(s_log); s_log = NULL; }
}

static void init_subsystems(void) {
    log_open();
    log_step("boot: start");

    /* Initialize service manager. */
    log_step("boot: gfxInitDefault");
    gfxInitDefault();

    /* cfgu is required by citro2d's system-font loader
     * (C2D_FontLoadSystem -> CFGU_SecureInfoGetRegion) and by region
     * queries. Without it the first C2D_TextParse can crash on hardware. */
    log_step("boot: cfguInit");
    s_cfgu_ok = R_SUCCEEDED(cfguInit());
    if (!s_cfgu_ok) log_step("boot: cfguInit FAILED");

    /* citro3d + citro2d for rendering. */
    log_step("boot: C3D_Init");
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    log_step("boot: C2D_Init");
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    log_step("boot: C2D_Prepare");
    C2D_Prepare();

    /* Touch + hid. */
    log_step("boot: input_init");
    input_init();

    /* romfs for assets bundled in the cia (optional). */
    log_step("boot: romfsInit");
    s_romfs_ok = R_SUCCEEDED(romfsInit());
    if (!s_romfs_ok) log_step("boot: romfsInit failed (non-fatal)");

    /* Network: ac:u provides the active connection slot that httpc relies
     * on internally. socInit/httpcInit alone are not enough on hardware. */
    log_step("boot: acInit");
    s_ac_ok = R_SUCCEEDED(acInit());
    if (!s_ac_ok) log_step("boot: acInit FAILED");
    log_step("boot: socInit");
    socInit(s_soc_buffer, SOC_BUFFER_SIZE);
    log_step("boot: httpcInit");
    httpcInit(0);

    /* Application-level state. */
    log_step("boot: app_init");
    app_init();
    log_step("boot: store_init");
    store_init();
    log_step("boot: render_init");
    render_init();
    log_step("boot: transition_init");
    transition_init();
    log_step("boot: scene_manager_init");
    scene_manager_init();
    log_step("boot: init done");
}

static void shutdown_subsystems(void) {
    scene_manager_shutdown();
    render_exit();

    store_free_lists();
    app_exit();

    httpcExit();
    socExit();
    if (s_ac_ok) acExit();

    if (s_romfs_ok) romfsExit();
    if (s_cfgu_ok) cfguExit();

    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

int main(int argc, char* argv[]) {
    init_subsystems();

    /* Boot straight into the splash scene. Splash transitions into home. */
    log_step("boot: push splash");
    scene_manager_push(scene_splash_get(), NULL);
    log_step("boot: enter main loop");

    u64 last_ns = osGetTime() * 1000000ULL;
    u64 accum_ns = 0;
    int frame_count = 0;

    while (aptMainLoop()) {
        u64 now_ns = osGetTime() * 1000000ULL;
        u64 frame_delta = now_ns - last_ns;
        last_ns = now_ns;
        accum_ns += frame_delta;

        /* Cap to one frame max delta to avoid spiral-of-death after pauses. */
        if (accum_ns > FRAME_NS * 4) {
            accum_ns = FRAME_NS * 4;
        }

        /* Pump input first so scenes can react. */
        input_update();
        if (input_pressed(KEY_START) && (input_held(KEY_SELECT))) {
            /* Secret force-quit (Select+Start). */
            break;
        }

        /* Step scene update logic. */
        scene_manager_update();

        /* Render. */
        render_begin_frame();

        render_target_top(EYE_2D);
        scene_manager_draw(SCREEN_TOP);

        render_target_bottom();
        scene_manager_draw(SCREEN_BOTTOM);

        render_end_frame();

        /* Log the first few frames so we know rendering works. */
        if (frame_count < 3) {
            char buf[64];
            snprintf(buf, sizeof(buf), "boot: frame %d ok", frame_count);
            log_step(buf);
            frame_count++;
        }

        /* Wait for next frame. */
        gspWaitForVBlank();
    }

    log_step("boot: exiting main loop");
    shutdown_subsystems();
    log_step("boot: shutdown done");
    log_close();
    return 0;
}
