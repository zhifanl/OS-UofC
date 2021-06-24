//CPSC457 A3 Q1
// Author: Zhifan Li 30089428
#include "calcpi.h"
#include <pthread.h>
#include <math.h>
#include <iostream>
using namespace std;

struct argument{
  int start;
  int end;
  int id;
  uint64_t partial_count=0;
};

uint64_t partial_count_array[256];
int global_r;
double global_rsq;

void * count_partial(void * param){
 struct argument * arg=((struct argument*) param);
 long count=0;
  for( long x = arg->start ; x <= arg->end ; x ++)
    for( long y = 0 ; y <= global_r ; y ++)
      if( x*x + y*y <= global_rsq) count ++;
partial_count_array[arg->id]=count;
pthread_exit(NULL);

}
uint64_t count_pixels(int r, int n_threads)
{
  global_r=r;
  double rsq = double(r) * r;
  global_rsq=rsq;
  struct argument args[n_threads];
  pthread_t thread_pool[n_threads];
  float num=(float)r/n_threads;
  int ceiling=ceil(num);
  int useful_threads=0;
  //printf("ceiling:%i",ceiling);
  for(int i=0;i<n_threads;i++){
    args[i].id=i;
    args[i].start=1+i*ceiling;
    args[i].end=args[i].start+ceiling-1; // for example, if partition is 4, then it will be 1-4, 5-8
   // printf("start:%i end:%i\n",args[i].start,args[i].end);
    if(args[i].end>=r){
      args[i].end=r;
     // last time of creating a useful thread
      pthread_create(&thread_pool[i],NULL,count_partial,&args[i]);
       useful_threads++; 
      break;// all the tasks has been assigned to these useful threads. break the loop and start waiting
    }
    useful_threads++;

    pthread_create(&thread_pool[i],NULL,count_partial,&args[i]);
  }
  for(int i=0;i<useful_threads;i++){
    pthread_join(thread_pool[i],NULL);
  }
  uint64_t result=0;
  for(int i=0;i<useful_threads;i++){
    result+=partial_count_array[i];
  }
  return result*4+1;
}

