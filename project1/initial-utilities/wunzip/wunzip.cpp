#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[]){
    if (argc == 1){
        cout << "wunzip: file1 [file2 ...]" << endl;
        return 1;
    }

    for (int i = 1; i < argc; i++){
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1){
            cout << "wunzip: cannot open file" << endl;
            return 1;
        }
        int count;
        char character;
        
        while((read(fd, &count, sizeof(count)) == sizeof(count)) && 
                (read(fd, &character, sizeof(character)) == sizeof(character))){
            for(int j = 0; j < count; j++){
                write(STDOUT_FILENO, &character, sizeof(character));
            }
        }
        close(fd);
    }
    return 0;
}