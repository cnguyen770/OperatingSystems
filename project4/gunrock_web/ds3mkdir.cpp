#include <iostream>
#include <string>
#include <vector>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile parentInode directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " a.img 0 a" << endl;
    return 1;
  }

  // Parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string directory = string(argv[3]);
  */

  string diskImage = argv[1];
  int parentInode = stoi(argv[2]);
  string directoryName = argv[3];

  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);
  
  int result = fs->create(parentInode, UFS_DIRECTORY, directoryName);
  if (result == -1) {
    cerr << "Error creating directory" << endl;
    return 1;
  }
  
  return 0;
}
