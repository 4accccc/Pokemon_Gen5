/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MI - include
  File:     uncomp_stream.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-18#$
  $Rev: 8573 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/

#ifndef NITRO_MI_UNCOMP_STREAM_H_
#define NITRO_MI_UNCOMP_STREAM_H_

#include <nitro/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    u8     *destp;                     // 書き込み先ポインタ          4B
    s32     destCount;                 // 残り書き込みサイズ          4B
    u16     destTmp;                   // 書き込み途中のデータ        2B
    u8      destTmpCnt;                // 書き込み途中のデータサイズ  1B
    u8      flags;                     // 圧縮フラグ                  1B
    u16     length;                    // 連続書き込み残りサイズ      2B
    u8      _padding[2];               //                             2B
    //                         計 16B
}
MIUncompContextRL;


typedef struct
{
    u8     *destp;                     // 書き込み先ポインタ              4B
    s32     destCount;                 // 残り書き込みサイズ              4B
    u32     length;                    // 連続書き込み残り長              4B
    u16     destTmp;                   // 書き込み途中のデータ            2B
    u8      destTmpCnt;                // 書き込み途中のデータサイズ      1B
    u8      flags;                     // 圧縮フラグ                      1B
    u8      flagIndex;                 // カレント圧縮フラグインデックス  1B
    u8      lengthFlg;                 // length取得済フラグ              1B
    u8      exFormat;                  // LZ77圧縮拡張オプション          1B
    u8      _padding[1];               //                                 1B
    //                             計 20B
}
MIUncompContextLZ;


typedef struct
{
    u8     *destp;                     // 書き込み先ポインタ                     4B
    s32     destCount;                 // 残り書き込みサイズ                     4B
    u8     *treep;                     // ハフマン符号テーブル、カレントポインタ 4B
    u32     srcTmp;                    // 読み込み途中のデータ                   4B
    u32     destTmp;                   // 復号化途中のデータ                     4B
    s16     treeSize;                  // ハフマン符号テーブルサイズ             2B
    u8      srcTmpCnt;                 // 読み込み途中のデータサイズ             1B
    u8      destTmpCnt;                // 復号化済のビット数                     1B
    u8      bitSize;                   // 符号化ビットサイズ                     1B
    u8      _padding2[3];              //                                        3B
    u8      tree[0x200];               // ハフマン符号テーブル                 512B  (4bit符号化であれば32Bで十分なのですが8bit分確保)
    //                                   計 540B  (4bit符号化の場合は60Bあればよい)
}
MIUncompContextHuffman;


void    MI_InitUncompContextRL(MIUncompContextRL *context, u8 *dest,
                               const MICompressionHeader *header);
void    MI_InitUncompContextLZ(MIUncompContextLZ *context, u8 *dest,
                               const MICompressionHeader *header);
void    MI_InitUncompContextHuffman(MIUncompContextHuffman *context, u8 *dest,
                                    const MICompressionHeader *header);

s32     MI_ReadUncompRL8(MIUncompContextRL *context, const u8 *data, u32 len);
s32     MI_ReadUncompRL16(MIUncompContextRL *context, const u8 *data, u32 len);
s32     MI_ReadUncompLZ8(MIUncompContextLZ *context, const u8 *data, u32 len);
s32     MI_ReadUncompLZ16(MIUncompContextLZ *context, const u8 *data, u32 len);
s32     MI_ReadUncompHuffman(MIUncompContextHuffman *context, const u8 *data, u32 len);

#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_MI_UNCOMP_STREAM_H_ */
#endif
