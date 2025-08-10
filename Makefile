NAME = Famine

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc

BLOCK_SIZE=$(shell stat -fc %s .)

SRCS = \
	infect.asm

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.asm=.o))

ASM = nasm
ASM_FLAGS = -f elf64
DEBUG_FLAGS = -dDEBUG
LD = ld
LD_FLAGS =

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LD_FLAGS)
	strip $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASM_FLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/

debug:
	$(MAKE) --no-print-directory ASM_FLAGS="$(ASM_FLAGS) $(DEBUG_FLAGS)"

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) infect infect.o

re: fclean all

.PHONY: all clean fclean re
