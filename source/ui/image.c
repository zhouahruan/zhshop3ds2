/* Image — PNG sprite display widget implementation. */
#include "image.h"
#include "../core/utils.h"
#include <citro2d.h>
#include <string.h>

static void image_on_draw(Widget* self) {
    Image* im = (Image*)self;
    if (!im->img.tex) return;
    C2D_DrawParams params = {
        .pos    = { im->base.x, im->base.y, im->base.w, im->base.h },
        .depth  = 0.5f,
        .angle  = 0.0f,
        .center = { 0.0f, 0.0f },
    };
    C2D_DrawImage(im->img, &params, NULL);
}

static const WidgetVTable image_vtable = {
    .on_touch = NULL,
    .on_draw  = image_on_draw,
};

void image_init(Image* img, float x, float y, float w, float h) {
    widget_init(&img->base, x, y, w, h);
    img->base.vtable = &image_vtable;
    memset(&img->img, 0, sizeof(img->img));
    img->owns_image = 0;
    img->scale_x = 1.0f;
    img->scale_y = 1.0f;
}

void image_set(Image* img, C2D_Image src, uint8_t owns) {
    if (img->owns_image) image_destroy(img);
    img->img = src;
    img->owns_image = owns ? 1 : 0;
}

void image_load_png(Image* img, const char* romfs_path) {
    (void)romfs_path;
    if (img->owns_image) image_destroy(img);
    memset(&img->img, 0, sizeof(img->img));
    /* citro2d has no built-in PNG loader; loading PNG textures requires
     * a sprite sheet (.t3x) produced by tex3ds. For now leave the image
     * empty so the widget draws nothing until a real texture is supplied
     * via image_set(). */
    img->owns_image = 0;
}

void image_destroy(Image* img) {
    if (img->owns_image && img->img.tex) {
        C3D_TexDelete(img->img.tex);
    }
    memset(&img->img, 0, sizeof(img->img));
    img->owns_image = 0;
}
