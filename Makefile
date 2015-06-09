default: release

.PHONY: default release debug all clean

include make-utils/flags.mk
include make-utils/cpp-utils.mk

CXX_FLAGS += -pedantic -Werror -ftemplate-backtrace-limit=0

$(eval $(call use_libcxx))

RELEASE_FLAGS += -fno-rtti

CXX_FLAGS += -Wno-documentation -Ietl/lib/include -Ietl/include/ -Imnist/include/ -ICatch/include -Inice_svm/include
LD_FLAGS += -lpthread

OPENCV_LD_FLAGS=-lopencv_core -lopencv_imgproc -lopencv_highgui
LIBSVM_LD_FLAGS=-lsvm
TEST_LD_FLAGS=$(LIBSVM_LD_FLAGS)

CXX_FLAGS += -DETL_VECTORIZE_FULL

# Activate BLAS mode on demand
ifneq (,$(ETL_MKL))
CXX_FLAGS += -DETL_MKL_MODE $(shell pkg-config --cflags cblas)
LD_FLAGS += $(shell pkg-config --libs cblas)
CXX_FLAGS += -Wno-tautological-compare
else
ifneq (,$(ETL_BLAS))
CXX_FLAGS += -DETL_BLAS_MODE $(shell pkg-config --cflags cblas)
LD_FLAGS += $(shell pkg-config --libs cblas)
endif
endif

CPP_FILES=$(wildcard test_compile/*.cpp)

TEST_CPP_FILES=$(wildcard test/*.cpp)
TEST_FILES=$(TEST_CPP_FILES:test/%=%)

DEBUG_D_FILES=$(CPP_FILES:%.cpp=debug/%.cpp.d) $(TEST_CPP_FILES:%.cpp=debug/%.cpp.d)
RELEASE_D_FILES=$(CPP_FILES:%.cpp=release/%.cpp.d) $(TEST_CPP_FILES:%.cpp=release/%.cpp.d)
RELEASE_DEBUG_D_FILES=$(CPP_FILES:%.cpp=release_debug/%.cpp.d) $(TEST_CPP_FILES:%.cpp=release_debug/%.cpp.d)

$(eval $(call folder_compile,test_compile))
$(eval $(call test_folder_compile,))

$(eval $(call add_executable,compile_rbm,test_compile/compile_rbm.cpp))
$(eval $(call add_executable,compile_conv_rbm,test_compile/compile_conv_rbm.cpp))
$(eval $(call add_executable,compile_conv_rbm_mp,test_compile/compile_conv_rbm_mp.cpp))
$(eval $(call add_executable,compile_dbn,test_compile/compile_dbn.cpp))
$(eval $(call add_executable,compile_dyn_dbn,test_compile/compile_dyn_dbn.cpp,$(OPENCV_LD_FLAGS)))
$(eval $(call add_executable,compile_dbn_svm,test_compile/compile_dbn_svm.cpp,$(LIBSVM_LD_FLAGS)))
$(eval $(call add_executable,compile_conv_dbn,test_compile/compile_conv_dbn.cpp))
$(eval $(call add_executable,compile_conv_dbn_mp,test_compile/compile_conv_dbn_mp.cpp))
$(eval $(call add_executable,compile_conv_dbn_svm,test_compile/compile_conv_dbn_svm.cpp,$(LIBSVM_LD_FLAGS)))
$(eval $(call add_executable,compile_ocv_1,test_compile/rbm_view.cpp,$(OPENCV_LD_FLAGS)))
$(eval $(call add_executable,compile_ocv_2,test_compile/crbm_view.cpp,$(OPENCV_LD_FLAGS)))
$(eval $(call add_executable,compile_ocv_3,test_compile/crbm_mp_view.cpp,$(OPENCV_LD_FLAGS)))

$(eval $(call add_test_executable,dll_test,$(TEST_FILES),$(TEST_LD_FLAGS)))

$(eval $(call add_executable_set,compile_rbm,compile_rbm))
$(eval $(call add_executable_set,compile_conv_rbm,compile_conv_rbm))
$(eval $(call add_executable_set,compile_conv_rbm_mp,compile_conv_rbm_mp))
$(eval $(call add_executable_set,compile_dbn,compile_dbn))
$(eval $(call add_executable_set,compile_dyn_dbn,compile_dyn_dbn))
$(eval $(call add_executable_set,compile_dbn_svm,compile_dbn_svm))
$(eval $(call add_executable_set,compile_conv_dbn_svm,compile_conv_dbn_svm))
$(eval $(call add_executable_set,compile_conv_dbn,compile_conv_dbn))
$(eval $(call add_executable_set,dll_test,dll_test))

$(eval $(call add_executable_set,compile,compile_rbm compile_conv_rbm compile_conv_rbm_mp compile_dbn compile_conv_dbn_mp compile_dbn_svm compile_conv_dbn compile_conv_dbn_svm compile_dyn_dbn))
$(eval $(call add_executable_set,compile_ocv,compile_ocv_1 compile_ocv_2 compile_ocv_3))

release: release_compile release_dll_test release_compile_ocv
release_debug: release_debug_compile release_debug_dll_test release_debug_compile_ocv
debug: debug_compile debug_dll_test debug_compile_ocv

all: release debug release_debug

debug_test: debug
	./debug/bin/dll_test

release_test: release
	./release/bin/dll_test

release_debug_test: release_debug
	./release_debug/bin/dll_test

test: all
	./debug/bin/dll_test
	./release/bin/dll_test
	./release_debug/bin/dll_test

update_tests: release_dll_test
	bash tools/generate_tests.sh

doc:
	doxygen Doxyfile

clean: base_clean
	rm -rf latex/ html/

-include tests.mk

-include $(DEBUG_D_FILES)
-include $(RELEASE_D_FILES)
-include $(RELEASE_DEBUG_D_FILES)
