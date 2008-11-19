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


//==============================================================================================
/**
 * フォントデータマネージャ作成
 *
 * @param   arcHandle		フォントデータを含むアーカイブハンドル
 * @param   datID			フォントデータが格納されているアーカイブ内ファイルID
 * @param   loadType		フォントデータの読み出し方式
 * @param   fixedFontFlag	TRUEなら等幅フォントとして扱う
 * @param   heapID			マネージャ作成先ヒープID
 *
 * @retval  GFL_FONT_HANDLE_MAN*	フォントデータマネージャポインタ
 *
 * @li  loadType が FONTDATA_LOADTYPE_ON_MEMORY の場合、マネージャと同じヒープ領域にフォントデータを読み込む
 */
//==============================================================================================
extern GFL_FONT* GFL_FONT_CreateHandle( ARCHANDLE* arcHandle, u32 datID, GFL_FONT_LOADTYPE loadType, BOOL fixedFontFlag, HEAPID heapID );


//==============================================================================================
/**
 * フォントデータマネージャ削除
 *
 * @param   wk		フォントデータマネージャのポインタ
 *
 */
//==============================================================================================
extern void FontDataMan_Delete( GFL_FONT* wk );


//==============================================================================================
/**
 * フォントビットデータの読み込みタイプを変更する
 *
 * @param   wk			フォントデータマネージャポインタ
 * @param   loadType	変更後の読み込みタイプ
 * @param   heapID		常駐タイプに変更の場合、フォントデータを読み込むヒープとして使用する。
 *
 */
//==============================================================================================
extern void FontDataMan_ChangeLoadType( GFL_FONT* wk, GFL_FONT_LOADTYPE loadType, HEAPID heapID );


//------------------------------------------------------------------
/*
 *	文字ビットマップデータ取得
 *
 * @param	wk			フォントデータマネージャ
 * @param	index		文字コード
 * @param	dst			ビットマップ取得ワーク
 *
 */
//------------------------------------------------------------------
extern void FontDataMan_GetBitmap( const GFL_FONT* wk, u32 index, void* dst );


extern void GFL_FONT_GetBitMap( const GFL_FONT* wk, u32 index, void* dst, u16* sizeX, u16* sizeY );
extern u16 GFL_FONT_GetWidth( const GFL_FONT* wk, u32 index );
extern u16 GFL_FONT_GetHeight( const GFL_FONT* wk, u32 index );


extern void GFL_FONTSYS_Init( void );
extern void GFL_FONTSYS_SetDefaultColor( void );
extern void GFL_FONTSYS_SetColor( u8 letterColor, u8 shadowColor, u8 backColor );
extern void GFL_FONTSYS_GetColor( u8* letter, u8* shadow, u8* back );


#if 0
//------------------------------------------------------------------
/**
 * 文字列をビットマップ化した時の長さ（ドット）を計算して返す
 *
 * @param   wk		フォントデータマネージャ
 * @param   str		文字列
 * @param   margin	字間（ドット）
 *
 * @retval  u32		長さ
 */
//------------------------------------------------------------------
extern u32 FontDataMan_GetStrWidth( const GFL_FONT* wk, const STRCODE* str, u32 margin );
#endif

#endif
