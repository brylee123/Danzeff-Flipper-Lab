# Danzeff Lab — Flipper Zero proof-of-concept text editor

An **experimental** native Flipper Zero application to evaluate whether the
PSP-era Danzeff 3×3 keyboard is intuitive and fast with only the Flipper's six
buttons. The keyboard is **not** a firmware-wide text-input replacement.

## Status

- Implemented: single-line, append/backspace ASCII text editor (255 characters)
- Implemented: authentic Danzeff character table, including cases, digits and symbols
- Implemented: two-stage navigation/choice UI with fixed-size minimized cells and a 44x20 hover preview
- Implemented: three fixed, timed typing trials and CSV result capture
- Implemented: persistent free-edit draft on the SD card
- Verified: portable keyboard engine passes local C11 unit tests
- Verified by initial tester: v0.1 compiles with uFBT and runs on Flipper Zero.
- Verified by tester: v0.2 renders on-device; inconsistent row reflow motivates v0.3.
- v0.4 is not yet verified by uFBT/device testing; v0.3 has been used on a physical Flipper Zero.

## Controls

| Context | Input | Action |
|---|---|---|
| Main menu | Up / Down | Select a menu item |
| Main menu | OK | Open the selection |
| Main menu | Back | Exit app |
| Editor: grid | Short D-pad | Move between 3×3 cells |
| Editor: grid | Hold Left / Right | Previous / next character set (wraps) |
| Editor: grid | Focused cell | Enlarges to show the 4 directional characters |
| Editor: grid | OK short | Select the cell (double-thick border) |
| Editor: choosing | Up / Left / Down / Right | Enter the displayed character, reset focus to center |
| Editor: choosing | Back short | Cancel choice |
| Editor: grid | Back short | Delete the last character |
| Editor | OK long | Cycle lowercase → uppercase → digits → symbols → extra symbols |
| Editor | Back long | Return to menu; saves free-edit draft |
| Results | OK short | Retry the same phrase |
| Results | Back short | Return to menu |

The character positions mirror the original PSP implementation: Triangle =
**Up**, Square = **Left**, Cross = **Down**, Circle = **Right**. The central
lowercase cell is `[backspace, m, space, n]` in that order. In the cell grid,
`<` means backspace; **⎵** indicates space (drawn as a tiny custom bitmap-like symbol, while the buffer stores ASCII space). Blank entries in the numbers layer
have no function; they leave the four-way chooser open. A usable `0` has been
added to the numbers layer at center-cell Left. An extra symbol page (`EX+`)
regroups common ASCII punctuation and digits for quicker access. The original
four Danzeff pages otherwise retain their mappings.

### Example: type `a`

From the default center cell, press **Up**, **Left**, **OK**, **Left**.
That enters `a` and returns to the grid **with the middle cell highlighted**.
A hovered cell previews its four characters in a small directional cross.
The selected cell returns to **44×20 px**, as in v0.2, and all other
cells always remain **40×12 px** regardless of which cell is highlighted.
Only cell positions shift to fit the expanded cell, not compact cell sizes.
All characters are centered horizontally (including the expanded cross), and
the additional two pixels at the bottom prevent symbols touching the border. Press
OK to activate the double-thick border; press a direction to type. Non-hovered
cells always keep their four characters inline, so the full layout remains
visible. Canceling selection with Back retains the hovered cell. Choosing an
empty position in the number layer keeps the selection open.

## Character-set switching

In **grid mode**, hold Left for the previous set and Right for the next set:
`abc ↔ ABC ↔ 123 ↔ SYM ↔ EX+` (wraps at either end). Hold OK still
advances one set. The current set is displayed next to the entered-text
preview, even during a timed trial. While choosing a character, normal short
arrow presses still type the expected character; hold Left/Right only switches
sets in grid mode. Horizontal navigation intentionally does not auto-repeat
because a hold switches the page.

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
- A/B testing of v0.1 retained-selection navigation vs v0.2+ home-reset navigation
- Alternative layouts designed specifically for the Flipper D-pad
- In-device comparison against the Flipper's stock text entry mechanism
- Adaptive visual hints and practice indicators
- Cursor editing and richer text buffer, only after input method feasibility

## Attribution and license

Danzeff Lab acknowledges the authors and maintainers of
[Danzeff-G](https://github.com/libcg/Danzeff-G): **danzel**, **Jeff Chen**, and
**geecko (Clément G)**. Our foundational four character layouts have been
reviewed against the latest `master` source (2012-07-04,
`daf3c4cd04b264504781d758c38411419d48bca1`), not copied as a PSP library.
The BSD 3-Clause copyright and disclaimer are preserved in full at
[docs/licenses/DANZEFF-G-LICENSE.txt](docs/licenses/DANZEFF-G-LICENSE.txt),
with provenance and port-specific differences in
[docs/THIRD_PARTY.md](docs/THIRD_PARTY.md). Our own Flipper-only implementation
is BSD 2-Clause. Attribution is not endorsement by the original authors.
