#include "headers.h"
bool process_finished = false;
void handle_sigusr2(int sig)
{
    if (sig == SIGUSR2) {
        process_finished = true;
    }
}

void print_process(struct PCB process) {
    printf("Process ID: %d\n", process.id);
    printf("Arrival Time: %d\n", process.arrival_time);
    printf("Runtime: %d\n", process.runtime);
    printf("Remaining Time: %d\n", process.remaining_time);
    printf("Priority: %d\n", process.priority);
    printf("State: %d\n", process.state);
}

void print_process_stats(struct PCB* p) {
    printf("Process finished at time %d\n", getClk());
    int TAT = getClk() - p->arrival_time;
    int WT = TAT - p->runtime;

    printf("Waiting Time (WT) for process %d: %d\n", p->id, WT);
    printf("Turnaround Time (TAT) for process %d: %d\n", p->id, TAT);
}


struct message schedulerMsg;


int main(int argc, char * argv[])
{

    int Algorithm = 1;
    int quantum = 0;
    
    //Read the user's choice from the process generator
    if (argc > 1) {
        Algorithm = atoi(argv[1]);
    }
    if (argc > 2) { 
        quantum = atoi(argv[2]); //Assuming 2nd argument is the quantum for RR
    }

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
            printf("Scheduler received process with \nID: %d\t arrival time: %d\n runtime: %d\t priority: %d\n", schedulerMsg.p.id, schedulerMsg.p.arrival, schedulerMsg.p.runtime, schedulerMsg.p.priority);
            
            // PUT IN APPROPRIATE QUEUE BEFORE CLOSING THE BRACKET
            if (Algorithm==1)
            {
                enqueue(newpcb);

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
            if (Algorithm == 2) 
            {
                enqueue_rr(newpcb);
            }
        } // <================ REC_VAL BRACKET CLOSES HERE! =================>
        
        
        
            
            if (process_finished) {
                printf("Process finished at time %d\n", getClk());
                // int TAT = getClk() - current_process->arrival_time;
                // int WT = TAT - current_process->runtime;
                // float WTAT = (float)TAT / current_process->runtime;
                // printf("Waiting Time (WT) for process %d: %d\n", current_process->id, WT);
                // printf("Turnaround Time (TAT) for process %d: %d\n", current_process->id, TAT);
                // printf("Weighted Turnaround Time (WTAT) for process %d: %.2f\n", current_process->id, WTAT);

                print_process_stats(current_process);

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
                    if (pid == 0) 
                        {
                            // In child process : excute the process file
                            char remaining_time_str[20];
                            char id_str[20];
                            sprintf(id_str, "%d", current_process->id);
                            sprintf(remaining_time_str, "%d", current_process->remaining_time);
                            execl("./process.out", "./process.out", remaining_time_str, id_str, NULL);

                        }
                        else 
                        {
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


            if (Algorithm == 2) //RR
            {
                int time_spent;
                if (current_process!=NULL){
                time_spent = getClk() - current_process->start_time;
                }
                
                if ((time_spent >= quantum ) && !process_finished && current_process != NULL) {

                    current_process->remaining_time -= quantum;
                    
                    kill(current_process->system_pid, SIGSTOP);
                    print_process_stats(current_process);
                    enqueue_rr(*current_process);
                    free(current_process);
                    current_process = NULL;
                    


                    //  if (current_process->remaining_time <= 0) { 
                    //    current_process->remaining_time = 0; // Ensure remaining time doesn't go negative

                    // printf("Process finished at time %d\n", getClk());
                    // int TAT = getClk() - current_process->arrival_time;
                    // int WT = TAT - current_process->runtime;
                    // float WTAT = (float)TAT / current_process->runtime;
                    // printf("Waiting Time (WT) for process %d: %d\n", current_process->id, WT);
                    // printf("Turnaround Time (TAT) for process %d: %d\n", current_process->id, TAT);
                    // printf("Weighted Turnaround Time (WTAT) for process %d: %.2f\n", current_process->id, WTAT);        

                    print_process_stats(current_process);

                    process_finished = false; // reset the flag for the next process
                    free(current_process); // free the memory allocated for the current process
                    current_process = NULL; // set the pointer to NULL after freeing

                }

                else if (  current_process != NULL && process_finished) { 

                    current_process->remaining_time = 0;
                    printf("Time %d: Process %d executed for %d units. Remaining: %d\n", getClk(), current_process->id, quantum, current_process->remaining_time);
                    free(current_process);        



                }
            }

   }   
    destroyClk(true);
    return 0;
}