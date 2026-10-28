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

/** Каталог данных приложения на SD-карте. */
#define TEA_TIMER_APP_DATA_PATH APP_DATA_PATH("")

/** Идентификаторы view, зарегистрированных в ViewDispatcher. */
typedef enum {
    TeaTimerViewMenu, /**< Главное меню (submenu) */
    TeaTimerViewFileSelect, /**< Выбор файла пресета (file_browser) */
    TeaTimerViewPresetCard, /**< Карточка пресета = экран ожидания (widget) */
    TeaTimerViewBrew, /**< Экран заваривания (собственный View) */
    TeaTimerViewCount,
} TeaTimerViewId;

/** Состояние приложения: владение ресурсами и данные сессии. */
typedef struct {
    Gui* gui;
    NotificationApp* notification;
    Storage* storage;
    DialogsApp* dialogs;
    ViewDispatcher* view_dispatcher;
    SceneManager* scene_manager;
    FuriString* file_path; /**< Путь, выбранный в файловом браузере */
    Submenu* submenu; /**< Главное меню */
    FileBrowser* file_browser; /**< Системный выбор `.tea` */
    Widget* preset_card; /**< Карточка пресета (IDLE) */
    Widget* no_presets; /**< Экран «нет пресетов» (реализуется в US3) */
    BrewView* brew_view; /**< Экран заваривания */
    BrewSession session; /**< Текущая сессия заваривания */
    char* parse_buf; /**< Буфер разбора пресета (TEA_PARSE_BUF_SIZE) в куче */
} TeaTimerApp;
