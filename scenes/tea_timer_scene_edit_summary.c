#include "../tea_timer_i.h"
#include "../lib/tea_logic/tea_preset_save.h"
#include "tea_timer_scene.h"

#define TAG "TeaTimerSummary"

/** Показать простое сообщение об ошибке записи (полные варианты — C-UI-20). */
static void tea_timer_scene_edit_summary_show_error(TeaTimerApp* app, PresetSaveResult result) {
    const char* text = "Cannot save file";
    if(result == PresetSaveErrNoName) {
        text = "Too many copies";
    } else if(result == PresetSaveErrDraft) {
        text = "Bad value";
    }

    FURI_LOG_E(TAG, "save failed: %d", (int)result);

    DialogMessage* message = dialog_message_alloc();
    dialog_message_set_header(message, "Cannot save", 64, 2, AlignCenter, AlignTop);
    dialog_message_set_text(message, text, 64, 32, AlignCenter, AlignCenter);
    dialog_message_set_buttons(message, NULL, NULL, "OK");
    dialog_message_show(app->dialogs, message);
    dialog_message_free(message);
}

/** Basename пути для экрана результата. */
static void tea_timer_scene_edit_summary_basename(const char* path, char* out, size_t size) {
    const char* base = path;
    for(const char* p = path; *p != '\0'; p++) {
        if(*p == '/') {
            base = p + 1;
        }
    }
    size_t len = strlen(base);
    if(len > size - 1) {
        len = size - 1;
    }
    memcpy(out, base, len);
    out[len] = '\0';
}

/** `Save` на сводке: проверка черновика, запись файла, экран `Saved`. */
static void tea_timer_scene_edit_summary_save_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    UNUSED(result);
    if(type != InputTypeShort) {
        return;
    }

    TeaTimerApp* app = context;

    char field[24];
    TeaPresetResult check = tea_preset_check_draft_ex(&app->draft, field, sizeof(field));
    if(check != TeaPresetResultOK) {
        FURI_LOG_E(TAG, "bad draft: %d field=%s", (int)check, field);
        DialogMessage* message = dialog_message_alloc();
        dialog_message_set_header(message, "Bad value", 64, 2, AlignCenter, AlignTop);
        dialog_message_set_text(message, field, 64, 32, AlignCenter, AlignCenter);
        dialog_message_set_buttons(message, NULL, NULL, "OK");
        dialog_message_show(app->dialogs, message);
        dialog_message_free(message);
        return;
    }

    char path[TEA_PATH_MAX];
    PresetSaveResult saved =
        tea_preset_save(app->storage, &app->draft, app->editing, path, sizeof(path));
    if(saved != PresetSaveOK) {
        tea_timer_scene_edit_summary_show_error(app, saved);
        return;
    }

    FURI_LOG_I(TAG, "saved %s", path);
    tea_timer_scene_edit_summary_basename(path, app->saved_file, sizeof(app->saved_file));
    app->saved_kind = TeaSavedKindSaved;
    scene_manager_next_scene(app->scene_manager, TeaTimerSceneSaved);
}

void tea_timer_scene_edit_summary_on_enter(void* context) {
    TeaTimerApp* app = context;
    Widget* widget = app->edit_summary;
    char buffer[48];

    widget_reset(widget);
    widget_add_string_element(
        widget,
        64,
        2,
        AlignCenter,
        AlignTop,
        FontPrimary,
        app->editing ? "Edit preset" : "New preset");

    widget_add_string_element(
        widget, 64, 14, AlignCenter, AlignTop, FontSecondary, app->draft.name);

    snprintf(
        buffer,
        sizeof(buffer),
        "Leaf %u.%u g/100ml",
        (unsigned)(app->draft.leaf_tenths_g / 10U),
        (unsigned)(app->draft.leaf_tenths_g % 10U));
    widget_add_string_element(widget, 64, 24, AlignCenter, AlignTop, FontSecondary, buffer);

    char first[16];
    char step[16];
    brew_math_format_time(app->draft.first_pour_sec, first, sizeof(first));
    brew_math_format_time(app->draft.increment_sec, step, sizeof(step));
    snprintf(buffer, sizeof(buffer), "First %s  Step %s", first, step);
    widget_add_string_element(widget, 64, 34, AlignCenter, AlignTop, FontSecondary, buffer);

    snprintf(buffer, sizeof(buffer), "Max %u pours", (unsigned)app->draft.max_pours);
    widget_add_string_element(widget, 64, 44, AlignCenter, AlignTop, FontSecondary, buffer);

    widget_add_button_element(
        widget, GuiButtonTypeCenter, "Save", tea_timer_scene_edit_summary_save_callback, app);

    view_dispatcher_switch_to_view(app->view_dispatcher, TeaTimerViewEditSummary);
}

bool tea_timer_scene_edit_summary_on_event(void* context, SceneManagerEvent event) {
    TeaTimerApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeBack) {
        // Назад к последнему шагу формы; файл не пишется.
        app->edit_step = TeaEditStepMaxPours;
        scene_manager_previous_scene(app->scene_manager);
        consumed = true;
    }

    return consumed;
}

void tea_timer_scene_edit_summary_on_exit(void* context) {
    UNUSED(context);
}
