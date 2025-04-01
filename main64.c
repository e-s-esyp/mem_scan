#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <malloc.h>
#include <sys/types.h>
#include <string.h>

char HEX[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

char* c2hex(const char* in) {
    int len = strlen(in);
    char* out = (char*)malloc(len * 3 + 2);
    const char* p = in;
    char* q = out;
    while(*p) {
        u_char c = (u_char)(*p++);
        *q++ = HEX[c>>4];
        *q++ = HEX[c&15];
        *q++ = ' ';
    }
    return out;
}

char* c2hexl(const char* in, int n) {
    char* out = (char*)malloc(n * 3 + 2 + n / 16);
    const char* p = in;
    char* q = out;
    for(int i = 0; i < n; ++i) {
        if(i % 16 == 0 && i > 0) *q++ = '\n';
        u_char c = (u_char)(*p++);
        *q++ = HEX[c>>4];
        *q++ = HEX[c&15];
        *q++ = ' ';
    }
    return out;
}

typedef struct block {
    u_int64_t start;
    u_int64_t end;
    char* info;
} Block;

int count(char* m, char c) {
    char* p = m;
    int r = 0;
    while(*p) {
        if(*p++ == c) r++;
    }
    return r;
}

Block* parse(char* m, int* size) {
    *size = count(m, '\n');
    printf("creating %d blocks\n", *size);
    Block* blocks = malloc(*size * sizeof(Block));
    u_int64_t a, b;
    char s[0x400];
    char* p = m;
    int i = 0;
    do {
        sscanf(p, "%lx-%lx ", &a, &b);
        blocks[i].start = a;
        blocks[i].end = b;
        blocks[i].info = p + 73;
        while(*p != '\n' && *p) {if(*p)++p;};
        if(*p){
            *p = 0;
            if(blocks[i].info > p) blocks[i].info = p;
            ++p;
        }
        printf("%lx-%lx %s\n", a, b, blocks[i].info);
        ++i;
    } while(*p);
    return blocks;
}

void scan(Block* b ,char* c, int l, int fd) {
    //printf("%lx-%lx %s\n", b->start, b->end, b->info);
    int size = b->end - b->start;
    char* m = malloc(size + 1);
    char* m2 = m + size - l;
    lseek(fd, b->start, SEEK_SET);
    read(fd, m, size);
    char* i = m;
    while(i != m2) {
        if(*i == *c) {
            if(memcmp(i, c, l) == 0) {
                printf("found: %8lx  %s\n", (u_int64_t)(i - m), b->info);
            }
        }
        i++;
    }

    free(m);
}

void monitor(Block* b) {

}

int main(int argc, char** arg) {
    setbuf(stdout, NULL);
    int pid = 0;
    if (argc == 1) {
        pid = getpid();
    } else if (argc == 2) {
        printf("Scan process %s\n", arg[1]);
        sscanf(arg[1], "%d", &pid);
    }
    printf("Scan process %d\n", pid);
    char filename[256];
    sprintf(filename, "/proc/%d/maps", pid);
    printf("File %s\n", filename);
    int size = 0x1000;
    int fd = open(filename, O_RDONLY);
    char* m = malloc(size + 1);
    int total_read = 0;
    int read_size;
    read_size = read(fd, m, size);
    printf("read: %d\n", read_size);
    total_read += read_size;
    m[total_read] = 0;
    printf("%s\n", m);
    close(fd);
    int size2;
    Block* B = parse(m, &size2);
    sprintf(filename, "/proc/%d/mem", pid);
    fd = open(filename, O_RDONLY);
    printf("        filename\n");
    for(int i = 0; i < size2; ++i) {
        scan(B + i, "filename", 8, fd);
    }
    printf("        ELF\n");
    for(int i = 0; i < size2; ++i) {
        scan(B + i, "ELF", 3, fd);
    }
    close(fd);
    free(B);
    free(m);
    printf("Total read: %d\n", total_read);
    return 0;
}
