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

/* ---------- ANSI Colors ---------- */
#define BLUE   "\033[1;34m"
#define GREEN  "\033[1;32m"
#define RESET  "\033[0m"

/* ---------- Permission Printer ---------- */
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

/* ---------- Color Helper ---------- */
void print_colored_name(const char *path, const char *name) {
struct stat st;
if (stat(path, &st) == -1) {
printf("%s", name);
return;
}

if (S_ISDIR(st.st_mode))
    printf(BLUE "%s" RESET, name);
else if (st.st_mode & S_IXUSR)
    printf(GREEN "%s" RESET, name);
else
    printf("%s", name);

}

/* ---------- Long listing (-l) ---------- */
void long_listing(const char *d) {
DIR *dir = opendir(d);
if (!dir) { perror("opendir"); return; }

struct dirent *e;
char **names = NULL;
int n = 0;

while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    names = realloc(names, sizeof(char*)*(n+1));
    names[n++] = strdup(e->d_name);
}
closedir(dir);

qsort(names, n, sizeof(char*), (int(*)(const void*, const void*))strcmp);

for (int i = 0; i < n; i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", d, names[i]);
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
    printf(" %s ", mt);
    print_colored_name(path, names[i]);
    printf("\n");
    free(names[i]);
}
free(names);

}

/* ---------- Utility ---------- */
int get_terminal_width(void) {
struct winsize w;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
return w.ws_col;
return 80;
}

/* Load sorted filenames */
char **load_sorted(const char *d, int *count, size_t *max_len) {
DIR *dir = opendir(d);
if (!dir) { perror("opendir"); return NULL; }

struct dirent *e;
char **names = NULL;
*count = 0;
*max_len = 0;

while ((e = readdir(dir)) != NULL) {
    if (e->d_name[0] == '.') continue;
    names = realloc(names, sizeof(char*) * (*count + 1));
    names[*count] = strdup(e->d_name);
    size_t len = strlen(e->d_name);
    if (len > *max_len) *max_len = len;
    (*count)++;
}
closedir(dir);

qsort(names, *count, sizeof(char*), (int(*)(const void*, const void*))strcmp);
return names;

}

/* Down-then-Across (default) */
void display_down(const char *d) {
int n; size_t max;
char **names = load_sorted(d, &n, &max);
if (!names || n == 0) return;

int width = get_terminal_width(), spacing = 2;
int cols = width / (max + spacing); if (cols < 1) cols = 1;
int rows = (n + cols - 1) / cols;

for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
        int i = c * rows + r;
        if (i < n) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", d, names[i]);
            print_colored_name(path, names[i]);
            printf("%*s", (int)(max - strlen(names[i]) + spacing), "");
        }
    }
    printf("\n");
}

for (int i = 0; i < n; i++) free(names[i]);
free(names);

}

/* Across-then-Down (-x) */
void display_across(const char *d) {
int n; size_t max;
char **names = load_sorted(d, &n, &max);
if (!names || n == 0) return;

int width = get_terminal_width(), spacing = 2;
int cols = width / (max + spacing); if (cols < 1) cols = 1;

for (int i = 0; i < n; i++) {
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", d, names[i]);
    print_colored_name(path, names[i]);
    printf("%*s", (int)(max - strlen(names[i]) + spacing), "");
    if ((i + 1) % cols == 0) printf("\n");
}
if (n % cols != 0) printf("\n");

for (int i = 0; i < n; i++) free(names[i]);
free(names);

}

/* ---------- Main ---------- */
int main(int argc, char *argv[]) {
int l = 0, x = 0;
const char *dir = ".";

for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) l = 1;
    else if (strcmp(argv[i], "-x") == 0) x = 1;
    else dir = argv[i];
}

if (l) long_listing(dir);
else if (x) display_across(dir);
else display_down(dir);

return 0;

}
