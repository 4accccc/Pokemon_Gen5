//=============================================================================================
/**
 * @file  gf_font.h
 * @brief フォントデータマネージャ
 * @author  taya
 * @date  2005.09.14
 *
 */
//=============================================================================================
#ifndef __GF_FONT_H__
#define __GF_FONT_H__


#include <heapsys.h>
#include <arc_tool.h>
#include <strbuf.h>

//--------------------------------------------------------------
/**
 * フォントデータハンドラ型定義
 */
//--------------------------------------------------------------
typedef struct _GFL_FONT  GFL_FONT;


//--------------------------------------------------------------
/**
 * フォントデータの読み出し方式
 */
//--------------------------------------------------------------
typedef enum {
  GFL_FONT_LOADTYPE_FILE,   ///< 必要な文字のみ、逐次ファイルから読み出す
  GFL_FONT_LOADTYPE_MEMORY, ///< 全文字をメモリに常駐させておき、そこから読み出す
}GFL_FONT_LOADTYPE;



//=============================================================================================
/**
 * システム初期化（プログラム起動後に１回のみ）
 */
//=============================================================================================
extern void GFL_FONTSYS_Init( void );

//=============================================================================================
/**
 * フォントデータハンドラ作成
 *
 * @param   arcID           フォントデータが含まれるアーカイブID
 * @param   datID           フォントデータのアーカイブ内ID
 * @param   loadType        フォントデータ読み出し方式
 * @param   monoSpaceFlag   等幅フォントとして扱うためのフラグ（TRUEなら等幅）
 * @param   heapID          ハンドラ生成用ヒープID
 *
 * @retval  GFL_FONT*   フォントデータハンドラ
 */
//=============================================================================================
extern GFL_FONT* GFL_FONT_Create( u32 arcID, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL monoSpaceFlag, HEAPID heapID );

//=============================================================================================
/**
 * フォントデータハンドラ削除
 *
 * @param   wk      フォントデータハンドラ
 */
//=============================================================================================
extern void GFL_FONT_Delete( GFL_FONT* wk );


extern void GFL_FONTSYS_SetDefaultColor( void );
extern void GFL_FONTSYS_SetColor( u8 letterColor, u8 shadowColor, u8 backColor );
extern void GFL_FONTSYS_GetColor( u8* letter, u8* shadow, u8* back );
extern BOOL GFL_FONTSYS_IsDifferentColor( u8 letter, u8 shadow, u8 back );


//=========================================================================
//-----------------------------------
/**
 *  文字ドットサイズ情報
 */
//-----------------------------------
typedef struct {
  u8 left_width;    ///< 左側余白部分の幅
  u8 glyph_width;   ///< 文字部分の幅
  u8 width;     ///< 余白、文字部分の合計幅
  u8 height;      ///< 高さ
}GFL_FONT_SIZE;

extern void GFL_FONT_GetBitMap( const GFL_FONT* wk, STRCODE code, void* dst, GFL_FONT_SIZE* size );
extern u16 GFL_FONT_GetWidth( const GFL_FONT* wk, STRCODE code );
extern u16 GFL_FONT_GetLineHeight( const GFL_FONT* wk );


#endif
