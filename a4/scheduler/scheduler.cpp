//********************************************************************************
// Author Zhifan Li
// CPSC 457 A4 Q2
// T02
// 30089428
// June 5
//********************************************************************************
#include "scheduler.h"
#include "common.h"
#include <climits>
#include <iostream>
#include <queue>
#include <utility>
using namespace std;
//********************************************************************************
// For documentation: The reason that I did not remove these std::cout is because this assignment is
// extremely hard to debug therefore, the cout is left for make the debugging easier in the future.
//********************************************************************************
void simulate_rr(
    int64_t quantum, int64_t max_seq_len, vector<Process> & processes, vector<int> & seq)
{
    seq.clear(); // clear the seq first since it is not empty at first
    int CPU_status = -1; // cpu -1 means idle
    int64_t burstRemaining = 0; // burst Remaining for the task currently being executed on the cpu
    queue<pair<int, int64_t>> readyQueue; // elements are ProcessID, burstRemaining, a queue of
                                          // tasks that are ready to be put on cpu at anytime
    int64_t currentTime = 0; // timer
    int64_t idle_time = 0; // the time that cpu being idle, useful for calculating if the running
                           // task reachs the time slice
    int64_t finish_early_time = 0; // the time to adjust the time slice limitation in case that any
                                   // task finishes shorter before the time slice reaches.
    bool array_of_checked[processes.size()]; // an array to check if the task that cpu having right
                                             // now is checked for giving a start time already
    for (uint64_t i = 0; i < processes.size(); i++) {
        array_of_checked[i] = false;
    } // initialize everything to be false first
    int jobsRemaining = processes.size(); // number of jobs remaining
    int current_arriving_process = 0; // the index of the upcoming process for the processes vector.
    // quantum
    // seq // execution order

    // Citation: Some of the implementation was based on the hint that pavol gave me.
    while (1) {

        if (jobsRemaining == 0 && CPU_status == -1
            && readyQueue.size()
                == 0) { // if no jobs remains, cpu idle, readyQ size is 0, then break and return.
            break;
        }

        // if cpu not idle, and burst ==0, set cpu to -1 and job remaining --.
        if (CPU_status != -1 && burstRemaining == 0) {
            processes[CPU_status].finish_time = currentTime; // record the finishing time
            CPU_status = -1; // set cpu to idle mode
            jobsRemaining--; // jobs --
            if ((currentTime - idle_time + finish_early_time) % quantum
                != 0) { // if it finishes inside the quantum, (earlier)
                finish_early_time = quantum + idle_time
                    - currentTime; // reset everything to be divisible by quantum in order to reset
                                   // the time slice limitation
            }
        } // if finish the task , need to record starting and finishing time.

        // if reached the quantum and cpu is not idle, then save it to the readyqueue.
        if (currentTime != 0 && (currentTime - idle_time + finish_early_time) % quantum == 0
            && CPU_status != -1) { // if reached the limit and cpu is not idle
            pair<int, int64_t> saved_task = make_pair(
                CPU_status, burstRemaining); // save the current cpu task as a pair of id and burst
            readyQueue.push(saved_task); // push the saved task to the queue
            CPU_status = -1; // set cpu to idle to continue again.

        } // if time slice reaches, switch

        // if currentTime there is arriving job, push it to the readyQueue
        while (current_arriving_process < (int)processes.size()
               && currentTime == processes[current_arriving_process].arrival_time) {
            readyQueue.push(make_pair(
                processes[current_arriving_process].id,
                processes[current_arriving_process].burst)); // push the process to ready queue
            current_arriving_process++; // index ++, means next arriving process's index
        }

        // if cpu is idle, and rq is not empty,
        int flag = 1; // make it accessible by other statements
        if (CPU_status == -1 && readyQueue.size() != 0) {

            queue<pair<int, int64_t>> temp_queue
                = readyQueue; // elements are ProcessID, burstRemaining
            vector<pair<int, int64_t>> temp_vector; // for easier to be used than a queue
            int64_t size = temp_queue.size(); // fixed size of queue
            for (int i = 0; i < size; i++) {
                temp_vector.push_back(temp_queue.front());
                temp_queue.pop();
            } // get everything from queue to the vector
            for (auto & a : temp_vector) {
                if (array_of_checked[a.first] == false) {
                    flag = 0; // if any task in queue does not have a start time, make the flag = 0
                }
            } // if any of the item in queue has not get a start time, then do not execute the
              // following.
            // cout<<(current_arriving_process < (int)processes.size())<<endl;
            if (flag == 1
                && current_arriving_process
                    < (int)processes.size()) { // if all the process in rq has start time, and
                // there are arriving process, then do the following, this is based on the hint
                // pavol gives. this will make sure there is no job will be done in here, and it
                // will not miss any upcoming arriving process basically speed up the program by
                // jumping the time slices for each process in the queue cout<<"***executing when
                // cpu idle and rq not empty, and arriving process coming***"<<endl;
                int64_t arrival
                    = processes[current_arriving_process].arrival_time; // upcoming arrival time
                int64_t smallest_burst = LLONG_MAX;
                for (auto & a : temp_vector) {
                    if (a.second < smallest_burst) { smallest_burst = a.second; }
                }
                // this is for choosing the smallest burst between the elements in queue, cpu is
                // idle now.

                //**************cout is for debugging
                // cout<<"arrival time: "<<arrival<<endl;
                // cout<<"current time: "<<currentTime<<endl;
                // cout<<"size of queue: "<<readyQueue.size()<<endl;
                //**************cout is for debugging

                int64_t n = (arrival - currentTime) / (size * quantum);
                // n is the number of times that each process can run the quantum time slices.
                // cout<<"original n iteration:"<<n<<endl;

                if (n * quantum > smallest_burst) {
                    n = smallest_burst / quantum;
                } // let n to be the smallest between n times of quantum, and smallest
                  // burst/quantum, so that no job can be finished while doing this
                if (n * quantum == smallest_burst) {
                    n--;
                } // ***** make sure that nothing can finish executing in this readyQueue speed up,
                  // because there will be time speed up in the following code, that will make the
                  // already done process miss the finish time it supposed to get.

                //**************cout is for debugging
                // cout<<"smallest_burst: "<<smallest_burst<<endl;
                // cout<<"fixed n iteration:"<<n<<endl;
                // cout<<"n * quantum: (to be subtracted for each process if n>0) is:
                // "<<n*quantum<<endl;
                //**************cout is for debugging

                // if n>0, means that the following program is meaningful to run, can make
                // progresses.
                if (n > 0) {
                    // cout<<"n is > 0 here, going through n>0 condition: "<<endl;
                    for (auto & a : temp_vector) {
                        // cout<<"id: "<<a.first<<"**original** burstRemaining for the id:
                        // "<<a.second<<endl;
                        a.second -= n * quantum;
                    } // each task in ready queue gets their burst time decreased by n*quantum

                    for (int i = 0; i < n; i++) {
                        for (auto & a : temp_vector) {
                            if ((seq.size() == 0 || seq.back() != a.first)
                                && (int)seq.size() < max_seq_len) {
                                seq.push_back(
                                    a.first); // push into seq and make sure it is compressed
                            } // push them into the sequence
                            if ((int)seq.size() == max_seq_len) {
                                break;
                            } // if reach the limit, then break
                        }
                        if ((int)seq.size() == max_seq_len) {
                            break;
                        } // if reach the max limit, then break
                    }
                    // cout<<"orginal time before readyqueue speed up: "<<currentTime<<endl;
                    currentTime += n * quantum * size; // increment the current time by n* quantum *
                                                       // size. size is the size of queue

                    //**************cout is for debugging
                    // cout<<"added to currentTime"<<n* quantum * size<<endl;
                    // cout<<"speeded up time: "<<currentTime<<endl;
                    // cout<<"now every proc in readyQ has speededup"<<endl;
                    //**************cout is for debugging

                    for (int i = 0; i < (int)temp_vector.size(); i++) {
                        temp_queue.push(temp_vector[i]);
                        // cout<<"id: "<<temp_vector[i].first<<"after boosting, burstRemaining for
                        // the id: "<<temp_vector[i].second<<endl;
                    } // make vector to a queue
                    readyQueue = temp_queue;
                    // update the readyQ
                    // cout<<"***END of executing when cpu idle and rq not empty, and arriving
                    // process coming***"<<endl<<endl;
                }
            }
            // this happens only when readyQueue is full, and there are no upcoming tasks to ready
            // queue, then we can optimize it make it super efficient
            if (flag == 1 && readyQueue.size() != 0
                && current_arriving_process == (int)processes.size()) {
                // if readyQueue is filled with the processes with start time specified, we can just
                // skip these adding parts and directly return the result.
                while (temp_vector.size() != 0) {

                    int64_t smallest_burst = LLONG_MAX;
                    int smallest_burst_id = -1; // pick smallest id
                    int smallest_burst_index
                        = -1; // pick that smallest burst process's index in the queue, for further
                              // assgining extra time quantum to the proc before the index, and
                              // reordering the vector when one queue is done
                    int i = 0;
                    for (auto & a : temp_vector) {
                        if (a.second < smallest_burst) {
                            smallest_burst = a.second;
                            smallest_burst_id = a.first;
                            smallest_burst_index = i;
                        }
                        i++;
                    } // find process
                    // cout<<"***smallest_burst is: "<<smallest_burst<<endl;
                    // cout<<"smallest_burst_id is: "<<smallest_burst_id<<endl;
                    // cout<<"smallest_burst_index in the queue is: "<<smallest_burst_index<<endl;

                    int64_t n = (smallest_burst / quantum); // find n so that n* quantum is 100%
                                                            // smaller than the smallest burst.
                    if (smallest_burst % quantum == 0) { n--; }
                    // cout<<"n is: smallest_burst/quantum= "<<n<<endl;

                    for (auto & a : temp_vector) {
                        a.second -= n * quantum;
                        currentTime += n * quantum;
                    } // for each process, speed it up n times quantum, and without finishing the
                      // smallest burst.

                    for (int i = 0; i < n && (int)seq.size() < max_seq_len; i++) {
                        // for pushing the sequence of execution into the seq vector
                        for (auto & a : temp_vector) {
                            if (seq.back() != a.first && (int)seq.size() < max_seq_len)
                                seq.push_back(a.first);
                        }
                    }
                    // cout<<"last time of the boost, only boost quantum up to the
                    // smallest_burst_index in the queue: "<<endl;
                    for (int i = 0; i < smallest_burst_index;
                         i++) { // now, there will be some left overs, and before the smallest
                                // index, these processes must be executed first in order to let the
                                // leftover of smallest burst to finish
                        temp_vector[i].second -= quantum;
                        currentTime += quantum;
                        if ((seq.size() == 0 || seq.back() != temp_vector[i].first)
                            && (int)seq.size() < max_seq_len) {
                            seq.push_back(temp_vector[i].first);
                        }
                    } // last iteration
                    // cout<<" number of process get last time boosted:
                    // "<<smallest_burst_index<<endl; cout<<"current time before last element boost:
                    // "<<currentTime<<endl;

                    if ((seq.size() == 0 || seq.back() != temp_vector[smallest_burst_index].first)
                        && (int)seq.size() < max_seq_len) {
                        seq.push_back(temp_vector[smallest_burst_index].first);
                    }
                    currentTime += temp_vector[smallest_burst_index]
                                       .second; // it is the smallest burst in the queue, and now it
                                                // is finished

                    int end_index = (smallest_burst_index)
                        % temp_vector
                              .size(); // ***** very important, record the end index of the last
                                       // finsihed index, and do a reordering algorithm below

                    // cout<<"*****current time and finish time after last element boost:
                    // "<<currentTime<<endl;

                    processes[smallest_burst_id].finish_time
                        = currentTime; // set the finish time of the smalles burst process

                    temp_vector.erase(
                        temp_vector.begin() + smallest_burst_index); // erase that element,
                    // cout<<"temp_vector size decrease to: "<<temp_vector.size()<<endl<<endl;
                    if (end_index != 0) {
                        vector<pair<int, int64_t>> temp_reordering_vector;
                        for (int i = end_index; i < end_index + (int)temp_vector.size(); i++) {
                            temp_reordering_vector.push_back(temp_vector[i % temp_vector.size()]);
                        }
                        temp_vector = temp_reordering_vector;
                    } // this is the reordering algorithm, so from where the smallest burst
                      // finsihed, the next element will be ordered as the first element for the
                      // queue and vector
                }
                return;
            }

            // change cpu status to working, and set burstRemaining
            CPU_status = readyQueue.front().first;
            burstRemaining = readyQueue.front().second;
            if (array_of_checked[CPU_status]
                == false) { // if it has not had a start time, then assign a start time to it, and
                            // set it to true means it already has start time

                array_of_checked[CPU_status] = true;
                processes[CPU_status].start_time = currentTime;
            }
            if ((seq.size() == 0 || seq.back() != CPU_status) && (int)seq.size() < max_seq_len) {
                seq.push_back(CPU_status);
            }
            // now cpu is loaded with that task, but readyQueue has not been updated, we check the
            // readQueue and try to finish as much possible tasks in turns.
            readyQueue.pop();

            // if rq is empty, and cpu has last job, then just update the finsh time and return.
            if (readyQueue.size() == 0 && current_arriving_process == (int)processes.size()) {
                // cout<<"***executing last task***"<<endl;
                // cout<<"task id: "<<CPU_status<<endl;

                currentTime += burstRemaining;
                processes[CPU_status].finish_time = currentTime;

                // cout<<"***all done,finished time of last task: "<<currentTime<<endl;
                return;
            }

            // if readyqueue is empty, cpu is on task, and there are arriving process
            if (readyQueue.size() == 0 && current_arriving_process < (int)processes.size()) {
                // if the arrival time is larger than the only burst time, then finish the task,
                // change cpu to -1, change time to arrival time, and do the next cpu task
                if (processes[current_arriving_process].arrival_time
                    > currentTime + burstRemaining) {
                    processes[CPU_status].finish_time = currentTime + burstRemaining;

                    currentTime = processes[current_arriving_process].arrival_time;
                    idle_time += currentTime - burstRemaining;
                    jobsRemaining--; // the job on cpu is done
                    if ((currentTime - idle_time + finish_early_time) % quantum
                        != 0) { // if it finishes inside the quantum,
                        finish_early_time = quantum + idle_time - currentTime;
                    }
                    burstRemaining = 0;
                    CPU_status = -1; // change it to -1 inorder to jump the first job-finished
                                     // handling if statement

                    if ((seq.size() == 0 || seq.back() != CPU_status)
                        && (int)seq.size() < max_seq_len) {
                        seq.push_back(CPU_status);
                    } // because it finished before arrival time, so cpu will be idle for a while,
                      // so -1 will be added to the seq

                    continue; // go to while loop again to execute, skip the rest

                }

                else { // if the task cpu executing is larger than the arrival time, then just do a
                       // few time slices.
                    int64_t times = (processes[current_arriving_process].arrival_time - currentTime)
                        / quantum;

                    if (times != 0) {
                        // this part happens only if the time is not 0, if 0, it should not go to
                        // "continue", instead, it get out of the loop and go to end of while loop
                        // to make currentTime+1, and reduce its burst.
                        //  cout<<"***cpu burst is larger than the arrival time, and RQ is
                        //  empty"<<endl; cout<<"ID: "<<CPU_status<<endl; cout<<"times of quantum to
                        //  be boosted: "<<times<<endl; cout<<"before burstRemainng:
                        //  "<<burstRemaining<<endl; cout<<"before currentTime:
                        //  "<<currentTime<<endl; cout<<"arrival_time:
                        //  "<<processes[current_arriving_process].arrival_time<<endl;
                        burstRemaining -= times * quantum;
                        currentTime += times * quantum;
                        //  cout<<"after burstRemaing: "<<burstRemaining<<endl;
                        //  cout<<"after currentTime: "<<currentTime<<endl<<endl;
                        continue; // means skip the rest of the while loop
                    }
                }
            }

            // if the burst is to be finished early inside the quantum, speed it up to reach its
            // time slice
            if (burstRemaining <= quantum) {
                //  cout<<"burstRemaining<=quantum happends: "<<endl;
                //  cout<<"CPU_status to be boosted: "<<CPU_status<<" burst it has :
                //  "<<burstRemaining<<" currentTime: "<<currentTime<<endl;
                currentTime += burstRemaining;
                // cout<<"after boosted:current Time "<<currentTime<<endl<<endl;
                burstRemaining = 0;

                while (current_arriving_process < (int)processes.size()
                       && (currentTime >= processes[current_arriving_process].arrival_time)) {
                    readyQueue.push(make_pair(
                        processes[current_arriving_process].id,
                        processes[current_arriving_process].burst));
                    current_arriving_process++;
                    // cout<<"pushed one when boosting the burstRemaining <= quantum, id:
                    // "<<processes[current_arriving_process].id<<endl;
                }
                continue;
            }

        } // line 70-317 end

        if ((seq.size() == 0 || seq.back() != CPU_status) && (int)seq.size() < max_seq_len) {
            seq.push_back(CPU_status);
        } // change the sequence

        // if cpu has no task, rq is empty, then jump to next arrival time.
        if (CPU_status == -1 && readyQueue.size() == 0
            && current_arriving_process < (int)processes.size()) {
            idle_time += processes[current_arriving_process].arrival_time
                - currentTime; // may check it again here
            currentTime = processes[current_arriving_process].arrival_time;
            finish_early_time = quantum + idle_time - currentTime;
            continue;
        } // avoid waiting for long time

        // cout<<"time: "<<currentTime<<" id: "<<CPU_status<<endl;

        // area for adding current time and speeding up if burst is smaller than quantum, just minus
        // quantum instead of minus 1.
        // tesing area:
        if (CPU_status == -1) {
            // if idle, do it normally.
            idle_time++;
            currentTime += 1;
        } else if (burstRemaining - quantum >= 0) { // if burstRemaining can be speeded up by
                                                    // subtracting quantum.
            //    cout<<"*** burstRemaining-quantum>=0 *** start speed up the burst by
            //    quantum"<<endl;
            //     cout<<"CPU_status"<<CPU_status<<endl;
            //     cout<<"before: burstRemaining-quantum>=0: "<<"burstRemaining:
            //     "<<burstRemaining<<"current time: "<<currentTime<<endl;
            burstRemaining -= quantum; // speed up the burst by the quantum
            currentTime += quantum; // change current time
            // cout<<"after: "<<"burstRemaining: "<<burstRemaining<<"current time:
            // "<<currentTime<<endl<<endl;

            while (current_arriving_process < (int)processes.size()
                   && (currentTime > processes[current_arriving_process].arrival_time)) {
                readyQueue.push(make_pair(
                    processes[current_arriving_process].id,
                    processes[current_arriving_process].burst));
                // cout<<"pushed one when boosting the burstRemaining - quantum>=0, id:
                // "<<processes[current_arriving_process].id<<endl;
                current_arriving_process++;
            } // see if there is any tasks coming when the burst is speeded up, if there are, put
              // them into ready queue

            if ((currentTime - idle_time + finish_early_time) % quantum
                == 0) { // if reached the time limit by boosting it
                pair<int, int64_t> saved_task = make_pair(CPU_status, burstRemaining);
                readyQueue.push(saved_task); // push the saved task to the queue
                // cout<<"current cpu "<<CPU_status<<"being saved to queue since it reached its time
                // slice"<<endl;
                CPU_status = -1; // set cpu to idle to continue again.
            }
        } else { // if cpu is not idle and burstRemaining cannot be substracted by quantum (it will
                 // be smaller than 0 if do so), then normally just subtract and increment normally
                 // time (1)
            if (burstRemaining > 0) burstRemaining -= 1;
            currentTime += 1;
        }
    }

} // end of program