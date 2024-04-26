#include "Server.hpp"

int	Server::authenticatePassword(Client& client, std::string& inputPassword) {
	if (inputPassword.empty())
		return (sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 1, "PASS"), client));
	if (inputPassword == _password) {
		client.beAuthenticated();
		return (0);
	}
	else
		return (sendToClient(buildReply(SERVER, client.getNickname(), 464, "", 0), client));
}

int	Server::changeNickname(const std::string& nick, Client& client)
{
	if (!client.getNickname().empty())
		return (0);
	if (nick.empty())
	{
		std::cerr << RED << "No nickname given" << RESET << std::endl;
		return (sendToClient(buildReply(SERVER, "*", 431, "", 0), client));
	}
	if (nick[0] == '#' || nick[0] == ':' || nick[0] == ' ')
	{
		std::cerr << RED << "Invalid nickname" << RESET << std::endl;
		return (sendToClient(buildReply(SERVER, "*", 432, "", 1, nick.c_str()), client));
	}
	if (findClient(nick)) {
		std::cerr << RED << "Nickname already in use" << RESET << std::endl;
		sendToClient(buildReply(SERVER, "*", 433, "", 1, nick.c_str()), client);
		return (-1);
	}
	sendToClient(buildReply(client.getNickname(), nick, NICK, "", 0), client);
	client.setNickname(nick);
	registerClient(client);
	return (0);
}

int	Server::setUsername(std::string& user, Client& client)
{
	if (client.isRegistered()) {
		std::cerr << RED << "User already registered" << RESET << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), 462, "", 0), client);
		return (-1);
	}
	if (user.empty())
	{
		std::cerr << RED << "No username given" << RESET << std::endl;
		sendToClient(buildReply(SERVER, "*", 431, "", 0), client);
		return (-1);
	}
	if (usernameIsRegistered(user)) {
		std::cerr << RED << "Username already in use" << RESET << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), NOTICE, ":Username is taken, registering as guest", 1, client.getNickname().c_str()), client);
		std::stringstream	ss;
		ss << ++guestCount;
		user = "Guest" + ss.str();
	}
	client.setUsername(user);
	registerClient(client);
	return (0);
}

void	Server::registerClient(Client& client) const {
	if (!client.isRegistered() && !client.getNickname().empty() && !client.getUsername().empty()) {
		client.beRegistered();
		std::cout << "New user " << client.getNickname() << " has been registered" << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), 001, "", 0), client);
	}
}
