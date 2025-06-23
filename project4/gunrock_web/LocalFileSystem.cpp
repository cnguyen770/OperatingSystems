#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <cstring>
#include <algorithm>

#include "include/LocalFileSystem.h"
#include "include/ufs.h"

using namespace std;


LocalFileSystem::LocalFileSystem(Disk *disk) {
  this->disk = disk;
}

void LocalFileSystem::readSuperBlock(super_t *super) {
  char block[UFS_BLOCK_SIZE];
  disk->readBlock(0, block);
  memcpy(super, block, sizeof(super_t));
}

void LocalFileSystem::readInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  int startBlock = super->inode_bitmap_addr;
  int numBlocks = super->inode_bitmap_len;
  
  for (int i = 0; i < numBlocks; i++) {
    disk->readBlock(startBlock + i, reinterpret_cast<void*>(inodeBitmap + (i * UFS_BLOCK_SIZE)));
  }
}

void LocalFileSystem::writeInodeBitmap(super_t *super, unsigned char *inodeBitmap) {
  int numBlocks = super->inode_bitmap_len;
  for (int i = 0; i < numBlocks; i++) {
    disk->writeBlock(super->inode_bitmap_addr + i, inodeBitmap + i * UFS_BLOCK_SIZE);
  }
}

void LocalFileSystem::readDataBitmap(super_t *super, unsigned char *dataBitmap) {
  int startBlock = super->data_bitmap_addr;
  int numBlocks = super->data_bitmap_len;
  
  for (int i = 0; i < numBlocks; i++) {
    disk->readBlock(startBlock + i, reinterpret_cast<void*>(dataBitmap + (i * UFS_BLOCK_SIZE)));
  }
}

void LocalFileSystem::writeDataBitmap(super_t *super, unsigned char *dataBitmap) {
  int numBlocks = super->data_bitmap_len;
  for (int i = 0; i < numBlocks; i++) {
    disk->writeBlock(super->data_bitmap_addr + i, dataBitmap + i * UFS_BLOCK_SIZE);
  }
}

void LocalFileSystem::readInodeRegion(super_t *super, inode_t *inodes) {
  int startBlock = super->inode_region_addr;
  int numBlocks = super->inode_region_len;
  
  for (int i = 0; i < numBlocks; i++) {
    disk->readBlock(startBlock + i,
      reinterpret_cast<void*>(reinterpret_cast<char*>(inodes) + (i * UFS_BLOCK_SIZE)));
  }
}

void LocalFileSystem::writeInodeRegion(super_t *super, inode_t *inodes) {
  int numBlocks = super->inode_region_len;
  for (int i = 0; i < numBlocks; i++) {
    disk->writeBlock(super->inode_region_addr + i,
      reinterpret_cast<char*>(inodes) + i * UFS_BLOCK_SIZE);
  }
}

int LocalFileSystem::lookup(int parentInodeNumber, string name) {
  inode_t parentInode;
  if (stat(parentInodeNumber, &parentInode) != 0) {
    return -1;
  }
  
  if (parentInode.type != UFS_DIRECTORY) {
    return -1;
  }
  
  //number of directory entries
  int numEntries = parentInode.size / sizeof(dir_ent_t);
  int entriesFound = 0;
  
  // iterate over the direct pointers in the parent inode
  for (int i = 0; i < DIRECT_PTRS && entriesFound < numEntries; i++) {
    if (parentInode.direct[i] == 0) continue;
    
    char blockBuffer[UFS_BLOCK_SIZE];
    disk->readBlock(parentInode.direct[i], blockBuffer);
    
    // find how many directory entries are in this block
    int entriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
    int entriesInBlock = (entriesFound + entriesPerBlock > numEntries)
                           ? (numEntries - entriesFound)
                           : entriesPerBlock;
    
    dir_ent_t *entries = reinterpret_cast<dir_ent_t*>(blockBuffer);
    for (int j = 0; j < entriesInBlock; j++) {
      if (strncmp(entries[j].name, name.c_str(), DIR_ENT_NAME_SIZE) == 0) {
        return entries[j].inum;
      }
    }
    entriesFound += entriesPerBlock;
  }
  
  return -1;
}

int LocalFileSystem::stat(int inodeNumber, inode_t *inode) {
  super_t super;
  readSuperBlock(&super);

  if (inodeNumber < 0 || inodeNumber >= super.num_inodes) {
    return -1;
  }

  int inodeRegionSize = super.inode_region_len * UFS_BLOCK_SIZE;
  char *buffer = new char[inodeRegionSize];
  for (int i = 0; i < super.inode_region_len; i++) {
    disk->readBlock(super.inode_region_addr + i, buffer + i * UFS_BLOCK_SIZE);
  }
  
  inode_t *inodeArray = reinterpret_cast<inode_t*>(buffer);
  *inode = inodeArray[inodeNumber];

  delete[] buffer;
  return 0;
}

int LocalFileSystem::read(int inodeNumber, void *buffer, int size) {
  inode_t fileInode;
  if (stat(inodeNumber, &fileInode) != 0) {
    return -1;
  }
  
  if (fileInode.type != UFS_REGULAR_FILE) {
    return -1;
  }
  
  if (size > fileInode.size) {
    size = fileInode.size;
  }
  
  int bytesRead = 0;
  int remaining = size;
  int totalBlocks = (fileInode.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  
  for (int i = 0; i < DIRECT_PTRS && i < totalBlocks && remaining > 0; i++) {
    if (fileInode.direct[i] == 0) break;
    
    char blockBuffer[UFS_BLOCK_SIZE];
    disk->readBlock(fileInode.direct[i], blockBuffer);
    
    int toCopy = (remaining < UFS_BLOCK_SIZE) ? remaining : UFS_BLOCK_SIZE;
    memcpy(reinterpret_cast<char*>(buffer) + bytesRead, blockBuffer, toCopy);
    
    bytesRead += toCopy;
    remaining -= toCopy;
  }
  
  return bytesRead;
}

static int allocateInode(super_t *super, unsigned char *inodeBitmap) __attribute__((unused));
static int allocateInode(super_t *super, unsigned char *inodeBitmap) {
  int effectiveSize = (super->num_inodes + 7) / 8;
  for (int i = 0; i < effectiveSize; i++) {
    for (int bit = 0; bit < 8; bit++) {
      int inodeNum = i * 8 + bit;
      if (inodeNum >= super->num_inodes)
        break;
      if (!(inodeBitmap[i] & (1 << bit))) {
        inodeBitmap[i] |= (1 << bit);
        return inodeNum;
      }
    }
  }
  return -1;
}


static int allocateDataBlock(super_t *super, unsigned char *dataBitmap) __attribute__((unused));
static int allocateDataBlock(super_t *super, unsigned char *dataBitmap) {
  int effectiveSize = (super->num_data + 7) / 8;
  for (int i = 0; i < effectiveSize; i++) {
    for (int bit = 0; bit < 8; bit++) {
      int dataNum = i * 8 + bit;
      if (dataNum >= super->num_data)
        break;
      if (!(dataBitmap[i] & (1 << bit))) {
        dataBitmap[i] |= (1 << bit);
        return super->data_region_addr + dataNum;
      }
    }
  }
  return -1;
}

static void freeDataBlock(super_t *super, unsigned char *dataBitmap, int blockNumber) __attribute__((unused));
static void freeDataBlock(super_t *super, unsigned char *dataBitmap, int blockNumber) {
  int dataNum = blockNumber - super->data_region_addr;
  int index = dataNum / 8;
  int bit = dataNum % 8;
  dataBitmap[index] &= ~(1 << bit);
}

static void freeInode(super_t *super, unsigned char *inodeBitmap, int inodeNum) __attribute__((unused));
static void freeInode(super_t *super, unsigned char *inodeBitmap, int inodeNum) {
  int index = inodeNum / 8;
  int bit = inodeNum % 8;
  inodeBitmap[index] &= ~(1 << bit);
}

int LocalFileSystem::create(int parentInodeNumber, int type, std::string name) {
  disk->beginTransaction();
  super_t super;
  readSuperBlock(&super);

  int inodeBitmapSize = super.inode_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *inodeBitmap = new unsigned char[inodeBitmapSize];
  readInodeBitmap(&super, inodeBitmap);

  int dataBitmapSize = super.data_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *dataBitmap = new unsigned char[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);

  int inodeRegionSize = super.inode_region_len * UFS_BLOCK_SIZE;
  int numInodesInRegion = inodeRegionSize / sizeof(inode_t);
  inode_t *inodes = new inode_t[numInodesInRegion];
  readInodeRegion(&super, inodes);

  if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  inode_t &parent = inodes[parentInodeNumber];
  if (parent.type != UFS_DIRECTORY) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }

  if (name.empty() || name == "." || name == "..") {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  if (name.length() >= DIR_ENT_NAME_SIZE) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }

  int maxEntries = DIRECT_PTRS * (UFS_BLOCK_SIZE / sizeof(dir_ent_t));
  if ((int)(parent.size / sizeof(dir_ent_t)) >= maxEntries) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }

  int existing = lookup(parentInodeNumber, name);
  if (existing != -1) {
    if (inodes[existing].type == type) {
      delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
      disk->commit();
      return existing;
    } else {
      delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
      disk->rollback();
      return -1;
    }
  }

  int newInum = allocateInode(&super, inodeBitmap);
  if (newInum == -1) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  inode_t &newInode = inodes[newInum];
  newInode.type = type;
  for (int i = 0; i < DIRECT_PTRS; i++) {
    newInode.direct[i] = 0;
  }

  if (type == UFS_DIRECTORY) {
    int block = allocateDataBlock(&super, dataBitmap);
    if (block == -1) {
      delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
      disk->rollback();
      return -1;
    }
    newInode.direct[0] = block;
    newInode.size = 2 * sizeof(dir_ent_t);
    char buf[UFS_BLOCK_SIZE] = {0};
    dir_ent_t *entries = reinterpret_cast<dir_ent_t*>(buf);
    strncpy(entries[0].name, ".", DIR_ENT_NAME_SIZE - 1);
    entries[0].name[DIR_ENT_NAME_SIZE - 1] = '\0';
    entries[0].inum = newInum;
    strncpy(entries[1].name, "..", DIR_ENT_NAME_SIZE - 1);
    entries[1].name[DIR_ENT_NAME_SIZE - 1] = '\0';
    entries[1].inum = parentInodeNumber;
    disk->writeBlock(block, buf);
  } else {
    newInode.size = 0;
  }

  dir_ent_t newEntry;
  strncpy(newEntry.name, name.c_str(), DIR_ENT_NAME_SIZE - 1);
  newEntry.name[DIR_ENT_NAME_SIZE - 1] = '\0';
  newEntry.inum = newInum;

  int entriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
  int newIndex = parent.size / sizeof(dir_ent_t);
  int blockIndex = newIndex / entriesPerBlock;
  int offset = newIndex % entriesPerBlock;

  if (blockIndex >= DIRECT_PTRS) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  if (parent.direct[blockIndex] == 0) {
    int block = allocateDataBlock(&super, dataBitmap);
    if (block == -1) {
      delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
      disk->rollback();
      return -1;
    }
    parent.direct[blockIndex] = block;
    char buf[UFS_BLOCK_SIZE] = {0};
    disk->writeBlock(parent.direct[blockIndex], buf);
  }
  char buf[UFS_BLOCK_SIZE];
  disk->readBlock(parent.direct[blockIndex], buf);
  dir_ent_t *ents = reinterpret_cast<dir_ent_t*>(buf);
  ents[offset] = newEntry;
  disk->writeBlock(parent.direct[blockIndex], buf);
  parent.size += sizeof(dir_ent_t);

  writeInodeRegion(&super, inodes);
  writeInodeBitmap(&super, inodeBitmap);
  writeDataBitmap(&super, dataBitmap);
  disk->commit();

  delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
  return newInum;
}




int LocalFileSystem::write(int inodeNumber, const void *buffer, int size) {
  disk->beginTransaction();
  super_t super;
  readSuperBlock(&super);
  
  int dataBitmapSize = super.data_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *dataBitmap = new unsigned char[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);
  
  int inodeRegionSize = super.inode_region_len * UFS_BLOCK_SIZE;
  int numInodesInRegion = inodeRegionSize / sizeof(inode_t);
  inode_t *inodes = new inode_t[numInodesInRegion];
  readInodeRegion(&super, inodes);
  
  if (inodeNumber < 0 || inodeNumber >= super.num_inodes) {
    delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  inode_t &fileInode = inodes[inodeNumber];
  if (fileInode.type != UFS_REGULAR_FILE) {
    delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return -1;
  }
  
  int oldBlocks = (fileInode.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  for (int i = 0; i < oldBlocks && i < DIRECT_PTRS; i++) {
    if (fileInode.direct[i] != 0) {
      freeDataBlock(&super, dataBitmap, fileInode.direct[i]);
      fileInode.direct[i] = 0;
    }
  }
  fileInode.size = 0;
  
  int maxCapacity = DIRECT_PTRS * UFS_BLOCK_SIZE;
  int actualSize = (size > maxCapacity) ? maxCapacity : size;
  int requiredBlocks = (actualSize + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  
  for (int i = 0; i < requiredBlocks && i < DIRECT_PTRS; i++) {
    int block = allocateDataBlock(&super, dataBitmap);
    if (block == -1) {
      requiredBlocks = i;
      break;
    }
    fileInode.direct[i] = block;
  }
  
  int bytesWritten = 0;
  const char *buf = reinterpret_cast<const char*>(buffer);
  for (int i = 0; i < requiredBlocks && bytesWritten < actualSize; i++) {
    char blockBuf[UFS_BLOCK_SIZE] = {0};
    int toCopy = (actualSize - bytesWritten < UFS_BLOCK_SIZE)
                   ? (actualSize - bytesWritten)
                   : UFS_BLOCK_SIZE;
    memcpy(blockBuf, buf + bytesWritten, toCopy);
    disk->writeBlock(fileInode.direct[i], blockBuf);
    bytesWritten += toCopy;
  }
  
  fileInode.size = bytesWritten;
  writeInodeRegion(&super, inodes);
  writeDataBitmap(&super, dataBitmap);
  disk->commit();
  
  delete[] dataBitmap; delete[] inodes;
  return bytesWritten;
}







int LocalFileSystem::unlink(int parentInodeNumber, std::string name) {
  disk->beginTransaction();
  super_t super;
  readSuperBlock(&super);

  int inodeBitmapSize = super.inode_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *inodeBitmap = new unsigned char[inodeBitmapSize];
  readInodeBitmap(&super, inodeBitmap);

  int dataBitmapSize = super.data_bitmap_len * UFS_BLOCK_SIZE;
  unsigned char *dataBitmap = new unsigned char[dataBitmapSize];
  readDataBitmap(&super, dataBitmap);

  int inodeRegionSize = super.inode_region_len * UFS_BLOCK_SIZE;
  int numInodesInRegion = inodeRegionSize / sizeof(inode_t);
  inode_t *inodes = new inode_t[numInodesInRegion];
  readInodeRegion(&super, inodes);

  if (name.empty() || name == "." || name == ".." || name.length() >= DIR_ENT_NAME_SIZE) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }

  if (parentInodeNumber < 0 || parentInodeNumber >= super.num_inodes) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }
  inode_t &parent = inodes[parentInodeNumber];
  if (parent.type != UFS_DIRECTORY) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }

  int totalEntries = parent.size / sizeof(dir_ent_t);
  if (totalEntries < 2) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }

  std::vector<dir_ent_t> dirEntries(totalEntries);
  int idx = 0;
  int entriesPerBlock = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
  for (int i = 0; i < DIRECT_PTRS && idx < totalEntries; i++) {
    if (parent.direct[i] == 0) continue;
    char blockBuf[UFS_BLOCK_SIZE];
    disk->readBlock(parent.direct[i], blockBuf);
    int count = std::min(entriesPerBlock, totalEntries - idx);
    memcpy(&dirEntries[idx], blockBuf, count * sizeof(dir_ent_t));
    idx += count;
  }

  int removeIndex = -1;
  for (int i = 0; i < totalEntries; i++) {
    if (strcmp(dirEntries[i].name, name.c_str()) == 0) {
      removeIndex = i;
      break;
    }
  }
  if (removeIndex == -1) {
    disk->commit();
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    return 0;
  }

  int removedInum = dirEntries[removeIndex].inum;
  if (removedInum < 0 || removedInum >= super.num_inodes ||
      !(inodeBitmap[removedInum / 8] & (1 << (removedInum % 8)))) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }

  inode_t &target = inodes[removedInum];
  if (target.type == UFS_DIRECTORY) {
    int targetEntries = target.size / sizeof(dir_ent_t);
    if (targetEntries > 2) {
      delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
      disk->rollback();
      return 1;
    }
  }

  if (totalEntries - 1 < 2) {
    delete[] inodeBitmap; delete[] dataBitmap; delete[] inodes;
    disk->rollback();
    return 1;
  }

  if (removeIndex != totalEntries - 1) {
    dirEntries[removeIndex] = dirEntries[totalEntries - 1];
  }
  totalEntries--;

  parent.size = totalEntries * sizeof(dir_ent_t);
  int usedBlocks = (parent.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  for (int i = usedBlocks; i < DIRECT_PTRS; i++) {
    if (parent.direct[i] != 0) {
      freeDataBlock(&super, dataBitmap, parent.direct[i]);
      parent.direct[i] = 0;
    }
  }
  idx = 0;
  for (int i = 0; i < DIRECT_PTRS && idx < totalEntries; i++) {
    if (parent.direct[i] == 0) continue;
    char blockBuf[UFS_BLOCK_SIZE] = {0};
    int count = std::min(entriesPerBlock, totalEntries - idx);
    memcpy(blockBuf, &dirEntries[idx], count * sizeof(dir_ent_t));
    idx += count;
    disk->writeBlock(parent.direct[i], blockBuf);
    if (idx >= totalEntries) break;
  }
  
  int targetBlocks = (target.size + UFS_BLOCK_SIZE - 1) / UFS_BLOCK_SIZE;
  for (int i = 0; i < targetBlocks && i < DIRECT_PTRS; i++) {
    if (target.direct[i] != 0) {
      freeDataBlock(&super, dataBitmap, target.direct[i]);
      target.direct[i] = 0;
    }
  }
  target.size = 0;
  target.type = 0;
  freeInode(&super, inodeBitmap, removedInum);

  writeInodeRegion(&super, inodes);
  writeInodeBitmap(&super, inodeBitmap);
  writeDataBitmap(&super, dataBitmap);
  disk->commit();

  delete[] inodeBitmap;
  delete[] dataBitmap;
  delete[] inodes;
  return 0;
}
