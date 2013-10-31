#include <stdio.h> //Input/output
#include <string.h>
#include <unistd.h>//Unix standard library

#include "String.h"
#include "irc.h"

/**
 * Buffer initialization
 */
#define IRC_WRITE_BUFFER_LEN 512
char write_buffer[IRC_WRITE_BUFFER_LEN];
char command_prefix = '!',
     command_separator = ' ',
     command_arg_separator = ' ';

void onMessageFunc(irc_connection_id id,const irc_message* message){
	switch(message->command_type){
		case IRC_MESSAGE_COMMAND_NUMBER:
			if(message->command_type_number == 1){
				irc_join_channel(id,"#bot");
				irc_join_channel(id,"#toa");
			}
			break;
		case IRC_MESSAGE_COMMAND_PRIVMSG:
			//If on a channel with a '#' prefix and the private message has a : prefix
			if(message->command.privmsg.target.chrs[0] == '#' && message->command.privmsg.text.chrs[0] == command_prefix){
				//Initialize read pointers
				char* read_ptr = message->command.privmsg.text.chrs+1,
				    * read_ptr_begin = read_ptr,
				    * read_ptr_end = message->command.privmsg.text.chrs+message->command.privmsg.text.len;

				//Initialize command string
				while(read_ptr<read_ptr_end && *read_ptr!=command_separator)
					++read_ptr;
				string command = STRING(read_ptr_begin,read_ptr-read_ptr_begin);
				read_ptr_begin=++read_ptr;

				//Commands
				switch(command.len){
					case 4:
						//Bool
						if(strneq(command.chrs,"bool",4)){
							irc_send_message(id,message->command.privmsg.target,rand()%2?STRING("true",4):STRING("false",5));
							goto SuccessCommand;
						}
						//Dice
						if(strneq(command.chrs,"dice",4)){
							write_buffer[0]=rand()%6+'1';
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,1));
							goto SuccessCommand;
						}
						//Test
						if(strneq(command.chrs,"test",4)){
							for(int i=0;i<5;++i)
								irc_send_message(id,message->command.privmsg.target,STRING("ABCDEF",6));
							goto SuccessCommand;
						}
						//Echo
						if(strneq(command.chrs,"echo",4)){
							irc_send_message(id,message->command.privmsg.target,STRING(read_ptr_begin,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						//Wikipedia
						if(strneq(command.chrs,"wiki",4)){
							int len = sputstring(STRING(write_buffer,IRC_WRITE_BUFFER_LEN),2,
								STRING("http://en.wikipedia.org/wiki/Special:Search?search=",51),
								STRING(read_ptr_begin,read_ptr_end-read_ptr_begin)
							);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						//IMDb
						if(strneq(command.chrs,"imdb",4)){
							int len = sputstring(STRING(write_buffer,IRC_WRITE_BUFFER_LEN),2,
								STRING("http://www.imdb.com/find?s=all&q=",33),
								STRING(read_ptr_begin,read_ptr_end-read_ptr_begin)
							);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						goto UnknownCommand;
					case 5:
						//Upper
						if(strneq(command.chrs,"upper",5)){
							char* write_ptr = write_buffer;
							while(read_ptr<read_ptr_end){
								*write_ptr++=*read_ptr>='a' && *read_ptr<='z'?(*read_ptr)+('A'-'a'):*read_ptr;
								++read_ptr;
							}

							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						//Lower
						if(strneq(command.chrs,"lower",5)){
							char* write_ptr = write_buffer;
							while(read_ptr<read_ptr_end){
								*write_ptr++=*read_ptr>='A' && *read_ptr<='Z'?(*read_ptr)+('a'-'A'):*read_ptr;
								++read_ptr;
							}

							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						//ROT13
						if(strneq(command.chrs,"rot13",5)){
							char* write_ptr = write_buffer;
							while(read_ptr<read_ptr_end){
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

							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						//ROT47
						if(strneq(command.chrs,"rot47",5)){
							char* write_ptr = write_buffer;
							while(read_ptr<read_ptr_end){
								if(*read_ptr>='!' && *read_ptr<='~')
									*write_ptr++=((*read_ptr)-'!'+47)%94+'!';
								else
									*write_ptr++=*read_ptr;
								++read_ptr;
							}

							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						goto UnknownCommand;
					case 6:
						//Random
						if(strneq(command.chrs,"random",6)){
							int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"Random: %u (0 to %u)",rand(),RAND_MAX);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						//Choose
						if(strneq(command.chrs,"choose",6)){
							irc_send_message(id,message->command.privmsg.target,STRING("Not implemented yet",19));
							goto SuccessCommand;
						}
						//Length
						if(strneq(command.chrs,"length",6)){
							int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"%li",read_ptr_end-read_ptr_begin);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						//Google
						if(strneq(command.chrs,"google",6)){
							int len = sputstring(STRING(write_buffer,IRC_WRITE_BUFFER_LEN),2,
								STRING("https://www.google.com/search?q=",32),
								STRING(read_ptr_begin,read_ptr_end-read_ptr_begin)
							);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						goto UnknownCommand;
					case 7:
						//Reverse
						if(strneq(command.chrs,"reverse",7)){
							char* write_ptr = write_buffer;
							read_ptr = read_ptr_end;
							while(read_ptr>read_ptr_begin)
								*write_ptr++=*--read_ptr;
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,read_ptr_end-read_ptr_begin));
							goto SuccessCommand;
						}
						goto UnknownCommand;
					case 9:
						//Word count
						if(strneq(command.chrs,"wordcount",9)){
							unsigned int count=0;
							while(read_ptr<read_ptr_end)
								if(*read_ptr++==' ')
									++count;

							int len=snprintf(write_buffer,IRC_WRITE_BUFFER_LEN,"Word Count: %u words",count);
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						goto UnknownCommand;
					case 10:
						//Magic 8-ball
						if(strneq(command.chrs,"magic8ball",10)){
							unsigned short len=0;
							if(read_ptr_end-read_ptr_begin<2 || *(read_ptr_end-1)!='?'){
								len=20;memcpy(write_buffer,"Ask a question first",len);
							}else{
								switch(rand()%20){
									case  0:len=13;memcpy(write_buffer,"It is certain"            ,len);break;
									case  1:len=18;memcpy(write_buffer,"It is decidedly so"       ,len);break;
									case  2:len=15;memcpy(write_buffer,"Without a doubt"          ,len);break;
									case  3:len=14;memcpy(write_buffer,"Yes definitely"           ,len);break;
									case  4:len=18;memcpy(write_buffer,"You may rely on it"       ,len);break;
									case  5:len=15;memcpy(write_buffer,"As I see it yes"          ,len);break;
									case  6:len=11;memcpy(write_buffer,"Most likely"              ,len);break;
									case  7:len=12;memcpy(write_buffer,"Outlook good"             ,len);break;
									case  8:len= 3;memcpy(write_buffer,"Yes"                      ,len);break;
									case  9:len=18;memcpy(write_buffer,"Signs point to yes"       ,len);break;
									case 10:len=20;memcpy(write_buffer,"Reply hazy try again"     ,len);break;
									case 11:len=15;memcpy(write_buffer,"Ask again later"          ,len);break;
									case 12:len=23;memcpy(write_buffer,"Better not tell you now"  ,len);break;
									case 13:len=18;memcpy(write_buffer,"Cannot predict now"       ,len);break;
									case 14:len=25;memcpy(write_buffer,"Concentrate and ask again",len);break;
									case 15:len=17;memcpy(write_buffer,"Don't count on it"        ,len);break;
									case 16:len=14;memcpy(write_buffer,"My reply is no"           ,len);break;
									case 17:len=17;memcpy(write_buffer,"My sources say no"        ,len);break;
									case 18:len=19;memcpy(write_buffer,"Outlook not so good"      ,len);break;
									case 19:len=13;memcpy(write_buffer,"Very doubtful"            ,len);break;
								}
								if(len==0){
									len=9;memcpy(write_buffer,"Try again",len);
								}
							}
							irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
							goto SuccessCommand;
						}
						goto UnknownCommand;
				}

				UnknownCommand:{
					int len = sputstring(STRING(write_buffer,IRC_WRITE_BUFFER_LEN),2,
						STRING("Unknown command: ",17),
						command
					);
					irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
				}
			}
			SuccessCommand:
			break;
	}
}

int main(){
	irc_connection_id connection_id=irc_connect("flygande-toalett.tk",1568);
	irc_set_nickname(connection_id,"Toabot");
	irc_set_username(connection_id,"Bot");

	char read_buffer[IRC_BUFFER_LENGTH+1];
	int read_len;

	//While a message is sent from the server
	while((read_len = read(connection_id,read_buffer,IRC_BUFFER_LENGTH))){
		if(read_len<0){//Error checking
			fprintf(stderr,"Error: read() returned negative value: %i\n",read_len);
			return EXIT_FAILURE;
		}

		//Print the raw message that was received
		fputsn(read_buffer,read_len,stdout);

		irc_parse_message(connection_id,STRING(read_buffer,read_len),&onMessageFunc);
	}	
	return EXIT_SUCCESS;
}
