#CLANG_BASE = /home/kaled/clang_base
#LLVM_BASE = /mnt/sda2/llvm-6.0.0
#CXX = $(CLANG_BASE)/bin/clang
CXX= gcc
#
CXXFLAGS = -Wall -Wextra -W  -I./include
#CXXFLAGS += -I$(CLANG_BASE)/include -I$(LLVM_BASE)/include
CXXFLAGS += -Wno-unused-parameter
#LDFLAGS = -L$(CLANG_BASE)/lib -L$(LLVM_BASE)/lib
#LDLIBS = 
LIB32 = libFuSeBMC_GoalTracerLib_32.a
LIB64 = libFuSeBMC_GoalTracerLib_64.a
SRC_DIR = src
INCLUDE_DIR = include
OBJ_DIR32 = obj32
OBJ_DIR64 = obj64
HEADER = $(wildcard $(INCLUDE_DIR)/*.h)
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ32 = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR32)/%.o)
OBJ64 = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR64)/%.o)
.PHONY: all clean debug32 release32 debug64 release64
debug32: CXXFLAGS += -g -o0 -DMYDEBUG -m32
debug64: CXXFLAGS += -g -o0 -DMYDEBUG -m64

debug32: $(LIB32)
debug64: $(LIB64)

release32: CXXFLAGS += -o3 -os -m32
release64: CXXFLAGS += -o3 -os -m64

release32: $(LIB32)
release64: $(LIB64)
all: release32 release64
$(OBJ_DIR32)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) -c $< -o $@
$(OBJ_DIR64)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR)/%.h
	$(CXX) $(CXXFLAGS) $(CFLAGS) -c $< -o $@
$(LIB32): $(OBJ32)
	ar rcs $@ $^
$(LIB64): $(OBJ64)
	ar rcs $@ $^

clean:
	rm -f $(OBJ_DIR32)/*.o
	rm -f $(OBJ_DIR64)/*.o
	rm -f $(LIB32)
	rm -f $(LIB64)
