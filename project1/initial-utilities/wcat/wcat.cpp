#include <iostream>

#include <fcntl.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        return 0;
    }
    for (int i = 1; i < argc; i++)
    {
        int fd = open(argv[i], O_RDONLY);
        if (fd == -1)
        {
            cout << "wcat: cannot open file\n";
            return 1;
        }

        char buffer[4096];
        ssize_t bytesRead;
        while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0)
        {
            if (write(STDOUT_FILENO, buffer, bytesRead) != bytesRead)
            {
                cout << "wcat: write error\n";
                close(fd);
                return 1;
            }
        }
        close(fd);
    }
    return 0;
}