// Author: Zhifan Li 30089428
// CPSC457 Assignment 2
#include "getDirStats.h"
#include "digester.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <dirent.h>
#include <algorithm>

#define MAX_DIR 10000
#define MAX_FILE 10000
#define MAX_FILE_PATH 4096
#define MAX_WORDS 1024
static bool
is_dir(const std::string & path)
{
  struct stat buff;
  if (0 != stat(path.c_str(), & buff)) return false;
  return S_ISDIR(buff.st_mode);
}


std::string read_word(FILE*file){
  std::string result;
  while(1){
  int c=fgetc(file);
  if(c==EOF) {
    if(result.size()<3){ // eof reached and result is not valid word, dump it
      return "";
    }else{
      return result;
    }
  }
   if(!isalpha(c)){
      if(result.size()<3){
      result.clear();
      continue;
      }else{
        break;
      }// if meet non-alpha and size is >=3, break while, return that word
  }else{
    
    result.push_back(tolower(c));
  }
  }
  return result;
}
bool compareSize(std::pair<std::string,std::vector<std::string>> & a1,std::pair<std::string,std::vector<std::string>> & a2){
  return (a1.second.size()>a2.second.size());
}//sort by descending order

// getDirStats() computes stats about directory a directory
//   dir_name = name of the directory to exastd::mine
//   n = how many top words/filet types/groups to report
//
// if successful, results.valid = true
// on failure, results.valid = false
//
Results 
getDirStats(const std::string & dir_name, int n)
{
  
  Results res;
  res.valid = false;// set the true once all done. // need to think when it cannot open, how to implment this.

  long  sum_file_size=0;
  long  number_of_files=0;
  long  number_of_subdir= -1;// start at -1, because the input dir itself is not a subdir. 
  std::string largest_file="";
  long  largest_file_size= -1; // if not found any file, it is -1.

  struct stat file_stats;

  std::unordered_map<std::string,int>file_types;
  std::unordered_map<std::string,int>common_words;
  std::unordered_map<std::string,std::vector<std::string>>duplicate_files;
  std::vector<std::string>stack;
  stack.push_back(dir_name);
  DIR * dir;
  dirent * de ;
  //Citation: This recursive search files and dir is I learned from my TA's tutorial.
  while(stack.size()!=0){
    auto dir_name=stack.back();
    stack.pop_back();
    //now check each content to see if it is file or dir.
    if(is_dir(dir_name)){ // if is dir, open and get all content into the stack
      number_of_subdir++;
       dir= opendir( dir_name.c_str());
      if(!dir){return res;} // if cannot open dir, return as valid=false;
      while(1){
         de = readdir( dir);
        if(!de){
          break;
        }
        std::string name=de->d_name;
        if(name=="."||name=="..")continue;
        std::string path=dir_name+"/"+name;  // modify the file name.
        stack.push_back(path); // push it into the LIFO stack, DFS
      }
      closedir(dir);
    } // if is_dir is true.
    else{ // if it is a file.
      number_of_files++;
      stat(dir_name.c_str(), &file_stats);
      sum_file_size+=file_stats.st_size;
      if(file_stats.st_size>largest_file_size){
        largest_file_size=file_stats.st_size;
        largest_file=dir_name;
      }
      std::string command="file -b "+dir_name;
      FILE * stream=popen(command.c_str(),"r");
      if(!stream){return res;}
      std::string result="";
      while(1){
        char c=fgetc(stream);
        if(c==EOF||c==','||c=='\n')break;
          result.push_back(c);
      }
      file_types[result]++; // add to the map, increment by 1
      pclose(stream); // close the stream

      //now getting most common words
      FILE* file=fopen(dir_name.c_str(),"r");
      if(!file){return res;}// if cannot open the file, return as valid=false;

      while(1)
      {
        std::string word=read_word(file);
        if(word.size()==0)break;
        common_words[word]++;
      }
      fclose(file);//close the file stream
      // got all words to common_words;

      //now put the current file's hash in duplicate_files.
      std::string hash_file = sha256_from_file(dir_name);
      duplicate_files[hash_file].push_back(dir_name);
    }

  }
  
  std::vector<std::pair<int ,std::string>> files;
  for(auto &s:file_types){
    files.emplace_back(-s.second,s.first);
  }
  if((int)files.size()>n)
  std::partial_sort(files.begin(),files.begin()+n,files.end());
  else
  std::sort(files.begin(),files.end());
  int smaller_1=std::min((int)files.size(),n);
  for(int i=0;i<smaller_1;i++) {
    res.most_common_types.push_back(make_pair(files[i].second,-(files[i].first)));
  }
  // all common_types populated.
  std::vector<std::pair<int ,std::string>> words;
  for(auto & a:common_words){
    words.emplace_back(-a.second,a.first);
  }
  //Citation: I learned how to use partial_sort API from Pavol's example code (word histogram)
  if((int)words.size()>n)
  std::partial_sort(words.begin(),words.begin()+n,words.end());
  else
  std::sort(words.begin(),words.end());
  int smaller_2=std::min((int)words.size(),n);
  for(int i=0;i<smaller_2;i++){
    res.most_common_words.push_back(make_pair(words[i].second,-(words[i].first)));
  }
  //all common words populated.
  std::vector<std::pair<std::string,std::vector<std::string>>>duplicate_groups;
  for(auto &b:duplicate_files){
    if(b.second.size()>=2){  // file is duplicate only if >=2 files are there
  duplicate_groups.emplace_back(make_pair(b.first,b.second));
    }
  }
  if((int)duplicate_groups.size()>n)
  std::partial_sort(duplicate_groups.begin(),duplicate_groups.begin()+n,duplicate_groups.end(),compareSize);//largest n
  else
  std::sort(duplicate_groups.begin(),duplicate_groups.end(),compareSize);// should start with largest size 
  int smaller_3=std::min((int)duplicate_groups.size(),n);
  for(int i=0;i<smaller_3;i++){
    res.duplicate_files.push_back(duplicate_groups[i].second);
  }
  res.all_files_size=sum_file_size;
  res.largest_file_path=largest_file;
  res.largest_file_size=largest_file_size;
  res.n_files=number_of_files;
  res.n_dirs=number_of_subdir;
  res.valid=true; // make it true, since no invalid operations.
  return res;
}

