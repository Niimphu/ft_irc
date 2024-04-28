#include "IRC.hpp"

int	Server::sendMessage(std::string& target, std::string& message, Client& client)
{
	if (target.empty() || message.empty())
	{
		std::cerr << RED << "Invalid command" << RESET << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 1, "PRIVMSG"), client);
		return (1);
	}
	if (target[0] == '#') {
		ChannelMessage(target, message, client);
		return (1);
	}
	Client	*recipient = findClient(target);

	if (!recipient)
	{
		std::cerr << RED << "Invalid target" << RESET << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), 401, "", 1, target.c_str()), client);
		return (1);
	}
	return (sendToClient(buildReply(client.getNickname(), recipient->getNickname(), PRIVMSG, message, 0), *recipient));
}

int	Server::joinChannel(std::string& channelName, std::string& key, Client& client)
{
	if (channelName.empty())
		return (sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 1, "JOIN"), client));
	if (channelName[0] != '#')
		return (sendToClient(buildReply(SERVER, client.getNickname(), 476, "", 0), client));
	Channel*	channel = findChannel(channelName);
	if (!channel) {
		Channel newChannel(channelName, key, this);
		newChannel.join(client, key);
		_channels.push_back(newChannel);
	}
	else
		channel->join(client, key);
	return (0);
}

int	Server::inviteChannel(const std::string& _target, const std::string& _channel, const Client& client)
{
	Channel		*channel = findChannel(_channel);
	std::string	name = client.getNickname();

	if (_target.empty() || _channel.empty())
		return (sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 1, "PRIVMSG"), client));
	if (!channel)
		return(sendToClient(buildReply(SERVER, client.getNickname(), 442, "", 1, _channel.c_str()), client));
	if (!findClient(_target))
		return (sendToClient(buildReply(SERVER, name, 401, "", 1, _target.c_str()), client));
	if (!channel->clientIsInChannel(name))
		return(sendToClient(buildReply(SERVER, client.getNickname(), 441, "", 1, _target.c_str()), client));
	if (channel->clientIsInChannel(_target))
		return(sendToClient(buildReply(SERVER, client.getNickname(), 443, "", 2, _target.c_str(), _channel.c_str()), client));
	if (!channel->clientIsOp(name) && channel->getInviteOnly())
		return(sendToClient(buildReply(SERVER, client.getNickname(), 482, "", 1, _channel.c_str()), client));
	channel->addInvitedClient(_target);
	sendToClient(buildReply(SERVER, client.getNickname(), 341, "", 2, _target.c_str(), _channel.c_str()), client);
	sendToClient(buildReply(SERVER, client.getNickname(), INVITE, "", 2, _target.c_str(), _channel.c_str()), *findClient(_target));
	return (1);
}

int	Server::ChannelMessage(std::string& target, std::string& message, Client& client)
{
	Channel	*channel = findChannel(target);

	if (!channel)
	{
		std::cerr << RED << "Invalid target" << RESET << std::endl;
		sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 1, target.c_str()), client);
		return (1);
	}
	return (channel->clientMessage(buildReply(client.getNickname(), channel->getName(), PRIVMSG, message, 0), client));
}

void	Server::names(Client& client, std::string& channelName)
{
	Channel	*channel = findChannel(channelName);
	if (!channel)
	{
		sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 1, channelName.c_str()), client);
		return;
	}
	sendToClient(buildReply(SERVER, client.getNickname(), 353, "", 3, "=", channelName.c_str(), channel->getClientList().c_str()), client);
	sendToClient(buildReply(SERVER, client.getNickname(), 366, "", 1, channelName.c_str()), client);
}

int	Server::channelTopic(const std::string& channelName,const std::string& newTopic, Client& client)
{
	std::string	name = client.getNickname();
	if (channelName.empty())
		return (sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 1, "PRIVMSG"), client));

	Channel	*channel = findChannel(channelName);
	if (!channel)
		return(sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 0), client));
	channel->topic(newTopic, client);
	return (0);
}

int	Server::mode(const std::string& channelName, const std::string& modeString, const std::string &arg,  Client &client)
{
	Channel*	channel = findChannel(channelName);
	if (!channel)
		return (sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 0), client));
	return (channel->mode(modeString, arg, client));
}

int	Server::kickClient(const std::string& channelName,const std::string& target, const std::string& reason, Client& client)
{
	Channel		*channel = findChannel(channelName);

	if (target.empty() || channelName.empty())
		return (sendToClient(buildReply(SERVER, client.getNickname(), 461, "", 0), client));
	if (!channel)
		return (sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 0), client));
	if (channel->kick(client, target, reason) == 2)
		removeChannel(*channel);
	return (1);
}

int	Server::partChannel(const std::string& channelName, const std::string& reason, Client& client)
{
	Channel	*channel = findChannel(channelName);

	if (!channel)
		return (sendToClient(buildReply(SERVER, client.getNickname(), 403, "", 0), client));
	channel->part(client, reason);
	return (0);
}

int	Server::quit(Client& client, std::string& quitMessage)
{
	std::string	nickname = client.getNickname();
	for (channelIt it = _channels.begin(); it != _channels.end(); ++it) {
		partChannel(it->getName(), "QUITTER", client);
	}
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (*it == client)
		{
			close(client.getSocket());
			_fds.erase(_fds.begin() + std::distance(_clients.begin(), it) + 1);
			_clients.erase(it);
			std::cout << GREEN << nickname << " has quit" << RESET << std::endl;
			break;
		}
	}
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it)
		sendToClient(buildReply(nickname, ":Quit", QUIT, quitMessage, 0), *it);
	return (0);
}
