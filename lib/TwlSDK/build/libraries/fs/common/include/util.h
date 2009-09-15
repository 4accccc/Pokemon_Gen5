/*---------------------------------------------------------------------------*
  Project:  TwlSDK - FS - libraries
  File:     util.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-25#$
  $Rev: 8648 $
  $Author: yosizaki $

 *---------------------------------------------------------------------------*/


#if	!defined(NITRO_FS_UTIL_H_)
#define NITRO_FS_UTIL_H_

#include <nitro/misc.h>
#include <nitro/types.h>


#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

// ARM7 からファイルシステムを使用したい場合はこの定義を有効にする
// #define SDK_ARM7FS

// ファイルシステムのフルセットを含むべき環境のみ定義される
#if	!defined(SDK_ARM7) || defined(SDK_ARM7FS)
#define	FS_IMPLEMENT
#endif


/*---------------------------------------------------------------------------*/
/* functions */

/*---------------------------------------------------------------------------*
  Name:         FSi_IsSlash

  Description:  内部関数.
                ディレクトリ区切り文字か判定.

  Arguments:    c                判定する文字.

  Returns:      ディレクトリ区切り文字なら TRUE.
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FSi_IsSlash(u32 c)
{
    return (c == '/') || (c == '\\');
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPosition

  Description:  Shift_JIS文字列の参照位置を1文字分進める。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      posを1文字進めた参照位置。
 *---------------------------------------------------------------------------*/
SDK_INLINE int FSi_IncrementSjisPosition(const char *str, int pos)
{
    return pos + 1 + STD_IsSjisLeadByte(str[pos]);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPosition

  Description:  Shift_JIS文字列の参照位置を1文字分戻す。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      posを1文字戻した参照位置か-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPosition(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_IncrementSjisPositionToSlash

  Description:  Shift_JIS文字列の参照位置を
                ディレクトリ区切り文字か終端まで進める。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos以降に現れる最初のディレクトリ区切りか終端の位置。
 *---------------------------------------------------------------------------*/
int FSi_IncrementSjisPositionToSlash(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementSjisPositionToSlash

  Description:  Shift_JIS文字列の参照位置を
                ディレクトリ区切り文字か先頭まで戻す。

  Arguments:    str              Shift_JIS文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos未満に現れる最初のディレクトリ区切りか-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementSjisPositionToSlash(const char *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_TrimSjisTrailingSlash

  Description:  Shift_JIS文字列の終端がディレクトリ区切り文字であれば除去。

  Arguments:    str              Shift_JIS文字列。

  Returns:      文字列長。
 *---------------------------------------------------------------------------*/
int FSi_TrimSjisTrailingSlash(char *str);

/*---------------------------------------------------------------------------*
  Name:         FSi_StrNICmp

  Description:  大小文字を区別せずに指定バイト数だけ文字列比較。
                終端のNULを考慮しない点に注意。

  Arguments:    str1             比較元文字列
                str2             比較先文字列
                len              比較バイト数

  Returns:      (str1 - str2) の比較結果
 *---------------------------------------------------------------------------*/
SDK_INLINE int FSi_StrNICmp(const char *str1, const char *str2, u32 len)
{
    int     retval = 0;
    int     i;
    for (i = 0; i < len; ++i)
    {
        u32     c = (u8)(str1[i] - 'A');
        u32     d = (u8)(str2[i] - 'A');
        if (c <= 'Z' - 'A')
        {
            c += 'a' - 'A';
        }
        if (d <= 'Z' - 'A')
        {
            d += 'a' - 'A';
        }
        retval = (int)(c - d);
        if (retval != 0)
        {
            break;
        }
    }
    return retval;
}

/*---------------------------------------------------------------------------*
  Name:         FSi_IsUnicodeSlash

  Description:  Unicodeのパス区切り文字か判定。

  Arguments:    c             Unicode1文字
                
  Returns:      L'/'(0x2F)かL'\\'(0x5C)ならTRUE。
 *---------------------------------------------------------------------------*/
SDK_INLINE BOOL FSi_IsUnicodeSlash(u16 c)
{
    return (c == L'/') || (c == L'\\');
}

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePosition

  Description:  Unicode文字列の参照位置を1文字分戻す。

  Arguments:    str              Unicode文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      posを1文字戻した参照位置か-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePosition(const u16 *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_DecrementUnicodePositionToSlash

  Description:  Unicode文字列の参照位置を
                ディレクトリ区切り文字か先頭まで戻す。

  Arguments:    str              Unicode文字列の先頭を指すポインタ。
                pos              現在の文字列参照位置。(バイト単位)

  Returns:      pos未満に現れる最初のディレクトリ区切りか-1。
 *---------------------------------------------------------------------------*/
int FSi_DecrementUnicodePositionToSlash(const u16 *str, int pos);

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitConditionOn

  Description:  特定のビットが1になるまでスリープ

  Arguments:    flags            監視するビット集合
                bits             1になるべきビット
                queue            スリープ用のキューまたはNULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FSi_WaitConditionOn(u32 *flags, u32 bits, OSThreadQueue *queue)
{
    OSIntrMode bak = OS_DisableInterrupts();
    while ((*flags & bits) == 0)
    {
        OS_SleepThread(queue);
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_WaitConditionOff

  Description:  特定のビットが0になるまでスリープ

  Arguments:    flags            監視するビット集合
                bits             0になるべきビット
                queue            スリープ用のキューまたはNULL

  Returns:      None.
 *---------------------------------------------------------------------------*/
SDK_INLINE void FSi_WaitConditionOff(u32 *flags, u32 bits, OSThreadQueue *queue)
{
    OSIntrMode bak = OS_DisableInterrupts();
    while ((*flags & bits) != 0)
    {
        OS_SleepThread(queue);
    }
    (void)OS_RestoreInterrupts(bak);
}

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFileLengthIfProc

  Description:  指定されたファイルがアーカイブプロシージャであればサイズを取得

  Arguments:    file             ファイルハンドル
                length           サイズの格納先

  Returns:      指定されたファイルがアーカイブプロシージャであればそのサイズ
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFileLengthIfProc(FSFile *file, u32 *length);

/*---------------------------------------------------------------------------*
  Name:         FSi_GetFilePositionIfProc

  Description:  指定されたファイルがアーカイブプロシージャであれば現在位置を取得

  Arguments:    file             ファイルハンドル
                length           サイズの格納先

  Returns:      指定されたファイルがアーカイブプロシージャであればその現在位置
 *---------------------------------------------------------------------------*/
BOOL FSi_GetFilePositionIfProc(FSFile *file, u32 *length);


/*---------------------------------------------------------------------------*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* NITRO_FS_UTIL_H_ */
