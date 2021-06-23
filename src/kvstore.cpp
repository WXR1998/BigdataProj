#include <map>
#include <cstdio>
#include <unistd.h>
#include <string>

#include "kvstore.hpp"
#include "config.hpp"


KeyInfo::KeyInfo(){
    start_byte = -1;
    del_flag = KeyState::DELETE;
}
KeyInfo::KeyInfo(std::string key, int start_byte, KeyState del_flag):
    key(key), start_byte(start_byte), del_flag(del_flag){
}
void KeyInfo::print(){
    printf("Key: %s, start byte: %d, del flag: %d\n", key.c_str(), start_byte, del_flag);
}

OperationLog::OperationLog(KeyState fr_del_flag, int fr_value, KeyState to_del_flag, int to_value):
    fr_del_flag(fr_del_flag), fr_value(fr_value), to_del_flag(to_del_flag), to_value(to_value){
}

KV::KV(){
    key = "";
    value = 0;
    lock = LockState::UNLOCK;
}
KV::KV(std::string key, int value): key(key), value(value){
    lock = LockState::UNLOCK;
}

KVStore::KVStore(){
    key_mapping_handle = NULL;
    data_handle = NULL;
    key_count = 0;
    key_info.clear();
}
KVStore::~KVStore(){
    if (key_mapping_handle)
        fclose(key_mapping_handle);
    if (data_handle)
        fclose(data_handle);
}
void KVStore::DaemonDataFileInit(){
    if (access(data_path, 0) == 0){
        data_handle = fopen(data_path, "rb+");
        for (auto kv: key_info){
            if (kv.second.del_flag == KeyState::DELETE)
                continue;
            std::string key = kv.first;
            int start_byte = kv.second.start_byte;
            fseek(data_handle, start_byte, SEEK_SET);
            IntValueUnion vu;
            fread(vu.c, sizeof(IntValueUnion), 1, data_handle);
            kvs[key] = KV(key, vu.value);
            printf("%s: %d\n", key.c_str(), vu.value);
        }
    }else{
        data_handle = fopen(data_path, "wb+");
        // IntValueUnion vu;
        // vu.value = 123214;
        // fwrite(vu.c, sizeof(IntValueUnion), 1, data_handle);
    }
}
void KVStore::DaemonKeyMappingFileInit(){
    char s[256];
    if (access(key_mapping_path, 0) == 0){
        key_mapping_handle = fopen(key_mapping_path, "r+");
        fscanf(key_mapping_handle, "%d", &key_count);
        for (int i = 0; i < key_count; ++i){
            int start_byte; 
            KeyState del_flag;
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
void KVStore::DropDatabase(){
    if (key_mapping_handle)
        fclose(key_mapping_handle);
    if (data_handle)
        fclose(data_handle);
    remove(key_mapping_path);
    remove(data_path);
}