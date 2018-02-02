#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include<ctype.h>
#include "common.h"
#include "usr_functions.h"

#define ALPHABET_COUNT 26
#define MAX_FILE_SIZE 5000

/* User-defined map function for the "Letter counter" task.  
   This map function is called in a map worker process.
   @param split: The data split that the map function is going to work on.
   Note that the file offset of the file descripter split->fd should be set to the properly
   position when this map function is called.
   @param fd_out: The file descriptor of the itermediate data file output by the map function.
   @ret: 0 on success, -1 on error.
   */


int letter_counter_map(DATA_SPLIT * split, int fd_out)
{
    int count[ALPHABET_COUNT];
    for(int i=0;i<ALPHABET_COUNT;++i){
        count[i] = 0;
    }

    char * buf = calloc(split->size,sizeof(char));
    read(split->fd,buf,split->size);

    for(int i=0;i<split->size;++i){
        int curr = (int)buf[i];
        if((97 <= curr) && (curr <= 122)){
            count[curr-32-65] = count[curr-32-65]+1;
        }
        else if((65 <= curr) && (curr <= 90)){
            count[curr-65] = count[curr-65]+1;
        }
    }
    free(buf);
    char buf2[MAX_FILE_SIZE] = {'\0'}; 

    for(int i=0;i<ALPHABET_COUNT;++i){

        char temp[50];
        sprintf(temp,"%c %d\n",(char)65+i,count[i]);
        strcat(buf2,temp);

    }

    int ret = write(fd_out,buf2,strlen(buf2));
    if(ret == -1){
        perror("Error while writing to file");
        exit(0);
    }
    close(fd_out);

    return 0;
}

/* User-defined reduce function for the "Letter counter" task.  
   This reduce function is called in a reduce worker process.
   @param p_fd_in: The address of the buffer holding the intermediate data files' file descriptors.
   The imtermeidate data files are output by the map worker processes, and they
   are the input for the reduce worker process.
   @param fd_in_num: The number of the intermediate files.
   @param fd_out: The file descriptor of the final result file.
   @ret: 0 on success, -1 on error.
   @example: if fd_in_num == 3, then there are 3 intermediate files, whose file descriptor is 
   identified by p_fd_in[0], p_fd_in[1], and p_fd_in[2] respectively.

*/
int letter_counter_reduce(int * p_fd_in, int fd_in_num, int fd_out)
{
    int count[ALPHABET_COUNT];
    for(int i=0;i<ALPHABET_COUNT;++i){
        count[i] = 0;
    }

    for(int i=0;i<fd_in_num;++i){
        
        int sz = lseek(p_fd_in[i], 0L, SEEK_END);
        lseek(p_fd_in[i],0L,SEEK_SET);

        char* buf;
        buf = calloc(sz+1,sizeof(char));
    

        read(p_fd_in[i],buf,sz+1);

        int i=0;
        char num[100];
        int j=0;
        int charNum = 0;
        while(1){
            if(buf[i] == '\0'){
                break; 
            }
            else if(buf[i] == '\n'){
                num[j] = '\0';
                int countCurr = atoi(num);
                count[charNum] +=countCurr;
                charNum++;
                j=0;
            }
            else{
                if(isdigit(buf[i])!=0){ //is digit
                    num[j] = buf[i];    
                    j++;
                }
            }
            i++;
        }
        free(buf);
    }

    char * buf2 = malloc(MAX_FILE_SIZE);
    buf2[0] = '\0';

    for(int i=0;i<ALPHABET_COUNT;++i){
        char temp[100];
        sprintf(temp,"%c %d\n",(char)65+i,count[i]);
        strcat(buf2,temp);
    }


    int ret = write(fd_out,buf2,strlen(buf2));
    if(ret == -1){
        perror("Error while writing to file");
        exit(0);
    }
    free(buf2);

    return 0;
}

/* User-defined map function for the "Word finder" task.  
   This map function is called in a map worker process.
   @param split: The data split that the map function is going to work on.
   Note that the file offset of the file descripter split->fd should be set to the properly
   position when this map function is called.
   @param fd_out: The file descriptor of the itermediate data file output by the map function.
   @ret: 0 on success, -1 on error.
   */
int word_finder_map(DATA_SPLIT * split, int fd_out)
{
    int fd_in = split->fd;
    int size = split->size+2;

    char * buf = calloc(size,sizeof(char));
    *buf = '\0';
    read(fd_in,buf,split->size);

    int mallocedSize = 100;
    char* line = calloc(mallocedSize,sizeof(char));

    int j=0;
    int i=0;
    for(i=0;i<size-1;i++){
        if(*(buf+i) == '\n'){

            if(i==(mallocedSize-1)){
                line=realloc(line,mallocedSize*2);
                mallocedSize *=2;
            }

            line[j]='\n';
            line[j+1] = '\0';
        
            // check if given word appers here
            char* ret = strstr(line,split->usr_data);
            if(ret != NULL){
                // Write to output file
                write(fd_out,line,strlen(line));
            }

            j=0;
        }
        else{
            line[j] = buf[i];
            j++;
        }
    }
    free(line);
    free(buf);
    
    return 0;
}

/* User-defined reduce function for the "Word finder" task.  
   This reduce function is called in a reduce worker process.
   @param p_fd_in: The address of the buffer holding the intermediate data files' file descriptors.
   The imtermeidate data files are output by the map worker processes, and they
   are the input for the reduce worker process.
   @param fd_in_num: The number of the intermediate files.
   @param fd_out: The file descriptor of the final result file.
   @ret: 0 on success, -1 on error.
   @example: if fd_in_num == 3, then there are 3 intermediate files, whose file descriptor is 
   identified by p_fd_in[0], p_fd_in[1], and p_fd_in[2] respectively.

*/
int word_finder_reduce(int * p_fd_in, int fd_in_num, int fd_out)
{

    char *temp = malloc(sizeof(char)*2);
    for(int i=0;i<fd_in_num;++i){
        while(1){
            int ret = read(p_fd_in[i],temp,1);
            if(ret == 0){
                break;
            }
            write(fd_out,temp,1);
        }
    }
    
    free(temp);
    return 0;
}


