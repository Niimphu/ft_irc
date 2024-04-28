# ft_irc

## IRC Protocol
IRC, or Internet Relay Chat, is a real-time messaging protocol that enables communication in the form of text between users over the Internet. It consists of various servers and clients, allowing individuals to join channels to engage in group discussions, or message each other in private conversations.

<img src='https://github.com/Niimphu/ft_irc/blob/main/screen.png' width='600'>

## Overview
In this project, we are tasked with developing an IRC server in C++98 standard that works with a specific "reference client". For our server, we chose to use [HexChat](https://hexchat.github.io/), and configured our parser and replies to follow ["modern IRC protocol"](https://modern.ircdocs.horse/#part-message).

The subject brief specifies that, as well as handling a certain set of commands, the server should also:
 - use one call to poll()
 - never hang
 - only have non-blocking I/O
 - use TCP/IP server-client connections

For the project, we developed an IRC server that can be started from terminal with `./ircserv <port> <password>`, and then from there, it handles all incoming requests from HexChat.


## Available commands:

| Function | Usage | Description |
| :-----------: | :----------- | :----------- |
| Password | `PASS <password>` | Authenticate the server password on connection. If the correct password is not provided, the user will be denied access to the rest of the server. |
| Nickname | `NICK <nickname>` | Set a user's nickname, which will be visible to other user on the server. |
| Username | `USER <username>` | Used during initial connection to set the username of the user. |
| Message | `PRIVMSG <nick/channel>` | Send a message to another user or a channel. |
| Join | `JOIN <channel> [key]` | Join a channel, or create the channel if it doesn't already exist.  |
| Part | `PART <channel> [reason]` | Removes the user from the specified channel. |
| Invite | `INVITE <nick> <channel>` | Invite a user to a channel. |
| Kick | `KICK <channel> <user>` | Force the removal of a user from a channel. |
| Names | `NAMES <channel>` | Retrieve a list of the users connected to the channel. |
| Mode | `MODE <nick/channel> <modestring> [argument]` | Set or remove options/modes from a given target. We have implemented user mode `o`, and channel modes `itkl`. |
| Topic | `TOPIC <channel> [topic]` | Change or view the topic of the given channel. |
| Quit | `QUIT [reason]` | Terminate the user's connection to the server. |
