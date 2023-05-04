#include <stdio.h>
#include <stdlib.h>
#include <elf.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];

    // Open the ELF file
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: could not open file '%s'\n", filename);
        return 1;
    }

    // Read the ELF header
    Elf32_Ehdr elf_header;
    fread(&elf_header, sizeof(Elf32_Ehdr), 1, file);

    // Read the section header table
    Elf32_Shdr *section_headers = malloc(elf_header.e_shentsize * elf_header.e_shnum);
    fseek(file, elf_header.e_shoff, SEEK_SET);
    fread(section_headers, elf_header.e_shentsize, elf_header.e_shnum, file);

    // Find the section that contains the symbol table
    Elf32_Shdr *symtab_header = NULL;
    Elf32_Shdr *strtab_header = NULL;
    for (int i = 0; i < elf_header.e_shnum; i++) {
        if (section_headers[i].sh_type == SHT_SYMTAB) {
            symtab_header = &section_headers[i];
            strtab_header = &section_headers[symtab_header->sh_link];
            break;
        }
    }

    if (!symtab_header || !strtab_header) {
        fprintf(stderr, "Error: could not find symbol table in file '%s'\n", filename);
        return 1;
    }

    // Read the symbol table
    Elf32_Sym *symbol_table = malloc(symtab_header->sh_size);
    fseek(file, symtab_header->sh_offset, SEEK_SET);
    fread(symbol_table, symtab_header->sh_size, 1, file);

    // Read the string table
    char *string_table = malloc(strtab_header->sh_size);
    fseek(file, strtab_header->sh_offset, SEEK_SET);
    fread(string_table, strtab_header->sh_size, 1, file);

    // Print the symbol table
    printf("Symbol table:\n");
    for (int i = 0; i < symtab_header->sh_size / sizeof(Elf32_Sym); i++) {
        Elf32_Sym *symbol = &symbol_table[i];
        if (symbol->st_name) {
            char *name = &string_table[symbol->st_name];
            printf("  %s\n", name);
        }
    }

    // Clean up
    free(section_headers);
    free(symbol_table);
    free(string_table);
    fclose(file);

    return 0;
}
