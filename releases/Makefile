.PHONY: all clean

CC = g++
CFLAGS = -fdiagnostics-color=always -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef			 \
		 -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs 		 \
		 -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual	 \
		 -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers \
		 -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual 			 \
		 -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel 		 \
		 -Wtype-limits -Wwrite-strings -Werror=vla -D_DEBUG -D_EJUDGE_CLIENT_SIDE

CFLAGS_SANITIZER = -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,$\
				   float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,$\
				   object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,$\
				   undefined,unreachable,vla-bound,vptr

RELEASE_NAME = $(name)
HEADER_NAME = tree.h

SRC_DIR = src
BUILD_DIR = build

OUTPUT_DIR = $(RELEASE_NAME)/$(BUILD_DIR)
NON_CODE_DIRS =

CD = $(shell pwd)

NESTED_CODE_DIRS_CD = $(shell find ../$(SRC_DIR) -maxdepth 5 -type d $(NON_CODE_DIRS:%=! -path "*%*"))
NESTED_CODE_DIRS = $(NESTED_CODE_DIRS_CD:../%=%)

FILES_FULL = $(shell find ../$(SRC_DIR) -name "*.cpp" ! -name "main.cpp")
FILES = $(FILES_FULL:%=%)

MAKE_DIRS = $(RELEASE_NAME) $(NESTED_CODE_DIRS:$(SRC_DIR)%=$(OUTPUT_DIR)%)

OBJ = $(FILES:../$(SRC_DIR)/%=$(OUTPUT_DIR)/%)

OBJECTS = $(OBJ:%.cpp=%.o)

all: $(OBJECTS)
	@ar -rcs $(RELEASE_NAME)/$(RELEASE_NAME).a $(OBJECTS)
	@cp ../$(SRC_DIR)/$(HEADER_NAME) ./$(RELEASE_NAME)/$(RELEASE_NAME).h
	@rm -rf ./$(RELEASE_NAME)/$(BUILD_DIR)

$(MAKE_DIRS):
	@mkdir ./$@

$(OUTPUT_DIR)/%.o: ../$(SRC_DIR)/%.cpp | $(MAKE_DIRS)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf ./$(OUTPUT_DIR)/*
