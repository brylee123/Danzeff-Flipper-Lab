#!/usr/bin/env python3
"""Guard the five-button typing policy: hardware Back must never delete text.

Source-level regression until firmware-level input tests can exercise the
ViewPort handler. The Danzeff center-cell on-screen backspace remains intact.
"""
from pathlib import Path

src = (Path(__file__).resolve().parents[1] / "danzeff_lab.c").read_text()
core = (Path(__file__).resolve().parents[1] / "danzeff_core.c").read_text()
handler = src.split("static void lab_handle_editor(", 1)[1].split(
    "static void lab_handle_input(", 1
)[0]

assert "dz_editor_backspace" not in handler, "Hardware editor handler must not delete directly"
assert "lab_back_to_menu(app)" in handler, "Back needs normal navigation"
assert "case InputKeyBack:" in handler, "Hardware Back must be handled"
assert "dz_editor_cancel_choice(&app->editor)" in handler, "Back should cancel chooser"
assert "if(input->type == InputTypeShort) lab_back_to_menu(app);" in handler
assert "lab_insert_choice(app, DzDirectionUp)" in handler

# On-screen Up in the center cell still triggers the core backspace.
assert r"{'\b', 'm', ' ', 'n'}" in core
assert "ch == '\\b'" in core and "dz_editor_backspace(editor)" in core

print("Five-button entry policy verified; on-screen backspace retained.")
