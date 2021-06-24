// this is the ONLY file you should edit and submit to D2L
//**********************************************************
// Author: Zhifan Li
// UCID: 30089428
// Assignment 4 Question 1
//**********************************************************
#include "deadlock_detector.h"
#include "common.h"
#include <algorithm>
#include <iostream>
using namespace std;
/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with edge_index set to the index that caused the deadlock, and dl_procs set
/// to contain with names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with edge_index = -1 and empty dl_procs[].
///
//***********Citation: this graph algorithm idea is from Pavol's hint
class Graph{
    public:
    vector<vector<int> >adj_list; // record a list of the adjacency list for each vertex of the graph
    vector<int>out_counts; // record a list of out-going degree for each vertex.
}my_graph; // object of Graph
//************ end of citation
Result detect_deadlock(const std::vector<std::string> & edges)
{
    Result res; // Result object
    res.edge_index=-1; // initially it is -1;
    vector<string> request; // request from input , 1 line
    Word2Int w2i; // Citation: from Pavol's common.h common.cpp
    vector<pair<string,bool> > int_to_words; // a vector to store indexes of pairs of string and boolean, string is its original name, boolean is true means: process, false means: resource to be used in order to identify if it is process or resource
    int count_words=0;
    for(uint64_t local_edge_index=0;local_edge_index<edges.size();local_edge_index++){ // iterate the vector of inputs

    request=split(edges[local_edge_index]); // call the split function from the file Pavol gave us.
    int num_p=w2i.get(request[0]);          //num_p means the integer representation of the string name of the process
    int num_r=w2i.get(request[2]);          //num_r means the integer representation of the string name of the resource

    if(num_p==count_words){                 // count words is the number of distinct words appear in the inputs, if it reaches the limit, it means it is a new word
        count_words++;                      // count words ++, a new word is detected
        my_graph.adj_list.resize(count_words); // resize the vector in order to let the new integer of the new process to be added
        my_graph.out_counts.resize(count_words); // resize
        int_to_words.resize(count_words);   //resize the int_to words, it is for keep tracking of the integer and string in order to convert them back
        int_to_words[num_p]=make_pair(request[0],true); // true means it is a process
    }else if(num_p<count_words ){              // if the num_p already exists, means either it is already declared as a process, or dec
        bool found=0;                           //set a flag
        for(uint64_t i=0;i<int_to_words.size();i++){
            if(int_to_words[i].first==request[0]&&int_to_words[i].second==true){ // if string name matches and it is a process, means originally it already declared as a process, then no need to build the current num_p to be a new index for all the three vectors
                found=1; // set flag =1, found
                num_p=i; // the num_p use the already-made process's index (so it will not make a new index for itself)
            }
        }// for loop ends
        if(found==0){   // if not found, it means that  it was originally declared as a resource, it means that it needs to make another process with the same name and complete different index for all the three vectors
        count_words++;  // new index to be made, so count words++, since different usage of (p,r)==different words
        num_p=count_words-1;// set as last index
        my_graph.adj_list.resize(count_words); // resize
        my_graph.out_counts.resize(count_words); //resize
        int_to_words.resize(count_words);        //resize
        int_to_words[num_p]=make_pair(request[0],true); // in int_to_word vector, at that index, makes a new pair of being a process not resource
        }

    }  
    //******************************************************************************
    // same documentation as above also can describe the followin code for num_r, just exchange all the num_p with num_r                         
    if(num_r==count_words){
        count_words++;
        my_graph.adj_list.resize(count_words); //resize
        my_graph.out_counts.resize(count_words); //resize
        int_to_words.resize(count_words); //resize
        int_to_words[num_r]=make_pair(request[2],false); // false means it is a resource
    }else if(num_r<count_words){ // if the word already exists, first we find if it is already declared, if no, it means it was first declared by a process, then it should be declared, and in the future, other resources will use its index to access the three vectors
            bool found=0;
        for(uint64_t i=0;i<int_to_words.size();i++){
            if(int_to_words[i].first==request[2]&&int_to_words[i].second==false){
                found=1;
                num_r=i;
            }
        }
        if(found==0){   // if not found as a resource, then build a new one
        count_words++;
        num_r=count_words-1; //num_r equals to the last index of the array, see below
        my_graph.adj_list.resize(count_words); //resize 
        my_graph.out_counts.resize(count_words); //resize
        int_to_words.resize(count_words); //resize
        int_to_words[num_r]=make_pair(request[2],false);
        }


    }
    //******************************************************************************
    // handling num_p and num_r ends, now it needs to change the adjacency list
    if(request[1]=="->"){
    //if requesting resource
    my_graph.out_counts[num_p]++;   //out-going edges count ++
    my_graph.adj_list[num_r].push_back(num_p); // adjacency list that points to it push new index in
    }else{
    //if assigning resource
    my_graph.out_counts[num_r]++; //out-going edges count ++
    my_graph.adj_list[num_p].push_back(num_r); // adjacency list that points to it push new index in
    
    }
    //return res;
    vector<int>list_of_zeros;
    
    vector<int>out_temp=my_graph.out_counts;// make a copy to modify it 

    //vector<int>out_temp_for_sorting=my_graph.out_counts;
    // might need to sort it later
   // sort(out_temp_for_sorting.begin(),out_temp_for_sorting.end());
   

    for(uint64_t i=0;i<out_temp.size();i++){
        if (out_temp[i]==0){
            list_of_zeros.push_back(i);
        }
        
    }// find if there is any zeros; populate them into the list of zeros.
    // Citation: **********************from my tutorial and pavol , the topological sorting algorithm
    while(list_of_zeros.size()!=0){ // if the list of zeros are not empty. do the topological sort
        int index=list_of_zeros.back(); // get last element
        list_of_zeros.pop_back();       // pop last element out
        for(auto &a : my_graph.adj_list[index]){
           out_temp[a]--;               // find its adjacency list of indexes, and make them decremented by 1 since the edges should be deleted
            if(out_temp[a]==0){         // if find it to be 0, immediately put into the list of zeros
                list_of_zeros.push_back(a);
            }
        }
    }// now the list's size is zero now
    // ************************end of citation.
     int none_zero_flag=0;
    for(uint64_t i=0;i<out_temp.size();i++){
        if(out_temp[i]!=0){             // now checking the content of out_temp, if sees anything that is not 0, it means there is deadlock existing.
            none_zero_flag=1;           // set the deadlock flag=1
            if(int_to_words[i].second==true){
            res.dl_procs.push_back(int_to_words[i].first);// if it is a process, then put it in the result.dl_procs vector for returning.
            } // push back the string
        }
    }
   
    if(none_zero_flag==1){   // if there is non-zero count existing, return the deadlock processes, and the edge_index is the current local_edge_index
        res.edge_index=local_edge_index;
        return res;
    }                       // if the none_zero_flag=1, edge_index=-1, and return empty vector back.
 }
 return res;                // return the result.
 
}

