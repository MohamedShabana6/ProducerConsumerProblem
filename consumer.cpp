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


struct cdata { //commodity data 
    std::string name;
    double current_price;
    double last_readings[4]; // To calculate the average of the last 5 prices 
    int pcount;       // number of prices recorded
    double average_price;
    int price_flag=0;
    int average_flag=0;
    
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
   if(shmget(shmkey,  sizeof(buffer) + sizeof(comDataBuffer) * (buffer_size), 0666)!=-1)
   {
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
      perror("shmctl");
      exit(1);
   }
   }
	exit(0);
	
	
}




void print_table(cdata commodities[11]) {
    std::cout << "\033[2J\033[1;1H"; //these lines to clear the screen for the output 
    //system("clear");
    printf("\e[1;1H\e[2J"); 
    std::cout<< "+-------------------------------------------+\n";
    std::cout<< "| Commodity       | Price      | AvgPrice  |\n";
    std::cout<< "+-------------------------------------------+\n";

    for (int i = 0; i < 11; i++) { //loop over the array of item structures 
        std::cout<<"|";
        std::cout.width(15);
        std::cout << std::left;
        std::cout << commodities[i].name;
        std::cout << "|";

        if (commodities[i].price_flag ==1) 
        {
            printf(" \033[;32m%-7.2lf↑\033[0m\t|",commodities[i].current_price); //\033[;32m for green color,\033[0m return color to default 
        }
        else if(commodities[i].price_flag==2)
        {
           printf(" \033[;31m%-7.2lf↓\033[0m\t|",commodities[i].current_price); //[;31m: Sets color to red
        }
        else if(commodities[i].price_flag==0)
        {
            printf(" \033[;36m%-7.2lf\033[0m\t|",commodities[i].current_price); //\033[;34m for cyan
        }
        //std::cout.width(11);
        if (commodities[i].average_flag==1)
        {
            printf(" \033[;32m%-7.2lf↑\033[0m\t|\n",commodities[i].average_price);
        }
        else if (commodities[i].average_flag==2)
        {
            printf(" \033[;31m%-7.2lf↓\033[0m\t|\n",commodities[i].average_price);

        }
        else if(commodities[i].average_flag==0)
        {
            printf(" \033[;36m%-7.2lf\033[0m\t|\n",commodities[i].average_price);
        }
        

    }

    std::cout << "+-------------------------------------------+\n";
}


int main(int argc, char* argv[]) {
    cdata commodities[11]; //the items 
  signal(SIGINT,clean_handler);
    const char* commodities_names[11] = {
        "GOLD","SILVER","CRUDEOIL", "NATURALGAS","ALUMINIUM", "COPPER","NICKEL", "LEAD","ZINC", "MENTHAOIL","COTTON"  
    };
    int i;
    for(i = 0; i < 11; i++) { //+=i ?
        commodities[i].name = commodities_names[i];
        commodities[i].current_price = 0.00;
        for (int j = 0; j < 4; j++) {
            commodities[i].last_readings[j] = 0.00;
        }
        commodities[i].pcount=0;
        commodities[i].average_price= 0.00;
        commodities[i].price_flag=0;
        commodities[i].average_flag=0;

    }
    if (argc != 2) {
        std::cerr << "Invalid number of arguments\n";
        return 1;
    }
     buffer_size = atoi(argv[1]);

    //Shared memory for reader process
    shmkey = ftok("shmfile", 65); // generate unique key
    if (shmkey==(key_t)-1){ 
        perror("ftok execution fails\n");
        exit(1);

    }
    size_t b_size = sizeof(buffer) + sizeof(comDataBuffer) * (buffer_size);
    printf("buffer size : %d\n",buffer_size);
    shmid = shmget(shmkey, b_size, 0666 | IPC_CREAT); //returns identifier in shmid , creation for shared memory ,R/W
    if (shmid == -1) {
            perror("creation of shared memory failed\n");
            exit(1);
    }

   
        
  
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
 
    
    
int empty_value = semctl(semsetID, 0, GETVAL);
//printf("empty semaphore value: %d\n", empty_value);

int num_of_items_value = semctl(semsetID, 1, GETVAL);
//printf("number of items semaphore value: %d\n", num_of_items_value);

int mutex_value = semctl(semsetID, 2, GETVAL);
//printf("mutex semaphore value: %d\n", mutex_value);
    
    
     while (1) {
      print_table(commodities);
     //equivalent to sem_wait(num_of_items)/
   sem_op[1].sem_num = 1;
   sem_op[1].sem_op = -1; /* Aloocate the resources  */
   sem_op[1].sem_flg = 0;
   
   semop(semsetID, &sem_op[1], 1);
   
   //equivalent to sem_wait(mutex)/
   sem_op[2].sem_num = 2;
   sem_op[2].sem_op = -1; /* Allocating the resources  */
   sem_op[2].sem_flg = 0;
  
  semop(semsetID, &sem_op[2], 1);
           
           buffer* b = (buffer*)shmat(shmid,(void*)0, 0);
        if (b ==(void*)-1) { 
        //    perror("attachment to shared memory failed");
            exit(1);
        }
       
       std::string name1 = b->commodityData[b->head].nameB; 

        double price = b->commodityData[b->head].current_priceB;
        b->head = (b->head + 1) % buffer_size;
        b->c_count--;
       
   //equivalent to sem_signal(mutex)/
   sem_op[2].sem_num = 2;
   sem_op[2].sem_op = 1; /* Release the resources  */
   sem_op[2].sem_flg = 0;
   
  semop(semsetID, &sem_op[2], 1);

        //the consumed item 
        for(i=0; i<11;i++){
          
            char* name2 = (char*) malloc(sizeof(char)*10);
            strcpy(name2,commodities[i].name.c_str()); //name 2 has the name of the commodity in the commodities array 
            if(strcmp(name2,name1.c_str())==0){
            
                if(price>commodities[i].current_price){
                    commodities[i].price_flag=1; //price increased

                }
                else if (price<commodities[i].current_price){
                    commodities[i].price_flag=2; //decreased
                    
                }
                else{
                    commodities[i].price_flag=0; //no change 
                }
                 // Shift prices and insert the new price
                for (int j = 3; j > 0; j--) { 
                    commodities[i].last_readings[j] = commodities[i].last_readings[j-1]; 
                }
                commodities[i].last_readings[0] = commodities[i].current_price; 

                // Update the current price with the new reading 
                commodities[i].current_price = price;
                if(commodities[i].pcount < 5)
                  {commodities[i].pcount++ ;} //number of readings increased 
                
                double a;//average price calculated
              
                 a=(commodities[i].current_price+commodities[i].last_readings[0]+commodities[i].last_readings[1]+commodities[i].last_readings[2]+commodities[i].last_readings[3])/commodities[i].pcount;
                     
                
                if(a>commodities[i].average_price){
                     commodities[i].average_flag=1; //average  increased
                     
                }
                else if(a<commodities[i].average_price){
                     commodities[i].average_flag=2; //average  decreased

                }
                else{
                    
                     commodities[i].average_flag=0; 
    
               
                }
            
                commodities[i].average_price=a;
            }
    
        }
  
  //equivalent to sem_signal(empty)/
   sem_op[0].sem_num = 0;
   sem_op[0].sem_op = 1; /* release the resources  */
   sem_op[0].sem_flg = 0;
   
   semop(semsetID, &sem_op[0], 1);
    // Detach from shared memory 
    shmdt(b);
    
           usleep(1000000); // 8s delay 

    

   
     
     }
     
    
    
    return 0 ;

}