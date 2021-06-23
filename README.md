# BigdataProj

KV Nosql数据库，支持CRUD，数据落盘，ACID特性，可并发执行

## 如何并发执行？
给每个key加一个锁，只有获得锁的key能修改和读取它的值

## 如何满足ACID特性？
利用Trans锁，保证同时只能有一个交易进行操作。
因为不会有任何掉电的操作，只需在内存里维护一个操作日志记录，当发现ABORT，就逐条回退记录即可。
只有Trans才需要日志记录。

## 如何落盘？
每进行一个操作，就进行一次落盘，保证每个时刻内存数据和磁盘数据的一致

## 落盘文件格式

### key_mapping.txt
    key_count
    key start_byte del_flag
    ... ... ...

记录key在数据文件的第几字节开始(每个value是4个字节)，32个bit。
del_flag为0表示未被删除，为1表示已被删除。

### data.db
    data1 data2 ...
记录了每个value的值。被删除的key的值没有意义。