### SerializationByContract - C++ framework for serialization by contract

In any project, when a function is created for serialization, another function is created for unserialization. 

Serialization by contract employs a different approach: a contract with a name and data structures is declared, and then serialization and deserialization of the data are automated.  

For instance, contract `XYZ`:
```C++
SERIALIZATION_CONTRACT(XYZ, std::vector<std::tuple<int, std::string>>, std::wstring);
```
Then to serialize these data in `std::vector<uint8_t> bytes`:

```C++
std::vector<uint8_t> bytes;
XYZ({{10, "ABC1"}, {11, "ABC2"}}, L"ABC3") >> bytes;
```

And to unserialize from `std::vector<uint8_t> bytes`:
```C++
std::vector<std::tuple<int, std::string>> out1;
std::wstring out2;
XYZ(out1, out2) << bytes;
```

#### Framework
[SerializationContract.h](https://github.com/amarmer/SerializationByContract/blob/main/SerializationContract.h) contains implementation of SERIALIZATION_CONTRACT macro.<br/>
[SerializationContractData.h](https://github.com/amarmer/SerializationByContract/blob/main/SerializationContractData.h) contains implementation for serialization, and unserialization for most STL data structures.<br/>

Serialization, and unserialization of a custom struct `Data` can be implemented like:
```C++
struct Data {
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
```
Then `Data` can be used like any other STL data structure that is implemented in [SerializationContractData.h](https://github.com/amarmer/SerializationByContract/blob/main/SerializationContractData.h)

#### IPC

The serialization framework can be used in IPC to serialize and unserialize data by contract.

If only one type of data is sent from a client to a server, then the approach described above can be used.<br/>
If different types are sent, then on the server, the types need to be unserialized correspondingly.<br/><br/>
On server, to subscribe to the contract: `ON_SERIALIZATION_CONTRACT`.<br/>
To unserialize `bytes` to the corresponding contract:  `PROCESS_SERIALIZATION_CONTRACT(byes)`. 

For instance, for the `XYZ` contract above:
```
ON_SERIALIZATION_CONTRACT(XYZ)[&](const std::vector<std::tuple<int, std::string>>& par1, const std::wstring& par2)
{
};
```

When `bytes` are received on the server, `PROCESS_SERIALIZATION_CONTRACT(bytes)` should be called,<br/>
and the unserialized data will be dispatched to one of the callbacks `ON_SERIALIZATION_CONTRACT`.

The examples of serialization and unserialization are in [main.cpp](https://github.com/amarmer/SerializationByContract/blob/main/Main.cpp)<br/>

The framework can be tested on [https://wandbox.org/permlink/s9uWjs37WfZIEVUl](https://wandbox.org/permlink/s9uWjs37WfZIEVUl)


 
