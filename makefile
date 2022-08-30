CC = g++
SRC := src
BUILD_DIR := cate/build
CFLAGS := -std=c++17 -fpermissive -lstdc++fs -pthread
SIZE_OPTIMIZATION_FLAGS := -O3 

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(BUILD_DIR)/%.o, $(SOURCES))

all: build_directory cate
smol: build_directory smol_cate
smol: SIZE_OPTIMIZATION_FLAGS += -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-exceptions \
-fno-ident -fomit-frame-pointer -fmerge-all-constants -Wl,--build-id=none

build_directory:
	@mkdir -p $(BUILD_DIR)

out/cate: $(OBJECTS)
	@$(CC) $^ -o out/cate $(CFLAGS) externals/linux_libfl.a

smol_cate: $(OBJECTS)
	@$(CC) $^ -o out/cate $(SIZE_OPTIMIZATION_FLAGS) $(CFLAGS)  externals/linux_libfl.a
	strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/cate

lex: src/lexer.l
	flex --noyywrap -osrc/Lexer.cpp --header-file=src/Lexer.hpp src/lexer.l
	sed -i 's/yywrap() { return 1;}/ /g' src/Lexer.hpp

$(BUILD_DIR)/%.o: $(SRC)/%.cpp
	@$(CC) -I$(SRC) -c $< -o $@ $(CFLAGS) $(SIZE_OPTIMIZATION_FLAGS)

install:
	@cp out/cate /usr/bin/cate
	@echo "Installed cate!"

uninstall:
	@rm /usr/bin/cate
	@echo "Uninstalled cate!"

clean:
	@rm -rf $(BUILD_DIR)/*.o