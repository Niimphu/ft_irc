#include "IRC.hpp"

int	Channel::join(Client& client, const std::string& key) {
	if(_clientLimit > 0 && _userCount >= _clientLimit)
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 471, "", 1, _name.c_str()), client));
	if (_isInviteOnly && clientIsInvited(client.getNickname()))
		removeInvitedClient(client.getNickname());
	else if (_isInviteOnly)
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 473, "", 1, _name.c_str()), client));
	else if (!_key.empty() && key != _key)
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 475, "", 1, _name.c_str()), client));

	if (getClientList().size() == 1)
		addOperator(client);
	else if (addClient(client)) {
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 471, "", 1, _name.c_str()), client));
	}
	channelMessage(buildReply(client.getNickname(), _name, JOIN, "", 0));
	_server->sendToClient(buildReply(SERVER, client.getNickname(), 332, _topic, 1, _name.c_str()), client);
	modeGet(client);
	return (0);
}

int	Channel::part(Client& client, const std::string& reason)
{
	std::string	name = client.getNickname();

	if (!clientIsInChannel(name) && reason != "QUITTER")
		return _server->sendToClient(buildReply(SERVER, client.getNickname(), 442, "", 0), client);
	if (reason != "QUITTER")
		channelMessage(buildReply(client.getNickname(), _name, PART, reason, 0));
	removeUser(name);
	return (0);
}

int	Channel::kick(Client& kicker, const std::string& user, const std::string& reason) {
	if (!clientIsOp(kicker.getNickname()))
		return(_server->sendToClient(buildReply(SERVER, kicker.getNickname(), 482, "", 1, _name.c_str()), kicker));
	if (!clientIsInChannel(user))
		return (_server->sendToClient(buildReply(SERVER, kicker.getNickname(), 441, "", 2, user.c_str(), _name.c_str()), kicker));
	channelMessage(buildReply(SERVER, _name, KICK, reason, 1, user.c_str()));
	removeUser(user);
	if (_userCount == 0)
		return (2);
	return (0);
}

int	Channel::topic(const std::string& newTopic, Client& client)
{
	std::string	name = client.getNickname();
	if (!clientIsInChannel(name))
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 442, "", 1, _name.c_str()), client));
	if (newTopic.empty())
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 332, "", 2, _name.c_str(), getTopic().c_str()), client));

	if (clientIsOp(name) || !_restrictTopic)
	{
		setTopic(newTopic);
		std::time_t currTime = std::time(NULL);
		std::stringstream notSS;
		notSS << currTime;
		std::string currTimestamp = notSS.str();
		channelMessage(buildReply(SERVER, name, 332, "", 2, _name.c_str(), _topic.c_str()));
		channelMessage(buildReply(SERVER, name, 333, "", 3, _name.c_str(), name.c_str(), currTimestamp.c_str()));
	}
	else
		return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 482, "", 1, _name.c_str()), client));
	return (0);
}
