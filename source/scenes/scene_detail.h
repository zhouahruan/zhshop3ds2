/**
 * Detail scene — full app information on the top screen (name, developer,
 * description, rating, download count) with Back / Download buttons on
 * the bottom screen. Pushed by the home and applist scenes; popping
 * returns to the previous scene. Selected via scene_detail_get().
 */
#ifndef SCENES_SCENE_DETAIL_H
#define SCENES_SCENE_DETAIL_H

#include "../core/scene.h"

DECLARE_SCENE(detail);

#endif /* SCENES_SCENE_DETAIL_H */
