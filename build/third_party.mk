# Third-party dependencies: submodules in third_party/, installed into local/.

ifeq ($(OS), Darwin)
setup: mac-setup
else
setup: maybe-boost linux-machine-setup
endif

mac-setup: mac-machine-setup
	Scripts/get-brew.sh
	brew install openssl boost libsodium gmp yasm ntl cmake

linux-machine-setup:
mac-machine-setup:

ifeq ($(AVX_OT), 1)
$(SIMPLEOT_ASM_LIB): third_party/SimpleOT/Makefile
	$(MAKE) -C third_party/SimpleOT

engine/ot/base_ot.o: third_party/SimpleOT/Makefile

third_party/SimpleOT/Makefile:
	git submodule update --init third_party/SimpleOT || git clone https://github.com/mkskeller/SimpleOT third_party/SimpleOT
endif

$(SIMPLEOT_C_LIB): third_party/SimplestOT_C/ref10/Makefile
	$(MAKE) -C third_party/SimplestOT_C/ref10

engine/ot/base_ot.o: third_party/SimplestOT_C/ref10/Makefile

third_party/SimplestOT_C/ref10/Makefile:
	git submodule update --init third_party/SimplestOT_C || git clone https://github.com/mkskeller/SimplestOT_C third_party/SimplestOT_C
	cd third_party/SimplestOT_C/ref10; PATH="$(CURDIR)/local/bin:$(PATH)" cmake .

.PHONY: Programs/Circuits
Programs/Circuits:
	git submodule update --init Programs/Circuits || git clone https://github.com/mkskeller/bristol-fashion Programs/Circuits

third_party/libOTe/libOTe:
	git submodule update --init --recursive third_party/libOTe || git clone --recurse-submodules https://github.com/mkskeller/softspoken-implementation third_party/libOTe

third_party/libOTe2/libOTe:
	git submodule update --init --recursive third_party/libOTe2 || git clone --recurse-submodules https://github.com/mkskeller/libOTe third_party/libOTe2

boost: third_party/libOTe/libOTe
	cd third_party/libOTe; \
	python3 build.py --setup --boost --install=$(CURDIR)/local
maybe-boost: third_party/libOTe/libOTe
	cd `mktemp -d`; \
	PATH="$(CURDIR)/local/bin:$(PATH)" cmake $(CURDIR)/third_party/libOTe -DCMAKE_CXX_COMPILER=$(CXX) || \
	{ cd -; make boost; }

third_party/secure-join/secure-join:
	git submodule update --init --recursive third_party/secure-join || git clone --recurse-submodules https://github.com/mkskeller/secure-join third_party/secure-join

OTE_OPTS += -DENABLE_SOFTSPOKEN_OT=ON -DENABLE_SILENTOT=ON -DCMAKE_CXX_COMPILER=$(OTE_CXX) -DCMAKE_INSTALL_LIBDIR=lib -DFETCH_COPROTO=ON -DENABLE_PORTABLE_AES=ON

ifeq ($(USE_TUNGSTEN), 1)
OTE_OPTS += -DENABLE_CIRCUITS=ON -DENABLE_SILENT_VOLE=ON
endif

ifeq ($(ARM), 1)
OTE_OPTS += -DENABLE_AVX=OFF -DENABLE_SSE=OFF
SJ_OPTS += -DSECUREJOIN_ENABLE_SSE=OFF
else
ifeq ($(AVX_OT), 0)
OTE_OPTS += -DENABLE_AVX=OFF
else
OTE_OPTS += -DENABLE_AVX=ON -DENABLE_SSE=ON
endif
endif

ifeq ($(USE_SHARED_OTE), 1)
OTE = $(LIBOTE_SHARED)
else
OTE = $(LIBOTE_STATIC)
endif

libote:
	rm $(LIBOTE_STATIC) $(LIBOTE_SHARED)* 2>/dev/null; true
	$(MAKE) $(OTE)

local/lib/libcryptoTools.a: $(LIBOTE_STATIC)
local/lib/libcryptoTools.so: $(LIBOTE_SHARED)
$(SECUREJOIN_DEPS):

ifeq ($(USE_KOS), 0)
engine/ot/ot_extension_with_matrix.o: $(OTE)
endif

engine/boolean/semi_prep.o engine/boolean/semi_secure_shuffle.o: $(SECUREJOIN)

ifeq ($(USE_LIBOTE2), 1)
local/lib/liblibOTe.a: third_party/libOTe2/libOTe
	cd third_party/libOTe2; \
	PATH="$(CURDIR)/local/bin:$(PATH)" python3 build.py --install=$(CURDIR)/local -DBUILD_SHARED_LIBS=0 $(OTE_OPTS) && \
	touch ../../local/lib/liblibOTe.a
else
local/lib/liblibOTe.a: third_party/libOTe/libOTe
	make maybe-boost; \
	cd third_party/libOTe; \
	PATH="$(CURDIR)/local/bin:$(PATH)" python3 build.py --install=$(CURDIR)/local -- -DBUILD_SHARED_LIBS=0 $(OTE_OPTS) && \
	touch ../../local/lib/liblibOTe.a
endif

$(LIBOTE_SHARED): third_party/libOTe/libOTe maybe-boost
	cd third_party/libOTe; \
	python3 build.py --install=$(CURDIR)/local -- -DBUILD_SHARED_LIBS=1 $(OTE_OPTS)

cmake:
	wget https://github.com/Kitware/CMake/releases/download/v3.24.1/cmake-3.24.1.tar.gz
	tar xzvf cmake-3.24.1.tar.gz
	cd cmake-3.24.1; \
	./bootstrap --parallel=8 --prefix=../local && make -j8 && make install

$(SECUREJOIN_DEPS): $(LIBOTE_STATIC)
$(SECUREJOIN_LIB): $(SECUREJOIN_DEPS)

SJ_OPTS += -DENABLE_SODIUM=OFF -DENABLE_MRR=OFF -DENABLE_RELIC=OFF -DENABLE_SIMPLESTOT_ASM=OFF -DENABLE_SOFTSPOKEN_OT=ON -DCMAKE_CXX_COMPILER=$(CXX) $(OTE_OPTS) -DFETCH_LIBOTE=OFF -DCMAKE_PREFIX_PATH=$(CURDIR)/lib -DEXTRA_INCLUDES="$(CURDIR)/local/include:$(CURDIR)/third_party" -DEXTRA_LINKS=$(CURDIR)/local/lib

secure-join: $(SECUREJOIN_LIB)
$(SECUREJOIN_LIB): third_party/secure-join/secure-join $(LIBOTE_STATIC)
	cd third_party/secure-join; \
	python3 build.py --install=$(CURDIR)/local $(SJ_OPTS) && \
	cd ../.. && \
	test -e local/lib/libsecureJoin.a && touch local/lib/libsecureJoin.a

third_party/simde/simde:
	git submodule update --init third_party/simde || git clone https://github.com/simd-everywhere/simde third_party/simde

third_party/sse2neon/sse2neon.h:
	git submodule update --init third_party/sse2neon || git clone https://github.com/DLTcollab/sse2neon third_party/sse2neon

clean-deps:
	-bash -c "rm -rf local/lib/{liblibOTe.*,libsecureJoin.a} third_party/*/out third_party/SimplestOT_C/{.git*,*} third_party/SimpleOT/{.git*,*}"
