#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a5_multimap.h"

typedef struct VALUE_NODE {
  Value value;
  struct VALUE_NODE *next;
} ValueNode;

typedef struct KEY_AND_VALUES {
  void *key;
  int num_values;
  ValueNode *head;
} KeyAndValues;

struct MULTIMAP {
  int num_keys;
  int max_keys;
  KeyAndValues *keys;
  int trav_pos;

  Compare compare_keys;
  Compare compare_values;
};

// Helper functions
static int find_key_pos(void *key, KeyAndValues *keys, int num_keys, Compare compare_keys);
static int insert_key_ordered(KeyAndValues *keys, int keys_length, void *key, Compare compare_keys);
static int insert_value_ordered(KeyAndValues *key, ValueNode *node, Compare compare_values);

#ifndef NDEBUG
static int validate_multimap(Multimap *mm)
{
  assert(NULL != mm);
  assert(NULL != mm->keys);
  assert(mm->max_keys >= 0);
  assert(mm->num_keys >= 0 && mm->num_keys <= mm->max_keys);
  assert(mm->trav_pos >= -1 && mm->trav_pos <= mm->max_keys);
  assert(NULL != mm->compare_keys);

  ValueNode *curr, *prev;
  int count;
  for (int i = 0; i < mm->num_keys; i++) {
    assert(mm->keys[i].num_values > 0); // can't have a key with no values
    assert(NULL != mm->keys[i].head);

    if (i > 0) {
      // ordering and duplication
      assert(mm->compare_keys(mm->keys[i - 1].key, mm->keys[i].key) < 0);
    }

    count = 0;
    curr = mm->keys[i].head;
    prev = NULL;
    while (NULL != curr) {
      count++;
      // assert(strlen(curr->value.str) < MAX_VALUE_LENGTH);
      if (prev != NULL) {
        // ordering
        // assert(mm->compare_values(&prev->value, &curr->value) <= 0);
      }
      prev = curr;
      curr = curr->next;
    }
    assert(count == mm->keys[i].num_values);
  }

  return 1; // always return TRUE
}
#endif

Multimap *mm_create(int max_keys, Compare compare_keys, Compare compare_values)
{
  assert(max_keys >= 0);
  assert(NULL != compare_keys);

  Multimap *mm = NULL;

  if (max_keys >= 0 && NULL != compare_keys) {
    mm = malloc(sizeof(Multimap));
    if (NULL != mm) {
      mm->keys = malloc(max_keys * sizeof(KeyAndValues));
      if (NULL == mm->keys) {
        free(mm);
        mm = NULL;
      } else {
        mm->max_keys = max_keys;
        mm->num_keys = 0;
        mm->trav_pos = -1;
        mm->compare_keys = compare_keys;
        mm->compare_values = compare_values;
      }
    }
  }

  assert(validate_multimap(mm));
  return mm;
}

int mm_insert_value(Multimap *mm, void *key, int value_num, void *value_data)
{
  assert(validate_multimap(mm));
  assert(NULL != key);
  // assert(NULL != value_str);

  int result = -1;
  int pos;
  ValueNode *node;

  if (NULL != mm && NULL != key && NULL != value_data) {
    pos = find_key_pos(key, mm->keys, mm->num_keys, mm->compare_keys);
    if (pos < 0 && mm->num_keys < mm->max_keys) {

      pos = insert_key_ordered(mm->keys, mm->num_keys, key, mm->compare_keys);
      assert(pos >= 0 && pos < mm->max_keys);
      mm->num_keys++;

      if (pos < mm->trav_pos && mm->trav_pos < mm->max_keys) {
        mm->trav_pos++;
      }
    }

    if (pos >= 0) {
      assert(pos < mm->num_keys);
      // key was either already there, or successfully added

      node = malloc(sizeof(ValueNode));
      if (NULL != node) {
        node->value.num = value_num;

        node->value.data = value_data;
        result = insert_value_ordered(&mm->keys[pos], node, mm->compare_values);
      }

      assert(result > 0);
    }
  }

  assert(validate_multimap(mm));
  return result;
}

int mm_count_keys(Multimap *mm)
{
  assert(validate_multimap(mm));

  int count = -1;

  if (NULL != mm) {
    count = mm->num_keys;
  }

  assert(count >= -1 && count <= mm->max_keys);
  return count;
}

int mm_count_values(Multimap *mm, void *key)
{
  assert(validate_multimap(mm));
  assert(NULL != key);

  int count = -1;
  int pos;

  if (NULL != mm && NULL != key) {
    count = 0;
    pos = find_key_pos(key, mm->keys, mm->num_keys, mm->compare_keys);
    if (pos >= 0) {
      assert(pos < mm->num_keys);
      count = mm->keys[pos].num_values;
    }
  }

  assert(count >= -1);
  return count;
}

int mm_get_values(Multimap *mm, void *key, Value values[], int max_values)
{
  assert(validate_multimap(mm));
  assert(NULL != key);
  assert(NULL != values);
  assert(max_values >= 0);

  int count = -1;
  int pos;
  ValueNode *node;

  if (NULL != mm && NULL != key && NULL != values && max_values >= 0) {
    count = 0;
    pos = find_key_pos(key, mm->keys, mm->num_keys, mm->compare_keys);
    if (pos >= 0) {
      assert(pos < mm->num_keys);
      node = mm->keys[pos].head;
      while (NULL != node && count < max_values) {
        values[count] = node->value;
        count++;
        node = node->next;
      }
    }
  }

  assert(validate_multimap(mm));
  assert(count >= -1);
  return count;
}

int mm_remove_key(Multimap *mm, void *key)
{
  assert(validate_multimap(mm));
  assert(NULL != key);

  int count = -1;
  int pos;
  ValueNode *curr, *next;

  if (NULL != mm && NULL != key) {
    count = 0;
    pos = find_key_pos(key, mm->keys, mm->num_keys, mm->compare_keys);
    if (pos >= 0) {
      assert(pos < mm->num_keys);

      count = 0;

      curr = mm->keys[pos].head;
      while (NULL != curr) {
        next = curr->next;
        free(curr);
        curr = next;
        count++;
      }

      for (int i = pos + 1; i < mm->num_keys; i++) {
        mm->keys[i - 1] = mm->keys[i];
      }
      mm->num_keys--;

      if (pos + 1 <= mm->trav_pos && mm->trav_pos > 0) {
        mm->trav_pos--;
      }
    }
  }

  assert(validate_multimap(mm));
  assert(count >= -1);
  return count;
}

void mm_print(Multimap *mm)
{
  assert(validate_multimap(mm));

  ValueNode *node;

  if (NULL != mm) {
    for (int i = 0; i < mm->num_keys; i++) {
      printf("[%3d] '%s' (%d):\n", i, (char *)mm->keys[i].key, mm->keys[i].num_values);
      node = mm->keys[i].head;
      while (NULL != node) {
        printf(" %9d '%s'\n", node->value.num, (char *)node->value.data);
        node = node->next;
      }
    }
  }

  assert(validate_multimap(mm));
}

int mm_destroy(Multimap *mm)
{
  int count = -1;

  assert(validate_multimap(mm));

  ValueNode *node, *next;

  if (NULL != mm) {
    count = mm->num_keys;
    for (int i = 0; i < mm->num_keys; i++) {
      node = mm->keys[i].head;
      while (NULL != node) {
        next = node->next;
        free(node);
        count++;
        node = next;
      }
    }
    free(mm->keys);

    mm->num_keys = 0;
    mm->max_keys = 0;
    mm->keys = NULL;

    free(mm);
  }

  return count;
}

int mm_get_first_key(Multimap *mm, void **key)
{
  assert(validate_multimap(mm));
  assert(NULL != key);

  int result = 0;

  if (NULL == mm || NULL == key) {
    return -1;
  }

  if (mm->num_keys > 0) {

    *key = mm->keys[0].key;
    result = 1;
    mm->trav_pos = 1;
  } else {
    mm->trav_pos = -1;
  }

  assert(result >= -1 && result <= 1);
  assert(validate_multimap(mm));

  return result;
}

int mm_get_next_key(Multimap *mm, void **key)
{
  assert(validate_multimap(mm));
  assert(NULL != key);

  int result = 0;

  if (NULL == mm || NULL == key) {
    return -1;
  }

  if (mm->trav_pos < mm->num_keys && mm->trav_pos >= 0) {
    *key = mm->keys[mm->trav_pos].key;
    result = 1;
    mm->trav_pos++;
  } else {
    if (mm->trav_pos < 0) {
      result = -1;
    }
    mm->trav_pos = -1;
  }

  assert(result >= -1 && result <= 1);
  assert(validate_multimap(mm));

  return result;
}

static int find_key_pos(void *key, KeyAndValues *keys, int num_keys, Compare compare_keys)
{
  assert(NULL != key);
  assert(NULL != keys);
  assert(num_keys >= 0);
  assert(NULL != compare_keys);

  int start = 0, end = num_keys - 1;
  int mid, comp;
  int pos = -1;

  while (start <= end && pos < 0) {
    mid = (end - start) / 2 + start;
    comp = compare_keys(key, keys[mid].key);
    if (comp < 0) {
      end = mid - 1;
    } else if (comp > 0) {
      start = mid + 1;
    } else {
      pos = mid;
    }
  }

  return pos;
}

static int insert_key_ordered(KeyAndValues *keys, int keys_length, void *key, Compare compare_keys)
{
  assert(NULL != keys);
  assert(keys_length >= 0);
  assert(NULL != key);
  assert(NULL != compare_keys);

  int pos = 0;

  while (pos < keys_length && compare_keys(key, keys[pos].key) >= 0) {
    pos++;
  }

  if (pos < keys_length) {
    memmove(&keys[pos + 1], &keys[pos], (keys_length - pos) * sizeof(KeyAndValues));
  }

  keys[pos].key = key;
  keys[pos].num_values = 0;
  keys[pos].head = NULL;

  return pos;
}

static int insert_value_ordered(KeyAndValues *key, ValueNode *node, Compare compare_values)
{
  assert(NULL != key);
  assert(NULL != node);

  ValueNode *curr = key->head;
  ValueNode *prev = NULL;

  while (NULL != curr && compare_values(&node->value, &curr->value) > 0) {
    prev = curr;
    curr = curr->next;
  }

  if (NULL == prev) {
    node->next = key->head;
    key->head = node;
  } else {
    node->next = curr;
    prev->next = node;
  }

  key->num_values++;

  return key->num_values;
}
