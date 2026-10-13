#include "../danzeff_core.h"
// Ensure host assertions remain active when NDEBUG is defined.
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_original_layout(void) {
    DzEditor e;
    dz_editor_init(&e);
    assert(e.row == 1 && e.col == 1);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionLeft) == 'a');
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionDown) == 'b');
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionRight) == 'c');
    assert(dz_editor_cell_char(&e, 1, 1, DzDirectionUp) == '\b');
    assert(dz_editor_cell_char(&e, 1, 1, DzDirectionDown) == ' ');
    // Current Danzeff-G master differs from older OSlib in this symbol.
    dz_editor_step_layer(&e, 1);
    dz_editor_step_layer(&e, 1);
    dz_editor_step_layer(&e, 1);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionUp) == ',');
    dz_editor_step_layer(&e, 1);
    dz_editor_step_layer(&e, 1);
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionLeft) == 'A');
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionRight) == '1');
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionLeft) == 0);
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 1, 2, DzDirectionLeft) == '\\');
    dz_editor_cycle_layer(&e);
    assert(e.layer == DzLayerExtraSymbols);
    dz_editor_cycle_layer(&e);
    assert(e.layer == DzLayerLower);
}

static void test_typing_and_cursor(void) {
    DzEditor e;
    dz_editor_init(&e);
    assert(dz_editor_move(&e, -1, -1));
    assert(e.col == 0 && e.row == 0);
    assert(!dz_editor_move(&e, -1, 0));
    dz_editor_select_cell(&e);
    assert(!dz_editor_move(&e, 1, 0));
    char ch = 0;
    assert(dz_editor_choose(&e, DzDirectionLeft, &ch));
    assert(ch == 'a' && strcmp(e.text, "a") == 0);
    assert(e.phase == DzPhaseGrid);
    assert(e.row == 1 && e.col == 1); // Home after typing 'a'.
    dz_editor_select_cell(&e);
    assert(dz_editor_choose(&e, DzDirectionDown, &ch));
    assert(ch == ' ' && strcmp(e.text, "a ") == 0);
    assert(e.row == 1 && e.col == 1); // Home even for space.
    dz_editor_select_cell(&e);
    assert(dz_editor_choose(&e, DzDirectionUp, &ch));
    assert(ch == '\b' && strcmp(e.text, "a") == 0);
    assert(e.row == 1 && e.col == 1); // Home after a chosen deletion.
    assert(dz_editor_backspace(&e));
    assert(!dz_editor_backspace(&e));
}

static void test_digits_empty_selection(void) {
    DzEditor e;
    dz_editor_init(&e);
    dz_editor_cycle_layer(&e);
    dz_editor_cycle_layer(&e);
    assert(dz_editor_move(&e, -1, -1));
    dz_editor_select_cell(&e);
    char ch = '\xff';
    assert(!dz_editor_choose(&e, DzDirectionLeft, &ch));
    assert(ch == '\0' && e.phase == DzPhaseChoose);
    assert(e.row == 0 && e.col == 0); // Invalid choice keeps its cell.
    assert(dz_editor_choose(&e, DzDirectionRight, &ch));
    assert(ch == '1' && strcmp(e.text, "1") == 0);
    assert(e.row == 1 && e.col == 1);
}


static void test_recenter_and_cancel(void) {
    DzEditor e;
    dz_editor_init(&e);
    assert(dz_editor_move(&e, 1, -1));
    assert(e.row == 0 && e.col == 2);
    dz_editor_select_cell(&e);
    dz_editor_cancel_choice(&e);
    assert(e.phase == DzPhaseGrid && e.row == 0 && e.col == 2);
    dz_editor_select_cell(&e);
    char ch = 0;
    assert(dz_editor_choose(&e, DzDirectionRight, &ch));
    assert(ch == 'i' && strcmp(e.text, "i") == 0);
    assert(e.phase == DzPhaseGrid && e.row == 1 && e.col == 1);

    assert(dz_editor_move(&e, -1, 1));
    dz_editor_select_cell(&e);
    assert(dz_editor_choose(&e, DzDirectionUp, &ch));
    assert(ch == '(' && strcmp(e.text, "i(") == 0);
    assert(e.row == 1 && e.col == 1);
}


static void test_layers_and_extra_symbols(void) {
    DzEditor e;
    dz_editor_init(&e);
    // Long-left wraps from lowercase to the new extra-symbol page.
    dz_editor_step_layer(&e, -1);
    assert(e.layer == DzLayerExtraSymbols);
    assert(strcmp(dz_layer_name(e.layer), "EX+") == 0);
    assert(dz_editor_cell_char(&e, 1, 1, DzDirectionLeft) == '0');
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionUp) == '@');
    dz_editor_select_cell(&e);
    char ch = 0;
    assert(dz_editor_choose(&e, DzDirectionLeft, &ch));
    assert(ch == '0' && strcmp(e.text, "0") == 0);
    assert(e.phase == DzPhaseGrid && e.row == 1 && e.col == 1);
    // Long-right wraps back to lowercase. Neighboring modes work both ways.
    dz_editor_step_layer(&e, 1);
    assert(e.layer == DzLayerLower);
    dz_editor_step_layer(&e, 1);
    assert(e.layer == DzLayerUpper);
    dz_editor_step_layer(&e, -1);
    assert(e.layer == DzLayerLower);
    dz_editor_step_layer(&e, 1);
    dz_editor_step_layer(&e, 1);
    assert(e.layer == DzLayerNumbers);
    // Number-pad includes 0, which the original Danzeff layout lacked.
    assert(dz_editor_cell_char(&e, 1, 1, DzDirectionLeft) == '0');
    dz_editor_select_cell(&e);
    dz_editor_step_layer(&e, -1);
    assert(e.layer == DzLayerUpper && e.phase == DzPhaseGrid);
}

static void test_capacity(void) {
    DzEditor e;
    dz_editor_init(&e);
    for(size_t i = 0; i < DZ_TEXT_CAPACITY - 1; ++i) assert(dz_editor_insert(&e, 'x'));
    assert(!dz_editor_insert(&e, 'y'));
    assert(e.length == DZ_TEXT_CAPACITY - 1);
    assert(e.text[e.length] == '\0');
}

int main(void) {
    test_original_layout();
    test_typing_and_cursor();
    test_digits_empty_selection();
    test_recenter_and_cancel();
    test_layers_and_extra_symbols();
    test_capacity();
    puts("All Danzeff core tests passed.");
    return 0;
}
