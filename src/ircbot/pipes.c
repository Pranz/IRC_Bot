#include "pipes.h"
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

struct PipedStream p2open(char* path,char* const argv[]){
	/**
	 *       |--------|
	 * Input |  Pipe  | Output
	 *  end  |--------|  end
	 *          <-->
	 * When no end is closed, data can flow
	 * in both directions. E.g. if the out-
	 * put end is closed, data can only
	 * enter through the input end and flow
	 * out from the output end from the 
	 * parent process' perspective.
	 * ====================================
	 *             ___________
	 * |---------||           ||---------|
	 * | inPipe  ||  Program  || outPipe |
	 * |---------||___________||---------|
	 *     -->     Process text    -->
	 */

	pid_t pid;
	int inPipe[2];
	int outPipe[2];

	//Create pipes
	if(pipe(inPipe)!=0 || pipe(outPipe)!=0){
		fprintf(stderr,"Creation of pipes failed\n");
		return (struct PipedStream){NULL,NULL};
	}

	//Create child process
	pid=fork();

	if(pid==(pid_t)0){
		//Child process (Subprocess)
		
		//Close other ends
		close(inPipe[1]);
		close(outPipe[0]);

		dup2(inPipe[0],STDIN_FILENO);
		dup2(outPipe[1],STDOUT_FILENO);
		close(inPipe[0]);
		close(outPipe[1]);

		if(execv(path,argv)<0){
			fprintf(stderr,"execv failed\n");
			return (struct PipedStream){NULL,NULL};
		}
	}else if(pid<(pid_t)0){
		//Forking failed
		fprintf(stderr,"Fork failed\n");
	}else{
		//Parent process (Current process)
		
		//Close input pipe reading end. Able to write to the input end
		close(inPipe[0]);
		//Close output pipe writing end. Able to read from the output end
		close(outPipe[1]);

		return (struct PipedStream){
			fdopen(inPipe[1],"w"),
			fdopen(outPipe[0],"r")
		};
	}
	return (struct PipedStream){NULL,NULL};
}

void p2close(struct PipedStream stream){
	int inPipe  = fileno(stream.in);
	int outPipe = fileno(stream.out);

	close(inPipe);
	close(outPipe);
}

extern void p2flushWrite(struct PipedStream stream);
extern void p2flushRead(struct PipedStream stream);

/*int main(void){
	char* argv[]={"rev",NULL};
	struct PipedStream stream=p2open("/usr/bin/rev",argv);

	fprintf(stream.in,"The shit");
	p2flushWrite(stream);

	fputs("Received: \"",stdout);
	int c;while((c=fgetc(stream.out))!=EOF)
		putchar(c);
	putchar('"');
	putchar('\n');
	p2flushRead(stream);

	p2close(stream);

	return 0;
}*/
