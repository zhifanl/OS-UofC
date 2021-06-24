// Author: Zhifan Li
// UCID: 30089428
// Assignment 5 Q2 
// T02
#include "fatsim.h"
#include <cstdio>
#include <vector>
#include <algorithm>
using namespace std;

vector<long> fat_check(const vector<long> & fat)
{
  vector<vector<long> > quick_find; // vector for storing index as each root, and values in that index as a vector of all the nodes pointing to each root
  quick_find.resize(fat.size()+1); // -1 is located at the last element of quick_find vector, and it is resized
  for(long i=0;i<(long)fat.size();i++){ 
    if(fat[i]!=-1)
      quick_find[fat[i]].push_back(i); // index is each root, push back each index i in fat for pointing to that root
    else
      quick_find[quick_find.size()-1].push_back(i); 
  } // key is the root, value is its adjcency that points at it
   vector<long> result; // result of max valid files
  long max_level=0; // max level of each valid file, reset to zero every time
  for(long i=0;i<(long)quick_find[quick_find.size()-1].size();i++){ // first use all the nodes that points to -1 as start, then DFS to the starting point of each file
        vector<pair<long,long> >stack; // pair of index and level
        stack.emplace_back(make_pair(quick_find[quick_find.size()-1][i],1)); // initial level is 1, because it already point to -1, so it has one node now
        while(!stack.empty()){ // DFS, if stack not empty
          pair<long,long> index_level_pair=stack.back(); // get last element of stack
          stack.pop_back(); // pop
          if(index_level_pair.second>max_level){ // if that element's second (level) is greater than max level, change max level
              max_level=index_level_pair.second;
          }
          for(long j=0;j<(long)quick_find[index_level_pair.first].size();j++){ // use a for loop for each node that point to current root
              long temp=index_level_pair.second;  // get its level value
              stack.emplace_back(make_pair(quick_find[index_level_pair.first][j],temp+1)); // push that node's index as a root, with level incremented by 1, to the end of stack
            }
        }
        result.push_back(max_level); // push the current result
        max_level=0; // reset max level
  }
  sort(result.begin(),result.end()); //sort it
   return result; // return the result
}