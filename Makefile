# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: jde-la-f <jde-la-f@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2023/06/13 14:36:41 by adcarnec          #+#    #+#              #
#    Updated: 2023/07/24 16:46:51 by jde-la-f         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME		=	webserv
CC			=	c++
FLAGS		=	-Wall -Wextra -Werror -std=c++98 -g3
SRC			=	./src/main.cpp ./src/Config.cpp ./src/WebServ.cpp \
				./src/VirtualServer.cpp ./src/Request.cpp ./src/Location.cpp ./src/Response.cpp \
				./src/CGI.cpp ./src/Utils.cpp
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
