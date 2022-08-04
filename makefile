CC = g++
SRC := src
OBJ := build
CFLAGS := -std=c++17 -fpermissive -lstdc++fs 
SIZE_OPTIMIZATION_FLAGS := -O3 -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-exceptions \
-fno-ident -fomit-frame-pointer -fmerge-all-constants -Wl,--build-id=none

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

all: build_directory cate
smol: build_directory cate

build_directory:
	@mkdir -p $(OBJ)

cate: $(OBJECTS)
	@$(CC) $^ -o cate $(CFLAGS)

smol_cate: $(OBJECTS)
	@$(CC) $^ -o cate $(SIZE_OPTIMIZATION_FLAGS) $(CFLAGS)
	@strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag cate

lex: src/lexer.l
	flex --noyywrap -osrc/Lexer.cpp --header-file=src/Lexer.hpp src/lexer.l
	sed -i 's/yywrap() { return 1;}/yywrap();/g' src/Lexer.hpp

$(OBJ)/%.o: $(SRC)/%.cpp
	@$(CC) -I$(SRC) -c $< -o $@ $(CFLAGS) $(SIZE_OPTIMIZATION_FLAGS)

install:
	@cp cate /usr/bin
	@echo "Installed cate!"

uninstall:
	@rm /usr/bin/cate
	@echo "Unnstalled cate!"

clean:
	@rm -rf CATE_DEBUG obj/*.o
