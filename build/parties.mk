# Executables.  Every program is listed once, by binary name, in the list
# describing how it is linked.  Its source is found under parties/*/ (or
# clients/) by name, so moving a program between groups needs no change
# here.

# virtual machines and their offline counterparts
VM_PROGRAMS = \
	Player-Online emulate no-party \
	astra-party astra-prep-party atlas-party brain-party \
	chaigear-party cowgear-party dealer-ring-party hemi-party \
	highgear-party lowgear-party malicious-rep-field-party \
	malicious-rep-ring-party malicious-shamir-party mama-party \
	mascot-party ps-rep-field-party ps-rep-ring-party rep4-ring-party \
	replicated-field-party replicated-ring-party semi-party \
	semi2k-party shamir-party soho-party spdz2k-party \
	sy-rep-field-party sy-rep-ring-party sy-shamir-party temi-party \
	trio-party trio-prep-party \
	ccd-party malicious-ccd-party malicious-rep-bin-party \
	ps-rep-bin-party replicated-bin-party semi-bin-party \
	tinier-party tiny-party yao-party \
	cowgear-offline hemi-offline lowgear-offline mal-shamir-offline \
	mascot-offline semi-offline semi2k-offline

# stand-alone tools and examples linked against the runtime library
UTILITY_PROGRAMS = \
	binary-example he-example l2h-example mixed-example paper-example \
	protocol-tutorial \
	export-a2b export-b2a export-msort export-sort export-trunc \
	Check-Offline Check-Offline-Z2k check-passive cnc-offline \
	ot-offline pairwise-offline simple-offline spdz2-offline \
	stream-fake-mascot-triples \
	Server gc-emulate gen_input_f2n gen_input_fp hyper prime \
	bankers-bonus-client

# tools that need nothing but their own object
STANDALONE_PROGRAMS = galois-degree default-prime-length

ECDSA_PROGRAMS = \
	atlas-ecdsa-party fake-spdz-ecdsa-party mal-rep-ecdsa-party \
	mal-shamir-ecdsa-party mascot-ecdsa-party rep-ecdsa-party \
	rep4-ecdsa-party semi-ecdsa-party shamir-ecdsa-party \
	sy-rep-ecdsa-party

BMR_PARTY_PROGRAMS = \
	mal-rep-bmr-party mal-shamir-bmr-party real-bmr-party \
	rep-bmr-party semi-bmr-party shamir-bmr-party

BMR_PROGRAM_PROGRAMS = bmr-program-party bmr-program-tparty

source_of = $(firstword $(wildcard parties/*/$(1).cpp clients/$(1).cpp))
object_of = $(patsubst %.cpp,%.o,$(call source_of,$(1)))

define vm_program
$(1).x: $(call object_of,$(1)) $$(RUNTIME_LIB)
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(LDLIBS) $$(RUNTIME_LIB)
endef

define utility_program
$(1).x: $(call object_of,$(1)) $$(RUNTIME_LIB)
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(LDLIBS)
endef

define standalone_program
$(1).x: $(call object_of,$(1))
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(LDLIBS)
endef

define ecdsa_program
$(1).x: $(call object_of,$(1)) engine/ecdsa/p256_element.o $$(RUNTIME_LIB)
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(LDLIBS)
endef

define bmr_party_program
$(1).x: $(call object_of,$(1)) $$(BMR_OBJS) $$(RUNTIME_LIB)
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(BOOST) $$(LDLIBS)
endef

define bmr_program_program
$(1).x: $$(BMR_OBJS) $$(RUNTIME_LIB) $(call source_of,$(1)) $$(SIMPLEOT_LIBS)
	$$(CXX) -o $$@ $$(CFLAGS) $$^ $$(BOOST) $$(LDLIBS)
endef

STATIC_LINK = -Wl,-Bstatic -static-libgcc -static-libstdc++
STATIC_TAIL = $(BOOST) $(LDLIBS) -lz -Wl,-Bdynamic -ldl

define static_program
static/$(1).x: $(call object_of,$(1)) $$(RELEASE_LIB) $$(SIMPLEOT_LIBS) \
		local/lib/libcryptoTools.a local/lib/liblibOTe.a
	$$(MAKE) static-dir
	$$(CXX) -o $$@ $$(CFLAGS) $$^ -Wl,-Map=$$<.map $$(STATIC_LINK) \
		$$(RELEASE_LIB) $$(LIBOTE) $$(SIMPLEOT_LIBS) $$(STATIC_TAIL)
endef

define static_ecdsa_program
static/$(1).x: $(call object_of,$(1)) engine/ecdsa/p256_element.o \
		$$(RUNTIME_OBJS) $$(OT_OBJS) $$(SIMPLEOT_LIBS)
	$$(CXX) $$(CFLAGS) -o $$@ $$^ -Wl,-Map=$$<.map $$(STATIC_LINK) \
		$$(STATIC_TAIL)
endef

$(foreach p,$(VM_PROGRAMS),$(eval $(call vm_program,$(p))))
$(foreach p,$(UTILITY_PROGRAMS),$(eval $(call utility_program,$(p))))
$(foreach p,$(STANDALONE_PROGRAMS),$(eval $(call standalone_program,$(p))))
$(foreach p,$(ECDSA_PROGRAMS),$(eval $(call ecdsa_program,$(p))))
$(foreach p,$(BMR_PARTY_PROGRAMS),$(eval $(call bmr_party_program,$(p))))
$(foreach p,$(BMR_PROGRAM_PROGRAMS),$(eval $(call bmr_program_program,$(p))))

STATIC_PROGRAMS = $(VM_PROGRAMS) $(BMR_PARTY_PROGRAMS) $(BMR_PROGRAM_PROGRAMS)
STATIC_RELEASE = $(filter %-party,$(STATIC_PROGRAMS)) emulate
$(foreach p,$(STATIC_PROGRAMS),$(eval $(call static_program,$(p))))
$(foreach p,$(ECDSA_PROGRAMS),$(eval $(call static_ecdsa_program,$(p))))

# programs with their own link line
Fake-Offline.x: parties/offline/Fake-Offline.o $(RUNTIME_LIB)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDLIBS)

Fake-ECDSA.x: parties/ecdsa/Fake-ECDSA.cpp engine/ecdsa/p256_element.o \
		$(RUNTIME_LIB) engine/vm/prep_base.o
	$(CXX) -o $@ $^ $(CFLAGS) $(LDLIBS)

ot.x: $(OT_OBJS) $(RUNTIME_LIB) parties/offline/ot.o \
		parties/shared/ot_machine.o $(SIMPLEOT_LIBS)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

# additional objects per program
REP4_BOOLEAN = engine/boolean/rep4_secret.o engine/boolean/rep4_prep.o
SPDZ2K_INSTANCES = $(patsubst %.cpp,%.o,$(wildcard parties/shared/spdz2k*.cpp))
MALREP_OPTIONS = engine/protocols/replicated/malrep_ring_options.o

export-trunc.x export-sort.x export-msort.x export-b2a.x: \
	parties/export/backends/export_ring.o
export-a2b.x: engine/boolean/atlas_secret.o parties/shared/spdz.o \
	parties/shared/spdz2k_64_64.o $(BOOLEAN_SEMI_OBJS) $(TINIER_OBJS) \
	$(EXPORT_BACKEND_OBJS) $(REP4_BOOLEAN) $(HE_LIB)

ot-offline.x: $(OT_OBJS) $(SIMPLEOT_LIBS) parties/shared/triple_machine.o
gc-emulate.x: engine/boolean/fake_secret.o engine/boolean/square64.o
simple-offline.x pairwise-offline.x cnc-offline.x spdz2-offline.x: $(HE_LIB)
yao-party.x static/yao-party.x: $(YAO_OBJS)

replicated-bin-party.x replicated-ring-party.x replicated-field-party.x \
brain-party.x malicious-rep-bin-party.x: engine/boolean/square64.o
ps-rep-bin-party.x: engine/boolean/postsacri_bin.o
semi-bin-party.x semi-party.x semi2k-party.x: $(BOOLEAN_SEMI_OBJS)
tiny-party.x tinier-party.x: $(OT_OBJS)
spdz2k-party.x: $(TINIER_OBJS) $(SPDZ2K_INSTANCES)
static/spdz2k-party.x: $(SPDZ2K_INSTANCES)
static/semi-party.x static/semi2k-party.x static/semi-bin-party.x: \
	$(BOOLEAN_STATIC_OBJS)
hemi-party.x temi-party.x soho-party.x: $(HE_LIB) $(BOOLEAN_SEMI_OBJS) \
	$(OT_OBJS)
static/hemi-party.x static/temi-party.x static/soho-party.x: $(HE_OBJS) \
	$(BOOLEAN_STATIC_OBJS)
cowgear-party.x chaigear-party.x: $(HE_LIB) $(TINIER_OBJS)
lowgear-party.x: $(HE_LIB) $(TINIER_OBJS) \
	engine/protocols/spdz/lowgear_key_gen.o
highgear-party.x: $(HE_LIB) $(TINIER_OBJS) \
	engine/protocols/spdz/highgear_key_gen.o
static/cowgear-party.x static/chaigear-party.x: $(HE_OBJS)
static/lowgear-party.x: $(HE_OBJS) engine/protocols/spdz/lowgear_key_gen.o
static/highgear-party.x: $(HE_OBJS) engine/protocols/spdz/highgear_key_gen.o
atlas-party.x: engine/boolean/atlas_secret.o
mascot-party.x static/mascot-party.x Player-Online.x: $(SPDZ_OBJS)
mama-party.x: $(TINIER_OBJS)
ps-rep-ring-party.x malicious-rep-ring-party.x sy-rep-ring-party.x: \
	$(MALREP_OPTIONS)
rep4-ring-party.x: $(REP4_BOOLEAN)
no-party.x static/no-party.x: engine/protocols/common/share_interface.o
semi-ecdsa-party.x: $(BOOLEAN_SEMI_OBJS) $(OT_OBJS) $(SIMPLEOT_LIBS)
mascot-ecdsa-party.x fake-spdz-ecdsa-party.x: $(OT_OBJS) $(SIMPLEOT_LIBS)
rep4-ecdsa-party.x: engine/boolean/rep4_prep.o
emulate.x: engine/boolean/fake_secret.o
semi-bmr-party.x: $(BOOLEAN_SEMI_OBJS) $(OT_OBJS)
real-bmr-party.x: $(OT_OBJS)
paper-example.x: $(RUNTIME_LIB) $(OT_OBJS) $(HE_LIB)
binary-example.x: $(RUNTIME_LIB) $(BOOLEAN_SEMI_OBJS) $(OT_OBJS) \
	engine/boolean/postsacri_bin.o engine/boolean/atlas_secret.o \
	engine/boolean/rep4_prep.o
mixed-example.x: $(RUNTIME_LIB) $(BOOLEAN_SEMI_OBJS) $(OT_OBJS) \
	engine/boolean/postsacri_bin.o engine/boolean/atlas_secret.o \
	engine/boolean/rep4_prep.o parties/shared/tinier.o
l2h-example.x: $(RUNTIME_LIB) $(OT_OBJS) parties/shared/tinier.o
he-example.x: $(HE_LIB)
mascot-offline.x: $(RUNTIME_LIB) $(TINIER_OBJS)
cowgear-offline.x: $(TINIER_OBJS) $(HE_LIB)
lowgear-offline.x: $(TINIER_OBJS) $(HE_LIB) \
	engine/protocols/spdz/cowgear_options.o \
	engine/protocols/spdz/lowgear_key_gen.o
semi-offline.x semi2k-offline.x: $(BOOLEAN_SEMI_OBJS) $(OT_OBJS)
hemi-offline.x: $(BOOLEAN_SEMI_OBJS) $(HE_LIB) $(OT_OBJS)
$(addprefix static/,$(addsuffix .x,$(BMR_PARTY_PROGRAMS) bmr-program-party)): \
	$(BMR_OBJS)
