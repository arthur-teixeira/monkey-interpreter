import subprocess
import os
import time
import signal


SRC_FOLDER_NAME = './src'
COMPILE_CMD = 'make all'

path = os.path.dirname(os.path.abspath(__file__))
os.chdir(path)


def has_changes():
    modify_time = max(
            os.path.getmtime(root) for root, _, _ in os.walk(SRC_FOLDER_NAME)
    )

    cur_time = time.time()

    time_diff = cur_time - modify_time

    return time_diff <= 0.2


def recompile():
    cmd = COMPILE_CMD.split()
    subprocess.call(cmd)


recompile()

process = subprocess.Popen(['./bin/program'])

while True:
    if (has_changes()):
        os.kill(process.pid, signal.SIGINT)

        recompile()
        process = subprocess.Popen(['./bin/program'])

        time.sleep(1)
