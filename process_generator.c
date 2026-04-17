#include "headers.h"

void clearResources(int);

int msgid;

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
      char* fileName;
  if (argc > 1) {
          fileName = argv[1]; // Use the argument if provided
      } else {
          fileName = "process.txt"; // Default filename
          printf("No file provided, using default: process.txt\n");
      } 


    // 1. Read the input files.
    FILE *file = fopen(fileName, "r");

// if (file == NULL) {
//         perror("FATAL ERROR"); // This prints the exact Linux system error
//         printf("Could not find the file named: '%s'\n", fileName);
//         printf("Check your folder to ensure it isn't named process.txt.txt\n");
//         exit(1); // Safely kill the program instead of segfaulting
//     }

  char line[100];
  struct process processes[100]; 
    int processCount = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
      if (line[0] =='#')  {
            continue; 
      }
      else {    
        

        int id, arrival, runtime, priority;
      sscanf(line, "%d%d%d%d", &id, &arrival, &runtime, &priority);
        processes[processCount].id = id;
        processes[processCount].arrival = arrival;
        processes[processCount].runtime = runtime;
        processes[processCount].priority = priority;
        processes[processCount].remainingTime = runtime;
        processCount++;

      }
    }
    fclose(file);
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    printf("Please choose the scheduling algorithm you want to use:\n");
    printf("1. HPF\n");
    printf("2. RR\n");
    int schedulingAlgorithm;
    scanf("%d",&schedulingAlgorithm);
    int quantumTime = 0;
    if (schedulingAlgorithm == 2) {
        printf("Please enter the quantum time for Round Robin: ");
        scanf("%d", &quantumTime);
        
    }
    char algo_str[10];
  char quantum_str[10];
  sprintf(algo_str, "%d", schedulingAlgorithm);      
  sprintf(quantum_str, "%d", quantumTime);            

    // 3. Initiate and create the scheduler and clock processes.
    int clockProcess = fork();
    if (clockProcess == 0) {
        execl("./clk.out", "./clk.out", NULL);
    }

     sleep(2);
    int schedulerpid = fork();
    if (schedulerpid == 0) {
        execl("./scheduler.out", "./scheduler.out", algo_str, quantum_str, NULL);
    }


   
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop

     msgid = msgget(MSGKEY, 0666 | IPC_CREAT);
    int i = 0;
    while (i < processCount) {
        int currentTime = getClk();
        while(i < processCount && processes[i].arrival <= currentTime) {
            struct message msg;
            msg.mtype = 1;
            msg.p = processes[i];
            msgsnd(msgid, &msg, sizeof(struct process), 0);

            i++;
            
        }
    }





    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
    waitpid(schedulerpid, NULL, 0);
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
     msgctl(msgid, IPC_RMID, NULL);  
      destroyClk(true);              
      exit(0);
}
