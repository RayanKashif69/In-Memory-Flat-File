#ifndef _A5_MULTIMAP
#define _A5_MULTIMAP

typedef struct VALUE { int num; void *data; } Value;
typedef struct MULTIMAP Multimap; // you need to define this yourself

typedef int (*Compare)(void *a, void *b);

Multimap *mm_create(int max_keys, Compare compare_keys, Compare compare_values);

int mm_insert_value(Multimap *mm, void *key, int value_num, void *value_data);

int mm_count_keys(Multimap *mm);

int mm_count_values(Multimap *mm, void *key);

int mm_get_values(Multimap *mm, void *key, Value values[], int max_values);

int mm_remove_key(Multimap *mm, void *key);

void mm_print(Multimap *mm);

int mm_destroy(Multimap *mm);

int mm_get_first_key(Multimap *mm, void **key);

int mm_get_next_key(Multimap *mm, void **key);

#endif
