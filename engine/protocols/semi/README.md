# Semi-honest dishonest majority

Additive secret sharing for any number of parties with semi-honest
security. Triples come from oblivious transfer (`semi.sh`, `semi2k.sh`),
or from homomorphic encryption: semi-homomorphic (`hemi.sh`), threshold
(`temi.sh`) or somewhat homomorphic (`soho.sh`).

## Main classes

- `SemiShare`, `Semi2kShare`, `HemiShare`, `TemiShare`, `SohoShare`: share types.
- `Semi`, `Hemi`: multiplication protocols.
- `SemiInput`, `SemiOpener`, `DirectSemiOpener`: input and opening.
- `SemiPrep`, `SemiPrep2k`, `HemiPrep`, `HemiMatrixPrep`, `TemiPrep`, `SohoPrep`: preprocessing.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
