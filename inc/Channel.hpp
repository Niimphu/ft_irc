#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include "Client.hpp"

class Channel
{
	private:
		std::string	_name;
		std::string	_topic;
		std::string _key;
		std::vector<Client>	_operators;
		std::vector<Client>	_clients;

	public:
		Channel(std::string name, std::string key): _name(name), _key(key) {
			_topic = "No topic yet";
		};
		~Channel() {};

		std::string	getName() { return _name; };
		std::string getTopic() { return _topic;};
		std::vector<Client>	getClients() { return _clients; };
		size_t	getClientsSize(){ return _clients.size(); }
		void	setTopic(const std::string& topic) { _topic = topic; };
		void	setKey(const std::string& key) { _key = key; };
		void	addClient(Client client) { _clients.push_back(client); };
		void	addOperator(Client client) { _operators.push_back(client); };
		void	removeClient(Client client) { (void)client; return ; };




		std::string	getClientList(void) {
			std::string	list = "";
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
			return list;
		}

		void	removeClient(const std::string &name)
		{
			for (size_t i = 0; i < _clients.size(); i++)
			{
				std::cout << " searching user to remove "<< _clients[i].getNickname() << std::endl;
				if (name == _clients[i].getNickname())
				{
					std::cout << "gotcha" << std::endl;
					_clients.erase(_clients.begin() + i);
					if (DEBUG)
						std::cout << "removed " << _clients[i].getNickname() << " from " << _name << std::endl;
					return ;
				}
			}
			return ;
		};
};

#endif
