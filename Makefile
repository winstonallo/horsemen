NAME = Famine

BUILD_DIR = build
SRC_DIR = src
INC_DIR = inc

SRCS = \
	builder.asm


STRIP_CMD = strip $(NAME)

all: $(NAME)


LD = ld
LD_FLAGS = -T $(SRC_DIR)/famine.ld

$(NAME): $(OBJS) builder scaffolding inject patient_zero
	./$(BUILD_DIR)/inject
	touch 4242
	rm -rf /tmp/test/*
	mv $(NAME) /tmp/test/
	./$(BUILD_DIR)/scaffolding
	rm 4242
	mv /tmp/test/Famine ./
	

ASM = nasm
ASM_FLAGS = -f elf64 -g

builder: $(BUILD_DIR)
	$(ASM) $(ASM_FLAGS) src/builder.asm -o $(BUILD_DIR)/builder.o
	$(LD) $(BUILD_DIR)/builder.o -o $(BUILD_DIR)/builder $(LD_FLAGS)

scaffolding: src/scaffolding.c
	cc src/scaffolding.c  -o $(BUILD_DIR)/scaffolding -nostartfiles
	
inject: src/inject.c
	cc src/inject.c -o $(BUILD_DIR)/inject

patient_zero: src/inject.c src/scaffolding.c
	rm -f $(NAME)
	cc src/patient_zero.c -o $(NAME)

$(BUILD_DIR): | $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/

clean:
	rm -rf $(BUILD_DIR)

fclean: clean
	rm -f ./Famine

re: fclean all

.PHONY: all debug clean fclean re
