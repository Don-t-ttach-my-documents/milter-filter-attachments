#ifndef PARSING_H
#define PARSING_H

#include <stddef.h>
#include <stdlib.h>

struct memory
{
  unsigned char *response;
  size_t size;
};

struct WriteThis
{
  const char *readptr;
  size_t sizeleft;
};

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);
static size_t cb(void *data, size_t size, size_t nmemb, void *userp);
static size_t read_callback(char *dest, size_t size, size_t nmemb, void *userp);


struct memory sendBodyToParsing(char* body, size_t lenBody);


#endif