/*---------------------------------------------------------------------------*
  Project:  TwlSDK - STD - include
  File:     string.h

  Copyright 2005-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-26#$
  $Rev: 8696 $
  $Author: yasuda_kei $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_STD_STRING_H_
#define NITRO_STD_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/mi/memory.h>

//---- aliases
#define STD_StrCpy          STD_CopyString
#define STD_StrLCpy         STD_CopyLString
#define STD_StrChr          STD_SearchChar
#define STD_StrRChr         STD_SearchCharReverse
#define STD_StrStr          STD_SearchString
#define STD_StrLen          STD_GetStringLength
#define STD_StrNLen         STD_GetStringNLength
#define STD_StrCat          STD_ConcatenateString
#define STD_StrLCat         STD_ConcatenateLString
#define STD_StrCmp          STD_CompareString
#define STD_StrNCmp         STD_CompareNString
#define STD_StrLCmp         STD_CompareLString

#define STD_MemCpy          STD_CopyMemory
#define STD_MemMove         STD_MoveMemory
#define STD_MemSet          STD_FillMemory

/*---------------------------------------------------------------------------*
  Name:         STD_CopyString

  Description:  same to strcpy

  Arguments:    destp : destination pointer
                srcp  : src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_CopyString(char *destp, const char *srcp);

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLStringZeroFill

  Description:  do not correspond with strlcpy

  Arguments:    destp : destination pointer
                srcp  : src pointer
                n     : copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
extern int STD_CopyLStringZeroFill(char *destp, const char *srcp, int n);

/*---------------------------------------------------------------------------*
  Name:         STD_CopyLString

  Description:  same to strlcpy

  Arguments:    destp : destination pointer
                srcp  : src pointer
                siz   : copy size + 1

  Returns:      size of src
 *---------------------------------------------------------------------------*/
extern int STD_CopyLString(char *destp, const char *srcp, int siz);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchChar

  Description:  same to strchr

  Arguments:    srcp : src string
                c    : character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchChar(const char *srcp, int c);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchCharReverse

  Description:  same to strrchr

  Arguments:    srcp : src string
                c    : character to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchCharReverse(const char *srcp, int c);

/*---------------------------------------------------------------------------*
  Name:         STD_SearchString

  Description:  same to strstr

  Arguments:    srcp : src string
                str  : string to search from src pointer

  Returns:      pointer to destination
 *---------------------------------------------------------------------------*/
extern char *STD_SearchString(const char *srcp, const char *str);

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringLength

  Description:  get string length. same to strlen

  Arguments:    str : string

  Returns:      string length
 *---------------------------------------------------------------------------*/
extern int STD_GetStringLength(const char *str);

/*---------------------------------------------------------------------------*
  Name:         STD_GetStringNLength

  Description:  get string length below len. same to strnlen

  Arguments:    str : string
                len : max length

  Returns:      string length
 *---------------------------------------------------------------------------*/
extern int STD_GetStringNLength(const char *str, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateString

  Description:  concatenate strings. same to strcat

  Arguments:    str1 : original string
                str2 : string to concatenate

  Returns:      concatenated string
 *---------------------------------------------------------------------------*/
extern char *STD_ConcatenateString(char *str1, const char *str2);

/*---------------------------------------------------------------------------*
  Name:         STD_ConcatenateLString

  Description:  concatenate strings. same to strlcat

  Arguments:    str1 : original string
                str2 : string to concatenate
                size : buffer size of str1

  Returns:      length of str1 + length of str2
 *---------------------------------------------------------------------------*/
extern int STD_ConcatenateLString(char *str1, const char *str2, int size);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareString

  Description:  compare strings. same to strcmp

  Arguments:    str1, str2 : strings

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareString(const char *str1, const char *str2);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  same to strncmp

  Arguments:    str1, str2 : strings
                len        : max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareNString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareLString

  Description:  same to strlcmp

  Arguments:    str1, str2 : strings
                len        : max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareLString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_CompareNString

  Description:  same to strncasecmp

  Arguments:    str1, str2 : strings
                len        : max length

  Returns:      0 if same
 *---------------------------------------------------------------------------*/
extern int STD_CompareNIString(const char *str1, const char *str2, int len);

/*---------------------------------------------------------------------------*
  Name:         STD_TSScanf

  Description:  省サイズを目的とした sscanf.
                基本的な書式指定 "%(*?)([lh]{,2})([diouxXpn])" に対応.

  Arguments:    src           入力文字列
                fmt           書式制御文字列

  Returns:      代入された値の総数.
                未代入での終端あるいは不正を検出すれば -1.
 *---------------------------------------------------------------------------*/
extern int STD_TSScanf(const char *src, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSScanf

  Description:  STD_TSScanf の va_list 対応版.
                基本的な書式指定 "%(*?)([lh]{,2})[diouxX]" に対応.

  Arguments:    src           入力文字列
                fmt           書式制御文字列
                vlist         パラメータ

  Returns:      代入された値の総数.
                未代入での終端あるいは不正を検出すれば -1.
 *---------------------------------------------------------------------------*/
extern int STD_TVSScanf(const char *src, const char *fmt, va_list vlist);

/*---------------------------------------------------------------------------*
  Name:         STD_TSPrintf

  Description:  引数の形式以外は STD_TVSNPrintf と同じ.

  Arguments:    dst           結果を格納するバッファ
                fmt           書式制御文字列

  Returns:      STD_VSNPrintf と同じ.
 *---------------------------------------------------------------------------*/
extern int STD_TSPrintf(char *dst, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSPrintf

  Description:  引数の形式以外は STD_TVSNPrintf と同じ.

  Arguments:    dst           結果を格納するバッファ
                fmt           書式制御文字列
                vlist         パラメータ

  Returns:      STD_VSNPrintf と同じ.
 *---------------------------------------------------------------------------*/
extern int STD_TVSPrintf(char *dst, const char *fmt, va_list vlist);

/*---------------------------------------------------------------------------*
  Name:         STD_TSNPrintf

  Description:  引数の形式以外は STD_TVSNPrintf と同じ.

  Arguments:    dst           結果を格納するバッファ
                len           バッファ長
                fmt           書式制御文字列
 
  Returns:      STD_VSNPrintf と同じ.
 *---------------------------------------------------------------------------*/
extern int STD_TSNPrintf(char *dst, size_t len, const char *fmt, ...);

/*---------------------------------------------------------------------------*
  Name:         STD_TVSNPrintf

  Description:  省サイズを目的とした sprintf.
                基本的な書式指定に対応.
                %([-+# ]?)([0-9]*)(\.?)([0-9]*)([l|ll|h||hh]?)([diouxXpncs%])

  Note:         CodeWarrior の MSL sprintf() の挙動にあわせて
                '+' と '#' は無効にしてある.
                { // exsample
                  char buf[5];
                  sprintf(buf, "%-i\n", 45);  // "45"  (OK)
                  sprintf(buf, "%0i\n", 45);  // "45"  (OK)
                  sprintf(buf, "% i\n", 45);  // " 45" (OK)
                  sprintf(buf, "%+i\n", 45);  // "%+i" ("+45" expected)
                  sprintf(buf, "%#x\n", 45);  // "%#x" ("0x2d" expected)
                  // but, this works correctly!
                  sprintf(buf, "% +i\n", 45); // "+45" (OK)
                }

  Arguments:    dst           結果を格納するバッファ
                len           バッファ長
                fmt           書式制御文字列
                vlist         パラメータ

  Returns:      書式文字列を正しく出力した場合の文字数を返す. ('\0'を含まない)
                バッファサイズが充分なら全ての文字を出力して終端を付与する.
                バッファサイズが不足なら切り詰めて dst[len-1] を終端とする.
                len が 0 の場合は何もしない.

 *---------------------------------------------------------------------------*/
extern int STD_TVSNPrintf(char *dst, size_t len, const char *fmt, va_list vlist);


static inline void* STD_CopyMemory(void *destp, const void *srcp, u32 size)
{
    MI_CpuCopy(srcp, destp, size);
    return destp;
}

static inline void* STD_MoveMemory(void *destp, const void *srcp, u32 size)
{
    MI_CpuMove(srcp, destp, size);
    return destp;
}

static inline void* STD_FillMemory(void *destp, u8 data, u32 size)
{
	MI_CpuFill(destp, data, size);
	return destp;
}

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_STD_STRING_H_ */
#endif
