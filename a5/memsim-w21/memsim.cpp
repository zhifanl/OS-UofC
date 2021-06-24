//**********************************
// Zhifan Li 30089428 CPSC457 Spring T02
// Assignment 5 Q1
//**********************************
#include "memsim.h"
#include <cassert>
#include <iostream>
#include <iterator>
#include <list>
#include <set>
#include <unordered_map>
#include <vector>
using namespace std;
// ********Citation: This code is from Pavol's hint.********
struct Partition {
  int tag;
  int64_t size;
  int64_t address;
};
typedef list<Partition>::iterator PartitionRef;

struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const
  {
    if (c1->size == c2->size) return c1->address < c2->address;
    else
      return c1->size > c2->size;
  }
};
//******** End of Citation ********
struct Simulator {
  // ********Citation: This code is from Pavol's hint.********
  list<Partition> list_partition; // list of Partition structs for occupied or empty memory
  set<PartitionRef, scmp>
      free_blocks; // set of iterator to list as key, and sorted by scmp sepcification
  unordered_map<int64_t, vector<PartitionRef>>
      tagged_blocks; // map of key:tag value: vector of iterator from list
  //******** End of Citation ********

  int64_t page_size; // page size
  int64_t page_requested = 0; // total page ordered
  int64_t max_free_partition_size = 0; // will be populated at the end of program
  int64_t max_free_partition_address = 0; // will be populated at the end of program
  Simulator(int64_t the_page_size)
  {
    // constructor
    page_size = the_page_size;
  }
  void allocate(int tag, int size)
  {
    // if no free memory in set, or largest size from set is smaller than required memory
    if (free_blocks.size() == 0 || (*free_blocks.begin())->size < size) {
      // cout<<"tag: "<<tag<<" size: "<<size<<" is asking for new pages"<<endl;
      Partition new_partition; // new parition
      new_partition.tag = tag;
      new_partition.size = size;

      int64_t num = size / page_size;
      if (size % page_size != 0) { 
        num++;
      } // calculate page needed, may not be used by some special conditions further.

      if (list_partition.size()
          == 0) { // if list is empty, set address=0, push to memory and update map
        new_partition.address = 0;
        page_requested += num;
        list_partition.emplace_back(new_partition);
        tagged_blocks[tag].push_back(prev(list_partition.end()));
        // cout<<"tag: "<<tag<<" size: "<<size<<"ask pages: "<<num<<endl;

        if (list_partition.back().size < num * page_size) { // if there are extra empty spaces in
                                                            // the page, make it a empty partion
          Partition new_free_partition;
          new_free_partition.tag = -10000001; // note that empty partition's tag is -10000001, which
                                              // is out of range from what we use
          new_free_partition.size = num * page_size - list_partition.back().size; // set size
          new_free_partition.address
              = list_partition.back().address + list_partition.back().size; // set address
          list_partition.emplace_back(new_free_partition);
          // cout<<"added free space to list: now size is: "<<list_partition.size()<<endl;
          free_blocks.insert(prev(list_partition.end())); // insert to set
          // assert: (remove later)
          // assert(prev(list_partition.end())->tag==-10000001);

          // cout<<"new free space created by: "<<tag<<" address: "<<new_free_partition.address<<"
          // size: "<<new_free_partition.size<<endl;
        }
      } else {
        if (list_partition.back().tag
            == -10000001) { // if the last partition on the list is empty, then can put some size
                            // into that empty one, then ask for new pages

          int64_t previous_free_size = list_partition.back().size;
          int64_t request_size = new_partition.size
              - previous_free_size; // update the new size if some are put into previous empty block
          int64_t num = request_size / page_size;
          if (request_size % page_size != 0) { num++; } // use new num here
          // cout<<"*****found the end of list have free space:"<<endl;
          // cout<<"previous free size is : "<<previous_free_size<<endl;
          // cout<<"recalculate request size: "<<" original: "<<new_partition.size<<" new size:
          // "<<request_size<<endl; cout<<"*****num of page asked: "<<num<<endl<<endl;
          page_requested += num;
          new_partition.address = list_partition.back().address;
          free_blocks.erase(prev(
              list_partition.end())); // remove the last free block, becasue it is no longer free.
          // assert: (remove later)
          // assert(prev(list_partition.end())->tag==-10000001);

          list_partition.pop_back(); // delete from list for the previous empty block
          list_partition.emplace_back(
              new_partition); // inseert new block in, and check if it generates empty blocks later

          tagged_blocks[tag].push_back(
              prev(list_partition.end())); // push it to the map for tracking one tag's allocations

          // assert
          // assert(prev(list_partition.end())->tag!=-10000001);

          if (request_size < num * page_size) { // *** use request size here, since the extra memory
                                                // has been given to the previous empty memory's
                                                // address, so it will decrease a bit
            Partition new_free_partition; // new partition
            new_free_partition.tag = -10000001;
            new_free_partition.size = num * page_size - request_size;
            new_free_partition.address = list_partition.back().address + list_partition.back().size;
            list_partition.emplace_back(new_free_partition); // push to list
            free_blocks.insert(prev(list_partition.end())); // insert
            // assert
            // assert(prev(list_partition.end())->tag==-10000001);

            // cout<<"new free space created by: "<<tag<<"address: "<<new_free_partition.address<<"
            // size: "<<new_free_partition.size<<endl;
          }
        } else { // if back of list is not empty
          new_partition.address = list_partition.back().address + list_partition.back().size;
          page_requested += num;
          list_partition.emplace_back(new_partition); // push to list
          tagged_blocks[tag].push_back(prev(list_partition.end())); // add to map
          // assert
          // assert(prev(list_partition.end())->tag!=-10000001);
          // cout<<"tag: "<<tag<<" size: "<<size<<"ask pages: "<<num<<endl;

          if (list_partition.back().size
              < num * page_size) { // if theres extra space, then add another empty block
            Partition new_free_partition; // new partition
            new_free_partition.tag = -10000001;
            new_free_partition.size = num * page_size - list_partition.back().size;
            new_free_partition.address = list_partition.back().address + list_partition.back().size;
            list_partition.emplace_back(new_free_partition);
            free_blocks.insert(prev(list_partition.end()));
            // assert
            // assert(prev(list_partition.end())->tag==-10000001);
            // cout<<"new free space created by: "<<tag<<" address: "<<new_free_partition.address<<"
            // size: "<<new_free_partition.size<<endl;
          }
        }
      }
    } else {
      // Now the new tag can fit into some memory without asking for new pages
      PartitionRef temp_free_space = *(free_blocks.begin());
      free_blocks.erase(free_blocks.begin()); // removed from free_blocks
      Partition new_partition; // new partition
      new_partition.tag = tag;
      new_partition.size = size;
      new_partition.address = temp_free_space->address;
      list_partition.insert(temp_free_space, new_partition); // insert new parition | free space
      tagged_blocks[tag].push_back(prev(temp_free_space)); // put the iterator in
      // cout<<"tag: "<<tag<<"address: "<<new_partition.address<<" size:
      // "<<new_partition.size<<endl;
      if (new_partition.size < temp_free_space->size) {
        // if theres extra , add new memeory as empty one, and check if can be merged with next
        // element
        if (next(temp_free_space) != list_partition.end()
            && next(temp_free_space)->tag == -10000001) {
          // if not the end and that partition is free, then can merge to bigger free partition
          free_blocks.erase(next(temp_free_space)); // removed from free_blocks
          // cout<<"***tag doesnot need new page, and the next element in list is free space, so
          // merge"<<endl;
          next(temp_free_space)->size += (temp_free_space->size - new_partition.size);
          next(temp_free_space)->address = new_partition.address + new_partition.size;
          // list_partition.erase(temp_free_space);
          free_blocks.insert(
              next(temp_free_space)); // insert it back because its size has been changed..... new
                                      // parition | new merged free space
          // assert
          // assert(next(temp_free_space)->tag==-10000001);
          // cout<<"***new bigger free space created, by merging: "<<"addres:
          // "<<next(temp_free_space)->address<<"size: "<<next(temp_free_space)->size<<endl<<endl;
        } else {
          // tag doesnot need new page, and the next element cannot merged
          // cout<<"***tag doesnot need new page, and the next element cannot merge"<<endl;
          Partition new_free_partition; // new partition
          new_free_partition.tag = -10000001;
          new_free_partition.size = temp_free_space->size - new_partition.size;
          new_free_partition.address = new_partition.address + new_partition.size;
          list_partition.insert(temp_free_space, new_free_partition); // insert
          free_blocks.insert(prev(temp_free_space)); // draw it out :   new parition | new temp free | free space
          // assert
          // assert(prev(temp_free_space)->tag==-10000001);
          // cout<<"***new free space created by: "<<tag<<"address: "<<new_free_partition.address<<"
          // size: "<<new_free_partition.size<<endl;
        }
      }
      list_partition.erase(temp_free_space); // new partition | new free if go through if cond
      // erase that free space at the end. always happens no matter what
    }
  }
  void deallocate(int tag)
  {
    auto ptr = tagged_blocks.find(tag); // find the tag
    if (ptr == tagged_blocks.end()) { return; } // if nothing found, return
    int64_t size = ptr->second.size(); // get size of that tag vector
                                       // cout<<"size of tag in map: "<<size<<endl<<endl;
    for (int64_t index = size - 1; index >= 0; index--) {
      auto a = (ptr->second)[index];
      ptr->second.pop_back();
      // a is PartitionRef here for the stored tag
      if (a == (list_partition.begin())) {
        // this is for tag starting as first element of the list

        a->tag = -10000001;
        int64_t end_size = a->size;
        int64_t end_address = a->address;
        // cout<<"a->size: "<<a->size<<endl;
        // cout<<"a->address: "<<a->address<<endl;

        while (next(a) != list_partition.end() && next(a)->tag == -10000001) {
          // cout<<"get to next one in list"<<endl;
          end_size += next(a)->size;
          a = next(a);
          free_blocks.erase(prev(a));
          list_partition.erase(prev(a));

        } // while loop to look for merging , erase that element from list and from free_blocks set
        free_blocks.erase(a); // erase a no matter what

        Partition new_free_partition;
        new_free_partition.tag = -10000001;
        new_free_partition.size = end_size;
        new_free_partition.address = end_address;
        // cout<<"size: "<<end_size<<endl;
        // cout<<"address: "<<end_address<<endl<<endl;
        // free_blocks.insert(new_free_partition);
        list_partition.insert(a, new_free_partition);
        free_blocks.insert(prev(a)); // new free partition | a
                                     // assert
                                     // assert(("1",prev(a)->tag==-10000001));
        list_partition.erase(a); // erase the old a

      } else if (a == prev(list_partition.end())) {
        // this is for tag starting as last element of the list

        a->tag = -10000001;
        int64_t end_size = a->size; // will be increased
        int64_t end_address = a->address; // need to be reduced
        while (a != list_partition.begin() && prev(a)->tag == -10000001) {
          end_size += prev(a)->size;
          end_address = prev(a)->address;
          a = prev(a);
          free_blocks.erase(next(a)); // erase of free blocks set
          list_partition.erase(next(a)); // erase that element that will be merged in the future
          // free_blocks.erase(next(a));
        }

        free_blocks.erase(a); // erase anyway

        Partition new_free_partition; // new partition created
        new_free_partition.tag = -10000001;
        new_free_partition.size = end_size;
        new_free_partition.address = end_address;
        // free_blocks.insert(new_free_partition);
        list_partition.insert(a, new_free_partition); // new free partition | a
        free_blocks.insert(prev(a));
        // assert
        // assert(("2",prev(a)->tag==-10000001));
        list_partition.erase(a); // new free partition

      } else {
        // use two while loops
        auto ptr_right = a; // the iterator for going to the right
        auto ptr_left = a; // the iterator for going to the left
        a->tag = -10000001;
        // cout<<"a->size: "<<a->size<<endl;
        // cout<<"a->address: "<<a->address<<endl;
        // free_blocks.erase(a);
        int64_t end_size_right = 0; // set to 0 first
        // int64_t end_address_right=a->address;
        // cout<<"right tag: "<<next(ptr_right)->tag<<endl;
        while (next(ptr_right) != list_partition.end()
               && next(ptr_right)->tag == -10000001) { // same while loop as above
          // cout<<"on the right: address: "<<next(ptr_right)->address<<" size:
          // "<<next(ptr_right)->size<<endl;
          end_size_right += next(ptr_right)->size;
          ptr_right = next(ptr_right);
          if (prev(ptr_right) != a) {
            free_blocks.erase(prev(ptr_right));
            list_partition.erase(prev(ptr_right));
          }
          // free_blocks.erase(prev(ptr_right));}
        }
        free_blocks.erase(ptr_right); // erase first, may add it back later
        if (ptr_right != a) { list_partition.erase(ptr_right); } // if it is not a, erase it
        // cout<<"end_size_right is: "<<end_size_right<<endl;

        // auto ptr_left=a;
        int64_t end_size_left = 0; // will be increased
        int64_t end_address_left = a->address; // need to be reduced
        while ((ptr_left) != list_partition.begin()
               && prev(ptr_left)->tag == -10000001) { // same while loop as above
          end_size_left += prev(ptr_left)->size;
          end_address_left = prev(ptr_left)->address;
          ptr_left = prev(ptr_left);
          if (next(ptr_left) != a) {
            free_blocks.erase(next(ptr_left));
            list_partition.erase(next(ptr_left));
          }
          // free_blocks.erase(next(ptr_left));}
        }

        free_blocks.erase(ptr_left);
        if (ptr_left != a) { list_partition.erase(ptr_left); }
        // cout<<"end_size_left is: "<<end_size_left<<endl;
        // cout<<"end_address_right is: "<<end_address_left<<endl<<endl;

        // if both left and right are not empty
        if (end_size_left == 0 && end_size_right == 0) {
          free_blocks.insert(a); // put original a back, since at first it was removed.
          // assert
          // assert(("3",a->tag==-10000001));
          // cout<<"didnt merge any, only the partition got free."<<endl;
        }
        // now end_size_right, end_size_left has the sizes that they get, if they are non-zero, can
        // do three-merge
        else if (end_size_left != 0 && end_size_right != 0) {
          Partition new_free_partition;
          new_free_partition.tag = -10000001;
          new_free_partition.size = end_size_left + (a->size) + end_size_right;
          new_free_partition.address = end_address_left;
          // free_blocks.insert(new_free_partition);
          // cout<<"Three merge: size is"<<new_free_partition.size<<" address is:
          // "<<end_address_left<<endl;
          list_partition.insert(a, new_free_partition); // | new free partition | a
          free_blocks.insert(prev(a));
          // assert
          // assert(("4",prev(a)->tag==-10000001));
          list_partition.erase(a); // | new free partition |
        } else if (end_size_left != 0) {
          // if only middle and left are empty
          Partition new_free_partition;
          new_free_partition.tag = -10000001;
          new_free_partition.size = end_size_left + (a->size);
          new_free_partition.address = end_address_left;
          // free_blocks.insert(new_free_partition);
          list_partition.insert(a, new_free_partition);
          // cout<<"inserted this: tag is"<<prev(a)->tag<<endl;

          free_blocks.insert(prev(a));
          // assert
          // assert(("5",prev(a)->tag==-10000001));
          list_partition.erase(a);
          // cout<<"erased this:tag is "<<a->tag<<endl;
          // cout<<"left and middle got merged: size is"<<new_free_partition.size<<" address is :
          // "<<new_free_partition.address<<endl;
        } else if (end_size_right != 0) {
          // if only middle and right are empty
          Partition new_free_partition; // new partition
          new_free_partition.tag = -10000001;
          new_free_partition.size = (a->size) + end_size_right;
          new_free_partition.address = a->address;
          // free_blocks.insert(new_free_partition);
          list_partition.insert(a, new_free_partition);
          free_blocks.insert(prev(a));
          // assert
          // assert(("6",prev(a)->tag==-10000001));
          list_partition.erase(a);
        }
      }
    }
    // assert(("taggle size is 0",(tagged_blocks.find(tag))->second.size()==0 ));
  }
  MemSimResult getStats()
  {
    MemSimResult result;
    if (free_blocks.size() != 0) {
      result.max_free_partition_size
          = (*(free_blocks.begin()))->size; // the first one in set is the biggest
      result.max_free_partition_address = (*(free_blocks.begin()))->address; // get its address
    } else {
      result.max_free_partition_size = 0;
      result.max_free_partition_address = 0;
    }
    result.n_pages_requested = page_requested; // page
    return result;
  }
  void check_consistency()
  {
    // make sure the sum of all partition sizes in your linked list_partition is
    // the same as number of page requests * page_size
    int64_t size = 0;
    for (auto & a : list_partition) { size += a.size; }
    int64_t another_size = page_size * page_requested;
    assert(another_size == size);

    // make sure your addresses are correct

    for (auto a = list_partition.begin(); a != list_partition.end(); a++) {
      if (next(a) != list_partition.end()) { assert(a->address + a->size == next(a)->address); }
    }
    // // make sure the number of all partitions in your tag data structure +
    // // number of partitions in your free blocks is the same as the size
    // // of the linked list_partition
    int64_t size_of_tag = 0;
    for (auto it = tagged_blocks.begin(); it != tagged_blocks.end(); it++) {
      for (auto & b : (tagged_blocks.find(it->first))->second) { size_of_tag += b->size; }
    }
    for (auto & a : free_blocks) { size_of_tag += a->size; }
    assert(size_of_tag == size);

    // // make sure that every free partition is in free blocks
    for (auto it = list_partition.begin(); it != list_partition.end(); it++) {
      if (it->tag == -10000001) {

        assert(free_blocks.find(it) != free_blocks.end());
        // make sure can be found in set
      }
    }

    // make sure that every partition in free_blocks is actually free
    for (auto & a : free_blocks) { assert(a->tag == -10000001); }

    // make sure that none of the partition sizes or addresses are < 1
  }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
    //sim.check_consistency();
  }

  return sim.getStats();
}
