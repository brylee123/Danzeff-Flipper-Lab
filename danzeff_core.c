#include "danzeff_core.h"

#include <string.h>

// Four chars per cell: up, left, down, right. Port of the character
// mapping in PSP-Archive/DanzeffOSLib/danzeff.c (BSD-licensed upstream).
static const char dz_layout[DzLayerCount][3][3][4] = {
    { // lowercase
        {{',', 'a', 'b', 'c'}, {'.', 'd', 'e', 'f'}, {'!', 'g', 'h', 'i'}},
        {{'-', 'j', 'k', 'l'}, {'\b', 'm', ' ', 'n'}, {'?', 'o', 'p', 'q'}},
        {{'(', 'r', 's', 't'}, {':', 'u', 'v', 'w'}, {')', 'x', 'y', 'z'}},
    },
    { // uppercase
        {{'^', 'A', 'B', 'C'}, {'@', 'D', 'E', 'F'}, {'*', 'G', 'H', 'I'}},
        {{'_', 'J', 'K', 'L'}, {'\b', 'M', ' ', 'N'}, {'"', 'O', 'P', 'Q'}},
        {{'=', 'R', 'S', 'T'}, {';', 'U', 'V', 'W'}, {'/', 'X', 'Y', 'Z'}},
    },
    { // digits: original mapping only has a rightward choice per digit
        {{0, 0, 0, '1'}, {0, 0, 0, '2'}, {0, 0, 0, '3'}},
        {{0, 0, 0, '4'}, {'\b', 0, ' ', '5'}, {0, 0, 0, '6'}},
        {{0, 0, 0, '7'}, {0, 0, 0, '8'}, {0, 0, 0, '9'}},
    },
    { // symbols
        {{'\'', '(', '.', ')'}, {'"', '<', '\'', '>'}, {'-', '[', '_', ']'}},
        {{'!', '{', '?', '}'}, {'\b', 0, ' ', 0}, {'+', '\\', '=', '/'}},
        {{':', '@', ';', '#'}, {'~', '$', '`', '%'}, {'*', '^', '|', '&'}},
    },
};

void dz_editor_init(DzEditor* editor) {
    memset(editor, 0, sizeof(*editor));
    editor->row = 1;
    editor->col = 1;
}

bool dz_editor_insert(DzEditor* editor, char ch) {
    if(!ch || editor->length >= DZ_TEXT_CAPACITY - 1) return false;
    editor->text[editor->length++] = ch;
    editor->text[editor->length] = '\0';
    return true;
}

bool dz_editor_backspace(DzEditor* editor) {
    if(!editor->length) return false;
    editor->text[--editor->length] = '\0';
    return true;
}

bool dz_editor_move(DzEditor* editor, int8_t dx, int8_t dy) {
    if(editor->phase != DzPhaseGrid) return false;
    int next_col = (int)editor->col + dx;
    int next_row = (int)editor->row + dy;
    if(next_col < 0 || next_col > 2 || next_row < 0 || next_row > 2) return false;
    editor->col = (uint8_t)next_col;
    editor->row = (uint8_t)next_row;
    return true;
}

void dz_editor_cycle_layer(DzEditor* editor) {
    editor->layer = (DzLayer)((editor->layer + 1) % DzLayerCount);
    editor->phase = DzPhaseGrid;
}

void dz_editor_select_cell(DzEditor* editor) {
    editor->phase = DzPhaseChoose;
}

void dz_editor_cancel_choice(DzEditor* editor) {
    editor->phase = DzPhaseGrid;
}

char dz_editor_cell_char(
    const DzEditor* editor, uint8_t row, uint8_t col, DzDirection direction) {
    if(row > 2 || col > 2 || direction > DzDirectionRight) return 0;
    return dz_layout[editor->layer][row][col][direction];
}

bool dz_editor_choose(DzEditor* editor, DzDirection direction, char* selected) {
    if(editor->phase != DzPhaseChoose) return false;
    char ch = dz_editor_cell_char(editor, editor->row, editor->col, direction);
    if(selected) *selected = ch;
    if(!ch) return false; // Keep choice open so another direction can be pressed.
    editor->phase = DzPhaseGrid;
    bool changed = (ch == '\b') ? dz_editor_backspace(editor) : dz_editor_insert(editor, ch);
    // Recenter after every successful entry, including a chosen backspace.
    // An empty choice retains the cell and remains in choice mode (above).
    if(changed) {
        editor->row = 1;
        editor->col = 1;
    }
    return changed;
}

const char* dz_layer_name(DzLayer layer) {
    static const char* const names[] = {"abc", "ABC", "123", "#$%"};
    if(layer >= DzLayerCount) return "?";
    return names[layer];
}
