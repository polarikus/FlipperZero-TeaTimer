#pragma once

#include <gui/scene_manager.h>
#include <storage/storage.h>

#include "tea_timer_scene_config.h"

/** Предварительное объявление состояния приложения (полное — в `tea_timer_i.h`). */
typedef struct TeaTimerApp TeaTimerApp;

/** Сцены приложения Tea Timer. */
typedef enum {
    TeaTimerSceneMenu,
    TeaTimerSceneFileSelect,
    TeaTimerScenePresetCard,
    TeaTimerSceneBrew,
    TeaTimerSceneNoPresets, /**< Каталог пуст или отсутствует (FR-008) */
    TeaTimerSceneError, /**< SD-карта недоступна (FR-024) */
    TeaTimerSceneCount,
} TeaTimerScene;

ADD_SCENE(tea_timer, menu, Menu)
ADD_SCENE(tea_timer, file_select, FileSelect)
ADD_SCENE(tea_timer, preset_card, PresetCard)
ADD_SCENE(tea_timer, brew, Brew)
ADD_SCENE(tea_timer, no_presets, NoPresets)
ADD_SCENE(tea_timer, error, Error)

/** Callback файлового браузера: выбран файл пресета. */
void tea_timer_scene_file_select_file_callback(void* context);

/** Маршрутизация «Presets»: Error / NoPresets / FileSelect по состоянию носителя. */
void tea_timer_show_presets(TeaTimerApp* app);

/** Есть ли в корне каталога приложения хотя бы один файл `.tea`. */
bool tea_timer_has_presets(Storage* storage);

extern const SceneManagerHandlers tea_timer_scene_handlers;
