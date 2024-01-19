#!/usr/bin/env python3

import os
import sys
import subprocess
import time


### Utility functions


def launch_process(command):
    # Launch the process with stdout and stdin pipes
    process = subprocess.Popen(
        command, bufsize=1, stdout=subprocess.PIPE, stdin=subprocess.PIPE, text=True
    )

    return process


def read_stdout(process, msg):
    # Read stdout line by line
    for line in process.stdout:
        print(line.strip())

        # Check for a specific message in stdout
        if msg in line:
            print(f"[{os.path.basename(sys.argv[0])}]: Found '{msg}' in stdout.")
            break


def send_message_to_stdin(process, message):
    # Send a message to stdin
    process.stdin.write(message + "\n")
    process.stdin.flush()


### Process manipulation functions


def test_example():
    """Example function to demonstrate how to use this script."""
    # Replace 'your_command_here' with the actual command you want to execute
    command = [
        "bash",
        "-c",
        "for i in {1..3}; do echo $i; sleep 1; done; read -p input: var1; echo received: $var1",
    ]

    # Launch the process
    my_process = launch_process(command)

    # Read stdout in a separate thread or process (if needed)
    read_stdout(my_process, "3")

    # Send a message to stdin
    message_to_send = "Hello from stdin!"
    time.sleep(2)
    send_message_to_stdin(my_process, message_to_send)
    print(my_process.stdout.read())

    # Optionally, wait for the process to complete
    my_process.wait()

    # Close stdin to indicate no more input will be sent
    my_process.stdin.close()


def test_2():
    """Example function to demonstrate how to use this script."""
    # Replace 'your_command_here' with the actual command you want to execute
    command = ["make", "test"]

    # Launch the process
    my_process = launch_process(command)

    # Read stdout in a separate thread or process (if needed)
    read_stdout(my_process, "bash")

    # Send a message to stdin
    time.sleep(2)
    send_message_to_stdin(my_process, "ls")
    print(my_process.communicate(timeout=1)[0])
    time.sleep(1)

    # Optionally, wait for the process to complete
    my_process.wait()

    # Close stdin to indicate no more input will be sent
    my_process.stdin.close()


def test_linux(args=[]):
    """Function to test if the Linux OS runs correctly on the FPGA."""
    # Command to launch Linux system on FPGA
    command = ["make", "fpga-connect"] + args

    # Launch the process
    my_process = launch_process(command)

    try:
        # Read stdout until buildroot prompts login
        read_stdout(my_process, "Welcome to Buildroot")
        time.sleep(1)

        # Send login credentials
        send_message_to_stdin(my_process, "root")
        time.sleep(3)
        print(my_process.stdout.read())
        print(1)

        # Send `uname -a` command for testing
        send_message_to_stdin(my_process, "uname -a")
        time.sleep(3)
        print(my_process.stdout.readline())
        print(2)
        read_stdout(my_process, "Linux buildroot")
        time.sleep(1)
        print(3)

        # Optionally, wait for the process to complete
        my_process.wait()

        # Close stdin to indicate no more input will be sent
        my_process.stdin.close()
    except:
        my_process.kill()
        raise


### Launcher code

# Will launch function given in `sys.argv[1]` with arguments from `sys.argv[2:]`
if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("No function name given")
        exit(1)
    if sys.argv[1] not in vars():
        print("Invalid function name")
        exit(1)
    # Call function given in sys.argv[1]
    if len(sys.argv) < 3:
        vars()[sys.argv[1]]()
    else:
        vars()[sys.argv[1]](sys.argv[2:])
