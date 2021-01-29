CXX_FLAGS   := g++
CXX_FLAGS   := -std=c++17 -rdynamic
OPT_FLAG    := -O2

AR_FLAGS    := ar
AR_FLAGS    := -rcs

INCLUDE_DIR := -Iinclude/ -I/opt/vc/include/
LIBRARY_DIR := -L/opt/vc/lib/

SRC         := $(wildcard src/*.cpp)
TEST_SRC    := $(wildcard test/*.cpp)
OBJ         := $(subst src/,obj/,$(SRC:.cpp=.o))
TEST_APPS   := $(subst test/,,$(TEST_SRC:.cpp=))
LIB_NAME    := QpuWrapper
TEST_LIB    := -lbcm_host -pthread

OBJ_DIR     := obj
BIN_DIR     := bin
LIB_DIR     := lib

QpuWrapper: $(OBJ)
	$(AR) $(AR_FLAGS) $(LIB_DIR)/$(LIB_NAME).a $^

test: $(TEST_APPS)

$(TEST_APPS): $(OBJ) | QpuWrapper
	$(CXX) $(CXX_FLAGS) $(OPT_FLAG) $(INCLUDE_DIR) $(LIBRARY_DIR) test/$@.cpp -o $(BIN_DIR)/$@ $(TEST_LIB) $(LIB_DIR)/$(LIB_NAME).a

$(OBJ): $(SRC) | pre-build
	$(CXX) $(CXX_FLAGS) $(OPT_FLAG) $(INCLUDE_DIR) -c $(subst obj/,src/,$(@:.o=.cpp)) -o $@

pre-build:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(LIB_DIR)

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(BIN_DIR)
	rm -rf $(LIB_DIR)