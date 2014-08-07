//
//  main.c
//  CustomDic
//
//  Created by Nick zhang on 14-8-1.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#include <stdio.h>
#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <time.h>
#include "idic.h"
#include "threads.h"

#define MAX_THREAD 80

//some buffers
char prefix[40],suffix[500];
char req[600];
char reply[50000];

struct sockaddr_in name;

char * format={ "%s\n"
    "\t/%s/\t%s\n"
};
const char* program_name;
//some options
int verbose=0;
const char* input_file_path="/Users/Nickzhang/Desktop/input.txt";
const char* output_file_path="/Users/Nickzhang/Desktop/output.txt";


int main(int argc, const char * argv[])
{
 
//parsing the arguments
    
    int next_option;
    
    const char* const short_options="ho:v";
    const struct option long_options[]={
        {"help",0,NULL,'h'},
        {"output",1,NULL,'o'},
        {"verbose",0,NULL,'v'},
        {NULL,0,NULL,0}
    };
    
    program_name=argv[0];
    
    do {
    next_option=getopt_long(argc, argv, short_options, long_options, NULL);
    
        switch (next_option) {
            case 'h':
                print_usage();
                exit(0);
                break;
            case 'o':
                output_file_path=optarg;
                break;
            case 'v':
                verbose=1;
                break;
            case '?':
                print_usage();
                exit(1);
                break;
            case -1:
                break;
            default:
                abort();
                break;
        }
        
    }   while (next_option!=-1);
    
    
    if (argv[optind]!=NULL) {
        input_file_path=argv[optind];
    }
    
    
//do some initialization
    
    
    int rval;
    rval=inet_init();
    if (rval!=0) {
        return 1;
    }
    ps_init();
    

    FILE* input=fopen(input_file_path, "r");
    if (input==NULL) {
        perror("input file");
        exit(1);
    }
    
    FILE* output=fopen(output_file_path, "w");
     if (output==NULL) {
     perror("output file");
     exit(1);
    }
    
    
//the searching process(multithread)
    
    pthread_t tid_holder[MAX_THREAD];
    lookup_t data_holder[MAX_THREAD];
    int this_thread=0;
    char word[20];
    int total_entry=0;
    
    //prevent unexpected memory deallocation
    for (int i=0; i<MAX_THREAD; i++) {
        data_holder[i].buffer=NULL;
    }
    
    time_t start_time=time(NULL);
    
while (1){
    while (1) {
        
        fscanf(input, "%s\n",word);
        
        //ignore empty line
        if (strcmp(word, "")==0) {
            continue;
        }
        //ignore comment line
        if (word[0]=='#'||word[0]==';') {
            continue;
        }
        
        data_holder[this_thread].buffer=malloc(50000);
        memset(data_holder[this_thread].buffer, 0, 50000);
        data_holder[this_thread].errorcode=0;
        data_holder[this_thread].number=this_thread;
        strcpy(data_holder[this_thread].word, word);
        pthread_create(&data_holder[this_thread].id, NULL, lookup, &data_holder[this_thread]);
        tid_holder[this_thread]=data_holder[this_thread].id;
        this_thread++;
        if (this_thread==MAX_THREAD) {
            break;
        }
        
        if (feof(input)) {
            break;
        }
    }
    
    //wait until all request has been answered
    for (int i=0; i<this_thread; i++) {
        rval=pthread_join(tid_holder[i], NULL);
        assert(rval==0);
        if (data_holder[i].errorcode==0) {
            total_entry++;
        }
        fprintf(output, "%s",data_holder[i].buffer);
        fflush(output);
    }
    
    //start another round
    this_thread=0;
    if (feof(input)) {
        break;
    }
}

    for (int i=0;i<MAX_THREAD; i++) {
        if (data_holder[i].buffer!=NULL) {
            free(data_holder[i].buffer);
        }
        
    }
    
    fclose(output);
    fclose(input);
    
    
    time_t end_time=time(NULL);
    time_t interval=end_time-start_time;
    double per_word=(double)interval/total_entry;
    fprintf(stderr, "threads:%d\ntotal word %d\ntotal time:%d\n%f sec per word\n",MAX_THREAD,total_entry,(int)interval,per_word);
    
    
        return 0;
}

