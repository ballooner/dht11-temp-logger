CC := gcc
C_FLAGS := -Wall
LIBS := -llgpio -lm

logger: main.c
	$(CC) $(C_FLAGS) main.c -o logger $(LIBS)
