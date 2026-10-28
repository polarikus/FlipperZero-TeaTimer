#include "tea_timer_scene.h"

static void (*const tea_timer_scene_on_enter_handlers[])(void*) = {
    tea_timer_scene_menu_on_enter,
    tea_timer_scene_file_select_on_enter,
    tea_timer_scene_preset_card_on_enter,
    tea_timer_scene_brew_on_enter,
    tea_timer_scene_no_presets_on_enter,
    tea_timer_scene_error_on_enter,
};

static bool (*const tea_timer_scene_on_event_handlers[])(void*, SceneManagerEvent) = {
    tea_timer_scene_menu_on_event,
    tea_timer_scene_file_select_on_event,
    tea_timer_scene_preset_card_on_event,
    tea_timer_scene_brew_on_event,
    tea_timer_scene_no_presets_on_event,
    tea_timer_scene_error_on_event,
};

static void (*const tea_timer_scene_on_exit_handlers[])(void*) = {
    tea_timer_scene_menu_on_exit,
    tea_timer_scene_file_select_on_exit,
    tea_timer_scene_preset_card_on_exit,
    tea_timer_scene_brew_on_exit,
    tea_timer_scene_no_presets_on_exit,
    tea_timer_scene_error_on_exit,
};

const SceneManagerHandlers tea_timer_scene_handlers = {
    .on_enter_handlers = tea_timer_scene_on_enter_handlers,
    .on_event_handlers = tea_timer_scene_on_event_handlers,
    .on_exit_handlers = tea_timer_scene_on_exit_handlers,
    .scene_num = TeaTimerSceneCount,
};
