#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "a5_multimap.h"


typedef struct {
    void *key;
    Value *values;
    int num_values;
    int max_values;
} KeyValuePair;

typedef struct MULTIMAP {
    int max_keys;
    Compare compare_keys_as_strings_case_insensitive;
    Compare compare_values_num_part;
    KeyValuePair *pairs;
    int num_pairs;
} Multimap;


int mm_count_keys(Multimap *mm) {
    return mm->num_pairs;
}

Multimap *mm_create(int max_keys, Compare compare_keys_as_strings_case_insensitive, Compare compare_values_num_part) {
    Multimap *mm = (Multimap *)malloc(sizeof(Multimap));
    if (mm == NULL) {
        return NULL;
    }

    mm->max_keys = max_keys;
    mm->compare_keys_as_strings_case_insensitive = compare_keys_as_strings_case_insensitive;
    mm->compare_values_num_part = compare_values_num_part;
    mm->pairs = NULL;
    mm->num_pairs = 0;

    return mm;
}

int find_key_index(Multimap *mm, void *key) {
    for (size_t i = 0; i < mm->num_pairs; ++i) {
        if (mm->compare_keys_as_strings_case_insensitive(mm->pairs[i].key, key) == 0) {
            return i;
        }
    }
    return -1;
}

int mm_insert_value(Multimap *mm, void *key, int value_num, void *value_data) {
    if (mm == NULL || key == NULL) {
        return 0;
    }

    // Find the correct position to insert the key based on key ordering
    size_t insert_position = 0;
    while (insert_position < mm->num_pairs &&
           mm->compare_keys_as_strings_case_insensitive(mm->pairs[insert_position].key, key) < 0) {
        ++insert_position;
    }

    // Check if the key already exists
    int key_index = find_key_index(mm, key);

    if (key_index == -1) {
        // Key not found, create a new key-value pair
        if (mm->num_pairs == mm->max_keys) {
            // Maximum number of keys reached
            return -1;  // Return -1 to indicate failure
        }

        mm->pairs = realloc(mm->pairs, (mm->num_pairs + 1) * sizeof(KeyValuePair));
        if (mm->pairs == NULL) {
            // Memory allocation failure
            return 0;
        }

        // Shift keys to make room for the new key
        for (size_t i = mm->num_pairs; i > insert_position; --i) {
            mm->pairs[i] = mm->pairs[i - 1];
        }

        mm->pairs[insert_position].key = key;
        mm->pairs[insert_position].values = NULL;
        mm->pairs[insert_position].num_values = 0;
        mm->pairs[insert_position].max_values = 0;

        key_index = insert_position;
        ++mm->num_pairs;
    }

    // Find the correct position to insert the value based on value_num
    size_t value_insert_position = 0;
    while (value_insert_position < mm->pairs[key_index].num_values &&
           value_num > mm->pairs[key_index].values[value_insert_position].num) {
        ++value_insert_position;
    }

    // Insert the value into the array list for the key at the determined position
    size_t num_values = mm->pairs[key_index].num_values;
    size_t max_values = mm->pairs[key_index].max_values;

    if (num_values == max_values) {
        // Need to resize the array list
        max_values = (max_values == 0) ? 1 : max_values * 2;
        mm->pairs[key_index].values = realloc(mm->pairs[key_index].values, max_values * sizeof(Value));
        if (mm->pairs[key_index].values == NULL) {
            // Memory allocation failure
            return 0;
        }
        mm->pairs[key_index].max_values = max_values;
    }

    // Shift values to make room for the new value
    for (size_t i = num_values; i > value_insert_position; --i) {
        mm->pairs[key_index].values[i] = mm->pairs[key_index].values[i - 1];
    }

    // Store the value_num and value_data in the Value structure at the determined position
    mm->pairs[key_index].values[value_insert_position].num = value_num;
    mm->pairs[key_index].values[value_insert_position].data = value_data;

    ++mm->pairs[key_index].num_values;

    return mm->pairs[key_index].num_values;
}

int mm_count_values(Multimap *mm, void *key) {
    int key_index = find_key_index(mm, key);
    return (key_index != -1) ? mm->pairs[key_index].num_values : 0;
}

int mm_get_values(Multimap *mm, void *key, Value values[], int max_values) {
    int key_index = find_key_index(mm, key);

    if (key_index != -1) {
        size_t num_values = mm->pairs[key_index].num_values;
        size_t copy_count = (num_values < max_values) ? num_values : max_values;

        for (size_t i = 0; i < copy_count; ++i) {
            values[i] = mm->pairs[key_index].values[i];
        }

        return copy_count;
    }

    return 0;
}

int mm_remove_key(Multimap *mm, void *key) {
    int key_index = find_key_index(mm, key);

    if (key_index != -1) {
        free(mm->pairs[key_index].values);

        // Remove the key-value pair from the array
        for (size_t i = key_index; i < mm->num_pairs - 1; ++i) {
            mm->pairs[i] = mm->pairs[i + 1];
        }

        --mm->num_pairs;

        mm->pairs = realloc(mm->pairs, mm->num_pairs * sizeof(KeyValuePair));

        return mm->pairs[key_index].num_values;
    }

    return 0;
}

void mm_print(Multimap *mm) {
    for (size_t i = 0; i < mm->num_pairs; ++i) {
        printf("Key: %p\n", mm->pairs[i].key);
        for (size_t j = 0; j < mm->pairs[i].num_values; ++j) {
            Value current_value = mm->pairs[i].values[j];
            printf("  Value %zu: num=%d, data=%p\n", j + 1, current_value.num, current_value.data);
        }
    }
}

int mm_destroy(Multimap *mm) {
    if (mm == NULL) {
        return 0;
    }

    size_t num_pairs = mm->num_pairs;

    for (size_t i = 0; i < num_pairs; ++i) {
        free(mm->pairs[i].values);
    }

    free(mm->pairs);
    free(mm);

    return num_pairs * 2;
}

int mm_get_first_key(Multimap *mm, void **key) {
    if (mm == NULL || key == NULL || mm->num_pairs == 0) {
        return 0;
    }

    *key = mm->pairs[0].key;

    return 1;
}

int mm_get_next_key(Multimap *mm, void **key) {
    if (mm == NULL || key == NULL || mm->num_pairs == 0) {
        return 0;
    }

    int key_index = find_key_index(mm, *key);

    if (key_index != -1 && key_index < mm->num_pairs - 1) {
        *key = mm->pairs[key_index + 1].key;
        return 1;
    }

    return 0;
}

// Function to compare two keys (strings)
int compare_keys_as_strings_case_insensitive(void *a, void *b){
    return strcasecmp((const char *)a, (const char *)b);
}

// Function to compare two values
int compare_values_num_part(void *a, void *b){
    Value *va = (Value *)a, *vb = (Value *)b;
    return va->num - vb->num;
}


