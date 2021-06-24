#include <map>
#include <cstdio>
#include <unistd.h>
#include <string>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include "kvstore.hpp"
#include "config.hpp"


KeyInfo::KeyInfo(){
    start_byte = -1;
    del_flag = KeyState::DELETE;
}
KeyInfo::KeyInfo(std::string key, int start_byte, int key_mapping_start_byte, KeyState del_flag):
    key(key), start_byte(start_byte), key_mapping_start_byte(key_mapping_start_byte), del_flag(del_flag){
}
void KeyInfo::print(){
    printf("Key: %s, start byte: %d, del flag: %d\n", key.c_str(), start_byte, del_flag);
}

OperationLog::OperationLog(std::string key, KeyState fr_del_flag, int fr_value, KeyState to_del_flag, int to_value):
    key(key), fr_del_flag(fr_del_flag), fr_value(fr_value), to_del_flag(to_del_flag), to_value(to_value){
}

KV::KV(){
    key = "";
    value = 0;
    del_flag = KeyState::EXIST;
}
KV::KV(std::string key, int value, KeyState del_flag): key(key), value(value), del_flag(del_flag){
    del_flag = KeyState::EXIST;
}

KVStore::KVStore(){
    key_mapping_handle = NULL;
    data_handle = NULL;
    key_count = 0;
    key_info.clear();
    DaemonKeyMappingFileInit();
    DaemonDataFileInit();
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
            kvs[key] = KV(key, vu.value, kv.second.del_flag);
            printf("Loaded %s: %d\n", key.c_str(), vu.value);
        }
        printf("\n");
    }else
        data_handle = fopen(data_path, "wb+");
}
void KVStore::DaemonKeyMappingFileInit(){
    char s[max_key_length];
    IntValueUnion vu;
    if (access(key_mapping_path, 0) == 0){
        key_mapping_handle = fopen(key_mapping_path, "rb+");
        fread(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
        key_count = vu.value;
        for (int i = 0; i < key_count; ++i){
            int start_byte; 
            KeyState del_flag;
            fread(s, sizeof(char), max_key_length, key_mapping_handle);
            fread(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
            start_byte = vu.value;
            fread(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
            del_flag = KeyState(vu.value);
            key_info[std::string(s)] = KeyInfo(std::string(s), start_byte, sizeof(int) + i * (sizeof(int)*2 + max_key_length), del_flag);
        }
    }else{
        key_mapping_handle = fopen(key_mapping_path, "wb+");
        vu.value = key_count = 0;
        fwrite(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
    }
}
void KVStore::DropDatabase(){
    if (key_mapping_handle)
        fclose(key_mapping_handle);
    if (data_handle)
        fclose(data_handle);
    remove(key_mapping_path);
    remove(data_path);
    kvs.clear();
}
void KVStore::ModifyKey(std::string key, KeyState del_flag){
    IntValueUnion vu;
    char s[max_key_length] = {};
    if (key_info.find(key) != key_info.end()){
        if (key_info[key].del_flag == del_flag)
            return;
        key_info[key].del_flag = del_flag;
        fseek(key_mapping_handle, key_info[key].key_mapping_start_byte + max_key_length + sizeof(int), SEEK_SET);
        vu.value = del_flag;
        fwrite(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
    }else{
        key_info[key] = KeyInfo(key, 
            sizeof(int) * key_count, 
            sizeof(int) + key_count * (sizeof(int)*2 + max_key_length), 
            del_flag);
        // 定位到这个key的信息处
        fseek(key_mapping_handle, sizeof(int) + key_count * (sizeof(int)*2 + max_key_length), SEEK_SET);

        strcpy(s, key.c_str());
        fwrite(s, sizeof(char), max_key_length, key_mapping_handle);

        vu.value = sizeof(int) * key_count;
        fwrite(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);

        vu.value = del_flag;
        fwrite(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);

        // 为这个新key的值加一个空位
        vu.value = 0;
        fseek(data_handle, sizeof(int) * key_count, SEEK_SET);
        fwrite(vu.c, sizeof(IntValueUnion), 1, data_handle);

        // 定位到文件头，key的数量
        fseek(key_mapping_handle, 0, SEEK_SET);
        vu.value = ++key_count;
        fwrite(vu.c, sizeof(IntValueUnion), 1, key_mapping_handle);
    }
    fflush(key_mapping_handle);
    fflush(data_handle);
}
void KVStore::ModifyData(std::string key, int value){
    if (key_info.find(key) == key_info.end())
        return;
    IntValueUnion vu;
    fseek(data_handle, key_info[key].start_byte, SEEK_SET);
    vu.value = value;
    fwrite(vu.c, sizeof(IntValueUnion), 1, data_handle);
    fflush(data_handle);
}

void KVStore::Update(std::string key, int value, KeyState state, bool need_lock){
    if (need_lock)
        lock.lock();
    std::pair<KeyState, int> current_value = Get(key, false);
    kvs[key] = KV(key, value, state);

    ModifyKey(key, state);
    ModifyData(key, value);

    if (need_lock)
        lock.unlock();
    else{   // 不需要锁，说明这个update操作是在一个Trans内部，需要记录日志以备还原
        OperationLog l(key, current_value.first, current_value.second, state, value);
        log.push(l);
    }
}
bool KVStore::Add1(std::string key, bool need_lock){
    if (need_lock)
        lock.lock();
    std::pair<KeyState, int> current_value = Get(key, false);
    bool ret = true;
    if (current_value.first == KeyState::EXIST)
        Update(key, current_value.second + 1, KeyState::EXIST, false);
    else
        ret = false;
    if (need_lock)
        lock.unlock();
    return ret;
}
bool KVStore::Delete(std::string key, bool need_lock){
    if (need_lock)
        lock.lock();
    std::pair<KeyState, int> current_value = Get(key, false);
    bool ret = true;
    if (current_value.first == KeyState::EXIST)
        Update(key, 0, KeyState::DELETE, false);
    else
        ret = false;
    if (need_lock)
        lock.unlock();
    return ret;
}
std::pair<KeyState, int> KVStore::Get(std::string key, bool need_lock){
    if (need_lock)
        lock.lock();
    std::pair<KeyState, int> ans;
    if (kvs.find(key) != kvs.end())
        ans = std::make_pair(kvs[key].del_flag, kvs[key].value);
    else
        ans = std::make_pair(KeyState::DELETE, 0);
    if (need_lock)
        lock.unlock();
    return ans;
}
void KVStore::StartTrans(){
    lock.lock();
    while (log.size())
        log.pop();
}
void KVStore::CommitTrans(){
    lock.unlock();
}
void KVStore::AbortTrans(){
    while (log.size()){
        OperationLog l = log.top();
        log.pop();
        assert(kvs[l.key].del_flag == l.to_del_flag);
        assert(kvs[l.key].value == l.to_value);
        kvs[l.key] = KV(l.key, l.fr_value, l.fr_del_flag);
        ModifyKey(l.key, l.fr_del_flag);
        ModifyData(l.key, l.fr_value);
    }
    lock.unlock();
}