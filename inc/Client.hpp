#ifndef CLIENT_HPP
# define CLIENT_HPP

#include "IRC.hpp"

class Client {
	private:
		bool		_authenticated;
		bool		_registered;
		int			_socket;
		std::string	_nick;
		std::string	_username;

	public:
		Client(int socket): _socket(socket), _nick(""), _username("") { _authenticated = false; _registered = false; };
		~Client() {};

		bool	operator==(const Client& other) const {
			return (this->_socket == other._socket && this->_username == other._username);
		}

		bool	operator!=(const Client& other) const {
			return (this->_socket != other._socket || this->_username != other._username);
		}

		int			getSocket() const { return _socket; };

		std::string	getNickname() const { return _nick; };
		void		setNickname(const std::string& nick) { _nick = nick; };
		std::string	getUsername() const { return _username; };
		void		setUsername(const std::string& username) { _username = username; };

		bool		isRegistered() { return _registered; };
		void		beRegistered() { _registered = true; };
		bool		isAuthenticated() { return _authenticated; };
		void		beAuthenticated() { _authenticated = true; };
};

#endif
