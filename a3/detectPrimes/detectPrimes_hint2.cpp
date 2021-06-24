// =============================================================================
// You must modify this file and then submit it for grading to D2L.
// =============================================================================

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
using namespace std;
// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static bool is_prime(int64_t n)
{
  // handle trivial cases
  if (n < 2) return false;
  if (n <= 3) return true; // 2 and 3 are primes
  if (n % 2 == 0) return false; // handle multiples of 2
  if (n % 3 == 0) return false; // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)
  int64_t i = 5;
  int64_t max = sqrt(n);
  while (i <= max) {
    if (n % i == 0) return false;
    if (n % (i + 2) == 0) return false;
    i += 6;
  }
  // didn't find any divisors, so it must be a prime
  return true;
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.
pthread_mutex_t myMutex;
 vector<int64_t>  inputs;
 vector<int64_t> global_result;

void * grab_num(void * id){
   
    
    while(1){

    int64_t num;
    int temp;
    pthread_mutex_lock(&myMutex); //lock
    temp=inputs.size();
    if(temp!=0){
    num=inputs.back();
    inputs.pop_back();}
    pthread_mutex_unlock(&myMutex); //unlock

    if(temp==0)
    pthread_exit(NULL);
    
    bool result=is_prime(num);
    if(result){
      pthread_mutex_lock(&myMutex); //lock
      global_result.push_back(num);
    pthread_mutex_unlock(&myMutex); // unlock
    }
    
    }
}





vector<int64_t>
detect_primes(const vector<int64_t> & nums, int n_threads)
{
  //vector<int64_t> result;
  inputs=nums;
  pthread_t thread_pool[n_threads];
  pthread_mutex_init(&myMutex,0);
  for(int i=0;i<n_threads;i++){
    pthread_create(&thread_pool[i],NULL,grab_num,NULL);
  }
  for(int i=0;i<n_threads;i++)
  pthread_join(thread_pool[i],NULL);

  // for (auto num : nums) {
  //   if (is_prime(num)) result.push_back(num);
  // }
  // return result;
  pthread_mutex_destroy(&myMutex);
  return global_result;
}
