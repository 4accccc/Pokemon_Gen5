/**
 *	@file	selwin.h
 *	@brief	選択リストウィンドウ描画・コントロールシステム
 *	@author	Miyuki Iwasawa
 *	@date	07.10.10
 */

#ifndef __H_SELWIN_H__
#define __H_SELWIN_H__

//コンパイルを通すため、取りあえず定義だけ持ってくる
#include "bmp_menulist.h"
#include "palanm.h"
#include "print/printsys.h"

#define SBOX_SELECT_NULL	(BMPMENULIST_NULL)
#define SBOX_SELECT_CANCEL	(BMPMENULIST_CANCEL)

#define SBOX_WINCGX_SIZ	(27)

typedef enum{
	SBOX_OFSTYPE_LEFT,		///<左寄せタイプ
	SBOX_OFSTYPE_CENTER,	///<センタリングタイプ
}SBOX_OFSTYPE;

//コールバック関数の呼び出しモード
typedef enum{
	SBOX_CB_MODE_INI,	//初期化時
	SBOX_CB_MODE_MOVE,	//カーソル移動時
	SBOX_CB_MODE_DECIDE,		//選択肢決定
	SBOX_CB_MODE_CANCEL,	//キャンセル
}SBOX_CB_MODE;

///<セレクトボックスBmpWin領域
typedef struct _SELBOX_BMPWIN{
	u8	frm_no;	///<フレームナンバー
	u8	siz_x;	///<キャラクタ領域サイズX
	u8	siz_y;	///<キャラクタ領域サイズY
	u8	chrofs;	///<使用キャラクタオフセット
	u16	palid;	///<使用パレットID
	u16	dmy;
}SELBOX_BMPWIN;

///<セレクトボックスポジションデータ
typedef struct _SELBOX_POS{
	u8	pos_x;
	u8	pos_y;
}SELBOX_POS;

///セレクトボックス　ヘッダデータ構造体
typedef struct _SELBOX_HEAD_PRM{
	u8	loop_f:1;		///<ループするならTRUE
	u8	ofs_type:7;	///<左寄せorセンタリング(SBOX_OFSTYPE_LEFT or SXBOX_OFSTYPE_CENTER)
	
	u8	ofs_x;	///<項目表示オフセットX座標(dot)
	u8	frm;	///<フレームNo
	u8	pal;	///<パレットID
	
	u8	bg_pri;	///<BGプライオリティ
	u8	s_pri;	///<ソフトウェアプライオリティ

	u16	scgx;	///<文字列領域cgx(キャラ単位)
	u16	fcgx;	///<フレーム領域cgx(キャラ単位)
	u16	cgx_siz;	///<占有するcgx領域サイズ(キャラ単位)
	
	//↑(文字幅でウィンドウサイズが変わるので、領域オーバーチェック用に使っていいサイズを登録しておく) 
}SELBOX_HEAD_PRM;

///セレクトボックス　ヘッダ構造体
typedef struct _SELBOX_HEADER{
	SELBOX_HEAD_PRM prm;		///<使用するVRAM領域やパレットNoなどのパラメータを格納する構造体型
	const BMPLIST_DATA*	list;	///<表示文字データポインタ(BMPLIST,BMPMENUと共通)

	GFL_FONT	*fontHandle;
	u8	count;	///<表示項目数
}SELBOX_HEADER;


#define SBOXOAM_RESMAX	(4)
#define SBOXOAM_ACTMAX	(4)

typedef enum{
	SBOXOAM_DRAW_MAIN = NNS_G2D_VRAM_TYPE_2DMAIN,
	SBOXOAM_DRAW_SUB = NNS_G2D_VRAM_TYPE_2DSUB,
	SBOXOAM_DRAW_BOTH = NNS_G2D_VRAM_TYPE_MAX
}SBOXOAM_DRAW;

typedef enum{
	SBOXOAM_VMAP_1D32 = GX_OBJVRAMMODE_CHAR_1D_32K,
	SBOXOAM_VMAP_1D64 = GX_OBJVRAMMODE_CHAR_1D_64K,
	SBOXOAM_VMAP_1D128 = GX_OBJVRAMMODE_CHAR_1D_128K,
	SBOXOAM_VMAP_1D256 = GX_OBJVRAMMODE_CHAR_1D_256K,
}SBOXOAM_VMAP;

/**
 *	セレクトボックス　システムワーク
 */
typedef struct _SELBOX_SYS{
	int heapID;

	NNSG2dCharacterData* p_char;
	NNSG2dPaletteData* p_pltt;
	void* char_buf;	
	void* pal_buf;	
	
	PALETTE_FADE_PTR palASys;
}SELBOX_SYS;

//構造体
typedef struct _SELBOX_WORK SELBOX_WORK;

//コールバック関数型定義
typedef void (*SELBOX_CB_FUNC)(SELBOX_WORK* sbox,u8 cur_pos,void* work,SBOX_CB_MODE mode);

///<セレクトボックス
struct _SELBOX_WORK{
	SELBOX_SYS*	sys_wk;	///<システムワークへの参照ポインタ
	SELBOX_HEADER	hed;	///<ヘッダーデータ

	GFL_BMPWIN 			**win;	///<BmpWinデータ
	GFL_UI_TP_HITTBL	*tbl;	///<Hitテーブル

	u8	cp;		///<カーソルポイント
	u8	seq;	///<シーケンス
	u8	width;	///<ウィンドウ幅
	u8	se_manual_f:1;	///<SEはコールバックを使って自分で再生する場合
	u8	key_mode:7;	///<キー操作モードorタッチ操作モード

	u8	px;		///<ポジションX
	u8	py;		///<ポジションY
	u8 ret;	///<リターン値格納ワーク
	u8	wait;	///<終了ウェイトカウンター
	u32 heapID;	///<メモリアロケートに利用するID

	SELBOX_CB_FUNC cb_func;	//コールバック
	void*			cb_work;	//コールバックに引き渡せるワーク
	
	PRINT_QUE	*printQue;
	PRINT_UTIL	*printUtil;
};


/**
 *	@brief	セレクトボックス　システムワーク確保
 *
 *	@param	heapID	利用ヒープ
 *	@param	palASys	パレットアニメワークの参照ポインタ。NULL指定可
 */
extern SELBOX_SYS*	SelectBoxSys_AllocWork( int heapID, PALETTE_FADE_PTR palASys );

/**
 *	@brief	セレクトボックス　システムワーク解放
 */
extern void SelectBoxSys_Free(SELBOX_SYS* wk);

/**
 *	@brief	セレクトボックス　セット
 *
 *	@param	header	セレクトボックスヘッダポインタ
 * 	@param	key_mode	操作モード(APP_END_KEY or APP_END_TOUCH)
 *	@param	x		描画開始位置x(char)
 *	@param	y		描画開始位置y(char)
 *	@param	width	ウィンドウ幅(char) 0を指定した場合、文字列サイズ依存
 *	@param	cursor	初期カーソルポジション
 
 *	@li	ワークを内部でAllocするため、必ずSelectBoxExitで解放してください
 */
extern SELBOX_WORK* SelectBoxSet(SELBOX_SYS* sp,const SELBOX_HEADER* header,
				u8 key_mode,u8 x,u8 y,u8 width,u8 cursor);

/**
 *	@brief	セレクトボックス　セットEx(コールバックを取り、SEを自動で鳴らすかどうか選べる)
 *
 *	@param	header	セレクトボックスヘッダポインタ
 *	@param	key_mode	操作モード(APP_END_KEY or APP_END_TOUCH)
 *	@param	x		描画開始位置x(char)
 *	@param	y		描画開始位置y(char)
 *	@param	width	ウィンドウ幅(char) 0を指定した場合、文字列サイズ依存
 *	@param	cursor	初期カーソルポジション
 *	@param	cb_func	コールバック関数へのポインタ
 *	@param	cb_work	コールバック関数に引き渡す、任意のワークポインタ
 *	@param	se_manual_f	TRUEを渡すとSEを自前で鳴らす必要がある
 
 *	@li	ワークを内部でAllocするため、必ずSelectBoxExitで解放してください
 */
extern SELBOX_WORK* SelectBoxSetEx(SELBOX_SYS* sp,const SELBOX_HEADER* header,u8 key_mode,
	u8 x,u8 y,u8 width,u8 cursor,SELBOX_CB_FUNC cb_func,void* cb_work,BOOL se_manual_f);

/**
 *	@brief	セレクトボックス　終了時のキータッチステータスを取得
 *
 *	@retval APP_END_KEY
 *	@retval APP_END_TOUCH
 *
 *	SelectBoxExit()のコール前に呼び出してください
 */
extern int SelectBoxGetKTStatus(SELBOX_WORK* wk);

/**
 *	@brief	セレクトボックス　終了・解放処理
 */
extern void SelectBoxExit(SELBOX_WORK* wk);

/**
 *	@brief	セレクトボックス　コントロールメイン
 *
 * @retval	"param = 選択パラメータ"
 * @retval	"SBOX_SELECT_NULL = 選択中"
 * @retval	"SBOX_SELECT_CANCEL	= キャンセル(Ｂボタン)"
 */
extern u32 SelectBoxMain(SELBOX_WORK* wk);

#endif	//__H_SELWIN_H__
