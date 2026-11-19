# Source provenance

- Upstream: https://github.com/data61/MP-SPDZ
- Source revision: `d3268aa3069536655c229ea714d27f16862797e3`
- Source branch: `master`
- Import date: 2026-11-09
- Imported commits: 1027 (exact upstream ancestry count).
- Import identity: aeropittura <101230990+aeropittura@users.noreply.github.com>

Imported commits preserve original author and committer timestamps, including
timezone offsets, and upstream parent relationships. Original-author,
Original-committer, Original-co-author, and Original-commit trailers retain
source attribution. Upstream Co-authored-by trailers are relabeled as
Original-co-author to distinguish source credit from importer metadata.
The Git author and committer fields identify the repository importer, not the
original developers. Historical dates represent upstream history, not work
performed by the importer on those dates.

Implementation files preserve upstream snapshots; import documentation is
present from the first imported commit onward. Repository naming and import
documentation are introduced in the first imported commit and carried forward
without adding commits.
Commit dates remain the original upstream dates and do not date the import work.
Library APIs, package names, licenses, source headers, and upstream credits are
preserved. Upstream signatures are omitted because rewritten commits have new
IDs and cannot retain valid upstream signatures. No upstream endorsement is implied.

## Modifications after the import

The statements above describe the imported history up to the upstream
revision named at the top. The commits that follow it modify the imported
code. They are a derivative work of MP-SPDZ, distributed under the same
licences (see [License.txt](License.txt)), and are not endorsed by the
upstream authors.

- The source tree is reorganised (C++ runtime in `engine/`, executables in
  `parties/`, compiler in the `mpcl` package, and so on; see
  [ARCHITECTURE.md](ARCHITECTURE.md)), and files are renamed.
  `devtools/restructure/moved-files.txt` maps every old path to its new one.
- The Python package `Compiler` is renamed `mpcl`. `Compiler` remains as a
  compatibility alias.
- The build is split into `build/*.mk`, with `clean-objects`, `compdb` and
  `check` targets.
- C++ identifiers are renamed to one naming scheme (PascalCase types,
  snake_case functions and variables, abbreviations spelled out), and
  abbreviated local variables in the compiler are spelled out. The old and
  new names are listed in `devtools/rename/cxx-names.tsv` and
  `devtools/rename/py-locals.tsv`.
- The protocol launch scripts are replaced by one script driven by
  `launch/protocols.tsv`; the script names are unchanged.
- TLS peer checking is ported to an OpenSSL interface that also exists in
  OpenSSL 4, and the certificate scripts add a subjectAltName.
- The compiler writes schedule options in sorted order, so that its output
  no longer depends on set iteration order.
- Each protocol family in `engine/protocols/` has a short README.
- Behaviour is checked against a baseline recorded from the unmodified
  upstream revision (`tests/`).

Copyright notices and the licence files are kept. Where a file was moved or
changed, its upstream history remains available through the imported commits.
