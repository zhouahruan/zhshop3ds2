/**
 * Chat scene — message stream for the active session on the top screen,
 * session list and a SWKBD-backed input bar on the bottom screen. Sending
 * a message re-fetches the thread. Selected via scene_chat_get().
 */
#ifndef SCENES_SCENE_CHAT_H
#define SCENES_SCENE_CHAT_H

#include "../core/scene.h"

DECLARE_SCENE(chat);

#endif /* SCENES_SCENE_CHAT_H */
