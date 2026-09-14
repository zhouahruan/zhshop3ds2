#include "scene_manager.h"
#include "transition.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#define MAX_SCENES 8

static Scene* s_stack[MAX_SCENES];
static int    s_top = -1;   /* -1 = empty */
static Scene* s_pending_push = NULL;
static void*  s_pending_push_args = NULL;
static int    s_pending_pop = 0;
static Scene* s_pending_switch = NULL;
static void*  s_pending_switch_args = NULL;
static int    s_pending_reset = 0;

static void apply_pending(void);

void scene_manager_init(void) {
    s_top = -1;
    s_pending_push = NULL;
    s_pending_pop  = 0;
    s_pending_switch = NULL;
    s_pending_reset  = 0;
}

void scene_manager_shutdown(void) {
    while (s_top >= 0) {
        Scene* s = s_stack[s_top];
        if (s && s->vtable && s->vtable->on_exit) s->vtable->on_exit(s);
        s_stack[s_top] = NULL;
        s_top--;
    }
}

void scene_manager_push(Scene* s, void* args) {
    if (!s) return;
    s_pending_push = s;
    s_pending_push_args = args;
    if (s_top < 0) {
        /* Initial scene on empty stack: apply immediately without transition delay */
        apply_pending();
    } else {
        transition_start_in();
    }
}

void scene_manager_pop(void) {
    if (s_top < 0) return;
    s_pending_pop = 1;
    transition_start_out();
}

void scene_manager_switch(Scene* s, void* args) {
    if (!s) return;
    s_pending_switch = s;
    s_pending_switch_args = args;
    transition_start_in();
}

void scene_manager_reset(Scene* s, void* args) {
    if (!s) return;
    s_pending_switch = s;
    s_pending_switch_args = args;
    s_pending_reset = 1;
    transition_start_in();
}

Scene* scene_manager_current(void) {
    if (s_top < 0) return NULL;
    return s_stack[s_top];
}

/* Apply pending navigation transitions after a transition completes. */
static void apply_pending(void) {
    if (s_pending_pop) {
        s_pending_pop = 0;
        if (s_top >= 0) {
            Scene* s = s_stack[s_top];
            if (s && s->vtable && s->vtable->on_exit) s->vtable->on_exit(s);
            s_stack[s_top] = NULL;
            s_top--;
            if (s_top >= 0) {
                Scene* t = s_stack[s_top];
                if (t && t->vtable && t->vtable->on_resume) t->vtable->on_resume(t);
            }
        }
    }
    if (s_pending_reset) {
        s_pending_reset = 0;
        while (s_top >= 0) {
            Scene* s = s_stack[s_top];
            if (s && s->vtable && s->vtable->on_exit) s->vtable->on_exit(s);
            s_stack[s_top] = NULL;
            s_top--;
        }
    }
    if (s_pending_push) {
        Scene* s = s_pending_push;
        void* args = s_pending_push_args;
        s_pending_push = NULL;
        s_pending_push_args = NULL;
        if (s_top >= 0) {
            Scene* prev = s_stack[s_top];
            if (prev && prev->vtable && prev->vtable->on_pause) prev->vtable->on_pause(prev);
        }
        if (s_top + 1 < MAX_SCENES) {
            s_top++;
            s_stack[s_top] = s;
            if (s->vtable && s->vtable->on_enter) s->vtable->on_enter(s, args);
        }
    }
    if (s_pending_switch) {
        Scene* s = s_pending_switch;
        void* args = s_pending_switch_args;
        s_pending_switch = NULL;
        s_pending_switch_args = NULL;
        if (s_top >= 0) {
            Scene* prev = s_stack[s_top];
            if (prev && prev->vtable && prev->vtable->on_exit) prev->vtable->on_exit(prev);
            s_stack[s_top] = s;
            if (s->vtable && s->vtable->on_enter) s->vtable->on_enter(s, args);
        } else {
            if (s_top + 1 < MAX_SCENES) {
                s_top++;
                s_stack[s_top] = s;
                if (s->vtable && s->vtable->on_enter) s->vtable->on_enter(s, args);
            }
        }
    }
}

void scene_manager_update(void) {
    transition_update();
    if (transition_done()) {
        apply_pending();
    }
    /* Only update scene logic when no transition is in progress to prevent
     * re-entrant navigation triggers during scene fades. */
    if (!transition_in_progress()) {
        Scene* cur = scene_manager_current();
        if (cur && cur->vtable && cur->vtable->update) cur->vtable->update(cur);
    }
}

void scene_manager_draw(ScreenId screen) {
    Scene* cur = scene_manager_current();
    if (cur && cur->vtable && cur->vtable->draw) {
        cur->vtable->draw(cur, screen);
    }
    /* If a transition is running, the outgoing scene draws over/under
     * an offset; transition.c applies that via render-target clip in
     * main, simpler is to just call it here. */
    if (transition_in_progress()) {
        transition_draw(screen);
    }
}
