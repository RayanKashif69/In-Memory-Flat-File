<<<<<<< HEAD
# In-Memory-Flat-File
=======
IMFFS (In-Memory Flash File System)

Overview

IMFFS is a simple implementation of a flash file system designed to work in-memory. It provides basic functionalities for managing files stored in blocks of memory, including creating, loading, saving, deleting, renaming files, directory listing, defragmentation, and destroying the file system.

File Structure

a4_boolean.h: Header file containing Boolean data type definition.
a5_multimap.h: Header file containing a simple implementation of a multimap data structure.
a5_imffs.h: Header file containing declarations for IMFFS functions and data structures.
a5_imffs.c: Source file containing the implementation of IMFFS functions.
Compilation

To compile the code, run the following command:

c
Copy code
gcc -o imffs a5_imffs.c -std=c99
Usage

Include Header File: Include the "a5_imffs.h" header file in your source code.
Create IMFFS Instance: Create an instance of the IMFFS using the imffs_create function.
Perform File Operations: Use functions like imffs_save, imffs_load, imffs_delete, imffs_rename, imffs_dir, imffs_fulldir, imffs_defrag to perform file operations.
Destroy IMFFS: Destroy the IMFFS instance when it's no longer needed using the imffs_destroy function.


Functionality

Creating a File System: imffs_create function initializes a new IMFFS instance with a specified number of blocks.
Loading a File: imffs_load function loads a file from the IMFFS to the system.
Saving a File: imffs_save function saves a file from the system to the IMFFS.
Deleting a File: imffs_delete function deletes a file from the IMFFS.
Renaming a File: imffs_rename function renames a file in the IMFFS.
Listing Directory Contents: imffs_dir and imffs_fulldir functions list the files in the IMFFS directory.
Defragmenting File System: imffs_defrag function defragments the file system.
Destroying File System: imffs_destroy function destroys the IMFFS instance.



Important Notes

This implementation assumes a fixed block size of 256 bytes.
Ensure proper error handling and memory management in your application when using IMFFS functions.
Refer to the function documentation in the header file for detailed usage instructions and parameters.
>>>>>>> 33115ea (Adding makefile and updated imffs project source files)
