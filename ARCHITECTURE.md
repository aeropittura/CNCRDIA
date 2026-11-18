# Code layout

CNCRDIA is organised by responsibility. The C++ runtime is under
`engine/`, the executables are under `parties/`, and the
Python compiler is the `mpcl` package.

```
engine/                 C++ runtime, included as "<area>/<file>.h"
  support/              buffers, octet streams, hashing, randomness, timers, options
  numeric/              prime fields, rings Z/2^k, GF(2^n), big integers
  net/                  players, sockets, TLS, message exchange
  vm/                   the virtual machine: machine, threads, memory,
                        instruction decoding, preprocessing files
  boolean/              the virtual machine for binary circuits
  garbled/yao/          Yao's garbled circuits
  garbled/bmr/          BMR, with its own transport layer in bmr/transport/
  he/core/              lattice-based homomorphic encryption (BGV)
  he/offline/           HE-based preprocessing: key generation, ZK proofs
  ot/                   base OT, OT extension, OT-based triple generation
  ecdsa/                threshold ECDSA building blocks
  protocols/<family>/   secret-sharing protocols, one directory per family:
      common/           share types, MAC checks, buffers, daBits, shuffles
      replicated/       three-party replicated secret sharing (all variants)
      rep4/             four-party replicated secret sharing
      shamir/           Shamir secret sharing
      spdz/             SPDZ over fields: MASCOT, LowGear, HighGear, CowGear, ChaiGear
      spdz2k/           SPDZ2k
      spdzwise/         SPDZ-wise (honest majority with MACs)
      semi/             semi-honest dishonest majority: Semi, Hemi, Temi, Soho
      dealer/           dealer-assisted protocols
      astra_trio/       Astra and Trio
      atlas/            ATLAS
      brain/            the Brain protocol
      fake/             insecure test preprocessing
parties/                executables; each source file is named after its binary
  arith/                arithmetic virtual machines   (mascot-party.x, ...)
  boolean/              binary virtual machines       (tinier-party.x, ...)
  bmr/                  BMR executables
  ecdsa/                ECDSA executables
  offline/              preprocessing-only executables and checkers
  export/               function export executables (and backends/)
  examples/             library usage examples        (paper-example.x, ...)
  tools/                utilities                     (gen_input_fp.x, ...)
  shared/               protocol instantiations shared by several executables
clients/                external client library and examples (C++ and Python)
mpcl/                   the compiler (Python), see below
Compiler/               compatibility alias of mpcl for existing code
examples/               example programs (formerly Programs/Source)
Programs/               compiler output and runtime data (Bytecode, Schedules, ...)
build/                  the build rules, see the top of Makefile
launch/                 protocol launch scripts and helpers (also reachable as Scripts/)
tests/                  regression gate and its recorded baseline
docs/                   documentation sources
devtools/               maintenance scripts, including the restructuring tools
third_party/            git submodules (libOTe, SimpleOT, simde, ...)
```

## The compiler package

| Subpackage     | Contents |
|----------------|----------|
| `mpcl.core`    | driver (`compile.py` entry point), program and tape representation, instructions, register allocation, basic types (`sint`, `cint`, `sfix`, `Array`, ...) |
| `mpcl.lib`     | the library available in programs: control flow, comparison, floating point, math functions, sorting, permutations, Bristol circuits |
| `mpcl.oram`    | oblivious RAM and oblivious data structures |
| `mpcl.apps`    | machine learning, decision trees, graph algorithms, sparse matrices |
| `mpcl.boolean` | binary-circuit types and instructions |

Programs see the same global names as before, and `from Compiler.x import y`
keeps working: every `Compiler.*` module name is an alias of the new module,
listed in `mpcl/legacy.py`.

## Coming from the original layout

* Every moved file and its new location is listed in
  `devtools/restructure/moved-files.txt`. That file is the place to look up
  where an `#include` of an old path has gone.
* The submodules moved from `deps/` to `third_party/`. CMake build trees
  record absolute paths, so an existing checkout needs `make clean-deps`
  once before `make`.
* Binary names, command-line options, file formats, bytecode and the
  `Programs/` runtime directories are unchanged. `./compile.py tutorial`
  finds `examples/tutorial.mpc`, and programs in `Programs/Source` are
  still found as before.
* Local build settings still go into `CONFIG.mine`. The defaults moved from
  `CONFIG` to `build/config.mk`.

## Checking that nothing changed

`tests/check.sh` builds every executable, recompiles all example programs
and compares the output byte for byte with the recorded baseline, and runs
the tutorial on every protocol. See the top of `tests/check.sh` and
`OVERHAUL_PLAN.md`.
