# ATLAS

Honest-majority Shamir secret sharing with the ATLAS multiplication
(degree reduction by a designated party per gate batch), semi-honest
security. Launcher: `atlas.sh` (honours `THRESHOLD`).

## Main classes

- `AtlasShare`: Shamir share type using `Atlas`.
- `Atlas`: multiplication protocol.
- `AtlasPrep`: preprocessing.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
