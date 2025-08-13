NAME = Famine

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc

SRCS = \
	infect.asm

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.asm=.o))

ASM = nasm
ASM_FLAGS = -f elf64
DEBUG_FLAGS = -dDEBUG
LD = ld
LD_FLAGS =

STRIP_CMD = strip $(NAME)

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LD_FLAGS)
	$(STRIP_CMD)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/

debug:
	$(MAKE) --no-print-directory ASM_FLAGS="$(ASM_FLAGS) $(DEBUG_FLAGS)" NAME="$(NAME)_debug" STRIP_CMD="@echo -n ''" clean all

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) $(NAME)_debug infect infect.o

re: fclean all

.PHONY: all debug clean fclean re

# clear && gcc main.c -o sample && clear && rm builder.o; clear && nasm -f elf64 -g builder.asm -o builder.o && ld -Tfamine.ld  -o builder builder.o && clear && ./builder; echo "exit code:" $? ; readelf -S builder