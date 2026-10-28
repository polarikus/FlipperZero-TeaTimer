#include "../../minunit.h"

#include <furi.h>
#include <storage/storage.h>

#include "../../../tea_timer.h"
#include "../../../lib/tea_logic/tea_preset.h"
#include "../../../lib/tea_logic/tea_preset_load.h"

/** Каталог временных файлов теста (создаётся и удаляется самим тестом). */
#define TEST_DIR          APP_DATA_PATH("test")
#define TEST_FILE_OK      APP_DATA_PATH("test/ok.tea")
#define TEST_FILE_1024    APP_DATA_PATH("test/size1024.tea")
#define TEST_FILE_1025    APP_DATA_PATH("test/size1025.tea")
#define TEST_FILE_MISSING APP_DATA_PATH("test/missing.tea")

/** Статический буфер: стек CLI-потока теста ограничен (2048 байт). */
static char test_file_buffer[TEA_PARSE_BUF_SIZE + 16];

static const char* const tea_reference_file = "# Green Tea - 5 g per 100 ml, 6 pours\n"
                                              "name: Green Tea\n"
                                              "leaf_g_per_100ml: 5.0\n"
                                              "first_pour_sec: 20\n"
                                              "increment_sec: 5\n"
                                              "max_pours: 6\n";

/** Эталонный файл из contracts/tea-file-format.md. */
static const char* const reference_file = "# Green Tea - 5 g per 100 ml, 6 pours\n"
                                          "name: Green Tea\n"
                                          "leaf_g_per_100ml: 5.0\n"
                                          "first_pour_sec: 20\n"
                                          "increment_sec: 5\n"
                                          "max_pours: 6\n";

/** Заготовка корректного файла с подстановкой одного поля. */
static TeaPresetResult
    parse_text(const char* text, TeaPreset* preset, char* field, size_t field_size) {
    return tea_preset_parse_buf(text, strlen(text), preset, field, field_size);
}

MU_TEST(test_reference_file) {
    TeaPreset preset;
    char field[24];
    mu_assert_int_eq(TeaPresetResultOK, parse_text(reference_file, &preset, field, sizeof(field)));
    mu_assert_string_eq("Green Tea", preset.name);
    mu_assert_int_eq(50, preset.leaf_tenths_g);
    mu_assert_int_eq(20, preset.first_pour_sec);
    mu_assert_int_eq(5, preset.increment_sec);
    mu_assert_int_eq(6, preset.max_pours);
}

MU_TEST(test_crlf_and_bom) {
    TeaPreset preset;
    char field[24];

    const char* crlf = "name: Oolong\r\n"
                       "leaf_g_per_100ml: 5.0\r\n"
                       "first_pour_sec: 20\r\n"
                       "increment_sec: 5\r\n"
                       "max_pours: 6\r\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(crlf, &preset, field, sizeof(field)));
    mu_assert_string_eq("Oolong", preset.name);

    const char* bom = "\xEF\xBB\xBFname: Green Tea\n"
                      "leaf_g_per_100ml: 5.0\n"
                      "first_pour_sec: 20\n"
                      "increment_sec: 5\n"
                      "max_pours: 6\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(bom, &preset, field, sizeof(field)));
}

MU_TEST(test_comments_and_empty_lines) {
    TeaPreset preset;
    char field[24];

    const char* text = "\n"
                       "# comment\n"
                       "   \n"
                       "  name:   Puer  \n"
                       "leaf_g_per_100ml: 5.0\n"
                       "\n"
                       "  # another comment\n"
                       "first_pour_sec: 20\n"
                       "increment_sec: 5\n"
                       "max_pours: 6\n"
                       "\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(text, &preset, field, sizeof(field)));
    mu_assert_string_eq("Puer", preset.name);
}

MU_TEST(test_leaf_boundaries) {
    TeaPreset preset;
    char field[24];

    const char* low =
        "name: A\nleaf_g_per_100ml: 0.1\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(low, &preset, field, sizeof(field)));
    mu_assert_int_eq(1, preset.leaf_tenths_g);

    const char* high =
        "name: A\nleaf_g_per_100ml: 50.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(high, &preset, field, sizeof(field)));
    mu_assert_int_eq(500, preset.leaf_tenths_g);

    const char* integer_invalid =
        "name: A\nleaf_g_per_100ml: 5\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrBadNumber, parse_text(integer_invalid, &preset, field, sizeof(field)));
    mu_assert_string_eq("leaf_g_per_100ml", field);

    const char* two_digits =
        "name: A\nleaf_g_per_100ml: 5.00\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrBadNumber, parse_text(two_digits, &preset, field, sizeof(field)));

    const char* no_integer =
        "name: A\nleaf_g_per_100ml: .5\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrBadNumber, parse_text(no_integer, &preset, field, sizeof(field)));

    const char* zero =
        "name: A\nleaf_g_per_100ml: 0.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(zero, &preset, field, sizeof(field)));

    const char* too_high =
        "name: A\nleaf_g_per_100ml: 50.1\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(too_high, &preset, field, sizeof(field)));
}

MU_TEST(test_numeric_boundaries) {
    TeaPreset preset;
    char field[24];

    const char* minimums =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 1\nincrement_sec: 0\nmax_pours: 1\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(minimums, &preset, field, sizeof(field)));
    mu_assert_int_eq(1, preset.first_pour_sec);
    mu_assert_int_eq(0, preset.increment_sec);
    mu_assert_int_eq(1, preset.max_pours);

    const char* maximums =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 3600\nincrement_sec: 600\nmax_pours: 99\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(maximums, &preset, field, sizeof(field)));
    mu_assert_int_eq(3600, preset.first_pour_sec);
    mu_assert_int_eq(600, preset.increment_sec);
    mu_assert_int_eq(99, preset.max_pours);

    const char* first_zero =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 0\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(first_zero, &preset, field, sizeof(field)));
    mu_assert_string_eq("first_pour_sec", field);

    const char* first_big =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 3601\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(first_big, &preset, field, sizeof(field)));

    const char* increment_big =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 601\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(increment_big, &preset, field, sizeof(field)));

    const char* max_zero =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 0\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(max_zero, &preset, field, sizeof(field)));

    const char* max_big =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 100\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(max_big, &preset, field, sizeof(field)));
}

MU_TEST(test_garbage_after_number) {
    TeaPreset preset;
    char field[24];

    const char* text =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20abc\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(TeaPresetResultErrBadNumber, parse_text(text, &preset, field, sizeof(field)));
    mu_assert_string_eq("first_pour_sec", field);
}

MU_TEST(test_missing_field) {
    TeaPreset preset;
    char field[24];

    const char* text = "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\n";
    mu_assert_int_eq(TeaPresetResultErrNoField, parse_text(text, &preset, field, sizeof(field)));
    mu_assert_string_eq("max_pours", field);
}

MU_TEST(test_name_errors) {
    TeaPreset preset;
    char field[24];

    const char* empty =
        "name:\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrEmptyName, parse_text(empty, &preset, field, sizeof(field)));

    const char* duplicate =
        "name: A\nname: B\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrDuplicateField, parse_text(duplicate, &preset, field, sizeof(field)));
    mu_assert_string_eq("name", field);

    const char* too_long =
        "name: 1234567890123456789012345\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrOutOfRange, parse_text(too_long, &preset, field, sizeof(field)));

    const char* bad_chars =
        "name: Green-Tea\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n";
    mu_assert_int_eq(
        TeaPresetResultErrBadName, parse_text(bad_chars, &preset, field, sizeof(field)));
    mu_assert_string_eq("name", field);
}

MU_TEST(test_unknown_key_ignored) {
    TeaPreset preset;
    char field[24];

    const char* text =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n"
        "temp_c: 90\n";
    mu_assert_int_eq(TeaPresetResultOK, parse_text(text, &preset, field, sizeof(field)));
    mu_assert_int_eq(0, field[0]);
}

MU_TEST(test_duplicate_numeric_field) {
    TeaPreset preset;
    char field[24];

    const char* text =
        "name: A\nleaf_g_per_100ml: 5.0\nfirst_pour_sec: 20\nincrement_sec: 5\nmax_pours: 6\n"
        "max_pours: 7\n";
    mu_assert_int_eq(
        TeaPresetResultErrDuplicateField, parse_text(text, &preset, field, sizeof(field)));
    mu_assert_string_eq("max_pours", field);
}

/** Записать файл целиком через Storage. */
static bool test_write_file(Storage* storage, const char* path, const char* data, size_t size) {
    File* file = storage_file_alloc(storage);
    bool written = storage_file_open(file, path, FSAM_WRITE, FSOM_CREATE_ALWAYS);
    if(written) {
        written = storage_file_write(file, data, size) == size;
    }
    storage_file_close(file);
    storage_file_free(file);
    return written;
}

/** Эталонный файл, дополненный комментариями до нужного размера в байтах. */
static size_t test_fill_reference(char* out, size_t target_size) {
    size_t base = strlen(tea_reference_file);
    memcpy(out, tea_reference_file, base);
    size_t pos = base;
    while(pos + 1 < target_size) {
        out[pos++] = '#';
    }
    if(pos < target_size) {
        out[pos++] = '\n';
    }
    return pos;
}

MU_TEST(test_load_from_sd) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    furi_check(storage != NULL);

    storage_common_mkdir(storage, TEST_DIR);

    bool written_ok =
        test_write_file(storage, TEST_FILE_OK, tea_reference_file, strlen(tea_reference_file));

    size_t size_1024 = test_fill_reference(test_file_buffer, 1024);
    bool written_1024 = test_write_file(storage, TEST_FILE_1024, test_file_buffer, size_1024);

    size_t size_1025 = test_fill_reference(test_file_buffer, 1025);
    bool written_1025 = test_write_file(storage, TEST_FILE_1025, test_file_buffer, size_1025);

    TeaPreset preset;
    PresetError err;
    TeaPresetResult result_ok = tea_preset_load(
        storage, TEST_FILE_OK, test_file_buffer, sizeof(test_file_buffer), &preset, &err);

    TeaPreset preset_big;
    PresetError err_big;
    TeaPresetResult result_1024 = tea_preset_load(
        storage, TEST_FILE_1024, test_file_buffer, sizeof(test_file_buffer), &preset_big, &err_big);

    PresetError err_1025;
    TeaPresetResult result_1025 = tea_preset_load(
        storage,
        TEST_FILE_1025,
        test_file_buffer,
        sizeof(test_file_buffer),
        &preset_big,
        &err_1025);

    PresetError err_missing;
    TeaPresetResult result_missing = tea_preset_load(
        storage,
        TEST_FILE_MISSING,
        test_file_buffer,
        sizeof(test_file_buffer),
        &preset_big,
        &err_missing);

    // Тест сам убирает за собой данные.
    storage_common_remove(storage, TEST_FILE_OK);
    storage_common_remove(storage, TEST_FILE_1024);
    storage_common_remove(storage, TEST_FILE_1025);
    storage_common_remove(storage, TEST_DIR);
    furi_record_close(RECORD_STORAGE);

    mu_assert("files must be written", written_ok && written_1024 && written_1025);
    mu_assert_int_eq(TeaPresetResultOK, result_ok);
    mu_assert_string_eq("Green Tea", preset.name);
    mu_assert_int_eq(50, preset.leaf_tenths_g);
    mu_assert_string_eq(TEST_FILE_OK, preset.path);
    mu_assert_int_eq(TeaPresetResultOK, result_1024);
    mu_assert_int_eq(TeaPresetResultErrTooLarge, result_1025);
    mu_assert_int_eq(TeaPresetResultErrNotFound, result_missing);
}

static void run_minunit_test_tea_preset_suite(void) {
    MU_RUN_TEST(test_reference_file);
    MU_RUN_TEST(test_crlf_and_bom);
    MU_RUN_TEST(test_comments_and_empty_lines);
    MU_RUN_TEST(test_leaf_boundaries);
    MU_RUN_TEST(test_numeric_boundaries);
    MU_RUN_TEST(test_garbage_after_number);
    MU_RUN_TEST(test_missing_field);
    MU_RUN_TEST(test_name_errors);
    MU_RUN_TEST(test_unknown_key_ignored);
    MU_RUN_TEST(test_duplicate_numeric_field);
    MU_RUN_TEST(test_load_from_sd);
}

int run_minunit_test_tea_preset(void) {
    run_minunit_test_tea_preset_suite();
    return minunit_status;
}
