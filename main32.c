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
    u_int32_t start;
    u_int32_t end;
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
    u_int32_t a, b;
    char s[0x400];
    char* p = m;
    int i = 0;
    do {
        int r = sscanf(p, "%x-%x ", &a, &b);
//        printf("sscanf %x\n", r);
        blocks[i].start = a;
        blocks[i].end = b;
        blocks[i].info = p + 73;
        while(*p != '\n' && *p) {if(*p)++p;};
        if(*p){
            *p = 0;
            if(blocks[i].info > p) blocks[i].info = p;
            ++p;
        }
//        printf("%x-%x %s\n", a, b, blocks[i].info);
        ++i;
    } while(*p);
    return blocks;
}

void scan(Block* b ,char* c, int l, int fd) {
    //printf("%x-%x %s\n", b->start, b->end, b->info);
    int size = b->end - b->start;
    char* m = malloc(size + 1);
    char* m2 = m + size - l;
    lseek(fd, b->start, SEEK_SET);
    read(fd, m, size);
    char* i = m;
    while(i != m2) {
        if(*i == *c) {
            if(memcmp(i, c, l) == 0) {
                printf("found: %8x  %s\n", (u_int32_t)(i - m), b->info);
            }
        }
        i++;
    }

    free(m);
}

void scanX(Block* b, uint32_t value, int fd) {
    int size = b->end - b->start;
    char* m = malloc(size);  // Буфер для чтения данных
    lseek(fd, b->start, SEEK_SET);
    read(fd, m, size);  // Чтение данных в буфер

    uint32_t* i = (uint32_t*) m;  // Интерпретируем данные как массив 32-битных значений
    uint32_t* m2 = (uint32_t*) (m + size);  // Указатель на конец буфера
    while (i < m2) {  // Перебор всех 32-битных значений
        if (*i == value) {  // Сравнение с искомым значением
            printf("found: %8x  %s\n", (uint32_t)(i - (uint32_t*)m), b->info);  // Вывод результата
        }
        i++;
    }

    free(m);  // Освобождение памяти
}

void monitor(Block* b) {

}

void print_usage() {
    printf("Usage: program [-i <PID>] [-s <string>] [-x <hex_number>]\n");
}

int main(int argc, char** argv) {
    setbuf(stdout, NULL);
    char* str = NULL;
    uint32_t hex_value = 0;
    uint32_t search_value = 0;
    int opt, has_args = 0;
    int pid = getpid();
    while ((opt = getopt(argc, argv, "i:s:x:")) != -1) {
        has_args = 1;
        switch (opt) {
            case 'i':          
                sscanf(optarg, "%d", &pid);                
                break;
            case 's':
                str = optarg;
                break;
            case 'x':
                sscanf(optarg, "%x", &hex_value);
                search_value = 1;
                break;
            case '?':
                print_usage();
                return 1;
        }
    }
    if (!has_args) {
        print_usage();
        return 1;
    }
    printf("Scan process %d\n", pid);
    char filename[256];
    sprintf(filename, "/proc/%d/maps", pid);
    printf("File %s\n", filename);
    int size = 0x100000;
    int fd = open(filename, O_RDONLY);
    char* m = malloc(size + 1);
    int total_read = 0;
    int read_size;
    do {
        read_size = read(fd, m + total_read, size);
 //       printf("read: %d\n", read_size);
        if (read_size > 0) {
            total_read += read_size;
        }
    } while (read_size > 0);
    printf("Total read: %d\n", total_read);
    m[total_read] = 0;
//    printf("%s\n", m);
    close(fd);
    int size2;
    Block* B = parse(m, &size2);
    sprintf(filename, "/proc/%d/mem", pid);
    fd = open(filename, O_RDONLY);
    if (str) {
        printf("Searching string: %s [%d]\n", str, strlen(str));
        for(int i = 0; i < size2; ++i) {
            scan(B + i, str, strlen(str), fd);
        }
    }
    if (search_value) {
        printf("Searching value: %08x (%d)\n", hex_value, hex_value);
        for(int i = 0; i < size2; ++i) {
            scanX(B + i, hex_value, fd);
        }    
    }
    close(fd);
    free(B);
    free(m);
    return 0;
}
