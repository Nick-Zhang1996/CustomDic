//
//  threads.c
//  CustomDic
//
//  Created by Nick zhang on 14-8-6.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//


#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include "threads.h"
#include "idic.h"

void* lookup(void * data){

    int rval;
    //for code compatability
    lookup_t* mydata=(lookup_t*)data;
    char* word=mydata->word;
    char* reply=mydata->buffer;


    
    //search the word via predifined API
    mk_req_trd(mydata->word, mydata->buffer);
    req_trd(mydata);
    
    
               //if there is no entry
        if (strstr(reply, "<suggestion>")) {
            sprintf(mydata->buffer, "%s---no such entry\n\n",word);


            mydata->errorcode=1;
            return mydata;
        }
        
        // limit the decode in the right entry
        char this_entry[30000];
        rval=rtv_entry(mydata->buffer, this_entry, word);
        
        if (!rval) {
            // there is only one entry,print it
            print_ety_trd(this_entry, mydata);

        }   else{
            //multiple entries, print them one by one
            int i=1;
            
            char entryid[20];
            char id[5];
            sprintf(id, "[%d]",i);
            strcpy(entryid, word);
            strcat(entryid, id);
            rval=rtv_entry(reply, this_entry, entryid);
            
            if (rval) {
                //retrive the first entry, regardless of the real entry id, this is designed to handle a derivitive being searched,such as -ing or -ed
                str_rtvr(reply, word, "<entry id=\"", "\">");
                rval=rtv_entry(reply, this_entry, word);
                if (rval!=0) {
                    sprintf(mydata->buffer, "%s---no such entry\n\n",word);
                    mydata->errorcode=1;
                    return mydata;
                }
                print_ety_trd(this_entry, mydata);
                //skit the next while to prevent duplicated entry
                //not a graceful handle!!!
                rval=1;
            }
            
            
            while (!rval) {
                print_ety_trd(this_entry, mydata);
                i++;
                sprintf(id, "[%d]",i);
                strcpy(entryid, word);
                strcat(entryid, id);
                rval=rtv_entry(reply, this_entry, entryid);
            }
        }
        //end if there are no more words
    strcat(mydata->buffer, "\n");

    
    return mydata;
}