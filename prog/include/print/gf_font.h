//=============================================================================================
/**
 * @file	gf_font.h
 * @brief	フォントデータマネージャ
 * @author	taya
 * @date	2005.09.14
 *
 */
//=============================================================================================
#ifndef __GF_FONT_H__
#define __GF_FONT_H__


#include <heapsys.h>
#include <arc_tool.h>

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
	GFL_FONT_LOADTYPE_FILE,		///< 必要な文字のみ、逐次ファイルから読み出す
	GFL_FONT_LOADTYPE_MEMORY,	///< 全文字をメモリに常駐させておき、そこから読み出す
}GFL_FONT_LOADTYPE;


//=============================================================================================
/**
 * フォントデータハンドラ作成
 *
 * @param   arcID			フォントデータが含まれるアーカイブID
 * @param   datID			フォントデータのアーカイブ内ID
 * @param   loadType		フォントデータ読み出し方式
 * @param   fixedFontFlag	等幅フォントとして扱うためのフラグ（TRUEなら等幅）
 * @param   heapID			ハンドラ生成用ヒープID
 *
 * @retval  GFL_FONT*		フォントデータハンドラ
 */
//=============================================================================================
extern GFL_FONT* GFL_FONT_Create( u32 arcID, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL fixedFontFlag, HEAPID heapID );

//=============================================================================================
/**
 * フォントデータハンドラ削除
 *
 * @param   wk			フォントデータハンドラ
 */
//=============================================================================================
extern void GFL_FONT_Delete( GFL_FONT* wk );

//=============================================================================================
/**
 * フォントBitmapデータ取得
 *
 * @param   wk		
 * @param   index		
 * @param   dst		
 * @param   sizeX		
 * @param   sizeY		
 *
 */
//=============================================================================================
extern void GFL_FONT_GetBitMap( const GFL_FONT* wk, u32 index, void* dst, u16* sizeX, u16* sizeY );


extern u16 GFL_FONT_GetWidth( const GFL_FONT* wk, u32 index );
extern u16 GFL_FONT_GetHeight( const GFL_FONT* wk, u32 index );


extern void GFL_FONTSYS_Init( void );
extern void GFL_FONTSYS_SetDefaultColor( void );
extern void GFL_FONTSYS_SetColor( u8 letterColor, u8 shadowColor, u8 backColor );
extern void GFL_FONTSYS_GetColor( u8* letter, u8* shadow, u8* back );


#endif
