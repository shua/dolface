#ifndef DATFILE_H
#define DATFILE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/param.h>

#if BYTE_ORDER == BIG_ENDIAN

#define swap32(x) (x)
#define swap16(x) (x)

#else

static inline uint32_t swap32(uint32_t v)
{
    return (v >> 24) |
           ((v >> 8) & 0x0000FF00) |
           ((v << 8) & 0x00FF0000) |
           (v << 24);
}

static inline uint16_t swap16(uint16_t v)
{
    return (v >> 8) | (v << 8);
}

#endif // BIG_ENDIAN   

typedef struct {
    uint32_t d_filesz;
    uint32_t d_entsz;
    uint32_t d_entnum;
    uint32_t d_nodenum;
    uint32_t d_nodenum2;
    uint32_t d_sp0;
    uint32_t d_sp1;
    uint32_t d_sp2;
} DAT_hdr;

typedef struct {
    uint32_t s_offset;
    uint32_t s_strtaboff;
} DAT_Shdr;

typedef struct {
    uint32_t j_magic;
    uint32_t j_flags;
    uint32_t j_childoff;
    uint32_t j_siblingoff;
    uint32_t j_dobjoff;
    float  j_rotx;
    float  j_roty;
    float  j_rotz;
    float  j_scalex;
    float  j_scaley;
    float  j_scalez;
    float  j_posx;
    float  j_posy;
    float  j_posz;
    uint32_t j_transformoff;
    uint32_t j_sp0;
} DAT_Joint;

void DAT_readhdr(DAT_hdr *hdr, FILE *dat);

#endif // DATFILE_H
