CXX_FLAGS	:= g++
CXX_FLAGS	:= -std=c++17 -rdynamic
OPT_FLAG	:= -O2

INCLUDE_DIR	:= -Iinclude/ -I/opt/vc/include/
LIBRARY_DIR	:= -L/opt/vc/lib/

SRC			:= $(wildcard src/*.cpp)
TEST_SRC	:= $(wildcard test/*.cpp)
OBJ			:= $(subst src/,obj/,$(SRC:.cpp=.o))
TEST_APPS	:= $(subst test/,,$(TEST_SRC:.cpp=))
LIB			:= -lbcm_host

OBJ_DIR		:= obj
BIN_DIR		:= bin
LIB_DIR		:= lib

QpuWrapper: $(OBJ)
	echo $(TEST_APPS)
#nothing for now

test: $(TEST_APPS)

$(TEST_APPS): $(OBJ)
	$(CXX) $(CXX_FLAGS) $(OPT_FLAG) $(INCLUDE_DIR) $(LIBRARY_DIR) test/$@.cpp $(OBJ) -o $(BIN_DIR)/$@ $(LIB)

$(OBJ): $(SRC) | pre-build
	$(CXX) $(CXX_FLAGS) $(OPT_FLAG) $(INCLUDE_DIR) -c $(subst obj/,src/,$(@:.o=.cpp)) -o $@

pre-build:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)
