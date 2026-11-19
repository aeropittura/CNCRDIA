# SPDZ2k

Dishonest-majority computation with MACs over the ring Z_2^k
(Cramer et al.), preprocessing via OT. Launcher: `spdz2k.sh`.

## Main classes

- `Spdz2kShare`: share type (value plus MAC in a larger ring).
- `Spdz2k`: multiplication protocol.
- `Spdz2kPrep`: preprocessing.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
