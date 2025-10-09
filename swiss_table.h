#pragma once

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct swiss_table_node node_t;
typedef struct swiss_table swiss_table_t;

enum errors
{
  NO_ERR = 0,
  UPDATED,
  KEY_NOT_FOUND,
  INVALID_ARGS
};

swiss_table_t* swiss_table_init(void);

void swiss_table_set_hash(swiss_table_t* tbl_ptr, uint64_t (*hash)(const char*));

uint8_t swiss_table_insert_update(swiss_table_t* tbl_ptr, const char* key, const char* data);

uint8_t swiss_table_delete(swiss_table_t* tbl_ptr, const char* key);

char* swiss_table_get_copy(const swiss_table_t* tbl_ptr, const char* key);

void swiss_table_destroy(swiss_table_t* tbl_ptr);
