#include "../swiss_table.h"

#define GROUP_SIZE 16
#define INITIAL_GROUP_COUNT 16
#define MAX_FILL 0.7f

#define DELETED 0xfe
#define EMPTY 0x80

#define METADATA_MASK 0x7e
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
  static const uint16_t noise[] = { 18894, 1149, 9843, 5236, 21354, 26871, 21465, 17861, 2414, 5219, 24185, 5711, 866, 16758, 26915, 27184 };
  uint64_t sum = 0, hash = 0;
  for (uint32_t i = 0; i < strlen(key); ++i) {
    sum += (key[i] * noise[i % 16]);
  }
  for (uint8_t i = 0; i < 8; ++i) {
    hash ^= ((sum << i) | (sum >> (64 - i)));
  }
  return hash;
}

static void
expand(swiss_table_t* tbl_ptr)
{
  tbl_ptr->_group_count *= 2;
  tbl_ptr->_current_size = 0;
  tbl_ptr->_deleted = 0;
  node_t** tmp_groups = tbl_ptr->_groups;
  uint8_t** tmp_control = tbl_ptr->_control;
  tbl_ptr->_groups = (node_t**)malloc(tbl_ptr->_group_count * sizeof(node_t*));
  tbl_ptr->_control = (uint8_t**)malloc(tbl_ptr->_group_count * sizeof(uint8_t*)); 
  for (uint32_t i = 0; i < tbl_ptr->_group_count; ++i) {
    tbl_ptr->_control[i] = (uint8_t*)malloc(GROUP_SIZE * sizeof(uint8_t));
    memset(tbl_ptr->_control[i], EMPTY, GROUP_SIZE);
    tbl_ptr->_groups[i] = (node_t*)calloc(GROUP_SIZE, sizeof(node_t));
  }
  for (uint32_t group_index = 0; group_index < tbl_ptr->_group_count / 2; ++group_index) {
    for (uint8_t node_index = 0; node_index < GROUP_SIZE; ++node_index) {
      if ((int8_t)tmp_control[group_index][node_index] >= 0) {
        swiss_table_insert_update(tbl_ptr, tmp_groups[group_index][node_index]._key, tmp_groups[group_index][node_index]._data);
        free(tmp_groups[group_index][node_index]._key);
        free(tmp_groups[group_index][node_index]._data);
      }
    }
    free(tmp_groups[group_index]);
    free(tmp_control[group_index]);
  }
  free(tmp_groups);
  free(tmp_control);
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
    new_table->_control[i] = (uint8_t*)malloc(GROUP_SIZE * sizeof(uint8_t));
    memset(new_table->_control[i], EMPTY, GROUP_SIZE);
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
  for (uint64_t group_index = ((h & HASH_MASK) >> 7) % tbl_ptr->_group_count;;group_index = (group_index + 1) % tbl_ptr->_group_count) {
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == metadata) {
        if (!strcmp(tbl_ptr->_groups[group_index][metadata_index]._key, key)) {
          free(tbl_ptr->_groups[group_index][metadata_index]._data);
          tbl_ptr->_groups[group_index][metadata_index]._data = strdup(data);
          return UPDATED;
        }
      }
    }
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == EMPTY) {
        tbl_ptr->_control[group_index][metadata_index] = metadata;
        tbl_ptr->_groups[group_index][metadata_index]._key = strdup( key);
        tbl_ptr->_groups[group_index][metadata_index]._data = strdup( data);
        ++tbl_ptr->_current_size;
        return NO_ERR;
      }
    }
  }
}

uint8_t
swiss_table_delete(swiss_table_t* tbl_ptr, const char* key)
{
  if (!tbl_ptr || !key) {
    return INVALID_ARGS;
  }
  uint64_t h = tbl_ptr->hash_f(key);
  uint8_t metadata = h & METADATA_MASK;
  for (uint64_t group_index = ((h & HASH_MASK) >> 7) % tbl_ptr->_group_count;;group_index = (group_index + 1) % tbl_ptr->_group_count) {
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == metadata) {
        if (!strcmp(tbl_ptr->_groups[group_index][metadata_index]._key, key)) {
          free(tbl_ptr->_groups[group_index][metadata_index]._key);
          free(tbl_ptr->_groups[group_index][metadata_index]._data);
          for (uint8_t m = 0; m < GROUP_SIZE; ++m) {
            if (tbl_ptr->_control[group_index][m] == EMPTY) {
              tbl_ptr->_control[group_index][metadata_index] = EMPTY;
              --tbl_ptr->_current_size;
              return NO_ERR;
            }
          }
          tbl_ptr->_control[group_index][metadata_index] = DELETED;
          ++tbl_ptr->_deleted;
          return NO_ERR;
        }
      }
    }
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == EMPTY) {
        return KEY_NOT_FOUND;
      }
    }
  }
}

char*
swiss_table_get_copy(const swiss_table_t* tbl_ptr, const char* key)
{
  if (!tbl_ptr || !key) {
    return NULL;
  }
  uint64_t h = tbl_ptr->hash_f(key);
  uint8_t metadata = h & METADATA_MASK;
  for (uint64_t group_index = ((h & HASH_MASK) >> 7) % tbl_ptr->_group_count;;group_index = (group_index + 1) % tbl_ptr->_group_count) {
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == metadata) {
        if (!strcmp(tbl_ptr->_groups[group_index][metadata_index]._key, key)) {
          return strdup(tbl_ptr->_groups[group_index][metadata_index]._data);
        }
      }
    }
    for (uint8_t metadata_index = 0; metadata_index < GROUP_SIZE; ++metadata_index) {
      if (tbl_ptr->_control[group_index][metadata_index] == EMPTY) {
        return NULL;
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
  for (uint32_t i = 0; i < tbl_ptr->_group_count; ++i) {
    for (uint8_t m = 0; m < GROUP_SIZE; ++m) {
      if ((int8_t)(tbl_ptr->_control[i][m]) >= 0) {
        free(tbl_ptr->_groups[i][m]._key);
        free(tbl_ptr->_groups[i][m]._data);
      }
    }
    free(tbl_ptr->_control[i]);
    free(tbl_ptr->_groups[i]);
  }
  free(tbl_ptr->_control);
  free(tbl_ptr->_groups);
  free(tbl_ptr);
}
