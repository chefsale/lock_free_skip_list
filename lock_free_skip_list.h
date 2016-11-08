#ifndef LOCK_FREE_SKIP_LIST_H
#define LOCK_FREE_SKIP_LIST_H

#include <atomic>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "timer.h"

template <class T>
class MarkableReference {
 private:
  uintptr_t val;
  static const uintptr_t mask = 1;

 public:
  MarkableReference(T* ref = nullptr, bool mark = false) {
    val = ((uintptr_t)ref & ~mask) | (mark ? 1 : 0);
  }
  T* getRef(void) { return (T*)(val & ~mask); }
  bool getMark(void) { return (val & mask); }
};

template <class V>
class Node {
 public:
  const int max_level_ = 32;
  V key_;
  V value_;
  int height_;
  std::vector<std::atomic<MarkableReference<Node<V>>>*> next_;
  Node(V value) : value_(value), height_(max_level_) {
    key_ = value;
    next_.resize(max_level_,
                 new std::atomic<MarkableReference<Node<V>>>(nullptr));
  }
  Node(V value, int height) : value_(value), height_(height) {
    key_ = value;
    next_.resize(height, new std::atomic<MarkableReference<Node<V>>>(nullptr));
  }
};

template <class V>
class LockFreeSkipList {
 private:
  const int max_level_ = 32;
  float probability_ = 0.5;
  std::random_device r;
  std::default_random_engine generator_;
  std::uniform_real_distribution<double> distribution_;
  Node<V>* head_;
  Node<V>* tail_;

 public:
  LockFreeSkipList(V min_value, V max_value)
      : head_(new Node<V>(min_value)), tail_(new Node<V>(max_value)),
generator_(r()), distribution_(0.0, 1.0) {
    for (int i = 0; i < head_->next_.size(); i++) {
      head_->next_[i] = new std::atomic<MarkableReference<Node<V>>>(tail_);
    }
  }
  int random_level() {
    int level = 1;


    while (distribution_(generator_) < probability_) level++;

    return std::min(level, max_level_);
  }
  bool find(V value, std::vector<Node<V>*>& preds,
            std::vector<Node<V>*>& succs) {
    V key = value;
    bool snip;

    Node<V>* pred;
    Node<V>* curr;
    Node<V>* succ;
    bool marked = false;
  

    retry:
    while (true) {
      pred = head_;
      for (int level = max_level_ - 1; level >= 0; level--) {
        curr = pred->next_[level]->load().getRef();
        while (true) {
          succ = curr->next_[level]->load().getRef();
          marked = curr->next_[level]->load().getMark();
          while (marked) {
            auto x = MarkableReference<Node<V>>(curr, false);
            snip = pred->next_[level]->compare_exchange_strong(
                x, MarkableReference<Node<V>>(succ));

            if (!snip) goto retry;
            curr = pred->next_[level]->load().getRef();
            succ = curr->next_[level]->load().getRef();
            marked = curr->next_[level]->load().getMark();
          }
          if (curr->key_ < key) {
            pred = curr;
            curr = succ;
          } else {
            break;
          }
        }
        preds[level] = pred;
        succs[level] = curr;
      }
      return curr->key_ == key;
    }
  }
  bool insert(V value) {
    Timer t;
    t.Start();

    int top_level = random_level();
    std::vector<Node<V>*> preds(max_level_);
    std::vector<Node<V>*> succs(max_level_);
    int f = 0;

    while (true) {
      bool found = find(value, preds, succs);
      if (found) {
        return false;
      } else {
        Node<V>* new_node = new Node<V>(value, top_level);
        for (int level = 0; level < top_level; level++) {
          Node<V>* succ = succs[level];
          new_node->next_[level] =
              new std::atomic<MarkableReference<Node<V>>>(succ);
        }
        Node<V>* pred = preds[0];
        Node<V>* succ = succs[0];
        new_node->next_[0] = new std::atomic<MarkableReference<Node<V>>>(succ);
        auto x = MarkableReference<Node<V>>(succ);
        if (!pred->next_[0]->compare_exchange_strong(
                x, MarkableReference<Node<V>>(new_node)))
          continue;
        for (int level = 1; level < top_level; level++) {
          while (true) {
            pred = preds[level];
            succ = succs[level];
            auto x = MarkableReference<Node<V>>(succ);
            if (pred->next_[level]->compare_exchange_strong(
                    x, MarkableReference<Node<V>>(new_node)))
              break;
            find(value, preds, succs);
          }
        }
      }
        return true;
    }
  }
  bool remove(V value) {
    std::vector<Node<V>*> preds(max_level_);
    std::vector<Node<V>*> succs(max_level_);
    Node<V>* succ;

    while (true) {
      bool found = find(value, preds, succs);
      if (!found)
        return false;
      else {
        std::cout << "Element exists" << std::endl;
        Node<V>* remove_node = succs[0];
        for (int level = remove_node->height_ - 1; level >= 0; level--) {
          succ = remove_node->next_[level]->load().getRef();
          bool marked = remove_node->next_[level]->load().getMark();
          auto x = MarkableReference<Node<V>>(
              remove_node->next_[level]->load().getRef());
          while (!marked) {
            std::cout << "Marking" << std::endl;
            auto y = MarkableReference<Node<V>>(
                remove_node->next_[level]->load().getRef(), true);
            remove_node->next_[level]->compare_exchange_strong(x, y);
            marked = remove_node->next_[level]->load().getMark();
          }
        }
        // next
        succ = remove_node->next_[0]->load().getRef();
        bool marked = remove_node->next_[0]->load().getMark();
        while (true) {
          auto x = MarkableReference<Node<V>>(
              remove_node->next_[0]->load().getRef(), false);
          auto y = MarkableReference<Node<V>>(
              remove_node->next_[0]->load().getRef(), true);
          bool i_marked_it =
              remove_node->next_[0]->compare_exchange_strong(x, y);
          marked = succs[0]->next_[0]->load().getMark();
          if (i_marked_it) {
            find(value, preds, succs);
            return true;
          } else if (marked)
            return false;
        }
      }
    }
  }
};

#endif
