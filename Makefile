#******************************************************************************#
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: banthony <banthony@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/06 13:17:44 by banthony          #+#    #+#              #
#    Updated: 2019/11/06 16:46:37 by banthony         ###   ########.fr        #
#                                                                              #
#******************************************************************************#

NAME = Durex

OBJ_PATH = ./obj/

PATH_SRC = ./src/

PATH_HEAD = ./include/

SRC_FILE +=	main.c
SRC_FILE +=	daemon.c
SRC_FILE +=	server.c
SRC_FILE +=	install_service.c

SRC = $(SRC_FILE:%c=$(PATH_SRC)%c)

OBJ = $(SRC_FILE:.c=.o)
OBJ2 = $(OBJ:%.o=$(OBJ_PATH)%.o)

UNAME := $(shell uname)

WITH_LIBFT = 0
ifneq ($(WITH_LIBFT), 0)
LIBFT = ./libft
HEAD_LIBFT = -I $(LIBFT)
LIBFT_NAME = -L $(LIBFT) -lft
LIBFT_NAME_SANIT = -L $(LIBFT) -lft_sanit
else
LIBFT =
HEAD_LIBFT =
LIBFT_NAME =
LIBFT_NAME_SANIT =
endif

ifeq ($(UNAME), Linux)
LIB =
HEAD_DIR = -I $(PATH_HEAD) $(HEAD_LIBFT)
FLAGS = -Wall -Wextra -Werror
endif

ifeq ($(UNAME), CYGWIN_NT-6.1)
LIB =
HEAD_DIR = -I $(PATH_HEAD) $(HEAD_LIBFT)
FLAGS = -Wall -Wextra -Werror
endif

ifeq ($(UNAME), Darwin)
LIB =
HEAD_DIR = -I $(PATH_HEAD) $(HEAD_LIBFT)
FLAGS = -Wall -Wextra -Werror -Weverything
endif

DEBUG = -g3 -fsanitize=address

TRASH = Makefile~		\
		./src/*.c~		\
		./include/*.h~	\
		./durex.lock	\

all: $(NAME)

$(NAME): $(SRC) $(INCLUDE)
ifneq ($(WITH_LIBFT), 0)
	make -C $(LIBFT) sanit
endif
	gcc $(FLAGS) $(HEAD_DIR) -c $(SRC) $(DEBUG)
	mkdir -p $(OBJ_PATH)
	mv $(OBJ) $(OBJ_PATH)
	gcc $(FLAGS) -o $(NAME) $(OBJ2) $(HEAD_DIR) $(LIBFT_NAME_SANIT) $(LIB) $(DEBUG)

normal: $(SRC) $(INCLUDE)
ifneq ($(WITH_LIBFT), 0)
	make -C $(LIBFT)
endif
	gcc $(FLAGS) $(HEAD_DIR) $(LIB) -c $(SRC)
	mkdir -p $(OBJ_PATH)
	mv $(OBJ) $(OBJ_PATH)
	gcc $(FLAGS) $(OBJ2) $(HEAD_DIR) $(LIBFT_NAME) -o $(NAME) $(LIB)

clean:
ifneq ($(WITH_LIBFT), 0)
	make clean -C $(LIBFT)
endif
	rm -rf $(OBJ_PATH) $(TRASH)

fclean: clean
ifneq ($(WITH_LIBFT), 0)
	make fclean -C $(LIBFT)
endif
	rm -f $(NAME)
	-rm $(OBJ)

re: fclean all
