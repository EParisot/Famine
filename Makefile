# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: eparisot <eparisot@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2021/07/25 14:55:42 by eparisot          #+#    #+#              #
#    Updated: 2021/07/25 14:55:42 by eparisot         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	Famine

SRCS	=	srcs/famine.c \
			srcs/elf_tools.c \
			srcs/tools.c \
			srcs/libft.c \
			srcs/syscalls.c

OBJS	=	$(SRCS:.c=.o)

INC		=	includes/famine.h

CC		= 	gcc

CFLAGS	=	-Wall -Wextra -Werror -fPIE -fPIC

RM 		= 	rm -rf

MKDIR_P = 	mkdir -p

all		:	$(NAME)

$(NAME)	:	$(OBJS) $(INC)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	$(MKDIR_P) /tmp/test
	$(MKDIR_P) /tmp/test2
	gcc $(CFLAGS) ressources/sample.c -o ressources/hello
	gcc $(CFLAGS) ressources/sample2.c -o ressources/hello2
	mv ressources/hello /tmp/test/hello
	mv ressources/hello2 /tmp/test/hello2
	cp /usr/bin/ls /tmp/test/ls
	cp /tmp/test/ls /tmp/test2/ls
	cp /tmp/test/hello /tmp/test2/hello
	cp /tmp/test/hello2 /tmp/test2/hello2

.FORCE	:

clean	:
	sudo $(RM) $(OBJS) /tmp/test /tmp/test2

fclean	: clean 
	$(RM) $(NAME)

re		: fclean all

.PHONY	: all re clean fclean