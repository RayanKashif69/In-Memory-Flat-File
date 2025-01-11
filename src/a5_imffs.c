#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>

#include "a4_boolean.h"
#include "a5_multimap.h"
#include "a5_imffs.h"

const int BYTES_PER_BLOCK = 256;
const uint8_t BLOCK_FREE = ' ';
const uint8_t BLOCK_USED = 'X';
#define TEMP_FILE ".temp"

struct IMFFS {
  uint8_t *data;
  uint8_t *used; // one byte per free space marker
  uint32_t block_count;
  Multimap *index;
};

typedef struct {
  char *name;
  uint32_t byte_len;
} File;

static int compare_files_by_name(void *a, void *b) {
  assert(NULL != a && NULL != b);
  File *fa = a, *fb = b;
  assert(NULL != fa->name && NULL != fb->name);

  return strcasecmp(fa->name, fb->name);

}

static int compare_always_greater(void *a, void *b) {
  assert(NULL != a && NULL != b);
  return 1;
}

static Boolean validate_fs(IMFFSPtr fs) {
  return TRUE;
}

static Boolean find_next_free_block(uint8_t *used, uint32_t block_count, uint32_t *pos) {

  assert(NULL != pos && *pos < block_count);

  while (*pos < block_count) {
    if (BLOCK_FREE == used[*pos]) {
      return TRUE;
    }
    (*pos)++;
  }
  return FALSE;

}

static File *find_matching_file(Multimap *index, char *name) {

  assert(NULL != index && NULL != name);

  File *file = NULL;
  void *key;

  if (NULL != index && NULL != name && mm_get_first_key(index, &key) > 0) {
    do {
      file = key;
      if (strcasecmp(name, file->name) != 0) {
        file = NULL;
      }
    } while (NULL == file && mm_get_next_key(index, &key) > 0);
  }

  return file;

}

static uint32_t block_ptr_to_index(void *base, void *block) {
  
  assert(NULL != base && NULL != block);
  assert(block >= base);

  uint8_t *base8 = base;
  uint8_t *block8 = block;

  uint32_t pos = block8 - base8;
  assert(pos % BYTES_PER_BLOCK == 0);
  pos = pos / BYTES_PER_BLOCK;

  return pos;
}

static void restore_free_space(IMFFSPtr fs, Value *values, int num_values) {
  assert(validate_fs(fs));
  assert(NULL != values && num_values > 0);

  for (int i = 0; i < num_values; i++) {
    uint32_t pos = block_ptr_to_index(fs->data, values[i].data);

    for (int j = 0; j < values[i].num; j++) {
      assert(pos + j < fs->block_count);
      assert(BLOCK_USED == fs->used[pos + j]);
      fs->used[pos + j] = BLOCK_FREE;
    }
  }
}

static int make_values_array(Multimap *index, File *file, Value **values, int old_value_size) {
  assert(NULL != file && NULL != values);

  int new_value_size = -1, count;
  Value *temp;

  count = mm_count_values(index, file);
  assert(count > 0);
  if (count > 0) {
    new_value_size = count;
    if (count > old_value_size || NULL == *values) {
      temp = malloc(count * sizeof(Value));
      if (NULL == temp) {
        new_value_size = -1;
      } else {
        free(*values);
        *values = temp;
      }
    }
  }

  return new_value_size;
}

IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs) {
  assert(NULL != fs);

  if (NULL == fs) {
    return IMFFS_INVALID;
  } else {

    *fs = malloc(sizeof(struct IMFFS));
    if (NULL == *fs) {
      fprintf(stderr, "Error: not enough memory to create filesystem.\n");
      return IMFFS_FATAL;
    } else {

      (*fs)->data = malloc(block_count * BYTES_PER_BLOCK);

      (*fs)->used = malloc(block_count + 1);
      for (uint32_t i = 0; i < block_count; i++) {
        (*fs)->used[i] = BLOCK_FREE;
      }
      (*fs)->used[block_count] = '\0';

      (*fs)->block_count = block_count;
      (*fs)->index = mm_create(block_count, compare_files_by_name, compare_always_greater);

      if (NULL == (*fs)->data || NULL == (*fs)->used || NULL == (*fs)->index) {
        fprintf(stderr, "Error: not enough memory to create filesystem data.\n");
        free((*fs)->data);
        free((*fs)->used);
        free((*fs)->index);
        free(*fs);
        return IMFFS_FATAL;
      }
    }
  }

  return IMFFS_OK;
}


/*


IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile) {
    // Implementation to load a file from IMFFS to the system
    // You need to read the content of 'imffsfile' in IMFFS and save it to 'diskfile'
    // Use the Multimap and block_usage to retrieve the file content

    // Find the file info in the files_and_chunks Multimap
    int key_index = find_key_index(fs->files_and_chunks, imffsfile);
    if (key_index == -1) {
        fprintf(stderr, "File not found: %s\n", imffsfile);
        return IMFFS_ERROR;  // File not found
    }

    // Get the file info
    FileInfo *info;
    mm_get_values(fs->files_and_chunks, imffsfile, (Value *)&info, 1);

    FILE *file = fopen(diskfile, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return IMFFS_ERROR;
    }

    for (int i = 0; i < info->size; ++i) {
        Chunk *chunk;
        mm_get_values(fs->files_and_chunks, imffsfile, (Value *)&chunk, 1);

        fwrite(chunk->data, 1, BLOCK_SIZE, file);
    }

    fclose(file);

    return IMFFS_OK;
}

IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile) {
    // Implementation to delete a file from IMFFS
    // You need to free the blocks occupied by 'imffsfile' and update the Multimap and block_usage accordingly

    // Find the file info in the files_and_chunks Multimap
    int key_index = find_key_index(fs->files_and_chunks, imffsfile);
    if (key_index == -1) {
        fprintf(stderr, "File not found: %s\n", imffsfile);
        return IMFFS_ERROR;  // File not found
    }

    // Get the file info
    FileInfo *info;
    mm_get_values(fs->files_and_chunks, imffsfile, (Value *)&info, 1);

    // Iterate through chunks and free blocks
    for (int i = 0; i < info->size; ++i) {
        Chunk *chunk;
        mm_get_values(fs->files_and_chunks, imffsfile, (Value *)&chunk, 1);

        int block_index = i;
        fs->block_usage[block_index] = 0;

        // Free the block data
        free(chunk->data);

        // Free the chunk structure
        free(chunk);
    }

    // Remove the file from the Multimap
    mm_remove_key(fs->files_and_chunks, imffsfile);

    return IMFFS_OK;
}

IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew) {
    // Implementation to rename a file in IMFFS
    // You need to update the Multimap to reflect the new filename

    // Find the file info in the files_and_chunks Multimap
    int key_index = find_key_index(fs->files_and_chunks, imffsold);
    if (key_index == -1) {
        fprintf(stderr, "File not found: %s\n", imffsold);
        return IMFFS_ERROR;  // File not found
    }

    // Get the file info
    FileInfo *info;
    mm_get_values(fs->files_and_chunks, imffsold, (Value *)&info, 1);

    // Update the filename in the Multimap
    mm_remove_key(fs->files_and_chunks, imffsold);

    // Create new FileInfo with the new filename
    FileInfo *new_info = create_file_info(imffsnew, info->size);

    // Insert the new file info into the Multimap
    mm_insert_value(fs->files_and_chunks, imffsnew, 1, new_info);

    return IMFFS_OK;
}

IMFFSResult imffs_dir(IMFFSPtr fs) {
    printf("Files in IMFFS:\n");

    // Check if files_and_chunks Multimap is NULL
    if (fs->files_and_chunks == NULL) {
        printf("Error: Files_and_chunks Multimap is NULL\n");
        return IMFFS_ERROR;
    }

    // Iterate through files_and_chunks Multimap
    void *key;
    int key_count = mm_get_first_key(fs->files_and_chunks, &key);

    printf("Got first key with count: %d\n", key_count);

    while (key_count > 0) {
        printf("Inside the loop\n");

        // Get the values associated with the current key
        FileInfo **info_array;
        int values_count = mm_get_values(fs->files_and_chunks, key, (Value *)(&info_array), key_count);

        // Check if the values were retrieved successfully
        if (values_count > 0 && info_array != NULL) {
            // Iterate through the values
            for (int i = 0; i < values_count; ++i) {
                FileInfo *info = info_array[i];

                if (info != NULL) {
                    printf("Filename: %s, Size: %u bytes\n", info->filename, info->size);
                    // Free the allocated FileInfo structure
                    free(info);
                } else {
                    printf("Error: Unable to retrieve info or info is NULL\n");
                    // Handle this case as needed
                }
            }

            // Free the allocated array of FileInfo pointers
            free(info_array);
        } else {
            printf("Error: Unable to retrieve values or info_array is NULL\n");
            // Handle this case as needed
        }

        // Get the next key
        key_count = mm_get_next_key(fs->files_and_chunks, &key);

        printf("Got next key with count: %d\n", key_count);
    }

    printf("Exited the loop\n");

    // Additional debug print
    printf("After the loop, before returning\n");

    return IMFFS_OK;
}

*/
IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile) {
  assert(validate_fs(fs));
  assert(NULL != diskfile);
  assert(NULL != imffsfile);

  FILE *in;
  IMFFSResult result = IMFFS_OK;
  uint32_t cluster_start, prev_free_block, next_free_block, blocks_in_cluster;
  uint8_t *block_data;
  Boolean eof, first;
  File *file = NULL;

  if (NULL == fs || NULL == diskfile || NULL == imffsfile) {
    return IMFFS_INVALID;
  }

  in = fopen(diskfile, "r");
  if (NULL == in) {
    fprintf(stderr, "Error: unable to open external file '%s'.\n", diskfile);
    result = IMFFS_ERROR;
  } else {

    file = malloc(sizeof(File));
    if (NULL != file) {
      file->byte_len = 0;
      file->name = malloc(strlen(imffsfile) + 1);
      if (NULL == file->name) {
        free(file);
        file = NULL;
      } else {
        strcpy(file->name, imffsfile);
      }
    }
    if (NULL == file) {
      fprintf(stderr, "Error: not enough memory to create file '%s'.\n", imffsfile);
      result = IMFFS_ERROR;
    } else {

      if (mm_count_values(fs->index, file) > 0) {
        fprintf(stderr, "Error: file '%s' already exists on device.\n", imffsfile);
        result = IMFFS_ERROR;
      } else {

        eof = FALSE;
        first = TRUE;
        cluster_start = 0;
        prev_free_block = 0;
        next_free_block = 0;
        blocks_in_cluster = 0;

        while (!eof && IMFFS_OK == result && find_next_free_block(fs->used, fs->block_count, &next_free_block)) {
          block_data = &fs->data[next_free_block * BYTES_PER_BLOCK];
          file->byte_len += fread(block_data, 1, BYTES_PER_BLOCK, in);
          if (ferror(in)) {
            fprintf(stderr, "Error reading from input file '%s'.\n", diskfile);
            result = IMFFS_ERROR;
          } else {
            if (first) {
              first = FALSE;
              cluster_start = next_free_block;
            } else if (next_free_block > prev_free_block + 1) {
              if (mm_insert_value(fs->index, file, blocks_in_cluster, &fs->data[cluster_start * BYTES_PER_BLOCK]) <= 0) {
                fprintf(stderr, "Error writing to file '%s'.\n", imffsfile);
                result = IMFFS_ERROR;
              }
              blocks_in_cluster = 0;
              cluster_start = next_free_block;
            }
            prev_free_block = next_free_block;
            fs->used[next_free_block] = BLOCK_USED;
            blocks_in_cluster++;

            eof = feof(in);
          }
        }

        if (IMFFS_OK == result && !eof) {
          fprintf(stderr, "Error: not enough free space on device to save '%s'.\n", imffsfile);
          result = IMFFS_ERROR;
        }

        if (blocks_in_cluster > 0) {
          if (mm_insert_value(fs->index, file, blocks_in_cluster, &fs->data[cluster_start * BYTES_PER_BLOCK]) <= 0) {
            fprintf(stderr, "Error writing to file '%s'.\n", imffsfile);
            result = IMFFS_ERROR;
          }
        }

        if (IMFFS_ERROR == result) {
          if (mm_count_values(fs->index, file) > 0) {
            result = imffs_delete(fs, imffsfile);
            if (IMFFS_OK != result) {
              result = IMFFS_ERROR;
            }
          }
        }
      }
      if (IMFFS_OK != result && NULL != file) {
        if (NULL != file->name) {
          free(file->name);
        }
        free(file);
      }
    }

    fclose(in);
  }

  return result;
}

IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile) {
  assert(validate_fs(fs));
  assert(NULL != diskfile);
  assert(NULL != imffsfile);

  IMFFSResult result = IMFFS_OK;
  FILE *out;
  File temp_file = { imffsfile, 0 }, *file;
  Value  *values;
  int num_values;
  uint32_t length, length_remaining;

  if (NULL == fs || NULL == diskfile || NULL == imffsfile) {
    return IMFFS_INVALID;
  }

  num_values = mm_count_values(fs->index, &temp_file);
  if (num_values <= 0) {
    fprintf(stderr, "Error: no such file '%s'.\n", imffsfile);
    result = IMFFS_ERROR;

  } else if (NULL == (file = find_matching_file(fs->index, imffsfile))) {
    assert(NULL != file);
    fprintf(stderr, "Error: no such file '%s'.\n", imffsfile);
    result = IMFFS_ERROR;
    
  } else if (NULL == (out = fopen(diskfile, "w"))) {
    fprintf(stderr, "Error: unable to open external file '%s'.\n", diskfile);
    result = IMFFS_ERROR;

  } else {
    values = malloc(sizeof(Value) * num_values);

    if (NULL == values || num_values != mm_get_values(fs->index, file, values, num_values)) {
      fprintf(stderr, "Error: unable read from file '%s'.\n", imffsfile);
      result = IMFFS_ERROR;
    } else {
      
      length_remaining = file->byte_len;

      for (int i = 0; i < num_values && IMFFS_OK == result; i++) {
        assert(values[i].num > 0);
        assert(values[i].data != NULL);
        
        // one cluster at a time
        length = values[i].num * BYTES_PER_BLOCK;
        if (length_remaining < length) {
          length = length_remaining;
        }
        fwrite(values[i].data, length, 1, out);
        length_remaining -= length;
        if (ferror(out)) {
          fprintf(stderr, "Error writing to file '%s'.\n", diskfile);
          result = IMFFS_ERROR;
        }
      }
      
      assert(IMFFS_OK != result || 0 == length_remaining);
    }

    free(values);
    fclose(out);
  }


  return result;
}

IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile) {
  assert(validate_fs(fs));
  assert(NULL != imffsfile);
  
  IMFFSResult result = IMFFS_OK;
  int num_values;
  Value *values = NULL;
  
  File temp_file = { imffsfile, 0 };
  File *file;

  if (NULL == fs || NULL == imffsfile) {
    return IMFFS_INVALID;
  }
  
  num_values = mm_count_values(fs->index, &temp_file);

  if (num_values <= 0) {
    fprintf(stderr, "Error: file not found '%s'.\n", imffsfile);
    result = IMFFS_ERROR;

  } else if (NULL == (file = find_matching_file(fs->index, imffsfile))) {
    assert(NULL != file);
    fprintf(stderr, "Error: no such file '%s'.\n", imffsfile);
    result = IMFFS_ERROR;

  } else {

    values = malloc(sizeof(Value) * num_values);

    if (NULL == values || num_values != mm_get_values(fs->index, file, values, num_values)) {
      result = IMFFS_ERROR;
    } else {
      
      restore_free_space(fs, values, num_values);
      
      if (num_values != mm_remove_key(fs->index, file)) {
        result = IMFFS_ERROR;
      } else {
        free(file->name);
        free(file);
      }
    }

    if (IMFFS_ERROR == result) {
      fprintf(stderr, "Error: unable to delete '%s'.\n", imffsfile);
    }

  }
  free(values);

  return result;
}

IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew) {
  assert(validate_fs(fs));
  assert(NULL != imffsold);
  assert(NULL != imffsnew);

  IMFFSResult result = IMFFS_OK;

  if (NULL == fs || NULL == imffsold || NULL == imffsnew) {
    return IMFFS_INVALID;
  }

  File new_file = { imffsnew, 0 };
  File *file = find_matching_file(fs->index, imffsold);

  if (NULL == file) {
    fprintf(stderr, "Error: file '%s' doesn't exist.\n", imffsold);
    result = IMFFS_ERROR;

  } else if (0 != mm_count_values(fs->index, &new_file)) {
    fprintf(stderr, "Error: file '%s' already exists.\n", imffsnew);
    result = IMFFS_ERROR;

  } else {

    
    Value *values = NULL;
    char *temp_name;
    int count = make_values_array(fs->index, file, &values, -1);
    if (count < 0) {
      result = IMFFS_ERROR;
    } else {

      if (mm_get_values(fs->index, file, values, count) != count || 
          mm_remove_key(fs->index, file) != count) {
        result = IMFFS_ERROR;    
      } else {
        
        temp_name = malloc(strlen(imffsnew) + 1);
        if (NULL == temp_name) {
          result = IMFFS_OK;
        } else {
          
          strcpy(temp_name, imffsnew);
          free(file->name);
          file->name = temp_name;
          
          for (int i = 0; i < count; i++) {
            if (mm_insert_value(fs->index, file, values[i].num, values[i].data) <= 0) {
              result = IMFFS_ERROR;
            }
          }
        }
      }
      
      free(values);
    }
    
    if (IMFFS_OK != result) {
      fprintf(stderr, "Error: unable to rename '%s' to '%s'.\n", imffsold, imffsnew);
    }
  }

  return result;
}

static uint32_t count_and_maybe_print_blocks(Multimap *index, File *file, Boolean print) {
  assert(NULL != index && NULL != file);
  
  int num_values;
  Value *values;
  uint32_t blocks = 0;
  
  num_values = mm_count_values(index, file);
  if (num_values > 0) {
    values = malloc(sizeof(Value) * num_values);
    if (num_values == mm_get_values(index, file, values, num_values)) {
      for (int i = 0; i < num_values; i++) {
        assert(values[i].num > 0);
        blocks += values[i].num;
        if (print) {
          printf("          | %6u | %6d | %p\n", values[i].num, i, values[i].data);
        }
      }
    }
    free(values);
  }

  return blocks;
}

static IMFFSResult imffs_dir_both(IMFFSPtr fs, Boolean full) {
  assert(validate_fs(fs));

  void *key;
  File *file;
  uint32_t total_bytes = 0, blocks;
  int chunks;
  
  if (NULL == fs) {
    return IMFFS_INVALID;
  }

  printf("----------+--------+--------+------------\n");

  printf("    Bytes | Blocks | Chunks | Filename\n");

  printf("----------+--------+--------+------------\n");

  if (mm_get_first_key(fs->index, &key) > 0) {
    do {

      file = key;

      if (full) {
        blocks = count_and_maybe_print_blocks(fs->index, file, TRUE);
      } else {
        blocks = count_and_maybe_print_blocks(fs->index, file, FALSE);
      }

      chunks = mm_count_values(fs->index, file);
      printf("%9u | %6u | %6d | %s\n", file->byte_len, blocks, chunks, file->name);
      total_bytes += file->byte_len;
      
      if (full) {
        printf("----------+--------+--------+------------\n");
      }
      
    } while (mm_get_next_key(fs->index, &key) > 0);

    if (!full) {
      printf("----------+--------+--------+------------\n");
    }
  }

  // printf("%s\n", fs->used);
  printf("\nTotal bytes: %u\n", total_bytes);
  
  return IMFFS_OK;
}

IMFFSResult imffs_dir(IMFFSPtr fs) {
  return imffs_dir_both(fs, FALSE);
}

IMFFSResult imffs_fulldir(IMFFSPtr fs) {
  return imffs_dir_both(fs, TRUE);
}

IMFFSResult imffs_defrag(IMFFSPtr fs) {

  assert(validate_fs(fs));
  
  
  IMFFSResult result = IMFFS_OK;
  File **owners = NULL;
  uint32_t owner_count;
  Value *values = NULL;
  int count = 0, value_size = -1;
  File *file = NULL;
  void *key;
  uint8_t pos, buffer[BYTES_PER_BLOCK];
  
  owners = calloc(fs->block_count, sizeof(File *));
  if (NULL == owners) {
    fprintf(stderr, "Code 1 ");
    result = IMFFS_ERROR;
  } else {


    owner_count = 0;
    if (mm_get_first_key(fs->index, &key) > 0) {
      do {
        file = key;
        
        count = make_values_array(fs->index, file, &values, value_size);
        if (count < 0) {
          fprintf(stderr, "Code 2 ");
          result = IMFFS_ERROR;
        } else {
          value_size = count;
        }
        
        if (result == IMFFS_OK) {
          if (count != mm_get_values(fs->index, file, values, value_size)) {

            assert(FALSE);
            fprintf(stderr, "Code 3 ");
            result = IMFFS_ERROR;
          } else {
            
            for (int i = 0; i < count; i++) {
              pos = block_ptr_to_index(fs->data, values[i].data);
              for (int j = 0; j < values[i].num; j++) {
                owners[pos + j] = file;
              }
            }
          }
        }
        
        owner_count++;
      } while (result == IMFFS_OK && mm_get_next_key(fs->index, &key) > 0);
    }
    free(values);

    
    if (result == IMFFS_OK) {
      Multimap *index = mm_create(fs->block_count, compare_files_by_name, compare_always_greater);

      if (NULL == index) {
        fprintf(stderr, "Code 4 ");
        result = IMFFS_ERROR;
      } else {
        
        uint32_t curr_file_block = 0;
        count = 0;
        while (count < owner_count) {
          File *curr_file = NULL;
          
          for (uint32_t pos = curr_file_block; pos < fs->block_count && NULL == curr_file; pos++) {
            if (NULL != owners[pos]) {
              curr_file = owners[pos];
            }
          }
          
          uint32_t curr_blocks = (curr_file->byte_len) / BYTES_PER_BLOCK + 1;
          uint8_t *from_ptr, *to_ptr;
          for (uint32_t pos = curr_file_block; pos < fs->block_count && curr_blocks > 0; pos++) {
            if (owners[pos] == curr_file) {

              // compact
              to_ptr = fs->data + curr_file_block * BYTES_PER_BLOCK;
              from_ptr = fs->data + pos * BYTES_PER_BLOCK;
              if (owners[curr_file_block] == NULL) {
                
                memcpy(to_ptr, from_ptr, BYTES_PER_BLOCK);
                owners[curr_file_block] = curr_file;
                owners[pos] = NULL;
              } else if (owners[curr_file_block] != curr_file) {

                memcpy(buffer, from_ptr, BYTES_PER_BLOCK);
              
                memmove(to_ptr + BYTES_PER_BLOCK, to_ptr, from_ptr - to_ptr);
                memmove(&owners[curr_file_block + 1], &owners[curr_file_block], (pos - curr_file_block) * sizeof(File *));
                // printf("moving %u from %d to %d\n", pos - curr_file_block, pos + 1, pos);
                
                // restore from temp buffer
                memcpy(to_ptr, buffer, BYTES_PER_BLOCK);

                // now this block is owned by this file
                owners[curr_file_block] = curr_file;
              }
              
              
              curr_file_block++;
              curr_blocks--;
            }
          }
          
          count++;
        }
        
        uint32_t files_left = owner_count + 1;
        uint32_t blocks_left = 0;
        File *curr_file = NULL;
        for (uint32_t pos = 0; pos < fs->block_count; pos++) {
          if (0 == files_left) {
            assert(NULL == owners[pos]);
          } else if (0 == blocks_left) {
            files_left--;
            if (files_left > 0) {
              curr_file = owners[pos];
              blocks_left = (curr_file->byte_len) / BYTES_PER_BLOCK;
            }
          } else {
            assert(curr_file == owners[pos]);
            assert(blocks_left > 0);
            blocks_left--;
          }
        }
        
        files_left = owner_count;
        curr_file = owner_count > 0 ? owners[0] : NULL;
        uint32_t curr_start = 0;
        uint32_t chunk_size = 0;
        for (uint32_t pos = 0; pos <= fs->block_count && IMFFS_OK == result; pos++) {
          
          if (NULL == curr_file) {
            if (pos < fs->block_count) {
              assert(NULL == owners[pos]);
              fs->used[pos] = BLOCK_FREE;
            }
          } else if (pos == fs->block_count || owners[pos] != curr_file) {
            if (mm_insert_value(index, curr_file, chunk_size, &fs->data[curr_start * BYTES_PER_BLOCK]) <= 0) {
              fprintf(stderr, "Code 5 ");
              result = IMFFS_ERROR;
            } else if (pos < fs->block_count) {
              curr_file = owners[pos];
              curr_start = pos;
              chunk_size = 0;
            }
          }
          chunk_size++;
        }
        
        if (IMFFS_OK == result) {
          mm_destroy(fs->index);
          fs->index = index;
        } else {
          mm_destroy(index);
        }
      }
    }
  }
  
  if (IMFFS_OK != result) {
    fprintf(stderr, "Error: unable to defragment file system.\n");
  }
  
  free(owners);

  return result;
}

IMFFSResult imffs_destroy(IMFFSPtr fs) {
  
  IMFFSResult result = IMFFS_OK;
  File *file = NULL;
  void *key;
  while (IMFFS_OK == result && mm_get_first_key(fs->index, &key) > 0) {
    file = key;
    if (mm_remove_key(fs->index, file) <= 0) {
      assert(FALSE);
      result = IMFFS_ERROR;
    }

    free(file->name);
    free(file);

  }
  
  free(fs->data);
  free(fs->used);
  mm_destroy(fs->index);
  
  free(fs);

  return result;
}
