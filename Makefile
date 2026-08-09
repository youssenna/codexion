CC = cc
CFLAGS = -Wall -Wextra -Werror -fsanitize=thread -g

NAME = codexion

FILES = main.c parser.c simulation_routine.c create_resources.c \
		simulation_utiles.c codexion_utiles.c monitor.c dongle_utiles.c \
		dongle_take.c

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
