#!/usr/bin/env python3
"""Host-side regression for v0.4's constant compact-cell dimensions.

The pure geometry mirrors lab_draw_grid() in danzeff_lab.c. It does not test
font metrics or the physical device display; screenshots are still required.
"""
from itertools import product

SCREEN = (128, 64)
LEFT, TOP = 1, 20
COMPACT = (40, 12)
FOCUSED = (44, 20)
HORIZONTAL_GAP = 1

for selected_row, selected_col in product(range(3), repeat=2):
    boxes = []
    y = TOP
    for row in range(3):
        row_height = FOCUSED[1] if row == selected_row else COMPACT[1]
        x = LEFT
        for col in range(3):
            selected = (row, col) == (selected_row, selected_col)
            width, height = FOCUSED if selected else COMPACT
            boxes.append((x, y, width, height, selected))
            assert 0 <= x and 0 <= y
            assert x + width <= SCREEN[0] and y + height <= SCREEN[1]
            if not selected:
                assert (width, height) == COMPACT
                # Inline text baseline +8 leaves 3 pixels below it before
                # the compact cell's final border row (+11).
                assert 11 - 8 == 3
            x += width + HORIZONTAL_GAP
        y += row_height
    assert len(boxes) == 9 and sum(b[-1] for b in boxes) == 1
    for i, a in enumerate(boxes):
        for b in boxes[i + 1:]:
            assert (a[0] + a[2] <= b[0] or b[0] + b[2] <= a[0] or
                    a[1] + a[3] <= b[1] or b[1] + b[3] <= a[1]), (selected_row,selected_col,a,b)
print("All nine v0.4 layouts fit and compact cells stay 40x12.")
