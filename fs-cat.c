// fs-cat.c
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

int parse_dir( char * ptr, char * path,  struct fs * sb, char * head, int tabs );
int parse_ino( char * ptr, char * path, struct fs * sb, int ino, int tabs, int is_file );

char * ptr = NULL;


int parse_dir( char * ptr, char * path, struct fs * sb, char * head, int tabs ) {
  int max_size = DEV_BSIZE; //also DIRBLKSIZE, usually 512 bytes
  int count = 0; //janky way of tracking distance in memory from head
  struct direct *dir = (struct direct *)(head);

  while ( count < max_size ) { //halt when reaching DIRBLKSIZE

    //halts if record length is zero, i.e. no direct struct
    if (dir->d_reclen == 0) {
      return 0;
    }
    //Iterate through directories that match provided filepath until we find a file
    int i = 0;
    while(dir->d_name[i] != '\0' && path[i] != '/'){
       if(path[i] == dir->d_name[i]){
          if(path[i+1] == '/'){
	     //Move over the pointer so that the file path is correct
	     path = path + i + 2;
	     parse_ino(ptr, path, sb, dir->d_ino, tabs + 1, 0);
             break;
          }
	     else if(path[i+1] == '\0'){
         //When we find the file, pass a 1 in as the last parameter
	        parse_ino(ptr, path, sb, dir->d_ino, tabs + 1, 1);
          break;
          }
	     i++;
      }
      else{
	        break;
       }
    }

    //increment count by the length of record and assign next direct struct
    count = count + dir->d_reclen; //record length
    dir = (struct direct *)(head + count);
 }
  return 0;
}

int parse_ino( char * ptr, char * path, struct fs * sb, int ino, int tabs, int is_file) {
  int block_offset;
  int block_add;
  int offset;
  struct ufs2_dinode *inode;
  ufs2_daddr_t *di_db;

  //ino block offset in filesystem in BLOCKS
  block_offset = sizeof(struct ufs2_dinode) * ino_to_fsbo(sb, ino);
  //ino block add in filesystem in FRAGMENTS
  block_add = 4096 * ino_to_fsba(sb, ino);

  //get the offset to the inode "ino" and declare inode struct
  offset = block_add + block_offset;
  inode = (struct ufs2_dinode *)(ptr + offset);

  /*
   * Declare contents of di_db array. This is a lazy implementation, so I didn't
   * account for directories larger than the fragment size.
   * TODO: this.
   */
  di_db = inode->di_db;
  //If it's not a file, keep recursing
  if(is_file == 0){
     parse_dir( ptr, path, sb, ptr + *di_db * sb->fs_fsize, tabs);
  }
  else
  {
     //If it is a file, we iterate through di_db address entries
     int i = 0;
     while(di_db[i]!=0){
      //Nowe we iterate through characters inside of each entry and print them!
      char * content = (char *)(di_db[i]*4096+ptr);
      while(*content != '\0'){
          printf("%c", *content);
      content++;
  	}
  	i++;
       }
    }
  return 0;
}



int main( int argc, char *argv[] ){
  struct fs * super_block;
  struct stat st;
  int size;
  int img;
  void *ret;

  // handle arguments
  if ( argc > 3) {
    printf("Too many arguments supplied.\n");
    exit(1);
  } else if ( argc < 3) {
    printf("Two arguments expected.\n");
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

  //recurse!
  parse_ino(ptr, argv[2], super_block, UFS_ROOTINO, 0, 0);

  return 0;
}
