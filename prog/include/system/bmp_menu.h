//======================================================================
/**
 * @file	bmp_menu.h
 * @brief	BMPメニュー処理
 * @author	Hiroyuki Nakamura
 * @date	2004.11.10
 */
//======================================================================
#ifndef BMP_MENU_H
#define BMP_MENU_H

#include <tcbl.h>
#include "print\printsys.h"
#include "print\gf_font.h"

#include "bmp_menuwork.h"

//======================================================================
//	シンボル定義
//======================================================================
#define	BMPMENU_NULL	( 0xffffffff )
#define	BMPMENU_CANCEL	( 0xfffffffe )
#define	BMPMENU_DUMMY	( 0xfffffffd )

typedef struct {
	const BMPMENU_DATA * menu;
	GFL_BMPWIN *win;
	u8	dummy;
	u8	x_max;			// 横方向項目最大数（必ず１以上）
	u8	y_max;			// 縦方向項目最大数（必ず１以上）
	u8	line_spc:4;		// 文字間隔Ｙ
	u8	c_disp_f:2;		// カーソル表示フラグ
	u8	loop_f:2;		// LOOPフラグ ( 0=OFF, 1=ON )
	
	u16 font_size_x;		//文字サイズX(ドット
	u16 font_size_y;		//文字サイズY(ドット
	GFL_MSGDATA *msgdata;	//表示に使用するメッセージバッファ
//	STRBUF *strbuf;			//表示に使用するメッセージバッファ
	PRINT_UTIL *print_util; //表示に使用するプリントユーティリティ
	PRINT_QUE *print_que;	//表示に使用するプリントキュー
	GFL_FONT *font_handle;	//表示に使用するフォントハンドル

  BOOL  b_trans;  // PRINT_QUE処理完了後に転送していればTRUE
}BMPMENU_HEADER;

typedef struct _BMPMENU_WORK	BMPMENU_WORK;

// 外部コントロールパラメータ
enum {
	BMPMENU_CNTROL_DECIDE = 0,	// 決定
	BMPMENU_CNTROL_CANCEL,		// キャンセル
	BMPMENU_CNTROL_UP,			// 上
	BMPMENU_CNTROL_DOWN,		// 下
	BMPMENU_CNTROL_LEFT,		// 左
	BMPMENU_CNTROL_RIGHT,		// 右
};

// 移動方向
enum {
	BMPMENU_MOVE_NONE = 0,	// 動作なし
	BMPMENU_MOVE_UP,		// 上へ移動
	BMPMENU_MOVE_DOWN,		// 下へ移動
	BMPMENU_MOVE_LEFT,		// 左へ移動
	BMPMENU_MOVE_RIGHT,		// 右へ移動
};

//======================================================================
//	プロトタイプ宣言
//======================================================================
//--------------------------------------------------------------
/**
 * BMPメニュー登録（スクリーン転送しない）
 *
 * @param	dat			ヘッダデータ
 * @param	px			項目表示X座標
 * @param	py			項目表示Y座標
 * @param	pos			初期カーソル位置
 * @aram	mode		メモリ取得モード
 * @param	cancel		キャンセルボタン
 *
 * @return	BMPメニューワーク
 *
 * @li	BMPリストワークはsys_AllocMemoryで確保
 */
//--------------------------------------------------------------
extern BMPMENU_WORK * BmpMenu_AddNoTrans( const BMPMENU_HEADER * dat,
		u8 px, u8 py, u8 pos, HEAPID heap_id, u32 cancel );

//--------------------------------------------------------------
/**
 * BMPメニュー登録（キャンセルボタン指定）
 *
 * @param	dat			ヘッダデータ
 * @param	px			項目表示X座標
 * @param	py			項目表示Y座標
 * @param	pos			初期カーソル位置
 * @aram	mode		メモリ取得モード
 * @param	cancel		キャンセルボタン
 *
 * @return	BMPメニューワーク
 *
 * @li	BMPリストワークはsys_AllocMemoryで確保
 */
//--------------------------------------------------------------
extern BMPMENU_WORK * BmpMenu_AddEx( const BMPMENU_HEADER * dat,
		u8 px, u8 py, u8 pos, HEAPID heap_id, u32 cancel );

//--------------------------------------------------------------
/**
 * BMPメニュー登録（簡易版）
 *
 * @param	dat			ヘッダデータ
 * @param	pos			初期カーソル位置
 * @aram	mode		メモリ取得モード
 *
 * @return	BMPメニューワーク
 *
 * @li	BMPリストワークはsys_AllocMemoryで確保
 * @li	Bボタンキャンセル
 */
//--------------------------------------------------------------
extern BMPMENU_WORK * BmpMenu_Add(
	const BMPMENU_HEADER * dat, u8 pos, HEAPID heap_id );

//--------------------------------------------------------------
/**
 * BMPメニュー破棄
 *
 * @param	mw		BMPメニューワーク
 * @param	backup	カーソル位置保存場所
 *
 * @return	none
 */
//--------------------------------------------------------------
extern void BmpMenu_Exit( BMPMENU_WORK *mw, u8 * backup );

//--------------------------------------------------------------
/**
 * メニュー動作関数
 *
 * @param	mw		BMPメニューワーク
 *
 * @return	動作結果
 */
//--------------------------------------------------------------
extern u32 BmpMenu_Main( BMPMENU_WORK * mw );

//--------------------------------------------------------------
/**
 * メニュー動作関数（十字キーのSEを指定できる）
 *
 * @param	mw		BMPメニューワーク
 * @param	key_se	十字キーのSE
 *
 * @return	動作結果
 */
//--------------------------------------------------------------
extern u32 BmpMenu_MainSE( BMPMENU_WORK * mw, u16 key_se );

//--------------------------------------------------------------
/**
 * メニュー外部コントロール
 *
 * @param	mw		BMPメニューワーク
 * @param	prm		コントロールパラメータ
 *
 * @return	動作結果
 */
//--------------------------------------------------------------
extern u32 BmpMenu_MainOutControl( BMPMENU_WORK * mw, u8 prm );


//--------------------------------------------------------------
/**
 * カーソル位置取得
 *
 * @param	mw		BMPメニューワーク
 *
 * @return	カーソル位置
 */
//--------------------------------------------------------------
extern u8 BmpMenu_CursorPosGet( BMPMENU_WORK * mw );

//--------------------------------------------------------------
/**
 * 移動方向取得
 *
 * @param	mw		BMPメニューワーク
 *
 * @return	移動方向取得
 */
//--------------------------------------------------------------
extern u8 BmpMenu_MoveSiteGet( BMPMENU_WORK * mw );

//--------------------------------------------------------------
/**
 * カーソル文字列セット
 * @param	mw	BMPMENU_WORK
 * @param	strID	文字列ID
 * @retval	nothing
 */
//--------------------------------------------------------------
extern void BmpMenu_SetCursorString( BMPMENU_WORK *mw, u32 strID );

//関数追加しました Ari081015
//--------------------------------------------------------------
/**
 * メニュー再描画
 * @param	mw	BMPMENU_WORK
 * @retval	nothing
 */
//--------------------------------------------------------------
extern void BmpMenu_RedrawString( BMPMENU_WORK *mw );


typedef struct {
	u8	frmnum;	///<ウインドウ使用フレーム
	u8	pos_x;		///<ウインドウ領域の左上のX座標（キャラ単位で指定）
	u8	pos_y;		///<ウインドウ領域の左上のY座標（キャラ単位で指定）
	u8	palnum;		///<ウインドウ領域のパレットナンバー
	u16	chrnum;		///<ウインドウキャラ領域の開始キャラクタナンバー
} BMPWIN_YESNO_DAT;

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえウィンドウセット（カーソル位置指定）
 *
 * @param	data	ウィンドウデータ
 * @param	cgx		ウィンドウキャラ位置
 * @param	pal		ウィンドウパレット番号
 * @param	pos		初期カーソル位置
 * @param	heap	ヒープID
 *
 * @return	BMPメニューワーク
 *
 * @li	BMPウィンドウとBMPメニューワークをAllocで取得している
 */
//--------------------------------------------------------------------------------------------
extern BMPMENU_WORK * BmpMenu_YesNoSelectInit(	const BMPWIN_YESNO_DAT *data, u16 cgx, u8 pal, u8 pos, HEAPID heap );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえ選択ウィンドウの制御
 *
 * @param	ini		BGLデータ
 * @param	heap	ヒープID
 *
 * @retval	"BMPMENU_NULL	選択されていない"
 * @retval	"0				はいを選択"
 * @retval	"BMPMENU_CANCEL	いいえorキャンセル"
 */
//--------------------------------------------------------------------------------------------
extern u32 BmpMenu_YesNoSelectMain( BMPMENU_WORK * mw );

//--------------------------------------------------------------------------------------------
/**
 * はい・いいえウィンドウ削除
 *
 * @param	ini		BGLデータ
 * @param	heap	ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
extern void BmpMenu_YesNoMenuExit( BMPMENU_WORK * mw );

//------------------------------------------------------------------
/**
 * ウィンドウにカーソル画像を描画
 *
 * @param   win		ウィンドウ
 * @param   x		描画Ｘ座標（ドット）
 * @param   y		描画Ｙ座標（ドット）
 *
 */
//------------------------------------------------------------------
extern void BmpWin_DrawCursorImage(GFL_BMPWIN* win, u32 x, u32 y);


#endif	/* BMP_MENU_H */
