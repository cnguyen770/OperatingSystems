#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]){
    if (argc == 1){
        cout << "wzip: file1 [file2 ...]" << endl;
        return 1;
    }
    char lastChar;
    int count = 0;

    for (int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1){
            cout << "wzip: cannot open file" << endl;
            return 1;
        }
        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0){
            for (int j = 0; j < bytesRead; j++){
                char currentChar = buffer[j];
                if (currentChar == lastChar){
                    count++;
                }
                else{
                    if (count > 0){
                        write(STDOUT_FILENO, &count, sizeof(count));
                        write(STDOUT_FILENO, &lastChar, sizeof(lastChar));
                    }
                    lastChar = currentChar;
                    count = 1;
                }
            }
        }
        close(fd);
    }
    if (count > 0){
        write(STDOUT_FILENO, &count, sizeof(count));
        write(STDOUT_FILENO, &lastChar, sizeof(lastChar));
    }
    return 0;
}