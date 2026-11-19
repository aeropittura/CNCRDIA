# Three-party replicated secret sharing

Honest-majority three-party replicated sharing in its semi-honest and
malicious variants, over fields, rings and bits. Malicious security
follows Lindell and Nof: triples are generated optimistically and
sacrificed, or, for the post-sacrifice (`ps-*`) variants, multiplications
run optimistically and are checked afterwards. The MAC-based `sy-*`
variants live in `../spdzwise/`.
Launchers: `ring.sh`, `rep-field.sh`, `replicated.sh`, `mal-rep-*.sh`,
`ps-rep-*.sh`.

## Main classes

- `Rep3Share`, `Rep3Share2`: semi-honest share types; `MaliciousRep3Share`, `MalRepRingShare`, `PostSacriRepFieldShare`, `PostSacriRepRingShare`: malicious ones.
- `Replicated`, `ReplicatedBase`: multiplication; `PostSacrifice`: multiplication checked afterwards.
- `ReplicatedInput`, `ReplicatedOpener`, `MaliciousRepOpener`, `HashMaliciousRepOpener`: input and opening (the malicious openers are security-critical).
- `MaliciousRepPrep`, `MalRepRingPrep`, `RepRingOnlyEdabitPrep`, `SemiRep3Prep`: preprocessing.
- `Rep3Shuffler`: shuffling.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
