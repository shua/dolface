#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#ifndef MIN
//! Get the minimum of two values
#define MIN(a, b)	(((a) < (b)) ? (a) : (b))
#endif

int verbosity = 0;

#if BYTE_ORDER == BIG_ENDIAN

#define swap32(x) (x)
#define swap16(x) (x)

#else

static inline uint32_t swap32(uint32_t v)
{
	return (v >> 24) |
	       ((v >> 8)  & 0x0000FF00) |
	       ((v << 8)  & 0x00FF0000) |
	       (v << 24);
}

static inline uint16_t swap16(uint16_t v)
{
	return (v >> 8) | (v << 8);
}

#endif /* BIG_ENDIAN */

typedef struct {

	uint32_t text_off[7];
	uint32_t data_off[11];
	uint32_t text_addr[7];
	uint32_t data_addr[11];
	uint32_t text_size[7];
	uint32_t data_size[11];
	uint32_t bss_addr;
	uint32_t bss_size;
	uint32_t entry;
	uint32_t pad[7];
} DOL_hdr;

void usage(const char* p)
{
    fprintf(stderr, "Usage: %s [-h] [-v] [--] dol-file destination\n", p);
    fprintf(stderr, " Chop a dol file into it's sections as defined by the header.\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "  -h    Show this help\n");
    fprintf(stderr, "  -v    Be more verbose (twice for even more)\n");
}

#define die(x) { fprintf(stderr, x "\n"); exit(1); }
#define perrordie(x) { perror(x); exit(1); }

void ferrordie(FILE *f, const char *str)
{
	if(ferror(f)) {
		fprintf(stderr, "Error while ");
		perrordie(str);
	} else if(feof(f)) {
		fprintf(stderr, "EOF while %s\n", str);
		exit(1);
	} else {
		fprintf(stderr, "Unknown error while %s\n", str);
		exit(1);
	}
}

#define BLOCK (1024*1024)

void fcpy(FILE *dst, FILE *src, uint32_t dst_off, uint32_t src_off, uint32_t size)
{
    int left = size;
    int read;
    int written;
    int block;
    void *blockbuf;

    if(fseek(src, src_off, SEEK_SET) < 0)
        ferrordie(src, "seeking to source offset");
    if(fseek(dst, dst_off, SEEK_SET) < 0)
        ferrordie(dst, "seeking to destination offset");

    blockbuf = malloc(MIN(BLOCK, left));

    while(left) {
        block = MIN(BLOCK, left);
        read = fread(blockbuf, 1, block, src);
        if(read != block) {
            free(blockbuf);
            ferrordie(src, "reading DOL segment data");
        }
        written = fwrite(blockbuf, 1, block, dst);
        if(written != block) {
            free(blockbuf);
            ferrordie(dst, "writing chopped bits");
        }
        left -= block;
    }
    free(blockbuf);
}

typedef struct {
    char name[8];
    uint32_t offset;
    uint32_t size;
    uint32_t address;
} DOL_tmem;

void fdolinfohdr(FILE *dst, DOL_hdr *dol)
{
    if(fseek(dst, 0, SEEK_SET) < 0)
        ferrordie(dst, "seeking to begginning of destination file");

    char sbuf[100] = "DOL header information\n";
    int written = fwrite(sbuf, 1, strlen(sbuf), dst);

    sprintf(sbuf, "name    : offset   : size     : end\n");
    written = fwrite(sbuf, 1, strlen(sbuf), dst);

    DOL_tmem *table = malloc(sizeof(DOL_tmem) * 20);
    memset(table, 0, sizeof(DOL_tmem) * 20);
    int tsize = 0;
    for(int i=0; i<7; ++i) {
        if(swap32(dol->text_size[i]) != 0) {
            sprintf(sbuf, "text %2d : %8x : %8x : %8x\n", i,
                    swap32(dol->text_off[i]), swap32(dol->text_size[i]), 
                    swap32(dol->text_off[i]) + swap32(dol->text_size[i]));
            written = fwrite(sbuf, 1, strlen(sbuf), dst);
            char tbuf[8];
            sprintf(tbuf, "text %2d", i);
            strcpy(table[tsize].name, tbuf);
            table[tsize].size = swap32(dol->text_size[i]);
            table[tsize].address = swap32(dol->text_addr[i]);
            ++tsize;
        }
    }
    for(int i=0; i<11; ++i) {
        if(swap32(dol->data_size[i]) != 0) {
            sprintf(sbuf, "data %2d : %8x : %8x : %8x\n", i,
                    swap32(dol->data_off[i]), swap32(dol->data_size[i]),
                    swap32(dol->data_off[i]) + swap32(dol->data_size[i]));
            written = fwrite(sbuf, 1, strlen(sbuf), dst);
            char tbuf[8];
            sprintf(tbuf, "data %2d", i);
            strcpy(table[tsize].name, tbuf);
            table[tsize].size = swap32(dol->data_size[i]);
            table[tsize].address = swap32(dol->data_addr[i]);
            ++tsize;
        }
    }
    strcpy(table[tsize].name, "bss");
    table[tsize].size = swap32(dol->bss_size);
    table[tsize].address = swap32(dol->bss_addr);
    ++tsize;
    strcpy(table[tsize].name, "entry");
    table[tsize].address = swap32(dol->entry);
    ++tsize;

    memset(sbuf, 0, sizeof(sbuf));
    sprintf(sbuf, "\nmemory map\n");
    written = fwrite(sbuf, 1, strlen(sbuf), dst);
    for(int i = 1; i < tsize; ++i) {
        for(int j = i; j != 0 && table[j].address < table[j-1].address; --j) {
            DOL_tmem temp = table[j-1];
            table[j-1] = table[j];
            table[j] = temp;
        }
    }

    for(int i=0; i<tsize; ++i) {
        sprintf(sbuf, "%7s : %8x : %8x : %8x\n", table[i].name,
                table[i].address, table[i].size, table[i].address + table[i].size);
        written = fwrite(sbuf, 1, strlen(sbuf), dst);
    }
}

int main (int argc, char **argv)
{
    char **arg;

    if(argc < 2) {
        usage(argv[0]);
        return 1;
    }
    arg = &argv[1];
    argc--;

    while(argc && *arg[0] == '-') {
        if(!strcmp(*arg, "-h")) {
            usage(argv[0]);
            return 1;
        } else if(!strcmp(*arg, "-v")) {
            verbosity++;
        } else if(!strcmp(*arg, "--")) {
            arg++;
            argc--;
            break;
        } else {
            fprintf(stderr, "Unrecognized option %s\n", *arg);
            usage(argv[0]);
            return 1;
        }
        arg++;
        argc--;
    }
    if(argc < 2) {
        usage(argv[0]);
        exit(1);
    }

    FILE *dol, *out;
    DOL_hdr hdr;
    int read, i;
    char buf[512];

    if(verbosity >= 2) {
        if(BYTE_ORDER == BIG_ENDIAN)
            fprintf(stderr, "System is big endian\n");
        if(BYTE_ORDER == LITTLE_ENDIAN)
            fprintf(stderr, "System is little endian\n");
    }

    dol = fopen(arg[0], "rb");
    if(!dol)
        perrordie("Could not open dol file");

    if(verbosity >= 1)
        fprintf(stderr, "Reading DOL...\n");

    read = fread(&hdr, sizeof(hdr), 1, dol);
    if(read != 1)
        ferrordie(dol, "reading DOL header");

    if(verbosity >= 2)
        fprintf(stderr, "Valid DOL header found.\n");

    if(verbosity >= 2)
        fprintf(stderr, "Writing dol-info.txt\n");
    sprintf(buf, "%s/dol-info.txt", arg[1]);
    out = fopen(buf, "wb");
    if(!out)
        perrordie("Could not open output file for writing");
    fdolinfohdr(out, &hdr);
    fclose(out);

    if(verbosity >= 1)
        fprintf(stderr, "Writing chopped bits to %s...\n", arg[1]);
    
    sprintf(buf, "%s/dol-hdr", arg[1]);
    if(verbosity >= 2)
        fprintf(stderr, "Writing DOL header to %s\n", buf);
    out = fopen(buf, "wb");
    if(!out)
        perrordie("Could not open output file for writing");
    fcpy(out, dol, 0, 0, sizeof(hdr));
    fclose(out);

    for(i=0; i<7; ++i) {
        if(swap32(hdr.text_size[i]) != 0) {
            sprintf(buf, "%s/dol-text.%d", arg[1], i);
            if(verbosity >= 2)
                fprintf(stderr, "Writing text segment %d to %s\n", i, buf);
            FILE* out = fopen(buf, "wb");
            if(!out)
                perrordie("Could not open output file for writing");
            fcpy(out, dol, 0, swap32(hdr.text_off[i]), swap32(hdr.text_size[i]));
            fclose(out);
        }
    }
    for(i=0; i<11; ++i) {
        if(swap32(hdr.data_size[i]) != 0) {
            sprintf(buf, "%s/dol-data.%d", arg[1], i);
            if(verbosity >= 2)
                fprintf(stderr, "Writing data segment %d to %s\n", i, buf);
            FILE* out = fopen(buf, "wb");
            if(!out)
                perrordie("Could not open output file for writing");
            fcpy(out, dol, 0, swap32(hdr.data_off[i]), swap32(hdr.data_size[i]));
            fclose(out);
        }
    }

    return 0;
}
