# CNCRDIA build.  The rules live in build/:
#
#   build/config.mk       compiler, flags and options (override in CONFIG.mine)
#   build/objects.mk      object groups and libraries
#   build/parties.mk      every executable and how it is linked
#   build/targets.mk      convenience targets, libraries, compilation rule
#   build/third_party.mk  submodules and libraries installed into local/
#
# Typical use:  make setup && make -j8 mascot-party.x

include build/config.mk
include build/objects.mk
include build/parties.mk
include build/targets.mk
include build/third_party.mk
