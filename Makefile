NAME = Famine

OBJ_DIR = obj
SRC_DIR = src
INC_DIR = inc
LIBFT_DIR = libft

BLOCK_SIZE=$(shell stat -fc %s .)

SRCS = \
	main.c

OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.c=.o))

HEADERS = $(wildcard $(INC_DIR)/*.h) $(wildcard $(LIBFT_DIR)/src/**/*.h)

LIBFT_SRCS = $(shell find $(LIBFT_DIR) -name "*.c")
LIBFT_HEADERS = $(shell find $(LIBFT_DIR) -name "*.h")

LIBFT = $(LIBFT_DIR)/libft.a
LIBFT_FLAGS = -L$(LIBFT_DIR) -lft

CC = cc
CFLAGS = -Wall -Wextra -Werror -I$(INC_DIR) -I$(LIBFT_DIR)/inc -DBLOCK_SIZE=$(BLOCK_SIZE)
LDFLAGS = $(LIBFT_FLAGS)

all: $(LIBFT) $(NAME)

$(NAME): $(OBJS) $(LIBFT)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)
	strip $(NAME)

$(LIBFT): $(LIBFT_SRCS)
	$(MAKE) -C $(LIBFT_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.c $(HEADERS) | $(OBJ_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/

clean:
	$(MAKE) -C $(LIBFT_DIR) clean
	rm -rf $(OBJ_DIR)

fclean: clean
	$(MAKE) -C $(LIBFT_DIR) fclean
	rm -f $(NAME) $(PAYLOAD_NAME)

re: fclean all

.PHONY: all clean fclean re
