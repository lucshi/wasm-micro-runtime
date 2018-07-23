#include "bh_platform.h"
#include "bh_memory.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

char*
bh_read_file_to_buffer(const char *filename, int *ret_size)
{
  char *buffer;
  int file;
  int file_size, read_size;
  struct stat stat_buf;

  if (!filename || !ret_size) {
    printf("Read file to buffer failed: invalid filename or ret size.\n");
    return NULL;
  }

  if ((file = open(filename, O_RDONLY, 0)) == -1) {
    printf("Read file to buffer failed: open file %s failed.\n",
           filename);
    return NULL;
  }

  if (fstat(file, &stat_buf) != 0) {
    printf("Read file to buffer failed: fstat file %s failed.\n",
           filename);
    close(file);
    return NULL;
  }

  file_size = stat_buf.st_size;

  if (!(buffer = bh_malloc(file_size))) {
    printf("Read file to buffer failed: alloc memory failed.\n");
    close(file);
    return NULL;
  }

  read_size = read(file, buffer, file_size);
  close(file);

  if (read_size < file_size) {
    printf("Read file to buffer failed: read file content failed.\n");
    bh_free(buffer);
    return NULL;
  }

  *ret_size = file_size;
  return buffer;
}

#define RSIZE_MAX 0x7FFFFFFF

int memcpy_s(
   void * s1,
   unsigned int s1max,
   const void * s2,
   unsigned int n)
{
  char *dest = (char*)s1;
  char *src = (char*)s2;
  if (n == 0) {
    return 0;
  }

  if (s1 == NULL || s1max > RSIZE_MAX) {
    return -1;
  }
  if (s2 == NULL || n > s1max) {
    memset(dest, 0, s1max);
    return -1;
  }
  memcpy(dest, src, n);
  return 0;
}

int strcpy_s(char * s1, size_t s1max, const char * s2)
{
  if (NULL == s1 || NULL == s2 || s1max < (strlen(s2) + 1) || s1max > RSIZE_MAX) {
    return -1;
  }

  strcpy(s1, s2);

  return 0;
}

int fopen_s(FILE ** pFile, const char *filename, const char *mode)
{
  if (NULL == pFile || NULL == filename || NULL == mode) {
    return -1;
  }

  *pFile = fopen(filename, mode);

  if (NULL == *pFile)
    return -1;

  return 0;
}

