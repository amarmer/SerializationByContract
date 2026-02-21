#include <iostream>
#include <cassert>
#include "SerializationContract.h"

using namespace std;

// Example of a custom struct 'Data' and its implementation of serialization and unserialization.
struct Data {
  bool operator == (const Data& data) const { return _str == data._str; }
  std::wstring _str;
};

// 'Data' serialization and unserialization.
namespace SerializationContract {
  Serializer& operator << (Serializer& serializer, const Data& data) {
    return serializer << data._str;
  }

  Unserializer& operator >> (Unserializer& unserializer, Data& data) {
    return unserializer >> data._str;
  }
}

// Serialization contract 'XYZ', using some of the STL data structures and custom struct 'Data'.
SERIALIZATION_CONTRACT(XYZ, std::vector<std::tuple<int, std::string>>, std::map<int, Data>);

SERIALIZATION_CONTRACT(ABC, std::variant<int, float, std::variant<int, std::string>>);

SERIALIZATION_CONTRACT(QAZ, std::optional<std::vector<std::string>>, std::optional<std::string>);

int main(int, char**) {
  std::vector<uint8_t> bytes;

  //
  // Example of serializing and unserializing data.
  //

  std::optional<std::vector<std::string>> qazIn1({ "QAZ" });
  std::optional<std::string> qazIn2 = std::nullopt;
  QAZ(qazIn1, qazIn2) >> bytes;

  std::optional<std::vector<std::string>> qazOut1;
  std::optional<std::string> qazOut2;
  QAZ(qazOut1, qazOut2) << bytes;

  // Compare In and Out 'QAZ' data.
  assert(qazOut1 == qazIn1 && qazOut2 == qazIn2);

  //
  // Example of serializing data on client, after receiving 'bytes' on server, 
  // invoking corresponding contract unserialization callback.
  //

  // Server code, subscribing to 'XYZ', 'ABC' contracts.
  std::vector<std::tuple<int, std::string>> xyzOut1;
  std::map<int, Data> xyzOut2;
  ON_SERIALIZATION_CONTRACT(XYZ)[&](const std::vector<std::tuple<int, std::string>>& par1, const std::map<int, Data>& par2)
  {
    xyzOut1 = par1;
    xyzOut2 = par2;
  };

  std::variant<int, float, std::variant<int, std::string>> abcOut;
  ON_SERIALIZATION_CONTRACT(ABC)[&](const auto& par1)
  {
    abcOut = par1;
  };

  // Client code, 'XYZ' contract creates 'bytes'.
  std::vector<std::tuple<int, std::string>> xyzIn1 = { {10, "XYZ1"}, {11, "XYZ2"} };
  std::map<int, Data> xyzIn2 = { {15, {L"ABC3"}} };
  XYZ(xyzIn1, xyzIn2) >> bytes;

  // IPC sends 'bytes' from client to server.

  // Server code, invokes 'XYZ' unserialization contract.
  bool processed = PROCESS_SERIALIZATION_CONTRACT(bytes);

  // Compare client and server 'XYZ' data.
  assert(processed && xyzOut1 == xyzIn1 && xyzOut2 == xyzIn2);

  // Client code, 'ABC' contract creates 'bytes'.
  std::variant<int, float, std::variant<int, std::string>> abcIn = "ABC";
  ABC(abcIn) >> bytes;

  // Server code, invokes 'ABC' unserialization contract.
  processed = PROCESS_SERIALIZATION_CONTRACT(bytes);

  // Compare client and server 'ABC' data.
  assert(processed && abcOut == abcIn);

  // Client code, 'QAZ' contract creates 'bytes'.
  QAZ(qazIn1, qazIn2) >> bytes;

  // Server code, there is no `QAZ` subscription, 'PROCESS_SERIALIZATION_CONTRACT' returns 'false'.
  processed = PROCESS_SERIALIZATION_CONTRACT(bytes);

  assert(!processed);

  std::cout << "!!!\n";
}
