// Implementation of 'SERIALIZATION_CONTRACT'

#pragma once 

#include "SerializationContractData.h"

namespace SerializationContract {
  template <typename T> 
  struct Processor: public Processor<decltype(&T::operator())>{};

  template <typename ClassType, typename... Params>
  struct Processor<void(ClassType::*)(Params...) const> {
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
      static constexpr auto LastIndex = std::tuple_size_v<TupleWithParams> - 1;
        
      // Serialization        
      void operator >> (std::vector<uint8_t>& bytes) {
        Serializer serializer(bytes);
        SerializeParams<0>(serializer);
      }

      template<int Index>
      void SerializeParams(Serializer& serializer) {
        serializer << std::get<Index>(tupleWithParams_);

        if constexpr (Index < LastIndex) {
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
        unserializer >> std::get<Index>(tupleWithParams_);

        if constexpr (Index < LastIndex) {
          UnserializeParams<Index + 1>(unserializer);
        }
      }

      TupleWithParams tupleWithParams_;
    };
  };
}
    
#define SERIALIZATION_CONTRACT(x, ...)                                                                       \
  static auto x = [](auto&&... params) {                                                                     \
    auto lambda = [](__VA_ARGS__) {};                                                                        \
    return SerializationContract::Processor<decltype(lambda)>()(std::forward<decltype(params)>(params)...);  \
  };
