#include "IRC.hpp"
#include "Commands.hpp"

int	Channel::clientMessage(const std::string& message, Client& sender)
{
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it) {
		if (*it != sender)
			_server->sendToClient(message, *it);
	}
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (*it != sender)
			_server->sendToClient(message, *it);
	}
	return (0);
}

int	Channel::channelMessage(const std::string& message)
{
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it)
		_server->sendToClient(message, *it);
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it)
		_server->sendToClient(message, *it);
	return (0);
}

int	Channel::addOperator(Client& client) {
	size_t	limit = getClientLimit();

	if (limit > 0 && _userCount >= limit)
		return (1);
	_operators.push_back(client);
	++_userCount;
	return (0);
}

void	Channel::removeOperator(const std::string& name)
{
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it) {
		if (name == (*it).getNickname())
		{
			_operators.erase(it);
			--_userCount;
			return ;
		}
	}
}

int	Channel::addClient(Client& client)
{
	size_t	limit = getClientLimit();

	if (limit > 0 && _userCount > limit)
		return (1);
	_clients.push_back(client);
	++_userCount;
	return (0);
}

void	Channel::removeClient(const std::string& name)
{
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (name == (*it).getNickname())
		{
			_clients.erase(it);
			--_userCount;
			return ;
		}
	}
}

void	Channel::removeUser(const std::string& name) {
	removeClient(name);
	removeOperator(name);
}

std::string	Channel::getClientList()
{
	std::string	list = ":";
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it) {
		if (!list.empty())
			list += " ";
		list += "@" + it->getNickname();
	}
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (!list.empty())
			list += " ";
		list += it->getNickname();
	}
	return (list);
}

bool	Channel::clientIsOp(const std::string& name)
{
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it) {
		if (name == (*it).getNickname())
			return (true);
	}
	return (false);
}

Client*	Channel::findOps(const std::string& name)
{
	for (size_t i = 0; i < _operators.size(); i++)
	{
		if (_operators[i].getNickname() == name)
			return (&_operators[i]);
	}
	return(NULL);
}

bool	Channel::clientIsInChannel(const std::string& name)
{
	for (clientIt it = _operators.begin(); it != _operators.end(); ++it) {
		if (name == (*it).getNickname())
			return (true);
	}
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (name == (*it).getNickname())
			return (true);
	}
	return (false);
}

bool	Channel::clientIsInvited(const std::string& name)
{
	for (stringIt it = _invitedClients.begin(); it != _invitedClients.end(); ++it) {
		if (name == *it)
			return (true);
	}
	return (false);
}

void	Channel::removeInvitedClient(const std::string& name)
{
	for (stringIt it = _invitedClients.begin(); it != _invitedClients.end(); ++it) {
		if (name == *it)
		{
			_invitedClients.erase(it);
			return ;
		}
	}
}

