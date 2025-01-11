#ifndef _A5_IMMFS
#define _A5_IMMFS

#include <stdint.h>

typedef struct IMFFS *IMFFSPtr;


typedef enum {
  IMFFS_OK = 0,
  IMFFS_ERROR = 1,
  IMFFS_FATAL = 2,
  IMFFS_INVALID = 3,
  IMFFS_NOT_IMPLEMENTED = 4
} IMFFSResult;

IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs);

IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile);

IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile);

IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile);

IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew);

IMFFSResult imffs_dir(IMFFSPtr fs);

IMFFSResult imffs_fulldir(IMFFSPtr fs);

IMFFSResult imffs_defrag(IMFFSPtr fs);

IMFFSResult imffs_destroy(IMFFSPtr fs);

#endif