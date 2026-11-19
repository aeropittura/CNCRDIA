# Rep4

Four-party replicated secret sharing with malicious security against
one corruption (Dalskov, Escudero and Keller). Launcher: `rep4-ring.sh`.

## Main classes

- `Rep4Share`, `Rep4Share2`: share types for fields and rings.
- `Rep4`: multiplication protocol.
- `Rep4Input`, `Rep4Opener`: input and opening.
- `Rep4RingPrep`, `Rep4RingOnlyPrep`: preprocessing.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
