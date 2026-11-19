# SPDZ family

Dishonest-majority protocols with MACs over fields: MASCOT
(`mascot.sh`, OT-based), and the homomorphic-encryption variants LowGear,
HighGear, CowGear and ChaiGear, plus MAMA (multiple MACs). Key
generation for the HE variants lives here too.

## Main classes

- `MascotPrep`, `MascotTriplePrep`, `MascotFieldPrep`, `MascotInputPrep`: OT-based preprocessing.
- `LowGearShare`, `HighGearShare`, `CowGearShare`, `ChaiGearShare`, `MamaShare`: share types.
- `CowGearPrep`, `ChaiGearPrep`, `MamaPrep`: preprocessing.
- `LowGearKeyGenerator`, `HighGearKeyGenerator`: distributed key generation (security-critical).
- `CowGearOptions`: command-line options shared by the HE variants.

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
