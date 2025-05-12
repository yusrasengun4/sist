#ifndef FS_H
#define FS_H


#define METADATA_SIZE 4096
#define RECORD_SIZE 264
#define MAX_FILENAME_LEN 256

extern int disk_fd;

int fs_init();
int check_disk();
int check_filename();
void fs_close();
int fs_create(char *filename);
int fs_write(char *filename, char *data);
int fs_delete(char *filename);
int fs_read(char *filename);
int fs_ls();
int fs_format();
int fs_rename(const char* oldname,const  char* newname);
int fs_exists(const char* filename);
int fs_size(char*filename);
int fs_append(char *filename, char *data,int size);
int fs_truncate(char *filename,int newsize);
int fs_copy(char*source,char *dest);
int fs_mv(char *old_path,char*new_path);
int fs_defragment();
int fs_check_integrity();
int fs_backup(const char *backup_filename);
int fs_cat(const char* filename);
int fs_restore(const char *backup_path);
int fs_diff(const char* file1,const char* file2);
void fs_log(const char *operation, const char *filename, const char *result);
int resolve_path(char *full_path);
int fs_mkdir(char *path)
#endif
