NAME = codexion

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread

SRC = \
	coders/main.c \
	coders/args.c \
	coders/time_utils.c \
	coders/logging.c \
	coders/stop.c \
	coders/heap_base.c \
	coders/heap.c \
	coders/dongles_take.c \
	coders/dongles_release.c \
	coders/coder_state.c \
	coders/coder_routine.c \
	coders/monitor.c \
	coders/simulation_init.c \
	coders/simulation_run.c \
	coders/simulation_destroy.c

OBJ = $(SRC:.c=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re