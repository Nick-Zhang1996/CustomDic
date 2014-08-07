//
//  threads.h
//  CustomDic
//
//  Created by Nick zhang on 14-8-6.
//  Copyright (c) 2014年 Nick Zhang. All rights reserved.
//

#ifndef CustomDic_threads_h
#define CustomDic_threads_h



typedef struct {
    int number;
    char word[20];
    char* buffer;
    pthread_t id;
    int errorcode;
    
} lookup_t;

void* lookup(void * data);


#endif
