#include "Server.hpp"
#include "Parser.hpp"
#include "Client.hpp"

int	Server::authenticatePassword(Client& client, std::string& inputPassword) {
	if (inputPassword.empty()) {
			sendToClient(":ft_irc 461 * :No password provided" + END, client);
			return -1;
		}
	if (inputPassword == _password) {
		client.beAuthenticated();
		return 0;
	}
	else {
		sendToClient(":ft_irc 464 * :Password is incorrect" + END, client);
		return -1;
	}
}

int	Server::changeNickname(const std::string& nick, Client &client)
{
	if (nick.empty())
	{
		std::cerr << RED << "No nickname given" << RESET << std::endl;
		sendToClient(":ft_irc 431 * :No nickname given" + END, client);
		return (-1);
	}
	if (nick[0] == '#' || nick[0] == ':' || nick[0] == ' ')
	{
		std::cerr << RED << "Invalid nickname" << RESET << std::endl;
		sendToClient(":ft_irc 432" + (client.getNickname().empty() ? "*" : nick) + " :Erroneous nickname" + END, client);
		return (-1);
	}
	if (getClient(nick)) {
		std::cerr << RED << "Nickname already in use" << RESET << std::endl;
		sendToClient(":ft_irc 433 " + (client.getNickname().empty() ? "*" : nick) + " :Nickname is already in use" + END, client);
		return (-1);
	}
	client.setNickname(nick);
	sendToClient(":ft_irc NICK " + nick + END, client);
	registerClient(client);
	return (0);
}

int	Server::setUsername(std::string& user, Client &client)
{
	if (client.isRegistered()) {
		std::cerr << RED << "User already registered" << RESET << std::endl;
		sendToClient(":ft_irc 462 :User is already registered" + END, client);
		return (-1);
	}
	if (user.empty())
	{
		std::cerr << RED << "No username given" << RESET << std::endl;
		sendToClient(":ft_irc 431 " + (client.getNickname().empty() ? "*" : client.getUsername()) + " :No username given" + END, client);
		return (-1);
	}
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (user == _clients[i].getUsername())
		{
			std::cerr << RED << "Username already in use" << RESET << std::endl;
			sendToClient(":ft_irc NOTICE " + (client.getNickname().empty() ? "*" : client.getNickname()) + " :Username is taken, registering as guest" + END, client);
			std::stringstream	ss;
			ss << ++guestCount;
			user = "Guest" + ss.str();
		}
	}
	client.setUsername(user);
	registerClient(client);
	return (0);
}

void	Server::registerClient(Client &client) const {
	if (!client.isRegistered() && !client.getNickname().empty() && !client.getUsername().empty()) {
		client.beRegistered();
		std::cout << "New user registered: Nickname: " << client.getNickname() << " Username: " << client.getUsername() << std::endl;
		sendToClient(":ft_irc 001 " + client.getNickname() + " :Welcome to our ft_irc server, " + client.getNickname() + END, client);
	}
}

int Server::ChannelMessage(std::string& target, std::string& message, std::string& receivedString, Client &client)
{
	if (target.empty() || message.empty())
	{
		std::cerr << RED << "Invalid command" << RESET << std::endl;
		sendToClient(":ft_irc 461 * :Not enough parameters" + END, client);
		return (1);
	}
	
	Channel	*channel = getChannel(target);
	if (!channel)
	{
		std::cerr << RED << "Invalid target" << RESET << std::endl;
		sendToClient(":ft_irc 401 * :No such channel" + END, client);
		return (1);
	}
	sendToChannel(":" + client.getNickname() + " " + receivedString + END, *channel, client);
	return (0);
}

int Server::message(std::string& target, std::string& message, std::string& receivedString, Client &client)
{
	if (target.empty() || message.empty())
	{
		std::cerr << RED << "Invalid command" << RESET << std::endl;
		sendToClient(":ft_irc 461 * :Not enough parameters" + END, client);
		return 1;
	}

	if (target[0] == '#') {
		ChannelMessage(target, message, receivedString, client);
		return 1;
	}

	Client	*recipient = getClient(target);
	if (!recipient)
	{
		std::cerr << RED << "Invalid target" << RESET << std::endl;
		sendToClient(":ft_irc 401 * :No such user" + END, client);
		return 1;
	}
	sendToClient(":" + client.getNickname() + " " + receivedString + END, *recipient);
	return 0;
}

void Server::responseForClientJoiningChannel(Client &client, Channel &channel)
{
	sendToClient(":" + client.getNickname() + " JOIN " + channel.getName() + END, client);
	sendToClient(":ft_irc 332 " + client.getNickname() + " " + channel.getName() + " :" + channel.getTopic() + END, client);
}

void	Server::names(Client& client, std::string& channelName)
{
	Channel	*channel = getChannel(channelName);
	if (!channel)
	{
		sendToClient(":ft_irc 401 * :No such channel" + END, client);
		return;
	}
	std::string	list = channel->getClientList();
	sendToClient(":ft_irc 353 " + client.getNickname() + " = " + channelName + " :" + channel->getClientList() + END, client);
	sendToClient(":ft_irc 366 " + client.getNickname() + " " + channelName + " :End of /NAMES list" + END, client);
//	sendToClient(":ft_irc 352 " + client.getNickname() + " " + channel->getName() + " " + list + END, client);
//	sendToClient(":ft_irc 315 " + client.getNickname() + " " + channel->getName() + " :End of /WHO list" + END, client);
}

int Server::joinChannel(std::string& channelName, std::string& key, Client &client)
{
	std::cout << "what is input " << channelName << " ." << std::endl;
	if (channelName.empty())
	{
		std::cout << "what is input " << client.getNickname() << " ." << std::endl;

		sendToClient(":ft_irc 461" + client.getNickname() + " JOIN " + ":Not enough parameters" + END, client);
		return (1);
	}
	if (channelName[0] != '#')
	{
		sendToClient(":ft_irc 476 " + channelName + " :Bad Channel Mask, Put '#' Before Channel Name" + END, client);
		return (1);
	}

	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (channelName == _channels[i].getName())
		{
			_channels[i].addClient(client);
			responseForClientJoiningChannel(client, _channels[i]);
			sendToChannel(":" + client.getNickname() + " JOIN " + _channels[i].getName() + END, _channels[i], client);
			return (0);
		}
	}
	Channel newChannel(channelName, key); //if no key is supplied, key is set to ""
	newChannel.addOperator(client);

	addChannel(newChannel);

	std::cout << "hiello?" << std::endl;
	responseForClientJoiningChannel(client, newChannel);

	return (0);
}

int Server::quit(Client &client, std::string& quitMessage)
{
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (*it == client)
		{
			//part from all channels

			close(client.getSocket());
			_fds.erase(_fds.begin() + std::distance(_clients.begin(), it) + 1);
			_clients.erase(it);
			std::cout << "Successful quit" << std::endl;
			break;
		}
	}
	for (size_t i = 0; i < _clients.size(); ++i) {
		sendToClient(":" + client.getNickname() + " QUIT :Quit " + quitMessage + END, _clients[i]);
	}
	return (0);
}

int isClientConnectedToChannel(Client &client, Channel &channel)
{
	for (size_t j = 0; j < channel.getClients().size(); j++)
    {
        if (channel.getClients()[j].getNickname() == client.getNickname())
            return (1);
    }	
    return (0);
}

int Server::cmdTopic(Client &client, std::string& channel, std::string& newTopic)
{	
	std::time_t currentTime = std::time(NULL);
	std::string timestampStr = std::ctime(&currentTime);

	std::cout << "curr time is: " << timestampStr << std::endl;
	if (channel.empty())
	{
		sendToClient(":ft_irc 461" + client.getNickname() + " TOPIC " + ":Not enough parameters" + END, client);
		return (1);
	}	
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (channel == _channels[i].getName())
		{
			if (!isClientConnectedToChannel(client, _channels[i]))
			{
				sendToClient(":ft_irc 442" + client.getNickname() + " " + _channels[i].getName() + " :You're not on that channel" + END, client);
				return (1);
			}
			else if (newTopic.empty())
			{
				sendToClient(":ft_irc 332 " + client.getNickname() + " " + _channels[i].getName() + " :" + _channels[i].getTopic() + END, client);
				return (1);
			}
			else
			{
				_channels[i].setTopic(newTopic);
				std::cout << "so now the topic is " << _channels[i].getTopic() << std::endl;
				for (size_t j = 0; j < _clients.size(); j++)
				{
					sendToClient(":ft_irc 333 " + _clients[j].getNickname() + " " + _channels[i].getName() + " " + client.getNickname() + " " + timestampStr + END, _clients[j]);
					sendToChannel(":ft_irc 332 " + _clients[j].getNickname() + " " + _channels[i].getName() + " " + _channels[i].getTopic() + END, _channels[i], _clients[j]);
				}
			}
		}
		else
		{
			sendToClient(":ft_irc 403" + client.getNickname() + " " + channel + " :No such channel" + END, client);
			return (1);
		}
	}
	return (0);
}

void Server::printClients(void)
{
	std::cout << "Total Clients:" << _clients.size() << std::endl;
	for (size_t i = 0; i < _clients.size(); i++)
	{
		std::cout << "\tClient " << i << ": " << _clients[i].getNickname() << " Username : " << _clients[i].getUsername() << "on Socket :" << _clients[i].getSocket() << std::endl;
	}
}

int Server::kickClient(const std::string &_channel,const std::string &_target, Client &client)
{
	//check if client is operator;
	Client *target = getClient(_target);
	if (target == NULL)
	{
		sendToClient(":ft_irc 441 " + client.getNickname() + " " + _target + " " + _channel + " " + ":They aren`t on that Channel" + END, client);
		return (0);
	}
	Channel *channel = getChannel(_channel);
	if (channel == NULL)
	{
		sendToClient(":ft_irc 401 * :No such channel" + END, client);
		return (0);
	}
	std::cout << "Channel : " << channel->getName() << std::endl;
	sendToClient(":" + client.getNickname() + " KICK " + _channel + " " + _target + END, client);
	sendToChannel(":" + client.getNickname() + " KICK " + _channel + " " + _target + END, *channel , client);
	channel->removeClient(_target);
	std::cout << "target : " << target->getNickname() << std::endl;
	std::cout << client.getNickname() << " kicked " << _target << " from " << _channel << std::endl;
	return (1);
}
