#pragma once
#include <map>
#include <string>
#include <stack>

enum KeyState{
    EXIST = 0, DELETE = 1
};
enum LockState{
    UNLOCK = 0, LOCK = 1
};
union IntValueUnion{
    int value;
    unsigned char c[4];
};

struct KeyInfo{
    std::string key;
    int start_byte;
    KeyState del_flag;
    KeyInfo();
    KeyInfo(std::string key, int start_byte, KeyState del_flag);
    void print();
};

struct OperationLog{
    /*
        操作日志，记录每个操作操作前的值和操作后的值，以及删除标记
        插入一个kv时，del_flag是从1到0
    */
    int fr_value, to_value;
    KeyState fr_del_flag, to_del_flag;
    OperationLog(KeyState fr_del_flag, int fr_value, KeyState to_del_flag, int to_value);
};

struct KV{
    std::string key;
    int value;
    LockState lock;

    KV();
    KV(std::string key, int value);
};

class KVStore{
private:
    FILE *key_mapping_handle;
    FILE *data_handle;
    std::map <std::string, KeyInfo> key_info;
    std::map <std::string, KV> kvs;
    // 记录Trans中的操作
    std::stack <OperationLog> log;
    int key_count;
public:
    KVStore();
    ~KVStore();
    void DaemonKeyMappingFileInit();
    void DaemonDataFileInit();
    void DropDatabase();
};