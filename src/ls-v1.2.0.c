/*

* ls-v1.2.0.c
* Feature 3: Column Display (Down Then Across)
* ---
* Default: multiple-column display adapting to terminal width.
* -l option: detailed long listing.
  */

#define _DEFAULT_SOURCE
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>

/* ---------- Helper functions ---------- */

void print_permissions(mode_t mode) {
char p[11];
p[0] = S_ISDIR(mode) ? 'd' : '-';
p[1] = (mode & S_IRUSR) ? 'r' : '-';
p[2] = (mode & S_IWUSR) ? 'w' : '-';
p[3] = (mode & S_IXUSR) ? 'x' : '-';
p[4] = (mode & S_IRGRP) ? 'r' : '-';
p[5] = (mode & S_IWGRP) ? 'w' : '-';
p[6] = (mode & S_IXGRP) ? 'x' : '-';
p[7] = (mode & S_IROTH) ? 'r' : '-';
p[8] = (mode & S_IWOTH) ? 'w' : '-';
p[9] = (mode & S_IXOTH) ? 'x' : '-';
p[10] = '\0';
printf("%s", p);
}

/* Long listing implementation */
void long_listing(const char *dirname) {
DIR *dir = opendir(dirname);
if (!dir) { perror("opendir"); return; }

struct dirent *e;
while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", dirname, e->d_name);
    struct stat st;
    if (stat(path, &st) == -1) { perror("stat"); continue; }

    print_permissions(st.st_mode);
    printf(" %ld", (long)st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    printf(" %s %s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

    printf(" %5ld", (long)st.st_size);

    char *mtime = ctime(&st.st_mtime);
    mtime[strlen(mtime) - 1] = '\0';
    printf(" %s %s\n", mtime, e->d_name);
}
closedir(dir);

}

/* Get terminal width using ioctl */
int get_terminal_width(void) {
struct winsize w;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
return w.ws_col;
return 80;
}

/* Column display: down then across */
void column_display(const char *dirname) {
DIR *dir = opendir(dirname);
if (!dir) { perror("opendir"); return; }

char **names = NULL;
int count = 0;
size_t maxlen = 0;

struct dirent *e;
while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    names = realloc(names, sizeof(char*) * (count + 1));
    names[count] = strdup(e->d_name);
    size_t len = strlen(e->d_name);
    if (len > maxlen) maxlen = len;
    count++;
}
closedir(dir);
if (count == 0) return;

int width = get_terminal_width();
int spacing = 2;
int cols = width / (maxlen + spacing);
if (cols < 1) cols = 1;
int rows = (count + cols - 1) / cols;

for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
        int idx = c * rows + r;
        if (idx < count)
            printf("%-*s", (int)(maxlen + spacing), names[idx]);
    }
    printf("\n");
}

for (int i = 0; i < count; i++) free(names[i]);
free(names);

}

/* ---------- Main ---------- */

int main(int argc, char *argv[]) {
int long_flag = 0;
const char *dirname = ".";

for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0)
        long_flag = 1;
    else
        dirname = argv[i];
}

if (long_flag)
    long_listing(dirname);
else
    column_display(dirname);

return 0;


}
