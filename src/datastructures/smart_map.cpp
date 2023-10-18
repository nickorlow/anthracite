#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <algorithm>
#include <bits/stdc++.h>

constexpr int benchmark_loops = 1000000;

template <typename KeyType, typename ValueType>
class smart_map {
  private:
    bool use_hmap = false;
    std::unordered_map<KeyType, ValueType> hmap;

    double assess_hmap(const std::vector<std::pair<KeyType, ValueType>> check) {
      const auto& start = std::chrono::high_resolution_clock::now();
      for(int i = 0; i < benchmark_loops; i++) {
        for(const auto& check_item : check) {
          assert(check_item.second == hmap[check_item.first]);
        }
      }
      const auto& end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration<double, std::nano>(end - start).count();
      return duration;
    }

    double assess_vmap(const std::vector<std::pair<KeyType, ValueType>> check) {
      const auto& start = std::chrono::high_resolution_clock::now();
      for(int i = 0; i < benchmark_loops; i++) {
        for(const auto& check_item : check) {
          for(const auto& item : hmap) {
            if(check_item.first == item.first) {
              assert(check_item.second == item.second);
            }
          }
        }
      }
      const auto& end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration<double, std::nano>(end - start).count();
      return duration;
    }

  public:
    smart_map () = default; 

    void assess_datastructure() {
      std::vector<std::pair<KeyType, ValueType>> vals(hmap.begin(), hmap.end());
      std::shuffle(vals.begin(), vals.end(), std::default_random_engine(570));
      use_hmap = assess_hmap(vals) > assess_vmap(vals);
    }

    bool will_use_hmap() {
      return use_hmap;
    }

    ValueType* get(const KeyType& key) {
      if(use_hmap) {
        if(hmap.find(key) != hmap.end()) {
          return &hmap[key];
        } else {
          return NULL;
        }
      }

      for(auto& item : hmap) {
        if(item.first == key) {
          std::string& ref = item.second;
          return &ref;
        }
      }
      
      return NULL; 
    }

    void insert(const KeyType key, const ValueType value) {
      hmap[key] = value;
    }
};
