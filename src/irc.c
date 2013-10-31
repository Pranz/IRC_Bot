#include "irc.h"

#include <stdio.h> //Input/output
#include <stdarg.h>//Variable length arguments
#include <stdlib.h>//Standard definitions

#include <string.h>//Null terminatd string operations

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
	connect(id,res->ai_addr,res->ai_addrlen);

	return id;
}

inline void irc_send_raw(irc_connection_id id,const char* str,size_t len){
	fputsn(">> ",3,stdout);
	fputsn(str,len,stdout);
	write(id,str,len);
}

inline void irc_send_rawnt(irc_connection_id id,const char* str){
	irc_send_raw(id,str,strlen(str));
}

inline void irc_send_message(irc_connection_id id,string target,string message){
	char write_buffer[message.len+target.len+12];

	int len = sputstring(STRING(write_buffer,IRC_BUFFER_LENGTH),5,
		STRING("PRIVMSG ",8),
		target,
		STRING(" :",2),
		message,
		STRING("\r\n",2)
	);
	irc_send_raw(id,write_buffer,len);
}

void irc_parse_message(irc_connection_id id,string raw_message,void(*onMessageFunc)(irc_connection_id id,const irc_message* message)){
	//If standard message prefix
	if(raw_message.chrs[0] == ':'){
		irc_message message;
		message.raw_message = raw_message;

		char* read_ptr       = raw_message.chrs+1,
		    * read_ptr_begin = read_ptr,
		    * read_ptr_end   = raw_message.chrs+raw_message.len;

		//Prefix
		message.prefix_type = IRC_MESSAGE_PREFIX_UNKNOWN;
		while(true){
			if(read_ptr>=read_ptr_end)
				return;
			if(read_ptr[0] == '\r' && read_ptr[1] == '\n')
				return irc_parse_message(id,STRING(read_ptr+2,raw_message.len-(read_ptr+2-raw_message.chrs)),onMessageFunc);

			if(*read_ptr == ' '){//If end of prefix
				if(message.prefix_type == IRC_MESSAGE_PREFIX_USER){//If already determined it is a user message, then it is a hostname
					message.prefix.user.host = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
					read_ptr_begin = ++read_ptr;
				}else if(message.prefix_type == IRC_MESSAGE_PREFIX_UNKNOWN){//If not yet determined, then it is a servername
					message.prefix.server.name = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
					message.prefix_type = IRC_MESSAGE_PREFIX_SERVER;
					read_ptr_begin = ++read_ptr;
				}
				break;
			}

			if(message.prefix_type == IRC_MESSAGE_PREFIX_USER && *read_ptr == '@'){//If already found '!' and finds '@', then it is a username
				message.prefix.user.username = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
				read_ptr_begin = ++read_ptr;
			}
			else if(*read_ptr == '!'){//If separated by '!', then it is a nickname
				message.prefix.user.nickname = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
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
				return irc_parse_message(id,STRING(read_ptr+2,raw_message.len-(read_ptr+2-raw_message.chrs)),onMessageFunc);

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
						if(strneq(read_ptr_begin,"JOIN",4))
							message.command_type = IRC_MESSAGE_COMMAND_JOIN;
						else if(strneq(read_ptr_begin,"PART",4))
							message.command_type = IRC_MESSAGE_COMMAND_PART;
						else if(strneq(read_ptr_begin,"NICK",4))
							message.command_type = IRC_MESSAGE_COMMAND_NICK;
						else if(strneq(read_ptr_begin,"KICK",4))
							message.command_type = IRC_MESSAGE_COMMAND_KICK;
						break;
					case 5:
						if(strneq(read_ptr_begin,"TOPIC",5))
							message.command_type = IRC_MESSAGE_COMMAND_TOPIC;
						break;
					case 6:
						if(strneq(read_ptr_begin,"NOTICE",6))
							message.command_type = IRC_MESSAGE_COMMAND_NOTICE;
						break;
					case 7:
						if(strneq(read_ptr_begin,"PRIVMSG",7))
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
		while(true){
			if((repeat=(read_ptr[0] == '\r' && read_ptr[1] == '\n')) || read_ptr>=read_ptr_end){
				if(paramCount==1){
					switch(message.command_type){
						case IRC_MESSAGE_COMMAND_PRIVMSG:
							message.command.privmsg.text = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
							break;
					}
				}
				break;
			}
			
			if(paramCount==0){
				if(read_ptr[0] == ' '){
					switch(message.command_type){
						case IRC_MESSAGE_COMMAND_PRIVMSG:
							message.command.privmsg.target = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
							break;
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
			return irc_parse_message(id,STRING(read_ptr+2,raw_message.len-(read_ptr+2-raw_message.chrs)),onMessageFunc);
		else
			return;

	}
	//Else if it is a ping request 
	else if(strneq(raw_message.chrs,"PING",4)){
		raw_message.chrs[1]='O';//Set buffer to PONG instead of PING
		irc_send_raw(id,raw_message.chrs,raw_message.len);//Send
	}
}
