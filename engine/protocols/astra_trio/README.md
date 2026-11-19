# Astra and Trio

Three-party protocols in the preprocessing model: two online parties
(`astra-online.sh`, `trio-online.sh`) run on correlations produced in a
separate three-party preprocessing phase (`astra-prep.sh`, `trio-prep.sh`),
which writes them to `Player-Data/3-astra-*` or `Player-Data/3-trio-*`.

## Main classes

- `AstraShare`, `TrioShare`: online shares; `AstraPrepShare`, `TrioPrepShare` for the preprocessing phase.
- `Astra`, `Trio`: multiplication; `AstraPrepProtocol`, `TrioPrepProtocol` produce the correlations.
- `AstraInput`, `TrioInput`, `AstraOpener`, `TrioOpener`: input and opening.
- `AstraPrep`: preprocessing that reads what the prep phase wrote.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
