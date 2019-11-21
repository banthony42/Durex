#******************************************************************************#
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: banthony <banthony@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/11/06 13:17:44 by banthony          #+#    #+#              #
#    Updated: 2019/11/21 12:22:15 by banthony         ###   ########.fr        #
#                                                                              #
#******************************************************************************#

NAME = Durex

OBJ_PATH = ./obj/

PATH_SRC = ./src/

PATH_HEAD = ./include/

SRC_FILE +=	main.c
SRC_FILE +=	daemon.c
SRC_FILE +=	durex_log.c
SRC_FILE +=	server.c
SRC_FILE +=	server_command.c
SRC_FILE +=	install_service.c

SRC_FILE +=	md5_digest.c
SRC_FILE +=	md5_function.c

SRC = $(SRC_FILE:%c=$(PATH_SRC)%c)

OBJ = $(SRC_FILE:.c=.o)
OBJ2 = $(OBJ:%.o=$(OBJ_PATH)%.o)

INCLUDE = $($(PAHT_HEAD)%h)

# Libft path and flags
LIBFT = ./libft
HEAD_LIBFT = -I $(LIBFT)
LIBFT_NAME = -L $(LIBFT) -lft
LIBFT_NAME_SANIT = -L $(LIBFT) -lft_sanit

# preprocessor macro at compile time
DEFINE =

# 1 : Durex will use syslog, to log info into file.
# 0 : Durex will use homemade tool to log info into file.
USE_SYSLOG = 0

ifeq ($(USE_SYSLOG), 1)
DEFINE += -D USE_SYSLOG
endif

OTHER_LIB =

HEAD_DIR = -I $(PATH_HEAD) $(HEAD_LIBFT)
FLAGS = -Wall -Wextra -Werror

DEBUG = -g3 -fsanitize=address

TRASH = Makefile~		\
		./src/*.c~		\
		./include/*.h~	\

all: $(NAME)
	@echo "\033[32m•\033[0m $(NAME) ready"
	@echo $(INCLUDE)

$(NAME): $(SRC) $(INCLUDE)
	make -C $(LIBFT) sanit
	gcc $(FLAGS) $(HEAD_DIR) $(OTHER_LIB) -c $(SRC) $(DEBUG) $(DEFINE)
	mkdir -p $(OBJ_PATH)
	mv $(OBJ) $(OBJ_PATH)
	gcc $(FLAGS) -o $(NAME) $(OBJ2) $(HEAD_DIR) $(LIBFT_NAME_SANIT) $(OTHER_LIB) $(DEBUG) $(DEFINE)

normal: $(SRC) $(INCLUDE)
	make -C $(LIBFT)
	gcc $(FLAGS) $(HEAD_DIR) $(OTHER_LIB) -c $(SRC) $(DEFINE)
	mkdir -p $(OBJ_PATH)
	mv $(OBJ) $(OBJ_PATH)
	gcc $(FLAGS) $(OBJ2) $(HEAD_DIR) $(LIBFT_NAME) -o $(NAME) $(OTHER_LIB) $(DEFINE)
	@echo "\033[32m•\033[0m $(NAME) ready

clean:
	make clean -C $(LIBFT)
	rm -rf $(OBJ_PATH) $(TRASH)

fclean: clean
	make fclean -C $(LIBFT)
	rm -f $(NAME)
	-rm $(OBJ)

re: fclean all
