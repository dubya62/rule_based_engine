"""
This program's purpose is to communicate with the rule based engine
and provide simple functions that allow interfacing with it using python
"""

import subprocess

RBE_BINARY = "./rbe"

def start_process(database_files:list[str], metric, direction):

    invocation = ["./rbe", metric, direction]
    for file in database_files:
        invocation.append(file)

    the_process = subprocess.Popen(invocation, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    
    return the_process

def optimize_tokens(process, tokens):
    # separate the tokens with spaces
    the_string = " ".join(tokens) + '\n'
    process.stdin.write(the_string.encode())
    process.stdin.flush()

    line = process.stdout.readline()

    return line.decode().strip()


if __name__ == '__main__':
    database_files = ["test2.rbe"]
    process = start_process(database_files, "0", "-1")
    result = optimize_tokens(process, ["(", "(", "4", "+", "2", ")", "^", "2", "+", "2", "^", "2", ")", "+", "(", "(", "4", "+", "2", ")", "^", "2", "+", "2", "^", "2", ")"])
    print("RESULT:")
    print(result)




