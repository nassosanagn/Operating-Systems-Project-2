#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <sys/time.h>

#include "list.h"

#define MAX_PAGES 10000

typedef struct {
    unsigned long long time;
    int page;
    bool dBit;                /* Used for writes counter */     
    bool write;              /* write = 1 if operation is "W" , if operation is "R" => write = 0 */
    bool full;               /* Indicates if the frame is full or not */
    bool refBit;            /* Used for SecondChance algorithm */
}frame;

/* Get the time in ms */
unsigned long long current_time_in_ms();

/* Page replacement algorithm */
int LRU(frame Ram[],int ramSize);
int SecondChance(frame Ram[], int ramSize);

/* Hash function */
unsigned int hash(unsigned int key, int RamSize);

/* Convert hexademical number to demical */
int hexadecimalToDecimal(char hex[]);

int main(int argc, char **argv){

    FILE *bzip;
    FILE *gcc;
    int replaceIndex;
    int oldPage;

    /* Data */
    int pageFault = 0;
    int reads = 0;
    int writes = 0;

    /* Main arguments (with defaults values) */
    int q = 250;
    bool LRUflag = 1;
    int RamSize = 100;

    /* Open both files */
    gcc = fopen("gcc.trace","r");   
    bzip = fopen("bzip.trace","r");

    /* Get arguments from main */
    for (int i = 1; i < argc; ++i){

        if (i == 1){                                /* First argument is the algorithm that will be used for page replacement */

            if (strcmp(argv[1],"LRU") == 0){
                printf("LRU algorithm will be used for page replacement \n");
                LRUflag = 1;

            }else if  (strcmp(argv[1],"SecondChance") == 0){
                printf("SecondChance algorithm \n");
                LRUflag = 0;
            }
        
        }else if (i == 2){                          /* Second argument is the number of frames in Ram */
            RamSize = atoi(argv[2]);
            printf("Number of frames in Ram: %d\n", RamSize);

        }else if (i == 3){                          /* Third arguments is the number q */
            q = atoi(argv[i]);
            printf("Number q is: %d\n\n", q);
        }
    }

    frame ram[RamSize];
    hashBlock* HashTable[RamSize];

    for(int i = 0; i < RamSize; i++){            /* List is empty in the beginning */
        HashTable[i] = NULL;
        ram[i].full = 0;                       /* Every frame in Ram is empty */
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char* pch;

    int counter = 0;
    char hex[5];
    int number;
    int pages = 0;                  /* number of pages read */

    while(pages < MAX_PAGES){            /* Read until MAX_PAGES */

        if (counter == 2*q){        /* Read q traces from the gcc file and then q traces from the bzip file */
            counter = 0;
        }
        
        /* Read the first Q lines from gcc.trace file */
        if (((read = getline(&line, &len, gcc)) != -1) && counter < q) {

            pch = strtok (line," ");

            strncpy(hex,pch,5);                     /* Get the first 5 digits */
            number = hexadecimalToDecimal(hex);        /* Convert the number from hexademical to demical */

            int hashedPage = hash(number,RamSize);

            pch = strtok (NULL,"  \n");                             /* Get the character 'R' or 'W' */
            
            /* Check if page is already in HashTable */
            if (search_list(HashTable[hashedPage],number)){      /* If page is already in HashTable */

                ram[HashTable[hashedPage]->frame].time = current_time_in_ms();      /* Update time in ram */
                ram[HashTable[hashedPage]->frame].refBit = 1;                       /* Update refBit */

            }else{     /* If page isn't in HashTable */

                pageFault++;
                reads++;

                int ramFull = 1;                        /* A flag to check if Ram is full */

                for(int j = 0; j < RamSize; j++){          /* If there is an available frame in Ram => insert there */
                    
                    if (ram[j].full == 0){                /* Ram isn't full */

                        ramFull = 0;
                        push(&(HashTable[hashedPage]),j,number);           /* push tha new page in the first available frame in Ram */

                        ram[j].page = number;
                        ram[j].full = 1;                                   /* now this frame in ram is full */
                        ram[j].refBit = 0;
                        ram[j].time = current_time_in_ms();
                        ram[j].dBit = 0;
                        
                        if (strcmp(pch,"W") == 0){
                            ram[j].write = 1;
                           
                        }else if (strcmp(pch,"R") == 0){
                            ram[j].write = 0;
                        }
                        break;
                    }
                }

                /* Ram is full => remove a page to insert the new one */
                if (ramFull){

                    if (LRUflag)                             /* If flag == 1 do LRU algorithm */
                        replaceIndex = LRU(ram,RamSize);
                    else                                    
                        replaceIndex = SecondChance(ram,RamSize);      /* Else do SecondChance Algorithm */
                     
                    oldPage = ram[replaceIndex].page;

                    ram[replaceIndex].page = number;                    /* Update Page */
                    ram[replaceIndex].time = current_time_in_ms();      /* Update the time */

                    /* dirty bit == true when operation is "W" in the same address */
                    if ((ram[replaceIndex].dBit == 0) && (strcmp(pch,"W") == 0)){
                        ram[replaceIndex].dBit = 1;
                    }

                    /* When dirty bit == true (from previous "W" operation) then update writes counter by 1 */
                    if (ram[replaceIndex].dBit == 1){
                        writes++;
                        ram[replaceIndex].dBit = 0;
                    }

                    if (strcmp(pch,"W") == 0)
                        ram[replaceIndex].write = 1;
                        
                    else if (strcmp(pch,"R") == 0)
                        ram[replaceIndex].write = 0;

                    remove_by_key(&(HashTable[hash(oldPage,RamSize)]),oldPage);       /* remove old page from HashTable */
                    push(&(HashTable[hashedPage]),replaceIndex,number);           /* push tha new page */
                                      
                }
            } 
        
            counter++;

        /* Read Q lines from bzip.trace file */
        }else if (((read = getline(&line, &len, bzip)) != -1) && (counter >= q)) {

            pch = strtok (line," ");
            strncpy(hex,pch,5);
            number = hexadecimalToDecimal(hex);

            int hashedPage = hash(number,RamSize);

            pch = strtok (NULL,"  \n");                             /* Get the character 'R' or 'W' */
            
            /* Check if page is already in HashTable */
            if (search_list(HashTable[hashedPage],number)){      /* If page is in HashTable */

                ram[HashTable[hashedPage]->frame].time = current_time_in_ms();
                ram[HashTable[hashedPage]->frame].refBit = 1;

            }else{                              /* If page isn't in HashTable */
                pageFault++;
                reads++;
                int ramFull = 1;                        /* A flag to check if Ram is full */

                for(int j = 0; j < RamSize; j++){          /* If there is an available frame in Ram => insert there */
                    
                    if (ram[j].full == 0){          /* Ram isn't full */

                        ramFull = 0;
                        push(&(HashTable[hashedPage]),j,number);           /* push tha new page */

                        ram[j].page = number;
                        ram[j].full = 1;                                   /* now this frame in ram is full */
                        ram[j].refBit = 0;
                        ram[j].time = current_time_in_ms();
                        ram[j].dBit = 0;
                        
                        if (strcmp(pch,"W") == 0){
                            ram[j].write = 1;
                           
                        }else if (strcmp(pch,"R") == 0){
                            ram[j].write = 0;
                        }
                        break;
                    }
                }

                /* Ram is full => remove a page to insert the new one */
                if (ramFull){

                    if (LRUflag)                             /* If flag == 1 do LRU algorithm */
                        replaceIndex = LRU(ram,RamSize);
                    else                                    
                        replaceIndex = SecondChance(ram,RamSize);      /* Else do SecondChance Algorithm */

                    oldPage = ram[replaceIndex].page;

                    ram[replaceIndex].page = number;           /* Update Page */
                    ram[replaceIndex].time = current_time_in_ms();

                    /* dirty bit == true when operation is "W" in the same address */
                    if ((ram[replaceIndex].dBit == 0) && (strcmp(pch,"W") == 0)){
                        ram[replaceIndex].dBit = 1;
                    }

                    /* When dirty bit == true (from previous "W" operation) then update writes counter by 1 */
                    if (ram[replaceIndex].dBit == 1){
                        writes++;
                        ram[replaceIndex].dBit = 0;
                    }

                    if (strcmp(pch,"W") == 0)
                        ram[replaceIndex].write = 1;
                        
                    else if (strcmp(pch,"R") == 0)
                        ram[replaceIndex].write = 0;

                    remove_by_key(&(HashTable[hash(oldPage,RamSize)]),oldPage);       /* remove old page from HashTable */
                    push(&(HashTable[hashedPage]),replaceIndex,number);           /* push tha new page */              
                }
            } 
        
            counter++;
            
        }else
            break;

        pages++;
    }

    printf("The number of pages read: %d \n", MAX_PAGES);
    printf("The number of frames in Ram: %d \n",RamSize);
    printf("Page Faults: %d \n", pageFault);
    printf("Num of reads: %d \n", reads);
    printf("Num of writes: %d \n", writes);

    return 0;
}

/* ---------------------------------------------- Page replacement algorithms ---------------------------------------------- */

int LRU(frame Ram[],int ramSize){

	unsigned long long min = Ram[0].time;              /* Initialize with the first time as min */
    int pos = 0;                             /* Initialize with position 0 */
 
	for(int i = 1; i < ramSize; i++){

		if (Ram[i].time < min){
			min = Ram[i].time;          /* Update new min value */
			pos = i;                    /* Update min index */           
		}
	}

	return pos;
}

int SecondChance(frame Ram[],int ramSize){

    unsigned long long min = Ram[0].time;              /* Initialize with the first time as min */
    int pos = 0;                                       /* Initialize with position 0 */
 
	for(int i = 1; i < ramSize; i++){

        if (Ram[i].refBit == 0){
        
            if (Ram[i].time < min){
                min = Ram[i].time;          /* Update new min value */
                pos = i;                    /* Update min index */           
            }
        }
    }

    for(int j = 0; j < ramSize; j++)
        if ((Ram[j].refBit == 1) && (Ram[j].time < min))
            Ram[j].refBit = 0;

    return pos;
}

/* ---------------------------------------------- Convert hexademical number to demical ---------------------------------------------- */

int hexadecimalToDecimal(char hex[]){

    int i = 0, val, len;
    int decimal = 0;

    len = strlen(hex);
    len--;

    for(i=0; hex[i]!='\0'; i++){
 
    /* Find the decimal representation of hex[i] */
    if(hex[i]>='0' && hex[i]<='9')
    {
        val = hex[i] - 48;
    }
    else if(hex[i]>='a' && hex[i]<='f')
    {
        val = hex[i] - 97 + 10;
    }
    else if(hex[i]>='A' && hex[i]<='F')
    {
        val = hex[i] - 65 + 10;
    }

    decimal += val * pow(16, len);
    len--;
    }

    return decimal;
}

/* ---------------------------------------------- Hash function ---------------------------------------------- */

unsigned int hash(unsigned int key,int RamSize){
    key ^= (key << 13);
    key ^= (key >> 17);    
    key ^= (key << 5); 
    return key%RamSize;
}

/* ---------------------------------------------- Get the time in ms ---------------------------------------------- */
unsigned long long current_time_in_ms() {

    struct timeval te;

    gettimeofday(&te, NULL);                                        /* get the current time */
    unsigned long long ms = te.tv_sec*1000LL + te.tv_usec/1000;    /* calculate milliseconds */
   
    return ms;
}