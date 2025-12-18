NAME	=   ircserv

SRCS 	=	main.cpp Server.cpp Client.cpp CmdClient.cpp Channel.cpp

CC		=	c++

CFLAGS	=	-Wall -Wextra -Werror -std=c++98

OBJS	=	$(SRCS:.cpp=.o)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME) 

%.o: %.cpp
	$(CC) $(CFLAGS) -I. -c $< -o $@

all: ${NAME}

clean:
	rm -f ${OBJS}

fclean: clean
	rm -f $(NAME)

re:			fclean all

.PHONY:	all clean fclean re