//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		touch_subwindow.c
 *	@brief		サブ画面　触れるウィンドウボタンシステム
 *	@author		tomoya takahashi & miyuki iwasawa
 *	@data		2008.06.23
 *				2009.01.15 GSより移植 Ariizumi
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include <string.h>

#include "system/gfl_use.h"
#include "arc_def.h"
#include "yn_touch.naix"

#define __TOUCH_SUBWINDOW_H_GLOBAL 
#include "include/system/touch_subwindow.h"

#define TSW_USE_SND (0)

//-----------------------------------------------------------------------------
/** 
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// 各バッファ・ワークの数
enum{		
	TOUCH_SW_BUF_YES,
	TOUCH_SW_BUF_NO,
	TOUCH_SW_BUF_NUM,


	TOUCH_SW_BUF_NO_TOUCH,	// タッチされたかチェック時の触っていない定数
};
#define TOUCH_SW_Y_OFS	( 32 )	// NOの方のオフセット

// アニメーション関係
#define TOUCH_SW_GRA_ID	(ARC_YNTOUCH_GRA)
#define TOUCH_SW_ANIME_NUM		( 2 )		// アニメーション数
#define TOUCH_SW_ANIME_SP		( FX32_ONE * 2 )// アニメスピード
#define TOUCH_SW_ANIME_TIMING	( FX32_ONE * 1 )

//反応するキー
#define TOUCH_SW_KEY_TRG	(PAD_BUTTON_DECIDE|PAD_BUTTON_CANCEL|PAD_KEY_UP|PAD_KEY_DOWN)

// 今のワーク内の状態
enum{
	TOUCH_SW_PARAM_NODATA,	// 空
	TOUCH_SW_PARAM_DO,		// 実行データ格納
};

// VRAM転送タスク
#define TOUCH_SW_VRAM_TRANS_TSK_PRI	( 128 )

#define TOUCH_FLG_INIT	( 0x8 )


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	アニメデータ
//=====================================
typedef struct {
	const fx32* cp_anime_tbl;
	u32 tbl_num;
	fx32 frame;
} TOUCH_SW_ANIME;


//-------------------------------------
//	ボタンデータ初期化データ
//=====================================
typedef struct {
	const fx32* cp_anime_tbl;	// アニメテーブル
	u16 fileidx;	// アーカイブファイル
	u16 scrn_arcidx[ TOUCH_SW_ANIME_NUM ];	// スクリーンデータ
	u16 char_offs;	// キャラクタオフセット
	u16 pltt_offs;	// パレットオフセット
	u8 bg_frame;			// BGナンバー
	u8 tbl_num;				// テーブル数
	u8 ofs_x;		// ｘ
	u8 ofs_y;		// ｙ
} TOUCH_SW_BUTTON_PARAM;

//-------------------------------------
//	1ボタンデータ
//=====================================
typedef struct {
	u32 bg_frame;			// BGナンバー
	void* p_scrn_buff[ TOUCH_SW_ANIME_NUM ];		// スクリーンバッファ
	NNSG2dScreenData* p_scrn[ TOUCH_SW_ANIME_NUM ];	// スクリーンデータ
	TOUCH_SW_ANIME anime;
	u8 ofs_x;	// ｘ座標
	u8 ofs_y;	// ｙ座標
	u8 anm_idx;	// 今の反映されているスクリーンIDX
	u8 dummy1;
} TOUCH_SW_BUTTON;

//-------------------------------------
//	キャラクタデータ転送タスク
//=====================================
typedef struct {
	NNSG2dCharacterData* p_char;
	void* p_buff;	
	u16 bg_frame;
	u16 char_ofs;
} TOUCH_SW_CHARACTER_TRANS;

//-------------------------------------
//	Vブランク　パレットデータ転送タスク
//=====================================
typedef struct {
	NNSG2dPaletteData* p_pltt;
	void* p_buff;	
	u16 pltype;	// パレット転送先
	u16 ofs;
	u16 size;
} TOUCH_SW_PALETTE_TRANS;


//-------------------------------------
//	システムワーク
//=====================================
struct _TOUCH_SW_SYS
{
	GFL_BUTTON_MAN* p_button_man;// ボタンマネージャ
	GFL_UI_TP_HITTBL hit_tbl[ TOUCH_SW_BUF_NUM+1 ];		// ボタンデータ(終端データ
	TOUCH_SW_BUTTON button[ TOUCH_SW_BUF_NUM ];	// ボタンデータ
	u32 bg_frame;			// BGナンバー
	u32 heapid;				// 使用ヒープID
	fx32 anime_tbl[ TOUCH_SW_ANIME_NUM ];	// アニメテーブル
	u8 x;					// x座標（ｷｬﾗｸﾀ単位）
	u8 y;					// y座標（ｷｬﾗｸﾀ単位）
	u8 sizx;				// xサイズ（ｷｬﾗｸﾀ単位）
	u8 sizy;				// yサイズ（ｷｬﾗｸﾀ単位）
	
	u8 kt_st:1;				// キーorタッチのステータス
	u8 type:1;				// 表示タイプ(TOUCH_SW_TYPE_S,TOUCH_SW_TYPE_L)
	u8 wait:6;				// ウェイト
	u8 key_pos;				// キーのポジション
	u8 button_move_flag;	// ボタン動作フラグ
	u8 now_work_param : 4;	// 現在のワーク内の状態
	u8 touch_flg: 4;		// タッチした瞬間感知
};


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void TouchSW_CleanWork( TOUCH_SW_SYS* p_touch_sw, u32 heapid );
static void touchsw_ktst_view_set(TOUCH_SW_SYS* p_touch_sw);

//-------------------------------------
//	ある程度処理をまとめた関数
//=====================================
static void TouchSW_SYS_CharTrans( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param );
static void TouchSW_SYS_PlttTrans( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param );
static void TouchSW_SYS_ButtonInit( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param );
static void TouchSW_SYS_ButtonManaInit( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param );

//-------------------------------------
//	ボタン動作システム
//=====================================
static void TouchSW_ButtonCallBack( u32 button_no, u32 event, void* p_work );

//-------------------------------------
//	ボタンワーク操作関数郡
//=====================================
static void TouchSW_ButtonInit( TOUCH_SW_BUTTON* p_bttn, const TOUCH_SW_BUTTON_PARAM* cp_param, u32 heapid );
static void TouchSW_ButtonDelete( TOUCH_SW_BUTTON* p_bttn );

//-------------------------------------
//	キャラクタデータ　パレットデータ転送
//=====================================
static void TouchSW_CharTransReq( u32 fileIdx, u32 dataIdx, u32 frm, u32 offs, u32 heapID );
static void TouchSW_PlttTransReq( u32 fileIdx, u32 dataIdx, PALTYPE palType, u32 offs, u32 transSize, u32 heapID );
static void TouchSW_CharTransTsk( GFL_TCB *tcb, void* p_work );
static void TouchSW_PlttTransTsk( GFL_TCB *tcb, void* p_work );


//-------------------------------------
//	スクリーンの反映を行う
//=====================================
static void TouchSW_ScrnSet( int frame, const NNSG2dScreenData* scrn, int x, int y );
static void TouchSW_ScrnCharOfsSet( const NNSG2dScreenData* scrn, int char_offs );
static void TouchSW_ScrnPlttOfsSet( const NNSG2dScreenData* scrn, int pltt_offs );

//----------------------------------------------------------------------------
/**
 *	@brief	システムワーク作成
 *	
 *	@param	heapid	使用ヒープID 
 *	
 *	@return	ワークポインタ
 */
//-----------------------------------------------------------------------------
TOUCH_SW_SYS* TOUCH_SW_AllocWork( u32 heapid )
{
	TOUCH_SW_SYS* p_touch_sw;
	p_touch_sw = GFL_HEAP_AllocMemory( heapid, sizeof(TOUCH_SW_SYS) );

	// ワーク初期化
	TouchSW_CleanWork( p_touch_sw, heapid );

	return p_touch_sw;
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムワーク破棄
 *	
 *	@param	p_touch_sw	システムワーク
 */
//-----------------------------------------------------------------------------
void TOUCH_SW_FreeWork( TOUCH_SW_SYS* p_touch_sw )
{
	// 実行データ格納済みならリセット
	if( p_touch_sw->now_work_param == TOUCH_SW_PARAM_DO ){
		TOUCH_SW_Reset( p_touch_sw );
	}
	GFL_HEAP_FreeMemory( p_touch_sw );
}

//----------------------------------------------------------------------------
/**
 *	@brief	システム初期化(内部共通処理)
 *
 *	@param	p_touch_sw	システムワーク
 *	@param	cp_param	システム動作データ
 */
//-----------------------------------------------------------------------------
static void touch_sw_init_com( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	static const u8 btn_siz[][2] = {
		{TOUCH_SW_USE_SCRN_SX,TOUCH_SW_USE_SCRN_SY},
		{TOUCH_SW_USE_SCRN_LX,TOUCH_SW_USE_SCRN_LY},
	};
	
	// データ格納
	p_touch_sw->bg_frame	= cp_param->bg_frame;
	p_touch_sw->x			= cp_param->x;
	p_touch_sw->y			= cp_param->y;

	//初期キーorタッチモード
	p_touch_sw->kt_st		= cp_param->kt_st;
	//YesNo選択の初期ポジション
	if(cp_param->key_pos < 2){
		p_touch_sw->key_pos		= cp_param->key_pos;
	}else{
		p_touch_sw->key_pos = 0;
	}
	
	//ボタンサイズ格納
	p_touch_sw->sizx = btn_siz[cp_param->type][0];
	p_touch_sw->sizy = btn_siz[cp_param->type][1];
	
	// キャラクタデータ転送
	TouchSW_SYS_CharTrans( p_touch_sw, cp_param );

	// パレットデータ転送
	TouchSW_SYS_PlttTrans( p_touch_sw, cp_param );
	
	// ボタンデータ初期化
	TouchSW_SYS_ButtonInit( p_touch_sw, cp_param );

	// ボタンマネージャ初期化
	TouchSW_SYS_ButtonManaInit( p_touch_sw, cp_param );

	// 実行データ保持
	p_touch_sw->now_work_param = TOUCH_SW_PARAM_DO;

	// タッチフラグ
	p_touch_sw->touch_flg = TOUCH_FLG_INIT;

	//操作モードによって、初期Viewを変更
	touchsw_ktst_view_set(p_touch_sw);
}

//----------------------------------------------------------------------------
/**
 *	@brief	システム初期化
 *
 *	@param	p_touch_sw	システムワーク
 *	@param	cp_param	システム動作データ
 */
//-----------------------------------------------------------------------------
void TOUCH_SW_Init( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	touch_sw_init_com(p_touch_sw,cp_param );
}

//----------------------------------------------------------------------------
/**
 *	@brief	システム初期化EX パレットアニメワーク初期化処理込み
 *
 *	@param	p_touch_sw	システムワーク
 *	@param	cp_param	システム動作データ
 *	@param	palASys		パレットアニメシステムワーク
 */
//-----------------------------------------------------------------------------
void TOUCH_SW_InitEx( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param,PALETTE_FADE_PTR palASys)
{
	NNSG2dPaletteData*	pPal;
	void* pSrc;

	touch_sw_init_com(p_touch_sw,cp_param );

	if(palASys == NULL){
		return;
	}
	//パレットアニメワークにデータ登録
	pSrc = GFL_ARC_UTIL_LoadPalette(ARCID_YN_TOUCH,
			NARC_yn_touch_yes_no_touch_NCLR,
			&pPal,p_touch_sw->heapid);

	PaletteWorkSet(palASys,pPal->pRawData,p_touch_sw->bg_frame/4,
			cp_param->pltt_offs*16,TOUCH_SW_USE_PLTT_NUM*32);

	GFL_HEAP_FreeMemory(pSrc);
}


//----------------------------------------------------------------------------
/**
 *	@brief	システムメイン動作
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@retval	TOUCH_SW_RET_NORMAL	// 何もなし
 *	@retval	TOUCH_SW_RET_YES	// はい
 *	@retval	TOUCH_SW_RET_NO	// いいえ
 *	@retval	TOUCH_SW_RET_YES_TOUCH	// はい押した瞬間
 *	@retval	TOUCH_SW_RET_NO_TOUCH	// いいえ押した瞬間
 */
//-----------------------------------------------------------------------------
u32 TOUCH_SW_MainMC( TOUCH_SW_SYS* p_touch_sw )
{
	u32 ret;

	// タッチフラグ初期化
	p_touch_sw->touch_flg = TOUCH_FLG_INIT;

	ret = TOUCH_SW_Main( p_touch_sw );

	// 押された瞬間も返す
	if( ret == TOUCH_SW_RET_NORMAL ){
		if( p_touch_sw->touch_flg == GFL_BMN_EVENT_TOUCH ){
			if( p_touch_sw->button_move_flag == TOUCH_SW_BUF_YES ){
				return TOUCH_SW_RET_YES_TOUCH;
			}else{
				return TOUCH_SW_RET_NO_TOUCH;
			}
		}
	}
	return ret;
}


//------------------------------------------------------------------
/*
 *	@brief	ボタンの状態書き込み
 */
//------------------------------------------------------------------
static void touchsw_btn_draw(TOUCH_SW_SYS* p_touch_sw,u8 pos,u8 state)
{
	TOUCH_SW_BUTTON* p_bttn;
	
	p_bttn = &p_touch_sw->button[pos];
	
	TouchSW_ScrnSet( p_bttn->bg_frame,
			p_bttn->p_scrn[state], p_bttn->ofs_x, p_bttn->ofs_y );
	
	GFL_BG_LoadScreenV_Req( p_bttn->bg_frame);
}

//------------------------------------------------------------------
/*
 *	@brief	キー＆タッチ切替時の描画切替処理
 */
//------------------------------------------------------------------
static void touchsw_ktst_view_set(TOUCH_SW_SYS* p_touch_sw)
{
	if(p_touch_sw->kt_st == GFL_APP_KTST_KEY){
		//キーに切り替わった
		touchsw_btn_draw(p_touch_sw,p_touch_sw->key_pos,TRUE);
		touchsw_btn_draw(p_touch_sw,p_touch_sw->key_pos^1,FALSE);
	}else{
		//タッチに切り替わった
		touchsw_btn_draw(p_touch_sw,p_touch_sw->key_pos,FALSE);
	}
}

//------------------------------------------------------------------
/*
 *	@brief	入力取得　キー
 */
//------------------------------------------------------------------
static int touchsw_ktst_input_key(TOUCH_SW_SYS* p_touch_sw)
{
	const int keyTrg = GFL_UI_KEY_GetTrg();
	if(keyTrg & PAD_BUTTON_DECIDE){
		// 触ったボタン保存
		p_touch_sw->button_move_flag = p_touch_sw->key_pos;
#if TSW_USE_SND
		Snd_SePlay( SE_TOUCH_SUB_WIN );
#endif
		return 1;
	}else if(keyTrg & PAD_BUTTON_CANCEL){
		p_touch_sw->button_move_flag = TOUCH_SW_BUF_NO;
#if TSW_USE_SND
		Snd_SePlay( SE_TOUCH_SUB_WIN );
#endif
		return 1;
	}
	if(keyTrg & (PAD_KEY_UP|PAD_KEY_DOWN)){
		p_touch_sw->key_pos ^= 1;
		touchsw_ktst_view_set(p_touch_sw);
#if TSW_USE_SND
		Snd_SePlay( SE_SELECT_SUB_WIN );
#endif
		return 0;
	}
	return 0;
}

//------------------------------------------------------------------
/*
 *	@brief	入力取得　リスト選択
 */
//------------------------------------------------------------------
static int touchsw_ktst_input(TOUCH_SW_SYS* p_touch_sw)
{
	BOOL  ret;
	const int keyTrg = GFL_UI_KEY_GetTrg();
	
	ret = GFL_BMN_Main( p_touch_sw->p_button_man );
	if(ret){
		p_touch_sw->kt_st = GFL_APP_KTST_TOUCH;
		return 1;
	}
	if(p_touch_sw->kt_st == GFL_APP_KTST_TOUCH){
		if(keyTrg & TOUCH_SW_KEY_TRG){
			p_touch_sw->kt_st = GFL_APP_KTST_KEY;
			touchsw_ktst_view_set(p_touch_sw);
			return 0;
		}
	}
	return touchsw_ktst_input_key(p_touch_sw);
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムメイン動作
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@retval	TOUCH_SW_RET_NORMAL	// 何もなし
 *	@retval	TOUCH_SW_RET_YES	// はい
 *	@retval	TOUCH_SW_RET_NO		// いいえ
 */
//-----------------------------------------------------------------------------
u32 TOUCH_SW_Main( TOUCH_SW_SYS* p_touch_sw )
{
	const int anmSpdRate = 2;
	BOOL check;
	
	// 実行データ設定済み
	GF_ASSERT( p_touch_sw->now_work_param == TOUCH_SW_PARAM_DO );

	//タッチ＆キーのモード切替をチェック
	// 触ったボタンが無いときは入力取得を実行
	if( p_touch_sw->button_move_flag == TOUCH_SW_BUF_NO_TOUCH ){
		check = touchsw_ktst_input(p_touch_sw);
		if(!check){
			return TOUCH_SW_RET_NORMAL;
		}
		//決定された瞬間の表示状態をセット
		touchsw_btn_draw(p_touch_sw,p_touch_sw->button_move_flag,TRUE);
		touchsw_btn_draw(p_touch_sw,p_touch_sw->button_move_flag^1,FALSE);
		return TOUCH_SW_RET_NORMAL;
	}

	if(p_touch_sw->wait == 8*anmSpdRate){
		//アニメ終了
		if( p_touch_sw->button_move_flag == TOUCH_SW_BUF_YES ){
			return TOUCH_SW_RET_YES;
		}else{
			return TOUCH_SW_RET_NO;
		}
	}
	//アニメ中
	if(p_touch_sw->wait % (2*anmSpdRate) == 0){
		if((p_touch_sw->wait / (2*anmSpdRate)) % 2 == 0){
			touchsw_btn_draw(p_touch_sw,p_touch_sw->button_move_flag,TRUE);
		}else{
			touchsw_btn_draw(p_touch_sw,p_touch_sw->button_move_flag,FALSE);
		}
	}
	p_touch_sw->wait++;	
	return TOUCH_SW_RET_NORMAL;
}

//----------------------------------------------------------------------------
/**
 *	@brief	現在の操作モードを取得(システムデータリセット前に取得すること)
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@retval GFL_APP_END_KEY
 *	@retval GFL_APP_END_TOUCH	
 */
//-----------------------------------------------------------------------------
int TOUCH_SW_GetKTStatus( TOUCH_SW_SYS* p_touch_sw )
{
	return p_touch_sw->kt_st;
}

//----------------------------------------------------------------------------
/**
 *	@brief	システムデータリセット	(Initの前の状態にする　いらないかもしれない)
 *
 *	@param	p_touch_sw	システムワーク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
void TOUCH_SW_Reset( TOUCH_SW_SYS* p_touch_sw )
{
	int i;
	
	// ボタンマネージャ破棄
	GFL_BMN_Delete( p_touch_sw->p_button_man );

	// ボタンデータ破棄
	for( i=0; i<TOUCH_SW_BUF_NUM; i++ ){
		TouchSW_ButtonDelete( &p_touch_sw->button[ i ] );
	}

	// ワーク初期化
	TouchSW_CleanWork( p_touch_sw, p_touch_sw->heapid );
}



//-----------------------------------------------------------------------------
/**
 *		static 関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief	ワークの初期化
 *
 *	@param	p_touch_sw 
 *	@param	heapid
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void TouchSW_CleanWork( TOUCH_SW_SYS* p_touch_sw, u32 heapid )
{
	memset( p_touch_sw, 0, sizeof(TOUCH_SW_SYS) );

	p_touch_sw->now_work_param = TOUCH_SW_PARAM_NODATA;	// 空ID
	p_touch_sw->heapid = heapid;
	p_touch_sw->button_move_flag = TOUCH_SW_BUF_NO_TOUCH;
	p_touch_sw->touch_flg = TOUCH_FLG_INIT;
}


//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータ転送処理
 *
 *	@param	p_touch_sw
 *	@param	cp_param 
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_SYS_CharTrans( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	// キャラクタデータ転送
	TouchSW_CharTransReq( ARCID_YN_TOUCH, NARC_yn_touch_yes_no_touch_NCGR, 
			p_touch_sw->bg_frame, cp_param->char_offs,
			p_touch_sw->heapid );
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットデータ転送
 *
 *	@param	p_touch_sw
 *	@param	cp_param 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void TouchSW_SYS_PlttTrans( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	u32 pal_type;
	if( p_touch_sw->bg_frame < GFL_BG_FRAME0_S ){
		pal_type = PALTYPE_MAIN_BG;
	}else{
		pal_type = PALTYPE_SUB_BG;
	}
	TouchSW_PlttTransReq( ARCID_YN_TOUCH, NARC_yn_touch_yes_no_touch_NCLR,
			pal_type,
			cp_param->pltt_offs * 32, TOUCH_SW_USE_PLTT_NUM * 32,
			p_touch_sw->heapid );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータ初期化
 *
 *	@param	p_touch_sw
 *	@param	cp_param 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void TouchSW_SYS_ButtonInit( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	int i,j,datIdx;
	TOUCH_SW_BUTTON_PARAM bt_pr;

	// ボタンアニメデータ作成
	for( i=0; i<TOUCH_SW_ANIME_NUM; i++ ){
		p_touch_sw->anime_tbl[ i ] = TOUCH_SW_ANIME_TIMING * (i+1);
	}

	// ボタンデータ作成
	bt_pr.bg_frame	= p_touch_sw->bg_frame;
	bt_pr.fileidx	= ARCID_YN_TOUCH;
	bt_pr.char_offs = cp_param->char_offs;
	bt_pr.pltt_offs	= cp_param->pltt_offs;
	bt_pr.cp_anime_tbl = p_touch_sw->anime_tbl;
	bt_pr.tbl_num	= TOUCH_SW_ANIME_NUM;
	bt_pr.ofs_x		= p_touch_sw->x;

	for( i=0; i<TOUCH_SW_BUF_NUM; i++ ){

		if(cp_param->type == TOUCH_SW_TYPE_S)
		{
			if( i == TOUCH_SW_BUF_YES )
				datIdx = NARC_yn_touch_yes_touch_s01_NSCR;
			else
				datIdx = NARC_yn_touch_no_touch_s01_NSCR;
		}
		else
		{
			if( i == TOUCH_SW_BUF_YES )
				datIdx = NARC_yn_touch_yes_touch_l01_NSCR;
			else
				datIdx = NARC_yn_touch_no_touch_l01_NSCR;
		}

		// スクリーンデータインデックスバッファ作成
		for( j=0; j<TOUCH_SW_ANIME_NUM; j++ ){
			bt_pr.scrn_arcidx[ j ] = datIdx + j;
		}

		// Yオフセット値
		bt_pr.ofs_y = (i*p_touch_sw->sizy) + p_touch_sw->y;

		// パレットオフセット
		bt_pr.pltt_offs += i;

		// ボタン初期化
		TouchSW_ButtonInit( &p_touch_sw->button[i], &bt_pr, p_touch_sw->heapid );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンマネージャ初期化
 *
 *	@param	p_touch_sw
 *	@param	cp_param 
 *
 *	@return	
 */
//-----------------------------------------------------------------------------
static void TouchSW_SYS_ButtonManaInit( TOUCH_SW_SYS* p_touch_sw, const TOUCH_SW_PARAM* cp_param )
{
	int i;
	
	// ボタンあたり判定データ作成
	for( i=0; i<TOUCH_SW_BUF_NUM; i++ ){
		p_touch_sw->hit_tbl[ i ].rect.top	= (p_touch_sw->y*8) + (i*p_touch_sw->sizy*8);
		p_touch_sw->hit_tbl[ i ].rect.left	= (p_touch_sw->x*8);
		p_touch_sw->hit_tbl[ i ].rect.bottom = 
			(p_touch_sw->y*8) + (i*p_touch_sw->sizy*8) + (p_touch_sw->sizy*8);
		p_touch_sw->hit_tbl[ i ].rect.right	= (p_touch_sw->x*8) + (p_touch_sw->sizx*8);
	}
	p_touch_sw->hit_tbl[ TOUCH_SW_BUF_NUM ].circle.code = GFL_UI_TP_HIT_END;
	
	p_touch_sw->p_button_man = GFL_BMN_Create( p_touch_sw->hit_tbl, 
							TouchSW_ButtonCallBack, p_touch_sw, p_touch_sw->heapid );
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンマネージャコールバック
 *
 *	@param	button_no
 *	@param	event
 *	@param	p_work 
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_ButtonCallBack( u32 button_no, u32 event, void* p_work )
{
	TOUCH_SW_SYS* p_touch_sw = p_work;
	TOUCH_SW_BUTTON* p_bttn;

	p_touch_sw->touch_flg = event;

	if( event == GFL_BMN_EVENT_TOUCH ){	
		// 触ったボタン保存
		p_touch_sw->button_move_flag = button_no;
		p_bttn = &p_touch_sw->button[button_no];
		TouchSW_ScrnSet( p_bttn->bg_frame,
			p_bttn->p_scrn[1], p_bttn->ofs_x, p_bttn->ofs_y );
#if TSW_USE_SND
		Snd_SePlay( SE_TOUCH_SUB_WIN );
#endif
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンワーク初期化
 *
 *	@param	p_bttn		ボタンワーク
 *	@param	cp_param	ボタン初期化データ
 *	@param	heapid		ヒープID
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_ButtonInit( TOUCH_SW_BUTTON* p_bttn, const TOUCH_SW_BUTTON_PARAM* cp_param, u32 heapid )
{
	int i;
	
	// アニメデータ登録
	{
		TOUCH_SW_ANIME* p_anime = &p_bttn->anime;
		
		p_anime->cp_anime_tbl	= cp_param->cp_anime_tbl;
		p_anime->tbl_num		= cp_param->tbl_num;
		p_anime->frame			= 0;
	}
	// データ代入
	p_bttn->bg_frame= cp_param->bg_frame;
	p_bttn->ofs_x	= cp_param->ofs_x;
	p_bttn->ofs_y	= cp_param->ofs_y;

	// スクリーンワーク読み込み
	for( i=0; i<TOUCH_SW_ANIME_NUM; i++ ){
		p_bttn->p_scrn_buff[i] = GFL_ARC_UTIL_LoadScreen( cp_param->fileidx, cp_param->scrn_arcidx[i], 
				FALSE, &p_bttn->p_scrn[ i ], heapid );

		// キャラクタオフセットを設定
		TouchSW_ScrnCharOfsSet( p_bttn->p_scrn[ i ], cp_param->char_offs );
		// パレットオフセットを設定
		TouchSW_ScrnPlttOfsSet( p_bttn->p_scrn[ i ], cp_param->pltt_offs );
	}
	
	//初期スクリーンの反映を行う
	TouchSW_ScrnSet( p_bttn->bg_frame,
			p_bttn->p_scrn[ 0 ], p_bttn->ofs_x, p_bttn->ofs_y );

	p_bttn->anm_idx = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンデータ破棄
 *
 *	@param	p_bttn		ボタンワーク
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_ButtonDelete( TOUCH_SW_BUTTON* p_bttn )
{
	int i;
	
	// スクリーン領域を初期化
	GFL_BG_FillScreen( p_bttn->bg_frame, 0, 
			p_bttn->ofs_x, p_bttn->ofs_y,
			p_bttn->p_scrn[0]->screenWidth/8, p_bttn->p_scrn[0]->screenHeight/8, 0 );
	GFL_BG_LoadScreenV_Req( p_bttn->bg_frame );

	// ワーク破棄
	for( i=0; i<TOUCH_SW_ANIME_NUM; i++ ){
		GFL_HEAP_FreeMemory( p_bttn->p_scrn_buff[ i ] );
	}

	memset( p_bttn, 0, sizeof( TOUCH_SW_BUTTON ) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータ転送リクエスト
 *
 *	@param	fileIdx		アーカイブファイルインデックス
 *	@param	dataIdx		データインデックス
 *	@param	bgl			BGデータ
 *	@param	frm			ﾌﾚｰﾑナンバー
 *	@param	offs		オフセット
 *	@param	heapID		ヒープ
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_CharTransReq( u32 fileIdx, u32 dataIdx, u32 frm, u32 offs, u32 heapID )
{
	TOUCH_SW_CHARACTER_TRANS* p_tw;

	p_tw = GFL_HEAP_AllocClearMemory( heapID, sizeof(TOUCH_SW_CHARACTER_TRANS) );

	// キャラクタデータ読み込み
	p_tw->p_buff = GFL_ARC_UTIL_LoadBGCharacter( fileIdx, dataIdx, FALSE, &p_tw->p_char, heapID );

	// 転送データ格納
	p_tw->bg_frame = frm;
	p_tw->char_ofs = offs;

	// タスク登録
	GFUser_VIntr_CreateTCB( TouchSW_CharTransTsk, p_tw, TOUCH_SW_VRAM_TRANS_TSK_PRI );
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットデータ転送リクエスト
 *
 *	@param	fileIdx		アーカイブファイルインデックス
 *	@param	dataIdx		データインデックス
 *	@param	palType		パレット転送先
 *	@param	offs		オフセット
 *	@param	transSize	転送size
 *	@param	heapID		ヒープ
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_PlttTransReq( u32 fileIdx, u32 dataIdx, PALTYPE palType, u32 offs, u32 transSize, u32 heapID )
{
	TOUCH_SW_PALETTE_TRANS* p_tw;

	p_tw = GFL_HEAP_AllocClearMemory( heapID, sizeof(TOUCH_SW_PALETTE_TRANS) );

	// キャラクタデータ読み込み
	p_tw->p_buff = GFL_ARC_UTIL_LoadPalette( fileIdx, dataIdx, &p_tw->p_pltt, heapID );

	// 転送データ格納
	p_tw->pltype = palType;
	p_tw->ofs = offs;
	p_tw->size = transSize;

	// タスク登録
	GFUser_VIntr_CreateTCB( TouchSW_PlttTransTsk, p_tw, TOUCH_SW_VRAM_TRANS_TSK_PRI );
}

//----------------------------------------------------------------------------
/**
 *	@brief	キャラクタデータ転送タスク
 *
 *	@param	tcb
 *	@param	p_work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void TouchSW_CharTransTsk( GFL_TCB *tcb, void* p_work )
{
	TOUCH_SW_CHARACTER_TRANS* p_tw = p_work;


	DC_FlushRange( p_tw->p_char->pRawData, p_tw->p_char->szByte );
	GFL_BG_LoadCharacter( p_tw->bg_frame, p_tw->p_char->pRawData, 
						p_tw->p_char->szByte, p_tw->char_ofs );

	// メモリ解放
	GFL_TCB_DeleteTask( tcb );
	GFL_HEAP_FreeMemory( p_tw->p_buff );
	GFL_HEAP_FreeMemory( p_tw );
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットデータ転送タスク
 *
 *	@param	tcb
 *	@param	p_work 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void TouchSW_PlttTransTsk( GFL_TCB *tcb, void* p_work )
{
	TOUCH_SW_PALETTE_TRANS* p_tw = p_work;


	DC_FlushRange( p_tw->p_pltt->pRawData, p_tw->size );

	if( p_tw->pltype == PALTYPE_MAIN_BG ){
		GX_LoadBGPltt( p_tw->p_pltt->pRawData, p_tw->ofs, p_tw->size );
	}else if( p_tw->pltype == PALTYPE_SUB_BG ){
		GXS_LoadBGPltt( p_tw->p_pltt->pRawData, p_tw->ofs, p_tw->size );
	}

	// メモリ解放
	GFL_TCB_DeleteTask( tcb );
	GFL_HEAP_FreeMemory( p_tw->p_buff );
	GFL_HEAP_FreeMemory( p_tw );
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクリーンデータ転送
 *
 *	@param	frame		BGナンバー
 *	@param	scrn		スクリーンワーク
 *	@param	x			ｘ座標
 *	@param	y			ｙ座標
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_ScrnSet( int frame, const NNSG2dScreenData* scrn, int x, int y )
{
	// 書きだし
	GFL_BG_WriteScreenExpand(
				frame,
				x, y,
				scrn->screenWidth / 8, scrn->screenHeight / 8,
				scrn->rawData,
				0, 0,
				scrn->screenWidth / 8, scrn->screenHeight / 8 );
	
	GFL_BG_LoadScreenV_Req( frame );
}

//----------------------------------------------------------------------------
/**
 *
 *	@brief	スクリーンデータのキャラクタネームの値をオフセット分ずらす
 *
 *	@param	scrn			スクリーンデータ
 *	@param	char_offs		キャラクタオフセット	キャラクタ単位
 *
 *	@return	none
 *
 * キャラクタネーム最大値チェックをしていないので注意してください
 *
 */
//-----------------------------------------------------------------------------
static void TouchSW_ScrnCharOfsSet( const NNSG2dScreenData* scrn, int char_offs )
{
	int i;							// ループ用
	u16* scrndata;					// スクリーンデータ
	int size = scrn->szByte / 2;	// ループ回数	２=2byte
	
	// スクリーンデータ代入
	scrndata = (u16*)scrn->rawData;
	for(i=0; i<size; i++){

		scrndata[ i ] += char_offs;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	パレットオフセットをスクリーンバッファに設定
 *
 *	@param	scrn		スクリーンバッファ
 *	@param	pltt_offs	パレットオフセット	(パレット本数単位)
 *
 *	@return	none
 */
//-----------------------------------------------------------------------------
static void TouchSW_ScrnPlttOfsSet( const NNSG2dScreenData* scrn, int pltt_offs )
{
	int i;							// ループ用
	u16* scrndata;					// スクリーンデータ
	int size = scrn->szByte / 2;	// ループ回数	２=2byte
	
	// スクリーンデータ代入
	scrndata = (u16*)scrn->rawData;
	for(i=0; i<size; i++){

		scrndata[ i ] &= 0x0fff;
		scrndata[ i ] |= pltt_offs << 12;
	}
}
