// Prof's Testing file for IMFFS -- not my own tests

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "a4_tests.h"
#include "a5_multimap.h"



#include "a5_imffs.c"

void test_multimap() {
  Multimap *mm;
  Value arr[4];
  void *key;
  File files[] = { { "ddd", 10 }, { "aaa", 20 }, { "bbb", 30 }, { "ccc", 40 }, { "eee", 50 } };
  File file_two = { "bbb", 30 };
  
  char zero[] = "zero";
  char *values[] = { zero, "one", "two", "three", "four" };

  printf("\n*** Testing the multimap using the IMFFS structs and functions:\n\n");
  
  
  VERIFY_NOT_NULL(mm = mm_create(4, compare_files_by_name, compare_always_greater));
  VERIFY_INT(1, mm_insert_value(mm, &files[0], 1, values[0]));
  VERIFY_INT(2, mm_insert_value(mm, &files[0], 2, values[1]));
  VERIFY_INT(1, mm_insert_value(mm, &files[1], 3, values[2]));
  VERIFY_INT(1, mm_insert_value(mm, &files[2], 4, values[3]));
  VERIFY_INT(1, mm_insert_value(mm, &files[3], 6, values[4]));
  VERIFY_INT(2, mm_insert_value(mm, &files[3], 5, values[0]));
  VERIFY_INT(3, mm_insert_value(mm, &files[3], 4, values[1]));
  
  // keys:
  
  VERIFY_INT(1, mm_get_first_key(mm, &key));
  VERIFY_STR("aaa", ((File *)key)->name);
  VERIFY_INT(20, ((File *)key)->byte_len);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("bbb", ((File *)key)->name);
  VERIFY_INT(30, ((File *)key)->byte_len);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("ccc", ((File *)key)->name);
  VERIFY_INT(40, ((File *)key)->byte_len);
  VERIFY_INT(1, mm_get_next_key(mm, &key));
  VERIFY_STR("ddd", ((File *)key)->name);
  VERIFY_INT(10, ((File *)key)->byte_len);
  VERIFY_INT(0, mm_get_next_key(mm, &key)); // should not change key
  VERIFY_STR("ddd", ((File *)key)->name);
  VERIFY_INT(10, ((File *)key)->byte_len);
  
  // values:

  VERIFY_INT(1, mm_get_values(mm, &files[1], arr, 4)); // first
  VERIFY_INT(3, arr[0].num);
  VERIFY_STR("two", arr[0].data);

  VERIFY_INT(2, mm_get_values(mm, &files[0], arr, 4)); // last
  VERIFY_INT(1, arr[0].num);
  VERIFY_STR("zero", arr[0].data);
  VERIFY_INT(2, arr[1].num);
  VERIFY_STR("one", arr[1].data);

  VERIFY_INT(3, mm_get_values(mm, &files[3], arr, 4)); // second-last
  VERIFY_INT(6, arr[0].num);
  VERIFY_STR("four", arr[0].data);
  VERIFY_INT(5, arr[1].num);
  VERIFY_STR("zero", arr[1].data);
  VERIFY_INT(4, arr[2].num);
  VERIFY_STR("one", arr[2].data);

  zero[0] = 'h';

  VERIFY_INT(2, mm_get_values(mm, &files[0], arr, 4)); // last
  VERIFY_INT(1, arr[0].num);
  VERIFY_STR("hero", arr[0].data);
  VERIFY_INT(2, arr[1].num);
  VERIFY_STR("one", arr[1].data);
  VERIFY_INT(3, mm_get_values(mm, &files[3], arr, 4)); // second-last
  VERIFY_INT(6, arr[0].num);
  VERIFY_STR("four", arr[0].data);
  VERIFY_INT(5, arr[1].num);
  VERIFY_STR("hero", arr[1].data);
  VERIFY_INT(4, arr[2].num);
  VERIFY_STR("one", arr[2].data);

  // removal:
  
  VERIFY_INT(1, mm_get_values(mm, &files[2], arr, 4)); // second
  VERIFY_INT(4, arr[0].num);
  VERIFY_STR("three", arr[0].data);
  VERIFY_INT(1, mm_remove_key(mm, &file_two));
  VERIFY_INT(0, mm_get_values(mm, &files[2], arr, 4));
 
  // insert again:

  VERIFY_INT(1, mm_insert_value(mm, &files[4], 1, "again"));
 
  // after removal and insertion:

  printf("\n*** Testing find_matching_file:\n\n");

  // these must be exact (pointer) matches
  VERIFY_INT(1, &files[1] == find_matching_file(mm, "aaa"));
  VERIFY_NULL(find_matching_file(mm, "bbb"));
  VERIFY_INT(1, &files[3] == find_matching_file(mm, "ccc"));
  VERIFY_INT(1, &files[0] == find_matching_file(mm, "ddd"));
  VERIFY_INT(1, &files[4] == find_matching_file(mm, "eee"));
  VERIFY_NULL(find_matching_file(mm, ""));

#ifdef NDEBUG
  VERIFY_NULL(find_matching_file(mm, NULL));
  VERIFY_NULL(find_matching_file(NULL, "bbb"));
#endif

  VERIFY_INT(11, mm_destroy(mm));
}

void test_find_next_free_block() {
  uint32_t pos;
  
  printf("\n*** Testing find_next_free_block:\n\n");

  pos = 0;
  VERIFY_INT(TRUE, find_next_free_block((uint8_t *)"XXX  X X", 8, &pos));
  VERIFY_INT(3, pos);
  pos = 4;
  VERIFY_INT(TRUE, find_next_free_block((uint8_t *)"XXX  X X", 8, &pos));
  VERIFY_INT(4, pos);
  pos = 5;
  VERIFY_INT(TRUE, find_next_free_block((uint8_t *)"XXX  X X", 8, &pos));
  VERIFY_INT(6, pos);
  pos = 7;
  VERIFY_INT(FALSE, find_next_free_block((uint8_t *)"XXX  X X", 8, &pos));

  pos = 0;
  VERIFY_INT(FALSE, find_next_free_block((uint8_t *)"XXX  X X", 3, &pos));

  pos = 0;
  VERIFY_INT(FALSE, find_next_free_block((uint8_t *)"XXXXXXXXX", 9, &pos));

  pos = 0;
  VERIFY_INT(TRUE, find_next_free_block((uint8_t *)" XXXXXXXXX", 10, &pos));
  VERIFY_INT(0, pos);
}

void test_block_ptr_to_index() {
  uint8_t base[1000];

  printf("\n*** Testing block_ptr_to_index:\n\n");

  VERIFY_INT(0, block_ptr_to_index(base, &base[0]));
  VERIFY_INT(1, block_ptr_to_index(base, &base[256]));
  VERIFY_INT(2, block_ptr_to_index(base, &base[512]));
  VERIFY_INT(1, block_ptr_to_index(&base[256], &base[512]));
  VERIFY_INT(0, block_ptr_to_index(&base[512], &base[512]));
  VERIFY_INT(3, block_ptr_to_index(base, &base[768]));
  VERIFY_INT(2, block_ptr_to_index(&base[256], &base[768]));
}

int main() {
  printf("*** Starting tests...\n");
  
  test_multimap();
  test_find_next_free_block();
  test_block_ptr_to_index();
  
  if (0 == Tests_Failed) {
    printf("\nAll %d tests passed.\n", Tests_Passed);
  } else {
    printf("\nFAILED %d of %d tests.\n", Tests_Failed, Tests_Failed+Tests_Passed);
  }
  
  printf("\n*** Tests complete.\n");  
  return 0;
}