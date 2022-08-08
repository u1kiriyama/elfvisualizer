#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define IS_ELF(ehdr)\
    ((ehdr).e_ident[EI_MAG0] == ELFMAG0 &&\
     (ehdr).e_ident[EI_MAG1] == ELFMAG1 &&\
     (ehdr).e_ident[EI_MAG2] == ELFMAG2 &&\
     (ehdr).e_ident[EI_MAG3] == ELFMAG3)

#define SECTION_PRINT(...) printf("\033[42m");printf(__VA_ARGS__);printf("\033[0m");


int main(int argc, char *argv[])
{
  int fd, i;
  struct stat sb;
  char *head;
  Elf64_Ehdr *ehdr;
  Elf64_Shdr *shdr;
  Elf64_Shdr *shstr;

  fd = open(argv[1], O_RDONLY);
  fstat(fd, &sb);
  head = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

  ehdr = (Elf64_Ehdr *)head;

  if (!IS_ELF(*ehdr)) {
    fprintf(stderr, "This is not ELF file. (%s)\n", argv[1]);
    exit (1);
  }

  /* shstr は shdr の i = ehdr->e_shstrndx(=10 for elfsamp.o) としたもの。*/
  shstr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * ehdr->e_shstrndx);
  //printf("\x1b[42m");
  //printf("\033[42m");
  printf("head\t\t:\t%lu\n", head);
  printf("head\t\t:\t%lu\n", (unsigned long int *)head);
  printf("section header\t:\t%lu\n", shstr);
  printf("section header\t:\t%lu\n", (unsigned long int *)shstr);
  printf("\t\t%lu\n", (unsigned long int *)shstr - (unsigned long int *)head);
  printf("Offset of .shstrtat : %lu (from file top)\n", shstr->sh_offset);
  //printf("\033[0m");
  SECTION_PRINT("head", head);printf("\t\t:\t%lu\n");

  printf("\n");
  for (i = 0; i < ehdr->e_shnum; i++) {
    shdr = (Elf64_Shdr *)(head + ehdr->e_shoff + ehdr->e_shentsize * i);
    printf("%s\n", (char *)(head + shstr->sh_offset + shdr->sh_name));
    /* shdr のアドレスについて
      .text は1440byte目になっており、セクションヘッダテーブルの開始位置と一致
      以降64byteごとになっている。
    */
    printf("\t%lu + %lu = %lu\n", shdr->sh_offset, shdr->sh_size, shdr->sh_offset+shdr->sh_size);
  }

  munmap(head, sb.st_size);
  close(fd);

  exit (0);
}