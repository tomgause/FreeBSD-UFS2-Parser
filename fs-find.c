// fs-find.c
// created by Tom Gause
// 5-9-2022
// git pull; cc -o fs-find.o fs-find.c ; ./fs-find.o ../dump.out

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "/usr/src/sys/ufs/ufs/quota.h"
#include "/usr/src/sys/ufs/ffs/fs.h"
#include "/usr/src/sys/ufs/ufs/dinode.h"
#include "/usr/src/sys/ufs/ufs/inode.h"
#include "/usr/src/sys/ufs/ufs/dir.h"



int main( int argc, char *argv[] ){

  struct fs *super_block;
  struct direct *head;
  struct stat st;
  int size;
  int img;
  void *ret;
  int *ptr = NULL;
  int block_offset = -1;
  int block_add = -1;
  int cylinder_group = -1;

  // handle arguments
  if( argc == 2 ) {
    printf("Parsing %s...\n", argv[1]);
  } else if ( argc > 2) {
    printf("Too many arguments supplied.\n");
    exit(1);
  } else {
    printf("One argument expected.\n");
    exit(1);
  }

  // read in .img file
  if ((img = open(argv[1], O_RDONLY)) == -1) {
    perror("open");
    return 1;
  }

  // get size of file
  stat(argv[1], &st);
  
  if ((size = st.st_size) < 1) {
    perror("size");
    return 1;
  }

  // map memory
  ret = mmap ( NULL, size, PROT_READ, MAP_SHARED, img, 0);
  if (ret == MAP_FAILED) {
    perror("mmap");
    return 1;
  } else {
    ptr = ret;
  }
  
  // create the superblock with the offset
  super_block = (struct fs *)(ptr + SBLOCK_UFS2/4);


  printf("\nname mounted on: %s\n\n", super_block->fs_fsmnt);
  
  printf("[fs->fs_firstfield]	Historic: %x\n", super_block->fs_firstfield);
  printf("[fs->fs_unused_1]	Unused: %x\n", super_block->fs_unused_1);
  printf("[fs->fs_sblkno]	Super-block: %x\n", super_block->fs_sblkno);
  printf("[fs->fs_cblkno]	Cylinder group block: %d\n", super_block->fs_cblkno);
  printf("[fs->fs_iblkno]	Inode Blocks: %x\n", super_block->fs_iblkno);
  printf("[fs->fs_dblkno]	Data Blocks: %d\n", super_block->fs_dblkno);
  printf("[fs->fs_ipg] Inodes per group: %u\n", super_block->fs_ipg);
  printf("[fs->fs_bsize] FS Basic Block Size: %u\n", super_block->fs_bsize);
  printf("[fs->fs_fsize] FS Fragment Size: %u\n", super_block->fs_fsize);
  printf("[fs->fs_old_size] Number of blocks: %d\n", super_block->fs_old_size);
  printf("[fs->fs_ncg] Number of cylinder groups: %d\n", super_block->fs_ncg);
  // let's use some macros from fs.h

  // cylinder gorup nmumber with ino
  cylinder_group = ino_to_cg(super_block, UFS_ROOTINO);

  printf("cylinder group for root ino: %d\n", cylinder_group);

  // ino block offset in filesystem
  block_offset = ino_to_fsbo(super_block, UFS_ROOTINO); 

  // ino block add in filesystem
  block_add = ino_to_fsba(super_block, UFS_ROOTINO);

  // inode block address in cylinder group
  int ino_cg_offset = cgimin(super_block, cylinder_group);

  // filesystem block numbers into disk block addresses
  int db_add = dbtofsb(super_block, block_add);

  printf("inode block offset in filesystem: %d\n", block_offset);
  printf("inode block address in filesystem: %d \n", block_add);
  printf("cylinder group inode block address: %d \n", ino_cg_offset);
  printf("db_add: %d \n", db_add);

  int cgstart = cgstart(super_block, cylinder_group);
  // printf("cgstart: %d\n", cgstart);

  // BASE: 65536
  // NEED INODE ADD: 294912
  // 294912 - 65536 = 229376
  int offset = ino_cg_offset * (super_block->fs_fsize) + block_offset * (super_block->fs_bsize);
  printf("Offset calc: %d\n", offset);
  head = (struct direct *)(ptr + (SBLOCK_UFS2 + ino_cg_offset* (super_block->fs_fsize) + block_offset * (super_block->fs_bsize))/4);


  printf("dir ino: %d\n", head->d_ino);


 
  return 0;
}





