#include <string.h>
#include <stdio.h>
#include <gc.h>
#include "hash.h"


static void finalize_hash(GC_PTR obj, GC_PTR env)
{
  hash_t *result = (hash_t *)obj;
  hdestroy_r(&result->table);
}

hash_t *make_hash(void)
{
  hash_t *result = GC_MALLOC(sizeof(hash_t));
  GC_register_finalizer(result, finalize_hash, 0, 0, 0);
  memset(result, 0, sizeof(hash_t));
  hcreate_r(65536, &result->table);
  result->items = make_list();
  return result;
}

static void *hash_find_str(hash_t *hash, char *key, void *value_if_not_found)
{
  ENTRY item = {key, value_if_not_found};
  ENTRY *result;
  hsearch_r(item, ENTER, &result, &hash->table);
  if (result->data == value_if_not_found)
    append_pointer(hash->items, key);
  return result->data;
}

int hash_find_index(hash_t *hash, int key1, int key2, int key3, int value_if_not_found)
{
  const int keysize = 30;
  char *str = GC_MALLOC_ATOMIC(keysize);
  snprintf(str, keysize, "%d,%d,%d", key1, key2, key3);
  long int result = value_if_not_found;
  result = (long int)hash_find_str(hash, str, (void *)result);
  return (int)result;
}

material_t *hash_find_material(hash_t *hash, const char *key, material_t *material)
{
  char *str = GC_MALLOC_ATOMIC(strlen(key) + 1);
  strcpy(str, key);
  material_t *result = hash_find_str(hash, str, material);
  if (result == material)
    append_pointer(hash->items, material);
  return result;
}
