/*---------------------------------------------------------------------------*
  Project:  TwlSDK - GX - demos - UnitTours/DEMOLib
  File:     DEMOBitmap.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17 #$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef DEMO_BITMAP_H_
#define DEMO_BITMAP_H_

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------------------------------*/
/* constants */

#define DEMO_RGB_NONE   GX_RGBA(31, 31, 31, 0)
#define DEMO_RGB_CLEAR  GX_RGBA( 0,  0,  0, 0)

extern const u32 DEMOAsciiChr[8 * 0x100];


/*---------------------------------------------------------------------------*/
/* fundtions */

// 条件演算子に対する利便性を考慮した簡易版。
#define DEMOSetBitmapTextColor(color)       DEMOiSetBitmapTextColor(DEMOVerifyGXRgb(color))
#define DEMOSetBitmapGroundColor(color)     DEMOiSetBitmapGroundColor(DEMOVerifyGXRgb(color))
#define DEMOFillRect(x, y, wx, wy, color)   DEMOiFillRect(x, y, wx, wy, DEMOVerifyGXRgb(color))
#define DEMODrawLine(sx, sy, tx, ty, color) DEMOiDrawLine(sx, sy, tx, ty, DEMOVerifyGXRgb(color))
#define DEMODrawFrame(x, y, wx, wy, color)  DEMOiDrawFrame(x, y, wx, wy, DEMOVerifyGXRgb(color))


/*---------------------------------------------------------------------------*
  Name:         DEMOInitDisplayBitmap

  Description:  ビットマップ描画モードでDEMOを初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOInitDisplayBitmap(void);

/*---------------------------------------------------------------------------*
  Name:         DEMO_DrawFlip

  Description:  現在の描画内容をVRAMへ反映する

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMO_DrawFlip();

/*---------------------------------------------------------------------------*
  Name:         DEMOHookConsole

  Description:  OS_PutStringをフックしてログにも出力するように設定

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOHookConsole(void);

/*---------------------------------------------------------------------------*
  Name:         DEMOVerifyGXRgb

  Description:  色値の正当性チェック。

  Arguments:    color : GXRgbの範囲内と思われる色値

  Returns:      正当なGXRgbならcolorをそのまま返し、範囲外なら0xFFFF
 *---------------------------------------------------------------------------*/
GXRgb DEMOVerifyGXRgb(int color);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetBitmapTextColor

  Description:  ビットマップ描画のテキスト色を設定

  Arguments:    color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiSetBitmapTextColor(GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetBitmapGroundColor

  Description:  ビットマップ描画の背景色を設定

  Arguments:    color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiSetBitmapGroundColor(GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOFillRect

  Description:  ビットマップに矩形を描画

  Arguments:    x     : 描画 X 座標
                y     : 描画 Y 座標
                wx    : 描画 X 幅
                wy    : 描画 Y 幅
                color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiFillRect(int x, int y, int wx, int wy, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMOBlitRect

  Description:  ビットマップに矩形を転送

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                wx     : 描画 X 幅
                wy     : 描画 Y 幅
                image  : 転送元のイメージ
                stroke : 転送元の1ラインあたりのピクセル数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOBlitRect(int x, int y, int wx, int wy, const GXRgb *image, int stroke);

/*---------------------------------------------------------------------------*
  Name:         DEMOBlitTex16

  Description:  ビットマップに16色テクスチャを転送

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                wx     : 描画 X 幅 (8ピクセルの整数倍である必要がある)
                wy     : 描画 Y 幅 (8ピクセルの整数倍である必要がある)
                chr    : キャラクタイメージ (4x8x8の1次元キャラクタフォーマット)
                plt    : パレットイメージ (インデックス0は透明扱い)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOBlitTex16(int x, int y, int wx, int wy, const void *chr, const GXRgb *plt);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawLine

  Description:  ビットマップに線を描画

  Arguments:    sx     : 始点 X 座標
                sy     : 始点 Y 座標
                tx     : 終点 X 幅
                ty     : 終点 Y 幅
                color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiDrawLine(int sx, int sy, int tx, int ty, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawFrame

  Description:  ビットマップに枠を描画

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                wx     : 描画 X 幅
                wy     : 描画 Y 幅
                color : 設定する色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOiDrawFrame(int x, int y, int wx, int wy, GXRgb color);

/*---------------------------------------------------------------------------*
  Name:         DEMODrawText

  Description:  ビットマップに文字列を描画

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                format : 書式文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMODrawText(int x, int y, const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOClearString

  Description:  BGテキストを全て消去

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOClearString(void);

/*---------------------------------------------------------------------------*
  Name:         DEMOPutString

  Description:  BGテキストを描画

  Arguments:    x      : 描画 X 座標
                y      : 描画 Y 座標
                format : 書式文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOPutString(int x, int y, const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOPutLog

  Description:  サブ画面にログ文字列を表示

  Arguments:    format : 書式文字列

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOPutLog(const char *format, ...);

/*---------------------------------------------------------------------------*
  Name:         DEMOSetViewPort

  Description:  ビューポートとProjectionを設定

  Arguments:    x      : 左上 X 座標
                y      : 左上 Y 座標
                wx     : ビューポート X 幅
                wy     : ビューポート Y 幅

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEMOSetViewPort(int x, int y, int wx, int wy);


#ifdef __cplusplus
}/* extern "C" */
#endif

/* DEMO_BITMAP_H_ */
#endif
