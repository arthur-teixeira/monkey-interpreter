import subprocess
import os
import time
import signal
import sys

SRC_FOLDER_NAME = './src'
CLEAN_CMD = 'make clean'
COMPILE_CMD = 'make all'
TEST_CMD = 'make run'

ARGC = len(sys.argv)
TESTING = False

if (ARGC > 1):
    TESTING = sys.argv[1] == '-t'


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
    cmd = None
    if (TESTING):
        cmd = TEST_CMD.split()
    else:
        subprocess.call(CLEAN_CMD.split())
        cmd = COMPILE_CMD.split()
    subprocess.call(cmd)


recompile()


def get_new_process():
    return subprocess.Popen(['./bin/monkey']) if not TESTING else None


def restart_process(process):
    if process:
        os.kill(process.pid, signal.SIGINT)
    recompile()


process = get_new_process()
while True:
    if has_changes():
        subprocess.call(['clear'])
        restart_process(process)
        process = get_new_process()

        time.sleep(1)
