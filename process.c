#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    remainingtime = atoi(argv[1]);
    initClk();
    
    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;


    int lasttime = getClk();
     
    while (remainingtime > 0)
    {
        int current_time = getClk();
        if (current_time > lasttime) {
            remainingtime--;
            lasttime = current_time;
        }
        
        // remainingtime = ??;
    }
    kill(getppid(), SIGUSR2); //notify the scheduler that the process has finished
    
    destroyClk(false);
    
    return 0;
}
