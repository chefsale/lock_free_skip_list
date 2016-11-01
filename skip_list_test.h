#ifndef SKIP_LIST_H
#define SKIP_LIST_H

#include <random>
#include <vector>

template <class K, class V>
class KVNode {
  public:
    K key_;
    V value_;
    std::vector<KVNode*> foward_;
    KVNode(K key, int level);
    KVNode(K key, V value, int level);
};


template <class K, class V>
class SkipList {
  private:
    constexpr static int max_level_ = 16;
    constexpr static float probability_ = 0.5;

    KVNode<K, V>* head = nullptr;
    KVNode<K, V>* tail = nullptr;

    int random_level() const;
  public:
    SkipList(K min_key, K max_key);
    ~SkipList();

    void insert(K key, V value);
    V get(K key) const;
};

template <class K, class V>
class LockFreeSkipList {
  
};


template <class K, class V>
KVNode<K, V>::KVNode(K key, int level) : key_(key) {
  for (int i = 0; i < level; i++)
    foward_.emplace_back(nullptr);
}

template <class K, class V>
KVNode<K, V>::KVNode(K key, V value, int level) : key_(key), value_(value) {
  for (int i = 0; i < level; i++)
    foward_.emplace_back(nullptr);
}

template <class K, class V>
SkipList<K, V>::SkipList(K min_key, K max_key) {
  head = new KVNode<K, V>(min_key);
  tail = new KVNode<K, V>(max_key);
  
  for (int i = 0; i < max_level_; i++)
    head->foward_[i] = tail;
}

template <class K, class V>
int SkipList<K, V>::random_level() const {
  int level = -1;

  std::default_random_engine generator;
  std::uniform_real_distribution<double> distribution(0.0,1.0);

  do {
    level++;
  } while (distribution(generator) < probability_);
  
  return std::min(level, max_level_);
}

template <class K, class V>
void SkipList<K, V>::insert(K key, V value) {
  auto* current = head;
  std::vector<KVNode<K, V>*> update(max_level_);
  for (int level = max_level_; level >= 1; level--) {
    while (current->forward_[level]->key < key ) {
      current = current->forward_[level];
    }
    update[level] = current;
  }
  current = current->foward_[0];

  if (current->key == key) {
    current->value = value;
  } else {
    int new_level = random_level();
    
  }
} 

#endif
