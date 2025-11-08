NAME = Pestilence

BUILD_DIR = build
SRC_DIR = src
INC_DIR = inc

CC = gcc
ASM = nasm
LD = ld

ASM_FLAGS = -f elf64 -g
LD_FLAGS = -T $(SRC_DIR)/pestilence.ld

STRIP_CMD = strip $(NAME)

ASM_SRCS = $(SRC_DIR)/builder.asm
C_SRCS   = $(SRC_DIR)/scaffolding.c $(SRC_DIR)/inject.c

OBJ_BUILDER = $(BUILD_DIR)/builder.o
BIN_BUILDER = $(BUILD_DIR)/builder
BIN_SCAFFOLD = $(BUILD_DIR)/scaffolding
BIN_INJECT = $(BUILD_DIR)/inject

all: $(NAME)

$(NAME): $(BIN_BUILDER) $(BIN_SCAFFOLD) $(BIN_INJECT)
	$(BIN_INJECT) > /dev/null
	cp $(BIN_SCAFFOLD) $(NAME)

$(OBJ_BUILDER): $(SRC_DIR)/builder.asm | $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) $< -o $@ > /dev/null 2>&1

$(BIN_BUILDER): $(OBJ_BUILDER)
	$(LD) $< -o $@ $(LD_FLAGS) > /dev/null 2>&1

$(BIN_SCAFFOLD): $(SRC_DIR)/scaffolding.c | $(BUILD_DIR)
	$(CC) $< -o $@ -nostartfiles > /dev/null 2>&1

$(BIN_INJECT): $(SRC_DIR)/inject.c | $(BUILD_DIR)
	$(CC) $< -o $@ > /dev/null 2>&1

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: all
	@./scripts/end_to_end.sh

.PHONY: all clean fclean re
