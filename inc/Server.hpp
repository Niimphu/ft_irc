#ifndef SERVER_HPP
# define SERVER_HPP

# include "IRC.hpp"

class Server
{
	private:
		const std::string&	_port;
		const std::string	_password;

		int					_socket;
		struct addrinfo		_hints;
		struct addrinfo*	_serverInfo;

		std::vector<Client>			_clients;
		std::vector<Channel>		_channels;
		std::vector<struct pollfd>	_fds;
		static bool					_running;

		int	guestCount;

	public:
		Server(const std::string& port = "7779", const std::string& password = "123");
		~Server();

		void	start();
		int		createServerSocket();
		void	setHints();
		int		acceptSocket();
		int		sendToClient(const std::string&, const Client&) const;
		int		receiveFromClient(Client&);
		void	closeSockets();

		static void	signalHandler(int signum);

		Client*		findClient(const std::string&);
		Channel*	findChannel(const std::string&);
		int			removeChannel(Channel& channel);
		int			clearEmptyChannels();

		Client*	usernameIsRegistered(const std::string&);

		void	parseCommand(const std::string&, Client&);

		int		authenticatePassword(Client&, std::string&);
		int		changeNickname(const std::string& nick, Client& client);
		int		setUsername(std::string& user, Client& client);
		void	registerClient(Client& client) const;

		int		sendMessage(std::string&, std::string&, Client& client);
		int		joinChannel(std::string&, std::string&, Client&);
		int		inviteChannel(const std::string& _target, const std::string& _channel, const Client& client);
		int		ChannelMessage(std::string&, std::string&, Client& client);
		void	names(Client& client, std::string& channelName);
		int		channelTopic(const std::string& channel,const std::string& newTopic, Client& client);
		int		mode(const std::string& channelName, const std::string& modeString, const std::string &arg,  Client &client);
		int		kickClient(const std::string& channelName, const std::string& target, const std::string& reason, Client& client);
		int		partChannel(const std::string&, const std::string&, Client&);
		int		quit(Client& client, std::string& quitMessage);


};

#endif
