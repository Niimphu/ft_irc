#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include "IRC.hpp"

# define PASS 100
# define NICK 101
# define USER 102
# define PRIVMSG 103
# define JOIN 104
# define WHO 105
# define TOPIC 106
# define MODE 107
# define KICK 108
# define PART 109
# define INVITE 110
# define NOTICE 111
# define QUIT 169

std::string	buildReply(const std::string& sender, const std::string& recipient, int messageCode, const std::string& message, int paramCount ...);

#endif
