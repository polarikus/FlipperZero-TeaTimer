#pragma once

#include <gui/scene_manager.h>

#include "tea_timer_scene_config.h"

/** Сцены приложения Tea Timer. */
typedef enum {
    TeaTimerSceneMenu,
    TeaTimerSceneFileSelect,
    TeaTimerScenePresetCard,
    TeaTimerSceneBrew,
    TeaTimerSceneCount,
} TeaTimerScene;

ADD_SCENE(tea_timer, menu, Menu)
ADD_SCENE(tea_timer, file_select, FileSelect)
ADD_SCENE(tea_timer, preset_card, PresetCard)
ADD_SCENE(tea_timer, brew, Brew)

/** Callback файлового браузера: выбран файл пресета. */
void tea_timer_scene_file_select_file_callback(void* context);

extern const SceneManagerHandlers tea_timer_scene_handlers;
