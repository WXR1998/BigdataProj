import socket
import sys

host = '127.0.0.1'
port = 12345

max_recv_len = 256

test11 = [
    'PUT A 3',
    'PUT B 4',
    'GET A',
    'GET B',
    'DEL A',
    'DEL B',
    'PUT A 5',
    'GET A',
    'GET B',
    'PUT B 5',
    'GET B'
    ]

test12 = [
    'GET A',
    'GET B'
]

test21 = [
    'PUT A 1',
    'PUT B 1',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'GET A',
    'DEL A',
    'GET B',
    'DEL B'
]

test22 = [
    'PUT C 1',
    'PUT D 1',
    'PUT C (C+1)',
    'PUT D (D+1)',
    'GET C',
    'DEL C',
    'GET D',
    'DEL D'
]

test23 = [
    'PUT E 1',
    'PUT F 1',
    'PUT E (E+1)',
    'PUT F (F+1)',
    'GET E',
    'DEL E',
    'GET F',
    'DEL F'
]

test31 = [
    'PUT A 1',
    'PUT B 1'
]

test32 = [
    'BEGIN',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'COMMIT',
    'BEGIN',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'PUT B (B+1)',
    'PUT A (A+1)',
    'PUT B (B+1)',
    'COMMIT',
    'BEGIN',
    'PUT A (A-1)',
    'PUT B (B-1)',
    'PUT A (A-1)',
    'PUT B (B-1)',
    'PUT A (A-1)',
    'PUT A (A-1)',
    'PUT B (B-1)',
    'PUT B (B-1)',
    'PUT A (A-1)',
    'PUT B (B-1)',
    'ABORT'
]

test33 = test12

def sendMessage(msg, repeat=1):
    client = socket.socket(family=socket.AF_INET, type=socket.SOCK_STREAM)
    client.settimeout(30)
    client.connect((host, port))

    def sendAll(msg):
        for cmd in msg:
            if not cmd.endswith('\n'):
                cmd = cmd + '\n'
            sendBytes = client.send(cmd.encode())
            recvData = client.recv(max_recv_len)
            recvStr = recvData.decode()
            print(cmd, end='')
            print(recvStr, end='')
    
    if repeat == -1:
        while True:
            sendAll(msg)
    else:
        for i in range(repeat):
            if i == repeat - 1:
                msg.append('EXIT\n')
            sendAll(msg)

    client.close()

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('必须给出待测试的数据名和执行次数')
        exit(0)
    exec(f'sendMessage({sys.argv[1]}, {sys.argv[2]})')