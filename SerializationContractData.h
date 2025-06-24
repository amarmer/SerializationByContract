// Serialization and unserialization of the STL data structures.

#pragma once

#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>
#include <stack>
#include <queue>
#include <memory>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream> 
#include <cstring>
#include <cstdint>

namespace SerializationContract {
  using bytes_t = std::vector<uint8_t>;

  struct Serializer {
    Serializer(bytes_t& bytes) : bytes_(bytes) { bytes_.clear(); }

    template <typename T>
    void Serialize(const T& t) {
      std::copy((char*)&t, (char*)&t + sizeof(T), back_inserter(bytes_));
    }

    template <typename T>
    Serializer& SequenceContainer(const T& arg) {
      Serializer& serializer = *this;

      serializer << arg.size();

      for (const auto& el : arg) {
        serializer << el;
      }

      return serializer;
    }

    template <typename T>
    Serializer& Set(const T& arg) {
      Serializer& serializer = *this;

      serializer << arg.size();

      for (const auto& el : arg) {
        serializer << el;
      }

      return serializer;
    }

    template<typename TKey, typename TValue>
    Serializer& Map(const std::map<TKey, TValue>& arg) {
      Serializer& serializer = *this;

      serializer << arg.size();

      for (auto& el : arg) {
        serializer << el.first << el.second;
      }

      return serializer;
    }

    template<typename T>
    Serializer& ContainerAdapter(const T& arg) {
      Serializer& serializer = *this;

      auto tmp = arg;

      serializer << arg.size();

      while (!tmp.empty()) {
        serializer << tmp.top();
        tmp.pop();
      }

      return serializer;
    }

    const bytes_t& Bytes() const {
      return bytes_;
    }

  private:
    bytes_t& bytes_;
  };

  struct Unserializer {
    Unserializer(const bytes_t& bytes) : bytes_(bytes) {}

    template <typename T>
    void Unserialize(T& t) {
      memcpy((void*)&t, &bytes_[index_], sizeof(T));

      index_ += sizeof(T);
    }

    template <typename T>
    Unserializer& SequenceContainer(T& arg) {
      Unserializer& unserializer = *this;

      arg.clear();

      size_t size;
      unserializer.Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        typename T::value_type el;

        unserializer >> el;

        arg.push_back(el);
      }

      return unserializer;
    }

    template <typename T>
    Unserializer& Set(T& arg) {
      Unserializer& unserializer = *this;

      arg.clear();

      size_t size;
      unserializer.Unserialize(size);

      using type = std::decay_t<decltype(*arg.begin())>;

      for (size_t i = 0; i < size; i++) {
        type el;
        unserializer >> el;

        arg.insert(el);
      }

      return unserializer;
    }

    template<typename TKey, typename TValue>
    Unserializer& Map(std::map<TKey, TValue>& arg) {
      Unserializer& unserializer = *this;

      arg.clear();

      size_t size;
      unserializer.Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        TKey key;
        unserializer >> key;

        TValue value;
        *this >> value;

        arg.insert(std::make_pair(key, value));
      }

      return unserializer;
    }

    template<typename T>
    Unserializer& ContainerAdapter(T& arg) {
      Unserializer& unserializer = *this;

      arg = {};

      size_t size;
      unserializer.Unserialize(size);

      using type = std::decay_t<decltype(arg.top())>;

      for (size_t i = 0; i < size; i++) {
        type el;
        unserializer >> el;

        arg.push(el);
      }

      return unserializer;
    }

  private:
    const bytes_t& bytes_;
    int index_ = 0;
  };

  // Built-in types
  template <typename T>
  Serializer& operator << (Serializer& serializer, const T& arg) {
    static_assert(!std::is_pointer_v<T>, "Cannot serialize pointer");

    static_assert(!std::is_class_v<T>, "Unserializable class");

    serializer.Serialize(arg);

    return serializer;
  }

  template <typename T>
  Unserializer& operator >> (Unserializer& unserializer, T& arg) {
    static_assert(!std::is_pointer_v<T>, "Cannot unserialize pointer");

    static_assert(!std::is_class_v<T>, "Unserializable class");

    unserializer.Unserialize(arg);

    return unserializer;
  }

  // string
  inline Serializer& operator << (Serializer& serializer, const std::string& arg) {
    return serializer.SequenceContainer(arg);
  }

  inline Unserializer& operator >> (Unserializer& unserializer, std::string& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // wstring
  inline Serializer& operator << (Serializer& serializer, const std::wstring& arg) {
    return serializer.SequenceContainer(arg);
  }

  inline Unserializer& operator >> (Unserializer& unserializer, std::wstring& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // vector
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::vector<T>& arg) {
    return serializer.SequenceContainer(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::vector<T>& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // list
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::list<T>& l) {
    return serializer.SequenceContainer(l);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::list<T>& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // deque
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::deque<T>& arg) {
    return serializer.SequenceContainer(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::deque<T>& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // forward_list
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::forward_list<T>& arg) {
    return serializer.SequenceContainer(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::forward_list<T>& arg) {
    return unserializer.SequenceContainer(arg);
  }

  // array
  template<typename T, int N>
  Serializer& operator << (Serializer& serializer, const std::array<T, N>& arg) {
    for (const auto& el : arg) {
      serializer << el;
    }

    return serializer;
  }

  template<typename T, int N>
  Unserializer& operator >> (Unserializer& unserializer, std::array<T, N>& arg) {
    for (size_t i = 0; i < arg.size(); i++) {
      T el;
      unserializer >> el;

      arg[i] = el;
    }

    return unserializer;
  }

  // set
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::set<T>& arg) {
    return serializer.Set(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::set<T>& arg) {
    return unserializer.Set(arg);
  }

  // unordered_set
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::unordered_set<T>& arg) {
    return serializer.Set(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_set<T>& arg) {
    return unserializer.Set(arg);
  }

  // multiset
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::multiset<T>& arg) {
    return serializer.Set(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::multiset<T>& arg) {
    return unserializer.Set(arg);
  }

  // map
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::map<TKey, TValue>& arg) {
    return serializer.Map(arg);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::map<TKey, TValue>& arg) {
    return unserializer.Map(arg);
  }

  // unordered_map
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::unordered_map<TKey, TValue>& arg) {
    return serializer.Map(arg);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_map<TKey, TValue>& arg) {
    return unserializer.Map(arg);
  }

  // multimap
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::multimap<TKey, TValue>& arg) {
    return serializer.Map(arg);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::multimap<TKey, TValue>& arg) {
    return unserializer.Map(arg);
  }

  // unordered_multimap
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::unordered_multimap<TKey, TValue>& arg) {
    return serializer.Map(arg);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_multimap<TKey, TValue>& arg) {
    return unserializer.Map(arg);
  }

  // stack
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::stack<T>& arg) {
    return serializer.ContainerAdapter(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::stack<T>& arg) {
    return unserializer.ContainerAdapter(arg);
  }

  // queue
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::queue<T>& arg) {
    auto tmp = arg;

    serializer << arg.size();

    while (!tmp.empty()) {
      serializer << tmp.front();
      tmp.pop();
    }

    return serializer;
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::queue<T>& arg) {
    arg = {};

    size_t size;
    unserializer.Unserialize(size);

    using type = std::decay_t<decltype(arg.front())>;

    for (size_t i = 0; i < size; i++) {
      type el;
      unserializer >> el;

      arg.push(el);
    }

    return unserializer;
  }

  // priority_queue
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::priority_queue<T>& arg) {
    return serializer.ContainerAdapter(arg);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::priority_queue<T>& arg) {
    return unserializer.ContainerAdapter(arg);
  }

  // tuple
  template<int n>
  struct Tuple {
    template <typename ...Ts>
    static void Serialize(Serializer& serializer, const std::tuple<Ts...>& arg) {
      if constexpr (n > 1) {
        Tuple<n - 1>::Serialize(serializer, arg);
      }

      serializer << std::get<n - 1>(arg);
    }

    template <typename ...Ts>
    static void Unserialize(Unserializer& unserializer, std::tuple<Ts...>& arg) {
      if constexpr (n > 1) {
        Tuple<n - 1>::Unserialize(unserializer, arg);
      }

      unserializer >> std::get<n - 1>(arg);
    }
  };

  template<typename ...Ts>
  Serializer& operator << (Serializer& serializer, const std::tuple<Ts...>& arg) {
    Tuple<sizeof...(Ts)>::Serialize(serializer, arg);

    return serializer;
  }

  template<typename ...Ts>
  Unserializer& operator >> (Unserializer& unserializer, std::tuple<Ts...>& arg) {
    Tuple<sizeof...(Ts)>::Unserialize(unserializer, arg);

    return unserializer;
  }

  // pair
  template<typename T1, typename T2>
  Serializer& operator << (Serializer& serializer, const std::pair<T1, T2>& arg) {
    return serializer << arg.first << arg.second;
  }

  template<typename T1, typename T2>
  Unserializer& operator >> (Unserializer& unserializer, std::pair<T1, T2>& arg) {
    return unserializer >> arg.first >> arg.second;
  }
}
