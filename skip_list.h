#pragma once

#include <iostream>
#include <random>
#include <vector>

template <class T>
class SkipNode {
 public:
  T* key_;
  std::vector<SkipNode<T>*> forward_;
  SkipNode(T* key, int level) : key_(key), forward_(level, nullptr) {}
};

template <class T>
class SkipList {
 private:
  int maximum_level_ = 16;
  constexpr static float probability_ = 0.5;
  SkipNode<T> head_;

  int RandomLevel() const {
    int level = 1;

    std::random_device r;
    std::default_random_engine generator_(r());
    std::uniform_real_distribution<double> distribution_(0.0, 1.0);

    while (distribution_(generator_) < probability_) level++;

    return std::min(level, maximum_level_);
  }

 public:
  SkipList();
  void insert(T* key);
  bool contains(T key);
};

template <class T>
SkipList<T>::SkipList() : head_(nullptr, maximum_level_) {}

template <class T>
void SkipList<T>::insert(T* key) {
  std::vector<SkipNode<T>*> update(maximum_level_, nullptr);
  SkipNode<T>* h = &head_;

  for (int i = maximum_level_ - 1; i >= 0; i--) {
    while (h->forward_[i] != nullptr && *(h->forward_[i]->key_) < *key)
      h = h->forward_[i];
    update[i] = h;
  }

  int new_level = RandomLevel();
  h = new SkipNode<T>(key, new_level);

  for (int i = 0; i < new_level; i++) {
    h->forward_[i] = update[i]->forward_[i];
    update[i]->forward_[i] = h;
  }
}

template <class T>
bool SkipList<T>::contains(T key) {
  SkipNode<T>* h = &head_;

  for (int i = maximum_level_ - 1; i >= 0; i--) {
    while (h->forward_[i] != nullptr && *(h->forward_[i]->key_) < key) {
      h = h->forward_[i];
    }
  }

  h = h->forward_[0];
  if (h == nullptr) return false;

  return *(h->key_) == key;
}
