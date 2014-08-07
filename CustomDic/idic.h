//
//  idic.h
//  CustomDic
//
//  Created by Nick zhang on 14-8-1.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#ifndef idic_h
#define idic_h

#include "threads.h"

//typedef char bool;
extern char* learner_key;
extern char prefix[40],suffix[500];
extern char reply[50000];
extern char req[600];
extern int verbose;
extern struct sockaddr_in name;
extern const char* program_name;
extern char * format;


int inet_init();
int ps_init();
int mk_req(char* word);
int send_req(int socket_fd);
int str_sub(char* buffer,char* from,char* to);
int str_rtv(char* in_buffer,char* out_buffer,char* key);
int str_rtvr(char* in_buffer,char* out_buffer,char* start,char* end);
int rtv_def(char* in_buffer,char* out_buffer);
int rtv_entry(char* in_buffer,char* out_buffer,char* word);
int print_ety(char* this_entry,FILE* output,char* word);

int mk_req_trd(char* word,char* buffer);
int req_trd(lookup_t* data);
int print_ety_trd(char* this_entry,lookup_t* data);
void msg_trd(lookup_t* data,char* message);


void print_usage();
void msg(char* message);
#endif
