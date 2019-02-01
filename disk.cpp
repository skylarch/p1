//
//  main.cpp
//  p1
//
//  Created by Skylar Chen on 1/30/19.
//  Copyright © 2019 Skylar Chen. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <vector>
#include "thread.h"
#include "disk.h"
#include "mutex.h"

using std::cout;
using std::cin;
using std::vector;
using std::ifstream;
using std::string;
using std::min;

int MAX_DISK_QUEUE = 0;
int ACT_REQ = 0;
vector<int, int> Q;
char** INPUT = nullptr;
int CUR_ID = 0;

mutex reqid;
mutex queue;
mutex print;

cv fullQ;
cv notFullQ;

void scheduler(void *a);
void requester(void *a);
void servicer(void *a);

void scheduler(void *a){
    for(int i = 0; i < ACT_REQ; i++){
        thread req ((thread_startfunc_t) requester, (void *) 0);
    }
    thread ser ((thread_startfunc_t) servicer, (void *) 0);
}

void requester(void *a){
    reqid.lock();
    int id = ++CUR_ID;
    reqid.unlock();
    
    ifstream fin(INPUT[id+2]);
    string word = "";
    while(fin >> word){
        int track = stoi(word);
        queue.lock();
        while(Q.size() >= MAX_DISK_QUEUE){
            fullQ.wait(queue);
        }
        
        pair<int, int> p(id, track);
        Q.push_back(p);
        print_request(id, track);           //*******optimize?
        notFullQ.signal();
        queue.unlock();
    }
    ACT_REQ--;
}

void servicer(void *a){
    int head = 0;
    
    queue.lock();
    while(Q.size() < min(MAX_DISK_QUEUE, ACT_REQ)){
        notFullQ.wait(queue);
    }
    int short_index = 0;
    for(int i = 0; i < (int)Q.size(); i++){
        if(Q[i].second-head < Q[short_index].second - head){
            short_index = i;
        }
    }
    head = Q[short_index].second;
    print_service(Q[short_index].first, head);
    Q.erase(Q.begin()+short_index);
    fullQ.broadcast();
    queue.unlock();
}

int main(int argc, char **args){
    MAX_DISK_QUEUE = atoi(args[1]);
    ACT_REQ = argc - 2;
    INPUT = args;
    
    cpu::boot((thread_startfunc_t) scheduler, (void *) 0, 0);
    return 0;
}
