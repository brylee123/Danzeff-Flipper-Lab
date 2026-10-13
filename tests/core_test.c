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
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionLeft) == 'A');
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionRight) == '1');
    assert(dz_editor_cell_char(&e, 0, 0, DzDirectionLeft) == 0);
    dz_editor_cycle_layer(&e);
    assert(dz_editor_cell_char(&e, 1, 2, DzDirectionLeft) == '\\');
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
    assert(dz_editor_move(&e, 1, 1));
    dz_editor_select_cell(&e);
    assert(dz_editor_choose(&e, DzDirectionDown, &ch));
    assert(ch == ' ' && strcmp(e.text, "a ") == 0);
    dz_editor_select_cell(&e);
    assert(dz_editor_choose(&e, DzDirectionUp, &ch));
    assert(ch == '\b' && strcmp(e.text, "a") == 0);
    assert(dz_editor_backspace(&e));
    assert(!dz_editor_backspace(&e));
}

static void test_digits_empty_selection(void) {
    DzEditor e;
    dz_editor_init(&e);
    dz_editor_cycle_layer(&e);
    dz_editor_cycle_layer(&e);
    dz_editor_select_cell(&e);
    char ch = '\xff';
    assert(!dz_editor_choose(&e, DzDirectionLeft, &ch));
    assert(ch == '\0' && e.phase == DzPhaseChoose);
    assert(dz_editor_choose(&e, DzDirectionRight, &ch));
    assert(ch == '5' && strcmp(e.text, "5") == 0);
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
    test_capacity();
    puts("All Danzeff core tests passed.");
    return 0;
}
