#include "../tea_timer_i.h"
#include "tea_timer_scene.h"

#define TAG "TeaTimerMenu"

/** Индекс пункта `Presets` в главном меню. */
#define TEA_TIMER_MENU_INDEX_PRESETS 0

/**
 * Маршрутизация перед показом списка файлов (storage-layout.md, «Матрица состояний носителя»):
 * нет SD → `Error`; каталога нет или он пуст → `NoPresets`; иначе → `FileSelect`.
 */
void tea_timer_show_presets(TeaTimerApp* app) {
    furi_assert(app != NULL);

    FS_Error status = storage_sd_status(app->storage);
    if(status != FSE_OK) {
        FURI_LOG_W(TAG, "sd status %d", (int)status);
        scene_manager_next_scene(app->scene_manager, TeaTimerSceneError);
        return;
    }

    if(!tea_timer_has_presets(app->storage)) {
        FURI_LOG_I(TAG, "no presets in %s", TEA_TIMER_APP_DATA_PATH);
        scene_manager_next_scene(app->scene_manager, TeaTimerSceneNoPresets);
        return;
    }

    scene_manager_next_scene(app->scene_manager, TeaTimerSceneFileSelect);
}

static void tea_timer_scene_menu_submenu_callback(void* context, uint32_t index) {
    TeaTimerApp* app = context;
    if(index == TEA_TIMER_MENU_INDEX_PRESETS) {
        tea_timer_show_presets(app);
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
