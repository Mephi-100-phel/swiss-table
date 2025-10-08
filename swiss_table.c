#include "swiss_table.h"

#define GROUP_SIZE 16
#define INITIAL_GROUP_COUNT 16
#define MAX_FILL 0.7f

#define DELETED 0x80
#define EMPTY 0x0

#define METADATA_MASK 0x7f
#define HASH_MASK 0xffffffffffffff80

struct swiss_table_node
{
  char* _key;
  char* _data;
};

struct swiss_table 
{
  uint8_t** _control;
  node_t** _groups;
  uint32_t _group_count;
  uint32_t _current_size;
  uint32_t _deleted;
  uint64_t (*hash_f)(const char*);
};

static uint64_t
hash(const char* key)
{
  
}

static void
expand(swiss_table_t* tbl_ptr)
{
  return;
}

swiss_table_t*
swiss_table_init(void)
{
  swiss_table_t* new_table = (swiss_table_t*)calloc(1, sizeof(swiss_table_t));
  new_table->_group_count = INITIAL_GROUP_COUNT;
  new_table->hash_f = &hash;
  new_table->_control = (uint8_t**)malloc(INITIAL_GROUP_COUNT * sizeof(uint8_t*));
  new_table->_groups = (node_t**)malloc(INITIAL_GROUP_COUNT * sizeof(node_t*));
  for (uint8_t i = 0; i < INITIAL_GROUP_COUNT; ++i) {
    new_table->_control[i] = (uint8_t*)calloc(GROUP_SIZE, sizeof(uint8_t));
    new_table->_groups[i] = (node_t*)calloc(GROUP_SIZE, sizeof(node_t));
  }
  return new_table;
}

void
swiss_table_set_hash(swiss_table_t* tbl_ptr, uint64_t (*hash_f)(const char*))
{
  if (!tbl_ptr || !hash_f) {
    return;
  }
  tbl_ptr->hash_f = hash_f;
}

uint8_t
swiss_table_insert_update(swiss_table_t* tbl_ptr, const char* key, const char* data)
{
  if (!tbl_ptr || !key || !data) {
    return INVALID_ARGS;
  }
  if (tbl_ptr->_current_size > tbl_ptr->_group_count * GROUP_SIZE * MAX_FILL) {
    expand(tbl_ptr);
  }
  uint64_t h = tbl_ptr->hash_f(key);
  uint8_t metadata = h & METADATA_MASK;
  for (uint64_t group_index = (h & HASH_MASK) >> 7;;group_index = (group_index + 1) % tbl_ptr->_group_count) {
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == metadata) {
        if (!strcmp(tbl_ptr->_groups[group_index][metadata_index]._key, key)) {
          free(tbl_ptr->_groups[group_index][metadata_index]._data);
          strcpy(tbl_ptr->_groups[group_index][metadata_index]._data, data);
          return NO_ERR;
        }
      }
    }
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == EMPTY) {
        tbl_ptr->_control[group_index][metadata_index] = metadata;
        strcpy(tbl_ptr->_groups[group_index][metadata_index]._key, key);
        strcpy(tbl_ptr->_groups[group_index][metadata_index]._data, data);
        ++tbl_ptr->_current_size;
        return NO_ERR;
      }
    }
  }
}

void
swiss_table_destroy(swiss_table_t* tbl_ptr)
{
  if (!tbl_ptr) {
    return;
  }
  for (uint8_t i = 0; i < tbl_ptr->_group_count; ++i) {
    free(tbl_ptr->_control[i]);
    free(tbl_ptr->_groups[i]);
  }
  free(tbl_ptr->_control);
  free(tbl_ptr->_groups);
  free(tbl_ptr);
}
