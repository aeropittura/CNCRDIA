# Shamir secret sharing

Honest-majority Shamir secret sharing for any number of parties, with
semi-honest (`shamir.sh`) and malicious (`mal-shamir.sh`) security; the
threshold is taken from `THRESHOLD`.

## Main classes

- `ShamirShare`, `MaliciousShamirShare`: share types.
- `Shamir`: multiplication protocol.
- `ShamirInput`, `ShamirOpener`, `MaliciousShamirOpener`, `IndirectShamirOpener`: input and opening.
- `ShamirOptions`, `ShamirMachine`: command-line options and the party driver.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
