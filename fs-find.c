// fs-find.c
// created by Tom Gause and Will Wolf
// 5-16-2022

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
#include "/usr/src/sys/ufs/ufs/dir.h"
#include "/usr/src/sys/sys/param.h"

int parse_dir( char * ptr, struct fs * sb, char * head, int tabs );
int parse_ino( char * ptr, struct fs * sb, int ino, int tabs );


int parse_dir( char * ptr, struct fs * sb, char * head, int tabs ) {
  int max_size = DEV_BSIZE; //also DIRBLKSIZE, usually 512 bytes
  int count = 0; //janky way of tracking distance in memory from head
  struct direct *dir = (struct direct *)(head);

  while ( count < max_size ) { //halt when reaching DIRBLKSIZE

    //halts if record length is zero, i.e. no direct struct
    if (dir->d_reclen == 0) {
      return 0;
    }

    //only print directory name if not . or .. because they're ugly!
    if (strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0) {
        for (int i = 0; i < tabs; i++) { printf("  ");}
        printf("%s\n", dir->d_name);
    }

    /*
     * If the direct struct type is a directory, then we parse the inode
     * of the direct struct to look for files (and directories) inside
     * this directory. We don't recurse when the directory is . or ..,
     * as that causes infinite loops.
     */
    if ((dir->d_type == 4)  &&
        (strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0)) {
      parse_ino(ptr, sb, dir->d_ino, tabs + 1);
    }

    //increment count by the length of record and assign next direct struct
    count = count + dir->d_reclen; //record length
    dir = (struct direct *)(head + count);
  }
  return 0;
}

int parse_ino( char * ptr, struct fs * sb, int ino, int tabs ) {
  int block_offset;
  int block_add;
  int offset;
  struct ufs2_dinode *inode;
  long *di_db;

  //ino block offset in filesystem in BLOCKS
  block_offset = sizeof(struct ufs2_dinode) * ino_to_fsbo(sb, ino); 
  //ino block add in filesystem in FRAGMENTS
  block_add = sb->fs_fsize * ino_to_fsba(sb, ino);

  //get the offset to the inode "ino" and declare inode struct
  offset = block_add + block_offset;
  inode = (struct ufs2_dinode *)(ptr + offset);

  printf("filetype: %d\n", inode->di_mode);

  /*
   * Declare contents of di_db array. This is a lazy implementation, so I didn't
   * account for directories larger than the fragment size.
   * TODO: this.
   */
  di_db = inode->di_db;
  parse_dir( ptr, sb, ptr + *di_db * sb->fs_fsize, tabs );

  return 0;
}



int main( int argc, char *argv[] ){
  struct fs *super_block;
  struct stat st;
  int size;
  int img;
  void *ret;
  char *ptr = NULL;

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
  
  //create the superblock using UFS2 offset
  super_block = (struct fs *)(ptr + SBLOCK_UFS2);
  printf("\nname mounted on: %s\n", super_block->fs_fsmnt);

  //recurse!
  parse_ino(ptr, super_block, UFS_ROOTINO, 0);

  return 0;
}

