#ifndef _CFG_WARRANTY_FILE_H
#define _CFG_WARRANTY_FILE_H

typedef struct
{
    unsigned long long deviceOnTime;
    unsigned long long psTime;
    unsigned long long csTime;
} File_Warranty_Struct;

#define CFG_FILE_WARRANTY_INFO_SIZE    sizeof(File_Warranty_Struct)
#define CFG_FILE_WARRANTY_INFO_TOTAL   1

#endif

