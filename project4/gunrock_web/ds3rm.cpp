#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 4) {
    cerr << argv[0] << ": diskImageFile parentInode entryName" << endl;
    return 1;
  }

  // Parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int parentInode = stoi(argv[2]);
  string entryName = string(argv[3]);
  */

  string diskImage = argv[1];
  int parentInode = stoi(argv[2]);
  string entryName = argv[3];

  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);
  
  int result = fs->unlink(parentInode, entryName);
  if (result != 0) {
    cerr << "Error removing entry" << endl;
    return 1;
  }

  return 0;
}
