#include "headers.h"
bool process_finished = false;
void handle_sigusr2(int sig)
{
    if (sig == SIGUSR2) {
        process_finished = true;
    }
}

struct message schedulerMsg;


int main(int argc, char * argv[])
{
    initClk();
    printf("Scheduler started at time %d\n", getClk());
    signal(SIGUSR2, handle_sigusr2);
    struct PCB* current_process = NULL;
    
    //TODO implement the scheduler :(
   int msgid = msgget(MSGKEY, 0666 | IPC_CREAT);
   struct message schedulerMsg;



   while(1)
   {

   int rec_val = msgrcv(msgid, &schedulerMsg, sizeof(struct message)-sizeof(long), 0, IPC_NOWAIT);
   if (rec_val != -1)
   {
    struct PCB newpcb;
    newpcb.id = schedulerMsg.p.id;
    newpcb.arrival_time = schedulerMsg.p.arrival;
    newpcb.runtime = schedulerMsg.p.runtime;
    newpcb.remaining_time = schedulerMsg.p.runtime;
    newpcb.priority = schedulerMsg.p.priority;
    newpcb.state = STATE_ARRIVED;
    enqueue(newpcb);
    printf("Scheduler received process with \nID: %d\t arrival time: %d\n runtime: %d\t priority: %d\n", schedulerMsg.p.id, schedulerMsg.p.arrival, schedulerMsg.p.runtime, schedulerMsg.p.priority);
    
    if(current_process != NULL)
    {
        if(newpcb.priority < current_process->priority && current_process->remaining_time > 0)
        {
            //preempt the current process
            kill(current_process->system_pid, SIGSTOP);
            int time_spent = getClk() - current_process->start_time;
            current_process->remaining_time -= time_spent;
            current_process->state = STATE_STOPPED;
            enqueue(*current_process);
            printf("Process %d PREEMPTED by process %d\n", current_process->id, newpcb.id);
            free(current_process);
            current_process = NULL;
            
        }
    }
   }
   
   
    
    if (process_finished) {
        printf("Process finished at time %d\n", getClk());
        int TAT = getClk() - current_process->arrival_time;
        int WT = TAT - current_process->runtime;
        float WTAT = (float)TAT / current_process->runtime;
        printf("Waiting Time (WT) for process %d: %d\n", current_process->id, WT);
        printf("Turnaround Time (TAT) for process %d: %d\n", current_process->id, TAT);
        printf("Weighted Turnaround Time (WTAT) for process %d: %.2f\n", current_process->id, WTAT);
        process_finished = false; // reset the flag for the next process
        free(current_process); // free the memory allocated for the current process
        current_process = NULL; // set the pointer to NULL after freeing
    }

    if (current_process == NULL && !isqueueEmpty()) {
        struct PCB new_process;
        dequeue(&new_process);
        current_process = (struct PCB*)malloc(sizeof(struct PCB));
        *current_process = new_process;
        if(current_process->state == STATE_ARRIVED)
        {
            int pid = fork();
            if (pid == 0) {
                // In child process : excute the process file
                char remaining_time_str[20];
                char id_str[20];
                sprintf(id_str, "%d", current_process->id);
                sprintf(remaining_time_str, "%d", current_process->remaining_time);
                execl("./process.out", "./process.out", remaining_time_str, id_str, NULL);

            }
            else {
                // In parent process : update the PCB and print the start time
                current_process->system_pid = pid;
                current_process->state = STATE_STARTED;
                current_process->start_time = getClk();
                 printf("Process %d started at time %d\n", current_process->id, getClk());
            }
        }
        else if(current_process->state == STATE_STOPPED)
        {
            kill(current_process->system_pid, SIGCONT);
            current_process->state = STATE_RESUMED;
            current_process->start_time = getClk();
            printf("Time %d: Process %d resumed. Remaining: %d\n", getClk(), current_process->id, current_process->remaining_time);
        }
        

    
    }
    

}
    
    destroyClk(true);
    return 0;
}
