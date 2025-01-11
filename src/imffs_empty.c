#include "a5_imffs.h"

IMFFSResult imffs_create(uint32_t block_count, IMFFSPtr *fs) {
  *fs = 1;
  return IMFFS_OK;
}

IMFFSResult imffs_save(IMFFSPtr fs, char *diskfile, char *imffsfile) {
  return IMFFS_OK;
}

IMFFSResult imffs_load(IMFFSPtr fs, char *imffsfile, char *diskfile) {
  return IMFFS_OK;
}

IMFFSResult imffs_delete(IMFFSPtr fs, char *imffsfile) {
  return IMFFS_OK;
}

IMFFSResult imffs_rename(IMFFSPtr fs, char *imffsold, char *imffsnew) {
  return IMFFS_OK;
}

IMFFSResult imffs_dir(IMFFSPtr fs) {
  return IMFFS_OK;
}

IMFFSResult imffs_fulldir(IMFFSPtr fs) {
  return IMFFS_OK;
}

IMFFSResult imffs_defrag(IMFFSPtr fs) {
  return IMFFS_NOT_IMPLEMENTED;
}

IMFFSResult imffs_destroy(IMFFSPtr fs) {
  return IMFFS_OK;
}
