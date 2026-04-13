#include "headers.h"

struct message schedulerMsg;


int main(int argc, char * argv[])
{
    initClk();
    printf("Scheduler started at time %d\n", getClk());
    
    //TODO implement the scheduler :(
   int msgid = msgget(MSGKEY, 0666 | IPC_CREAT);
    msgrcv(msgid, &schedulerMsg, sizeof(struct message)-sizeof(long), 0, 0);
    printf("Scheduler received process with ID: %d\n", schedulerMsg.p.id);
    printf("Scheduler received process with arrival time: %d\n", schedulerMsg.p.arrival);
    printf("Scheduler received process with runtime: %d\n", schedulerMsg.p.runtime);
    printf("Scheduler received process with priority: %d\n", schedulerMsg.p.priority);
    //upon termination release the clock resources.
    
    destroyClk(true);
}
