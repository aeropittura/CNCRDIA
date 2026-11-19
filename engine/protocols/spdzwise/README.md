# SPDZ-wise

Honest-majority protocols made malicious by adding SPDZ-style MACs to a
semi-honest protocol (replicated or Shamir), as in the SPDZ-wise paper.
Launchers: `sy-rep-field.sh`, `sy-rep-ring.sh`, `sy-shamir.sh`.

## Main classes

- `SpdzWiseShare`, `SpdzWiseRingShare`: share types.
- `SpdzWise`, `SpdzWiseRing`: multiplication with MAC checking.
- `SpdzWiseInput`, `SpdzWiseOpener`: input and opening.
- `SpdzWisePrep`, `SpdzWiseRingPrep`: preprocessing.
- `SpdzWiseRep3Shuffler`: shuffling.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
