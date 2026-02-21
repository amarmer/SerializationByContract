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
#include <optional>
#include <variant>
#include <functional>

namespace SerializationContract {
  using bytes_t = std::vector<uint8_t>;

  struct Serializer {
    Serializer(bytes_t& bytes) : bytes_(bytes) { bytes_.clear(); }

    template <typename T>
    void Serialize(const T& t) {
      const uint8_t* dataPtr = reinterpret_cast<const uint8_t*>(&t);
      bytes_.insert(bytes_.end(), dataPtr, dataPtr + sizeof(T));
    }

    template <typename T>
    Serializer& SequenceContainer(const T& t) {
      *this << t.size();

      for (const auto& el : t) {
        *this << el;
      }

      return *this;
    }

    template <typename T>
    Serializer& Set(const T& t) {
      *this << t.size();

      for (const auto& el : t) {
        *this << el;
      }

      return *this;
    }

    template<typename TKey, typename TValue>
    Serializer& Map(const std::map<TKey, TValue>& t) {
      *this << t.size();

      for (auto& el : t) {
        *this << el.first << el.second;
      }

      return *this;
    }

    template<typename T>
    Serializer& ContainerAdapter(const T& t) {
      auto tmp = t;

      *this << t.size();

      while (!tmp.empty()) {
        *this << tmp.top();
        tmp.pop();
      }

      return *this;
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
    Unserializer& SequenceContainer(T& t) {
      t.clear();

      size_t size;
      Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        typename T::value_type el;

        *this >> el;

        t.push_back(el);
      }

      return *this;
    }

    template <typename T>
    Unserializer& Set(T& t) {
      t.clear();

      size_t size;
      Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        typename T::value_type el;

        *this >> el;

        t.insert(el);
      }

      return *this;
    }

    template<typename TKey, typename TValue>
    Unserializer& Map(std::map<TKey, TValue>& t) {
      t.clear();

      size_t size;
      Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        TKey key;
        *this >> key;

        TValue value;
        *this >> value;

        t.insert(std::make_pair(key, value));
      }

      return *this;
    }

    template<typename T>
    Unserializer& ContainerAdapter(T& t) {
      t = {};

      size_t size;
      Unserialize(size);

      for (size_t i = 0; i < size; i++) {
        typename T::value_type el;

        *this >> el;

        t.push(el);
      }

      return *this;
    }

  private:
    const bytes_t& bytes_;
    int index_ = 0;
  };

  // Built-in types
  template <typename T>
  Serializer& operator << (Serializer& serializer, const T& t) {
    static_assert(!std::is_pointer_v<T>, "Cannot serialize raw pointers.");

    static_assert(!std::is_class_v<T>, "The class doesn't implement 'Serializer& operator <<'.");

    serializer.Serialize(t);

    return serializer;
  }

  template <typename T>
  Unserializer& operator >> (Unserializer& unserializer, T& t) {
    static_assert(!std::is_pointer_v<T>, "Cannot unserialize into a raw pointer.");

    static_assert(!std::is_class_v<T>, "The class doesn't implement 'Unserializer& operator >>'.");

    unserializer.Unserialize(t);

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
  Serializer& operator << (Serializer& serializer, const std::vector<T>& t) {
    return serializer.SequenceContainer(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::vector<T>& t) {
    return unserializer.SequenceContainer(t);
  }

  // list
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::list<T>& t) {
    return serializer.SequenceContainer(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::list<T>& t) {
    return unserializer.SequenceContainer(t);
  }

  // deque
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::deque<T>& t) {
    return serializer.SequenceContainer(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::deque<T>& t) {
    return unserializer.SequenceContainer(t);
  }

  // forward_list
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::forward_list<T>& t) {
    return serializer.SequenceContainer(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::forward_list<T>& t) {
    return unserializer.SequenceContainer(t);
  }

  // array
  template<typename T, int N>
  Serializer& operator << (Serializer& serializer, const std::array<T, N>& t) {
    for (const auto& el : t) {
      serializer << el;
    }

    return serializer;
  }

  template<typename T, int N>
  Unserializer& operator >> (Unserializer& unserializer, std::array<T, N>& t) {
    for (size_t i = 0; i < t.size(); i++) {
      T el;
      unserializer >> el;

      t[i] = el;
    }

    return unserializer;
  }

  // set
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::set<T>& t) {
    return serializer.Set(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::set<T>& t) {
    return unserializer.Set(t);
  }

  // unordered_set
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::unordered_set<T>& t) {
    return serializer.Set(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_set<T>& t) {
    return unserializer.Set(t);
  }

  // multiset
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::multiset<T>& t) {
    return serializer.Set(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::multiset<T>& t) {
    return unserializer.Set(t);
  }

  // map
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::map<TKey, TValue>& t) {
    return serializer.Map(t);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::map<TKey, TValue>& t) {
    return unserializer.Map(t);
  }

  // unordered_map
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::unordered_map<TKey, TValue>& t) {
    return serializer.Map(t);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_map<TKey, TValue>& t) {
    return unserializer.Map(t);
  }

  // multimap
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::multimap<TKey, TValue>& t) {
    return serializer.Map(t);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::multimap<TKey, TValue>& t) {
    return unserializer.Map(t);
  }

  // unordered_multimap
  template<typename TKey, typename TValue>
  Serializer& operator << (Serializer& serializer, const std::unordered_multimap<TKey, TValue>& t) {
    return serializer.Map(t);
  }

  template<typename TKey, typename TValue>
  Unserializer& operator >> (Unserializer& unserializer, std::unordered_multimap<TKey, TValue>& t) {
    return unserializer.Map(t);
  }

  // stack
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::stack<T>& t) {
    return serializer.ContainerAdapter(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::stack<T>& t) {
    return unserializer.ContainerAdapter(t);
  }

  // queue
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::queue<T>& t) {
    auto tmp = t;

    serializer << t.size();

    while (!tmp.empty()) {
      serializer << tmp.front();
      tmp.pop();
    }

    return serializer;
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::queue<T>& t) {
    t = {};

    size_t size;
    unserializer.Unserialize(size);

    for (size_t i = 0; i < size; i++) {
      T el;
      unserializer >> el;

      t.push(el);
    }

    return unserializer;
  }

  // priority_queue
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::priority_queue<T>& t) {
    return serializer.ContainerAdapter(t);
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::priority_queue<T>& t) {
    return unserializer.ContainerAdapter(t);
  }

  // tuple
  template<int n>
  struct Tuple {
    template <typename ...Ts>
    static void Serialize(Serializer& serializer, const std::tuple<Ts...>& t) {
      if constexpr (n > 1) {
        Tuple<n - 1>::Serialize(serializer, t);
      }

      serializer << std::get<n - 1>(t);
    }

    template <typename ...Ts>
    static void Unserialize(Unserializer& unserializer, std::tuple<Ts...>& t) {
      if constexpr (n > 1) {
        Tuple<n - 1>::Unserialize(unserializer, t);
      }

      unserializer >> std::get<n - 1>(t);
    }
  };

  template<typename ...Ts>
  Serializer& operator << (Serializer& serializer, const std::tuple<Ts...>& t) {
    Tuple<sizeof...(Ts)>::Serialize(serializer, t);

    return serializer;
  }

  template<typename ...Ts>
  Unserializer& operator >> (Unserializer& unserializer, std::tuple<Ts...>& t) {
    Tuple<sizeof...(Ts)>::Unserialize(unserializer, t);

    return unserializer;
  }

  // pair
  template<typename T1, typename T2>
  Serializer& operator << (Serializer& serializer, const std::pair<T1, T2>& t) {
    return serializer << t.first << t.second;
  }

  template<typename T1, typename T2>
  Unserializer& operator >> (Unserializer& unserializer, std::pair<T1, T2>& t) {
    return unserializer >> t.first >> t.second;
  }

  // optional
  template<typename T>
  Serializer& operator << (Serializer& serializer, const std::optional<T>& t) {
    if (t.has_value()) {
      serializer << true;
      serializer << t.value();
    } else {
      serializer << false;
    }

    return serializer;
  }

  template<typename T>
  Unserializer& operator >> (Unserializer& unserializer, std::optional<T>& t) {
    bool hasValue;

    unserializer >> hasValue;

    if (hasValue) {
      T tmp;
      unserializer >> tmp;

      t = std::make_optional(tmp);
    } else {
      t = std::nullopt;
    }

    return unserializer;
  }

  // variant
  template<typename... Ts>
  Serializer& operator << (Serializer& serializer, const std::variant<Ts...>& t) {
    serializer << t.index();

    std::visit(
      [&](const auto& t) {
        serializer << t;
      }, 
      t);

    return serializer;
  }

  template <size_t I = 0, typename... Ts>
  void UnserializeVariant(Unserializer& unserializer, std::variant<Ts...>& t, size_t index) {
    if constexpr (I < sizeof...(Ts)) {
      if (I == index) {
        std::variant_alternative_t<I, std::variant<Ts...>> val;
        unserializer >> val;

        t.template emplace<I>(std::move(val));

        return;
      }

      UnserializeVariant<I + 1>(unserializer, t, index);
    }
  }

  template<typename... Ts>
  Unserializer& operator >> (Unserializer& unserializer, std::variant<Ts...>& t) {
    size_t index;
    unserializer >> index;

    UnserializeVariant(unserializer, t, index);

    return unserializer;
  }
}
