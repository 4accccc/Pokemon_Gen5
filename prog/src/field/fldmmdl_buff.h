#ifndef FLDMMDL_BUFF_H_FILE
#define FLDMMDL_BUFF_H_FILE

#define FLDMMDL_BUFFER_MAX (48)

typedef struct _TAG_FLDMMDL_BUFFER FLDMMDL_BUFFER;

u32 FLDMMDL_BUFFER_GetWorkSize( void );

void FLDMMDL_BUFFER_InitBuffer( void *p );

#endif //FLDMMDL_BUFF_H_FILE
