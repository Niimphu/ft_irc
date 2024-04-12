#include "Server.hpp"
#include "Client.hpp"
#define DEBUG 1

Server::Server(const std::string& port, const std::string& password): _port(port), _password(password)
{
	_running = false;
	memset(&_hints, 0, sizeof(_hints));
}

Server::~Server()
{
	closeSocket();
	std::cout << "Shutting down" << std::endl;
}

void Server::start()
{
	std::cout << GREEN << "Server starting" << RESET << std::endl;
	createSocket();
	_running = true;
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);

	if (DEBUG)
	{
		std::cout << GREEN << "Server started, on socket " << _socket << RESET << std::endl;
		std::cout <<  GREEN "\tListening on port " << _port << RESET <<  std::endl;
	}
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

//creates a socket
// setsockopt is used to set the socket options, in this case we set the socket to reuse the address
//fcntl is used to set the socket to non-blocking, calls like recv() will suspend the program until data is received, non-blocking will return immediately if no data is available
//binds the socket to the address and port
//listens to the socket
//SOMAXCONN = maximum number of connections(128)
//fds = pollfd struct that will store the socket and events
//fds.fd = socket
//fds.events = POLLIN = data can be read
//fds.revents = 0 = no r resent events
//_fd.push_back(fds) = adds the fds struct to the vector
int Server::createSocket()
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

void	Server::setHints(void)
{
	_hints.ai_family = AF_INET;                       // Ipv4
	_hints.ai_socktype = SOCK_STREAM;                 // Use TCP stream sockets
	_hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;     // Bind to any suitable address
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
	if (DEBUG)
		std::cout << GREEN << "Client connected from " << newClient.getSocket() << RESET << std::endl;
	return (0);
}

//sends a message to the client
int Server::sendToClient(std::string message, Client &client)
{
	if (send(client.getSocket(), message.c_str(), message.length(), 0) < 0)
	{
		std::cerr << RED << "Error sending message" << RESET << std::endl;
		return (-1);
	}
	else
		if (DEBUG)
			std::cout << GREEN << "Sent: " << message << " to socket " << client.getSocket() << RESET << std::endl;
	return (0);
}

int Server::receiveFromClient(Client &sender)
{
	char	buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	int		bytes = recv(sender.getSocket(), buffer, BUFFER_SIZE, 0);
	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		std::string	bufferStr(buffer);
		if (DEBUG)
			std::cout << GREEN << "Received: " << bufferStr << RESET << std::endl;
		if (!sender.isAuthenticated()) {
			authenticateClient(sender, bufferStr);
			return bytes;
		}
		parseCommand(bufferStr, sender);
		return (bytes);
	}

	std::cout << RED << "Client disconnected" << RESET << std::endl;
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].getSocket() == sender.getSocket())
		{
			_clients.erase(_clients.begin() + i);
			_fds.erase(_fds.begin() + i + 1);
		}
	}
	close(sender.getSocket());
	return (0);
}

void	Server::authenticateClient(Client& client, std::string& buffer) {
	std::cout << YELLOW << "Attempting to authenticate user" << RESET << std::endl;
	if (authenticatePassword(client, buffer)) {
		client.beAuthenticated();
		std::cout << GREEN << "Client successfully authenticated" << RESET << std::endl;
	}
	else {
		std::cerr << RED << "Could not authenticate client" << RESET << std::endl;
		return ;
	}
	if (registerClientNames(client, buffer)) {
		client.beRegistered();
		std::cout << GREEN << "Client successfully registered" << RESET << std::endl;
	}
	else {
		std::cerr << RED << "Could not register client" << RESET << std::endl;
		return ;
	}
	sendToClient( ":ft_irc 001 " + client.getNickname() + " :Welcome to the 42 Network, " + client.getNickname() + END, client);
	sendToClient( ":ft_irc 002 " + client.getNickname() + " :Your host is ft_irc, running version 1.0" + END, client);
	sendToClient( ":ft_irc 003 " + client.getNickname() + " :This server was created today, probably" + END, client);
	sendToClient( ":ft_irc 004 " + client.getNickname() + " :ft_irc 1.0" + END, client);

}

Client*	Server::getClient(const std::string& nick)
{
	if (DEBUG)
	{
		std::cout << "getClient: " << nick << std::endl;
		std::cout << "clients.size(): " << _clients.size() << std::endl;
	}
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (DEBUG)
			std::cout << "clients[i].getNickname(): " << _clients[i].getNickname() << std::endl;
		if (nick == _clients[i].getNickname())
			return &_clients[i];
	}
	return (NULL);
}

Client*	Server::checkClientRegistered(const std::string& username)
{
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it) {
		if (username == it->getUsername())
			return &(*it);
	}
	return NULL;
}

void Server::closeSocket()
{
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (DEBUG)
			std::cout << RED << "Closing socket" << RESET << std::endl;
		close(_clients[i].getSocket());
	}
	if (_socket != -1)
	{
		if (DEBUG)
			std::cout << RED << "Closing server socket" << RESET << std::endl;
		close(_socket);
	}
}

bool Server::_running = false;
void Server::signalHandler(int signum)
{
	std::cout << YELLOW <<"Signal received" << signum << RESET << std::endl;
	_running = false;
}

//parses the command from the client
//the command is in the format "COMMAND ARG1 ARG2 ARG3"
//You must be able to authenticate, set a nickname, a username, join a channel,
//send and receive private messages using your reference client.
void Server::parseCommand(std::string command, Client &client)
{
	std::vector<std::string>	args;
	std::stringstream			not_ss(command);

	while (not_ss >> command)
		args.push_back(command);
	if (args[0] == "NICK")
		cmd_nick(args[1], client);
	if (args[0] == "PRIVMSG")
		cmd_msg(args, args.size() , client);
	if (DEBUG)
	{
		if (args[0] == "LIST")
			printClients();
	}
	if (args[0] == "JOIN")
	{
		cmd_join(args, client);
		// std::cout << _channels[0].getName() << " was created woooho " << std::endl;
	}
}
