### SerializationByContract - C++ framework for serialization by contract

In any project, when a function is created for serialization, another function is created for unserialization. 

Serialization by contract uses another approach - a contract with name and data structures is declared, and then serialization and unserialization of the data are automatic.  

For instance contract `XYZ`:
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
[SerializationContract.h](https://github.com/amarmer/SerializationByContract/blob/main/SerializationContract.h) contains implementation of SERIALIZATION_CONTRACT, SERIALIZE, UNSERIALIZE macros.<br/>
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

An example of a contract `XYZ` and its serialization and unserialization is in [main.cpp](https://github.com/amarmer/SerializationByContract/blob/main/Main.cpp)<br/>

The framework can be tested on [[https://wandbox.org/permlink/C5mXEEAsVHgf6mnt](https://wandbox.org/permlink/aub8aG1ywtQiAtVR)]


 
