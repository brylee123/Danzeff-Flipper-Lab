#include <furi.h>
#include <gui/canvas.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <storage/storage.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "danzeff_core.h"

#define TAG "DanzeffLab"
#define MENU_ITEMS 5

// Keep these unchanged across users to permit comparisons between sessions.
static const char* const trial_phrases[] = {
    "hello world",
    "flipper zero",
    "the quick brown fox",
};

typedef enum {
    LabPageMenu,
    LabPageHelp,
    LabPageEditor,
    LabPageResults,
} LabPage;

typedef struct {
    Gui* gui;
    Storage* storage;
    ViewPort* viewport;
    FuriMessageQueue* queue;
    FuriMutex* mutex;
    DzEditor editor;
    LabPage page;
    uint8_t menu_selection;
    int8_t trial_index; // -1: free edit; 0-2: timed typing task.
    bool running;
    bool trial_started;
    bool csv_saved;
    uint32_t trial_start_ms;
    uint32_t duration_ms;
    uint32_t buttons_pressed;
    uint32_t backspaces;
    uint32_t wrong_entries;
    uint32_t insertions;
    uint32_t wpm_x10;
} DanzeffLab;

static uint32_t lab_wpm_x10(size_t characters, uint32_t duration_ms) {
    if(duration_ms == 0) duration_ms = 1;
    return (uint32_t)((characters * 120000UL) / duration_ms);
}

static void lab_save_draft(DanzeffLab* app) {
    if(app->trial_index != -1) return;
    File* file = storage_file_alloc(app->storage);
    if(storage_file_open(file, APP_DATA_PATH("draft.txt"), FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        if(storage_file_write(file, app->editor.text, app->editor.length) != app->editor.length) {
            FURI_LOG_E(TAG, "Cannot write complete draft");
        }
    } else {
        FURI_LOG_E(TAG, "Cannot open draft.txt");
    }
    storage_file_close(file);
    storage_file_free(file);
}

static void lab_load_draft(DanzeffLab* app) {
    File* file = storage_file_alloc(app->storage);
    if(storage_file_open(file, APP_DATA_PATH("draft.txt"), FSAM_READ, FSOM_OPEN_EXISTING)) {
        size_t count = storage_file_read(file, app->editor.text, DZ_TEXT_CAPACITY - 1);
        app->editor.text[count] = '\0';
        // The editor intentionally supports ASCII; preserve only the first C string.
        app->editor.length = strlen(app->editor.text);
    }
    storage_file_close(file);
    storage_file_free(file);
}

static bool lab_save_trial(DanzeffLab* app) {
    bool ok = false;
    File* file = storage_file_alloc(app->storage);
    if(storage_file_open(file, APP_DATA_PATH("results.csv"), FSAM_WRITE, FSOM_OPEN_APPEND)) {
        bool header_ok = true;
        if(storage_file_size(file) == 0) {
            static const char header[] =
                "trial_id,target,duration_ms,wpm_x10,button_presses,backspaces,wrong_entries,insertions\n";
            header_ok = storage_file_write(file, header, sizeof(header) - 1) == sizeof(header) - 1;
            if(!header_ok) FURI_LOG_E(TAG, "Cannot write CSV header");
        }
        char row[192];
        int len = snprintf(
            row,
            sizeof(row),
            "%d,%s,%lu,%lu,%lu,%lu,%lu,%lu\n",
            (int)app->trial_index + 1,
            trial_phrases[app->trial_index],
            (unsigned long)app->duration_ms,
            (unsigned long)app->wpm_x10,
            (unsigned long)app->buttons_pressed,
            (unsigned long)app->backspaces,
            (unsigned long)app->wrong_entries,
            (unsigned long)app->insertions);
        if(len > 0 && (size_t)len < sizeof(row)) {
            ok = header_ok && (storage_file_write(file, row, (size_t)len) == (size_t)len);
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    if(!ok) FURI_LOG_E(TAG, "Failed saving trial results");
    return ok;
}

static void lab_begin_editor(DanzeffLab* app, int8_t trial_index) {
    dz_editor_init(&app->editor);
    app->trial_index = trial_index;
    app->trial_started = false;
    app->duration_ms = 0;
    app->buttons_pressed = 0;
    app->backspaces = 0;
    app->wrong_entries = 0;
    app->insertions = 0;
    app->wpm_x10 = 0;
    app->csv_saved = false;
    if(trial_index == -1) lab_load_draft(app);
    app->page = LabPageEditor;
}

static void lab_complete_trial(DanzeffLab* app) {
    app->duration_ms = (uint32_t)(furi_get_tick() - app->trial_start_ms);
    if(!app->duration_ms) app->duration_ms = 1;
    app->wpm_x10 = lab_wpm_x10(strlen(trial_phrases[app->trial_index]), app->duration_ms);
    app->csv_saved = lab_save_trial(app);
    app->page = LabPageResults;
}

static void lab_back_to_menu(DanzeffLab* app) {
    if(app->page == LabPageEditor && app->trial_index == -1) lab_save_draft(app);
    app->page = LabPageMenu;
}

static void lab_insert_choice(DanzeffLab* app, DzDirection direction) {
    size_t before = app->editor.length;
    char chosen = 0;
    bool changed = dz_editor_choose(&app->editor, direction, &chosen);
    if(!changed) return;
    if(chosen == '\b') {
        app->backspaces++;
    } else {
        app->insertions++;
        if(app->trial_index >= 0) {
            const char* target = trial_phrases[app->trial_index];
            if(before >= strlen(target) || target[before] != chosen) app->wrong_entries++;
        }
    }
    if(app->trial_index >= 0 &&
       strcmp(app->editor.text, trial_phrases[app->trial_index]) == 0) {
        lab_complete_trial(app);
    }
}

static void lab_handle_editor(DanzeffLab* app, const InputEvent* input) {
    if(input->type == InputTypePress && app->trial_index >= 0) {
        app->buttons_pressed++;
        if(!app->trial_started) {
            app->trial_start_ms = furi_get_tick();
            app->trial_started = true;
        }
    }

    if(input->type == InputTypeLong) {
        if(input->key == InputKeyBack) {
            lab_back_to_menu(app);
        } else if(input->key == InputKeyOk) {
            dz_editor_cycle_layer(&app->editor);
        } else if(app->editor.phase == DzPhaseGrid && input->key == InputKeyLeft) {
            dz_editor_step_layer(&app->editor, -1);
        } else if(app->editor.phase == DzPhaseGrid && input->key == InputKeyRight) {
            dz_editor_step_layer(&app->editor, 1);
        }
        return;
    }

    if(input->type != InputTypeShort && input->type != InputTypeRepeat) return;

    // Horizontal hold is reserved for layer changes. Ignore repeats emitted
    // after the long-press event so holding a button cannot move the cursor.
    if(input->type == InputTypeRepeat &&
       (input->key == InputKeyLeft || input->key == InputKeyRight)) return;

    if(app->editor.phase == DzPhaseChoose) {
        if(input->type != InputTypeShort) return;
        switch(input->key) {
        case InputKeyUp: lab_insert_choice(app, DzDirectionUp); break;
        case InputKeyLeft: lab_insert_choice(app, DzDirectionLeft); break;
        case InputKeyDown: lab_insert_choice(app, DzDirectionDown); break;
        case InputKeyRight: lab_insert_choice(app, DzDirectionRight); break;
        case InputKeyBack:
            // Cancel a pending choice without modifying the text buffer.
            dz_editor_cancel_choice(&app->editor);
            break;
        default: break;
        }
        return;
    }

    switch(input->key) {
    case InputKeyUp: dz_editor_move(&app->editor, 0, -1); break;
    case InputKeyLeft: dz_editor_move(&app->editor, -1, 0); break;
    case InputKeyDown: dz_editor_move(&app->editor, 0, 1); break;
    case InputKeyRight: dz_editor_move(&app->editor, 1, 0); break;
    case InputKeyOk:
        if(input->type == InputTypeShort) dz_editor_select_cell(&app->editor);
        break;
    case InputKeyBack:
        // Back is navigation only. Delete through the on-screen center
        // cell (OK, then Up), never through a hardware shortcut.
        if(input->type == InputTypeShort) lab_back_to_menu(app);
        break;
    default: break;
    }
}

static void lab_handle_input(DanzeffLab* app, const InputEvent* event) {
    if(app->page == LabPageEditor) {
        lab_handle_editor(app, event);
        return;
    }
    if(event->type == InputTypeLong && event->key == InputKeyBack) {
        if(app->page == LabPageMenu) app->running = false;
        else app->page = LabPageMenu;
        return;
    }
    if(event->type != InputTypeShort) return;
    if(app->page == LabPageMenu) {
        switch(event->key) {
        case InputKeyUp:
            if(app->menu_selection > 0) app->menu_selection--;
            break;
        case InputKeyDown:
            if(app->menu_selection < MENU_ITEMS - 1) app->menu_selection++;
            break;
        case InputKeyOk:
            if(app->menu_selection == 0) lab_begin_editor(app, -1);
            else if(app->menu_selection == MENU_ITEMS - 1) app->page = LabPageHelp;
            else lab_begin_editor(app, (int8_t)app->menu_selection - 1);
            break;
        case InputKeyBack: app->running = false; break;
        default: break;
        }
    } else if(app->page == LabPageHelp) {
        if(event->key == InputKeyBack || event->key == InputKeyOk) app->page = LabPageMenu;
    } else if(app->page == LabPageResults) {
        if(event->key == InputKeyOk) lab_begin_editor(app, app->trial_index);
        else if(event->key == InputKeyBack) app->page = LabPageMenu;
    }
}

// Space is rendered with a small U-shaped \u23b5 indicator, not a font glyph.
// The stock FontKeyboard does not reliably contain that Unicode symbol.
// This is display-only; the underlying editor continues to store ASCII ' '.
static char lab_visual_char(char c) {
    if(c == 0) return ' ';
    return c;
}

static size_t lab_key_width(Canvas* canvas, char character) {
    if(character == ' ') return 7; // 5px glyph + 1px padding on each side.
    if(character == '\b') return 11; // 9px glyph + 1px padding on each side.
    return canvas_glyph_width(canvas, (uint8_t)lab_visual_char(character));
}

static void lab_draw_key(Canvas* canvas, int32_t x, int32_t baseline, char character) {
    if(character == 0) return;
    if(character == '\b') {
        // Draw a 9x7 pixel U+232B ERASE TO THE LEFT (backspace) symbol.
        // Reserve 1px at either side inside the glyph's advance width.
        x += 1;
        // The icon has a left-pointing outline and a compact X inside.
        // A pixel drawing avoids relying on FontKeyboard Unicode support.
        canvas_draw_line(canvas, x + 3, baseline - 6, x + 8, baseline - 6);
        canvas_draw_line(canvas, x + 8, baseline - 6, x + 8, baseline);
        canvas_draw_line(canvas, x + 3, baseline, x + 8, baseline);
        canvas_draw_line(canvas, x, baseline - 3, x + 3, baseline - 6);
        canvas_draw_line(canvas, x, baseline - 3, x + 3, baseline);
        canvas_draw_line(canvas, x + 4, baseline - 4, x + 6, baseline - 2);
        canvas_draw_line(canvas, x + 6, baseline - 4, x + 4, baseline - 2);
        return;
    }
    if(character == ' ') {
        // 5x3-pixel U+23B5-like mark, matched to the v0.6 pixel mockup:
        // Reserve 1px at either side inside the glyph's advance width.
        x += 1;
        // #...#
        // #...#
        // #####
        // Keep text storage as ASCII ' '; this glyph is visual only.
        canvas_draw_line(canvas, x, baseline - 2, x, baseline);
        canvas_draw_line(canvas, x, baseline, x + 4, baseline);
        canvas_draw_line(canvas, x + 4, baseline, x + 4, baseline - 2);
        return;
    }
    canvas_draw_glyph(canvas, x, baseline, (uint8_t)lab_visual_char(character));
}

static void lab_draw_label(Canvas* canvas, uint8_t x, uint8_t y, const char* text, bool selected) {
    if(selected) {
        canvas_set_color(canvas, ColorBlack);
        canvas_draw_box(canvas, x, y, 122, 9);
        canvas_set_color(canvas, ColorWhite);
    } else {
        canvas_set_color(canvas, ColorBlack);
    }
    canvas_draw_str(canvas, x + 3, y + 8, text);
    canvas_set_color(canvas, ColorBlack);
}

static void lab_draw_menu(Canvas* canvas, const DanzeffLab* app) {
    static const char* const options[MENU_ITEMS] = {
        "Free text editor", "Trial: hello world", "Trial: flipper zero",
        "Trial: quick brown fox", "How to use",
    };
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 3, 10, "Danzeff Lab v0.9");
    canvas_set_font(canvas, FontSecondary);
    for(uint8_t i = 0; i < MENU_ITEMS; ++i) {
        lab_draw_label(canvas, 2, (uint8_t)(13 + i * 10), options[i], i == app->menu_selection);
    }
}

static void lab_draw_help(Canvas* canvas) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Danzeff controls");
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 21, "D-pad: move focus");
    canvas_draw_str(canvas, 2, 31, "OK: select cell / 2x border");
    canvas_draw_str(canvas, 2, 41, "D-pad: type, then home");
    canvas_draw_str(canvas, 2, 51, "Hold L/R: previous/next set");
    canvas_draw_str(canvas, 2, 61, "Hold OK: set Back:cancel/exit");
}

static void lab_draw_preview(Canvas* canvas, const DanzeffLab* app) {
    char line[40];
    canvas_set_color(canvas, ColorBlack);
    canvas_set_font(canvas, FontSecondary);
    // Reserve the upper line for the full trial target. Put the current
    // character set on the text line, including during timed trials.
    if(app->trial_index >= 0) {
        snprintf(line, sizeof(line), "%s", trial_phrases[app->trial_index]);
    } else {
        snprintf(line, sizeof(line), "Free edit");
    }
    canvas_draw_str(canvas, 2, 9, line);

    char layer_label[10];
    snprintf(layer_label, sizeof(layer_label), "[%s]", dz_layer_name(app->editor.layer));
    const uint8_t layer_x = (uint8_t)(126 - canvas_string_width(canvas, layer_label));
    const int32_t text_width = (int32_t)layer_x - 12;
    const char* tail = app->editor.text;
    // Bound both the pixel width and the preview buffer size.
    while(*tail && (canvas_string_width(canvas, tail) > text_width ||
                    strlen(tail) >= sizeof(line) - 2)) tail++;
    snprintf(line, sizeof(line), ">%s_", tail);
    canvas_draw_str(canvas, 2, 18, line);
    canvas_draw_str(canvas, layer_x, 18, layer_label);
}

// Layout experiment v0.5: anchor the compact cells to the LCD edges.
// Every nonfocused cell stays 40x12. The focused cell occupies the empty
// gutters between the fixed-size cells; the outer columns and bottom row
// remain flush with the LCD edges. No cell is resized except the focused one.
//
// Horizontal positions (128px LCD):
//   Compact:  left x=0..39, middle x=44..83, right x=88..127
//   Focus L:  x=0..43 (44px), middle compact x=44
//   Focus M:  x=40..87 (48px), fills both 4px gutters
//   Focus R:  x=84..127 (44px), middle compact ends at x=83
//
// Vertical positions (20..63): top anchored at y=20; bottom always
// reaches pixel 63. One expanded row is 20px and two compact rows are
// 12px each (total height 44px), so the focus absorbs the vertical gaps.
#define LAB_GRID_TOP 20
#define LAB_GRID_BOTTOM 63
#define LAB_CELL_WIDTH 40
#define LAB_CELL_HEIGHT 12
#define LAB_HOVER_HEIGHT 20
#define LAB_HOVER_EDGE_WIDTH 44
#define LAB_HOVER_CENTER_WIDTH 48

static int32_t lab_cell_x(uint8_t col, bool focused) {
    if(col == 0) return 0;
    if(col == 1) return focused ? 40 : 44;
    return focused ? 84 : 88;
}

static int32_t lab_cell_y(uint8_t row, uint8_t focused_row, bool focused) {
    if(row == 0) return LAB_GRID_TOP;
    if(row == 1) return focused_row == 0 ? 40 : 32;
    return focused ? LAB_GRID_BOTTOM - LAB_HOVER_HEIGHT + 1 :
                     LAB_GRID_BOTTOM - LAB_CELL_HEIGHT + 1;
}

static void lab_draw_grid(Canvas* canvas, const DanzeffLab* app) {
    const DzEditor* editor = &app->editor;
    const bool choosing = editor->phase == DzPhaseChoose;
    canvas_set_font(canvas, FontKeyboard);
    canvas_set_color(canvas, ColorBlack);

    for(uint8_t row = 0; row < 3; ++row) {
        for(uint8_t col = 0; col < 3; ++col) {
            const bool focused = row == editor->row && col == editor->col;
            const int32_t width = focused ?
                (col == 1 ? LAB_HOVER_CENTER_WIDTH : LAB_HOVER_EDGE_WIDTH) :
                LAB_CELL_WIDTH;
            const int32_t height = focused ? LAB_HOVER_HEIGHT : LAB_CELL_HEIGHT;
            const int32_t x = lab_cell_x(col, focused);
            const int32_t y = lab_cell_y(row, editor->row, focused);

            canvas_draw_frame(canvas, x, y, width, height);
            if(focused) {
                if(choosing) {
                    canvas_draw_frame(canvas, x + 1, y + 1, width - 2, height - 2);
                }
                // Keep the cross symmetrical relative to each focused cell.
                static const uint8_t baseline[] = {8, 13, 17, 13};
                for(uint8_t direction = 0; direction < 4; ++direction) {
                    const char ch = dz_editor_cell_char(editor, row, col, (DzDirection)direction);
                    if(!ch) continue;
                    const int32_t glyph_width = (int32_t)lab_key_width(canvas, ch);
                    int32_t char_x;
                    if(direction == DzDirectionLeft) char_x = x + 7;
                    else if(direction == DzDirectionRight) char_x = x + width - 7 - glyph_width;
                    else char_x = x + (width - glyph_width) / 2;
                    lab_draw_key(canvas, char_x, y + baseline[direction], ch);
                }
            } else {
                // All eight minimized cells remain the same 40x12 size,
                // with their inline labels centered and two lower pixels free.
                char characters[4];
                int32_t total_width = 0;
                for(uint8_t direction = 0; direction < 4; ++direction) {
                    characters[direction] = dz_editor_cell_char(editor, row, col, (DzDirection)direction);
                    total_width += (int32_t)lab_key_width(canvas, characters[direction]);
                }
                int32_t char_x = x + (width - total_width) / 2;
                for(uint8_t direction = 0; direction < 4; ++direction) {
                    lab_draw_key(canvas, char_x, y + 8, characters[direction]);
                    char_x += (int32_t)lab_key_width(canvas, characters[direction]);
                }
            }
        }
    }
}

static void lab_draw_results(Canvas* canvas, const DanzeffLab* app) {
    char line[48];
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Trial complete!");
    canvas_set_font(canvas, FontSecondary);
    snprintf(line, sizeof(line), "Time: %lu.%02lus  WPM: %lu.%lu",
             (unsigned long)(app->duration_ms / 1000),
             (unsigned long)(app->duration_ms % 1000 / 10),
             (unsigned long)(app->wpm_x10 / 10),
             (unsigned long)(app->wpm_x10 % 10));
    canvas_draw_str(canvas, 2, 20, line);
    snprintf(line, sizeof(line), "Button presses: %lu", (unsigned long)app->buttons_pressed);
    canvas_draw_str(canvas, 2, 30, line);
    snprintf(line, sizeof(line), "Backspaces: %lu", (unsigned long)app->backspaces);
    canvas_draw_str(canvas, 2, 40, line);
    snprintf(line, sizeof(line), "Wrong entries: %lu", (unsigned long)app->wrong_entries);
    canvas_draw_str(canvas, 2, 50, line);
    canvas_draw_str(canvas, 2, 61,
                    app->csv_saved ? "Saved. OK:retry Back:menu" : "Save failed! Back:menu");
}

static void lab_draw(Canvas* canvas, void* context) {
    DanzeffLab* app = context;
    if(furi_mutex_acquire(app->mutex, FuriWaitForever) != FuriStatusOk) return;
    canvas_clear(canvas);
    canvas_set_color(canvas, ColorBlack);
    switch(app->page) {
    case LabPageMenu: lab_draw_menu(canvas, app); break;
    case LabPageHelp: lab_draw_help(canvas); break;
    case LabPageEditor:
        lab_draw_preview(canvas, app);
        lab_draw_grid(canvas, app);
        break;
    case LabPageResults: lab_draw_results(canvas, app); break;
    }
    furi_mutex_release(app->mutex);
}

static void lab_input(InputEvent* event, void* context) {
    DanzeffLab* app = context;
    // GUI callbacks should never block on the application's message queue.
    furi_message_queue_put(app->queue, event, 0);
}

int32_t danzeff_lab_app(void* args) {
    UNUSED(args);
    DanzeffLab* app = malloc(sizeof(DanzeffLab));
    furi_check(app);
    memset(app, 0, sizeof(*app));
    app->page = LabPageMenu;
    app->trial_index = -1;
    app->running = true;
    app->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    app->queue = furi_message_queue_alloc(32, sizeof(InputEvent));
    furi_check(app->mutex && app->queue);
    app->gui = furi_record_open(RECORD_GUI);
    app->storage = furi_record_open(RECORD_STORAGE);
    app->viewport = view_port_alloc();
    view_port_draw_callback_set(app->viewport, lab_draw, app);
    view_port_input_callback_set(app->viewport, lab_input, app);
    gui_add_view_port(app->gui, app->viewport, GuiLayerFullscreen);

    InputEvent event;
    while(app->running) {
        if(furi_message_queue_get(app->queue, &event, FuriWaitForever) != FuriStatusOk) continue;
        furi_mutex_acquire(app->mutex, FuriWaitForever);
        lab_handle_input(app, &event);
        furi_mutex_release(app->mutex);
        view_port_update(app->viewport);
    }

    gui_remove_view_port(app->gui, app->viewport);
    view_port_free(app->viewport);
    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->queue);
    furi_mutex_free(app->mutex);
    free(app);
    return 0;
}
