CC = cc
CFLAGS = -Wall -Wextra -Werror

NAME = codexion

FILES = src/main.c src/parser.c src/simulation_routine.c src/create_resources.c \
		src/simulation_utiles.c src/codexion_utiles.c src/monitor.c src/dongle_utiles.c \
		src/dongle_take.c

OBJS = $(FILES:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -pthread -o $(NAME)

clean:
	rm -f $(OBJS) a.out

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re


.SECONDARY: