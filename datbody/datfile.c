#define DATFILE_IMPLEMENT
#include "datfile.h"

int main(int argc, char** argv) {
    FILE* fp;
    char hdrbuf[DatHeaderbufsize] = {0};
    DatHeader hdr = {0};

    fp = fopen("PlPp.dat", "r");
    fread(hdrbuf, DatHeaderbufsize, 1, fp);

    deserializeDatHeader(&hdr, hdrbuf);
    printDatHeader(&hdr);
    return 0;
}
