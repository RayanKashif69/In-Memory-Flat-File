# In-Memory Flash File System (IMFFS)

**IMFFS** (In-Memory Flash File System) is a lightweight in-memory file system designed to manage files as blocks in memory. It offers essential file management capabilities such as creating, loading, saving, deleting, renaming files, directory listing, defragmentation, and destroying the file system.

## Project Overview

IMFFS simulates the functionality of a flash-based file system, with all operations occurring in-memory. This project serves as an educational example of file system architecture and memory management.

### Features:
- **File Operations**: Create, load, save, delete, and rename files.
- **Directory Management**: List files and directories, and perform defragmentation.
- **Memory Management**: Efficient block-based management for file storage.
  
## File Structure

- **a4_boolean.h**: Defines a Boolean data type used in various functions.
- **a5_multimap.h**: Implements a simple multimap data structure used for managing file metadata.
- **a5_imffs.h**: Header file containing function declarations and IMFFS data structures.
- **a5_imffs.c**: Implements core IMFFS functionality, including file system operations.

## Compilation and Running the Code

### Using Makefile

1. **To compile the project**: 
   Run the following command to build the project using the provided Makefile.

   ```bash
   make


## Usage
Include Header Files: Include the a5_imffs.h header file in your application.
Create an IMFFS Instance: Call imffs_create to initialize the IMFFS file system with a set number of memory blocks.
Perform File Operations: Use functions like imffs_save, imffs_load, imffs_delete, imffs_rename, imffs_dir, and imffs_fulldir to manage files.
Defragment File System: Use imffs_defrag to reorganize the memory blocks.
Destroy IMFFS: Clean up by calling imffs_destroy when the file system is no longer needed.

## Functionality Overview
Creating a File System: Use imffs_create to initialize a new IMFFS instance with a given number of blocks.
Loading a File: imffs_load loads a file from the in-memory file system to the user system.
Saving a File: imffs_save saves a file from the system into the IMFFS.
Deleting a File: imffs_delete removes a file from the IMFFS.
Renaming a File: imffs_rename allows renaming files within the IMFFS.
Directory Listing: imffs_dir and imffs_fulldir list the files present in the system.
Defragmenting: imffs_defrag re-organizes and compacts memory blocks to improve performance.
Destroying the File System: imffs_destroy cleans up and frees all resources used by the file system.

## Important Notes
Block Size: This implementation assumes a fixed block size of 256 bytes.
Error Handling: Proper error handling is essential for stability and proper memory management.
Documentation: Refer to the header files for detailed function descriptions, parameters, and usage examples.
Contributing
Feel free to fork the repository, contribute improvements, or suggest new features. Please open an issue for bugs or enhancement suggestions.



