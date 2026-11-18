# Object groups shared by the libraries and the party executables.

objects = $(patsubst %.cpp,%.o,$(wildcard $(addsuffix /*.cpp,$(1))))

NUMERIC_OBJS = $(call objects,engine/numeric)
SUPPORT_OBJS = $(call objects,engine/support)
NET_OBJS = $(call objects,engine/net)

VM_CORE_OBJS = $(call objects,engine/vm) \
	engine/protocols/shamir/shamir_options.o \
	engine/protocols/common/share_interface.o

HE_OBJS = $(call objects,engine/he/offline engine/he/core) \
	engine/protocols/spdz/cowgear_options.o

BOOLEAN_OBJS = $(filter-out engine/boolean/semi_secure_shuffle.o, \
	$(call objects,engine/boolean)) $(VM_CORE_OBJS)
BOOLEAN_SEMI_OBJS = engine/boolean/semi_prep.o engine/boolean/square64.o \
	engine/boolean/semi.o engine/boolean/semi_secure_shuffle.o \
	engine/boolean/f3.o engine/boolean/f3_code.o

OT_OBJS = $(call objects,engine/ot) $(SIMPLEOT_LIBS)

CORE_OBJS = $(NUMERIC_OBJS) $(SUPPORT_OBJS) $(NET_OBJS) \
	engine/boolean/square64.o engine/vm/online_options.o \
	engine/vm/base_machine.o engine/vm/data_positions.o \
	engine/vm/thread_queues.o engine/vm/thread_queue.o

YAO_OBJS = $(call objects,engine/garbled/yao) $(OT_OBJS) \
	engine/garbled/bmr/key.o
BMR_OBJS = $(call objects,engine/garbled/bmr engine/garbled/bmr/transport)

RUNTIME_OBJS = $(VM_CORE_OBJS) $(CORE_OBJS) engine/boolean/square64.o \
	engine/boolean/instruction.o engine/ot/ot_triple_setup.o \
	engine/ot/base_ot.o $(SIMPLEOT_LIBS)

TINIER_OBJS = parties/shared/tinier.o $(OT_OBJS)
SPDZ_OBJS = parties/shared/spdz.o $(TINIER_OBJS)
EXPORT_BACKEND_OBJS = $(call objects,parties/export/backends)

# libraries built from the groups above
RUNTIME_LIB = libSPDZ.so
HE_LIB = libFHE.so
RELEASE_LIB = librelease.a

SIMPLEOT_C_LIB = third_party/SimplestOT_C/ref10/libSimplestOT.a
SIMPLEOT_LIBS += $(SIMPLEOT_C_LIB)
ifeq ($(AVX_OT), 1)
SIMPLEOT_ASM_LIB = third_party/SimpleOT/libsimpleot.a
SIMPLEOT_LIBS += $(SIMPLEOT_ASM_LIB)
endif

LIBOTE_STATIC = local/lib/liblibOTe.a
LIBOTE_SHARED = local/lib/liblibOTe.so
SECUREJOIN_DEPS = local/lib/libmacoro.a local/lib/libcryptoTools.a \
	local/lib/libcoproto.a
SECUREJOIN_LIB = local/lib/libsecureJoin.a

ifeq ($(USE_LIBOTE2), 1)
ifeq ($(USE_TUNGSTEN), 1)
SECUREJOIN = $(SECUREJOIN_LIB)
endif
SECUREJOIN += $(LIBOTE_STATIC) $(SECUREJOIN_DEPS)
endif

BOOLEAN_SHUFFLE_OBJS = engine/boolean/semi_secure_shuffle.o
BOOLEAN_STATIC_OBJS = $(BOOLEAN_SHUFFLE_OBJS) $(SECUREJOIN)

ifeq ($(USE_KOS), 0)
ifeq ($(USE_SHARED_OTE), 1)
OT_OBJS += $(LIBOTE_SHARED) local/lib/libcryptoTools.so \
	local/lib/libcoproto.so
else
ifeq ($(USE_LIBOTE2), 1)
OT_OBJS += $(LIBOTE_STATIC) local/lib/libmacoro.a \
	local/lib/libcryptoTools.a local/lib/libcoproto.a
else
OT_OBJS += $(LIBOTE_STATIC) local/lib/libcryptoTools.a
endif
endif
endif

BOOLEAN_SEMI_OBJS += $(SECUREJOIN) $(OT_OBJS)

# everything that can be compiled, used for dependency tracking
SOURCE_DIRS = engine parties clients
ALL_OBJS = $(patsubst %.cpp,%.o,$(shell find $(SOURCE_DIRS) -name '*.cpp')) \
	$(SECUREJOIN)
DEPENDENCY_FILES := $(shell find $(SOURCE_DIRS) static -name '*.d' \
	2> /dev/null)

# never delete intermediate objects
.SECONDARY: $(ALL_OBJS)
