/*

* ls-v1.1.0.c
* Feature 2: Long Listing (-l)
* ---
* Adds -l option to show permissions, links, owner, group, size, and modification time.
* Uses system calls: stat(), getpwuid(), getgrgid(), ctime().
  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

void print_permissions(mode_t mode) {
char perms[11];
perms[0] = S_ISDIR(mode) ? 'd' : '-';
perms[1] = (mode & S_IRUSR) ? 'r' : '-';
perms[2] = (mode & S_IWUSR) ? 'w' : '-';
perms[3] = (mode & S_IXUSR) ? 'x' : '-';
perms[4] = (mode & S_IRGRP) ? 'r' : '-';
perms[5] = (mode & S_IWGRP) ? 'w' : '-';
perms[6] = (mode & S_IXGRP) ? 'x' : '-';
perms[7] = (mode & S_IROTH) ? 'r' : '-';
perms[8] = (mode & S_IWOTH) ? 'w' : '-';
perms[9] = (mode & S_IXOTH) ? 'x' : '-';
perms[10] = '\0';
printf("%s", perms);
}

void long_listing(const char *dirname) {
DIR *dir = opendir(dirname);
if (!dir) {
perror("opendir");
return;
}

struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.') continue; // skip hidden files

    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dirname, entry->d_name);

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("stat");
        continue;
    }

    print_permissions(st.st_mode);
    printf(" %ld", (long)st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group *gr = getgrgid(st.st_gid);
    printf(" %s %s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

    printf(" %5ld", (long)st.st_size);

    char *mtime = ctime(&st.st_mtime);
    mtime[strlen(mtime) - 1] = '\0'; // remove newline
    printf(" %s", mtime);

    printf(" %s\n", entry->d_name);
}

closedir(dir);

}

void simple_listing(const char *dirname) {
DIR *dir = opendir(dirname);
if (!dir) {
perror("opendir");
return;
}

struct dirent *entry;
while ((entry = readdir(dir)) != NULL) {
    if (entry->d_name[0] == '.') continue;
    printf("%s  ", entry->d_name);
}
printf("\n");
closedir(dir);


}

int main(int argc, char *argv[]) {
int long_flag = 0;
const char *dirname = ".";

// Parse command line arguments
for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0)
        long_flag = 1;
    else
        dirname = argv[i];
}

if (long_flag)
    long_listing(dirname);
else
    simple_listing(dirname);

return 0;

}
