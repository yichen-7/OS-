#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MSGKEY 77


// Enum to define all possible states of a process in the system
enum ProcessState {
    STATE_ARRIVED,
    STATE_STARTED,
    STATE_STOPPED,
    STATE_RESUMED,
    STATE_FINISHED
};

// Process Control Block (PCB) structure to hold all process information
struct PCB {
    int id;                  // Process ID from the input file
    int system_pid;          // Actual PID returned by fork() when the process starts
    int arrival_time;        // The time the process arrived at the scheduler
    int runtime;             // Total execution time required by the process
    int remaining_time;      // Time left for the process to finish execution
    int waiting_time;        // Total time the process spent waiting in the ready queue
    int priority;            // Priority of the process (0 is the highest priority)
    int start_time;          // The time the process started execution
    enum ProcessState state; // Current state of the process
};





struct process
    {
       
        int id;
        int priority;
        int arrival;
        int runtime;
        int remainingTime;
        
    };

    struct message
    {
        long mtype;
        struct process p;
    };

///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}


/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }


    
    
}

//!============================== Priority Queue Implementation ==============================!//
typedef struct Node
{
    struct PCB process;
    struct Node* next;
} Node;

Node *head = NULL;
bool isqueueEmpty()
{
    return head == NULL;
}

bool enqueue(struct PCB process)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->process = process;
    newNode->next = NULL;

    if (isqueueEmpty() || head->process.priority > process.priority)
    {
        newNode->next = head;
        head = newNode;
        return true;
    }

    Node* current = head;
    while (current->next != NULL && current->next->process.priority <= process.priority)
    {
        current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;
    return true;
}
bool dequeue(struct PCB* process)
{
    if (isqueueEmpty())
    {
        return false;
    }
    Node* temp = head;
    *process = head->process;
    head = head->next;
    free(temp);
    return true;
}
struct PCB peek()
{
    if (isqueueEmpty())
    {
        struct PCB emptyPCB;
        emptyPCB.id = -1; // Indicate an empty PCB
        return emptyPCB;
    }
    return head->process;
}
//!============================== End of Priority Queue Implementation ==============================!//

bool enqueue_rr(struct PCB process)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->process = process;
    newNode->next = NULL;

    if (isqueueEmpty())
    {
        newNode->next = head;
        head = newNode;
        return true;
    }

    Node* current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    newNode->next = current->next;
    current->next = newNode;
    return true;
}



float total_wta = 0;
float total_wta_sq = 0;
int total_processes_count = 0;


void print_process_stats(struct PCB* p) {
    printf("Process finished at time %d\n", getClk());
    int TAT = getClk() - p->arrival_time;
    int WT = TAT - p->runtime;
    // Calculate WTA for current process
    float current_wta = (float)TAT / p->runtime; 

    // Update totals
    total_wta += current_wta;
    total_wta_sq += (current_wta * current_wta);
    total_processes_count++;

    
    printf("Waiting Time (WT) for process %d: %d\n", p->id, WT);
    printf("Turnaround Time (TAT) for process %d: %d\n", p->id, TAT);
}



void print_process(struct PCB process) {
    printf("Process ID: %d\n", process.id);
    printf("Arrival Time: %d\n", process.arrival_time);
    printf("Runtime: %d\n", process.runtime);
    printf("Remaining Time: %d\n", process.remaining_time);
    printf("Priority: %d\n", process.priority);
    printf("State: %d\n", process.state);
}


float get_std_wta() {
    if (total_processes_count == 0) return 0.0;
    
    double avg_wta = total_wta / total_processes_count;
    // قانون التباين: (مجموع المربعات / العدد) - (مربع المتوسط)
    double variance = (total_wta_sq / total_processes_count) - (avg_wta * avg_wta);
    
    // منعاً لأي أخطاء حسابية لو التباين طلع برقم سالب ضئيل جداً نتيجة التقريب
    if (variance < 0) variance = 0;
    
    return sqrt(variance);
}