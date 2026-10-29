#include "../tea_timer_i.h"
#include "tea_timer_scene.h"

/** Заглушка каркаса 002: сцена `Actions` реализуется в US3. */
void tea_timer_scene_actions_on_enter(void* context) {
    TeaTimerApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, TeaTimerViewActions);
}

bool tea_timer_scene_actions_on_event(void* context, SceneManagerEvent event) {
    TeaTimerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void tea_timer_scene_actions_on_exit(void* context) {
    UNUSED(context);
}
