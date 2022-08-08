#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define IS_ELF(ehdr)\
    ((ehdr).e_ident[EI_MAG0] == ELFMAG0 &&\
     (ehdr).e_ident[EI_MAG1] == ELFMAG1 &&\
     (ehdr).e_ident[EI_MAG2] == ELFMAG2 &&\
     (ehdr).e_ident[EI_MAG3] == ELFMAG3)

static int elfdump(char *head)
{
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *shdr, *shstr, *str, *sym, *rel;
    Elf64_Phdr *phdr;
    Elf64_Sym *symp;
    Elf64_Rel *relp;
    int i, j, size;
    char *sname;

    ehdr = (Elf64_Ehdr *)head;

    if (!IS_ELF(*ehdr)) {
        fprintf(stderr, "This is not ELF file.\n");
        return (1);
    }

    if (ehdr->e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "unknown class. (%d)\n", (int)ehdr->e_ident[EI_CLASS]);
        return (1);
    }

    if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
        fprintf(stderr, "unknown endian. (%d)\n", (int)ehdr->e_ident[EI_DATA]);
        return (1);
    } 

    /* added */
    printf("OSABI : %d\n",ehdr->e_ident[EI_OSABI]);
    printf("if 0, UNIX System V ABI\n");

    /* .shstrtab */
    shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);

    /* list of section name */
    printf("Secitons:\n");
    for (i = 0; i < ehdr->e_shnum; i++) {
        shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        sname = (char *)(head + shstr->sh_offset + shdr->sh_name);
        printf("\t[%d]\t%s\n", i , sname);
        if (!strcmp(sname, ".strtab")) str = shdr;
    }

    /* list of segment */
    printf("Segment : \n");
    for ( i = 0; i < ehdr->e_phnum; i++) {
        phdr = (Elf64_Phdr *)(head + ehdr->e_phoff + ehdr->e_phentsize * i);
        printf("\t[%d]\t", i);
        for (j = 0; j < ehdr->e_shnum; j++) {
            shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * j);
            // #define SHT_NOBITS	  8		/* Program space with no data (bss) */
            size = (shdr->sh_type != SHT_NOBITS) ? shdr->sh_size : 0;
            if (shdr->sh_offset < phdr->p_offset) continue;
            if (shdr->sh_offset + size > phdr->p_offset + phdr->p_filesz) continue;
            sname = (char *)(head + shstr->sh_offset + shdr->sh_name);
            printf("%s ", sname);
        }
        printf("\n");
    }

    /* list of symbol */
    printf("Symbols:\n");
    for (i = 0; i < ehdr->e_shnum; i++) {
        shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        if (shdr->sh_type != SHT_SYMTAB) continue;
        sym = shdr;
        for (j = 0; j < sym->sh_size / sym->sh_entsize; j++) {
            symp = (Elf64_Sym *)(head + sym->sh_offset + sym->sh_entsize * j);
            if (!symp->st_name) continue;
            printf("\t[%d]\t%d\t%lu\t%s\n",
            j, (int)ELF64_ST_TYPE(symp->st_info), symp->st_size,
            (char *)(head + str->sh_offset + symp->st_name));
        }
    }

    /* list of relocated symbole */
    printf("Relocations:\n");
    for (i = 0; i < ehdr->e_shnum; i++) {
        shdr =(Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        if ((shdr->sh_type != SHT_REL) && (shdr->sh_type != SHT_RELA)) continue;
        rel = shdr;
        for (j = 0; j < rel->sh_size / rel->sh_entsize; j++) {
            relp = (Elf64_Rel *)(head + rel->sh_offset + rel->sh_entsize * j);
            symp = (Elf64_Sym *)(head + sym->sh_offset + 
                                (sym->sh_entsize * ELF64_R_SYM(relp->r_info)));
            if (!symp->st_name) continue;
            printf("\t[%d]\t%lu\t%s\n",
                    j, ELF64_R_SYM(relp->r_info),(char *)(head + str->sh_offset + symp->st_name));
        }
    }

    return (0);
}

Elf64_Shdr* bubblesort(char *head, Elf64_Shdr *shdr_arr[])
{
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *shdr, *shstr;
    Elf64_Shdr *tmp;

    ehdr = (Elf64_Ehdr *)head;
    shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
    //Elf64_Shdr *shdr_arr[ehdr->e_shnum]; // offset順などで並べ替えられるように各セクションヘッダへのポインタの配列を定義。
    
    printf("\n");
    printf("bubble sort\n");

    for (int i  = 0; i < ehdr->e_shnum-1; i++) {
        for (int j  = 0; j < ehdr->e_shnum-1; j++) {
            if (shdr_arr[j]->sh_offset > shdr_arr[j+1]->sh_offset) {
                tmp = shdr_arr[j];
                shdr_arr[j] = shdr_arr[j+1];
                shdr_arr[j+1] = tmp;
                }
        }
    }

/*
    for (int i = 0; i < ehdr->e_shnum; i++) {
        printf("%s\n", (char *)(head + shstr->sh_offset + shdr_arr[i]->sh_name));
        printf("%lu\n",shdr_arr[i]->sh_offset);
    }
*/
    return *shdr_arr;
}

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

int makeFrame(char *head, Elf64_Shdr *shdr_arr[])
{
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *shdr, *shstr;
    int horizontal = 31;
    int vertical;

    ehdr = (Elf64_Ehdr *)head;
    shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
    vertical = (ehdr->e_shnum) * 2 + 1;
    printf("\n");

    char parts[vertical][horizontal];
    for (int i = 0; i < ehdr->e_shnum; i++) {
        makeParts('W', horizontal, parts[2*i]);
        makeParts('E', horizontal, parts[2*i+1]);
        char* sname = (char *)(head + shstr->sh_offset + shdr_arr[i]->sh_name);
        memcpy(&parts[2*i+1][2], sname, strlen(sname));
    }
    makeParts('W', horizontal, parts[vertical-1]);

    for (int i = 0; i < ehdr->e_shnum; i++) {
        printf("%03d ", 2*i);
        printf("%05lu ", shdr_arr[i]->sh_offset);
        printf("%s\n", parts[2*i]);
        printf("%03d ", 2*i+1);
        printf("      ");
        printf("%s\n", parts[2*i+1]);
    }
    printf("%03d ", 2*ehdr->e_shnum);
    printf("      ");
    printf("%s\n", parts[vertical-1]);
    printf("\n");

}

static int elfvisualizer(char *head)
{
    Elf64_Ehdr *ehdr;
    Elf64_Shdr *shdr, *shstr, *str, *sym, *rel;
    Elf64_Phdr *phdr;
    Elf64_Sym *symp;
    Elf64_Rel *relp;
    int i, j, size;
    char *sname;

    ehdr = (Elf64_Ehdr *)head;

    printf("\tnumber of sections : %d\n", ehdr->e_shnum);

    Elf64_Shdr *shdr_arr[ehdr->e_shnum]; // offset順などで並べ替えられるように各セクションヘッダへのポインタの配列を定義。
    for (i = 0; i < ehdr->e_shnum; i++) {
        shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
        shdr_arr[i] = shdr;
    }
    
    *shdr_arr = bubblesort(head, shdr_arr);    
    
    shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
    for (i = 0; i < ehdr->e_shnum; i++) {
        printf("%s\n", (char *)(head + shstr->sh_offset + shdr_arr[i]->sh_name));
        printf("%lu + %lu = %lu\n",
                shdr_arr[i]->sh_offset, shdr_arr[i]->sh_size, 
                shdr_arr[i]->sh_offset + shdr_arr[i]->sh_size);
    }

    makeFrame(head, shdr_arr);

    return (0);
}

int main(int argc, char *argv[])
{
    int fd;
    struct stat sb;
    char *head;

    fd = open(argv[1], O_RDONLY);
    if (fd < 0) exit (1);
    fstat(fd, &sb) ;
    head = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    elfdump(head);
    printf("==========================\n\n");
    elfvisualizer(head);
    munmap(head, sb.st_size);
    close(fd);

    exit (0);  
}