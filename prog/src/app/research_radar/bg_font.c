/////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  BmpWin + 文字列描画 パッケージオブジェクト
 * @file   bg_font.h
 * @author obata
 * @date   2010.02.09
 */
///////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h>
#include "bg_font.h"
#include "print/printsys.h"  // for PRINTSYS_xxxx


//===============================================================================
// ■BG_FONT
//===============================================================================
struct _BG_FONT
{
  BG_FONT_PARAM param;    // 基本パラメータ
  GFL_FONT*     font;     // 使用するフォント
  GFL_MSGDATA*  message;  // 参照するメッセージデータ
  GFL_BMPWIN*   bmpWin;   // ビットマップウィンドウ
};


//===============================================================================
// ■プロトタイプ宣言
//===============================================================================


//-------------------------------------------------------------------------------
/**
 * @brief BG_FONT オブジェクトを生成する
 *
 * @param param   生成オブジェクトのパラメータ
 * @param font    使用するフォント
 * @param message 参照するメッセージデータ
 * @param heapID  使用するヒープID
 *
 * @return 生成した BG_FONT オブジェクト
 */
//-------------------------------------------------------------------------------
BG_FONT* BG_FONT_Create( const BG_FONT_PARAM* param, 
                         GFL_FONT* font, GFL_MSGDATA* message, HEAPID heapID )
{
  BG_FONT* BGFont;

  // 生成
  BGFont = GFL_HEAP_AllocMemory( heapID, sizeof(BG_FONT) );

  // 初期化
  BGFont->param   = *param;
  BGFont->font    = font;
  BGFont->message = message;
  BGFont->bmpWin  = GFL_BMPWIN_Create( param->BGFrame, 
                                       param->posX, param->posY, 
                                       param->sizeX, param->sizeY,
                                       param->paletteNo,
                                       GFL_BMP_CHRAREA_GET_F ); 
  return BGFont;
}

//-------------------------------------------------------------------------------
/**
 * @brief BG_FONT オブジェクトを破棄する
 *
 * @param BGFont
 */
//-------------------------------------------------------------------------------
void BG_FONT_Delete( BG_FONT* BGFont )
{
  GF_ASSERT( BGFont );
  GF_ASSERT( BGFont->bmpWin );

  GFL_BMPWIN_Delete( BGFont->bmpWin );
  GFL_HEAP_FreeMemory( BGFont );
}

//-------------------------------------------------------------------------------
/**
 * @brief 文字列を設定する
 *
 * @param BGFont
 * @param strID  書き込む文字列ID
 */
//-------------------------------------------------------------------------------
void BG_FONT_SetMessage( BG_FONT* BGFont, u32 strID )
{
  STRBUF* strbuf;

  // 文字列を取得
  strbuf = GFL_MSG_CreateString( BGFont->message, strID );

  // 書き込み
  BG_FONT_SetString( BGFont, strbuf );

  GFL_HEAP_FreeMemory( strbuf );
}

//-------------------------------------------------------------------------------
/**
 * @brief 文字列を設定する
 *
 * @param BGFont
 * @param strbuf 書き込む文字列
 */
//------------------------------------------------------------------------------- 
void BG_FONT_SetString( BG_FONT* BGFont, const STRBUF* strbuf )
{
  BG_FONT_PARAM* param;
  GFL_BMP_DATA*  bmpData;
  PRINTSYS_LSB   color;

  param   = &(BGFont->param);
  color   = PRINTSYS_LSB_Make( param->colorNo_L, param->colorNo_S, param->colorNo_B );
  bmpData = GFL_BMPWIN_GetBmp( BGFont->bmpWin );

  // クリア
  GFL_BMP_Clear( bmpData, BGFont->param.colorNo_B );

  // 書き込み
  PRINTSYS_PrintColor( bmpData, 
                       param->offsetX, param->offsetY, 
                       strbuf, BGFont->font, color ); 
  // VRAMへ転送
  GFL_BMPWIN_MakeTransWindow( BGFont->bmpWin );
}

//-------------------------------------------------------------------------------
/**
 * @brief 表示・非表示を設定する
 *
 * @param BGFont
 * @param enable 表示する場合TRUE, 非表示にする場合FALSE
 */
//-------------------------------------------------------------------------------
void BG_FONT_SetDrawEnable( BG_FONT* BGFont, BOOL enable )
{
  if( enable ) {
    // スクリーンを作成
    GFL_BMPWIN_MakeScreen( BGFont->bmpWin );
  }
  else {
    // スクリーンをクリア
    GFL_BMPWIN_ClearScreen( BGFont->bmpWin );
  }

  // VRAMへ転送
  GFL_BG_LoadScreenReq( GFL_BMPWIN_GetFrame( BGFont->bmpWin ) );
}
