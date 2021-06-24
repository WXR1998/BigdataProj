#pragma once
#include <map>
#include <string>
#include <stack>
#include <mutex>
#include <cstdlib>

enum KeyState{
    EXIST = 0, DELETE = 1
};
enum TransState{
    FREE = 0, RUNNING = 1
};
union IntValueUnion{
    int value;
    unsigned char c[4];
};

struct KeyInfo{
    std::string key;
    int start_byte;
    int row_num;
    KeyState del_flag;
    KeyInfo();
    KeyInfo(std::string key, int start_byte, int row_num, KeyState del_flag);
    void print();
};

struct OperationLog{
    /*
        操作日志，记录每个操作操作前的值和操作后的值，以及删除标记
        插入一个kv时，del_flag是从1到0
    */
    std::string key;
    int fr_value, to_value;
    KeyState fr_del_flag, to_del_flag;
    OperationLog(std::string key, KeyState fr_del_flag, int fr_value, KeyState to_del_flag, int to_value);
};

struct KV{
    std::string key;
    int value;
    KeyState del_flag;

    KV();
    KV(std::string key, int value, KeyState del_flag);
};

class KVStore{
private:
    FILE *key_mapping_handle;
    FILE *data_handle;
    std::map <std::string, KeyInfo> key_info;
    std::map <std::string, KV> kvs;
    std::mutex lock;

    // 记录Trans中的操作
    std::stack <OperationLog> log;
    bool need_lock;

    int key_count;
public:
    KVStore();
    ~KVStore();
    void DaemonKeyMappingFileInit();
    void DaemonDataFileInit();
    void ModifyKey(std::string key, KeyState del_flag);
    void ModifyData(std::string key, int value);
    void DropDatabase();

    // 以下操作方法都需要传入need_lock
    void Update(std::string key, int value, KeyState state, bool need_lock);
    bool Add1(std::string key, bool need_lock);
    bool Delete(std::string key, bool need_lock);
    std::pair<KeyState, int> Get(std::string key, bool need_lock);
    void StartTrans();
    void CommitTrans();
    void AbortTrans();
};