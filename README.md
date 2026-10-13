# Danzeff Lab — Flipper Zero proof-of-concept text editor

An **experimental** native Flipper Zero application to evaluate whether the
PSP-era Danzeff 3×3 keyboard is intuitive and fast with only the Flipper's six
buttons. The keyboard is **not** a firmware-wide text-input replacement.

## Status

- Implemented: single-line, append/backspace ASCII text editor (255 characters)
- Implemented: authentic Danzeff character table, including cases, digits and symbols
- Implemented: two-stage navigation/choice UI with inline cells and directional hover
- Implemented: three fixed, timed typing trials and CSV result capture
- Implemented: persistent free-edit draft on the SD card
- Verified: portable keyboard engine passes local C11 unit tests
- Verified by initial tester: v0.1 compiles with uFBT and runs on Flipper Zero.
- Not yet verified on-device: v0.2 expanded hover and automatic home reset.

## Controls

| Context | Input | Action |
|---|---|---|
| Main menu | Up / Down | Select a menu item |
| Main menu | OK | Open the selection |
| Main menu | Back | Exit app |
| Editor: grid | D-pad | Move between 3×3 cells |
| Editor: grid | Focused cell | Enlarges to show the 4 directional characters |
| Editor: grid | OK short | Select the cell (double-thick border) |
| Editor: choosing | Up / Left / Down / Right | Enter the displayed character, reset focus to center |
| Editor: choosing | Back short | Cancel choice |
| Editor: grid | Back short | Delete the last character |
| Editor | OK long | Cycle lowercase → uppercase → digits → symbols |
| Editor | Back long | Return to menu; saves free-edit draft |
| Results | OK short | Retry the same phrase |
| Results | Back short | Return to menu |

The character positions mirror the original PSP implementation: Triangle =
**Up**, Square = **Left**, Cross = **Down**, Circle = **Right**. The central
lowercase cell is `[backspace, m, space, n]` in that order. In the cell grid,
`<` means backspace and `_` means space. Blank entries in the numbers layer
have no function; they leave the four-way chooser open.

### Example: type `a`

From the default center cell, press **Up**, **Left**, **OK**, **Left**.
That enters `a` and returns to the grid **with the middle cell highlighted**.
A hovered cell previews its four characters in a small directional cross. Press
OK to activate the double-thick border; press a direction to type. Non-hovered
cells always keep their four characters inline, so the full layout remains
visible. Canceling selection with Back retains the hovered cell. Choosing an
empty position in the number layer keeps the selection open.

## Build and install

This directory is a complete standalone uFBT application project; it does not
need to be placed inside a full firmware checkout.

Install the official [uFBT tool](https://github.com/flipperdevices/flipperzero-ufbt)
(on Windows, use PowerShell):

```powershell
py -m pip install --upgrade ufbt
```

From inside the extracted project directory:

```powershell
ufbt
ufbt launch
```

`ufbt` creates a `.fap` under `dist/`. `ufbt launch` deploys it to a connected
Flipper Zero via USB. The first run downloads the toolchain and official SDK.
For reliability, match the SDK channel with the firmware installed on your
Flipper. If compiling with uFBT locally is inconvenient, the included GitHub
workflow publishes a downloadable `.fap` build artifact after each push.

To test the portable C core independently on macOS/Linux:

```bash
./tests/run.sh
```

## Real-world feasibility testing

Use the built-in trial modes and the reproducible protocol in
[docs/TEST_PROTOCOL.md](docs/TEST_PROTOCOL.md). The app records the following:

- `duration_ms`: from first physical button press to exact correct completion
- `wpm_x10`: gross five-character-standard words per minute, multiplied by 10
- `button_presses`: physical presses, including presses that initiate long actions
- `backspaces`: effective deletions
- `wrong_entries`: inserted characters that mismatched the target at insertion time
- `insertions`: total accepted character insertions, including later-corrected errors

Trial results are appended to `results.csv`; the free-edit draft is stored in
`draft.txt`. Both live in the Flipper app data directory, normally
`/ext/apps_data/danzeff_lab/` on the microSD card. The editor stores up to
255 ASCII bytes and supports append/backspace only, not cursor insertion or
multiline editing. The app is purpose-built to benchmark keyboard ergonomics,
not to serve as a full-featured notepad.

## Future experiments

- Two-button-chord or long-press shortcuts to skip the cell-open step
- A/B testing of v0.1 retained-selection navigation vs v0.2 home-reset navigation
- Alternative layouts designed specifically for the Flipper D-pad
- In-device comparison against the Flipper's stock text entry mechanism
- Adaptive visual hints and practice indicators
- Cursor editing and richer text buffer, only after input method feasibility

## Attribution and license

This proof of concept reuses the Danzeff **character layout and selection
semantics**, but not its PSP graphics or rendering code. Attribution is in
[docs/THIRD_PARTY.md](docs/THIRD_PARTY.md). This project's original code is
BSD-2-Clause; review the upstream license prior to broader distribution.
