# Common protocol building blocks

Interfaces and generic components that every protocol family plugs into:
share interfaces, MAC checking, Beaver multiplication, generic
preprocessing (triples, bits, daBits, edaBits), shuffling, and the
`ProtocolSet`/`ProtocolSetup` helpers used by the examples in
`parties/examples/`. Start here when adding a protocol (see
`docs/add-protocol.rst`).

## Main classes

- `ShareInterface`, `ValueInterface`: the static interface a share type provides.
- `NoShare`, `NoProtocol`, `NoInput`, `NoOutput`: a skeleton to copy for a new protocol.
- `Beaver`: multiplication with Beaver triples.
- `MacCheckBase`, `MacCheck_`, `MacCheckZ2k`, `TreeMacCheck`, `DirectMacCheck`: opening with and without MAC checks (security-critical).
- `Opener`, `MatrixOpener`: convenience wrappers for opening values.
- `BufferPrep`, `RingPrep`, `ReplicatedPrep`, `MaliciousRingPrep`, `RingOnlyPrep`, `LimitedPrep`: preprocessing layers.
- `Dabit`, `DabitSacrifice`, `Edabitvec`, `ShuffleSacrifice`: mixed-circuit preprocessing.
- `SecureShuffle`: oblivious shuffling.
- `ProtocolSet`, `ProtocolSetup`, `BinaryProtocolSet`, `MixedProtocolSet`: set up a protocol for use from C++.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
