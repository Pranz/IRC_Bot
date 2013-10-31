#ifndef __LOLIROFLE_IRC_IRC_H_INCLUDED__
#define __LOLIROFLE_IRC_IRC_H_INCLUDED__

#include <lolie/Stringp.h>

#define IRC_BUFFER_LENGTH 512
#define IRC_FORMAT_BUFFER_LENGTH 512

typedef enum irc_message_prefix_type{
	IRC_MESSAGE_PREFIX_UNKNOWN,
	IRC_MESSAGE_PREFIX_USER,
	IRC_MESSAGE_PREFIX_SERVER
}irc_message_prefix_type;

typedef enum irc_message_command_type{
	IRC_MESSAGE_COMMAND_UNKNOWN,
	IRC_MESSAGE_COMMAND_NUMBER,
	IRC_MESSAGE_COMMAND_JOIN,
	IRC_MESSAGE_COMMAND_PART,
	IRC_MESSAGE_COMMAND_NICK,
	IRC_MESSAGE_COMMAND_TOPIC,
	IRC_MESSAGE_COMMAND_KICK,
	IRC_MESSAGE_COMMAND_PRIVMSG,
	IRC_MESSAGE_COMMAND_NOTICE
}irc_message_command_type;

typedef struct irc_message{
	Stringp raw_message;

	irc_message_prefix_type prefix_type;
	union{
		struct{
			Stringp nickname;
			Stringp username;
			Stringp host;
		}user;

		struct{
			Stringp name;
		}server;
	}prefix;

	struct{
		unsigned int /*irc_message_command_type*/ command_type        :6; //Value limit: 0 to 64
		unsigned int /*unsigned int*/             command_type_number :10;//Value limit: 0 to 1024
	};
	union{
		struct{
			Stringp channel;
		}join;

		struct{
			Stringp channel;
		}part;

		struct{
			Stringp target;
			Stringp text;
		}privmsg;
	}command;
}irc_message;

/**
 * Id of the connection, got from socket()
 */
typedef int irc_connection_id;

/**
 * Sends a raw message to the irc server
 *
 * @param id  Id of the connection
 * @param str String to be sent
 * @param len Length of Stringp to be sent
 */
void irc_send_raw(irc_connection_id id,const char* str,size_t len);

/**
 * Sends a null terminated raw message to the irc server
 *
 * @param id  Id of the connection
 * @param str String to be sent
 */
void irc_send_rawnt(irc_connection_id id,const char* str);

/**
 * Sends a formatted raw message to the irc server
 *
 * @param id     Id of the connection
 * @param format Equivalent to the printf formatting, see printf documentation
 * @param ...    Data 
 */
void irc_send_rawf(irc_connection_id id,const char* format,...);

/**
 * Connects to a irc server by host and port
 *
 * @param host Hostname (address/ip) of the irc server
 * @param port Port for the hostname of the irc server
 */
irc_connection_id irc_connect(const char* host,unsigned short port);

/**
 * Sets nickname for irc connection
 *
 * @param id   Id of the connection
 * @param name Specified nickname
 */
inline void irc_set_nickname(irc_connection_id id,const char* name){
	irc_send_rawf(id,"NICK %s\r\n",name);
}

/**
 * Sets username for irc connection
 *
 * @param id   Id of the connection
 * @param name Specified username
 */
inline void irc_set_username(irc_connection_id id,const char* name){
	irc_send_rawf(id,"USER %s 0 0 :%s\r\n",name,name);
}

/**
 * Joins a channel in a irc connection
 *
 * @param id      Id of the connection
 * @param channel Channel name
 */
inline void irc_join_channel(irc_connection_id id,const char* channel){
	irc_send_rawf(id,"JOIN %s\r\n",channel);
}

void irc_parse_message(irc_connection_id id,Stringp raw_message,void(*onMessageFunc)(irc_connection_id id,const irc_message* message));

void irc_send_message(irc_connection_id id,Stringp target,Stringp message);

#endif
