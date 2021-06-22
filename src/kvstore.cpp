#include <map>
#include <cstdio>
#include <unistd.h>
#include <string>

#include "kvstore.hpp"
#include "config.hpp"


KeyInfo::KeyInfo(){
    start_byte = -1;
    del_flag = 0;
}
KeyInfo::KeyInfo(std::string key, int start_byte, int del_flag):
    key(key), start_byte(start_byte), del_flag(del_flag){
}
void KeyInfo::print(){
    printf("Key: %s, start byte: %d, del flag: %d\n", key.c_str(), start_byte, del_flag);
}

KV::KV(){
    key = "";
    value = 0;
    lock = 0;
}

KVStore::KVStore(){
    key_mapping_handle = NULL;
    data_handle = NULL;
    key_count = 0;
    key_info.clear();
}
void KVStore::DaemonDataFileInit(){
}
void KVStore::DaemonKeyMappingFileInit(){
    char s[256];
    if (access(key_mapping_path, 0) == 0){
        key_mapping_handle = fopen(key_mapping_path, "r+");
        fscanf(key_mapping_handle, "%d", &key_count);
        for (int i = 0; i < key_count; ++i){
            int start_byte, del_flag;
            fscanf(key_mapping_handle, "%s%d%d", s, &start_byte, &del_flag);
            key_info[std::string(s)] = KeyInfo(std::string(s), start_byte, del_flag);
        }
        for (auto i: key_info)
            i.second.print();
    }else{
        key_mapping_handle = fopen(key_mapping_path, "w+");
        fprintf(key_mapping_handle, "%d\n", key_count = 0);
    }
}