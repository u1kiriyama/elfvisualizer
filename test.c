#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <elf.h>

int makeParts(char c, int n, char array[n]){
    char side;
    char inner;

    if (c == 'W') {
        side = '+';
        inner = '-';
    }else if (c == 'E') {
        side = '|';
        inner  = ' ';
    }else {
        side = '*';
        inner ='*';
    }
    array[0] = side;
    for (int i = 1; i < n-2; i++) {
        array[i] = inner;
    }
    array[n-2] = side;
    array[n-1] = '\0';

    return (0);
}

int main(){
    int horizontal = 31;
    int vertical = 13;

    char parts[vertical][horizontal];
    makeParts('W', horizontal, parts[0]);
    makeParts('E', horizontal, parts[1]);
    makeParts('W', horizontal, parts[2]);
    makeParts('E', horizontal, parts[3]);
    makeParts('W', horizontal, parts[4]);
    makeParts('E', horizontal, parts[5]);
    makeParts('W', horizontal, parts[6]);
    makeParts('E', horizontal, parts[7]);
    makeParts('W', horizontal, parts[8]);
    makeParts('E', horizontal, parts[9]);
    makeParts('W', horizontal, parts[10]);
    makeParts('E', horizontal, parts[11]);
    makeParts('W', horizontal, parts[12]);

    char* EHtag = "ELF header";
    memcpy(&parts[1][2], EHtag, strlen(EHtag));


    for (int i = 0; i < vertical; i++) {
        printf("%03d ", i);
        printf("%s\n", parts[i]);
    }
    printf("\n");

    

    return (0);
}