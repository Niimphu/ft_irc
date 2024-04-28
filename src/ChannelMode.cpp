#include "IRC.hpp"

int	Channel::modeGet(Client &client)
{
	std::string	modeString = "";
	std::string	modeArgs = "";
	if (_isInviteOnly)
		modeString += "i";
	if (_restrictTopic)
		modeString += "t";
	if (_clientLimit > 0) {
		modeString += "l";
		std::stringstream notSS;
		notSS << _clientLimit;
		modeArgs += " " + notSS.str();
	}
	if (!_key.empty()) {
		modeString += "k";
		modeArgs += " " + _key;
	}
	if (!modeString.empty())
		modeString = "+" + modeString;

	std::string	message = modeString + modeArgs;
	_server->sendToClient(buildReply(_name, client.getNickname(), 324, message, 1, _name.c_str()), client);
	return (0);
}

int Channel::modeInvite(const std::string& modeString)
{
    if (modeString == "-i")
	{
		setInviteOnly(false);
		channelMessage(buildReply(_name, _name, MODE, "", 1, modeString.c_str()));
        return (1);
	}
	if (modeString == "+i")
	{
		setInviteOnly(true);
		channelMessage(buildReply(_name, _name, MODE, "", 1, modeString.c_str()));
        return (1);
	}
    return (0);
}

int Channel::modeTopic(const std::string& modeString)
{
    if (modeString == "-t")
	{
		setrestrictTopic(false);
		channelMessage(buildReply(_name, _name, MODE, "", 1, modeString.c_str()));
        return (1);
	}
	if (modeString == "+t")
	{
		setrestrictTopic(true);
		channelMessage(buildReply(_name, _name, MODE, "", 1, modeString.c_str()));
        return (1);
	}
    return (0);
}

int Channel::modeKey(const std::string& modeString, const std::string &arg)
{
    if (modeString == "-k")
    {
        setKey("");
		channelMessage(buildReply(_name, _name, MODE, "", 1, modeString.c_str()));
        return (1);
    }
    if (modeString == + "+k")
    {
        if (arg.empty())
            return (0);
		setKey(arg);
		channelMessage(buildReply(_name, _name, MODE, "", 2, modeString.c_str(), arg.c_str()));
        return (1);
    }
    return (0);
}

int Channel::modeOp(const std::string& modeString, const std::string& arg, Client &client)
{
    if (modeString == "+o")
    {
        if (!clientIsOp(arg))
        {
            if (!_server->findClient(arg))
                return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 401, "", 1, arg.c_str()), client)); //wrong error 
            addOperator(*_server->findClient(arg));
            removeClient(arg);
            channelMessage(buildReply(_name, _name, MODE, "", 2, modeString.c_str(), arg.c_str()));
            return(1);
        }
        return (0);
    }
    if (modeString == "-o")
    {
        if (clientIsOp(arg))
        {
            if (!findOps(arg))
                return (_server->sendToClient(buildReply(SERVER, client.getNickname(), 401, "", 1, arg.c_str()), client)); //wrong error
            removeOperator(arg);
            addClient(*_server->findClient(arg));
            channelMessage(buildReply(_name, _name, MODE, "", 2, modeString.c_str(), arg.c_str()));
            return (1);
        }
        return (0);
    }
    return (0);
}

int Channel::modeLimit(const std::string& modeString, const std::string &arg)
{
	if (modeString == "-l")
	{
		setClientLimit(0);
		channelMessage(buildReply(_name, _name, MODE, "", 2, modeString.c_str(), arg.c_str()));
        return (1);
	}
    if(modeString == "+l")
    {
		if (arg.empty())
			return (0);
		setClientLimit(atoi(arg.c_str()));
		channelMessage(buildReply(_name, _name, MODE, "", 2, modeString.c_str(), arg.c_str()));
        return (1);
	}
    return (0);
}

int	Channel::mode(const std::string& modeString, const std::string &arg,  Client &client)
{
	std::string clientName = client.getNickname();

    if (modeString.empty())
		return (modeGet(client));
	if (!clientIsOp(clientName))
		return(_server->sendToClient(buildReply(SERVER, clientName, 482, "", 1, _name.c_str()), client));
	if ((modeString == "-i" || modeString == "+i") && modeInvite(modeString))
		return (0);
	if ((modeString == "-t" || modeString == "+t") && modeTopic(modeString))
        return (0);
	if ((modeString == "-k" || modeString == "+k") && modeKey(modeString, arg))
        return (0);
	if ((modeString == "+o" || modeString == "-o") && modeOp(modeString, arg, client))
		return (0);
	if ((modeString == "+l" || modeString == "-l") && modeLimit(modeString, arg))
		return (0);
	return (0);
}
