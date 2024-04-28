#ifndef IRC_HPP
# define IRC_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <netdb.h>
# include <algorithm>
# include <exception>
# include <fcntl.h>
# include <vector>
# include <map>
# include <netinet/in.h>
# include <csignal>
# include <poll.h>
# include <sstream>
# include <cstdlib>
# include <ctime>
# include <sstream>
# include <iomanip>
# include <cstdarg>
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"
# include "Parser.hpp"
# include "Commands.hpp"

typedef std::vector<std::string>::iterator	stringIt;
typedef std::vector<Client>::iterator		clientIt;
typedef std::vector<Channel>::iterator		channelIt;


# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define RESET "\033[0m"

# define BUFFER_SIZE 1024
# define DEBUG 1

# define ERR_NONICKNAMEGIVEN 431
# define ERR_ERRONEUSNICKNAME 432
# define ERR_NICKNAMEINUSE 433
# define ERR_NEEDMOREPARAMS 461

const	std::string	SERVER = "ft_irc";
const	std::string	END = "\r\n";

inline std::map<int, std::string> generateNumericReplies() {
	std::map<int, std::string> map;
	map[001] = ":Welcome to our ft_irc server, ";
	map[324] = ":Welcome to our ft_irc server, ";
	map[366] = ":End of /NAMES list";
	map[401] = ":No such nick/channel";
	map[403] = ":No such channel";
	map[404] = ":Cannot send to channel";
	map[431] = ":Not enough parameters";
	map[432] = ":Erroneous nickname";
	map[433] = ":Nickname already in use";
	map[441] = ":User is not on that channel";
	map[442] = ":You're not on that channel";
	map[443] = ":is already on channel";
	map[461] = ":Not enough parameters";
	map[462] = ":User is already registered";
	map[464] = ":Password is incorrect";
	map[471] = ":Cannot join channel (+l)";
	map[473] = ":You're not invited, get out! ";
	map[475] = "::Cannot join channel (+k)";
	map[476] = ":Bad channel mask";
	map[482] = ":You're not channel operator";
	return map;
}
const std::map<int, std::string> NUMERIC_REPLIES = generateNumericReplies();


#endif
