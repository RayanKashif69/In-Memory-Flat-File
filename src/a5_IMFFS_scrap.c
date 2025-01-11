#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "a5_multimap.h"
#include "a5_imffs.h"

#define BLOCK_SIZE 256

// Structure to represent a file in IMFFS
typedef struct {
    char *filename;
    uint32_t size;
} FileInfo;

// Structure to represent a chunk in IMFFS
typedef struct {
    int num_blocks;
    void *data;
} Chunk;

// Structure to represent IMFFS
struct IMFFS {
    Multimap *files_and_chunks;  // Multimap to store file metadata and chunks
    int block_count;             // Number of blocks in IMFFS
    char *device;                // Memory device
    int *block_usage;            // Array to track block usage
};

// Helper function to allocate memory for FileInfo
FileInfo *create_file_info(char *filename, uint32_t size) {
    FileInfo *info = (FileInfo *)malloc(sizeof(FileInfo));
    if (info == NULL) {
        return NULL;  // Memory allocation failure
    }

    info->filename = strdup(filename);
    if (info->filename == NULL) {
        free(info);
        return NULL;  // Memory allocation failure
    }

    info->size = size;

    return info;
}

// Helper function to allocate memory for Chunk
Chunk *create_chunk(int num_blocks, void *data) {
    Chunk *chunk = (Chunk *)malloc(sizeof(Chunk));
    if (chunk == NULL) {
        return NULL;  // Memory allocation failure
    }

    chunk->num_blocks = num_blocks;
    chunk->data = data;

    return chunk;
}

// Constructor function
IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs) {
    // Allocate memory for IMFFS
    *fs = (IMFFSPtr)malloc(sizeof(struct IMFFS));

    if (*fs == NULL) {
        perror("Error: Failed to allocate memory for IMFFS");
        return IMFFS_FATAL;  // Memory allocation failure
    }

    // Initialize IMFFS components
    (*fs)->block_count = block_count;

    // Allocate memory for device
    (*fs)->device = (char *)malloc(block_count * BLOCK_SIZE);
    if ((*fs)->device == NULL) {
        perror("Error: Failed to allocate memory for device");
        free(*fs);
        return IMFFS_FATAL;  // Memory allocation failure
    }

    // Allocate memory for block_usage
    (*fs)->block_usage = (int *)calloc(block_count, sizeof(int));
    if ((*fs)->block_usage == NULL) {
        perror("Error: Failed to allocate memory for block_usage");
        free((*fs)->device);
        free(*fs);
        return IMFFS_FATAL;  // Memory allocation failure
    }

    // Create Multimap for files and chunks
    (*fs)->files_and_chunks = mm_create(block_count, compare_keys_as_strings_case_insensitive, compare_values_num_part);

    if ((*fs)->files_and_chunks == NULL) {
        perror("Error: Failed to create Multimap for files and chunks");
        free((*fs)->device);
        free((*fs)->block_usage);
        free(*fs);
        return IMFFS_FATAL;  // Memory allocation failure
    }

    return IMFFS_OK;
}


IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile) {

    FILE *file = fopen(diskfile, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return IMFFS_ERROR;
    }

    fseek(file, 0, SEEK_END);
    uint32_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Calculate the number of blocks needed
    int num_blocks = (file_size / BLOCK_SIZE) + ((file_size % BLOCK_SIZE == 0) ? 0 : 1);

    printf("File Size: %u bytes\n", file_size);
    printf("Number of Blocks: %d\n", num_blocks);

    // Find free blocks
    int start_block = -1;
    for (int i = 0; i < fs->block_count; ++i) {
        if (fs->block_usage[i] == 0) {
            start_block = i;
            break;
        }
    }

    if (start_block == -1) {
        fclose(file);
        fprintf(stderr, "No free blocks available\n");
        return IMFFS_ERROR;  // No free blocks available
    }

    // Read file content into memory
    void *file_data = malloc(file_size);
    if (file_data == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation failure\n");
        return IMFFS_FATAL;  // Memory allocation failure
    }

    fread(file_data, 1, file_size, file);
    fclose(file);

       // Save the file data to IMFFS
    int remaining_size = file_size;
    for (int i = 0; i < num_blocks; ++i) {
        int block_index = start_block + i;
        void *block_data = (char *)file_data + (i * BLOCK_SIZE);
        int block_size = (remaining_size > BLOCK_SIZE) ? BLOCK_SIZE : remaining_size;

        fs->block_usage[block_index] = 1;

        // Insert into files_and_chunks Multimap
        Chunk *chunk = create_chunk(1, malloc(BLOCK_SIZE));  // Allocate new memory
        memcpy(chunk->data, block_data, block_size);
        //mm_insert_value(fs->files_and_chunks, imffsfile, 1, chunk);

        remaining_size -= block_size;
        if (remaining_size <= 0) {
            break;  // All data has been written
        }
    }


    // Insert file metadata into files_and_chunks Multimap
    FileInfo *info = create_file_info(imffsfile, file_size);
    mm_insert_value(fs->files_and_chunks, imffsfile, 1, info);

    free(file_data);

    return IMFFS_OK;
}

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

IMFFSResult imffs_fulldir(IMFFSPtr fs) {
    // Implementation to list all files, details about chunks, and their sizes in IMFFS
    // You need to iterate through both Multimaps and print filenames, sizes, and chunk details

    printf("Detailed files in IMFFS:\n");

    // Iterate through files_and_chunks Multimap
    void *key;
    int key_count = mm_get_first_key(fs->files_and_chunks, &key);

    while (key_count > 0) {
        FileInfo *info;
        mm_get_values(fs->files_and_chunks, key, (Value *)&info, 1);

        printf("Filename: %s, Size: %u bytes\n", info->filename, info->size);

        // Iterate through chunks Multimap
        for (int i = 0; i < info->size; ++i) {
            Chunk *chunk;
            mm_get_values(fs->files_and_chunks, &i, (Value *)&chunk, 1);

            printf("  Chunk %d: %d blocks\n", i + 1, chunk->num_blocks);
        }

        // Get the next key
        key_count = mm_get_next_key(fs->files_and_chunks, &key);
    }

    return IMFFS_OK;
}

IMFFSResult imffs_destroy(IMFFSPtr fs) {
    // Free all allocated memory
    mm_destroy(fs->files_and_chunks);
    free(fs->device);
    free(fs->block_usage);
    free(fs);

    return IMFFS_OK;
}

IMFFSResult imffs_defrag(IMFFSPtr fs) {
    return IMFFS_OK;

}