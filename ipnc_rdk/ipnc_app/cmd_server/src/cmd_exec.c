#include <pthread.h>
#include <sem_util.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/vfs.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/types.h>
#include <sys/ioctl.h>
#include "cmd_server.h"




int SYSTEM(char *arg)
{
    int numArg,i,j,k,len,status;
    pid_t chId;
    char exArg[20][64];

    if(arg[0] == '\0')
        return 0;

    VI_DEBUG("\n>>>> cmd arg is %s\n",arg);

    j   = 0;
	k   = 0;
	len = strlen(arg);

    for(i = 0;i < len;i ++)
    {
        if(arg[i] == ' ')
        {
		    exArg[j][k] = '\0';
		    j ++;
		    k = 0;
		}
		else
		{
		    exArg[j][k] = arg[i];
		    k ++;
		}
	}

    if(exArg[j][k - 1] == '\n')
    {
	    exArg[j][k - 1] = '\0';
	}
	else
	{
	    exArg[j][k] = '\0';
	}

	numArg = j + 1;

    for(i = 0;i < numArg;i ++)
    {
	    VI_DEBUG(">>>> arg %d = %s\n",i,exArg[i]);
	}

    //SemWait(gFORKSem);
    chId = vfork();
    //SemRelease(gFORKSem);

    if(chId == 0)
    {
	    // child process
	    VI_DEBUG(">>>> in child process with %d args\n",numArg);
	    switch(numArg)
	    {
		    case 1:
		        execlp(exArg[0],exArg[0],NULL);
		        break;
		    case 2:
		        execlp(exArg[0],exArg[0],exArg[1],NULL);
		        break;
		    case 3:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],NULL);
		        break;
		    case 4:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],NULL);
		        break;
		    case 5:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],NULL);
		        break;
		    case 6:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],NULL);
		        break;
		    case 7:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],NULL);
		        break;
		    case 8:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],NULL);
		        break;
		    case 9:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],NULL);
		        break;
		    case 10:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],NULL);
		        break;
		    case 11:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],exArg[10],NULL);
		        break;
		    case 12:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],exArg[10],exArg[11],NULL);
		        break;
		    case 13:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],exArg[10],exArg[11],exArg[12],NULL);
		        break;
		    case 14:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],exArg[10],exArg[11],exArg[12],exArg[13],NULL);
		        break;
		    case 15:
		        execlp(exArg[0],exArg[0],exArg[1],exArg[2],exArg[3],exArg[4],exArg[5],exArg[6],exArg[7],exArg[8],exArg[9],exArg[10],exArg[11],exArg[12],exArg[13],exArg[14],NULL);
		        break;
		}

	    VI_DEBUG(">>>> execlp failed\n");
	    exit(0);
	}
	else if(chId < 0)
	     {
		     VI_DEBUG(">>>> vfork() failed\n");
		     return -1;
		 }
		 else
		 {
		     // parent process
		     VI_DEBUG(">>>> in parent process\n");

		     // wait for the completion of the child process
		     waitpid(chId,&status,0);

		     VI_DEBUG(">>>> child exited\n");
		 }

    return 0;
}

