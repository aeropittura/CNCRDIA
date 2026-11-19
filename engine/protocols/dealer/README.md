# Dealer

A trusted dealer (the last party) generates preprocessing for the other
parties, who then compute with additive sharing over rings. Launcher:
`dealer-ring.sh`.

## Main classes

- `DealerShare`: share type.
- `Dealer`: multiplication protocol.
- `DealerPrep`, `DealerMatrixPrep`: the dealer's preprocessing.
- `DealerInput`, `DealerOpener`, `DirectDealerOpener`: input and opening.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
