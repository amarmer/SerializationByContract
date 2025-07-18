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
    
int main(int, char**) {
  std::vector<uint8_t> bytes;
    
  // Serialization via 'XYZ' contract.
  std::vector<std::tuple<int, std::string>> in1 = {{10, "ABC1"}, {11, "ABC2"}};
  std::map<int, Data> in2 = {{15, {L"ABC3"}}};
  XYZ(in1, in2) >> bytes;

  // Unserialization via 'XYZ' contract.
  std::vector<std::tuple<int, std::string>> out1;
  std::map<int, Data> out2;
  XYZ(out1, out2) << bytes;

  assert(out1 == in1 && out2 == in2);
    
  std::cout << "!!!\n";
}
