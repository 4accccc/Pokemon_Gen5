/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     argument.h

  Copyright 2005-2009 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-06-04#$
  $Rev: 10698 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef NITRO_OS_ARGUMENT_H_
#define NITRO_OS_ARGUMENT_H_

#ifdef SDK_TWL
#include <twl/hw/common/mmap_parameter.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---- force to be available argument area
//#define OS_ARGUMENT_FORCE_TO_BE_AVAILABLE   TRUE

// if finalrom, no argument (for NITRO)
#ifndef SDK_TWL
#ifdef SDK_FINALROM
#define OS_NO_ARGUMENT      TRUE
#define OS_NO_ARGC_AND_ARGV TRUE
#endif
#endif

// if forced, let argument area available
#ifdef OS_ARGUMENT_FORCE_TO_BE_AVAILABLE
#ifdef OS_NO_ARGUMENT
#undef OS_NO_ARGUMENT
#endif
#ifdef OS_NO_ARGC_AND_ARGV
#undef OS_NO_ARGC_AND_ARGV
#endif
#endif

// no assert on win32
#if (defined(SDK_WIN32) || defined(SDK_FROM_TOOL))
#define SDK_ASSERT(exp)                         ((void) 0)
#define SDK_ALIGN4_ASSERT(exp)                  ((void) 0)
#define SDK_MINMAX_ASSERT(exp, min, max)        ((void) 0)
#define SDK_NULL_ASSERT(exp)                    ((void) 0)
// inline for VC
#if (defined(_MSC_VER) && !defined(__cplusplus))
#define inline __inline
#endif
#endif

//================================================================================

//---- argument string buffer size
#define OS_ARGUMENT_BUFFER_SIZE  	256

#ifndef SDK_TWL
//==== Argument defines for NITRO
//---- argument buffer identification string (max 17 chars)
#define OS_ARGUMENT_ID_STRING      	":$@$Argument$@$:"
#define OS_ARGUMENT_ID_STRING_BUFFER_SIZE 18

//---- argument buffer struct
typedef struct OSArgumentBuffer
{
    char    argMark[OS_ARGUMENT_ID_STRING_BUFFER_SIZE];
    u16     size;
#if defined(SDK_WIN32) || defined(SDK_FROM_TOOL)
    char    buffer[OS_ARGUMENT_BUFFER_SIZE];
#else
    const char buffer[OS_ARGUMENT_BUFFER_SIZE];
#endif
}
OSArgumentBuffer;

#else // ifndef SDK_TWL
//==== Argument defines for TWL
typedef struct OSArgumentBuffer
{
#if defined(SDK_WIN32) || defined(SDK_FROM_TOOL)
    char    buffer[OS_ARGUMENT_BUFFER_SIZE];
#else
	const char buffer[OS_ARGUMENT_BUFFER_SIZE];
#endif
}
OSArgumentBuffer;
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgc

  Description:  Get number of valid arguments.
                This function is for debug.

  Arguments:    None

  Returns:      number of valid arguments.
                return 1 if no argument.
                return 0 if not set argument buffer.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGC_AND_ARGV
extern int OS_GetArgc(void);
#else
static inline int OS_GetArgc(void)
{
	return 0;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgv

  Description:  Get the pointer to the specified argument string.
                This function is for debug.

  Arguments:    n : index of argument. n==1 means the first argument.
                    n must less than value of OS_GetArgc()

  Returns:      pointer to the specified argument string
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGC_AND_ARGV
extern const char *OS_GetArgv(int n);
#else
static inline const char *OS_GetArgv(int n)
{
#pragma unused(n)
    return NULL;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetOpt/OS_GetOptArg/OS_GetOptInd/OS_GetOptOpt

  Description:  getopt() like function to get command line options
  
  Arguments:    optstring  オプションキャラクタの列
                           NULL なら内部パラメータがリセットされる

  Returns:      オプション文字コード
                '?' なら不明なオプション文字コード
                -1  ならオプションが存在しない
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
int     OS_GetOpt(const char *optstring);
#else
static inline int OS_GetOpt(const char *optstring)
{
#pragma unused(optstring)
    return -1;
}
#endif

extern const char *OSi_OptArg;
extern int OSi_OptInd;
extern int OSi_OptOpt;

static inline const char *OS_GetOptArg(void)
{
    return OSi_OptArg;
}
static inline int OS_GetOptInd(void)
{
    return OSi_OptInd;
}
static inline int OS_GetOptOpt(void)
{
    return OSi_OptOpt;
}

/*---------------------------------------------------------------------------*
  Name:         OS_ConvertToArguments

  Description:  convert string data to arg binary
  
  Arguments:    str     : string
                cs      : character to separate
                buffer  : buffer to store
                bufSize : max buffer size
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
extern void OS_ConvertToArguments(const char *str, char cs, char *buffer, u32 bufSize);
#else
static inline void OS_ConvertToArguments(const char *str, char cs, char *buffer, u32 bufSize)
{
#pragma unused(str, cs, buffer, bufSize)
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_SetArgumentBuffer

  Description:  force to set argument buffer.
  
  Arguments:    buffer : argument buffer
 
  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
extern void OS_SetArgumentBuffer(const char *buffer);
#else
static inline void OS_SetArgumentBuffer(const char *buffer)
{
#pragma unused(buffer)
}
#endif

/*---------------------------------------------------------------------------*
  Name:         OS_GetArgumentBuffer

  Description:  get pointer to argument buffer.
  
  Arguments:    None.
 
  Returns:      pointer to argument buffer.
 *---------------------------------------------------------------------------*/
#ifndef OS_NO_ARGUMENT
extern const char *OS_GetArgumentBuffer(void);
#else
static inline const char *OS_GetArgumentBuffer(void)
{
    return NULL;
}
#endif

//================================================================================
// for TWL
//================================================================================
#ifdef SDK_TWL

//---- header of argument area
typedef struct
{
    u64 titleId;
    u8  reserved1;
    u8  flag;
    u16 makerCode;
    u16 argBufferSize;
    u16 binarySize;
    u16 crc;
    u16 sysParam;
}
OSDeliverArgHeader;

//---- size of argument data buffer
#define OS_DELIVER_ARG_BUFFER_SIZE   (HW_PARAM_DELIVER_ARG_SIZE -sizeof(OSDeliverArgHeader))

//---- deliver argument info
typedef struct
{
    OSDeliverArgHeader header;
    u8 buf[OS_DELIVER_ARG_BUFFER_SIZE];

} OSDeliverArgInfo;

//---- status
#define OS_DELIVER_ARG_BUF_INVALID      0
#define OS_DELIVER_ARG_BUF_ACCESSIBLE   1
#define OS_DELIVER_ARG_BUF_WRITABLE     2

//---- result value
#define OS_DELIVER_ARG_SUCCESS          0
#define OS_DELIVER_ARG_NOT_READY       -1
#define OS_DELIVER_ARG_OVER_SIZE       -2

//---- flags
#define OS_DELIVER_ARG_ENCODE_FLAG		1
#define OS_DELIVER_ARG_VALID_FLAG       2

/*---------------------------------------------------------------------------*
  Name:         OS_InitDeliverArgInfo

  Description:  initialize the argument delivery system for TWL
  
  Arguments:    binSize : buffer size for binary area.
 
  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_InitDeliverArgInfo( OSDeliverArgInfo* info, int binSize );

/*---------------------------------------------------------------------------*
  Name:         OS_SetStringToDeliverArg

  Description:  write specified string as argument data
  
  Arguments:    str : string to write
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY ... 
 *---------------------------------------------------------------------------*/
int OS_SetStringToDeliverArg( const char* str );

/*---------------------------------------------------------------------------*
  Name:         OS_SetBinaryToDeliverArg

  Description:  write sprcified binary 
  
  Arguments:    bin  : pointer of binary
                size : binary size
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY ... 
 *---------------------------------------------------------------------------*/
int OS_SetBinaryToDeliverArg( const void* bin, int size );

/*---------------------------------------------------------------------------*
  Name:         OS_ConvertStringToDeliverArg

  Description:  convert string to arguments
  
  Arguments:    str : string
                cs  : character to separate
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success 
                OS_DELIVER_ARG_OVER_SIZE  ... failed. (over buffer)
                OS_DELIVER_ARG_NOT_READY  ... not ready
 *---------------------------------------------------------------------------*/
int OS_ConvertStringToDeliverArg(const char *str, char cs);

/*---------------------------------------------------------------------------*
  Name:         OS_EncodeDeliverArg

  Description:  Encode argument buffer.
  
  Arguments:    None
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success 
                OS_DELIVER_ARG_NOT_READY  ... not ready
 *---------------------------------------------------------------------------*/
int OS_EncodeDeliverArg(void);

/*---------------------------------------------------------------------------*
  Name:         OS_DecodeDeliverArg

  Description:  decode to work buffer from HW_PARAM_DELIVER_ARG
  
  Arguments:    None
 
  Returns:      OS_DELIVER_ARG_SUCCESS    ... success
                OS_DELIVER_ARG_NOT_READY  ... not ready
 *---------------------------------------------------------------------------*/
int OS_DecodeDeliverArg(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgState

  Description:  get deliber arg state
  
  Arguments:    None
 
  Returns:      state.
                one of the following:
                  OS_DELIVER_ARG_BUF_INVALID
                  OS_DELIVER_ARG_BUF_ACCESSIBLE
                  OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE
 *---------------------------------------------------------------------------*/
u32 OS_GetDeliverArgState(void);

/*---------------------------------------------------------------------------*
  Name:         OS_SetDeliverArgStateInvalid

  Description:  set OS_DELIVER_ARG_BUF_INVALID to deliber arg state
  
  Arguments:    None
 
  Returns:      None
 *---------------------------------------------------------------------------*/
void OS_SetDeliverArgStateInvalid( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetBinarySizeFromDeliverArg

  Description:  read binary size
  
  Arguments:    None
 
  Returns:      0> ... size
                -1 ... work area not ready
 *---------------------------------------------------------------------------*/
int OS_GetBinarySizeFromDeliverArg(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetBinaryFromDeliverArg

  Description:  read binary data
  
  Arguments:    buffer  : pointer of binary
                size    : pointer of binary size
                maxSize : max size
 
  Returns:      OS_DELIVER_ARG_SUCCESS   ... success.
                OS_DELIVER_ARG_OVER_SIZE ... success but buffer over.
                OS_DELIVER_ARG_NOT_READY ... 
 *---------------------------------------------------------------------------*/
int OS_GetBinaryFromDeliverArg( void* buffer, int* size, int maxSize );

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleIdFromDeliverArg

  Description:  get title ID
  
  Arguments:    None
 
  Returns:      title ID.
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
OSTitleId OS_GetTitleIdFromDeliverArg( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetGameCodeFromDeliverArg

  Description:  get game code
  
  Arguments:    None
 
  Returns:      game code
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
u32 OS_GetGameCodeFromDeliverArg( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetMakerCodeFromDeliverArg

  Description:  get maker code
  
  Arguments:    None
 
  Returns:      maker code
                if not accessible, return 0
 *---------------------------------------------------------------------------*/
u16 OS_GetMakerCodeFromDeliverArg( void );

/*---------------------------------------------------------------------------*
  Name:         OS_IsValidDeliverArg

  Description:  check if deliver arg contains valid data
  
  Arguments:    None
 
  Returns:      TRUE if valid
                FALSE if not valid
 *---------------------------------------------------------------------------*/
BOOL OS_IsValidDeliverArg( void );

/*---------------------------------------------------------------------------*
  Name:         OS_IsDeliverArgEncoded

  Description:  get titleId that is contained in last encoded deliverArg
  
  Arguments:    None
 
  Returns:      TRUE  if deliverArg has been encoded
                FALSE if deliverArg has never been encoded since application was
                launched
 *---------------------------------------------------------------------------*/
BOOL OS_IsDeliverArgEncoded( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetTitleIdLastEncoded

  Description:  get titleId that is contained in last encoded deliverArg
  
  Arguments:    None
 
  Returns:      TitleID if deliverArg has been encoded
                0 if deliverArg has never been encoded since application was
                launched
 *---------------------------------------------------------------------------*/
OSTitleId OS_GetTitleIdLastEncoded( void );

/*---------------------------------------------------------------------------*
  Name:         OS_SetSysParamToDeliverArg

  Description:  set sysParam to deliverArg
  
  Arguments:    param: parameter to set
 
  Returns:      None
 *---------------------------------------------------------------------------*/
int OS_SetSysParamToDeliverArg( u16 param );

/*---------------------------------------------------------------------------*
  Name:         OS_GetSysParamFromDeliverArg

  Description:  get sysParam from deliverArg
  
  Arguments:    None
 
  Returns:      parameter. 0 if FAIL.
 *---------------------------------------------------------------------------*/
u16 OS_GetSysParamFromDeliverArg( void );

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgc

  Description:  Get number of valid arguments in deliverArg.

  Arguments:    None

  Returns:      number of valid arguments.
                return 1 if no argument.
                return 0 if buffer is not prepared.
 *---------------------------------------------------------------------------*/
int OS_GetDeliverArgc(void);

/*---------------------------------------------------------------------------*
  Name:         OS_GetDeliverArgv

  Description:  Get the pointer to the specified argument string in deliverArg.

  Arguments:    n : index of argument. n==1 means the first argument.
                    n must less than value of OS_GetDeliverArgc()

  Returns:      pointer of string
 *---------------------------------------------------------------------------*/
const char *OS_GetDeliverArgv(int n);


//----------------------------------------------------------------
// These functions are for system use.
// Don't use them.
//
void OSi_SetDeliverArgState(u32 state);


#endif // ifdef SDK_TWL

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_OS_ARGUMENT_H_ */
#endif
