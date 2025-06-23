#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << argv[0] << ": diskImageFile inodeNumber" << endl;
    return 1;
  }

  // Parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  int inodeNumber = stoi(argv[2]);
  */

  string diskImage = argv[1];
  int inodeNumber = atoi(argv[2]);

  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);

  inode_t inode;
  if (fs->stat(inodeNumber, &inode) != 0) {
    cerr << "Error reading file" << endl;
    return 1;
  }
  if (inode.type != UFS_REGULAR_FILE) {
    cerr << "Error reading file" << endl;
    return 1;
  }

  cout << "File blocks" << "\n";
  int totalBlocks = (inode.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  for (int i = 0; i < totalBlocks && i < DIRECT_PTRS; i++) {
    if (inode.direct[i] == 0) break;
    cout << inode.direct[i] << "\n";
  }
  cout << "\n";

  cout << "File data" << "\n";
  char *buffer = new char[inode.size + 1];
  int bytesRead = fs->read(inodeNumber, buffer, inode.size);
  if (bytesRead < 0) {
    cerr << "Error reading file" << endl;
    delete[] buffer;
    return 1;
  }
  buffer[bytesRead] = '\0';
  cout << buffer;
  //cout << "\n";
  // if (bytesRead > 0 && buffer[bytesRead - 1] != '\n') {
  //   cout << "\n";
  // }
  delete[] buffer;
  return 0;
}
