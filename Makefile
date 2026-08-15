NAME = ircserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp server/Server.cpp \
client/Client.cpp client/client-auth.cpp client/client-static.cpp client/client-join.cpp \
client/client-privmsg.cpp client/client-part.cpp client/client-channelops.cpp client/client-mode.cpp \
channel/channel-static.cpp channel/Channel.cpp

OBJS = ${SRC:%.cpp=objects/%.o}

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Compiling..."
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "Build complete!"

objects/%.o: %.cpp
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@$(RM) -rf objects

fclean: clean
	@$(RM) $(NAME)

re: fclean all

##############################################

IRC_PORT = 8080
IRC_IP = localhost
IRC_PASSWORD = zzz

r: run
run: all
	@clear; ./$(NAME) $(IRC_PORT) $(IRC_PASSWORD)

v: valgrind
valgrind: CXXFLAGS += -g3
valgrind: all
	@clear; valgrind --leak-check=full --track-fds=all ./$(NAME) $(IRC_PORT) $(IRC_PASSWORD)

c: client
client:
	@nc -NC $(IRC_IP) $(IRC_PORT) 

##############################################

.PHONY: all clean fclean re r run v valgrind c client
