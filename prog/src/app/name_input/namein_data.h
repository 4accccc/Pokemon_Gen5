//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		namein_data.h
 *	@brief	２種類のデータ読みこみ
 *
 *					１．名前入力用データ
 *					入っているデータは、
 *					キーボードで入力するときの文字です。
 *		
 *					かなならば以下
 *
 *					あか…
 *					いき…
 *					うく…
 *					えけ…
 *					おこ…
 *
 *					左上が原点で、右下に＋です。
 *					上記例で、GetStr( wk, 1, 3 )した場合、「け」を取得します
 *
 *					
 *					２．変換用データ
 *					以下のデータが入っている
 *					・変換される文字列
 *					・変換後の文字列
 *					・SHIFTを押したときの変換後の文字列
 *					・変換後、変換バッファに残る文字列
 *
 *
 *	@author	Toru=Nagihashi
 *	@date		2009.10.08
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *					キー配列データの変数
*/
//=============================================================================
//-------------------------------------
///		読み込むタイプ
//=====================================
typedef enum
{
	NAMEIN_KEYMAPTYPE_KANA,
	NAMEIN_KEYMAPTYPE_KATA,
	NAMEIN_KEYMAPTYPE_ABC,
	NAMEIN_KEYMAPTYPE_KIGOU,
	NAMEIN_KEYMAPTYPE_QWERTY,
	NAMEIN_KEYMAPTYPE_MAX,
} NAMEIN_KEYMAPTYPE;

//=============================================================================
/**
 *					キー配列データの構造体宣言
*/
//=============================================================================
//-------------------------------------
///	名前入力データワーク
//=====================================
typedef struct _NAMEIN_KEYMAP NAMEIN_KEYMAP;

//-------------------------------------
///	名前入力データワークハンドル
//=====================================
typedef struct _NAMEIN_KEYMAP_HANDLE NAMEIN_KEYMAP_HANDLE;

//=============================================================================
/**
 *					キー配列データの外部公開
*/
//=============================================================================
//データ読みこみ
extern NAMEIN_KEYMAP *NAMEIN_KEYMAP_Alloc( NAMEIN_KEYMAPTYPE type, HEAPID heapID );
extern void NAMEIN_KEYMAP_Free( NAMEIN_KEYMAP *p_wk );

//メモリに全データをおいて、読み込む版
extern NAMEIN_KEYMAP_HANDLE * NAMEIN_KEYMAP_HANDLE_Alloc( HEAPID heapID );
extern void NAMEIN_KEYMAP_HANDLE_Free( NAMEIN_KEYMAP_HANDLE *p_handle );
extern NAMEIN_KEYMAP *NAMEIN_KEYMAP_HANDLE_GetData( NAMEIN_KEYMAP_HANDLE *p_handle, NAMEIN_KEYMAPTYPE type );

//データ取得
extern u16 NAMEIN_KEYMAP_GetWidth( const NAMEIN_KEYMAP *cp_wk );
extern u16 NAMEIN_KEYMAP_GetHeight( const NAMEIN_KEYMAP *cp_wk );
extern STRCODE NAMEIN_KEYMAP_GetStr( const NAMEIN_KEYMAP *cp_wk, u16 x, u16 y );


//=============================================================================
/**
 *					変換データの変数
*/
//=============================================================================
//-------------------------------------
///		読み込むタイプ
//=====================================
typedef enum
{
	NAMEIN_STRCHANGETYPE_DAKUTEN,
	NAMEIN_STRCHANGETYPE_HANDAKUTEN,
	NAMEIN_STRCHANGETYPE_QWERTY,
	NAMEIN_STRCHANGETYPE_MAX,
} NAMEIN_STRCHANGETYPE;

//EOMをのぞく文字最大数（ここの定義は、namein_change_conv.pl内と同じ）
#define NAMEIN_STRCHANGE_CODE_LEN	(3)

//ないばあいの返り値
#define NAMEIN_STRCHANGE_NONE			(0xFFFF)

//=============================================================================
/**
 *					変換の構造体宣言
*/
//=============================================================================
//-------------------------------------
///	名前入力データワーク
//=====================================
typedef struct _NAMEIN_STRCHANGE NAMEIN_STRCHANGE;

//=============================================================================
/**
 *					変換の外部公開
*/
//=============================================================================
//データ読みこみ
extern NAMEIN_STRCHANGE *NAMEIN_STRCHANGE_Alloc( NAMEIN_STRCHANGETYPE type, HEAPID heapID );
extern void NAMEIN_STRCHANGE_Free( NAMEIN_STRCHANGE *p_wk );

//データ取得
extern u16 NAMEIN_STRCHANGE_GetNum( const NAMEIN_STRCHANGE *cp_wk );
extern void NAMEIN_STRCHANGE_GetInputStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len );
extern BOOL NAMEIN_STRCHANGE_GetChangeStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_len, u8 *p_len );
extern BOOL NAMEIN_STRCHANGE_GetChangeShiftStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_le, u8 *p_len );
extern BOOL NAMEIN_STRCHANGE_GetRestStr( const NAMEIN_STRCHANGE *cp_wk, u16 idx, STRCODE *p_code, u8 code_le, u8 *p_len );
extern u16 NAMEIN_STRCHANGE_GetIndexByInputStr( const NAMEIN_STRCHANGE *cp_wk, const STRCODE *cp_code );
