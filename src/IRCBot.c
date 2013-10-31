#include <stdio.h> //Input/output
#include <unistd.h>//Unix standard library

#include "String.h"
#include "irc.h"

/**
 * Buffer initialization
 */
char write_buffer[512];

void onMessageFunc(irc_connection_id id,const irc_message* message){
	switch(message->command_type){
		case IRC_MESSAGE_COMMAND_NUMBER:
			if(message->command_type_number == 1){
				irc_join_channel(id,"#bot");
				irc_join_channel(id,"#toa");
			}
			break;
		case IRC_MESSAGE_COMMAND_PRIVMSG:
			if(message->command.privmsg.target.chrs[0] == '#' && message->command.privmsg.text.chrs[0] == '.'){
				if(message->command.privmsg.text.len==5 && strneq(message->command.privmsg.text.chrs+1,"dice",4)){
					write_buffer[0]=rand()%6+'1';
					irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,1));
				}else if(message->command.privmsg.text.len==5 && strneq(message->command.privmsg.text.chrs+1,"test",4)){
					for(int i=0;i<5;++i)
						irc_send_message(id,message->command.privmsg.target,STRING("ABCDEF",6));
				}else if(message->command.privmsg.text.len>6 && strneq(message->command.privmsg.text.chrs+1,"echo ",5)){
						irc_send_message(id,message->command.privmsg.target,STRING(message->command.privmsg.text.chrs+6,message->command.privmsg.text.len-6));
				}else{
					int len = sputstring(STRING(write_buffer,IRC_BUFFER_LENGTH),2,
						STRING("Unknown command: ",17),
						STRING(message->command.privmsg.text.chrs+1,message->command.privmsg.text.len-1)
					);
					irc_send_message(id,message->command.privmsg.target,STRING(write_buffer,len));
				}
			}
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
