#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include "server.hpp"
#include "config.hpp"
#include "kvstore.hpp"

KVStore *kvs;
// 某线程是否正在占用交易锁
bool trans_state[client_num];

void sendMessage(int socket, std::string msg){
    send(socket, msg.c_str(), msg.size() + 1, 0);
}

bool parseCommand(std::string cmd, int s_client){
    if (cmd == "ABORT"){
        if (!trans_state[s_client])
            return false;
        trans_state[s_client] = false;
        kvs->AbortTrans();
        sendMessage(s_client, done_message);
        return true;
    }else if (cmd == "COMMIT"){
        if (!trans_state[s_client])
            return false;
        trans_state[s_client] = false;
        kvs->CommitTrans();
        sendMessage(s_client, done_message);
        return true;
    }else if (cmd == "BEGIN"){
        if (trans_state[s_client])
            return false;
        trans_state[s_client] = true;
        kvs->StartTrans();
        sendMessage(s_client, done_message);
        return true;
    }else if (cmd.substr(0, 3) == "PUT"){
        cmd = cmd.substr(4);
        int p = cmd.find(' ');
        std::string key = cmd.substr(0, p);
        std::string r = cmd.substr(p+1);
        if (r[0] == '(' && r[r.size() - 1] == ')'){
            kvs->Add1(key, !trans_state[s_client]);
            sendMessage(s_client, done_message);
        }else{
            int value = atoi(r.c_str());
            kvs->Update(key, value, KeyState::EXIST, !trans_state[s_client]);
            sendMessage(s_client, done_message);
        }
        return true;
    }else if (cmd.substr(0, 3) == "GET"){
        cmd = cmd.substr(4);
        std::string key = cmd;
        auto res = kvs->Get(key, !trans_state[s_client]);
        if (res.first == KeyState::DELETE)
            sendMessage(s_client, key_not_exist_message);
        else{
            char buf[buf_size] = {};
            strcpy(buf, std::to_string(res.second).c_str());
            buf[strlen(buf)] = '\n';
            sendMessage(s_client, buf);
        }
        return true;
    }else if (cmd.substr(0, 3) == "DEL"){
        cmd = cmd.substr(4);
        std::string key = cmd;
        if (!kvs->Delete(key, !trans_state[s_client]))
            sendMessage(s_client, key_not_exist_message);
        else
            sendMessage(s_client, done_message);
        return true;
    }
    return false;
}

void* session(void *args){
    int s_client = *((int*)args);
    char recv_buf[buf_size], send_buf[buf_size];
    trans_state[s_client] = false;
    printf("创建连接 [%d]\n", s_client);
    while (true){
        memset(recv_buf, 0, sizeof(recv_buf));
        int recv_len = recv(s_client, recv_buf, buf_size, 0);
        if (recv_len <= 0){
            printf("连接 [%d] 已断开\n", s_client);
            if (trans_state[s_client])
                kvs->AbortTrans();
            return NULL;
        }
        while (recv_buf[strlen(recv_buf) - 1] == '\n' || recv_buf[strlen(recv_buf) - 1] == '\r')
            recv_buf[strlen(recv_buf) - 1] = 0;
        // printf("[%d] %s\n", s_client, recv_buf);
        std::string cmd = std::string(recv_buf);

        if (cmd == "EXIT"){
            sendMessage(s_client, connection_close_message);
            close(s_client);
            if (trans_state[s_client])
                kvs->AbortTrans();
            return NULL;
        }else if (!parseCommand(cmd, s_client))
            sendMessage(s_client, error_command_message);
    }
    return NULL;
}

void server(){
    memset(trans_state, 0, sizeof(trans_state));
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    int s_server = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(s_server, (sockaddr*)&server_addr, sizeof(sockaddr)) < 0){
        printf("无法绑定端口 %d\n", port);
        return;
    }

    kvs = new KVStore();

    printf("KVDB服务已启动，在 127.0.0.1:%d 监听连接中...\n", port);
    listen(s_server, SOMAXCONN);

    sockaddr_in client_addr;
    socklen_t client_addr_len;
    pthread_t tids[client_num];
    int s_client_list[client_num];
    int client_count = 0;
    while (true){
        int s_client = accept(s_server, (sockaddr*)&client_addr, &client_addr_len);
        s_client_list[client_count] = s_client;
        int t_ret = pthread_create(&tids[client_count], NULL, 
            session, (void*)&s_client_list[client_count]);
        client_count++;
    }

    pthread_exit(NULL);
    delete kvs;
    return ;
}