// fs-find.c
// created by Tom Gause
// 5-8-2022

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Represents a node of an n-ary tree
struct Node
{
  int32_t d_ino;            // inode number of entry
  int8_t d_type;            // file type. Mostly 4=Dir and 8=File
  int8_t d_namlen;          // length of string in d_name
  char d_name[256];         // Node name. Rather than dynamic allocation,
                            // I'll just allocate a set size.

  /* This is a clever way of creating an n-ary tree without lots of external
  arrays. It's similar to a left-right tree.
  Source: https://blog.mozilla.org/nnethercote/2012/03/07/n-ary-trees-in-c/ */
  struct Node *child;
  struct Node *neighbor;
};

int find_bytes( unsigned char sequence[], FILE *img ) {
  // read img in chunks of 512 bytes
  unsigned char buffer[512];
  int32_t bytesRead = 0;
  int32_t add = 0;

  while ((bytesRead = fread(buffer, 1, sizeof(buffer), img)) > 0)
  {
    // testing function
    /* if (buffer[0] + buffer[1] + buffer[2] + buffer[3] != 0x0) {
      printf("%x\t%x\t%x\t%x\t:%x \n", 
              buffer[0], buffer[1], buffer[2], buffer[3], add);
      printf("%x\t%x\t%x\t%x\n\n",
            sequence[0], sequence[1], sequence[2], sequence[3]);
    } */
    if (buffer[0] == sequence[0] && buffer[1] == sequence[1] &&
        buffer[2] == sequence[2] && buffer[3] == sequence[3]) {
      return add;
    }
    add = add + 512;
  }
  printf("Address not found.\n");
  return -1;
}
  
// Utility function to create a new tree node
struct Node* newNode(int32_t location, FILE *img)
{
  // Allocate memory for new node
  struct Node* temp = (struct Node*)malloc(sizeof(struct Node));

  // Assign data to this node
  unsigned char data[] = fread(location, 1, 8, img);

  int32_t id = data[4] * 0x100 + data[5];

  // If inode number is 0, we create a NULL node to indicate
  // that this is the last neighbor in the row.
  if (id == 0) {
    temp->d_ino = NULL;
    temp->d_type = NULL;
    temp->d_namlen = NULL;
    strcpy(temp->d_name, "NULL");
    temp->child = NULL;
    struct->neighbor = NULL;
    exit(1);
  }

  // If not, we fill out the struct.
  temp->d_ino = id;
  temp->d_type = data[6];
  temp->d_namlen = data[7];
  unsigned char name[] = fread(location + 8, 1, data[7], img);
  strcpy(temp->d_name, name);

  // Initialize child and neighbor as NULL
  if (data[6] == 8) {
    temp->child = NULL; // File, doesn't have a child
  } else {
    // Directory, has a child that we identify by inode id
    int32_t add = find_bytes(id, img);
    if (add == -1) {
      printf("Exiting program.\n");
      exit(1);
    } else {
      temp->child = newNode(add, img);
    }
  }
  
  temp->neighbor = newNode(location + 0x8 * (int)ceil(1+data[7]/8), img);
  return temp;
}

int main( int argc, char *argv[] ){

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
	FILE *img=fopen(argv[1],"rb");
  if(img == NULL) {
    printf("Error opening file.\n");
    exit(1);
  }

  /* For now, we'll find the root node with a simple search. From our reverse engineering, the d_ino number for the root appears to be 0x0200. The size
  and type are also consistent--0x0c00 size, 04 type (directory)--so we want
  a sector that starts with the hex string 0x02000000. */
  unsigned char sequence[4]={0x02, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x04, 0x01};
  int32_t location = find_bytes(sequence, img);
  printf("Root inode at %x.\n", location);
  Node root = newNode(location, img);

  return 0;
}





