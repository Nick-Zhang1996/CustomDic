//
//  idic.c
//  CustomDic
//
//  Created by Nick zhang on 14-8-1.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#include <stdio.h>
#include "idic.h"
#include "threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
//not thread safe
int inet_init(){

    
    struct hostent * hostinfo;

    
    name.sin_family=AF_INET;
    msg("initializing network, resolving host..." );
    
    hostinfo=gethostbyname("www.dictionaryapi.com");
    if (hostinfo==NULL) {
        herror(NULL);
        for (int i=0; i<4; i++) {
        
            fprintf(stderr, "cannot get host,retrying in 5 seconds...%d\n",i+1);
            hostinfo=gethostbyname("www.dictionaryapi.com");
            if (hostinfo!=NULL) {
                break;
            }
            
        }
        
        if (hostinfo==NULL) {
            fprintf(stderr, "cannot resolve host!\n");
            herror(NULL);
            return 1;
        }
    }
    
    name.sin_addr=*((struct in_addr*) hostinfo->h_addr);;
    name.sin_port=htons(80);
    
    msg("resolved!");
    return 0;
}



//shared by all threads,not thread safe
int ps_init(){
    msg("initializing prefix&suffix...");
    int prefix_fd=open("/Users/Nickzhang/Desktop/prefix.txt", O_RDONLY);
    if (prefix_fd==-1){
        fprintf(stderr, "error opening file");
        return 1;
    }
    ssize_t p_size=read(prefix_fd, prefix, 40);
    close(prefix_fd);
    
    int suffix_fd=open("/Users/Nickzhang/Desktop/suffix.txt", O_RDONLY);
    if (suffix_fd==-1){
        fprintf(stderr, "error opening file");
        close(prefix_fd);
        return 1;
    }
    
    p_size=read(suffix_fd, suffix,500);
    close(suffix_fd);
    msg("done!");
    return 0;
}

//not thread safe
int mk_req(char* word){
    memset(req, 0, 600);
    strcpy(req, prefix);
    strcat(req, word);
    strcat(req, suffix);
    return 0;
}

//thread safe
int mk_req_trd(char* word,char* buffer){
    memset(buffer, 0, 600);
    strcpy(buffer, prefix);
    strcat(buffer, word);
    strcat(buffer, suffix);
    return 0;
    
}

//not thread safe
int send_req(int socket_fd){
    ssize_t byte_read;
    int total_read=0;
    int rval;
    msg("sending request...");
    socket_fd=socket(PF_INET, SOCK_STREAM, 0);
    assert(socket_fd!=-1);
    
    
    rval=connect(socket_fd, (const struct sockaddr *)&name, sizeof(struct sockaddr_in));
    if (rval==-1) {
        perror("connect");
        
        for (int i=0; i<3; i++) {
            sleep(2);
            fprintf(stderr, "reconnecting...");
            rval=connect(socket_fd, (const struct sockaddr *)&name, sizeof(struct sockaddr_in));
            if (rval!=-1) {
                break;
            }
            if (rval==-1) {
                close(socket_fd);
                fprintf(stderr, "error connecting!");
                return 1;
            }
        }
        
    }
    
    write(socket_fd, req,strlen(req));
    msg("done!");
    msg("waiting for relpy...");
    //sleep(1);
    
    byte_read=read(socket_fd, reply, 1000);
    total_read+=byte_read;
    
    while (byte_read!=0) {
        byte_read=read(socket_fd, reply+total_read, 1000);
        total_read+=byte_read;
        assert(total_read<50000);
    }

    close(socket_fd);
    msg("done!");
    return 0;
}


int req_trd(lookup_t* data){
    ssize_t byte_read;
    int total_read=0;
    int rval;
    msg_trd(data, "sending request...");
    
    int socket_fd=socket(PF_INET, SOCK_STREAM, 0);
    //!!!no assert for real release
    assert(socket_fd!=-1);
    
    
    rval=connect(socket_fd, (const struct sockaddr *)&name, sizeof(struct sockaddr_in));
    if (rval==-1) {
        
        perror("connect");
        
        for (int i=0; i<3; i++) {
            sleep(2);
            fprintf(stderr, "reconnecting...");
            rval=connect(socket_fd, (const struct sockaddr *)&name, sizeof(struct sockaddr_in));
            if (rval!=-1) {
                break;
            }
            if (rval==-1) {
                close(socket_fd);
                fprintf(stderr, "error connecting!");
                return 1;
            }
        }
        
    }
    
    write(socket_fd,data->buffer,strlen(data->buffer));
    msg_trd(data, "waiting for relpy...");
    
    byte_read=read(socket_fd, data->buffer, 1000);
    total_read+=byte_read;
    
    while (byte_read!=0) {
        byte_read=read(socket_fd, data->buffer+total_read, 1000);
        total_read+=byte_read;
        //not graceful!!!
        if (total_read>50000) {
            break;
        }
    }
    
    //just in case
    *(data->buffer+total_read)='\0';
    
    close(socket_fd);
    msg_trd(data,"reply received");
    return 0;
}

//safe
int str_sub(char* buffer,char* from,char* to){
    char* from_start=strstr(buffer, from);
    if (from_start==NULL) {
        return 1;
    }
    int offset=(int)strlen(to)-(int)strlen(from);
   
        if (offset>0)
            for (char* i=buffer+strlen(buffer); i>=from_start+strlen(from); i--) {
            *(i+offset)=*i;}
        else if (offset<0)
            for (char* i=from_start+strlen(from); i<=buffer+strlen(buffer); i++) {
                *(i+offset)=*i;}
        
        
    
    
    strncpy(from_start,to,strlen(to));
    return 0;
}

//buggy!!!
int str_rtv(char* in_buffer,char* out_buffer,char* key){
    char start[10],end[10];
    sprintf(start, "<%s>",key);
    sprintf(end, "</%s>",key);
    int rval=str_rtvr(in_buffer, out_buffer,start,end);
    return rval;
}

int str_rtvr(char* in_buffer,char* out_buffer,char* start,char* end){
    char* p_start;
    char* p_end;
    p_start=strstr(in_buffer, start);
    
    if (p_start==NULL) {
        //can not find a match
        return 1;
    }
    //start searching from P_START to avoid corruption(find END before START)
    p_end=strstr(p_start,end);
    if (p_end==NULL) {
        return 1;
    }
    p_start+=strlen(start);
    
    long length=p_end-p_start;
    assert(length>0);
    
    char* temp=stpncpy(out_buffer,p_start,length);
    *temp='\0';
    return 0;
}

int rtv_def(char* in_buffer,char* out_buffer){
    
    int rval=str_rtvr(in_buffer, out_buffer,"", "<vi>");
    if (rval) {
        strcpy(out_buffer, in_buffer);
    }
        return 0;
    
}

void msg(char* message){
    if (verbose) {
        fprintf(stderr, message);
        fprintf(stderr, "\n");
    }
    
    return;
}

int rtv_entry(char* in_buffer,char* out_buffer,char* word){
    char* start[40];
    char* end="</entry>";
    sprintf(start, "<entry id=\"%s\">",word);
    int rval=str_rtvr(in_buffer, out_buffer, start, end);
    return rval;
    
}

int print_ety(char* this_entry,FILE* output,char* word){
//print the word, pronounciation ,and attribute
    char pr[50];
    int rval;
    rval=str_rtv(this_entry, pr, "pr");
    str_sub(pr, "<it>", "\"");
    str_sub(pr, "</it>", "\"");
    if (rval) {
        pr[0]='\0';
    }

    char fl[50];
    rval=str_rtv(this_entry, fl, "fl");
    if (rval) {
        fl[0]='\0';
    }
 


    fprintf(output,format,word,pr,fl);

    char dt[1000];
    rval=str_rtv(this_entry, dt, "dt");
    assert(rval==0);



    //enumerate every definition in the entry,print example for each
    while (str_rtv(this_entry, dt, "dt")==0) {
        rval=str_sub(this_entry, "dt", "xx");
        assert(rval==0);
        
        //print definition
        char this_def[300];
        memset(this_def, 0, 300);
        rval=rtv_def(dt, this_def);
        assert(rval==0);
        
        //change line if there are more than one definition
        //+2 to skip the first ":"
        str_sub(this_def+2, ":", "\n\n:");
        fprintf(output,"\t%s\n",this_def);
        
        //print examples,at most 3
        int i=1;
        char vi[200];
        while (str_rtv(dt, vi, "vi")==0 && i<=3) { //list no more than three sample sentences
            
            while (str_sub(vi, "<it>", "<"));
            while (str_sub(vi, "</it>", ">"));
            
            fprintf(output,"\t   ->%s\n",vi);
            str_sub(dt, "<vi>", "xxxx");
            str_sub(dt, "</vi>", "xxxxx");
            i++;
        }
    }


    fflush(output);
    
    return 0;

}

int print_ety_trd(char* this_entry,lookup_t* data){
    //print the word, pronounciation ,and attribute
    char pr[50];
    ssize_t byte_wrote=strlen(data->buffer);
    int rval;
   
    while (!str_sub(this_entry, "<it>", "\"")){;}
    while (!str_sub(this_entry, "</it>", "\"")){;}
    while (!str_sub(this_entry, "<wsgram>", "(")){;}
    while (!str_sub(this_entry, "</wsgram>", ")")){;}
    
     rval=str_rtv(this_entry, pr, "pr");
    if (rval) {
        pr[0]='\0';
    }
    
    char fl[50];
    rval=str_rtv(this_entry, fl, "fl");
    if (rval) {
        fl[0]='\0';
    }
    
    
    
    sprintf(data->buffer,format,data->word,pr,fl);
    byte_wrote=strlen(data->buffer);
    
    char dt[1000];
   
    //enumerate every definition in the entry,print example for each
    while (str_rtv(this_entry, dt, "dt")==0) {
        rval=str_sub(this_entry, "dt", "xx");
        assert(rval==0);
        rval=str_sub(this_entry, "dt", "xx");
        assert(rval==0);
        
        //print definition
        char this_def[300];
        memset(this_def, 0, 300);
        rval=rtv_def(dt, this_def);
        assert(rval==0);
        
        //change line if there are more than one definition
        //+2 to skip the first ":"
        str_sub(this_def+2, ":", "\n\t:");
        sprintf(data->buffer+byte_wrote,"\t%s\n",this_def);
        byte_wrote=strlen(data->buffer);
        
        //print examples,at most 3
        int i=1;
        char vi[200];
        while (str_rtv(dt, vi, "vi")==0 && i<=3) { //list no more than three sample sentences
            
            while (!str_sub(vi, "<it>", "<"));
            while (!str_sub(vi, "</it>", ">"));
            
            sprintf(data->buffer+byte_wrote,"\t   ->%s\n",vi);
            byte_wrote=strlen(data->buffer);
            
            str_sub(dt, "<vi>", "xxxx");
            str_sub(dt, "</vi>", "xxxxx");
            i++;
        }
    }
    return 0;
    
}

void msg_trd(lookup_t* data,char* message){
    
    if (verbose) {
        fprintf(stderr, "thread:%d\t \'%s\':",data->number,data->word);
        fprintf(stderr, "%s\n",message);
        
    }
    return;
}

void print_usage(){
    
    const char* usage="Usage: %s options [inputfile]\n"
                    "   -h  --help          Display this usage information\n"
                    "   -o  --output        Write output to file\n"
                    "   -v  --verbose       Print verbose messages\n";
    
    fprintf(stdout, usage,program_name);
}
