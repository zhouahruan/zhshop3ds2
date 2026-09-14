/**
 * Image — PNG sprite display widget (citro2d).
 *
 * Loads a PNG via C2D_ImageLoadPNG and draws it scaled inside its bounds.
 * The optional owns flag lets the widget free the image on destroy.
 */
#ifndef UI_IMAGE_H
#define UI_IMAGE_H

#include "widget.h"
#include <citro2d.h>

typedef struct {
    Widget base;
    C2D_Image img;         /* citro2d image handle (zeroed until set/loaded) */
    uint8_t  owns_image;   /* 1 = call C2D_ImageDelete on destroy */
    float scale_x, scale_y;
} Image;

void image_init(Image* img, float x, float y, float w, float h);
void image_set(Image* img, C2D_Image src, uint8_t owns);
void image_load_png(Image* img, const char* romfs_path);
void image_destroy(Image* img);

#endif /* UI_IMAGE_H */
