# Insecure preprocessing and emulation

Insecure components for testing and benchmarking: preprocessing written to
files by `Fake-Offline.x` and read back, and the `FakeProtocol` used by
the emulator (`emulate.x`). Never use these where security matters.

## Main classes

- `FakeShare`, `FakeProtocol`, `FakePrep`, `FakeInput`, `FakeOpener`: the emulator's protocol.
- `Files`, `FilesBase`, `KeySetup`: writing fake preprocessing and keys (`fake_stuff.h`).

Headers declare, `.hpp` files hold the template definitions.
See `docs/navigating-c++.rst` for how the pieces fit together.
