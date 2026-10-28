#include "../tea_timer_i.h"
#include "../lib/tea_logic/tea_preset_load.h"
#include "tea_timer_scene.h"

#define TAG "TeaTimerFile"

/** Показать диалог с причиной, по которой файл не принят (C-UI-10). */
static void tea_timer_scene_file_select_show_error(TeaTimerApp* app, const PresetError* err) {
    char text[64];
    switch(err->result) {
    case TeaPresetResultErrTooLarge:
        snprintf(text, sizeof(text), "File is larger than 1 KB");
        break;
    case TeaPresetResultErrNoField:
        snprintf(text, sizeof(text), "Missing field: %s", err->error_field);
        break;
    case TeaPresetResultErrBadNumber:
        snprintf(text, sizeof(text), "Not a number: %s", err->error_field);
        break;
    case TeaPresetResultErrOutOfRange:
        snprintf(text, sizeof(text), "Out of range: %s", err->error_field);
        break;
    case TeaPresetResultErrDuplicateField:
        snprintf(text, sizeof(text), "Duplicate field: %s", err->error_field);
        break;
    case TeaPresetResultErrEmptyName:
        snprintf(text, sizeof(text), "Name is empty");
        break;
    case TeaPresetResultErrBadName:
        snprintf(text, sizeof(text), "Bad name: %s", err->error_field);
        break;
    case TeaPresetResultErrNotFound:
    default:
        snprintf(text, sizeof(text), "Cannot open file");
        break;
    }

    FURI_LOG_W(TAG, "preset error %d field=%s", (int)err->result, err->error_field);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Cannot use file", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, "Retry", NULL, "OK");
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

/** Загрузить выбранный файл и перейти к карточке пресета (T024a, T024b). */
static void tea_timer_scene_file_select_load_preset(TeaTimerApp* app) {
    const char* path = furi_string_get_cstr(app->file_path);

    // Серия сохраняется между открытиями одного файла и сбрасывается при выборе другого.
    bool same_preset = app->session.has_preset && (strcmp(path, app->session.preset.path) == 0);

    TeaPreset preset;
    PresetError err;
    TeaPresetResult result =
        tea_preset_load(app->storage, path, app->parse_buf, TEA_PARSE_BUF_SIZE, &preset, &err);
    if(result != TeaPresetResultOK) {
        tea_timer_scene_file_select_show_error(app, &err);
        return;
    }

    if(same_preset) {
        app->session.preset = preset;
    } else {
        brew_session_reset_from_preset(&preset, &app->session);
    }

    FURI_LOG_I(TAG, "preset loaded: %s", preset.name);
    scene_manager_next_scene(app->scene_manager, TeaTimerScenePresetCard);
}

void tea_timer_scene_file_select_file_callback(void* context) {
    TeaTimerApp* app = context;
    view_dispatcher_send_custom_event(app->view_dispatcher, TeaTimerEventFileSelected);
}

void tea_timer_scene_file_select_on_enter(void* context) {
    TeaTimerApp* app = context;

    FURI_LOG_I(TAG, "file select at %s", TEA_TIMER_APP_DATA_PATH);
    furi_string_set(app->file_path, TEA_TIMER_APP_DATA_PATH);
    file_browser_start(app->file_browser, app->file_path);
    view_dispatcher_switch_to_view(app->view_dispatcher, TeaTimerViewFileSelect);
}

bool tea_timer_scene_file_select_on_event(void* context, SceneManagerEvent event) {
    TeaTimerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == TeaTimerEventFileSelected) {
            tea_timer_scene_file_select_load_preset(app);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void tea_timer_scene_file_select_on_exit(void* context) {
    TeaTimerApp* app = context;
    file_browser_stop(app->file_browser);
}
