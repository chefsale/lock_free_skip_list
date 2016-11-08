#include <chrono>
#include <thread>

#include "concurrent_skip_list.h"
#include "lock_free_skip_list.h"
#include "skip_list.h"

LockFreeSkipList<int> flist(0, 1000000000);
int* arr = new int[1000000];

void insert_range(int start, int end) {
  while (start < end) {
    flist.insert(*(arr + start));
    start++;
  }
}

int main() {
  Timer timer;
  SkipList<int> list;

  std::random_device r;
  std::default_random_engine generator_(r());
  std::uniform_int_distribution<int> distribution_(0, 100000);

  for (int i = 0; i < 1000000; i++) {
    int rand = distribution_(generator_);
    arr[i] = rand;
  }

  timer.Start();
  for (int i = 0; i < 1000000; i++) list.insert(arr + i);
  timer.End();
  timer.Print();

  timer.Start();
  std::vector<std::thread> threads;
  for (int j = 0; j < 4; j++)
    threads.push_back(std::thread(insert_range, 250000 * j, 250000 * (j + 1)));
  for (auto& thread : threads) thread.join();
  timer.End();
  timer.Print();

  return 0;
}
