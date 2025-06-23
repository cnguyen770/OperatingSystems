#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

#include "StringUtils.h"
#include "LocalFileSystem.h"
#include "Disk.h"
#include "ufs.h"

#include <sstream>

using namespace std;

/*
  Use this function with std::sort for directory entries
bool compareByName(const dir_ent_t& a, const dir_ent_t& b) {
    return std::strcmp(a.name, b.name) < 0;
}
*/

vector<string> splitPath(const string &path) {
  vector<string> tokens;
  istringstream iss(path);
  string token;
  while (getline(iss, token, '/')) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << argv[0] << ": diskImageFile directory" << endl;
    cerr << "For example:" << endl;
    cerr << "    $ " << argv[0] << " tests/disk_images/a.img /a/b" << endl;
    return 1;
  }

  // parse command line arguments
  /*
  Disk *disk = new Disk(argv[1], UFS_BLOCK_SIZE);
  LocalFileSystem *fileSystem = new LocalFileSystem(disk);
  string directory = string(argv[2]);
  */
  
  string diskImage = argv[1];
  string path = argv[2];

  Disk *disk = new Disk(diskImage, UFS_BLOCK_SIZE);
  LocalFileSystem *fs = new LocalFileSystem(disk);

  // Start at root inode.
  int currentInodeNumber = UFS_ROOT_DIRECTORY_INODE_NUMBER;
  vector<string> tokens = splitPath(path);

  for (const auto &token : tokens) {
    int nextInode = fs->lookup(currentInodeNumber, token);
    if (nextInode == -1) {
      cerr << "Directory not found" << endl;
      return 1;
    }
    currentInodeNumber = nextInode;
  }

  inode_t inode;
  if (fs->stat(currentInodeNumber, &inode) != 0) {
    cerr << "Directory not found" << endl;
    return 1;
  }

  if (inode.type == UFS_REGULAR_FILE) {
    string basename = tokens.empty() ? "/" : tokens.back();
    cout << currentInodeNumber << "\t" << basename << "\n";
    return 0;
  }

  int totalEntries = inode.size / sizeof(dir_ent_t);
  vector<dir_ent_t> entries;
  int entriesCollected = 0;
  for (int i = 0; i < DIRECT_PTRS && entriesCollected < totalEntries; i++) {
    if (inode.direct[i] == 0) continue;
    dir_ent_t blockEntries[UFS_BLOCK_SIZE / sizeof(dir_ent_t)];
    disk->readBlock(inode.direct[i], reinterpret_cast<void*>(blockEntries));
    int entriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
    int toRead = ((entriesCollected + entriesPerBlock) > totalEntries) ? (totalEntries - entriesCollected) : entriesPerBlock;
    for (int j = 0; j < toRead; j++) {
      entries.push_back(blockEntries[j]);
    }
    entriesCollected += entriesPerBlock;
  }

  sort(entries.begin(), entries.end(), [](const dir_ent_t &a, const dir_ent_t &b) {
    return strcmp(a.name, b.name) < 0;
  });

  for (const auto &ent : entries) {
    cout << ent.inum << "\t" << ent.name << "\n";
  }

  return 0;
}
