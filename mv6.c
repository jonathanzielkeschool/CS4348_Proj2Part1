// Group 10: Cassandra Palmer: add_free_block(), report and readme
//           Hernan Gonzales: get_free_block(), report and readme 
//           Jonathan Zielke: base code, initfs implementation
// CS 4348.005
// Project 2, Part 1

#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<errno.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>

#define BLOCK_SIZE 1024
#define INODE_SIZE 64


typedef struct {
 int isize;
 int fsize;
 int nfree;
 unsigned int free[200];
 char flock;
 char ilock;
 char fmod;
 unsigned int time;
} superblock_type; // Block size is 1024 Bytes; not all bytes of superblock
//are used.
superblock_type superBlock;


int add_free_block(int addr) {
  int ix;
  for (ix = 0; ix < superBlock.nfree; ix++) {
    if (superBlock.free[ix] == NULL) {
      superBlock.free[ix] = addr;
      return 1; // Successful add
    }
  }
  return -1; // Unsuccessful add
}

int get_free_block() {
  int ix;
  for(ix = 0; ix < superBlock.nfree; ix++)
  {
    if(superBlock.free[ix] != NULL)
    {
    return superBlock.free[ix];
    }
  }
  return -1;
}

typedef struct {
    unsigned short flags;
    unsigned short nlinks;
    unsigned int uid;
    unsigned int gid;
    unsigned int size0;
    unsigned int size1;
    unsigned int addr[9];
    unsigned int actime;
    unsigned int modtime;
} inode_type;

typedef struct {
    unsigned int inode;
    char filename[28];
} dir_type;  //32 Bytes long

inode_type root;

int fd;

int open_fs(char *file_name){
    fd = open(file_name, O_RDWR | O_CREAT, 0600);

    if(fd == -1){
        return -1;
    }
    else{
        return 1;
    }
}

// Function to write inode
void inode_writer(int inum, inode_type inode){

   lseek(fd,2*BLOCK_SIZE+(inum-1)*INODE_SIZE,SEEK_SET);
    write(fd,&inode,sizeof(inode));
}

// Function to read inodes
inode_type inode_reader(int inum, inode_type inode){
   lseek(fd,2*BLOCK_SIZE+(inum-1)*INODE_SIZE,SEEK_SET);
    read(fd, &inode, sizeof(inode));
    return inode;
}


// Function to write inode number after filling some fileds
void fill_an_inode_and_write(int inum){
inode_type root;
int i;

    root.flags |= 1 << 15; //Root is allocated
    root.flags |= 1 <<14; //It is a directory
    root.actime = time(NULL);
    root.modtime = time(NULL);

    root.size0 = 0;
    root.size1 = 2 * sizeof(dir_type);
    root.addr[0]=get_free_block(); //assuming that blocks 2 to 99 are for i-nodes; 100 is the first data block that can hold root's directory contents
	for (i=1;i<9;i++) root.addr[i]=-1;//all other addr elements are null so setto -1
    inode_writer(inum, root);

}

int commandReader(char* input){ //Reads command for switch statement, will update in later parts to more efficiently pass command args

  if( strcmp(input,"initfs") == 0) return 1;
  else if (strcmp(input,"q\n") ==0) return 2;
  else return 0;
}


void initfs(){

  char* file_name;
  int n1, n2;
  file_name = strtok(NULL, " ");
  n1 = atoi(strtok(NULL, " "));
  n2 = atoi(strtok(NULL, " "));

  //Make sure valid arguments are passed, will have to be updated when there are more commands
  if(file_name && n1 && n2){
    printf("%s %d %d\n", file_name, n1, n2);
  }
  else{
    printf("Invalid Arguments\n");
    return;
  }


  printf("Opening file\n");
  inode_type inode1;
  open_fs(file_name);
  superBlock.fsize = n1;
  superBlock.isize = n2;
  superBlock.nfree = n1 - (n2 + 2); //Total blocks - (inode blocks + block 0 + superBlock)
  int ix;
  for(ix = 2 + superBlock.isize; ix < superBlock.nfree + superBlock.isize + 2; ix++){
    if(add_free_block(ix)); //Add all blocks after inodes to free array
    else break;
  }
  superBlock.time = time(NULL);//update superBlock time
  //Write inode1
  fill_an_inode_and_write(1);
  inode1=inode_reader(1,inode1);
  printf("Value of inode1's addr[0] is %d\n",inode1.addr[0]);
  printf("Value of inode1's addr[1] is %d\n",inode1.addr[1]);
}



// The main function
int main(){

    int loop = 1;
    char* input;
    while (loop){
      printf("$>: ");
      fgets(input, 100, stdin);//Read command

      switch(commandReader(strtok(input, " "))){ //Switch based on first token in command
        case 1://On initfs
          initfs();
          break;
        case 2://On q
          loop = 0;//Breaks loop
          break;
        default:
          printf("Invaild Command\n");
          break;
      }
    }


}
