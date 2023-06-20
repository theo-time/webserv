# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: teliet <teliet@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/09/29 20:41:39 by teliet            #+#    #+#              #
#    Updated: 2023/06/13 16:35:11 by teliet           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv 

SRC = main.cpp Request.cpp 

OBJ = $(SRC:.cpp=.o)

CC = c++

HEADERS = -I ./

FLAGS = -Wall -Wextra -Werror -std=c++98


all: ${NAME}
 
$(NAME): $(OBJ) 
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)
	
debug: $(LIBS) clean
	$c(CC) $(SRC) $(HEADERS) -o $(NAME) 

%.o: %.cpp
	$(CC) $(HEADERS)  $(FLAGS) -c $< -o $@

clean:
	/bin/rm -f ${OBJ}

fclean: clean
	/bin/rm -f ${NAME}

re: fclean all