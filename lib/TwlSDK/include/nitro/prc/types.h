/*---------------------------------------------------------------------------*
  Project:  TwlSDK - PRC - include
  File:     prc/types.h

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

#ifndef NITRO_PRC_TYPES_H_
#define NITRO_PRC_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro/misc.h>
#include <nitro/types.h>
#include <nitro/fx/fx.h>

/*===========================================================================*
  Constant Definitions
 *===========================================================================*/

//--- ペンが上がったことを示す印の値
#define PRC_PEN_UP_MARKER_X (-1)
#define PRC_PEN_UP_MARKER_Y (-1)

//--- 未認識・認識不能を表すコード
#define PRC_CODE_UNKNOWN (-1)

//--- すべての種類をあらわす kindMask 値
#define PRC_KIND_ALL      (0xffffffffU)

//--- 番犬用の十分に大きい/小さい座標値
#define PRC_LARGE_ENOUGH_X (512)
#define PRC_SMALL_ENOUGH_X (-512)
#define PRC_LARGE_ENOUGH_Y (512)
#define PRC_SMALL_ENOUGH_Y (-512)

//--- fx32 値で小さな値(0除算避け)
#define PRC_TINY_LENGTH (1)

/*===========================================================================*
  Type Definitions
 *===========================================================================*/

/*---------------------------------------------------------------------------*
  2次元座標
 *---------------------------------------------------------------------------*/
typedef struct PRCPoint
{
    s16     x;
    s16     y;
}
PRCPoint;

/*---------------------------------------------------------------------------*
  bounding box
 *---------------------------------------------------------------------------*/
typedef struct PRCBoundingBox
{
    s16     x1, y1;                    // バウンディングボックスの左上の座標
    s16     x2, y2;                    // バウンディングボックスの右下の座標
}
PRCBoundingBox;

/*---------------------------------------------------------------------------*
  画の集合
  必ず、PenUpMarker で terminate されていないといけない
 *---------------------------------------------------------------------------*/
typedef struct PRCStrokes
{
    PRCPoint *points;
    int     size;
    u32     capacity;
}
PRCStrokes;

/*---------------------------------------------------------------------------*
  認識パターン
 *---------------------------------------------------------------------------*/
typedef struct PRCPrototypeEntry
{
    BOOL    enabled;
    u32     kind;
    u16     code;
    fx16    correction;
    void   *data;
    int     pointIndex;
    u16     pointCount;
    u16     strokeCount;
}
PRCPrototypeEntry;

/*---------------------------------------------------------------------------*
  見本DBの元データ
 *---------------------------------------------------------------------------*/
typedef struct PRCPrototypeList
{
    const PRCPrototypeEntry *entries;
    int     entrySize;
    const PRCPoint *pointArray;
    int     pointArraySize;

    union
    {
        int     normalizeSize;         // 見本DB内の座標値の正規化サイズ
        int     regularizeSize;        // 旧名
    };
}
PRCPrototypeList;

/*---------------------------------------------------------------------------*
  リサンプリング手法
 *---------------------------------------------------------------------------*/
typedef enum PRCResampleMethod
{
    PRC_RESAMPLE_METHOD_NONE = 0,
    PRC_RESAMPLE_METHOD_DISTANCE,
    PRC_RESAMPLE_METHOD_ANGLE,
    PRC_RESAMPLE_METHOD_RECURSIVE,
    PRC_RESAMPLE_METHOD_USER = 256
}
PRCResampleMethod;


#ifdef __cplusplus
} /* extern "C" */
#endif

/* NITRO_PRC_TYPES_H_ */
#endif
