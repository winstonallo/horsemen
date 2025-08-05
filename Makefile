NAME = Famine

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc
LIBFT_DIR = libft

BLOCK_SIZE=$(shell stat -fc %s .)

SRCS = \
	infect.asm

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.asm=.o))

HEADERS = $(wildcard $(INC_DIR)/*.inc) $(wildcard $(LIBFT_DIR)/src/**/*.inc)

ASM = nasm
ASMFLAGS = -f elf64
LD = ld
LDFLAGS = 

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LDFLAGS)
	strip $(NAME)

$(LIBFT): $(LIBFT_SRCS)
	$(MAKE) -C $(LIBFT_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.asm $(HEADERS) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.asm $(HEADERS) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -rf $(OBJ_DIR)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME) infect infect.o

re: fclean all

.PHONY: all clean fclean re