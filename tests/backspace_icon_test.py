#!/usr/bin/env python3
"""Verify Danzeff Lab draws ⌫ for on-screen backspace without changing input semantics."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
screen = (root / "danzeff_lab.c").read_text()
core = (root / "danzeff_core.c").read_text()

assert "if(character == '\\b') return 9;" in screen
assert "if(character == '\\b') {" in screen
assert "canvas_draw_line(canvas, x + 3, baseline - 6, x + 8, baseline - 6);" in screen
assert "canvas_draw_line(canvas, x + 4, baseline - 4, x + 6, baseline - 2);" in screen
assert "if(c == '\\b') return '<';" not in screen
assert "ch == '\\b'" in core and "dz_editor_backspace(editor)" in core
print("Custom backspace icon and on-screen delete behavior verified.")
