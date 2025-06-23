#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <fcntl.h>
using namespace std;

char error_message[30] = "An error has occurred\n";
static vector<string> searchPaths = {"/bin"};


string findExecutable(const string& command) {
    // Check if the command already includes a path
    if (command.find('/') != string::npos) {
        // Command includes a path, check if executable directly
        if (access(command.c_str(), X_OK) == 0) {
            return command;
        }
        return "";
    }

    // Search in the specified PATH directories
    for (const auto& dir : searchPaths) {
        string fullPath = dir + "/" + command;
        if (access(fullPath.c_str(), X_OK) == 0) {
            return fullPath; // Executable found
        }
    }
    return "";
}

struct ParsedCommand {
    std::string command;
    bool background;
};

vector<ParsedCommand> parseParallel(const string& input) {
    vector<ParsedCommand> parsedCommands;
    stringstream ss(input);
    string token;

    // split at '&'
    while (getline(ss, token, '&')) {
        // Trim leading/trailing whitespace
        auto start = token.find_first_not_of(" \t");
        auto end   = token.find_last_not_of(" \t");

        if (start == string::npos) {
            // purely whitespace or empty
            // skip if it's an empty parted
            continue;
        }

        string trimmed = token.substr(start, end - start + 1);
        bool stillData = !ss.eof();  
        // if not at end, that means an '&' is after
        parsedCommands.push_back({ trimmed, stillData });
    }
    return parsedCommands;
}


vector<string> parseRedir(const string& input) {
    vector<string> args;
    string buffer;
    for (int i = 0; i < (int)input.size(); ++i) {
        if (input[i] == '>') {
            if (!buffer.empty()) {
                args.push_back(buffer);
                buffer.clear();
            }
            args.push_back(">");
            // Continue to handle cases where '>' is immediately followed by filename without spaces
            if (i + 1 < (int)input.size() && input[i + 1] != ' ') {
                args.push_back(input.substr(i + 1));
                break;
            }
        } else if (input[i] == ' ') {
            if (!buffer.empty()) {
                args.push_back(buffer);
                buffer.clear();
            }
        } else {
            buffer += input[i];
        }
    }
    if (!buffer.empty()) {
        args.push_back(buffer);
    }
    return args;
}

int handleRedirection(vector<string>& args) {
    auto it = find(args.begin(), args.end(), ">");
    if (it != args.end()) {
        if (it == args.begin()) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            args.clear();
            return -1; //  an error occurred
        }
        auto nextIt = next(it);
        if (nextIt == args.end() || next(nextIt) != args.end()) { // No filename specified after '>'
            write(STDERR_FILENO, error_message, strlen(error_message));
            args.clear();
            return -1; //  an error occurred
        }
        //ogfd = dup(STDOUT_FILENO);
        int fd = open((next(it))->c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return -1;
        }
        if (dup2(fd, STDOUT_FILENO) == -1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            close(fd);
            return -1;
        }
        args.erase(it, args.end()); // Erase the '>' and the filename from the args
        close(fd);
    }
    return STDOUT_FILENO;
}




pid_t executeCommand(vector<string> &args, bool runInBackground){
    if (args.empty()) return -1;

    if (args.size() == 1 && args[0] == "&") {
        return -1;
    }

    //int ogfd = handleRedirection(args);

    // if (ogfd == -1) {  // Redirection error occurred
    //     return -1;
    // }

    if (args.empty()) return -1;

    if (args[0] == "exit") {
        if (args.size() > 1) { 
            write(STDERR_FILENO, error_message, strlen(error_message));
        } else {
            exit(0);
        }
        return -1;
    }

    if (args[0] == "cd"){
        if(args.size() == 2){
            if(chdir(args[1].c_str()) != 0){
                write(STDERR_FILENO, error_message, strlen(error_message)); 

            }
        }
        else{
            write(STDERR_FILENO, error_message, strlen(error_message)); 
        }
        return -1;
    }

    if (args[0] == "path") {
        searchPaths.clear();
        for (size_t i = 1; i < args.size(); ++i) {
            searchPaths.push_back(args[i]);
        }
        return -1;
    }


    string executablePath = findExecutable(args[0]);
    if (executablePath.empty()) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return -1;
    }


    pid_t pid = fork();
    //cout << "here" << endl;

    // child
    if (pid == 0){

        if (handleRedirection(args) == -1) {
            exit(1); // Exit the child process if redirection fails
        }

        vector<char*> child_args;
        for (const auto &arg : args){
            child_args.push_back(const_cast<char*>(arg.c_str()));
        }
        child_args.push_back(nullptr);
        execv(executablePath.c_str(), child_args.data());
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    //parent
    else if (pid > 0){
        if (!runInBackground){
            //cout << "waiting" << endl;
            waitpid(pid, nullptr, 0);
        }
    }
    else{
        cerr << "Error: Failed to create new process." << endl;
    }

    // if(ogfd != -1){
    //     dup2(ogfd, STDOUT_FILENO);
    //     close(ogfd);
    // }
    return pid;
}

int main(int argc, char* argv[]){
    string input;
    vector<string> args;

    istream* input_stream = &cin;
    ifstream file;

    if (argc > 1) {
        file.open(argv[1], ifstream::ate | ifstream::binary);
        if (!file) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        if (file.tellg() == 0) { // Check if the file size is 0
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        file.seekg(0, ios::beg);
        input_stream = &file; // Use file stream if in batch mode
    }

    while (true) {
        if (input_stream == &cin) {
            cout << "wish> "; // prompt in interactive mode
        }
        if (!getline(*input_stream, input)) {
            break;
        }

        // parse input for parallel commands
        auto commands = parseParallel(input);
        // for (auto i = commands.begin(); i != commands.end(); ++i){
        //     cout << (*i).command << " ";
        // }
        // cout << endl;
        // cout << commands.size() << endl;
        
        std::vector<pid_t> launchedPIDs;  // holds pids of all parted commands


        // run parallel commands
        for (auto &pc : commands) {
            vector<string> args = parseRedir(pc.command);
            if (args.empty()) {
                continue;
            }

            pid_t pid = executeCommand(args, pc.background);
            if (pid > 0) {
                launchedPIDs.push_back(pid);
            }

            //cout << "here" << endl;
            //executeCommand(args, pc.background);
        }

        // make sure all commands execute before moving to next line
        for (auto pid : launchedPIDs) {
            waitpid(pid, nullptr, 0);
        }
    }
    if (file.is_open()) {
        file.close();
    }
    return 0;
}