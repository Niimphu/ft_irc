CXX = c++
CXX_FLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -g -I./inc \

SRC_DIR = src
SRC =		main.cpp Parser.cpp \
			Server.cpp ServerRegistration.cpp ServerCommands.cpp \
			Channel.cpp ChannelCommands.cpp ChannelMode.cpp \

OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

NAME = ircserv

PRUNE = git fetch --prune

all:$(NAME)

$(NAME): $(OBJ)
	$(PRUNE)
	@$(CXX) $(CXX_FLAGS) $(OBJ) -o $(NAME)
	@echo "OK"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Working on: $<"
	@mkdir -p $(OBJ_DIR)
	@$(CXX) $(CXX_FLAGS) -c $< -o $@

clean:
	@-rm -rf **/*.o

fclean:	clean
	@-rm -rf $(NAME)

re:		fclean all

.PHONY:	all clean fclean re
