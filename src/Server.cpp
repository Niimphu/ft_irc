#include "Server.hpp"
#include "Client.hpp"
#include "Commands.hpp"

Server::Server(const std::string& port, const std::string& password): _port(port), _password(password)
{
	_running = false;
	guestCount = 0;
	memset(&_hints, 0, sizeof(_hints));
}

Server::~Server()
{
	closeSockets();
	free(_serverInfo);
	std::cout << "Shutting down" << std::endl;
}

void	Server::start()
{
	std::cout << GREEN << "Server starting" << RESET << std::endl;
	createServerSocket();
	_running = true;
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);

	while (_running)
	{
		if (poll(&_fds[0], _fds.size(), -1) == -1 && _running)
			std::cerr << RED << "Error polling" << RESET << std::endl;
		else
		{
			if (_fds[0].revents & POLLIN)
				acceptSocket();
			for (size_t i = 1; i < _fds.size(); i++)
			{
				if (_fds[i].revents & POLLIN)
					receiveFromClient(_clients[i - 1]);
			}
		}
	}
}

int	Server::createServerSocket()
{
	int					i = 1;
	struct pollfd		fds;

	setHints();
	_socket = socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol);
	if (_socket < 0)
		throw std::runtime_error("Error creating socket");
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(int)) < 0)
		throw std::runtime_error("Error setting socket options");
	if (fcntl(_socket, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("Error setting socket to non-blocking");
	if (bind(_socket, _serverInfo->ai_addr, _serverInfo->ai_addrlen) < 0)
		throw std::runtime_error("Error binding socket");
	if (listen(_socket, SOMAXCONN) < 0)
		throw std::runtime_error("Error listening socket");
	fds.fd = _socket;
	fds.events = POLLIN;
	fds.revents = 0;
	_fds.push_back(fds);
	return (0);
}

void	Server::setHints()
{
	_hints.ai_family = AF_INET;							// Ipv4
	_hints.ai_socktype = SOCK_STREAM;					// Use TCP stream sockets
	_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;		// Bind to any suitable address
	if (getaddrinfo(NULL, _port.c_str(), &_hints, &_serverInfo) < 0) {
		throw std::runtime_error("Error retrieving address information");
	}
}

int Server::acceptSocket()
{
	struct sockaddr_in	addr;
	socklen_t			addrLength = sizeof(addr);
	int clientSocket = accept(_socket, (struct sockaddr *)&addr, &addrLength);
	if (clientSocket < 0) {
		std::cerr << RED << "Error accepting client" << RESET << std::endl;
		return (1);
	}
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << RED << "Error setting client socket to non-blocking" << RESET << std::endl;
		return (1);
	}

	struct pollfd	newClientFD;
	newClientFD.fd = clientSocket;
	newClientFD.events = POLLIN;
	newClientFD.revents = 0;
	Client	newClient(clientSocket);

	_clients.push_back(newClient);
	_fds.push_back(newClientFD);
	return (0);
}

int	Server::sendToClient(const std::string& message, const Client& client) const
{
	if (send(client.getSocket(), message.c_str(), message.length(), 0) < 0)
	{
		std::cerr << RED << "Error sending message" << RESET << std::endl;
		return (-1);
	}
	std::cout << GREEN << "Sent: " << message << " to " << client.getNickname() << RESET << std::endl;
	return (0);
}

int	Server::receiveFromClient(Client& sender)
{
	char	buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	int		bytes = recv(sender.getSocket(), buffer, BUFFER_SIZE, 0);
	if (bytes > 0)
	{
		std::cout << GREEN << "Received: " << buffer << " from " << sender.getNickname() << RESET << std::endl;
		buffer[bytes] = '\0';
		std::string	bufferStr(buffer);
		parseCommand(bufferStr, sender);
		return (bytes);
	}
	else {
		std::cout << RED << "Client disconnected" << RESET << std::endl;
		std::string	message = "User has disconnected";
		quit(sender, message);
		clearEmptyChannels();
	}
	return (0);
}

void	Server::closeSockets()
{
	for (size_t i = 0; i < _clients.size(); i++)
		close(_clients[i].getSocket());
	if (_socket != -1)
		_socket = close(_socket);
}

Client*	Server::findClient(const std::string& nick)
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (nick == _clients[i].getNickname())
			return (&_clients[i]);
	}
	return (NULL);
}

Channel*	Server::findChannel(const std::string& name)
{
	for (size_t i = 0; i < _channels.size(); i++)
	{
		if (name == _channels[i].getName())
			return (&_channels[i]);
	}
	return (NULL);
}

int	Server::removeChannel(Channel& channel)
{
	for (channelIt it = _channels.begin(); it != _channels.end(); ++it) {
		if (*it == channel) {
			_channels.erase(it);
			return (0);
		}
	}
	return (1);
}

int	Server::clearEmptyChannels()
{
	for (channelIt it = _channels.begin(); it != _channels.end(); ++it) {
		if (it->getUserCount() == 0) {
			_channels.erase(it--);
		}
	}
	return (1);
}

Client*	Server::usernameIsRegistered(const std::string& username)
{
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (username == it->getUsername())
			return &(*it);
	}
	return NULL;
}

bool	Server::_running = false;
void	Server::signalHandler(int signum)
{
	std::cout << YELLOW <<"Signal received" << signum << RESET << std::endl;
	_running = false;
}
