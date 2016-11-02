#include <chrono>
#include <thread>

#include "concurrent_skip_list.h"
#include "lock_free_skip_list.h"
#include "skip_list.h"

int main() {
  Timer timer;
  SkipList<int> list;
  LockFreeSkipList<int> flist(0, 10000000);
  int* arr = new int[1000000];

  std::random_device r;
  std::default_random_engine generator_(r());
  std::uniform_int_distribution<long long> distribution_(0, 100000);
  
  timer.Start();
  for (int i = 0; i < 1000000; i++) {
    long long rand = distribution_(generator_);
    // std::cout << rand << " ";
    arr[i] = i;
  }
  timer.End();
  timer.Print();

  timer.Start();
  for (int i = 0; i < 1000000; i++) {
    list.insert(arr + i);
  }
  timer.End();
  timer.Print();

  timer.Start();
  for (int i = 0; i < 250000; i++) {
    std::vector<std::thread> threads;
    for (int j = 0; j < 4; j++) {
      // std::cout << "Pushing thread #" << (i*j+j)+1 << std::endl;
      threads.push_back(std::thread(&LockFreeSkipList<int>::insert,
                                    std::ref(flist), 250000*j+i));
    }
    for (auto& thread : threads) thread.join();
  }
  timer.End();
  timer.Print();

  int* rand = new int[20000];
  for (int i = 0; i < 20000; i++) {
    *(rand + i) = distribution_(generator_);
    //    std::cout << *(rand+i) << " ";
  }
  std::cout << std::endl;

  for (int i = 0; i < 20000; i++) {
    //  std::cout << *(rand+i) << " ";
    //  std::cout << list.contains(*(rand+i)) << "\n";
  }

  return 0;
}
