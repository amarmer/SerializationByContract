// Implementation of SERIALIZATION_CONTRACT, SERIALIZE, UNSERIALIZE macros.

#pragma once 

#include "SerializationContractData.h"

namespace SerializationContract {
    // SerializeProxy
    template <typename> struct SerializeProxy;

    template <typename ...Params>
    struct SerializeProxy<void (*)(Params...)> {
        struct SerializeProcessor {
            static auto Instance(std::vector<uint8_t>& bytes) {
                return SerializeProcessor(bytes);
            }

            void operator()(const Params&... params) {
                Serializer serializer(bytes_);

                Serialize<Params...>(serializer, params...);
            }

        private:
            SerializeProcessor(std::vector<uint8_t>& bytes) : bytes_(bytes) {}

            template <typename T, typename ...Ts>
            static void Serialize(Serializer& serializer, const T& t, const Ts&... ts) {
                serializer << t;

                if constexpr (sizeof...(ts)) {
                    Serialize<Ts...>(serializer, ts...);
                }
            }

            std::vector<uint8_t>& bytes_;
        };
    };

    // UnserializeProxy
    template <typename> struct UnserializeProxy;

    template <typename ...Params>
    struct UnserializeProxy<void (*)(Params...)> {
        struct UnserializeProcessor
        {
            static auto Instance(const std::vector<uint8_t>& bytes) {
                return UnserializeProcessor(bytes);
            }

            void operator()(Params&... params) {
                Unserializer unserializer(bytes_);

                Unserialize<Params...>(unserializer, params...);
            }

        private:
            UnserializeProcessor(const std::vector<uint8_t>& bytes) : bytes_(bytes) {}

            template <typename T, typename ...Ts>
            static void Unserialize(Unserializer& unserializer, T& t, Ts&... ts) {
                unserializer >> t;

                if constexpr (sizeof...(ts)) {
                    Unserialize<Ts...>(unserializer, ts...);
                }
            }

            const std::vector<uint8_t>& bytes_;
        };
    };

}

#define SERIALIZATION_CONTRACT(x) void SerializationContract##x

#define SERIALIZE(x) SerializationContract::SerializeProxy<decltype(&SerializationContract##x)>::SerializeProcessor::Instance
#define UNSERIALIZE(x) SerializationContract::UnserializeProxy<decltype(&SerializationContract##x)>::UnserializeProcessor::Instance
