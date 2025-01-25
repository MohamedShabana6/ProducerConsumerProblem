#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <ctime>
#include <time.h>
#include <cmath>
#include <math.h>
#include <cstdlib>
#include <stdlib.h>
#include <sys/time.h> 
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <iomanip>
#include <random>
#include <semaphore.h>
#include <fcntl.h>
 
 
#define commodity_length 10
 
struct comDataBuffer{
    char nameB[11];
    double current_priceB;
};
 
struct buffer {
    int c_count;// Number of commodities in the buffer
     int head, tail; // pointers 
    comDataBuffer commodityData[];   

};
 
 
buffer* b;
int buffer_size;
int shmid;
int semsetID;
key_t shmkey;
key_t semkey;
 
void clean_handler(int x) 
{	
   if(semget(semkey, 3, 0666)!=-1)
    {
   if (semctl(semsetID, 0, IPC_RMID) == -1) {
      perror("Remove Semaphore: Semaphore CTL: ");
   }
   }
   if(shmget(shmkey, sizeof(buffer) + sizeof(comDataBuffer) * (buffer_size), 0666)!=-1)
   {
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
      perror("shmctl");
      exit(1);
   }
   }
	exit(0);

}

int main(int argc, char* argv[]) {
 
signal(SIGINT,clean_handler);
    if (argc != 6) {
        std::cerr << "Invalid number of arguments";
        return 1;
    }
 
    std::string commodity_name = argv[1];
    double mean = atof(argv[2]);
    double standard_dev = atof(argv[3]);
    int sleep_interval = atoi(argv[4]);
     buffer_size = atoi(argv[5]);
    double variance = pow(standard_dev,2);
 
 
    if (commodity_name.length() > commodity_length) {
        std::cerr << "Commodity name exceeds 10 characters.\n";
        exit(1); 
    }
//Shared memory for writer process
shmkey = ftok("shmfile", 65); // generate unique key
if (shmkey==(key_t)-1){ 
    perror("ftok execution fails\n");
    exit(1);
 
}
size_t b_size = sizeof(buffer) + sizeof(comDataBuffer) * (buffer_size);
shmid = shmget(shmkey, b_size, 0666 | IPC_CREAT); //returns identifier in shmid , creation for shared memory ,R/W
if (shmid == -1) {
        perror("creation of shared memory failed\n");
        exit(1);
}
 
   
std:: default_random_engine rg;
std::normal_distribution <double> n (mean,standard_dev); //cretes normal distribution object ,generate double point numbers
 
  
int retval;
 
// Create an array of 3 sembuf structures
semkey = ftok("shmfile", 75); // generate unique key
if (semkey==(key_t)-1){ 
    perror("ftok execution fails\n");
    exit(1);
 
}
    struct sembuf sem_op[3];  // 0 empty  1 num_of_items 2 mutex  
    semsetID=semget(semkey, 3, IPC_CREAT |IPC_EXCL| 0666); 
if (semsetID < 0) {
        // The semaphore set already exists, we just need to access it
        semsetID = semget(semkey, 3, 0666); 

    }
else{

retval =semctl(semsetID, 0, SETVAL, buffer_size);//empty , set value buffer_size
if (retval == -1) {
    perror("semctl (emptyID) failed");
    exit(1);
}
 
retval =semctl(semsetID, 1, SETVAL, 0);//num_of_items , set value 0 
if (retval == -1) {
    perror("semctl (num_of_itemsID) failed");
    exit(1);
}
 
retval =semctl(semsetID, 2, SETVAL, 1);//mutex , set value 1 
if (retval == -1) {
    perror("semctl (mutexID) failed");
    exit(1);
}
 
}

struct timespec t;
while(1)
{
     time_t current_time = time(NULL);
     struct tm format = *localtime(&current_time);
     clock_gettime(CLOCK_REALTIME, &t);
     int ns = t.tv_nsec; 
     double random_number = abs(n(rg));
 
    
            std::cerr << "["<<format.tm_mday<<"/"<<format.tm_mon+1<<"/"<<format.tm_year+1900<<" "<<format.tm_hour<<":"<<format.tm_min<<":"<<format.tm_sec<<"."<<ns<<"]";
            std::cerr <<" " <<commodity_name<< ":" <<"generating a new value "<<""<<random_number<<"\n";
 
//equivalent to sem_wait(empty)
   sem_op[0].sem_num = 0;
   sem_op[0].sem_op = -1; /* Allocating the resources  */
   sem_op[0].sem_flg = 0;
   semop(semsetID, &sem_op[0], 1);

 
            
            std::cerr << "["<<format.tm_mday<<"/"<<format.tm_mon+1<<"/"<<format.tm_year+1900<<" "<<format.tm_hour<<":"<<format.tm_min<<":"<<format.tm_sec<<"."<<ns<<"]";
            std::cerr <<" " <<commodity_name<< ":" <<"trying to get mutex on shared buffer\n";
  //equivalent to sem_wait(mutex)/
   sem_op[2].sem_num = 2;
   sem_op[2].sem_op = -1; /* Allocating the resources  */
   sem_op[2].sem_flg = 0;
  semop(semsetID, &sem_op[2], 1);
           
 
  int empty_value = semctl(semsetID, 0, GETVAL);
 // printf("empty semaphore value: %d\n", empty_value);
  int mutex_value = semctl(semsetID, 2, GETVAL);
 // printf("mutex semaphore value: %d\n", mutex_value);

   b = (buffer*)shmat(shmid,(void*)0, 0);
    if (b ==(void*)-1) { 
   //     perror("attachment to shared memory failed");
        exit(1);
    }
   
                                 
            std::cerr << "["<<format.tm_mday<<"/"<<format.tm_mon+1<<"/"<<format.tm_year+1900<<" "<<format.tm_hour<<":"<<format.tm_min<<":"<<format.tm_sec<<"."<<ns<<"]";
            std::cerr <<" " <<commodity_name<< ":" <<"placing "<<random_number<<" on shared buffer\n";

        // Place the price and commodity name in the shared buffer
         b->commodityData[b->tail].current_priceB= random_number; //price
 
        std::strcpy(b->commodityData[b->tail].nameB,commodity_name.c_str());
    
 
        b->tail = (b->tail + 1) % buffer_size; //update the tail pointer to the next position 
        b->c_count++; //increment no of commidities in buffer
          
   //equivalent to sem_signal(mutex)/
   sem_op[2].sem_num = 2;
   sem_op[2].sem_op = 1; /* Release the resources  */
   sem_op[2].sem_flg = 0;
  semop(semsetID, &sem_op[2], 1);
 
   //equivalent to sem_signal(num_of_items)/
   sem_op[1].sem_num = 1;
   sem_op[1].sem_op = 1; /* Release the resources  */
   sem_op[1].sem_flg = 0;
   semop(semsetID, &sem_op[1], 1);


     std::cerr << "["<<format.tm_mday<<"/"<<format.tm_mon+1<<"/"<<format.tm_year+1900<<" "<<format.tm_hour<<":"<<format.tm_min<<":"<<format.tm_sec<<"."<<ns<<"]";
     std::cerr <<" " <<commodity_name<< ":" <<"sleeping for "<<sleep_interval<<"ms\n"; 
    shmdt(b);
    usleep(sleep_interval*1000);// input is in micro so multiply by 100 to convert to milli

}


return 0;
}