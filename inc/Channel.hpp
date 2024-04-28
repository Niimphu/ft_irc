#ifndef CHANNEL_HPP
# define CHANNEL_HPP
# include "IRC.hpp"

class Server;

class Channel
{
	private:
		Server*		_server;
		std::string	_name;
		std::string	_topic;
		std::string _key;

		std::vector<Client>	_operators;
		std::vector<Client>	_clients;
		std::vector<std::string> _invitedClients;

		bool	_isInviteOnly;
		bool	_restrictTopic;
		size_t	_userCount;
		size_t	_clientLimit;

	public:
		Channel(const std::string& name, const std::string& key, Server* server): _server(server), _name(name), _topic("No topic yet"), _key(key), _isInviteOnly(false), _restrictTopic(false), _userCount(0), _clientLimit(0){}
		~Channel() {}

		bool	operator==(Channel& channel) { return _name == channel._name; }

		std::string	getName() { return _name; }
		std::string getTopic() { return _topic;}

		void	setTopic(const std::string& topic) { _topic = topic; }
		void	setKey(const std::string& key) { _key = key; }

		void 	setInviteOnly(bool isInviteOnly) { _isInviteOnly = isInviteOnly; }
		bool	getInviteOnly(){ return _isInviteOnly; }
		void 	setrestrictTopic(bool restrictTopic) { _restrictTopic = restrictTopic;}
		void	setClientLimit(size_t limit) { _clientLimit = limit; }
		size_t	getClientLimit() { return _clientLimit; }
		void	addInvitedClient(const std::string& name) { _invitedClients.push_back(name); }
		size_t	getUserCount() { return _userCount; }

		int		clientMessage(const std::string& message, Client& sender);
		int		channelMessage(const std::string& message);

		int		join(Client& client, const std::string& key);
		int		part(Client& client, const std::string& reason);
		int		kick(Client& kicker, const std::string& user, const std::string& reason);

		int		topic(const std::string& newTopic, Client& client);

		int		mode(const std::string& modeString, const std::string &arg,  Client &client);
		int		modeGet(Client &client);
		int		modeInvite(const std::string& modeString);
		int		modeTopic(const std::string& modeString);
		int		modeKey(const std::string& modeString, const std::string &arg);
		int		modeOp(const std::string& modeString, const std::string& arg, Client &client);
		int		modeLimit(const std::string& modeString, const std::string &arg);

		int		addOperator(Client& client);
		int		addClient(Client& client);
		void	removeClient(const std::string& name);
		void	removeOperator(const std::string& name);
		void	removeUser(const std::string& name);

		std::string	getClientList();

		bool	clientIsOp(const std::string& name);
		Client*	findOps(const std::string& name);
		bool	clientIsInChannel(const std::string& name);
		bool	clientIsInvited(const std::string& name);
		void	removeInvitedClient(const std::string& name);

};

#endif
