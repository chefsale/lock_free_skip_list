#include "lock_free_skip_list.h"

#include <thread>

int main() {
  LockFreeSkipList<int> l(1, 10000);
  std::cout << "inserting" << std::endl;
  std::cout << 6 << std::endl;
  l.insert(6);
  std::cout << "inserting" << std::endl;
  std::cout << 226 << std::endl;
  l.insert(226);
  std::cout << "inserting" << std::endl;
  std::cout << 126 << std::endl;

  std::cout << "inserting in parallel 126 and 226" << std::endl;
  std::thread t1(&LockFreeSkipList<int>::insert, std::ref(l), 126);
  std::thread t2(&LockFreeSkipList<int>::insert, std::ref(l), 226);
  t2.join();
  t1.join();

  std::cout << "inserting" << std::endl;
  std::cout << 1226 << std::endl;
  l.insert(1226);
  std::cout << "inserting" << std::endl;
  std::cout << 26 << std::endl;
  l.insert(26);

  std::cout << "remove" << std::endl;
  l.remove(26);
  std::cout << "inserting in parallel 126 and 226" << std::endl;
  std::thread t3(&LockFreeSkipList<int>::insert, std::ref(l), 526);
  std::thread t4(&LockFreeSkipList<int>::insert, std::ref(l), 26);
  t3.join();
  t4.join();

  return 0;
}
