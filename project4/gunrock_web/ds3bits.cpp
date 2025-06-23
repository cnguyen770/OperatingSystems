#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>

#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

using namespace std;


int main(int argc, char *argv[]) {
  if (argc != 2) {
    cerr << argv[0] << ": diskImageFile" << endl;
    return 1;
  }

  // Parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  */
  
  string diskImage = argv[1];

  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);

  super_t super;
  fs->readSuperBlock(&super);

  cout << "Super" << "\n";
  cout << "inode_region_addr " << super.inode_region_addr << "\n";
  cout << "inode_region_len " << super.inode_region_len << "\n";
  cout << "num_inodes " << super.num_inodes << "\n";
  cout << "data_region_addr " << super.data_region_addr << "\n";
  cout << "data_region_len " << super.data_region_len << "\n";
  cout << "num_data " << super.num_data << "\n";
  cout << "\n";

  int effectiveInodeBitmapSize = (super.num_inodes + 7) / 8;
  int effectiveDataBitmapSize = (super.num_data + 7) / 8;

  int fullInodeBitmapSize = super.inode_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *inodeBitmap = new unsigned char[fullInodeBitmapSize];
  fs->readInodeBitmap(&super, inodeBitmap);
  cout << "Inode bitmap" << "\n";
  for (int i = 0; i < effectiveInodeBitmapSize; i++) {
    cout << (unsigned int) inodeBitmap[i] << " ";
  }
  cout << "\n\n";
  delete[] inodeBitmap;

  int fullDataBitmapSize = super.data_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *dataBitmap = new unsigned char[fullDataBitmapSize];
  fs->readDataBitmap(&super, dataBitmap);
  cout << "Data bitmap" << "\n";
  for (int i = 0; i < effectiveDataBitmapSize; i++) {
    cout << (unsigned int) dataBitmap[i] << " ";
  }
  cout << "\n";
  delete[] dataBitmap;

  return 0;
}
