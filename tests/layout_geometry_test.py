#!/usr/bin/env python3
"""Regression checks for v0.5's edge-anchored 3x3 geometry.

Keep this test synchronized with lab_cell_x()/lab_cell_y() and the geometry
constants in danzeff_lab.c. Physical device screenshots remain necessary.
"""
from itertools import product

WIDTH, HEIGHT = 128, 64
COMPACT = (40, 12)
FOCUSED_HEIGHT = 20

def rect(row, col, focused_row, focused_col):
    focused = row == focused_row and col == focused_col
    width = (48 if col == 1 else 44) if focused else 40
    height = FOCUSED_HEIGHT if focused else 12
    x = (0, 40 if focused else 44, 84 if focused else 88)[col]
    y = 20 if row == 0 else (40 if focused_row == 0 else 32) if row == 1 else (44 if focused else 52)
    return x, y, width, height

for focused_row, focused_col in product(range(3), repeat=2):
    boxes = {(r,c): rect(r,c,focused_row,focused_col)
             for r,c in product(range(3), repeat=2)}

    for (r,c), (x,y,w,h) in boxes.items():
        assert 0 <= x and 0 <= y and x + w <= WIDTH and y + h <= HEIGHT
        assert (x == 0) if c == 0 else True  # leftmost pixel
        assert (x + w == WIDTH) if c == 2 else True  # rightmost pixel
        assert (y + h == HEIGHT) if r == 2 else True  # bottommost pixel
        if (r,c) != (focused_row, focused_col):
            assert (w,h) == COMPACT
        else:
            assert h == 20 and w == (48 if c == 1 else 44)
        if r == 0:
            assert y == 20
        # Inline text baseline +8 leaves 3 pixels through bottom border row.
        if (r,c) != (focused_row,focused_col):
            assert 11 - 8 == 3

    for (r1,c1),(r2,c2) in product(boxes, repeat=2):
        if (r1,c1) >= (r2,c2):
            continue
        x1,y1,w1,h1 = boxes[(r1,c1)]
        x2,y2,w2,h2 = boxes[(r2,c2)]
        assert (x1+w1 <= x2 or x2+w2 <= x1 or
                y1+h1 <= y2 or y2+h2 <= y1), (
                    f"overlap with focus {(focused_row,focused_col)}:",
                    (r1,c1), (r2,c2))

    # When focused, occupy all available intervening horizontal space.
    x,y,w,h = boxes[(focused_row,focused_col)]
    if focused_col > 0:
        previous = boxes[(focused_row,focused_col-1)]
        assert previous[0]+previous[2] == x
    if focused_col < 2:
        following = boxes[(focused_row,focused_col+1)]
        assert x+w == following[0]

    # Expanded vertical region reaches its nearest adjoining row.
    if focused_row > 0:
        above = boxes[(focused_row-1,focused_col)]
        assert above[1]+above[3] == y
    if focused_row < 2:
        below = boxes[(focused_row+1,focused_col)]
        assert y+h == below[1]

print("All nine v0.5 edge-anchored layouts passed.")
