#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <cstdlib>

using namespace std;

void grepFromFile(int fd, const string& searchTerm){
    const size_t bufferSize = 4096;
    char buffer[bufferSize];
    ssize_t bytesRead;
    string currentLine;

    while ((bytesRead = read(fd, buffer, bufferSize - 1)) > 0){
        buffer[bytesRead] = '\0';
        currentLine += buffer;
        size_t pos;
        while ((pos = currentLine.find('\n')) != string::npos){
            string line = currentLine.substr(0, pos);
            if (line.find(searchTerm) != string::npos){
                cout << line << endl;
            }
            currentLine = currentLine.substr(pos + 1);
        }
    }
    if (!currentLine.empty() && currentLine.find(searchTerm) != string::npos){
        cout << currentLine << endl;
    }
    if (bytesRead == -1){
        cout << "wgrep: read error" << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]){
    if (argc == 1){
        cout << "wgrep: searchterm [file ...]" << endl;
        return 1;
    }
    string searchTerm = argv[1];
    if (argc == 2){
        grepFromFile(STDIN_FILENO, searchTerm);
    }
    else{
        for (int i = 2; i < argc; i++){
            int fd = open(argv[i], O_RDONLY);
            if (fd == -1){
                cout << "wgrep: cannot open file" << endl;
                return 1;
            }
            grepFromFile(fd, searchTerm);
            close(fd);
        }
    }
    return 0;
}