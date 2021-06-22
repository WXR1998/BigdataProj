#pragma once
#include <map>
#include <string>

struct KeyInfo{
    std::string key;
    int start_byte;
    int del_flag;
    KeyInfo();
    KeyInfo(std::string key, int start_byte, int del_flag);
    void print();
};

struct KV{
    std::string key;
    int value;

    int lock;
    KV();
};

class KVStore{
private:
    FILE *key_mapping_handle;
    FILE *data_handle;
    std::map <std::string, KeyInfo> key_info;
    int key_count;
public:
    KVStore();
    void DaemonKeyMappingFileInit();
    void DaemonDataFileInit();
};