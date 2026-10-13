# Third-party attribution and license compliance

**Danzeff-G** — https://github.com/libcg/Danzeff-G

Danzeff Lab uses the Danzeff 3x3 character-group concept and four-way
selection mapping and has compared these mappings against the latest commit
on Danzeff-G's `master` branch: [`daf3c4cd04b264504781d758c38411419d48bca1`](https://github.com/libcg/Danzeff-G/commit/daf3c4cd04b264504781d758c38411419d48bca1),
dated **2012-07-04**. That is the latest commit *in that repository*, not a
claim that no other Danzeff-derived forks or implementations are newer.

## Original author credits

As named in Danzeff-G's LICENSE:
- **danzel** — copyright (c) 2006
- **Jeff Chen** — copyright (c) 2006
- **geecko** (Clément G) — copyright (c) 2012; Danzeff-G rewrite

Original BSD 3-Clause license, copyright notices and disclaimer are reproduced
**verbatim** at [docs/licenses/DANZEFF-G-LICENSE.txt](licenses/DANZEFF-G-LICENSE.txt).
The upstream license additionally prohibits endorsement using the authors'
names without prior written permission. Attribution does not imply endorsement.

## Port scope and source-lineage distinction

Danzeff-G is a PSP-specific rewrite. We do **not** bundle its PSP rendering
code, glib2d dependencies or images; the Flipper application implements its
own drawing, input-event handling, and a fifth convenience symbol page.
The original four character mappings now match Danzeff-G's latest source,
except that our number page adds `0` in the center-left direction where
Danzeff-G leaves that slot empty. That position is intentional and documented.

The project was initially prototyped against the DanzeffOSLib archival port:
https://github.com/PSP-Archive/DanzeffOSLib (which credits **Sakya** for the
OSlib port). One symbol differs between these two sources: the upper-left
symbol cell's Up action is a comma in Danzeff-G and an apostrophe in the
OSlib port. Danzeff Lab v0.4 follows **Danzeff-G** (comma).

This repository's original Flipper-specific code is released under its own
BSD 2-Clause LICENSE. Both applicable license notices must be retained when
distributing source or binary derivatives as required by their terms.
