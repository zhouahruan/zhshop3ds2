/**
 * App list scene — paginated grid of apps on the top screen driven by
 * the active category/keyword/sort in the store; the bottom screen hosts
 * a search bar, sort tabs and the nav bar. Selecting an app pushes the
 * detail scene. Selected via scene_applist_get().
 */
#ifndef SCENES_SCENE_APPLIST_H
#define SCENES_SCENE_APPLIST_H

#include "../core/scene.h"

DECLARE_SCENE(applist);

#endif /* SCENES_SCENE_APPLIST_H */
