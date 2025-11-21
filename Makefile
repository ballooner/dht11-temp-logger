CC := gcc
C_FLAGS := -Wall
LINK_LIBS := lgpio

logger: main.c
	$(CC) $(C_FLAGS) main.c -o logger -l$(LINK_LIBS)
