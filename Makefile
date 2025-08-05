NAME = Famine

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc

BLOCK_SIZE=$(shell stat -fc %s .)

SRCS = \
	infect.asm

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.asm=.o))

ASM = nasm
ASMFLAGS = -f elf64
LD = ld
LDFLAGS = 

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LDFLAGS)
	strip $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.asm | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME) infect infect.o

re: fclean all

.PHONY: all clean fclean re