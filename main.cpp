#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>

#include <sys/mman.h>

using namespace std;

typedef unsigned char (*functionType)(unsigned char);

const string startLine = ">> ";
const string infoMessage = "JustInTime (JIT) compiler by Baykalov Vladimir. Group M3234\nCommands:\n1) Help - shows all possible commands.\n2) <pass_value> - returns result of <pass_value> + 10\n3) <pass_value> <function_value> - returns result of <pass_value> / <function_value>\nNote: all results and variables given have type \'unsigned char\'\n4) Exit - shuts the current program down.\nNote: passing empty command will shut current program down.";
const string exitMessage = "Shuts down program.";
const unsigned char defaultFunctionValue = 10;

//00000000000005fa <_Z10myFunctionh>:
//5fa:	55                   	push   %rbp
//5fb:	48 89 e5             	mov    %rsp,%rbp
//5fe:	89 f8                	mov    %edi,%eax
//600:	88 45 fc             	mov    %al,-0x4(%rbp)
//603:	0f b6 45 fc          	movzbl -0x4(%rbp),%eax
//607:	83 c0 0a             	add    $0xa,%eax
//60a:	5d                   	pop    %rbp
//60b:	c3                   	retq

unsigned char func_code[] = {
        0x55,
        0x48, 0x89, 0xe5,
        0x89, 0xf8,
        0x88, 0x45, 0xfc,
        0x0f, 0xb6, 0x45, 0xfc,
        0x83, 0xc0, 0x0a,
        0x5d,
        0xc3
};

size_t func_size = sizeof(func_code);

void printMessage(const string &message) {
    cout << message << endl;
}

void printErrorMessage(const string &message) {
    cout << "Error: " << message << endl;
}

void launchFunction(unsigned char &passValue) {

    auto memory = mmap(nullptr, func_size, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        throw runtime_error("Mapping memory error.");
    }


    memcpy(memory, func_code, func_size);
    if (mprotect(memory, func_size, PROT_EXEC | PROT_READ) == -1) {
        throw runtime_error("Changing memory protection type.");
    }


    auto newFunction = (functionType) memory;
    cout << "Return: " << to_string(newFunction(passValue)) << endl;
    if (munmap(memory, func_size) == -1) {
        throw runtime_error("Cleaning memory error.");
    }
}

int main(int argc, char *argv[]) {
    printMessage(infoMessage);

    string command;
    while (true) {
        cout << startLine;
        cout.flush();

        getline(cin, command);

        if (command.empty()) {
            printMessage(exitMessage);
            exit(0);
        }

        vector<string> tokens;
        stringstream commandStream(command);
        string tokenString;

        while (!commandStream.eof()) {
            getline(commandStream, tokenString, ' ');
            tokens.push_back(tokenString);
        }

        if (tokens.size() == 1 || tokens.size() == 2) {
            if (tokens[0] == "Help") {
                printMessage(infoMessage);
            }

            if (tokens[0] == "Exit") {
                printMessage(exitMessage);
                exit(0);
            }

            unsigned char passValue;
            unsigned char functionValue = defaultFunctionValue;

            try {
                passValue = stoi(tokens[0]);
            } catch (invalid_argument &e) {
                printErrorMessage("invalid <pass_value> given.");
                continue;
            }

            if (tokens.size() == 2) {
                try {
                    functionValue = stoi(tokens[1]);
                } catch (invalid_argument &e) {
                    printErrorMessage("invalid <function_value> given.");
                    continue;
                }

                memcpy(func_code + 15, &functionValue, 1);
            }

            cout << "Now our function will compute " << (int) passValue << " + " << (int) functionValue << endl;
            launchFunction(passValue);
            continue;
        }

        printErrorMessage("Wrong arguments. Write \'Help\' to get help message");
    }
}
