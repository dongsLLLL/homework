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
int ofs=0;
//flag for running
//  0 : there is no running process right now
//  1 : there is running process right now
int RUNNING=0; 
 
//quantum for robin round algotithm
int quantum=-1;

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

  //for robin
  // 0 : current burst finished
  // 1 : still in current cpu burst
  int signal;
  int burst_remain;
};

process running, running_prepare;
list<process> ready_queue;
list<process> block_queue;
vector<process> all_processes;

//solutions
int FCFS=1;
int LCFS=2;
int STJ=3;
int ROBIN=4;
int TYPE=-1;

//the random time of running cpu or block
int cpu_run_or_block_time(int burst){
  return 1 + (randvals[ofs++] % burst); 
}

void finish_process(){
  running.ft=running.finish_time;
  running.tt=running.ft-running.at;
  process_termination_no++;

  //set finished processes
  all_processes[running.id] = running;
  RUNNING=0;
}

//block running process for IO burst
void block_process(){
  int burst = cpu_run_or_block_time(running.io);
  cout << "IO burst------Running:  " << running.id << "   Timestamp:  " << timestamp << "   Burst:  " << burst << endl;
  running.it += burst;
  running.finish_time = timestamp + burst;
  running.generate_time=timestamp;
  running.signal=0;
  block_queue.push_back(running);
  RUNNING=0;
}

void robin_round_finished(){
  ready_queue.push_back(running);
}

//check running process if it has been finished or got blocked
void stop_process (){
  if (RUNNING==1) {
    //current event finished
    if (running.finish_time<=timestamp) {
      if (running.tc_remain ==0) {
        finish_process();
      } else {
        block_process();
      }
    }
  }
}

//grab process from the ready queue
void pick_up_running_process (){
  //********************* STJ *****************************
  if (TYPE==STJ){
    get the process with smallest tc_remain
    list<process>::iterator next_running_process=ready_queue.begin();
    for (list<process>::iterator it=ready_queue.begin(); it!= ready_queue.end(); ++it) {
      if ((*it).tc_remain < (*next_running_process).tc_remain) {
        next_running_process=it;
      }
    }
    running=*next_running_process;
    ready_queue.erase(next_running_process);
  }

  //****************** FCFS or Robin Round ***************
  if (TYPE==FCFS || TYPE==ROBIN){
    running=ready_queue.front();
    ready_queue.pop_front();
  }

  //****************** LCFS *****************************
  if (TYPE==LCFS){
    running=ready_queue.back();
    ready_queue.pop_back();
  }

  //set the cpu burst
  int burst;
  //to skip robin round 
  if (running.signal==0){
    burst = cpu_run_or_block_time(running.cb);
    if (running.tc_remain<burst) burst=running.tc_remain;
    burst_remain = burst;
    running.signal=1;
  }

  if (TYPE==ROBIN)
    if (burst_remain>quantum){
      burst=quantum;
    }else{
      burst=burst_remain;
    }
  }
  cpu_time += burst;

  cout << "CPU burst------Running:  " << p.id << "   Timestamp:  " << timestamp << "   Burst:  " << burst << endl;

  //set running process
  running.generate_time=timestamp;
  if (running.at_new < timestamp) {
    running.cw += (timestamp-running.at_new);
  }
  running.tc_remain -= burst;
  running.finish_time = timestamp + burst;

  //set running flag
  RUNNING=1;
}

//solve block finished process and running process
void check_ready_queue_and_running (){
  //current cpu burst of running process has been finished
  if (RUNNING==1 && running.finish_time == timestamp){
    //stop running process and pick up a new running process
    if (!ready_queue.empty()){
      stop_process ();
      pick_up_running_process();
    }
    //no process to run currently, just stop running process
    else{
      stop_process ();
    }
  }
  //there is no running process
  else if (RUNNING==0){
    //pick up a new process
    if (!ready_queue.empty()){
      pick_up_running_process();
    }
  }
}

//check if there is any process in block list has done its io bound
void check_block_queue (){
  if (!block_queue.empty()) {
    list<process>::iterator it = block_queue.begin();
    list<process>::iterator temp;
    while (it != block_queue.end()){
      //IO burst finished
      if ((*it).finish_time == timestamp){
        temp=it;
        block_queue.erase(temp);

        (*it).at_new=timestamp;
        (*it).generate_time=timestamp;
        ready_queue.push_back(*(it++));
      }else{
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

  cout << "TYPES:" << endl;
  cout << "1. FCFS" << endl;
  cout << "2. LCFS" << endl;
  cout << "3. STJ"  << endl;
  cout << "4. ROBIN"<< endl;
  cout << "Please enter the number to choose a type :";

  cin >> TYPE;
  cout << endl;
  if (TYPE==FCFS){
    cout << "FCFS choosed." << endl;
  }else if (TYPE==LCFS){
    cout << "LCFS choosed." << endl;
  }else if (TYPE==STJ){
    cout << "STJ choosed." << endl;
  }else if (TYPE==ROBIN){
    cout << "ROBIN_ROUND choosed." << endl;
  }else{
    cout << "Invalid Choice, Exiting ..." << endl;
  }

  if (TYPE==ROBIN){
    char choose;
    cout << endl;
    cout << "Do you want to set  your quantum ? DEFAULT:10 Y/N :" << endl;
    cin >> choose;
    if (choose == 'Y'){
      cout << "Please enter the quantum:" << endl;
      cin >> quantum;
      cout << endl;
    }else if (choose == 'N'){
      cout << "Please enter the quantum:" << endl;
      quantum=10;
    }
  }
  cout << endl;

  //initiate running 
  running.finish_time=0;
  running.ft=0;
  running.tt=0;
  running.it=0;
  running.cw=0;
  
  //read the input file
  string file_name=argv[2];
  ifstream infile;
  infile.open (file_name.c_str());
  if (!infile.is_open()){
      cout << "Error Opening File, Please make sure correct input file name typed" << endl;
      exit(0);
  }
  int at,tc,cb,io;
  int process_id=0;
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
  
  cout << "FCFS" << endl;
  int index = 0;
  while (true) {
    //begin reading the input file
    while (index < all_processes.size()){
      process p = all_processes[index];
      if (p.at == timestamp){
        ready_queue.push_back(p);
        index++; 
      }else{
        break;
      }
    }

    //begin
    check_block_queue();
    check_ready_queue_and_running();

    if (!block_queue.empty()) {
      io_time++;
    }

    //cout << "TimeStamp: " << timestamp << endl;

    if (process_termination_no==all_processes.size()) {

      int tt_sum=0;
      int cw_sum=0;
      int pcount=all_processes.size();

      //print the result
      for (vector<process>::iterator it=all_processes.begin();it!=all_processes.end();it++){
        printf("%04d: %4d %4d %4d %4d | %4d %4d %4d %4d\n",(*it).id,(*it).at,(*it).tc,(*it).cb,(*it).io,(*it).ft,(*it).tt,(*it).it,(*it).cw);
        tt_sum += (*it).tt;
        cw_sum += (*it).cw;
      }

      //print the summary
      printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n",running.ft,(double)cpu_time/running.ft*100,(double)io_time/running.ft*100,
                            (double)tt_sum/pcount,(double)cw_sum/pcount,(double)pcount*100/timestamp);
      break;
    }

    //timestamp increasement
    timestamp++;
  }
}
