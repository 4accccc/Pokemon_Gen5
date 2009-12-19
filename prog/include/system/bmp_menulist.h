//======================================================================
/**
 * @file	bmp_list.h	
 * @brief	ビットマップＢＧ上でのリスト表示システム
 * @author	GAME FREAK inc.
 * @date	tetsu
 *
 * ・2008.09 DPからWBへ移植
 */
//======================================================================
#ifndef __BMP_LIST_H__
#define __BMP_LIST_H__

#include <tcbl.h>
#include "print\printsys.h"
#include "print\gf_font.h"
#include "bmp_menuwork.h"

/********************************************************************/
/*                                                                  */
/*				使用定義											*/
/*                                                                  */
/********************************************************************/
#define	BMPMENULIST_MAX		( 4 )		// 登録最大数（適当）

typedef struct _BMPMENULIST_WORK	BMPMENULIST_WORK;

typedef void	(*BMPMENULIST_CURSOR_CALLBACK)(BMPMENULIST_WORK * wk,u32 param,u8 mode);
typedef void	(*BMPMENULIST_PRINT_CALLBACK)(BMPMENULIST_WORK * wk,u32 param,u8 y);

///リストヘッダ構造体
typedef struct {
	//表示文字データポインタ
	const BMP_MENULIST_DATA *list;
	//カーソル移動ごとのコールバック関数
	BMPMENULIST_CURSOR_CALLBACK call_back;
	//一列表示ごとのコールバック関数
	BMPMENULIST_PRINT_CALLBACK icon;

	GFL_BMPWIN *win;
	
	u16		count;		//リスト項目数
	u16		line;		//表示最大項目数
	u8		rabel_x;	//ラベル表示Ｘ座標
	u8		data_x;		//項目表示Ｘ座標
	u8		cursor_x;	//カーソル表示Ｘ座標
	u8		line_y:4;	//表示Ｙ座標
	u8		f_col:4;	//表示文字色
	u8		b_col:4;	//表示背景色
	u8		s_col:4;	//表示文字影色
	u16		msg_spc:3;	//文字間隔Ｘ
	u16		line_spc:4;	//文字間隔Ｙ
	u16		page_skip:2;//ページスキップタイプ
	u16		font:6;	//文字指定(本来はu8だけどそんなに作らないと思うので)
	u16		c_disp_f:1;	//ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
	void * work;
	
	u16 font_size_x;		//文字サイズX(ドット
	u16 font_size_y;		//文字サイズY(ドット
	GFL_MSGDATA *msgdata;	//表示に使用するメッセージバッファ
//	STRBUF *strbuf;			//表示に使用するメッセージバッファ
	PRINT_UTIL *print_util; //表示に使用するプリントユーティリティ
	PRINT_QUE *print_que;	//表示に使用するプリントキュー
	GFL_FONT *font_handle;	//表示に使用するフォントハンドル
}BMPMENULIST_HEADER;

///リストヘッダ定義
#define	BMPMENULIST_NO_SKIP		(0)
#define	BMPMENULIST_LRKEY_SKIP	(1)
#define	BMPMENULIST_LRBTN_SKIP	(2)

///リストアトリビュート構造体
typedef struct {
	u8		pos_x;		//開始Ｘ座標オフセット(キャラ単位)
	u8		pos_y;		//開始Ｙ座標オフセット(キャラ単位)
	u8		siz_x;		//Ｘサイズ(キャラ単位)
	u8		siz_y;		//Ｙサイズ(キャラ単位)
	u8		pal;		//指定パレット
}BMPMENULIST_ATTR;

///リストパラメータ＆返り値定義
#define	BMPMENULIST_NULL	(0xffffffff)
#define	BMPMENULIST_CANCEL	(0xfffffffe)
#define	BMPMENULIST_RABEL	(0xfffffffd)

// パラメータ取得・変更ＩＤ
enum {
	BMPMENULIST_ID_CALLBACK = 0,///<カーソル移動ごとのコールバック関数
	BMPMENULIST_ID_ICONFUNC,	///<一列表示ごとのコールバック関数
	BMPMENULIST_ID_COUNT,		///<リスト項目数
	BMPMENULIST_ID_LINE,		///<表示最大項目数
	BMPMENULIST_ID_INDEX,		///<使用ビットマップＩＮＤＥＸ
	BMPMENULIST_ID_RABEL_X,		///<ラベル表示Ｘ座標
	BMPMENULIST_ID_DATA_X,		///<項目表示Ｘ座標
	BMPMENULIST_ID_CURSOR_X,	///<カーソル表示Ｘ座標
	BMPMENULIST_ID_LINE_Y,		///<表示Ｙ座標
	BMPMENULIST_ID_LINE_YBLK,   ///<１行分シフト量取得(フォントの大きさ＋Ｙ間隔)
	BMPMENULIST_ID_F_COL,		///<表示文字色
	BMPMENULIST_ID_B_COL,		///<表示背景色
	BMPMENULIST_ID_S_COL,		///<表示文字影色
	BMPMENULIST_ID_MSG_SPC,		///<文字間隔Ｘ
	BMPMENULIST_ID_LINE_SPC,	///<文字間隔Ｙ
	BMPMENULIST_ID_PAGE_SKIP,	///<ページスキップタイプ
	BMPMENULIST_ID_FONT,		///<文字指定
	BMPMENULIST_ID_C_DISP_F,	///<ＢＧカーソル(allow)表示フラグ
	BMPMENULIST_ID_WIN,			///<ウィンドウデータ
	BMPMENULIST_ID_WORK			///<ワーク
};

// 移動方向
enum {
	BMPMENULIST_MOVE_NONE = 0,	// 動作なし
	BMPMENULIST_MOVE_UP,		// 上へ移動
	BMPMENULIST_MOVE_DOWN,		// 下へ移動
	BMPMENULIST_MOVE_UP_SKIP,	// 上へ移動（スキップ）
	BMPMENULIST_MOVE_DOWN_SKIP,	// 下へ移動（スキップ）
};

//キャンセルモード
typedef enum {
  BMPMENULIST_CANCELMODE_USE, //Bキャンセル使う
  BMPMENULIST_CANCELMODE_NOT, //Bキャンセル無効
}BMPMENULIST_CANCELMODE;

/********************************************************************/
/**
 *					簡易リスト関数
 *
 * @param	bmpdata		リスト用ビットマップデータポインタ
 * @param	bmplist		表示リストヘッダポインタ
 * @param	mode		標準ウインドウ枠表示スイッチ
 *							EASYBMPMENULIST_NO_FRAME	枠なし
 *							EASYBMPMENULIST_SCRFRAME	枠あり、キャラ転送なし
 *							EASYBMPMENULIST_ALLFRAME	枠、キャラ転送あり
 * @param	cgx			枠ＣＧＸ転送オフセット
 * @param	pal			枠ＰＡＬ転送オフセット(palanm.h の定義に準拠)
 *
 * @retval	param		BMPMENULIST_NULLでなければ、選択された項目パラメータ
 */
/********************************************************************/
#define	EASYBMPMENULIST_NO_FRAME	(0)
#define	EASYBMPMENULIST_SCRFRAME	(1)
#define	EASYBMPMENULIST_ALLFRAME	(2)

/*u32	BmpListEasy
(const BMPWIN_DAT *bmpdata,const BMPMENULIST_HEADER *bmplist,u8 mode,u16 cgx,u16 pal);
*/

/********************************************************************/
/**
 *					設定関数（拡張）
 *
 * @param	bmplist		表示リストヘッダポインタ
 * @param	bmplist		表示リストパレットアトリビュートポインタ
 * @param	list_p		リスト初期位置
 * @param	cursor_p	カーソル初期位置
 *
 * @retval	id			リストＩＤ(=work_id)
 *						※メイン関数の引数となるので管理を忘れずに
 *
 * call_back 設定に関して、この関数からの引数は 
 *   BMPMENULIST_DATA 構造体 param
 * となる
 */
/********************************************************************/
/*
extern u8		BmpListSetExpand
	(const BMPMENULIST_HEADER* bmplist,const BMPMENULIST_ATTR* bmpattr,u16 list_p,u16 cursor_p);
*/

//--------------------------------------------------------------------------------------------
/**
 * 設定関数
 *
 * @param	bmplist		表示リストヘッダポインタ
 * @param	list_p		リスト初期位置
 * @param	cursor_p	カーソル初期位置
 * @param	mode		メモリ取得モード
 *
 * @return	BMPリストワーク
 *
 * @li	BMPリストワークはsys_AllocMemoryで確保
 */
//--------------------------------------------------------------------------------------------
extern BMPMENULIST_WORK * BmpMenuList_Set(
						const BMPMENULIST_HEADER * bmplist, u16 list_p, u16 cursor_p, HEAPID heapID );

//--------------------------------------------------------------------------------------------
/**
 * メイン関数
 *
 * @param	lw		BMPリストワーク
 *
 * @retval	"param = 選択パラメータ"
 * @retval	"BMPMENULIST_NULL = 選択中"
 * @retval	"BMPMENULIST_CANCEL	= キャンセル(Ｂボタン)"
 */
//--------------------------------------------------------------------------------------------
extern u32 BmpMenuList_Main( BMPMENULIST_WORK * lw );

//--------------------------------------------------------------------------------------------
/**
 * 終了関数
 *
 * @param	lw			BMPリストワーク
 * @param	list_bak	リスト位置バックアップワークポインタ
 * @param	cursor_bak	カーソル位置バックアップワークポインタ
 *
 * @return	none
 *
 * @li	lwをsys_FreeMemoryで開放
 * @li	バックアップワークは、NULL指定で保存しない
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_Exit( BMPMENULIST_WORK * lw, u16 * list_bak, u16 * cursor_bak );

//--------------------------------------------------------------------------------------------
/**
 * リスト再描画
 *
 * @param	lw		BMPリストワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_Rewrite( BMPMENULIST_WORK * lw );

//--------------------------------------------------------------------------------------------
/**
 * 文字色変更関数
 *
 * @param	lw			BMPリストワーク
 * @param	f_col		文字色
 * @param	b_col		背景色
 * @param	s_col		影色
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_ColorControl( BMPMENULIST_WORK * lw, u8 f_col, u8 b_col, u8 s_col );

//--------------------------------------------------------------------------------------------
/**
 * 表示位置変更関数
 *
 * @param	lw			BMPリストワーク
 * @param	x			表示Ｘ座標(キャラ単位)
 * @param	y			表示Ｙ座標(キャラ単位)
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_MoveControl( BMPMENULIST_WORK * lw, u8 x, u8 y );

//--------------------------------------------------------------------------------------------
/**
 * 外部コントロール関数(描画・コールバック指定)
 *
 * @param	lw			BMPリストワーク
 * @param	lh			表示リストヘッダポインタ
 * @param	list_p		リスト初期位置
 * @param	cursor_p	カーソル初期位置
 * @param	print		描画フラグ
 * @param	direct		指定方向(キー定義)
 * @param	list_bak	リスト位置バックアップワークポインタ
 * @param	cursor_bak	カーソル位置バックアップワークポインタ
 *
 * @return	BMPMENULIST_NULL	
 *
 * リスト関数が動作していないとき、外部からの指定により
 * カーソル位置の変動をチェックし、指定バックアップワークに保存
 */
//--------------------------------------------------------------------------------------------
extern u32 BmpMenuList_MainOutControlEx(
			BMPMENULIST_WORK * lw, BMPMENULIST_HEADER * bmplist,
			u16 list_p, u16 cursor_p, u16 print, u16 direct, u16 * list_bak, u16 * cursor_bak );

//--------------------------------------------------------------------------------------------
/**
 * 外部コントロール関数(カーソル移動判定)
 *
 * @param	lw			BMPリストワーク
 * @param	bmplist		表示リストヘッダポインタ
 * @param	list_p		リスト初期位置
 * @param	cursor_p	カーソル初期位置
 * @param	direct		指定方向(キー定義)
 * @param	list_bak	リスト位置バックアップワークポインタ
 * @param	cursor_bak	カーソル位置バックアップワークポインタ
 *
 * @return	BMPMENULIST_NULL	
 *
 * リスト関数が動作していないとき、外部からの指定により
 * カーソル位置の変動をチェックし、指定バックアップワークに保存
 */
//--------------------------------------------------------------------------------------------
extern u32 BmpMenuList_MainOutControl(
				BMPMENULIST_WORK * lw, BMPMENULIST_HEADER * bmplist,
				u16 list_p, u16 cursor_p, u16 direct, u16 * list_bak, u16 * cursor_bak );

//--------------------------------------------------------------------------------------------
/**
 * BMP_MENULIST_DATAを再度設定する
 *
 * @param	BMP_MENULIST_DATA			BMPMENUリストポインタ
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_SetMenuListData(BMPMENULIST_WORK * lw, BMP_MENULIST_DATA* pList);

//--------------------------------------------------------------------------------------------
/**
 * 文字色一時変更
 *
 * @param	lw		BMPリストワーク
 * @param	f_col	文字色
 * @param	b_col	背景色
 * @param	s_col	影色
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_TmpColorChange( BMPMENULIST_WORK * lw, u8 f_col, u8 b_col, u8 s_col );

//--------------------------------------------------------------------------------------------
/**
 * BMPリストの全体位置を取得
 *
 * @param	lw			BMPリストワーク
 * @param	pos_bak		全体リスト位置バックアップワークポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_DirectPosGet( BMPMENULIST_WORK * lw, u16 * pos_bak );

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのリスト位置、カーソル位置を取得
 *
 * @param	lw			BMPリストワーク
 * @param	list_bak	リスト位置バックアップワークポインタ
 * @param	cursor_bak	カーソル位置バックアップワークポインタ
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_PosGet( BMPMENULIST_WORK * lw, u16 * list_bak, u16 * cursor_bak );

//----------------------------------------------------------------------------
/**
 * BMPリストのリスト位置、カーソル位置を設定
 *
 * @param	lw			BMPリストワーク
 * @param	list		リスト位置
 * @param	cursor	カーソル位置
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
extern void BmpMenuList_PosSet( BMPMENULIST_WORK * lw, u16 list, u16 cursor );
//--------------------------------------------------------------------------------------------
/**
 * BMPリストのカーソルY座標を取得
 *
 * @param	lw		BMPリストワーク
 *
 * @return	カーソルY座標
 */
//--------------------------------------------------------------------------------------------
extern u16 BmpMenuList_CursorYGet( BMPMENULIST_WORK * lw );

//--------------------------------------------------------------------------------------------
/**
 * BMPリストのカーソル移動方向取得
 *
 * @param	lw		BMPリストワーク
 *
 * @return	カーソル移動方向取得
 */
//--------------------------------------------------------------------------------------------
extern u8 BmpMenuList_MoveSiteGet( BMPMENULIST_WORK * lw );

//--------------------------------------------------------------------------------------------
/**
 * 指定位置のパラメータ取得
 *
 * @param	lw		BMPリストワーク
 * @param	pos		位置
 *
 * @return	パラメータ
 */
//--------------------------------------------------------------------------------------------
extern u32 BmpMenuList_PosParamGet( BMPMENULIST_WORK * lw, u16 pos );

//--------------------------------------------------------------------------------------------
/**
 * パラメータ取得関数
 *
 * @param	lw		BMPリストワーク
 * @param	mode	パラメータ取得ＩＤ
 *
 * @retval	"0xffffffff = エラー"
 * @retval	"0xffffffff != パラメータ"
 */
//--------------------------------------------------------------------------------------------
extern u32	BmpListParamGet( BMPMENULIST_WORK * lw, u8 mode );

//--------------------------------------------------------------------------------------------
/**
 * パラメータ変更関数
 *
 * @param	lw		BMPリストワーク
 * @param	mode	パラメータ取得ＩＤ
 * @param	param	変更値
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenuList_ParamSet( BMPMENULIST_WORK * lw, u8 mode, u32 param );

//--------------------------------------------------------------
/**
 * カーソル文字列セット
 */
//--------------------------------------------------------------
extern void BmpMenuList_SetCursorString( BMPMENULIST_WORK *lw, u32 strID );

//--------------------------------------------------------------
/**
 * カーソルビットマップセット
 */
//--------------------------------------------------------------
extern void BmpMenuList_SetCursorBmp( BMPMENULIST_WORK *lw, u32 heapID );

//--------------------------------------------------------------
/**
 * ヘッダーにセットしたワークを取り出す
 */
//--------------------------------------------------------------
extern void* BmpMenuList_GetWorkPtr( BMPMENULIST_WORK *lw );

//--------------------------------------------------------------
/// キャンセルモードセット
//--------------------------------------------------------------
extern void BmpMenuList_SetCancelMode( BMPMENULIST_WORK *lw, BMPMENULIST_CANCELMODE mode );

#endif //__BMP_LIST_H__
