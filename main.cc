#include <chrono>
#include <thread>

#include "concurrent_skip_list.h"
#include "lock_free_skip_list.h"
#include "skip_list.h"

int main() {
  SkipList<long long> list;
  LockFreeSkipList<long long> flist(0, 10000000000000);
  long long* arr = new long long[1000000];

  std::random_device r;
  std::default_random_engine generator_(r());
  std::uniform_int_distribution<long long> distribution_(0, 100000000000);

  auto startr = std::chrono::steady_clock::now();
  for (int i = 0; i < 1000000; i++) {
    long long rand = distribution_(generator_);
    // std::cout << rand << " ";
    arr[i] = rand;
  }
  auto endr = std::chrono::steady_clock::now();

  std::cout << "time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(endr -
                                                                     startr)
                   .count()
            << std::endl;


  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < 1000000; i++) {
    list.insert(arr + i);
  }
  auto end = std::chrono::steady_clock::now();

  std::cout << "time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << std::endl;

  start = std::chrono::steady_clock::now();

  for (int i = 0; i < 125000; i++) {
    std::vector<std::thread> threads;
    for (int j = 0; j < 8; j++) {
      // std::cout << "Pushing thread #" << (i*j+j)+1 << std::endl;
      threads.push_back(std::thread(&LockFreeSkipList<long long>::insert,
                                    std::ref(flist), *(arr + (i * j + j))));
    }
    for (auto& thread : threads) thread.join();
  }

  end = std::chrono::steady_clock::now();

  std::cout << "time: "
            << std::chrono::duration_cast<std::chrono::microseconds>(end -
                                                                     start)
                   .count()
            << std::endl;

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
