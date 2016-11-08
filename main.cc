#include <chrono>
#include <thread>

#include "lock_free_skip_list.h"
#include "concurrent_skip_list.h"
#include "skip_list.h"

LockFreeSkipList<int> flist(0, 1000000000);
ConcurrentSkipList<int> clist;
int* arr = new int[1000000];
int number_of_elements = 1000000;

void lock_free_insert_range(int thread_number) {
  int threads = std::thread::hardware_concurrency();
  int start = number_of_elements / threads * thread_number;
  int end = start + (number_of_elements / threads);
  while (start < end) {
    flist.insert(*(arr + start));
    start++;
  }
}

void concurrent_insert_range(int thread_number) {
  int threads = std::thread::hardware_concurrency();
  int start = number_of_elements / threads * thread_number;
  int end = start + (number_of_elements / threads);
  while (start < end) {
    clist.insert(arr + start);
    start++;
  }
}

int main() {
  Timer timer;
  SkipList<int> list;
  std::vector<std::thread> threads;

  std::random_device r;
  std::default_random_engine generator_(r());
  std::uniform_int_distribution<int> distribution_(0, 100000000);

  for (int i = 0; i < number_of_elements; i++) {
    int rand = distribution_(generator_);
    arr[i] = rand;
  }

  std::cout << "Single threaded" << std::endl;
  timer.Start();
  for (int i = 0; i < number_of_elements; i++) list.insert(arr + i);
  timer.End();
  timer.Print();

  std::cout << "Multithreaded with locks" << std::endl;
  timer.Start();
  for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
    threads.push_back(std::thread(concurrent_insert_range, i));
  }
  for (auto& thread : threads) thread.join();
  timer.End();
  timer.Print();  


  std::cout << "Multithreaded lock-free" << std::endl;
  timer.Start();
  threads.clear();
  for (int j = 0; j < 4; j++)
    threads.push_back(std::thread(lock_free_insert_range, j));
  for (auto& thread : threads) thread.join();
  timer.End();
  timer.Print();

  return 0;
}
