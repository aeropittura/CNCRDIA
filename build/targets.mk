# Convenience targets, libraries and the generic compilation rule.

all: arithmetic binary gen_input online offline externalIO bmr ecdsa export
all: overdrive she-offline
vm: arithmetic binary

arithmetic: rep-ring rep-field shamir mascot sy dealer-ring-party.x fd \
	emulate.x semi gear trio
binary: rep-bin yao semi-bin-party.x tinier-party.x tiny-party.x \
	ccd-party.x malicious-ccd-party.x real-bmr

semi: semi-bin-party.x semi2k-party.x semi-party.x semi-he
online: Fake-Offline.x Server.x Player-Online.x Check-Offline.x emulate.x \
	mascot-party.x
offline: ot.x ot-offline.x Check-Offline.x mascot-offline.x \
	cowgear-offline.x lowgear-offline.x mal-shamir-offline.x
gen_input: gen_input_f2n.x gen_input_fp.x
externalIO: bankers-bonus-client.x
bmr: bmr-program-party.x bmr-program-tparty.x
real-bmr: $(addsuffix .x,$(BMR_PARTY_PROGRAMS))
yao: yao-party.x
she-offline: Check-Offline.x spdz2-offline.x
overdrive: simple-offline.x pairwise-offline.x cnc-offline.x gear
gear: cowgear-party.x chaigear-party.x lowgear-party.x highgear-party.x
semi-he: hemi-party.x soho-party.x temi-party.x
rep-field: malicious-rep-field-party.x replicated-field-party.x \
	ps-rep-field-party.x
rep-ring: replicated-ring-party.x brain-party.x \
	malicious-rep-ring-party.x ps-rep-ring-party.x rep4-ring-party.x
rep-bin: replicated-bin-party.x malicious-rep-bin-party.x \
	ps-rep-bin-party.x Fake-Offline.x
replicated: rep-field rep-ring rep-bin
spdz2k: spdz2k-party.x ot-offline.x Check-Offline-Z2k.x galois-degree.x \
	Fake-Offline.x
mascot: mascot-party.x spdz2k mama-party.x
shamir: shamir-party.x malicious-shamir-party.x atlas-party.x \
	galois-degree.x
sy: sy-rep-field-party.x sy-rep-ring-party.x sy-shamir-party.x
astra: astra-party.x astra-prep-party.x
trio: trio-party.x trio-prep-party.x
fd: astra trio
ecdsa: $(addsuffix .x,$(ECDSA_PROGRAMS)) Fake-ECDSA.x
ecdsa-static: static-dir $(addprefix static/,$(addsuffix .x,$(ECDSA_PROGRAMS)))
export: $(addsuffix .x,$(filter export-%,$(UTILITY_PROGRAMS)))
machines: $(patsubst %.cpp,%.o,$(wildcard parties/shared/*.cpp))
static-release: static-dir $(addprefix static/,$(addsuffix .x,$(STATIC_RELEASE)))

static-dir:
	@ mkdir static 2> /dev/null; true

tldr: setup
	$(MAKE) mascot-party.x
	mkdir Player-Data 2> /dev/null; true

.PHONY: doc
doc:
	cd docs; $(MAKE) html

-include $(DEPENDENCY_FILES)

$(ALL_OBJS) $(LIBOTE_STATIC): build/config.mk CONFIG.mine
CONFIG.mine:
	touch CONFIG.mine

%.o: %.cpp
	$(CXX) -o $@ $< $(CFLAGS) -MMD -MP -c

ifeq ($(ARM), 1)
$(ALL_OBJS): third_party/simde/simde third_party/sse2neon/sse2neon.h
endif

CFLAGS += -fPIC
LDLIBS += -Wl,-rpath -Wl,$(CURDIR)
LIBOTE = -llibOTe -lcryptoTools

ifeq ($(USE_LIBOTE2), 1)
LIBOTE := -llibOTe -lcoproto -lcryptoTools
else
CFLAGS += -DUSE_LIBOTE1
endif

$(RUNTIME_LIB): $(VM_CORE_OBJS) $(CORE_OBJS) engine/boolean/square64.o \
		engine/boolean/instruction.o
	$(CXX) $(CFLAGS) -shared -o $@ $^ $(LDLIBS)

$(HE_LIB): $(HE_OBJS) $(RUNTIME_LIB)
	$(CXX) $(CFLAGS) -shared -o $@ $^ $(LDLIBS)

$(RELEASE_LIB): $(MALREP_OPTIONS) $(VM_CORE_OBJS) $(CORE_OBJS) \
		$(TINIER_OBJS) $(BOOLEAN_OBJS)
	$(AR) -csr $@ $^

# clean-objects keeps the third-party builds, which take long to redo
clean: clean-objects clean-deps

clean-objects:
	-rm -f $(shell find $(SOURCE_DIRS) -name '*.o' -o -name '*.d') \
		*.x core.* *.a *.so static/*.x static/*.map

bmr-clean:
	-rm -f $(shell find engine/garbled/bmr engine/boolean -name '*.o')

yao-clean:
	-rm -f engine/garbled/yao/*.o
