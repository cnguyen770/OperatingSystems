#include <iostream>
#include <string>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

#include <fstream>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile src_file dst_inode" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img dthread.cpp 3" << endl;
    return 1;
  }

  // Parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string srcFile = string(argv[2]);
  int dstInode = stoi(argv[3]);
  */

  string diskImage = argv[1];
  string srcFile = argv[2];
  int dstInode = stoi(argv[3]);

  ifstream infile(srcFile, ios::binary);
  if (!infile) {
    cerr << "Could not open source file" << endl;
    return 1;
  }
  stringstream buffer;
  buffer << infile.rdbuf();
  string fileContent = buffer.str();
  
  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);
  
  int bytesWritten = fs->write(dstInode, fileContent.data(), fileContent.size());
  if (bytesWritten < 0 /* || bytesWritten != static_cast<int>(fileContent.size()) */) {
    cerr << "Could not write to dst_file" << endl;
    return 1;
  }
  
  return 0;
}
