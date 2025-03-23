#include <dirent.h>
#define ERROR -1
#define SUCCESS 0
#define ERROR_MESSAGE_LENGHT FILENAME_MAX + 256

int reverseDirectory(const char *srcDir, const char *dstDir);
void getFullPath(const char *dir, const char *name, char *result);
int openSrcFile(const char *srcPath);
int openDstFile(const char *dstPath, mode_t mode);
char* readFileContents(int fd, size_t size, const char *srcPath);
void reverseBuffer(char *buf, size_t size);
int processRegularFile(const char *srcPath, const char *dstPath, struct stat st) ;
int getReversedPath(const char *oldPath, char *newPath);
int processEntry(const char *srcDir, const char *dstDir, const struct dirent *ent);
int reverseDirectory(const char *srcDir, const char *dstDir);