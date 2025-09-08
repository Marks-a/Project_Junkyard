#include <stdio.h>
#include <stdlib.h>

#define MAXLIST 10
struct pointList
{
    struct pointList *next;
    void *value;
};

void printStructValue(struct pointList value)
{
    int val = *(int *)value.value;
    printf("%d\n", val);
};

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <starter> <multiplier>\n", argv[0]);
        return 1;
    }
    int starter = atoi(argv[1]);
    int multiplier = atoi(argv[2]);

    struct pointList *pHead = (struct pointList *)malloc(sizeof(struct pointList));
    struct pointList *p = pHead;

    printf("\n ================= \n");
    printf(" &p: %p\n", &p);
    printf(" &pHead: %p\n", &pHead);
    /*
        printf(" p: %p\n", p);
        printf(" pHead: %p\n", pHead);

    Note: No point in this '*p' and '*pHead' dereferencing as they are pointers to struct pointList.
          The dereferencing would be useful if we wanted to access the value inside the struct.
          Here, we are just printing the addresses of the pointers.
          That is not innitlized yet, so it will print garbage values.

        printf(" *p: %p\n", *p);
        printf(" *pHead: %p\n", *pHead);
    */
    printf(" (void *)pHead: %p\n", (void *)pHead);
    printf(" (void *)p: %p\n", (void *)p);

    printf("\n ================= \n");
    for (int i = 0; i < MAXLIST; i++)
    {
        p->value = malloc(sizeof(int));
        *(int *)p->value = (int)starter + i * (int)multiplier;
        printf("Current address: %p\n", (void *)p);
        printf("Current value added to the address: %d\n", *(int *)p->value);
        if (i < MAXLIST - 1)
        {
            p->next = malloc(sizeof(struct pointList));
            printf("Going to the next address: %p\n\n", (void *)p->next);
            p = p->next;
        }
        else
        {
            p->next = NULL;
        }
    };
    printf("Reusing the *p pointer\n");
    p = pHead;
    printf("Current *p address should match the pHead address| (p) -> %p , (head)-> %p\n ", (void *)p, (void *)pHead);

    printf("\n\n ================= \n\n");
    printf("Reading and freeing the memory\n");
    while (p != NULL)
    {
        printf("Current address (just p): %p\n", p);
        printf("Current Value: %d\n", *(int *)p->value);
        struct pointList *next = p->next;
        free(p->value);
        free(p);
        p = next;
    };
    pHead = NULL;
    p = NULL;

    printf("\n ################## \n");
    printf("Current address with of pHead: %p\n", (void *)pHead);
    printf("Current address of p: %p\n", (void *)p);

    return 0;
}