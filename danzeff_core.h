#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DZ_TEXT_CAPACITY 256

typedef enum {
    DzLayerLower,
    DzLayerUpper,
    DzLayerNumbers,
    DzLayerSymbols,
    DzLayerExtraSymbols,
    DzLayerCount,
} DzLayer;

typedef enum {
    DzPhaseGrid,
    DzPhaseChoose,
} DzPhase;

// Order follows the original Danzeff PSP face buttons:
// Triangle (up), Square (left), Cross (down), Circle (right).
typedef enum {
    DzDirectionUp,
    DzDirectionLeft,
    DzDirectionDown,
    DzDirectionRight,
} DzDirection;

typedef struct {
    char text[DZ_TEXT_CAPACITY];
    size_t length;
    uint8_t row;
    uint8_t col;
    DzLayer layer;
    DzPhase phase;
} DzEditor;

void dz_editor_init(DzEditor* editor);
bool dz_editor_insert(DzEditor* editor, char ch);
bool dz_editor_backspace(DzEditor* editor);
bool dz_editor_move(DzEditor* editor, int8_t dx, int8_t dy);
void dz_editor_cycle_layer(DzEditor* editor);
// 1 = next, -1 = previous; wraps around and returns to cell navigation.
void dz_editor_step_layer(DzEditor* editor, int8_t step);
void dz_editor_select_cell(DzEditor* editor);
void dz_editor_cancel_choice(DzEditor* editor);
// Returns the selected character, including '\b' for delete and '\0' for an empty choice.
char dz_editor_cell_char(const DzEditor* editor, uint8_t row, uint8_t col, DzDirection direction);
// Returns true for an inserted character or effective backspace.
bool dz_editor_choose(DzEditor* editor, DzDirection direction, char* selected);
const char* dz_layer_name(DzLayer layer);
