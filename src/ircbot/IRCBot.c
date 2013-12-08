#include <stdio.h> //Input/output
#include <string.h>
#include <time.h>

#include <openssl/md4.h>
#include <openssl/md5.h>

#include <lolie/Stringp.h>
#include <lolie/LinkedList.h>
#include <lolie/Memory.h>
#include <lolie/Math.h>
#include <lolie/ControlStructures.h>
#include <lolie/url.c>

#include <ircinterface/irc.h>
#include "Locale.h"

//TODO: "&&"" to combine commands and maybe `command` to insert a command with output as return value to an argument
//TODO: Help pages for a list of commands and syntax, explanation, etc.
//TODO: Command aliases

/**
 * Buffer initialization
 */
#define IRC_WRITE_BUFFER_LEN 512
char write_buffer[IRC_WRITE_BUFFER_LEN];
Stringp command_prefix;
Stringcp bot_nickname,//TODO: irc connect should store this information and be able to change it
         version_signature;

char command_separator = ' ';
enum Languages language = LANG_SWEDISH;

Stringp string_splitted(Stringp str,size_t(*delimiterFunc)(Stringp str),bool(*onSplitFunc)(const char* begin,const char* end)){
	const char* arg_begin=str.ptr;

	loop{
		if(str.length<1){
			onSplitFunc(arg_begin,str.ptr);
			break;
		}else{
			size_t delim = delimiterFunc(str);
			if(delim>0){
				str.length-=delim;
				if(!onSplitFunc(arg_begin,str.ptr))
					return STRINGP(str.ptr+delim,str.length);
				str.ptr+=delim;
				arg_begin=str.ptr;
				continue;
			}
		}

		++str.ptr;
		--str.length;
	}

	return str;
}

Stringp string_splitted_delim(Stringp str,Stringp delimiter,bool(*onSplitFunc)(const char* begin,const char* end)){
	return string_splitted(str,function(size_t,(Stringp str){
		if(str.length>=delimiter.length && Memory_equals(str.ptr,delimiter.ptr,delimiter.length))
			return delimiter.length;
		else
			return 0;
	}),onSplitFunc);
}

Stringp Stringp_find_substr(Stringp str,bool(*findFunc)(Stringp str));

struct CommandArgument{
	Stringp name;
	enum{
		COMMAND_ARGUMENT_TYPE_INTEGER,
		COMMAND_ARGUMENT_TYPE_STRING,
		COMMAND_ARGUMENT_TYPE_FLOATINGPOINT,
		COMMAND_ARGUMENT_TYPE_CHARACTER,
	}type;

	enum{
		COMMAND_ARGUMENT_REQUIRED,
		COMMAND_ARGUMENT_OPTIONAL,
		COMMAND_ARGUMENT_VARARG
	}requirement;
};

struct Command{
	Stringp name;
	Stringp help;
	Stringp delimiter;
	struct CommandArgument args[];
};

void onCommand(const irc_connection* connection,Stringcp command,Stringcp target,const char* arg_begin,const char* arg_end,const irc_message* message){
	const char* read_ptr = arg_begin;

	switch(command.length){
		case 3:
			//md5
			if(Memory_equals(command.ptr,"md5",3)){
				unsigned char md5_buffer[MD5_DIGEST_LENGTH];

				MD5_CTX context;
				MD5_Init(&context);
				MD5_Update(&context,arg_begin,arg_end-arg_begin);
				MD5_Final(md5_buffer,&context);

				char* write_ptr=write_buffer;
				for(unsigned int i=0;i<MD5_DIGEST_LENGTH;++i)
					write_ptr+=sprintf(write_ptr,"%02x",md5_buffer[i]);

				irc_send_message(connection,target,STRINGCP(write_buffer,write_ptr-write_buffer));
				goto SuccessCommand;
			}
			//md4
			if(Memory_equals(command.ptr,"md4",3)){
				unsigned char md4_buffer[MD4_DIGEST_LENGTH];

				MD4_CTX context;
				MD4_Init(&context);
				MD4_Update(&context,arg_begin,arg_end-arg_begin);
				MD4_Final(md4_buffer,&context);

				char* write_ptr=write_buffer;
				for(unsigned int i=0;i<MD4_DIGEST_LENGTH;++i)
					write_ptr+=sprintf(write_ptr,"%02x",md4_buffer[i]);

				irc_send_message(connection,target,STRINGCP(write_buffer,write_ptr-write_buffer));
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 4:
			//Bool
			if(Memory_equals(command.ptr,"bool",4)){
				irc_send_message(connection,target,locale[language].boolean[rand()%2]);
				goto SuccessCommand;
			}
			//Dice
			if(Memory_equals(command.ptr,"dice",4)){
				write_buffer[0]=rand()%6+'1';
				irc_send_message(connection,target,STRINGCP(write_buffer,1));
				goto SuccessCommand;
			}
			//Test
			if(Memory_equals(command.ptr,"test",4)){
				for(int i=0;i<5;++i)
					irc_send_message(connection,target,STRINGCP("ABCDEF",6));
				goto SuccessCommand;
			}
			//Echo
			if(Memory_equals(command.ptr,"echo",4)){
				irc_send_message(connection,target,STRINGCP(arg_begin,arg_end-arg_begin));
				goto SuccessCommand;
			}
			//Wikipedia
			if(Memory_equals(command.ptr,"wiki",4)){
				int len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),1,
					Stringcp_from_cstr("http://en.wikipedia.org/wiki/")
				);
				len+=url_encode(STRINGP(arg_begin,arg_end-arg_begin),STRINGP(write_buffer+len,IRC_WRITE_BUFFER_LEN-len));
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//IMDb
			if(Memory_equals(command.ptr,"imdb",4)){
				int len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),1,
					Stringp_from_cstr("http://www.imdb.com/find?s=all&q=")
				);
				len+=url_encode(STRINGP(arg_begin,arg_end-arg_begin),STRINGP(write_buffer+len,IRC_WRITE_BUFFER_LEN-len));
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//Date
			if(Memory_equals(command.ptr,"date",4)){
				time_t t;time(&t);
					struct tm* time_data = localtime(&t);

				int len = strftime(write_buffer,IRC_WRITE_BUFFER_LEN,"%F %X %Z, %A v.%V, Day %j of the year",time_data);
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 5:
			//Upper
			if(Memory_equals(command.ptr,"upper",5)){
				char* write_ptr = write_buffer;
				while(read_ptr<arg_end){
					*write_ptr++=*read_ptr>='a' && *read_ptr<='z'?(*read_ptr)+('A'-'a'):*read_ptr;
					++read_ptr;
				}

				irc_send_message(connection,target,STRINGCP(write_buffer,arg_end-arg_begin));
				goto SuccessCommand;
			}
			//Lower
			if(Memory_equals(command.ptr,"lower",5)){
				char* write_ptr = write_buffer;
				while(read_ptr<arg_end){
					*write_ptr++=*read_ptr>='A' && *read_ptr<='Z'?(*read_ptr)+('a'-'A'):*read_ptr;
					++read_ptr;
				}

				irc_send_message(connection,target,STRINGCP(write_buffer,arg_end-arg_begin));
				goto SuccessCommand;
			}
			//ROT13
			if(Memory_equals(command.ptr,"rot13",5)){
				char* write_ptr = write_buffer;
				while(read_ptr<arg_end){
					if(*read_ptr>='A' && *read_ptr<='Z')
						*write_ptr++=((*read_ptr)-'A'+13)%26+'A';
					else if(*read_ptr>='a' && *read_ptr<='z')
						*write_ptr++=((*read_ptr)-'a'+13)%26+'a';
					else if(*read_ptr>='0' && *read_ptr<='9')
						*write_ptr++=((*read_ptr)-'0'+5)%10+'0';
					else
						*write_ptr++=*read_ptr;
					++read_ptr;
				}

				irc_send_message(connection,target,STRINGCP(write_buffer,arg_end-arg_begin));
				goto SuccessCommand;
			}
			//ROT47
			if(Memory_equals(command.ptr,"rot47",5)){
				char* write_ptr = write_buffer;
				while(read_ptr<arg_end){
					if(*read_ptr>='!' && *read_ptr<='~')
						*write_ptr++=((*read_ptr)-'!'+47)%94+'!';
					else
						*write_ptr++=*read_ptr;
					++read_ptr;
				}

				irc_send_message(connection,target,STRINGCP(write_buffer,arg_end-arg_begin));
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 6:
			//Random
			if(Memory_equals(command.ptr,"random",6)){
				unsigned int value=rand(),min=1,max=RAND_MAX;

				if(arg_end-arg_begin>0)//If argument 1 exists (random <max>)
					while(true)
						if(read_ptr>arg_begin && (read_ptr>=arg_end || read_ptr[0]==' ')){
							max=decStr2int(arg_begin,MIN(read_ptr-arg_begin,9));
							arg_begin=++read_ptr;

							if(arg_end-arg_begin>0)//If argument 2 exists (random <min> <max>)
								while(true)
									if(read_ptr>arg_begin && (read_ptr>=arg_end || read_ptr[0]==' ')){
										min=max;
										max=decStr2int(arg_begin,MIN(read_ptr-arg_begin,9));
										arg_begin=++read_ptr;
										break;
									}
									else if(read_ptr[0]>='0' && read_ptr[0]<='9')
										++read_ptr;
									else
										break;
							break;
						}
						else if(read_ptr[0]>='0' && read_ptr[0]<='9')
							++read_ptr;
						else
							break;
				int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"%u (%u to %u)",max>min?(value%(max-min+1))+min:value,min,max);
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//Choose
			if(Memory_equals(command.ptr,"choose",6)){
				if(arg_begin>=arg_end){
					irc_send_message(connection,target,locale[language].missing_argument);
					goto SuccessCommand;
				}

				LinkedList* list = LinkedList_init;
				size_t list_length = 0;

				string_splitted(STRINGP(arg_begin,arg_end-arg_begin),function(size_t,(Stringp str){
					if(str.length>=1 && (*str.ptr=='|' || *str.ptr==',')){
						unsigned short whitespaces=1;
						while(str.length>0 && *++str.ptr==' ')
							++whitespaces;
						return whitespaces;
					}
					else
						return 0;
				}),function(bool,(const char* begin,const char* end){
					if(begin >= end)
						return true;

					Stringp* arg = smalloc(sizeof(Stringp));
					*arg = STRINGP(begin,end-begin);
					LinkedList_push(&list,arg);

					++list_length;

					return true;
				}));

				if(list_length>0)
					irc_send_message(connection,target,*(Stringcp*)LinkedList_get(list,rand()%list_length));

				LinkedList_clean(&list,function(bool,(void* elem){
					free(elem);
					return true;
				}));

				goto SuccessCommand;
			}
			//Length
			if(Memory_equals(command.ptr,"length",6)){
				int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"%li",arg_end-arg_begin);
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//Google
			if(Memory_equals(command.ptr,"google",6)){
				int len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),1,
					Stringp_from_cstr("https://www.google.com/search?q=")
				);
				len+=url_encode(STRINGP(arg_begin,arg_end-arg_begin),STRINGP(write_buffer+len,IRC_WRITE_BUFFER_LEN-len));
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//Prefix
			if(Memory_equals(command.ptr,"prefix",6)){
				int len = arg_end-arg_begin,
				    write_len;
				if(len>0){
					free(command_prefix.ptr);
					command_prefix = STRINGP(smalloc(len),len);
					memcpy(command_prefix.ptr,arg_begin,len);
					write_len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),4,
						locale[language].prefix.set,
						STRINGP(" \"",2),
						STRINGP(arg_begin,len),
						STRINGP("\"",1)
					);
				}else{
					write_len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),4,
						locale[language].prefix.get,
						STRINGP(" \"",2),
						command_prefix,
						STRINGP("\"",1)
					);
				}
				irc_send_message(connection,target,STRINGCP(write_buffer,write_len));
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 7:
			//Reverse
			if(Memory_equals(command.ptr,"reverse",7)){
				char* write_ptr = write_buffer;
				read_ptr = arg_end;
				while(read_ptr>arg_begin)
					*write_ptr++=*--read_ptr;
				irc_send_message(connection,target,STRINGCP(write_buffer,arg_end-arg_begin));
				goto SuccessCommand;
			}
			//Version
			if(Memory_equals(command.ptr,"version",7)){
				irc_send_message(connection,target,version_signature);
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 8:
			//Language
			if(Memory_equals(command.ptr,"language",8)){
				for(unsigned int i=0;i<LANG_COUNT;++i)
					if(Memory_equals(locale[i].lang_name.ptr,arg_begin,locale[i].lang_name.length)){
						language=i;
						irc_send_message(connection,target,locale[language].language.set);		
						goto SuccessCommand;
					}
				irc_send_message(connection,target,locale[language].language.unknown);
				goto SuccessCommand;
			}
			//Shutdown
			if(Memory_equals(command.ptr,"shutdown",8))
				exit(0);
			goto UnknownCommand;
		case 9:
			//Word count
			if(Memory_equals(command.ptr,"wordcount",9)){
				unsigned int count=0;
				while(read_ptr<arg_end)
					if(*read_ptr++==' ')
						++count;

				int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"%u words",count);
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			//URL encode
			if(Memory_equals(command.ptr,"urlencode",9)){
				irc_send_message(connection,target,STRINGCP(write_buffer,url_encode(STRINGP(arg_begin,arg_end-arg_begin),STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN))));
				goto SuccessCommand;
			}
			goto UnknownCommand;
		case 10:
			//Magic 8-ball
			if(Memory_equals(command.ptr,"magic8ball",10)){
				unsigned short len=0;
				if(arg_end-arg_begin<2 || *(arg_end-1)!='?'){
					len=locale[language].magic8ball.ask.length;  memcpy(write_buffer,locale[language].magic8ball.ask.ptr,len);
				}else{
					switch(rand()%20){
						case  0:len=locale[language].magic8ball.yes[0].length;  memcpy(write_buffer,locale[language].magic8ball.yes[0].ptr,len);break;
						case  1:len=locale[language].magic8ball.yes[1].length;  memcpy(write_buffer,locale[language].magic8ball.yes[1].ptr,len);break;
						case  2:len=locale[language].magic8ball.yes[2].length;  memcpy(write_buffer,locale[language].magic8ball.yes[2].ptr,len);break;
						case  3:len=locale[language].magic8ball.yes[3].length;  memcpy(write_buffer,locale[language].magic8ball.yes[3].ptr,len);break;
						case  4:len=locale[language].magic8ball.yes[4].length;  memcpy(write_buffer,locale[language].magic8ball.yes[4].ptr,len);break;
						case  5:len=locale[language].magic8ball.yes[5].length;  memcpy(write_buffer,locale[language].magic8ball.yes[5].ptr,len);break;
						case  6:len=locale[language].magic8ball.yes[6].length;  memcpy(write_buffer,locale[language].magic8ball.yes[6].ptr,len);break;
						case  7:len=locale[language].magic8ball.yes[7].length;  memcpy(write_buffer,locale[language].magic8ball.yes[7].ptr,len);break;
						case  8:len=locale[language].magic8ball.yes[8].length;  memcpy(write_buffer,locale[language].magic8ball.yes[8].ptr,len);break;
						case  9:len=locale[language].magic8ball.yes[9].length;  memcpy(write_buffer,locale[language].magic8ball.yes[9].ptr,len);break;
						case 10:len=locale[language].magic8ball.maybe[0].length;memcpy(write_buffer,locale[language].magic8ball.maybe[0].ptr,len);break;
						case 11:len=locale[language].magic8ball.maybe[1].length;memcpy(write_buffer,locale[language].magic8ball.maybe[1].ptr,len);break;
						case 12:len=locale[language].magic8ball.maybe[2].length;memcpy(write_buffer,locale[language].magic8ball.maybe[2].ptr,len);break;
						case 13:len=locale[language].magic8ball.maybe[3].length;memcpy(write_buffer,locale[language].magic8ball.maybe[3].ptr,len);break;
						case 14:len=locale[language].magic8ball.maybe[4].length;memcpy(write_buffer,locale[language].magic8ball.maybe[4].ptr,len);break;
						case 15:len=locale[language].magic8ball.no[0].length;   memcpy(write_buffer,locale[language].magic8ball.no[0].ptr,len);break;
						case 16:len=locale[language].magic8ball.no[1].length;   memcpy(write_buffer,locale[language].magic8ball.no[1].ptr,len);break;
						case 17:len=locale[language].magic8ball.no[2].length;   memcpy(write_buffer,locale[language].magic8ball.no[2].ptr,len);break;
						case 18:len=locale[language].magic8ball.no[3].length;   memcpy(write_buffer,locale[language].magic8ball.no[3].ptr,len);break;
						case 19:len=locale[language].magic8ball.no[4].length;   memcpy(write_buffer,locale[language].magic8ball.no[4].ptr,len);break;
					}
					if(len==0){
						len=locale[language].magic8ball.failure.length;  memcpy(write_buffer,locale[language].magic8ball.failure.ptr,len);
					}
				}
				irc_send_message(connection,target,STRINGCP(write_buffer,len));
				goto SuccessCommand;
			}
			goto UnknownCommand;
	}

	UnknownCommand:{
		int len = Stringp_vcopy(STRINGP(write_buffer,IRC_WRITE_BUFFER_LEN),4,
			locale[language].unknown_command,
			STRINGP(": \"",3),
			command,
			STRINGP("\"",1)
		);
		irc_send_message(connection,target,STRINGCP(write_buffer,len));
	}
	SuccessCommand:
		return;
}

void onMessageFunc(const irc_connection* connection,const irc_message* message){
	switch(message->command_type){
		case IRC_MESSAGE_COMMAND_NUMBER:
			if(message->command_type_number == 1){
				irc_join_channel(connection,"#bot");
				//irc_join_channel(connection,"#toa");
			}
			break;
		case IRC_MESSAGE_COMMAND_PRIVMSG:{
			void onCommandRaw(Stringcp target,char* read_ptr_begin,char* read_ptr_end){
				//Initialize read pointers
				char* read_ptr = read_ptr_begin;

				//Initialize command Stringp
				while(read_ptr<read_ptr_end && *read_ptr!=command_separator)
					++read_ptr;
				Stringcp command = STRINGCP(read_ptr_begin,read_ptr-read_ptr_begin);
				read_ptr_begin=++read_ptr;

				//Commands
				onCommand(connection,command,target,MIN(read_ptr_begin,read_ptr_end),read_ptr_end,message);
			}

			//If private message command
			if(message->command.privmsg.target.length == bot_nickname.length && Memory_equals(message->command.privmsg.target.ptr,bot_nickname.ptr,bot_nickname.length))
				onCommandRaw(STRINGCP_CONST(message->prefix.user.nickname),message->command.privmsg.text.ptr,message->command.privmsg.text.ptr+message->command.privmsg.text.length);

			//If on a channel with a '#' prefix and the private message has the correct prefix
			else if(message->command.privmsg.target.ptr[0] == '#'){
				if(message->command.privmsg.text.length<=command_prefix.length || !Memory_equals(message->command.privmsg.text.ptr,command_prefix.ptr,command_prefix.length))
					break;

				onCommandRaw(STRINGCP_CONST(message->command.privmsg.target),message->command.privmsg.text.ptr+command_prefix.length,message->command.privmsg.text.ptr+message->command.privmsg.text.length);
			}
		}	break;
	}
}

int main(){
	bot_nickname = Stringcp_from_cstr("Toabot");
	version_signature = Stringcp_from_cstr("Flygande Toalett IRC Bot v1.0.4-20131208");

	command_prefix = STRINGP(smalloc(1),1);
	command_prefix.ptr[0]='!';

	//Connect to server
	irc_connection connection=irc_connect("server",1568);
	if(connection.id<0){//Error checking
		fprintf(stderr,"Error: IRC connection id is a negative value: %i\n",connection.id);
		return EXIT_FAILURE;
	}

	irc_set_nickname(&connection,bot_nickname.ptr);
	irc_set_username(&connection,"Bot");

	//While a message is sent from the server
	while(irc_read(&connection,&onMessageFunc));

	//Disconnect connection
	if(!irc_disconnect(&connection)){//Error checking
		fputs("Error: IRC disconnect failed",stderr);
		return EXIT_FAILURE;
	}

	free(command_prefix.ptr);

	return EXIT_SUCCESS;
}
