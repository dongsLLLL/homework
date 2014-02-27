//
//  main.cpp
//  os lab2
//
//  Created by Junye Zhu on 2/19/14.
//  Copyright (c) 2014 Junye Zhu. All rights reserved.
//

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <queue>
#include <list>

//#include "Events.h"
//#include "FCFS.h"
//#include "LCFS.h"
//#include "SJF.h"
//#include "Scheduler.h"
//#include "Processes.h"
//#include "RoundRobin.h"

using namespace std;
int timestamp=0;
int process_termination_no=0;
double cpu_time=0;
double io_time=0;
vector<int> randvals;
int ofs;

struct process{
    int id;
//input
    int at; //Arrival Time
    int tc; //Total CPU Time
    int cb; //CPU Burst
    int io; //IO Burst
//parameter
    int tc_remain; //the remaining CPU Time
    int at_new; //the latest arrival time to ready queue
    int generate_time;// the event generted time
    int finish_time; //the predicted finishing time
//output
    int ft; //finishing time
    int tt; //ft-at
    int it; //time in blocked state
    int cw; //time in ready state
};
process running,running_finished;
list<process> ready_queue;
list<process> block_queue;
list<process> block_ready_queue;
vector<process> all_processes;

//the random time of running cpu or block
int cpu_run_or_block_time(int burst)
{
  return 1 + (randvals[ofs++] % burst); 
}

//a provess is finished and print it out
void process_termination_or_block (){
    if (running_finished.finish_time!=0)
    {
        if (running_finished.finish_time<=timestamp) {
            if (running_finished.tc_remain ==0) {
                running_finished.ft=running_finished.finish_time;
                running_finished.tt=running_finished.ft-running_finished.at;
                printf("%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n",process_termination_no,running_finished.at,running_finished.tc,running_finished.cb,running_finished.io,running_finished.ft,running_finished.tt,running_finished.it,running_finished.cw);
                /*cout<<process_termination_no<<":    "<<running_finished.at<<"    "<<running_finished.tc<<"    "<<running_finished.cb<<"    "<<running_finished.io<<" |  "<<running_finished.ft<<"    "<<running_finished.tt<<"    "<<running_finished.it<<"    "<<running_finished.cw<<endl;*/
                process_termination_no++;
            }
            else {
                int burst = cpu_run_or_block_time(running_finished.io);
                running_finished.it += burst;
                running_finished.finish_time = timestamp + burst;
                running_finished.generate_time=timestamp;
                block_queue.push_back(running_finished);
            }
        }
    }
}

//cpu run the current process
void cpu_run () {
    int burst = cpu_run_or_block_time(running.cb);
    if (running.tc_remain<burst) burst=running.tc_remain;
    cpu_time +=burst;
    running.tc_remain -= burst;
    running.finish_time = timestamp + burst;
}

//grab process from the ready queue
void ready_to_run (){
    process p;
    if (!ready_queue.empty()) {
        p=ready_queue.front();
        ready_queue.pop_front();
        if (p.at_new < timestamp) {
            cout << p.at_new << "*****" << timestamp << endl;
            p.cw += (timestamp-p.at_new);
        }
        p.generate_time=timestamp;
        running =p;
        cpu_run();
    }
}

//solve block finished process and running process
void solve_block_and_running_finished (){
    if (running.finish_time!=0&&running.finish_time<=timestamp) {
        running_finished=running;
        running.finish_time =0;
        if (!block_ready_queue.empty()) {
          list<process>::iterator it=block_ready_queue.begin();
          while (it != block_ready_queue.end()){
            if ((*it).generate_time<running_finished.generate_time) {
              ready_queue.push_back(*(it++));
              block_ready_queue.pop_front();
              if (running.finish_time ==0) {
                  ready_to_run();
              }
            }
            else{
              process_termination_or_block();
              it++;
            }
          }
        }
        else process_termination_or_block();
    }
    else if (running.finish_time>timestamp){
      if (!block_ready_queue.empty()) {
        list<process>::iterator it=block_ready_queue.begin();
        while (it != block_ready_queue.end()){
            ready_queue.push_back(*(it++));
            block_ready_queue.pop_front();
        }
      }
    } 
    else{
      if (!block_ready_queue.empty()) {
        list<process>::iterator it=block_ready_queue.begin();
        while (it != block_ready_queue.end()){
            ready_queue.push_back(*(it++));
            block_ready_queue.pop_front();
        }
      }
      ready_to_run();
    }
}

//check if there is any process in block list has done its io bound
void check_block_list (){
    if (!block_queue.empty()) {
        list<process>::iterator it=block_queue.begin();
        while (it != block_queue.end()){
          if ((*it).finish_time <= timestamp){
            (*it).at_new=timestamp;
            list<process>::iterator it2=block_ready_queue.end();
            block_ready_queue.splice(it2, block_queue, it); 
            it++;
          }
        }
    }
}

//main function
int main(int argc, char** argv) {
    if (argc != 2){
        cout << "Wrong Number of arguments, exiting ..." << endl;
        exit(0);
    }
    
    //read the input file
    string file_name=argv[1];
	  ifstream infile;
    infile.open (file_name.c_str());
   	if (!infile.is_open()){
        cout << "Error Opening File, Please make sure correct input file name typed" << endl;
        exit(0);
    }
    int at,tc,cb,io;
    int process_id=1;
    string line;
    while (!infile.eof()) {
      getline(infile, line);
      if (line.length()>0){
        istringstream iss(line);
        iss >> at >> tc >> cb >> io;
        //get a new process
        process p;
        p.id=process_id++;
        p.at=at;
        p.tc=tc;
        p.cb=cb;
        p.io=io;
        p.tc_remain = p.tc;
        p.at_new = p.at;
        p.finish_time=0; 
        p.ft=0; 
        p.tt=0;
        p.it=0;
        p.cw=0; 

        all_processes.push_back(p);
      }
    }

    //read the random value from random file
    ifstream rfile;
    rfile.open ("rfile");
    if (!rfile.is_open()){
        cout << "Error Opening \"rfile\", Please make sure correct input file name typed" << endl;
        exit(0);
    }
    int random;
    while (!rfile.eof()){
      rfile >> random;
      randvals.push_back(random);
    }
    
    cout<<"FCFS"<<endl;
    int index = 0;
    while (true) {
        //begin reading the input file
        while (index < all_processes.size()){
          process p = all_processes[index];
          if (p.at == timestamp){
            ready_queue.push_back(p);
            index++; 
          }
          break;
        }

        //begin
        check_block_list();
        solve_block_and_running_finished();
        if (!block_queue.empty()) {
          io_time++;
        }
        timestamp++;
        if (block_queue.empty()&&ready_queue.empty()&&infile.eof()&&running.finish_time==0&&index==all_processes.size()) {
            printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",running_finished.ft,(double)cpu_time/running_finished.ft*100,(double)io_time/running_finished.ft*100,0.00,0.00,0.000);
            break;
        }
    }
}

