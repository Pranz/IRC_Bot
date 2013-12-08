#ifndef __LOLIROFLE_IRC_IRC_H_INCLUDED__
#define __LOLIROFLE_IRC_IRC_H_INCLUDED__

#include <lolie/Stringp.h>
#include <lolie/LinkedList.h>
size_t strlen(const char* str);

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
		LinkedList/*<Stringp>*/* channels;
		struct{
			Stringp target;
			Stringp text;
		}privmsg;
	}command;
}irc_message;

/**
 * IRC Connection structure
 */
typedef struct irc_connection{
	int id;//Id of the connection, got from socket()
}irc_connection;

/**
 * Sends a raw message to the irc server
 *
 * @param id  Id of the connection
 * @param str String to be sent
 * @param len Length of Stringp to be sent
 */
void irc_send_raw(const irc_connection* connection,const char* str,size_t len);

/**
 * Sends a raw message to the irc server using a null terminated string
 *
 * @param id  Id of the connection
 * @param str String to be sent
 */
inline void irc_send_rawnt(const irc_connection* connection,const char* str){
	irc_send_raw(connection,str,strlen(str));
}
/**
 * Sends a formatted raw message to the irc server
 *
 * @param id     Id of the connection
 * @param format Equivalent to the printf formatting, see printf documentation
 * @param ...    Data 
 */
void irc_send_rawf(const irc_connection* connection,const char* format,...);

/**
 * Connects to a irc server by host and port
 *
 * @param host Hostname (address/ip) of the irc server
 * @param port Port for the hostname of the irc server
 */
irc_connection irc_connect(const char* host,unsigned short port);

/**
 * Sets nickname for irc connection
 *
 * @param id   Id of the connection
 * @param name Specified nickname
 */
inline void irc_set_nickname(const irc_connection* connection,const char* name){
	irc_send_rawf(connection,"NICK %s\r\n",name);
}

/**
 * Sets username for irc connection
 *
 * @param id   Id of the connection
 * @param name Specified username
 */
inline void irc_set_username(const irc_connection* connection,const char* name){
	irc_send_rawf(connection,"USER %s 0 0 :%s\r\n",name,name);
}

/**
 * Joins a channel in a irc connection
 *
 * @param id      Id of the connection
 * @param channel Channel name
 */
inline void irc_join_channel(const irc_connection* connection,const char* channel){
	irc_send_rawf(connection,"JOIN %s\r\n",channel);
}

void irc_parse_message(const irc_connection* connection,Stringp raw_message,void(*onMessageFunc)(const irc_connection* connection,const irc_message* message));

void irc_send_message(const irc_connection* connection,Stringp target,Stringp message);

bool irc_read(const irc_connection* connection,void(*onMessageFunc)(const irc_connection* connection,const irc_message* message));

bool irc_disconnect(const irc_connection* connection);

#endif
