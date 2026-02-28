// Implementation of 'SERIALIZATION_CONTRACT'

#pragma once 

#include "SerializationContractData.h"

namespace SerializationContract {
  //
  // Processor
  //
  template <const char* Name, typename T>
  struct Processor;

  template <const char* Name, typename... Params>
  struct Processor<Name, std::function<void(Params...)>> {
    template <typename IsConstParams, typename ...Ts>
    auto CreateTupleWithParamsProxy(Ts&&... ts) {
      auto tuple = std::tuple<Ts&...>(std::forward<Ts&>(ts)...);
      return TupleWithParamsProxy<IsConstParams, decltype(tuple)>{std::move(tuple)};
    }

    // Used only for serialization.
    auto operator ()(const Params&... params) {
      return CreateTupleWithParamsProxy<std::true_type>(std::forward<const Params&>(params)...);
    }

    // Used for serialization and unserialization.
    auto operator ()(Params&... params) {
      return CreateTupleWithParamsProxy<std::false_type>(std::forward<Params&>(params)...);
    }

    template <typename IsConstParams, typename TupleWithParams>
    struct TupleWithParamsProxy {
      static constexpr auto LastTupleIndex = std::tuple_size_v<TupleWithParams> -1;

      // Serialization        
      void operator >> (std::vector<uint8_t>& bytes) {
        Serializer serializer(bytes);
        SerializeParams<0>(serializer);
      }

      template<int Index>
      void SerializeParams(Serializer& serializer) {
        if constexpr (Index == 0) {
          serializer << std::string(Name);
        }

        serializer << std::get<Index>(tupleWithParams_);

        if constexpr (Index < LastTupleIndex) {
          SerializeParams<Index + 1>(serializer);
        }
      }

      // Unserialization
      void operator << (const std::vector<uint8_t>& bytes) {
        static_assert(std::is_same_v<IsConstParams, std::false_type>, "Cannot unserialize to const");

        Unserializer unserializer(bytes);
        UnserializeParams<0>(unserializer);
      }

      template<int Index>
      void UnserializeParams(Unserializer& unserializer) {
        if constexpr (Index == 0) {
          std::string name;
          unserializer >> name;
        }

        unserializer >> std::get<Index>(tupleWithParams_);

        if constexpr (Index < LastTupleIndex) {
          UnserializeParams<Index + 1>(unserializer);
        }
      }

      TupleWithParams tupleWithParams_;
    };
  };

  //
  // UnserializeDispatcher
  //
  class UnserializeDispatcher {
  public:
    static UnserializeDispatcher& Instance() {
      static UnserializeDispatcher s_unserializeDispatcher;
      return s_unserializeDispatcher;
    }

    struct IDispatcher {
      virtual ~IDispatcher() = default;

      virtual bool Dispatch(const std::string& contractName, Unserializer& unserializer) = 0;
    };

    template <typename F, const char* Name, typename... Params>
    class TDispatcher : public IDispatcher {
    public:
      TDispatcher(F f)
        : f_(f)
      {}

      template <typename ...> struct ArgsCollector;

      template <typename T, typename ...Ts>
      struct ArgsCollector<T, Ts...> {
        template <typename ...Args>
        static void CollectArgs(F f, Unserializer& unserializer, const Args&...args) {
          T arg;
          unserializer >> arg;

          if constexpr (0 == sizeof...(Ts)) {
            f(args..., (const T&)arg);
          } else {
            ArgsCollector<Ts...>::template CollectArgs<>(f, unserializer, args..., arg);
          }
        }
      };

      bool Dispatch(const std::string& contractName, Unserializer& unserializer) override {
        if (contractName != Name) {
          return false;
        }

        ArgsCollector<Params...>::template CollectArgs<>(f_, unserializer);

        return true;
      }

      F f_;
    };

    bool Dispatch(const std::vector<uint8_t>& bytes) {
      Unserializer unserializer(bytes);

      std::string contractName;
      unserializer >> contractName;

      bool dispatched = false;

      for (const auto& pDispatcher : vDispatcher_) {
        if (pDispatcher->Dispatch(contractName, unserializer)) {
          dispatched = true;
          break;
        }
      }

      return dispatched;
    }

    template <const char* Name, typename... Params, typename F>
    void Subscribe(const Processor<Name, std::function<void(Params...)>>&, F f) {
      vDispatcher_.emplace_back(std::make_unique<TDispatcher<F, Name, Params...>>(f));
    }

    std::vector<std::unique_ptr<IDispatcher>> vDispatcher_;
  };

  //
  // UnserializeDispatcherProxy
  //
  template <const char* Name, typename... Params>
  class UnserializeDispatcherProxy {
  public:
    UnserializeDispatcherProxy(const Processor<Name, std::function<void(Params...)>>& processor)
      : processor_(processor)
    {}

    template <typename F>
    bool operator = (F f)
    {
      UnserializeDispatcher::Instance().Subscribe(processor_, f);
      return true;
    }

    Processor<Name, std::function<void(Params...)>> processor_;
  };
}

#define SERIALIZATION_CONTRACT(x, ...)                                                                                      \
  [[maybe_unused]] inline static constexpr char s_contractName##x[] = #x;                                                   \
  [[maybe_unused]] static auto x = SerializationContract::Processor<s_contractName##x, std::function<void(__VA_ARGS__)>>(); \

#define ON_SERIALIZATION_CONTRACT(x) \
    [[maybe_unused]] static bool s_onContract##x = SerializationContract::UnserializeDispatcherProxy(x) = 

#define PROCESS_SERIALIZATION_CONTRACT(bytes) \
  SerializationContract::UnserializeDispatcher::Instance().Dispatch(bytes);
