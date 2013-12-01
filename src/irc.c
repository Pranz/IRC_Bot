#include "irc.h"

#include <stdio.h> //Input/output
#include <stdarg.h>//Variable length arguments
#include <stdlib.h>//Standard definitions
#include <string.h>

#include <lolie/Stringp.h>//Null terminatd Stringp operations
#include <lolie/Memory.h>//Memory_equals
#include <lolie/LinkedList.h>

#include <unistd.h>//Unix standard library
#include <netdb.h> //Networking

void irc_send_rawf(irc_connection_id id,const char* format,...){
	static char format_buffer[IRC_FORMAT_BUFFER_LENGTH];

	va_list ap;
	va_start(ap,format);
	vsnprintf(format_buffer,IRC_FORMAT_BUFFER_LENGTH,format,ap);
	va_end(ap);
	irc_send_rawnt(id,format_buffer);
}

irc_connection_id irc_connect(const char* host,unsigned short port){
	struct addrinfo hints,
	               *res;
	char portStr[6];sprintf(portStr,"%d",port);

	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(host,portStr,&hints,&res);
	irc_connection_id id = socket(res->ai_family,res->ai_socktype,res->ai_protocol);

	int connection = connect(id,res->ai_addr,res->ai_addrlen);
	if(connection<0)
		printf("Error: connect() returned %i\n",connection);

	return id;
}

inline void irc_send_raw(irc_connection_id id,const char* str,size_t len){
	Stringp_put(STRINGP(">> ",3),stdout);
	Stringp_put(STRINGP(str,len),stdout);
	write(id,str,len);
}

inline void irc_send_rawnt(irc_connection_id id,const char* str){
	irc_send_raw(id,str,strlen(str));
}

inline void irc_send_message(irc_connection_id id,Stringp target,Stringp message){
	char write_buffer[message.length+target.length+12];

	int len = Stringp_vcopy(STRINGP(write_buffer,IRC_BUFFER_LENGTH),5,
		STRINGP("PRIVMSG ",8),
		target,
		STRINGP(" :",2),
		message,
		STRINGP("\r\n",2)
	);
	irc_send_raw(id,write_buffer,len);
}

void irc_parse_message(irc_connection_id id,Stringp raw_message,void(*onMessageFunc)(irc_connection_id id,const irc_message* message)){
	//If standard message prefix
	if(raw_message.ptr[0] == ':'){
		irc_message message;
		message.raw_message = raw_message;

		char* read_ptr       = raw_message.ptr+1,
		    * read_ptr_begin = read_ptr,
		    * read_ptr_end   = raw_message.ptr+raw_message.length;

		//Prefix
		message.prefix_type = IRC_MESSAGE_PREFIX_UNKNOWN;
		while(true){
			if(read_ptr>=read_ptr_end)
				return;
			if(read_ptr[0] == '\r' && read_ptr[1] == '\n')
				return irc_parse_message(id,STRINGP(read_ptr+2,raw_message.length-(read_ptr+2-raw_message.ptr)),onMessageFunc);

			if(*read_ptr == ' '){//If end of prefix
				if(message.prefix_type == IRC_MESSAGE_PREFIX_USER){//If already determined it is a user message, then it is a hostname
					message.prefix.user.host = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
					read_ptr_begin = ++read_ptr;
				}else if(message.prefix_type == IRC_MESSAGE_PREFIX_UNKNOWN){//If not yet determined, then it is a servername
					message.prefix.server.name = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
					message.prefix_type = IRC_MESSAGE_PREFIX_SERVER;
					read_ptr_begin = ++read_ptr;
				}
				break;
			}

			if(message.prefix_type == IRC_MESSAGE_PREFIX_USER && *read_ptr == '@'){//If already found '!' and finds '@', then it is a username
				message.prefix.user.username = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
				read_ptr_begin = ++read_ptr;
			}
			else if(*read_ptr == '!'){//If separated by '!', then it is a nickname
				message.prefix.user.nickname = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
				message.prefix_type = IRC_MESSAGE_PREFIX_USER;
				read_ptr_begin = ++read_ptr;
			}

			++read_ptr;
		}

		//Command
		message.command_type = IRC_MESSAGE_COMMAND_UNKNOWN;
		while(true){
			if(read_ptr>=read_ptr_end)
				return;
			if(read_ptr[0] == '\r' && read_ptr[1] == '\n')
				return irc_parse_message(id,STRINGP(read_ptr+2,raw_message.length-(read_ptr+2-raw_message.ptr)),onMessageFunc);

			if(*read_ptr == ' '){//If end of command
				switch(read_ptr-read_ptr_begin){
					case 3:
						if(read_ptr_begin[0]>='0' && read_ptr_begin[0]<='9' &&
						   read_ptr_begin[1]>='0' && read_ptr_begin[1]<='9' &&
						   read_ptr_begin[2]>='0' && read_ptr_begin[2]<='9'){
							message.command_type = IRC_MESSAGE_COMMAND_NUMBER;
							message.command_type_number = atoi((char[4]){read_ptr_begin[0],read_ptr_begin[1],read_ptr_begin[2],'\0'});
						}
						break;
					case 4:
						if(Memory_equals(read_ptr_begin,"JOIN",4)){
							message.command_type = IRC_MESSAGE_COMMAND_JOIN;
							message.command.join.channels=NULL;
						}else if(Memory_equals(read_ptr_begin,"PART",4)){
							message.command_type = IRC_MESSAGE_COMMAND_PART;
							message.command.part.channels=NULL;
						}else if(Memory_equals(read_ptr_begin,"NICK",4))
							message.command_type = IRC_MESSAGE_COMMAND_NICK;
						else if(Memory_equals(read_ptr_begin,"KICK",4))
							message.command_type = IRC_MESSAGE_COMMAND_KICK;
						break;
					case 5:
						if(Memory_equals(read_ptr_begin,"TOPIC",5))
							message.command_type = IRC_MESSAGE_COMMAND_TOPIC;
						break;
					case 6:
						if(Memory_equals(read_ptr_begin,"NOTICE",6))
							message.command_type = IRC_MESSAGE_COMMAND_NOTICE;
						break;
					case 7:
						if(Memory_equals(read_ptr_begin,"PRIVMSG",7))
							message.command_type = IRC_MESSAGE_COMMAND_PRIVMSG;
						break;
				}
				read_ptr_begin = ++read_ptr;
				break;
			}

			++read_ptr;
		}

		//Params
		bool repeat=false;
		unsigned char paramCount=0;
		while(true){//TODO: Implement all the other message command types and not only privmsg
			if((repeat=(read_ptr[0] == '\r' && read_ptr[1] == '\n')) || read_ptr>=read_ptr_end){
				if(paramCount==1){
					switch(message.command_type){
						case IRC_MESSAGE_COMMAND_PRIVMSG:
							message.command.privmsg.text = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
							break;
					}
				}
				break;
			}
			
			if(paramCount==0){
				if(read_ptr[0] == ' '){
					switch(message.command_type){
						case IRC_MESSAGE_COMMAND_PRIVMSG:
							message.command.privmsg.target = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
							break;
						case IRC_MESSAGE_COMMAND_JOIN:{
							Stringp* tmp = malloc(sizeof(Stringp*));
							*tmp = STRINGP(read_ptr_begin,read_ptr-read_ptr_begin);
							LinkedList_push(&message.command.part.channels,tmp);
						}	break;
					}
					if(*++read_ptr == ':')
						++read_ptr;
					read_ptr_begin = read_ptr;
					++paramCount;
				}
			}
			++read_ptr;
		}

		if(onMessageFunc!=NULL)
			onMessageFunc(id,&message);
		if(repeat)
			return irc_parse_message(id,STRINGP(read_ptr+2,raw_message.length-(read_ptr+2-raw_message.ptr)),onMessageFunc);
		else
			return;

	}
	//Else if it is a ping request 
	else if(Memory_equals(raw_message.ptr,"PING",4)){
		raw_message.ptr[1]='O';//Set buffer to PONG instead of PING
		irc_send_raw(id,raw_message.ptr,raw_message.length);//Send
	}
}
