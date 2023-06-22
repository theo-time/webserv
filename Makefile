# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: adcarnec <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/06/13 14:36:41 by adcarnec          #+#    #+#              #
#    Updated: 2023/06/13 14:36:46 by adcarnec         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	webserv
CC			=	c++
FLAGS		=	-Wall -Wextra -Werror -std=c++98 -g3
SRC			=	./src/main.cpp ./src/Config.cpp ./src/WebServ.cpp \
				./src/VirtualServer.cpp ./src/ClientCnx.cpp
OBJ			=	$(SRC:.cpp=.o)
INC			=	./inc/*.hpp
INC_PATH	=	./inc/

all: $(NAME)

$(NAME): $(OBJ) $(INC)
	$(CC) $(FLAGS) -I$(INC_PATH) $(OBJ) -o $(NAME)

%.o: %.cpp $(INC)
	$(CC) $(FLAGS) -I$(INC_PATH) -o $@ -c $<

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: re fclean clean all
