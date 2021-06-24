#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <vector>
using namespace std;
#define BUFFER_SIZE 1024*1024

// Author: Zhifan Li, UCID 30089428
// CPSC 457 Assignment 1 T02

// Used from Dr.Federl's slow-pali code
bool is_palindrome( const string & s)
{
  for( unsigned int  i = 0 ; i < s.size()/2 ; i++)
    if( tolower(s[i]) != tolower(s[s.size()-i-1]))
      return false;
  return true;
}
// End of Dr.Federl's code


char buffer[BUFFER_SIZE];
int buffer_size;

string max_pali;
int current_pos=0;
string current_string;
 string read_input(){
     if(current_pos==0){
   buffer_size=read(STDIN_FILENO,buffer,BUFFER_SIZE);}
      if(buffer_size<=0)
      return "END_OF_FILE_REACHED_ID1234567"; //end of file reached
      while(1){
          if(current_pos==buffer_size){
              current_pos=0;
              return "";
          }
          if(isspace(buffer[current_pos])&&current_string!=""){
                 string result=current_string;
                
                current_string.clear(); // return the completed string, since there is a space terminate the word, and move to next position.
                current_pos++;
                return result;          
          }
          if(isspace(buffer[current_pos])){
            
            current_pos++;
        }else{
            break;
        }
      }// skip all the white spaces at first.
      while(1){

           if(current_pos==buffer_size-1&&!isspace(buffer[buffer_size-1])){ // if reached last element of buffer and it is not space. return nothing and set position to 0
               current_string.push_back(buffer[current_pos]);
              current_pos=0;
              return "";
          }//edge case when last word reached and not ended with space, it will return nothing and leave string unchanged.
        if(isspace(buffer[current_pos])){
            
            break; // if see blank space, break
        }else{
        current_string.push_back(buffer[current_pos++]);
        }
      }
      string result=current_string;
      
      current_string.clear();
      return result;

      
    }

string get_longest_palindrome(){
  string line;
  while(1){

    line=read_input();
    if(line=="END_OF_FILE_REACHED_ID1234567"){
        line=current_string;
        
        break;
    }
    if(line.size()>max_pali.size()){
      if(is_palindrome(line))
      max_pali=line;
    }
  }
  //last time when reaches the end, while loop quitted, assign line to be the current_string that it saved from buffer,
  // and compare if it can be max_pali the last time.
  if(line.size()>max_pali.size()){
      if(is_palindrome(line))
      max_pali=line;
    }
  return max_pali;
}

int main()
{
  string max_pali=get_longest_palindrome();
  

  printf("Longest palindrome: %s\n", max_pali.c_str());

  return 0;
}
   
