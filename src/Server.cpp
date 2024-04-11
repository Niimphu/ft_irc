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

int Server::verifyPassword(int clientSocket, std::string password)
{
	//send password request to client
	(void)clientSocket;
	(void)password;
	return (0);
}

void Server::start()
{
	std::cout << GREEN << "Server starting" << RESET << std::endl;
	createSocket();
	_running = true;
	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);

//	std::cout << GREEN << "Server started, on socket " << _socket << RESET << std::endl;
//	std::cout <<  GREEN "\tListening on port " << _port << RESET <<  std::endl;

	std::cout << GREEN << "Listening..." << RESET << std::endl;
	while (_running)
	{
		if (poll(&_fds[0], _fds.size(), -1) == -1 && _running)
			std::cerr << RED << "Error polling" << RESET << std::endl;
		else
		{
			if (_fds[0].revents & POLLIN)
			{
				acceptSocket();
				/* while (!verifyPassword(_clients[_clients.size() - 1].getSocket(), _password))
					std::cerr << RED << "Incorrect password" << RESET << std::endl; */
				//suitable place to get initial data from client
			}
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
//	struct sockaddr_in	addr;

//	addr.sin_family = AF_INET;
//	addr.sin_addr.s_addr = INADDR_ANY;
//	addr.sin_port = htons(static_cast<uint16_t>(std::atoi(_port.c_str())));
	setHints();
	_socket = socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol);
//	if ((_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
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

void	Server::setHints(void) {
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
		return 1;
	}
	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << RED << "Error setting client socket to non-blocking" << RESET << std::endl;
		return 1;
	}

	struct pollfd	newClientFD;
	newClientFD.fd = clientSocket;
	newClientFD.events = POLLIN;
	newClientFD.revents = 0;

	Client	newClient(clientSocket);
//	client.setSocket(clientSocket);
//	client.setIp(inet_ntoa(addr.sin_addr));
	_clients.push_back(newClient);
	_fds.push_back(newClientFD);
//	if (DEBUG)
//		std::cout << GREEN << "Client connected from " << client.getIp() << RESET << std::endl;
	return 0;
}

//sends a message to the client
int Server::sendToClient(std::string message, Client client)
{
	if (send(client.getSocket(), message.c_str(), message.length(), 0) < 0)
	{
		std::cerr << RED << "Error sending message" << RESET << std::endl;
		return (-1);
	}
	else
		if (DEBUG)
			std::cout << GREEN << "Sent: " << message << RESET << std::endl;
	return (0);
}

int Server::receiveFromClient(Client client)
{
	char	buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);

	int		bytes = recv(client.getSocket(), buffer, BUFFER_SIZE, 0);
	if (bytes > 0)
	{
		buffer[bytes] = '\0';
		if (DEBUG)
			std::cout << GREEN << "Received: " << buffer << RESET << std::endl;
//		sendSocket(":server!server@server.com PRIVMSG 42bober :Hey, what's up?", client.getSocket());
//		handleClientRequest();
		return (bytes);
	}

	std::cout << RED << "Client disconnected" << RESET << std::endl;
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].getSocket() == client.getSocket())
		{
			_clients.erase(_clients.begin() + i);
			_fds.erase(_fds.begin() + i + 1);
		}
	}
	close(client.getSocket());
	return (0);
}

Client*	Server::getClient(const std::string& nick)
{
	for (clientIt it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (nick == it->getNickname())
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

int Server::cmd_nick(std::string nick, Client &client)
{
	//if no nickname is given
	if (nick.empty())
	{
		std::cerr << RED << "No nickname given" << RESET << std::endl;
		sendToClient(":ft_irc 431 :No nickname given" + END, client);
		return (0);
	}
	//if nickname is invalid (starts with #, :, or space)
	if (nick[0] == '#' || nick[0] == ':' || nick[0] == ' ')
	{
		std::cerr << RED << "Invalid nickname" << RESET << std::endl;
		sendToClient(":ft_irc 432 :Erroneous nickname" + END, client);
		return (0);
	
	}
	//if nickname is invalid
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (nick == _clients[i].getNickname())
		{
			std::cerr << RED << "Nickname already in use" << RESET << std::endl;
			sendToClient(":ft_irc 433 :Nickname is already in use" + END, client);
			return (0);
		}
	}
	client.setNickname(nick);
	return (1);
}

// int Server::cmd_msg(std::vector<std::string> args, Client &client)
// {
	// Server *server = &client->getServer();
	// if (args.size() < i + 1)
	// {
	// 	std::cerr << RED << "Invalid command" << RESET << std::endl;
	// 	return (0);
	// }
	// Client *receiver = server->getClient(args[i + 1]);
	// if (receiver == NULL)
	// {
	// 	std::cerr << RED << "Client not found" << RESET << std::endl;
	// 	return (0);
	// }
	// std::string message;
	// while ( ++i < args.size())
	// 	message += args[i] + " ";
	// server->sendToClient(client->getNickname() + " : " + message + "\n", *receiver);
	// return (args.size() - i);
// }

// int Server::cmd_join(std::vector<std::string> args)
// {
	
// 	return (0);
// }

// int Server::cmd_leave(std::vector<std::string> args)
// {
// 	(void)client;
// 	if (args.size() < i + 1)
// 	{
// 		std::cerr << RED << "Invalid command" << RESET << std::endl;
// 		return (0);
// 	}
// 	//client->leaveChannel(args[i + 1]);
// 	return (1);
// }


//parses the command from the client
//the command is in the format "COMMAND ARG1 ARG2 ARG3"
//You must be able to authenticate, set a nickname, a username, join a channel,
//send and receive private messages using your reference client.
void Server::parseCommand(std::string command, Client &client)
{
	size_t i = 0;
	std::vector<std::string>	args;
	std::string					arg;
	std::stringstream			ss(command);

	while (ss >> arg)
		args.push_back(arg);
	if (args.size() == 0)
		return;
	while (i < args.size())
	{
		if (args[i++] == "/nick" && cmd_nick(args[i], client))
			i++;
		// if (args[i] == "/msg")
		// 	i += cmd_msg(args, client);
		// if (args[i] == "/join")
		// 	i += Cmd_join(args);
		// if (args[i] == "/leave") /quit
		// 	i += Cmd_join(args);
		else
			i++;
	}
}