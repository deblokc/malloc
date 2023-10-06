#include <stdlib.h>

void show_alloc_mem(void);

int main()
{
    void *p;
    p = malloc(1024);
    p = malloc(1024 * 32);
    p = malloc(1024 * 1024);
    p = malloc(1024 * 1024 * 16);
    p = malloc(1024 * 1024 * 128);
    show_alloc_mem(); 
    return (0); 
}
