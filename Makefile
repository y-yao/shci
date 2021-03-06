# Default options.
CXX := mpic++
CXX_WARNING_OPTIONS := -Wall -Wextra -Wno-unused-result
CXXFLAGS := -std=c++11 -O3 -fopenmp $(CXX_WARNING_OPTIONS)
LDLIBS := -pthread -lpthread
SRC_DIR := src
BUILD_DIR := build
LIB_DIR := lib
EXE := shci
TEST_EXE := shci_test

# Libraries.
CXXFLAGS := $(CXXFLAGS) -I $(LIB_DIR)
UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	TOOLS_DIR := $(HOME)/tools
	GPERFTOOLS_DIR := $(TOOLS_DIR)/gperftools
	ifneq ($(wildcard $(GPERFTOOLS_DIR)),)
#		LDLIBS := -L $(GPERFTOOLS_DIR)/lib $(LDLIBS) -ltcmalloc
	endif
endif

# Load Makefile.config if exists.
LOCAL_MAKEFILE := local.mk
ifneq ($(wildcard $(LOCAL_MAKEFILE)),)
	include $(LOCAL_MAKEFILE)
endif

# Sources and intermediate objects.
MAIN_SRC := $(SRC_DIR)/main.cc
SRCS := $(shell find $(SRC_DIR) ! -name "main.cc" ! -name "*_test.cc" -name "*.cc")
HEADERS := $(shell find $(SRC_DIR) -name "*.h")
SUBMODULES := $(LIB_DIR)/eigen $(LIB_DIR)/googletest $(LIB_DIR)/hpmr $(LIB_DIR)/hps $(LIB_DIR)/json
OBJS := $(SRCS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
TESTS := $(shell find $(SRC_DIR) -name "*_test.cc")
GTEST_DIR := $(LIB_DIR)/googletest/googletest
GMOCK_DIR := $(LIB_DIR)/googletest/googlemock
TEST_MAIN_SRC := gtest_main_mpi.cc
TEST_OBJS := $(TESTS:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
GTEST_ALL_SRC := ${GTEST_DIR}/src/gtest-all.cc
GMOCK_ALL_SRC := ${GMOCK_DIR}/src/gmock-all.cc
TEST_MAIN_OBJ := $(BUILD_DIR)/gtest_main.o
TEST_LIB := $(BUILD_DIR)/libgtest.a
TEST_CXXFLAGS := $(CXXFLAGS) -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -pthread

.PHONY: all test test_mpi clean

.SUFFIXES:

all: $(EXE)

test: $(TEST_EXE)
	./$(TEST_EXE)

clean:
	rm -rf $(BUILD_DIR)
	rm -f ./$(EXE)
	rm -f ./$(TEST_EXE)

$(EXE): $(OBJS) $(MAIN_SRC) $(HEADERS) $(GPERFTOOLS_DIR)
	$(CXX) $(CXXFLAGS) $(MAIN_SRC) $(OBJS) -o $(EXE) $(LDLIBS)

$(OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	mkdir -p $(@D) && $(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_EXE): $(TEST_OBJS) $(OBJS) $(TEST_MAIN_SRC) $(TEST_LIB) 
	$(CXX) $(TEST_CXXFLAGS) $(TEST_OBJS) $(OBJS) $(TEST_MAIN_SRC) $(TEST_LIB) -o $(TEST_EXE) $(LDLIBS)

$(BUILD_DIR)/gtest-all.o: $(GTEST_ALL_SRC)
	mkdir -p $(@D) && $(CXX) $(TEST_CXXFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) -c $(GTEST_ALL_SRC) -o $@

$(BUILD_DIR)/gmock-all.o: $(GMOCK_ALL_SRC)
	mkdir -p $(@D) && $(CXX) $(TEST_CXXFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) -c $(GMOCK_ALL_SRC) -o $@

$(TEST_LIB): $(BUILD_DIR)/gtest-all.o $(BUILD_DIR)/gmock-all.o
	$(AR) $(ARFLAGS) $@ $(BUILD_DIR)/gtest-all.o $(BUILD_DIR)/gmock-all.o

$(TEST_OBJS): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	mkdir -p $(@D) && $(CXX) $(TEST_CXXFLAGS) -c $< -o $@

$(GPERFTOOLS_DIR):
#	$(warning tcmalloc not found)
