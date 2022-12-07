#ifndef PARSING_H
#define PARSING_H

#include <stddef.h>
#include <stdlib.h>

struct ReceiveData
{
  unsigned char *response;
  size_t size;
  size_t total_size;
};

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct WriteThis
{
  const char *readptr;
  size_t sizeleft;
};

static size_t write_data_in_file(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t receive_data(void *data, size_t size, size_t nmemb, void *userp);
static size_t send_data_callback(char *dest, size_t size, size_t nmemb, void *userp);


struct MemoryStruct sendBodyToParsing(char* body, size_t lenBody);


#endif