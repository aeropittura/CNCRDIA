# CNCRDIA overhaul plan

Goal: restructure and rewrite the MP-SPDZ-derived code base so that it is
organised, named and documented differently and better, **without changing
behaviour**. Copyright notices, `License.txt` and upstream credit stay; the
provenance docs are updated to say this is a modified derivative.

Work happens on the `overhaul` branch, one reviewable commit per step. Every
step must pass the verification gate (below) before it is committed.

## Invariants (never broken)

| Contract | Why | Rule |
|---|---|---|
| Bytecode opcodes and argument layouts (`instructions*.py` ↔ VM) | Compiler and VM must agree | Values and order frozen; names may change |
| Compiler output for every example program | Strongest refactor check | Byte-identical `.bc`/`.sch` for non-behavioural commits |
| On-disk formats: `Player-Data/`, preprocessing headers, `type_string()` values, `GlobalParams-*` names | Existing data and parties stay compatible | String literals frozen |
| Runtime paths `Programs/{Bytecode,Schedules,Functions,Public-Input}` | Hardcoded in the VM and in user workflows | Kept |
| Party binary names (`*-party.x` etc.) and CLI flags | Used by scripts, docs and users | Kept |
| DSL surface (`sint`, `sfix`, `for_range`, `from Compiler import ml` …) | Every `.mpc` program depends on it | Kept, via a compatibility shim if the package moves |
| Network wire format | Mixed-version parties | Unchanged |

## macOS/Linux gotchas

- The default APFS volume is case-insensitive: never do case-only renames
  (`Compiler` → `compiler`, `Share.h` → `share.h`) as a single step, and never
  let two paths differ only by case. New names are chosen to be distinct.
- macOS resolves `#include` paths case-insensitively, so a wrong-case include
  builds here and breaks on Linux. A checker script verifies every quoted
  include matches the real path exactly; GitHub CI (Linux) runs on the PR.
- `tools/` would clash with the old `Tools/` on case-insensitive volumes
  when switching between old and new commits, so it is not used as a name.
- Basenames repeat across directories (`SemiPrep`, `Rep4Prep`, `Secret`,
  `Instruction`, `Program`, `Machine` …), so moves keep them in distinct
  directories and all renames are done by scripts with a mapping table, not by
  hand.

## Phase 0: safety net (done)

Baseline recorded on macOS arm64, clang 17, Python 3.14: 100 executables,
164 compiled programs plus 206 reproducible compile errors (missing
arguments or optional packages), 94 successful tutorial runs.  Programs
that download data sets or models at compile time (torch, BERT) are not
covered.

1. Install dependencies, initialise submodules, `make setup`.
2. Build the reference binary set (the CI list plus emulate, BMR, ECDSA and the
   offline binaries).
3. `tests/baseline/`: compile every program in `Programs/Source` (with the
   flag sets they need: `-R 64`, `-F 64`, `-B`, `-Y`, …) and store SHA-256
   hashes of every `.bc`/`.sch`.
4. Record outputs of `Scripts/test_tutorial.sh` across protocols,
   `test_ecdsa.sh`, `test_streaming.sh` and `emulate.x` runs.
5. `tests/check.sh`: one command that rebuilds, recompiles, diffs hashes and
   reruns the suite. This is the gate for every later commit.

## Phase 1: new layout (done)

Implemented as planned, with these deviations: `Scripts/` became `launch/`
(kept as a symlink), dev scripts live in `devtools/`, and `doc/` became
`docs/`. See ARCHITECTURE.md for the result.

```
engine/                 C++ runtime (was the top-level dirs)
  support/              ← Tools + Utils (shared helpers, buffers, timers, crypto utils)
  numeric/              ← Math (fields, rings, bigint, gf2n)
  net/                  ← Networking
  vm/                   ← Processor (machine, threads, memory, instructions)
  boolean/              ← GC (binary-circuit VM)
  garbled/
    yao/                ← Yao
    bmr/                ← BMR (BMR/network → bmr/transport, so it cannot
                          shadow engine/net in include lookups)
  he/
    core/               ← FHE
    offline/            ← FHEOffline
  ot/                   ← OT
  protocols/            ← Protocols, split by family
    common/             shares, MAC check base, buffers, dabit/edabit, shuffles
    replicated/         rep3 (semi-honest, malicious, post-sacrifice, PS)
    rep4/
    shamir/
    spdz/               mascot, lowgear/highgear, cowgear/chaigear
    spdz2k/
    spdzwise/
    semi/               semi, semi2k, hemi, temi, soho
    dealer/
    astra_trio/
    atlas/
    brain/
    fake/
  ecdsa/                ← ECDSA
  extio/                ← ExternalIO
parties/                ← Machines, grouped: arith/, binary/, offline/, export/, bmr/
mpcl/                   ← Compiler (Python package, new name)
  core/                 types, instructions, program, allocator, config
  lib/                  library, comparison, floating point, math, sorting
  oram/                 oram, path_oram, circuit_oram, sqrt_oram, heaps
  apps/                 ml, decision_tree, dijkstra, graph, sparse …
  boolean/              ← Compiler/GC
Compiler/               thin back-compat shim mapping `Compiler.*` → `mpcl.*`
examples/               ← Programs/Source (compile.py also searches the old path)
Programs/               runtime output dirs only (Bytecode, Schedules, …)
launch/                 ← Scripts (protocol launchers and helpers; `Scripts` stays as a symlink)
devtools/               maintenance scripts (including the restructuring tools)
tests/                  new regression harness + baseline hashes
third_party/            ← deps (submodules)
build/                  Makefile split into fragments (config, objects, parties, deps)
docs/                   ← doc
```

Done by a script that runs `git mv` (keeps history), rewrites every quoted
`#include` from a path map, and updates the Makefile, `setup.py`,
`.gitmodules`, CI, the Dockerfile and docs. One commit per top-level move,
each through the gate.

## Phase 2: renaming (done)

- C++ file names in snake_case, `#pragma once`, file banners naming the
  new path; Python module names (`mpcl.*`).
- C++ identifiers: `devtools/rename/cxx_collect.py` scans every
  translation unit with libclang, `cxx_plan.py` derives new names (foreign
  API names, macros, `##`-pasted prefixes and invariant spellings frozen;
  collisions rejected) and `cxx_apply.py` rewrites code tokens, code-like
  names in comments and code spans in the docs. Table:
  `devtools/rename/cxx-names.tsv`.
- Python locals: `devtools/rename/py_locals.py` (LibCST, scope-aware) spells
  out abbreviated function locals; parameters, module-level names, nested
  defs and functions using `locals()`/`exec` are left alone. Table:
  `devtools/rename/py-locals.tsv`.
- Shell launchers: one `launch/party.sh` driven by `launch/protocols.tsv`;
  every old script name is a link to it.

## Phase 3: improvements

Done:

- `-Wall -Wextra -Werror` builds (already the default flags).
- OpenSSL 4 support for TLS; certificates with subjectAltName.
- Deterministic schedule files (sorted options).
- A README per protocol family in `engine/protocols/`.
- Build: `make check`, `make compdb`, `make clean-objects`.

Not done (each needs a full run of the gate below to be safe):

- Splitting `types.py`, `library.py`, `instructions.py` and `ml.py` into
  submodules. `driver.py` exposes module-level names to programs, so a split
  has to re-export every name and keep the `Compiler.*` aliases.
- Type hints, docstrings, dead code and Python 2 leftovers.
- `const`/RAII/`override` clean-ups and macro-to-template rewrites.
- Protocol coverage for families CI does not exercise.

Security-critical code (MAC checks, ZK proofs, OT checks, key handling)
gets renames and comments only, no logic rewrites.

## Verification gate (every commit)

1. Build the reference binaries with no new warnings.
2. Recompile all examples; hashes match the baseline (or the difference is
   explained in the commit message for intended changes).
3. Run the regression suite.
4. Include-case checker and a "no personal data" scan of the diff.
5. Linux check via Docker locally, and GitHub CI on the PR.

## Git hygiene

- Commits as `aeropittura <101230990+aeropittura@users.noreply.github.com>`
  (repo-local config); every commit's author/committer is checked before push.
- Tokens are never written to the remote URL, git config or any file in the
  repo; pushes use a one-shot credential.
