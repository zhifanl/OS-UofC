// =============================================================================
// Author Zhifan Li
// CPSC457 Assignment 3 Q3
// 2021 Spring
// 30089428
// =============================================================================

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <atomic>
#include <iostream>
using namespace std;

atomic<bool> global_finished;
atomic<bool> is_a_prime;
pthread_barrier_t my_barrier;
vector<int64_t> inputs;
vector<int64_t> global_result;
int thread_num;
int64_t global_num;

void parallel_work(int id)
{
  int64_t sqrt_num = sqrt(global_num); // set sqrt_num to be the max possible number that a factor could be, will be used for setting boundary for the value to be divided
  // set the partition of the range, with number of threads that used
  int64_t partition = ((sqrt_num - 5 + 1) / thread_num) + 1;
  int64_t start = 5 + id * partition; // set start value depends on its thread id
  int64_t temp = start;               // temp value used for end
  while ((start - 5) % 6 != 0)
  {
    start--;
  } // adjust start so that it is sure to start with 5+6*m
  int64_t end = temp + partition - 1;
  while ((end - 5) % 6 != 0)
  {
    end++;
  } //same way, adjust end
  if (end >= sqrt_num)
    end = sqrt_num; //if end exceeds the limit, set end to max
  if (is_a_prime == false)
  {
    return;
  } // before running, if already checked not prime, stop task
  while (start <= end)
  {
    if (!is_a_prime)
    {
      return;
    } // if it is checked that it is not a prime, return directly
    if (global_num % start == 0)
      is_a_prime = false; // global_is_a prime set to false since found its composite
    if (global_num % (start + 2) == 0)
      is_a_prime = false; // global_is_a prime set to false since found its composite
    start += 6;           // increment by 6
  }
}

void *grab_num(void *paramter)
{

  int id = (intptr_t)paramter;
  while (1)
  {
    int result = pthread_barrier_wait(&my_barrier);
    if (result == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      int64_t num;
      while (1)
      {
        if (inputs.size() == 0)
        {
          global_finished = true;
          break;
        }
        num = inputs.back();
        inputs.pop_back();
        // Citation : This part of code is from the code Dr.Federl gave us
        if (num < 2)
          continue;
        if (num <= 3)
        {
          global_result.emplace_back(num);
          continue; // keep going to get another element
        }           // 2 and 3 are primes
        if (num % 2 == 0)
          continue; // handle multiples of 2
        if (num % 3 == 0)
          continue; // handle multiples of 3
        // End of Citation.
        if (num > 3000000000)
        {
          break;
        } //if num is larger than 3000000000, break the loop and let multithread handle it
        else
        {
          int64_t start = 5;
          int64_t end = sqrt(num);
          bool is_prime = true;
          while (start <= end)
          {
            if (num % start == 0)
            {
              is_prime = false;
              break;
            }
            if (num % (start + 2) == 0)
            {
              is_prime = false;
              break;
            }
            start += 6; // increment by 6
          }
          if (is_prime)
          {
            global_result.emplace_back(num);
          }
        }
      }
      global_num = num;
      is_a_prime = true;
    }                                  // reset it to true; if found any composite, it becomes false;
    pthread_barrier_wait(&my_barrier); // now all threads back to work

    if (global_finished == true)
    {
      pthread_exit(NULL);
    }
    parallel_work(id);
    pthread_barrier_wait(&my_barrier);

    if (result == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      if (is_a_prime == true)
      {
        global_result.emplace_back(global_num);
      }
    }
    pthread_barrier_wait(&my_barrier);
    // now all thread is back to work
  }
}

vector<int64_t>
detect_primes(const vector<int64_t> &nums, int n_threads)
{
  thread_num = n_threads;
  inputs = nums;
  pthread_t thread_pool[n_threads];
  global_finished = false;
  pthread_barrier_init(&my_barrier, NULL, n_threads);
  for (int i = 0; i < n_threads; i++)
  {
    pthread_create(&thread_pool[i], NULL, grab_num, (void *)(intptr_t)i);
  }
  for (int i = 0; i < n_threads; i++)
    pthread_join(thread_pool[i], NULL);
  pthread_barrier_destroy(&my_barrier);
  return global_result;
}