# Danzeff Lab: usability and speed experiment (v0.1)

## Research questions

1. Can a new user discover the two-stage grid -> direction interaction with minimal instruction?
2. How fast do novices vs. experienced users type after 1, 3, and 10 minutes?
3. Is the Danzeff approach faster than the stock Flipper text-input keyboard for the same phrases?
4. Which errors come from cell navigation vs. directional selection?

## Participants

Aim for at least 5 novice participants and 5 experienced Flipper owners for exploratory testing.
This is *qualitative feasibility testing*, not a statistically powered superiority study.
Do not collect names; assign anonymous IDs. Ask participants for informed consent.

## Trial design

- Version: record app commit, firmware branch/version, and Flipper model.
- Rotate whether stock keyboard or Danzeff is tested first across participants.
- Each participant uses the same three exact phrases in each interface:
  - `hello world`
  - `flipper zero`
  - `the quick brown fox`
- Phase A: no instruction; observe 30 seconds of exploration, then offer the Help screen.
- Phase B: three timed phrases with Danzeff.
- Phase C: same phrases in stock keyboard; manually time from first physical press until correct completion.
- Phase D: repeat after 3 minutes of free practice and again after 10 minutes.
- Do not include phrase-reading or setup latency in measured time for either keyboard.

## Five-button typing policy (v0.7 onward)

Text entry and correction use the D-pad and OK only. **Do not use hardware Back
to delete.** To correct text, choose the center cell then press Up for its
on-screen backspace character. A short Back cancels a pending choice or
returns to the menu from grid mode; holding Back also returns to the menu.
If a participant exits a trial by mistake, discard that attempt and restart.
This change affects comparability with v0.1–v0.6 trials; always record the
application version with each result.

## Measures

Automatic: elapsed milliseconds, gross WPM (5 characters per standard word),
physical button presses, effective backspace actions, character entries that did
not match the target position at time of entry, total character insertions.
Trial ends only when the editor's text exactly matches the target.

Manual: stock-keyboard time, task failures, time-to-discover controls, perceived
mental workload (1-5), ease of learning (1-5), preference, observer notes.

Cautions: `wrong_entries` measures incorrect entries at the current position,
not final edit distance or a formal corrected-error rate. `button_presses`
counts InputTypePress events (including a long press once), not repeat events.
Elapsed time begins at the first InputTypePress in the editor, not when the
prompt appears. A participant who walks away mid-task can return and invalidate
the time; restart that task in such cases.

## Recommended comparison

Use medians, median WPM, and median button-presses-per-character, separated by
novice/experienced user and practice exposure. Report failure counts and
subjective ratings alongside speed. Avoid cherry-picking each participant's
fastest run or declaring performance superiority based on a few participants.

## Data locations

On the Flipper SD card (application data):

- `/ext/apps_data/danzeff_lab/draft.txt`
- `/ext/apps_data/danzeff_lab/results.csv`

Application paths use Flipper's `APP_DATA_PATH` alias; on most installations this
resolves to the external app data folder shown above.

CSV column `wpm_x10` is WPM multiplied by ten (e.g., 182 means 18.2 WPM).
The app currently does not log unique participant IDs or timestamps. Rename or
copy each tester's CSV file between participants or add a session ID before a
formal study. Deleting the CSV safely resets recorded trials.

## Acceptance gates for progressing to system-wide integration

- Every study participant can complete all three trial phrases after brief help.
- No data loss, crashes, or missed-character issues across 30 consecutive runs.
- The majority can explain navigation/choice/layer switching unaided after practice.
- Measured speed, corrections, and preference justify further prototyping.

These are provisional criteria, not findings; collect actual user data first.
