#ifndef _CFG_OPERATOR_FILE_H
#define _CFG_OPERATOR_FILE_H

typedef struct  
{  
    char OPERATOR[64];
}File_OPERATOR_Struct;

#define CFG_FILE_OPERATOR_REC_SIZE    sizeof(File_OPERATOR_Struct)
#define CFG_FILE_OPERATOR_REC_TOTAL   1

#endif
