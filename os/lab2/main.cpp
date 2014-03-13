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

using namespace std;

int process_termination_no=0;
double cpu_time=0;
double io_time=0;
double io_finish_time=0;
int current_time=0;
int ofs=0;
int RUNNING=0; 
//solutions
int FCFS=0;
int LCFS=1;
int STJ=2;
int ROBIN=3;
int TYPE=1;
//quantum for robin round algotithm
int quantum=2;

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
  //for robin_round
  int burst_remain;
};

process running, running_prepare;
list<process> ready_queue;
list<process> block_queue;
vector<process> all_processes;
//store rand numbers
vector<int> randvals;

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
}

//block running process for IO burst
void block_process(){
  int burst = cpu_run_or_block_time(running.io);
   // cout << "IO burst------Running:  " << running.id  << "   Burst:  " << burst << endl;
    if (io_finish_time>running.finish_time) {
        if(io_finish_time<(running.finish_time+burst)){
            io_time += burst-(io_finish_time-running.finish_time);
            io_finish_time=running.finish_time+burst;
        }
    }
    else {
        io_time+=burst;
        io_finish_time=running.finish_time+burst;
    }
  running.it += burst;
  running.finish_time = running.finish_time + burst;
  running.generate_time=running.finish_time;

  //block_queue is a sorted queue
  list<process>::iterator bq=block_queue.begin();
  for (; bq != block_queue.end(); bq++){
    if ((*bq).finish_time > running.finish_time){
      block_queue.insert(bq,running);
      break;
    }
  }
  if (bq == block_queue.end()){
    block_queue.push_back(running);
  }
}

void robin_round_finished(){
  running.at_new = running.finish_time;
  running.generate_time = running.finish_time;
  ready_queue.push_back(running);
}

//check running process if it has been finished or got blocked
void stop_process (){
  //current event finished
  if (running.tc_remain == 0) {
    finish_process();
  } else {
    if (TYPE == ROBIN && running.burst_remain !=0 ){
      robin_round_finished();
    }else{
      block_process();
    }
  }
  //set RUNNING tag
  RUNNING=0;
}

void set_running_process(){
  if (TYPE==STJ){
    //get the process with smallest tc_remain
    list<process>::iterator next_running_process=ready_queue.begin();
    for (list<process>::iterator it=ready_queue.begin(); it!= ready_queue.end(); ++it) {
      if ((*it).tc_remain < (*next_running_process).tc_remain) {
        next_running_process=it;
      }
    }
    running=*next_running_process;
    ready_queue.erase(next_running_process);
  }

  if (TYPE==FCFS || TYPE==ROBIN){
    running=ready_queue.front();
    ready_queue.pop_front();
  }

  if (TYPE==LCFS){
    running=ready_queue.back();
    ready_queue.pop_back();
  }
}

void pick_up_running_process (){
  set_running_process();
  int burst;
  if (running.burst_remain==0){
    burst = cpu_run_or_block_time(running.cb);
    if (running.tc_remain<burst) burst=running.tc_remain;

    if (TYPE==ROBIN){
      running.burst_remain = burst;
    }
  }

  if (TYPE==ROBIN){
    if (running.burst_remain>quantum){
      burst=quantum;
    } else {
      burst=running.burst_remain;
    }
    running.burst_remain -= burst;
  }

  //cout << "CPU burst------Running:  " << running.id << "   Burst:  " << burst << endl;
  //set running process
  running.generate_time=current_time;
  if (running.at_new < current_time) {
    //cout << "CPU waiting time: " << running.cw << " += " << current_time << " - " << running.at_new << endl;
    running.cw += (current_time-running.at_new);
  }
  cpu_time += burst;
  running.tc_remain -= burst;
  running.finish_time = current_time + burst;
  RUNNING=1;
}

//solve block finished process and running process
void check_ready_queue_and_running (){
  //current cpu burst of running process has been finished
  if (RUNNING==1){
    current_time=running.finish_time;
    stop_process ();

    //pick up a new process
    if (!ready_queue.empty() || (TYPE==ROBIN && running.burst_remain!=0)){
      pick_up_running_process();
    }
  }
  //there is no running process
  else if (RUNNING==0){
    //pick up a new process
    if (!ready_queue.empty()){
      current_time=ready_queue.front().finish_time;
      pick_up_running_process();
    }
  }
}

//check if there is any process in block list has done its io bound
void check_block_queue (){

  if (!block_queue.empty()){
    if (RUNNING==1) {
      current_time = running.finish_time;
    }else if (!ready_queue.empty()){
      current_time = ready_queue.front().finish_time;
    }else{
      current_time = block_queue.front().finish_time;
    }

    while (!block_queue.empty() && block_queue.front().finish_time <= current_time){
      process p = block_queue.front();
      p.at_new = p.finish_time;
      ready_queue.push_back(p);
      block_queue.pop_front();
    }
  }
}

//main function
int main(int argc, char* argv[]) {

  char *sval=NULL;
  int c;
  
  while((c=getopt(argc,argv,"vs:")) != -1){
      switch (c){
          case 's':  sval=optarg;
              break;
          case '?': {
              if (optopt == 's')
                  fprintf (stderr, "Option -%c requires an argument.\n", optopt);
              else if (isprint (optopt))
                  fprintf (stderr, "Unknown option `-%c'.\n", optopt);
              else
                  fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
              return 1;
          default:
              abort ();
          }
      }
  }
  
  if (strncmp(sval,"F",1)==0) TYPE=FCFS;
  else if (strncmp(sval,"L",1)==0) TYPE=LCFS;
  else if (strncmp(sval,"S",1)==0) TYPE=STJ;
  else if (strncmp(sval,"R",1)==0) {
      TYPE=ROBIN;
      string roundno=string(sval);
      roundno.erase(roundno.begin());
      quantum=atoi(roundno.c_str());
  }
  
  ifstream infile(argv[optind]);
  ifstream rfile(argv[optind+1]);
  
  //read the input file
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
      p.finish_time=p.at; 
      p.ft=0; 
      p.tt=0;
      p.it=0;
      p.cw=0; 
      p.burst_remain=0;

      all_processes.push_back(p);
    }
  }

  //initiate running 
  running.finish_time=0;
  running.generate_time=0;
  running.ft=0;
  running.tt=0;
  running.it=0;
  running.cw=0;
  
  //read the random value from random file
  if (!rfile.is_open()){
      cout << "Error Opening \"rfile\", Please make sure correct input file name typed" << endl;
      exit(0);
  }
  int random;
    int r_count;
    rfile >>r_count;
  while (!rfile.eof()){
    rfile >> random;
    randvals.push_back(random);
  }
  
  string title[4] = {"FCFS", "LCFS", "SJF", "RR"};
  if (TYPE == ROBIN){
    cout << title[TYPE] << " " <<quantum  << endl;
  }else{
    cout << title[TYPE] << endl;
  }

  int index = 0;
  while (true) {
    //begin reading the input file
    if (RUNNING==0 && block_queue.empty()){
      current_time = all_processes[index].at;
    }
    
    if (RUNNING==0 && !block_queue.empty()){
      current_time = block_queue.front().finish_time;
    }

    if (RUNNING == 1){
      current_time = running.finish_time;
    }

    while (index < all_processes.size()){
      process p = all_processes[index];
      if (p.at <= current_time){
        //block_queue is a sorted queue
        list<process>::iterator rq=ready_queue.begin();
        for (; rq != ready_queue.end(); rq++){
          if ((*rq).finish_time > current_time){
            p.at_new = p.finish_time;
            ready_queue.insert(rq,p);
            break;
          }
        }
        if (rq == ready_queue.end()){
          ready_queue.push_back(p);
        }

        index++; 
      }else{
        break;
      }
    }

//    cout << block_queue.size() << "  " << ready_queue.size() << endl;
    //begin
    check_block_queue();
    check_ready_queue_and_running();

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
                            (double)tt_sum/pcount,(double)cw_sum/pcount,(double)pcount*100/current_time);
      break;
    }
  }
}
