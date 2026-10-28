#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/scene_manager.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/file_browser.h>
#include <gui/modules/widget.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include <storage/storage.h>
#include <dialogs/dialogs.h>

#include "tea_timer.h"
#include "tea_timer_custom_event.h"
#include "lib/tea_logic/brew_math.h"
#include "views/brew_view.h"

/**
 * Каталог данных приложения на SD-карте.
 *
 * Используется алиас `/data` без завершающего слэша: прошивка разворачивает его
 * в `/ext/apps_data/<appid>`. Слэш в конце (`APP_DATA_PATH("")` -> `/data/`)
 * приводит к `FSE_INVALID_NAME` при открытии каталога.
 */
#define TEA_TIMER_APP_DATA_PATH STORAGE_APP_DATA_PATH_PREFIX

/** Идентификаторы view, зарегистрированных в ViewDispatcher. */
typedef enum {
    TeaTimerViewMenu, /**< Главное меню (submenu) */
    TeaTimerViewFileSelect, /**< Выбор файла пресета (file_browser) */
    TeaTimerViewPresetCard, /**< Карточка пресета = экран ожидания (widget) */
    TeaTimerViewBrew, /**< Экран заваривания (собственный View) */
    TeaTimerViewNoPresets, /**< Каталог пуст или отсутствует (widget, FR-008) */
    TeaTimerViewError, /**< SD-карта недоступна (widget, FR-024) */
    TeaTimerViewCount,
} TeaTimerViewId;

/** Состояние приложения: владение ресурсами и данные сессии. */
typedef struct TeaTimerApp {
    Gui* gui;
    NotificationApp* notification;
    Storage* storage;
    DialogsApp* dialogs;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    FuriString* file_path; /**< Путь, выбранный в файловом браузере */
    Submenu* submenu; /**< Главное меню */
    FileBrowser* file_browser; /**< Системный выбор `.tea` */
    bool file_browser_started; /**< Браузер запущен: file_browser_stop обязателен (принцип V) */
    Widget* preset_card; /**< Карточка пресета (IDLE) */
    Widget* no_presets; /**< Экран «нет пресетов» */
    Widget* error; /**< Экран «нет SD-карты» */
    BrewView* brew_view; /**< Экран заваривания */
    BrewSession session; /**< Текущая сессия заваривания */
    char* parse_buf; /**< Буфер разбора пресета (TEA_PARSE_BUF_SIZE) в куче */
} TeaTimerApp;
