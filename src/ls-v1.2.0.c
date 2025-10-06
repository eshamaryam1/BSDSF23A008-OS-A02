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

/* ---------- Helper ---------- */
void print_permissions(mode_t m) {
char p[11];
p[0] = S_ISDIR(m) ? 'd' : '-';
p[1] = (m & S_IRUSR) ? 'r' : '-';
p[2] = (m & S_IWUSR) ? 'w' : '-';
p[3] = (m & S_IXUSR) ? 'x' : '-';
p[4] = (m & S_IRGRP) ? 'r' : '-';
p[5] = (m & S_IWGRP) ? 'w' : '-';
p[6] = (m & S_IXGRP) ? 'x' : '-';
p[7] = (m & S_IROTH) ? 'r' : '-';
p[8] = (m & S_IWOTH) ? 'w' : '-';
p[9] = (m & S_IXOTH) ? 'x' : '-';
p[10] = '\0';
printf("%s", p);
}

/* ---------- Long listing (-l) ---------- */
void long_listing(const char *d) {
DIR *dir = opendir(d);
if (!dir) { perror("opendir"); return; }

struct dirent *e;
while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", d, e->d_name);
    struct stat st;
    if (stat(path, &st) == -1) { perror("stat"); continue; }

    print_permissions(st.st_mode);
    printf(" %ld", (long)st.st_nlink);

    struct passwd *pw = getpwuid(st.st_uid);
    struct group  *gr = getgrgid(st.st_gid);
    printf(" %s %s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");
    printf(" %6ld", (long)st.st_size);

    char *mt = ctime(&st.st_mtime);
    mt[strlen(mt) - 1] = '\0';
    printf(" %s %s\n", mt, e->d_name);
}
closedir(dir);

}

/* ---------- Helpers for column view ---------- */
int get_terminal_width(void) {
struct winsize w;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
return w.ws_col;
return 80;
}

/* Down-then-across (default) */
void display_down(const char *d) {
DIR *dir = opendir(d);
if (!dir) { perror("opendir"); return; }

char **names = NULL;
int n = 0; size_t max = 0;
struct dirent *e;

while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    names = realloc(names, sizeof(char*)*(n+1));
    names[n] = strdup(e->d_name);
    size_t len = strlen(e->d_name);
    if (len > max) max = len;
    n++;
}
closedir(dir);
if (n == 0) return;

int width = get_terminal_width();
int spacing = 2;
int cols = width / (max + spacing);
if (cols < 1) cols = 1;
int rows = (n + cols - 1) / cols;

for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
        int i = c * rows + r;
        if (i < n)
            printf("%-*s", (int)(max + spacing), names[i]);
    }
    printf("\n");
}

for (int i = 0; i < n; i++) free(names[i]);
free(names);

}

/* ---------- Main ---------- */
int main(int argc, char *argv[]) {
int l = 0;
const char *dir = ".";

for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) l = 1;
    else dir = argv[i];
}

if (l) long_listing(dir);
else   display_down(dir);

return 0;


}
