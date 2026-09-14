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

static void init_subsystems(void) {
    /* Initialize service manager. */
    gfxInitDefault();

    /* cfgu is required by citro2d's system-font loader
     * (C2D_FontLoadSystem -> CFGU_SecureInfoGetRegion) and by region
     * queries. Without it the first C2D_TextParse can crash on hardware. */
    s_cfgu_ok = R_SUCCEEDED(cfguInit());

    /* citro3d + citro2d for rendering. */
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

    /* Touch + hid. */
    input_init();

    /* romfs for assets bundled in the cia (optional). */
    s_romfs_ok = R_SUCCEEDED(romfsInit());

    /* Network: ac:u provides the active connection slot that httpc relies
     * on internally. socInit/httpcInit alone are not enough on hardware. */
    s_ac_ok = R_SUCCEEDED(acInit());
    socInit(s_soc_buffer, SOC_BUFFER_SIZE);
    httpcInit(0);

    /* Application-level state. */
    app_init();
    store_init();
    render_init();
    transition_init();
    scene_manager_init();
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
    scene_manager_push(scene_splash_get(), NULL);

    u64 last_ns = osGetTime() * 1000000ULL;
    u64 accum_ns = 0;

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

        /* Wait for next frame. */
        gspWaitForVBlank();
    }

    shutdown_subsystems();
    return 0;
}
