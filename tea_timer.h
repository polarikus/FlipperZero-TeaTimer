#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Максимальная длина названия пресета в байтах (без терминатора). */
#define TEA_NAME_MAX 24

/** Максимальная длина пути к файлу пресета. */
#define TEA_PATH_MAX 128

/** Размер буфера разбора: 1024 байта файла + терминатор. */
#define TEA_PARSE_BUF_SIZE 1025

/** Максимальный допустимый размер файла пресета. */
#define TEA_FILE_MAX_SIZE 1024

/** Результат разбора и загрузки файла пресета. */
typedef enum {
    TeaPresetResultOK, /**< Файл корректен */
    TeaPresetResultErrNotFound, /**< Файл не открывается */
    TeaPresetResultErrTooLarge, /**< Файл больше 1 КБ */
    TeaPresetResultErrNoField, /**< Отсутствует обязательное поле */
    TeaPresetResultErrBadNumber, /**< Значение не является числом в нужном формате */
    TeaPresetResultErrOutOfRange, /**< Значение вне допустимого диапазона */
    TeaPresetResultErrDuplicateField, /**< Поле встречается дважды */
    TeaPresetResultErrEmptyName, /**< Пустое название */
    TeaPresetResultErrBadName, /**< Недопустимые символы в названии */
} TeaPresetResult;

/** Описание ошибки чтения пресета (для диалога и логов). */
typedef struct {
    TeaPresetResult result; /**< Код результата */
    char error_field[24]; /**< Имя проблемного ключа или пустая строка */
    char file_name[32]; /**< Имя файла (basename) для текста диалога */
} PresetError;

/** Пресет чая, загруженный из файла `.tea`. */
typedef struct {
    char name[TEA_NAME_MAX + 1]; /**< 1–24 байта: латиница, цифры, пробел, `_` */
    uint16_t leaf_tenths_g; /**< Граммовка в десятых долях: 1–500 (0.1–50.0 г) */
    uint16_t first_pour_sec; /**< Длительность первого пролива: 1–3600 с */
    uint16_t increment_sec; /**< Приращение на каждый следующий пролив: 0–600 с */
    uint8_t max_pours; /**< Рекомендованное число проливов: 1–99 */
    char path[TEA_PATH_MAX]; /**< Путь к файлу пресета на SD-карте */
} TeaPreset;

/** Точка входа приложения. */
int32_t tea_timer_app(void* p);

#ifdef __cplusplus
}
#endif
