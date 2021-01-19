#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"
#include "csv.c"


#ifndef NOEMS
#include <emscripten.h>
#endif

// Basic types and decls.
typedef   signed char        int8_t;
typedef unsigned char       uint8_t;
typedef          short      int16_t;
typedef unsigned short     uint16_t;
typedef          int        int32_t;
typedef unsigned int       uint32_t;
typedef          long long  int64_t;
typedef unsigned long long uint64_t;

typedef unsigned long size_t;
typedef unsigned char byte;
typedef unsigned int uint;

#define NULL ((void*)0)

#define USER_DATA_COUNTRY 0
#define USER_DATA_COUNTRY_STR "country"

#define USER_DATA_ASN 1
#define USER_DATA_ASN_STR "asn"

#define USER_DATA_COLO 2
#define USER_DATA_COLO_STR "colo"

#define USER_DATA_CITY 3
#define USER_DATA_CITY_STR "city"

#define USER_DATA_CONTINENT 4
#define USER_DATA_CONTINENT_STR "continent"

#define USER_DATA_TIMEZONE 5
#define USER_DATA_TIMEZONE_STR "timezone"

#define USER_DATA_LATITUDE 6
#define USER_DATA_LATITUDE_STR "latitude"

#define USER_DATA_LONGITUDE 7
#define USER_DATA_LONGITUDE_STR "longitude"

#define USER_DATA_IP 8
#define USER_DATA_IP_STR "ip"

#define USER_DATA_UA 9
#define USER_DATA_UA_STR "ua"

#define USER_DATA_COUNT 10

struct params {
  int hdr;
  char * reply;
  size_t cur;
  size_t size;
};
struct params p;

static const char needCsvQuote[] = {
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 0, 1, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
};

void
write_csv(size_t *cur, size_t *size, char ** reply, char *w) {
  size_t l = strlen(w);
  if(*cur + l + 1 >= *size) {
    size_t new_size = (*size) * 2;
    if(new_size > *size+1000000 && *size >= 10000000) {
      new_size = *size+1000000;
    }

    char* treply = realloc(*reply, new_size);
    *reply = treply;
    *size = new_size;
  }

  memcpy(*reply + *cur, w, l+1); // strcat is not working properly

  *cur = *cur + l;
}

int check_quotes(char *col) {
  int l = strlen(col);
  for(int i=0;i<l;i++) {
    if(needCsvQuote[col[i]] || col[i] == ',') {
      return 1;
    }
  }
  return 0;
}

int cb(
  void *pArg,
  int nArg,        /* Number of result columns */
  char **azArg,    /* Text of each result column */
  char **azCol     /* Column names */
) {
  int i;
  
  struct params *p = (struct params*) pArg;

  if( p->hdr ){
    for(i=0; i<nArg; i++){
      int q = check_quotes(azCol[i]);
      if(q) {
        write_csv(&(p->cur), &(p->size), &(p->reply), "\"");
      }
      write_csv(&(p->cur), &(p->size), &(p->reply), sqlite3_mprintf("%w", azCol[i]));
      if(q) {
        write_csv(&(p->cur), &(p->size), &(p->reply), "\"");
      }
      if(i<nArg-1) {
        write_csv(&(p->cur), &(p->size), &(p->reply), ",");
      }
    }
    write_csv(&(p->cur), &(p->size), &(p->reply), "\n");
    p->hdr = 0;
    
  }
  if( nArg>0 ){
    for(i=0; i<nArg; i++){
      int q = check_quotes(azArg[i]);
      if(q) {
        write_csv(&(p->cur), &(p->size), &(p->reply), "\"");
      }
      write_csv(&(p->cur), &(p->size), &(p->reply), sqlite3_mprintf("%w", azArg[i]));
        if(q) {
        write_csv(&(p->cur), &(p->size), &(p->reply), "\"");
      }
      if(i<nArg-1) {
        write_csv(&(p->cur), &(p->size), &(p->reply), ",");
      }
    }
    write_csv(&(p->cur), &(p->size), &(p->reply), "\n");
  }
  return 0;
}

int cb2(void * a, int b, char ** c, char ** d) {
  return 0;
}

char ** userdata;

static void user_data_func(
  sqlite3_context * context,
  int argc,
  sqlite3_value ** argv
) {
  if (argc != 1) {
    sqlite3_result_error(context, "must identify the variable to use", -1);
  }
  char* argStr = sqlite3_value_text(argv[0]);
  int dataId = 0;
  if(strcmp(argStr, USER_DATA_COUNTRY_STR) == 0) {
    dataId = USER_DATA_COUNTRY;
  } else if(strcmp(argStr, USER_DATA_ASN_STR) == 0) {
    dataId = USER_DATA_ASN;
  } else if(strcmp(argStr, USER_DATA_COLO_STR) == 0) {
    dataId = USER_DATA_COLO;
  } else if(strcmp(argStr, USER_DATA_CITY_STR) == 0) {
    dataId = USER_DATA_CITY;
  } else if(strcmp(argStr, USER_DATA_CONTINENT_STR) == 0) {
    dataId = USER_DATA_CONTINENT;
  } else if(strcmp(argStr, USER_DATA_TIMEZONE_STR) == 0) {
    dataId = USER_DATA_TIMEZONE;
  } else if(strcmp(argStr, USER_DATA_LATITUDE_STR) == 0) {
    dataId = USER_DATA_LATITUDE;
  } else if(strcmp(argStr, USER_DATA_LONGITUDE_STR) == 0) {
    dataId = USER_DATA_LONGITUDE;
  } else if(strcmp(argStr, USER_DATA_IP_STR) == 0) {
    dataId = USER_DATA_IP;
  } else if(strcmp(argStr, USER_DATA_UA_STR) == 0) {
    dataId = USER_DATA_UA;
  } else {
    sqlite3_result_error(context, "unknown argument to function getdata", -1);
  }

  char * data = userdata[dataId];
  int c = strlen(data);
  sqlite3_result_blob(context, data, c, SQLITE_TRANSIENT);
}

char * 
#ifndef NOEMS
EMSCRIPTEN_KEEPALIVE 
#endif
//query(int sth)
query(char * query,
  char * country,
  char * asn,
  char * colo,
  char * city,
  char * continent,
  char * timezone,
  char * latitude,
  char * longitude,
  char * ip,
  char * ua
  )
{
  sqlite3* pDb = NULL;
  int ret = sqlite3_open_v2("", &pDb, SQLITE_OPEN_MEMORY | SQLITE_OPEN_READWRITE, NULL);
  if(ret != SQLITE_OK) {
    static char buf[300];
    sprintf(buf, "open database error:%s\n", sqlite3_errmsg(pDb));
    return buf;
  }

  sqlite3_create_module(pDb, "csv", &CsvModule, 0);

  userdata = calloc(USER_DATA_COUNT, sizeof(char*));
  userdata[USER_DATA_COUNTRY] = country;
  userdata[USER_DATA_ASN] = asn;
  userdata[USER_DATA_COLO] = colo;
  userdata[USER_DATA_CITY] = city;
  userdata[USER_DATA_CONTINENT] = continent;
  userdata[USER_DATA_TIMEZONE] = timezone;
  userdata[USER_DATA_LATITUDE] = latitude;
  userdata[USER_DATA_LONGITUDE] = longitude;
  userdata[USER_DATA_IP] = ip;
  userdata[USER_DATA_UA] = ua;

  sqlite3_create_function(pDb, "getdata", 1,
    SQLITE_UTF8 | SQLITE_INNOCUOUS | SQLITE_DETERMINISTIC,
    0, user_data_func, 0, 0);

  size_t size = 100000*sizeof(char);
  
  char * err;
  p.hdr = 1;

  p.reply = malloc(size);
  p.cur = 0;
  p.size = size;

  sqlite3_exec(pDb, query, cb, &p, & err);
  if(err != NULL) {
    return err;
  }

  sqlite3_close(pDb);

  return p.reply;
}
