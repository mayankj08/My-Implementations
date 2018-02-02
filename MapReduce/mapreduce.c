#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mapreduce.h"
#include "common.h"
#include <string.h>
#include <unistd.h>
#include "usr_functions.h"
#include <fcntl.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/wait.h>

#define LINE_MAX 10

#include <dirent.h> 
#include <stdio.h> 

int getDir(void)
{
    DIR           *d;
    struct dirent *dir;
    d = opendir(".");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }

        closedir(d);
    }

    return(0);
}

void closeFds(int* p_fd_in,int fd_in_num){
    for(int i=0;i<fd_in_num;++i){
        int ret = close(p_fd_in[i]);
        if(ret == -1){
            perror("Error while closing file descriptor");
            exit(0);
        }

    }
}

long getFileLineCount(char* filePath){
    FILE *fp;
    char length[LINE_MAX];
    char *command = malloc(strlen(filePath)+10);    
    sprintf(command,"wc -l %s",filePath);
    fp = popen(command,"r");
    if(fp == NULL){
        perror("Failed in fopen");
        exit(0);
    }

    int i=0;
    while (1){
        char ch = fgetc(fp);
        if(ch == ' ')
            break;

        length[i] = ch;
        i++;
    }
    length[i] = '\0';

    long lines = atol(length);

    int status = pclose(fp);
    if (status == -1) {
        perror("Error while closing fp");
        exit(0);
    }
    free(command);

    return lines;
}

void mapreduce(MAPREDUCE_SPEC * spec, MAPREDUCE_RESULT * result)
{

    struct timeval start, end;

    if (NULL == spec || NULL == result)
    {
        EXIT_ERROR(ERROR, "NULL pointer!\n");
    }

    gettimeofday(&start, NULL);

    // Get length of the file
    long lines = getFileLineCount(spec->input_data_filepath); 

    // Now start split number of process and make them work on input
    off_t currOffset = 0;
    long lineIncrement = (long)(lines/spec->split_num);

    FILE* fp;
    fp = fopen(spec->input_data_filepath,"r");
    if(fp == NULL){
        perror("fopen failed");
        exit(0);
    }

    int fd_pipe[2];

    {
        int ret = pipe(fd_pipe);
        if(ret == -1){
            perror("Error while opening file");
            exit(0);
        }
    }

    for(int i=0;i<spec->split_num;++i){
        // Calculate offset for next child
        off_t startOffset = currOffset;
        off_t endOffset;
        int temp = 0;
        while(temp != lineIncrement){
            char ch = '\0';
            do{
                ch = fgetc(fp);
            }while(ch!= '\n');

            temp++;
        }

        endOffset = ftell(fp);
        endOffset = endOffset-1;
        if(endOffset == -1){
            perror("ftell failed");
            exit(0);
        }

        currOffset = endOffset+1;

        //fprintf(stderr, "Start offset:%jd End offset:%jd\n",startOffset,endOffset);

        // In case this is the last process
        // then offset should be end of file.
        if(i == (spec->split_num-1)){
            int ret = fseek(fp,0,SEEK_END);
            if(ret == -1){
                perror("Error while seeking to end");
                exit(0);
            }
            endOffset = ftell(fp);

            if(endOffset == -1){
                perror("ftell failed");
                exit(0);
            }
        }


        // Open the file which child would be writing to.
        char * filename = malloc(10);
        sprintf(filename,"mr-%d.itm",i);
        int fd_out = open(filename,O_RDWR|O_TRUNC|O_CREAT,0640);
        if(fd_out == -1){
            perror("Error in opening File");
            exit(0);
        }
        free(filename);


        // Now create worker process
        int ret = fork();

        // Child will enter here. Let's exec the child
        if(ret == 0){
            fclose(fp);

            // If this is first worker process then close
            // write end of pipe for this process
            // else close read end
            if(i==0){
                close(fd_pipe[1]);
            }
            else{
                close(fd_pipe[0]);
            }

            DATA_SPLIT dat;
            int fd_temp = open(spec->input_data_filepath,O_RDONLY);
            int ret1  = lseek(fd_temp,startOffset,SEEK_SET);

            if(ret1 == -1){
                perror("Error in lseek");
                exit(0);
            }

            dat.fd = fd_temp;
            dat.size = endOffset-startOffset+1;
            dat.usr_data = spec->usr_data;

            spec->map_func(&dat,fd_out); 

            close(fd_temp);

            // If this worker ins first worker 
            // then this process needs to run
            // reduce too
            // Other's just close their respective  write 
            // end of pipe and exit the process
            if(i==0){
                // Read from pipe so that this process
                // gets blocked till all the 
                char * temp = malloc(1);
                read(fd_pipe[0],temp,1);
                close(fd_pipe[0]);
                free(temp);
                // Call reduce now

                // Open file to write to
                int fd_out_final;
                fd_out_final =open("mr.rst",O_WRONLY|O_TRUNC|O_CREAT,0640); 

                int* p_fd_in = malloc(sizeof(int)*spec->split_num);

                // Open all the input files in read mode
                for(int j=0;j<spec->split_num;++j){
                    char * filename = malloc(10);
                    sprintf(filename,"mr-%d.itm",j);
                    p_fd_in[j] = open(filename,O_RDONLY);
                    if(p_fd_in[j] == -1){
                        //getDir();
                        fprintf(stderr,"Failed in file %s %zd\n",filename,strlen(filename));
                        sleep(50);
                        perror("Error in opening file");
                        exit(0);
                    }
                    free(filename);
                }
                spec->reduce_func(p_fd_in,spec->split_num,fd_out_final);

                // Close all the open fds
                closeFds(p_fd_in,spec->split_num);

                free(p_fd_in);
                {
                    int ret = close(fd_out_final); 
                    if(ret == -1){
                        perror("Error while closing fd");
                        exit(0);
                    }
                }
            }
            else{
                close(fd_pipe[1]);
            }

            free(result->map_worker_pid);
            _exit(0); //to avoid fork bomb..
        }
        else if(ret > 0){ //main process
            close(fd_out);

            // Either pid in map_worker_pid
            result->map_worker_pid[i] = ret;
            if(i==0){
                result->reduce_worker_pid = ret;
            }
        }
        else{
            perror("Error in fork");
            exit(0);
        }
    }

    // main process don't need pipe at all
    close(fd_pipe[0]);
    close(fd_pipe[1]);
    fclose(fp);

    for(int i=0;i<spec->split_num;++i){
        wait(NULL);   
    }

    result->filepath = "mr.rst";
    gettimeofday(&end, NULL);   
    result->processing_time = (end.tv_sec - start.tv_sec) * US_PER_SEC + (end.tv_usec - start.tv_usec);
}
