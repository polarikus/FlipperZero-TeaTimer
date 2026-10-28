#include "../tea_timer_i.h"
#include "tea_timer_scene.h"

#define TAG "TeaTimerMenu"

/** Индекс пункта `Presets` в главном меню. */
#define TEA_TIMER_MENU_INDEX_PRESETS 0

static void tea_timer_scene_menu_submenu_callback(void* context, uint32_t index) {
    TeaTimerApp* app = context;
    if(index == TEA_TIMER_MENU_INDEX_PRESETS) {
        scene_manager_next_scene(app->scene_manager, TeaTimerSceneFileSelect);
    }
}

void tea_timer_scene_menu_on_enter(void* context) {
    TeaTimerApp* app = context;

    submenu_reset(app->submenu);
    submenu_set_header(app->submenu, "Tea Timer");
    submenu_add_item(
        app->submenu,
        "Presets",
        TEA_TIMER_MENU_INDEX_PRESETS,
        tea_timer_scene_menu_submenu_callback,
        app);

    view_dispatcher_switch_to_view(app->view_dispatcher, TeaTimerViewMenu);
}

bool tea_timer_scene_menu_on_event(void* context, SceneManagerEvent event) {
    TeaTimerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // BACK в корневой сцене завершает приложение.
        FURI_LOG_I(TAG, "exit by back");
        view_dispatcher_stop(app->view_dispatcher);
        consumed = true;
    }

    return consumed;
}

void tea_timer_scene_menu_on_exit(void* context) {
    UNUSED(context);
}
