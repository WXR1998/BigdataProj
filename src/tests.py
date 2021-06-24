import client
import time

from threading import Thread
def test2():
    def run(testcase, times):
        cmd = f'client.sendMessage(client.{testcase}, {times})'
        exec(cmd)
    testcases = ['test21', 'test22', 'test23']
    threads = []
    for i in testcases:
        t = Thread(target=run, args=(i, 10))
        threads.append(t)
    for t in threads:
        t.start()

def test3():
    def run(testcase, times):
        cmd = f'client.sendMessage(client.{testcase}, {times})'
        exec(cmd)
    run('test31', 1)
    testcases = ['test32', 'test32', 'test32']
    threads = []
    for i in testcases:
        t = Thread(target=run, args=(i, -1))
        t.daemon = 1
        threads.append(t)
    start_time = time.time()
    for t in threads:
        t.start()

    while time.time() - start_time < 60:
        time.sleep(0.5)
    
    exit(0)

if __name__ == '__main__':
    test3()