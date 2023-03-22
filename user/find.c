
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

void 
find(char *file, char* path) {
  int fd;
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open current %s\n", path);
    return;
  }

  char buf[512], *p;
  struct stat st;
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
  
  switch(st.type){
  case T_DEVICE:
  case T_FILE:
    break;
  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    struct dirent de;
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if (strcmp(file, p) == 0) {
          printf("%s\n", buf);
      }
      if (strcmp(".", p) == 0 || strcmp("..", p) == 0)
          continue;
      find(file, buf);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  // find all files in a directory with a given name
  if(argc <= 2){
    fprintf(2, "usage: find [directory] [file]\n");
    exit(1);
  }
  find(argv[2],argv[1]);
  exit(0);
}
