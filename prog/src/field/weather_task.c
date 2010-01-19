//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_task.c
 *	@brief		１天気　動作管理
 *	@author		tomoya takahshi
 *	@date		2009.03.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "weather_task.h"

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
//-------------------------------------
///	WEATHER_TASK
//=====================================
#define WEATHER_TASK_OBJBUFF_MAX	( 48 )

// 状態遷移
enum{
	WEATHER_TASK_SEQ_NONE,				// なし
	WEATHER_TASK_SEQ_INIT_ALL,			// 一気に初期化
	WEATHER_TASK_SEQ_INIT_DIV_OAM,		// 分割読み込み OAM
	WEATHER_TASK_SEQ_INIT_DIV_BG,		// 分割読み込み BG
	WEATHER_TASK_SEQ_INIT_DIV_CREATE,	// 分割読み込み 構築
	WEATHER_TASK_SEQ_CALL_INIT,			// 管理関数　初期化	　		呼び出し
	WEATHER_TASK_SEQ_CALL_FADEIN,		// 管理関数  フェードイン	呼び出し
	WEATHER_TASK_SEQ_CALL_NOFADE,		// 管理関数	 フェードなし	呼び出し
	WEATHER_TASK_SEQ_CALL_MAIN,			// 管理関数　メイン			呼び出し
	WEATHER_TASK_SEQ_CALL_FADEOUT_INIT,	// 管理関数　フェードアウト 呼び出し
	WEATHER_TASK_SEQ_CALL_FADEOUT,		// 管理関数　フェードアウト 呼び出し
	WEATHER_TASK_SEQ_CALL_DEST,			// 管理関数　破棄			呼び出し
	WEATHER_TASK_SEQ_DELETE_ALL,		// 全情報の破棄

	WEATHER_TASK_SEQ_NUM,				// システム内で使用
};


//-------------------------------------
///	グラフィック定数
//=====================================
#define WEATHER_GRAPHIC_OAM_LOAD_DRAW_TYPE	( CLSYS_DRAW_MAIN )


//-------------------------------------
///	天気オブジェ定数
//=====================================
#define WEATHER_OBJ_WORK_BG_PRI		( 1 )
#define WEATHER_OBJ_WORK_SOFT_PRI	( 64 )
#define WEATHER_OBJ_WORK_SERFACE_NO	( CLSYS_DEFREND_MAIN )

#define WEATHER_OBJ_MINTURN_X	(-64)
#define WEATHER_OBJ_MINTURN_Y	(-64)
#define WEATHER_OBJ_MAXTURN_X	((256) + (64))
#define WEATHER_OBJ_MAXTURN_Y	((192) + (64))


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	天気１オブジェ（雪の粒など用　OAM）
//=====================================
struct _WEATHER_OBJ_WORK {
	// あんまり好きな書き方じゃないけど・・・
	// 親のワークポインタ
	WEATHER_TASK* p_parent;

	// 描画ワーク
	GFL_CLWK* p_clwk;

	// ユーザワーク
	u8	user_work[ WEATHER_OBJ_WORK_USE_WORKSIZE ];

	// リンク情報
	WEATHER_OBJ_WORK* p_last;
	WEATHER_OBJ_WORK* p_next;
};

//-------------------------------------
///	グラフィック情報
//=====================================
typedef struct {
	u16 oam_load;
	u16	oam_cgx_id;
	u16	oam_pltt_id;
	u16 oam_cell_id;

	u32 bg_load[WEATHER_TASK_3DBG_NUM];

	ARCHANDLE* p_handle;
} WEATHER_TASK_GRAPHIC;

//-------------------------------------
///	ループ音情報
//=====================================
typedef struct {
	u16	play;
	u16 snd_no;
} WEATHER_TASK_SND_LOOP;


//-------------------------------------
///	オブジェクト追加フェード
//=====================================
typedef struct {
	WEATHER_TASK* p_parent;

	// カウンタ
	s16 obj_addnum;		// オブジェクト登録数
	s16 obj_addtmg;		// 登録タイミングカウンタ
	s16 obj_addtmg_max;	// 登録タイミング
	s16 obj_addnum_sum;	// オブジェクト登録数加減カウンタ
	
	// 定数
	s16	obj_add_num_end;		// オブジェクト１回の登録数　終了値
	s16	obj_add_tmg_end;		// 登録タイミングカウンタ　終了値
	s16	obj_add_tmg_sum;		// 登録タイミングカウンタ変更値
	s16 obj_add_num_sum_tmg;	// オブジェクト１回の登録数を変更するタイミング
	s32 obj_add_num_sum;		// 変更する値

	// 関数
	WEATHER_TASK_OBJADD_FUNC* p_addfunc;

} WEATHER_TASK_OBJ_FADE;

//-------------------------------------
///	スクロール座標管理
//=====================================
typedef struct {
	VecFx32 last_target;
} WEATHER_TASK_SCROLL_WORK;


//-------------------------------------
///	天気タスク
//=====================================
struct _WEATHER_TASK {

	/* MemClearしないワーク 
	 * こっちにメンバを増やすときは、
	 * WEATHER_TASK_WK_Clear関数にコードを追加すること*/
	// GFL_CLUNIT
	GFL_CLUNIT* p_unit;
	
	// フォグ情報
	FIELD_FOG_WORK*	p_fog;
	
	// ライト情報
	FIELD_LIGHT* p_light;

	// サウンド情報
	const FIELD_SOUND* cp_sound;

	// ゾーン用フォグ、ライト情報
	const FIELD_ZONEFOGLIGHT*	cp_zonefog;
	
	// カメラ情報
	const FIELD_CAMERA* cp_camera;

	// 3DBGシステム
	FIELD_3DBG** pp_3dbg;
	

	/* MemClearするワーク */
	
	// 天気基本情報
	const WEATHER_TASK_DATA*	cp_data;
	u8							fog_cont;	// フォグ操作の許可
	u8							fade;		// フェード有無
	u8							init_mode;	// 初期化モード
	u8							seq;		// 全体シーケンス

	// グラフィック情報
	WEATHER_TASK_GRAPHIC graphic;
	
	
	// ユーザワーク
	void*	p_user_work;

	// 音情報
	WEATHER_TASK_SND_LOOP	snd_loop;

	// 動作オブジェ追加フェードシステム
	WEATHER_TASK_OBJ_FADE obj_fade;

	// スクロール管理ワーク
	WEATHER_TASK_SCROLL_WORK scroll;
	
	// 動作オブジェワーク
	WEATHER_OBJ_WORK	objbuff[ WEATHER_TASK_OBJBUFF_MAX ];
	WEATHER_OBJ_WORK*	p_objlist;
	u32					active_objnum;

};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	システム管理
//=====================================
static void WEATHER_TASK_WK_Clear( WEATHER_TASK* p_wk );
static WEATHER_TASK_FUNC_RESULT WEATHER_TASK_WK_CallFunc( WEATHER_TASK* p_wk, WEATHER_TASK_FUNC* p_func, u32 heapID );
static WEATHER_TASK_INFO WEATHER_TASK_WK_GetInfo( const WEATHER_TASK* cp_wk );
static WEATHER_OBJ_WORK* WEATHER_TASK_WK_GetClearObj( WEATHER_TASK* p_wk );
static void WEATHER_TASK_WK_PushObjList( WEATHER_TASK* p_wk, WEATHER_OBJ_WORK* p_obj );
static void WEATHER_TASK_WK_PopObjList( WEATHER_TASK* p_wk, WEATHER_OBJ_WORK* p_obj );
static void WEATHER_TASK_WK_MainObjList( WEATHER_TASK* p_wk );
static void WEATHER_TASK_WK_InitOam( WEATHER_TASK* p_wk, u32 heapID );
static void WEATHER_TASK_WK_ExitOam( WEATHER_TASK* p_wk );
static void WEATHER_TASK_WK_InitBg( WEATHER_TASK* p_wk, u32 heapID );
static void WEATHER_TASK_WK_ExitBg( WEATHER_TASK* p_wk );
static void WEATHER_TASK_WK_InitOther( WEATHER_TASK* p_wk, u32 heapID );
static void WEATHER_TASK_WK_ExitOther( WEATHER_TASK* p_wk );


//-------------------------------------
///	グラフィックシステム
//=====================================
static void WEATHER_TASK_GRAPHIC_Clear( WEATHER_TASK_GRAPHIC* p_wk );
static void WEATHER_TASK_GRAPHIC_InitHandle( WEATHER_TASK_GRAPHIC* p_wk, const WEATHER_TASK_DATA* cp_data, u32 heapID );
static void WEATHER_TASK_GRAPHIC_ExitHandle( WEATHER_TASK_GRAPHIC* p_wk );
static void WEATHER_TASK_GRAPHIC_InitOam( WEATHER_TASK_GRAPHIC* p_wk, const WEATHER_TASK_DATA* cp_data, u32 heapID );
static void WEATHER_TASK_GRAPHIC_ExitOam( WEATHER_TASK_GRAPHIC* p_wk );
static void WEATHER_TASK_GRAPHIC_InitBg( WEATHER_TASK_GRAPHIC* p_wk, WEATHER_TASK_3DBG_TYPE type, FIELD_3DBG* p_3dbg, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID );
static void WEATHER_TASK_GRAPHIC_ExitBg( WEATHER_TASK_GRAPHIC* p_wk, WEATHER_TASK_3DBG_TYPE type, FIELD_3DBG* p_3dbg );


//-------------------------------------
///	オブジェクト追加フェード
//=====================================
static void WEATHER_OBJ_FADE_InitFade( WEATHER_TASK_OBJ_FADE* p_wk, WEATHER_TASK* p_sys, s32 objAddTmgMax, s32 objAddNum, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum_tmg, s32 obj_add_num_sum, WEATHER_TASK_OBJADD_FUNC* p_addfunc );

static void WEATHER_OBJ_FADE_SetFadeOut( WEATHER_TASK_OBJ_FADE* p_wk, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum );
static BOOL WEATHER_OBJ_FADE_Main( WEATHER_TASK_OBJ_FADE* p_wk, u32 heapID );
static BOOL WEATHER_TASK_OBJ_FADE_NoFadeMain( WEATHER_TASK_OBJ_FADE* p_wk, u32 heapID );


//-------------------------------------
///	天気オブジェワーク管理
//=====================================
static void WEATHER_OBJ_WK_Clear( WEATHER_OBJ_WORK* p_wk );
static BOOL WEATHER_OBJ_WK_IsUse( const WEATHER_OBJ_WORK* cp_wk );
static void WEATHER_OBJ_WK_Init( WEATHER_OBJ_WORK* p_wk, WEATHER_TASK* p_parent, const WEATHER_TASK_GRAPHIC* cp_graphic, GFL_CLUNIT* p_unit, u32 heapID );
static void WEATHER_OBJ_WK_Exit( WEATHER_OBJ_WORK* p_wk );
static void* WEATHER_OBJ_WK_GetWork( const WEATHER_OBJ_WORK* cp_wk );
static GFL_CLWK* WEATHER_OBJ_WK_GetClWk( const WEATHER_OBJ_WORK* cp_wk );


//-------------------------------------
///	サウンドシステム
//=====================================
static void WEATHER_SND_LOOP_Play( WEATHER_TASK_SND_LOOP* p_wk, int snd_no );
static void WEATHER_SND_LOOP_Stop( WEATHER_TASK_SND_LOOP* p_wk );


//-------------------------------------
///	ツール
//=====================================
static void TOOL_GetPerspectiveScreenSize( const MtxFx44* cp_pers_mtx, fx32 dist, fx32* p_width, fx32* p_height );





//----------------------------------------------------------------------------
/**
 *	@brief	天気タスクシステム初期化
 *
 *	@param	p_clunit		セルアクターユニット
 *	@param	cp_camera		フィールドカメラ
 *	@param	p_light			フィールドライト
 *	@param	p_fog			フィールドフォグ
 *	@param	p_zonefog		ゾーン用フォグライト除法
 *	@param	pp_3dbg			3DBGシステム
 *	@param	p_3dbg_back	3DBGシステム
 *	@param  cp_sound    サウンドシステム
 *	@param	heapID			ヒープID
 *	
 *	@return	天気タスクワーク
 */
//-----------------------------------------------------------------------------
WEATHER_TASK* WEATHER_TASK_Init( GFL_CLUNIT* p_clunit, const FIELD_CAMERA* cp_camera, FIELD_LIGHT* p_light, FIELD_FOG_WORK* p_fog, const FIELD_ZONEFOGLIGHT* cp_zonefog, FIELD_3DBG** pp_3dbg, const FIELD_SOUND* cp_sound, u32 heapID )
{
	WEATHER_TASK* p_wk;

	p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(WEATHER_TASK) );

	// 基本情報設定
	p_wk->p_unit		= p_clunit;	
	p_wk->cp_camera		= cp_camera;	
	p_wk->p_fog			= p_fog;
	p_wk->p_light		= p_light;
	p_wk->cp_zonefog	= cp_zonefog;
	p_wk->pp_3dbg		= pp_3dbg;
	p_wk->cp_sound	= cp_sound;

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	天気タスクシステム破棄
 *
 *	@param	p_wk		システムワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_Exit( WEATHER_TASK* p_wk )
{
	// 実行天気の破棄
	WEATHER_TASK_ForceEnd( p_wk );

	// 破棄
	GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_Main( WEATHER_TASK* p_wk, u32 heapID )
{
	// 処理分岐
	switch( p_wk->seq ){
	// なし
	case WEATHER_TASK_SEQ_NONE:				
		break;
		
	// 一気に初期化
	case WEATHER_TASK_SEQ_INIT_ALL:			
		WEATHER_TASK_WK_InitOam( p_wk, heapID );
		WEATHER_TASK_WK_InitBg( p_wk, heapID );
		WEATHER_TASK_WK_InitOther( p_wk, heapID );

		p_wk->seq = WEATHER_TASK_SEQ_CALL_INIT;
		break;

	// 分割読み込み OAM
	case WEATHER_TASK_SEQ_INIT_DIV_OAM:		
		WEATHER_TASK_WK_InitOam( p_wk, heapID );

		p_wk->seq = WEATHER_TASK_SEQ_INIT_DIV_BG;
		break;

	// 分割読み込み BG
	case WEATHER_TASK_SEQ_INIT_DIV_BG:		
		WEATHER_TASK_WK_InitBg( p_wk, heapID );

		p_wk->seq = WEATHER_TASK_SEQ_INIT_DIV_CREATE;
		break;

	// 分割読み込み 構築
	case WEATHER_TASK_SEQ_INIT_DIV_CREATE:	
		WEATHER_TASK_WK_InitOther( p_wk, heapID );

		p_wk->seq = WEATHER_TASK_SEQ_CALL_INIT;
		break;

	// 管理関数　初期化	　		呼び出し
	case WEATHER_TASK_SEQ_CALL_INIT:		
		if( WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_init, heapID ) ){
			if( p_wk->fade ){
				p_wk->seq = WEATHER_TASK_SEQ_CALL_FADEIN;
			}else{
				p_wk->seq = WEATHER_TASK_SEQ_CALL_NOFADE;
			}
		}
		break;

	// 管理関数  フェードイン	呼び出し
	case WEATHER_TASK_SEQ_CALL_FADEIN:

    // BGM 読み込み中は、この処理を行わない
    if( PMSND_IsLoading() )
    {
      TOMOYA_Printf( "Weather BGM wait...\n" );
      break;
    }
    
		if( WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_fadein, heapID ) ){
			p_wk->seq = WEATHER_TASK_SEQ_CALL_MAIN;
		}
		WEATHER_TASK_WK_MainObjList( p_wk );
		break;

	// 管理関数	 フェードなし	呼び出し
	case WEATHER_TASK_SEQ_CALL_NOFADE:		

    // BGM 読み込み中は、この処理を行わない
    if( PMSND_IsLoading() )
    {
      TOMOYA_Printf( "Weather BGM wait...\n" );
      break;
    }
    
		if( WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_nofade, heapID ) ){
			p_wk->seq = WEATHER_TASK_SEQ_CALL_MAIN;
		}
		break;

	// 管理関数　メイン			呼び出し
	case WEATHER_TASK_SEQ_CALL_MAIN:		
		// *外部的なリクエストで、終了する
		WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_main, heapID );
		WEATHER_TASK_WK_MainObjList( p_wk );
		break;

	// 管理関数　フェードアウト 呼び出し
	case WEATHER_TASK_SEQ_CALL_FADEOUT_INIT:
		if( WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_init_fadeout, heapID ) ){
			p_wk->seq = WEATHER_TASK_SEQ_CALL_FADEOUT;
		}
		WEATHER_TASK_WK_MainObjList( p_wk );
		break;

	// 管理関数　フェードアウト 呼び出し
	case WEATHER_TASK_SEQ_CALL_FADEOUT:		
		if( WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_fadeout, heapID ) ){
			p_wk->seq = WEATHER_TASK_SEQ_CALL_DEST;
		}
		WEATHER_TASK_WK_MainObjList( p_wk );
		break;

	// 管理関数　破棄			呼び出し
	// １回で終わる必要がある
	case WEATHER_TASK_SEQ_CALL_DEST:
		{
			WEATHER_TASK_FUNC_RESULT result;
			result = WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_dest, heapID );

			// 破棄処理は１シンクで終わる必要があります。
			GF_ASSERT( result == WEATHER_TASK_FUNC_RESULT_FINISH );
			p_wk->seq = WEATHER_TASK_SEQ_DELETE_ALL;
		}
		break;

	// 全情報の破棄
	//
	// *破棄処理は、WEATHER_TASK_ForceEndにもあります。
	case WEATHER_TASK_SEQ_DELETE_ALL:		
		WEATHER_TASK_WK_ExitOther( p_wk );
		WEATHER_TASK_WK_ExitBg( p_wk );
		WEATHER_TASK_WK_ExitOam( p_wk );
		WEATHER_TASK_WK_Clear( p_wk );

		p_wk->seq = WEATHER_TASK_SEQ_NONE;
		break;

	default:
		// ありえない
		GF_ASSERT(0);
		break;
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	天気発動
 *
 *	@param	p_wk			ワーク
 *	@param	cp_data			天気データ
 *	@param	init_mode		初期化モード
 *	@param	fade			フェードの有無	　TRUE：あり　FALSE：なし
 *	@param	fog_cont		フォグ管理のモード
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_Start( WEATHER_TASK* p_wk, const WEATHER_TASK_DATA* cp_data, WEATEHR_TASK_INIT_MODE init_mode, BOOL fade, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	GF_ASSERT( p_wk->seq == WEATHER_TASK_SEQ_NONE );
	
	// 情報の設定
	p_wk->cp_data		= cp_data;
	p_wk->fog_cont		= fog_cont;
	p_wk->fade			= fade;
	p_wk->init_mode		= init_mode;


	// 初期化シーケンス設定
	if( p_wk->init_mode == WEATHER_TASK_INIT_NORMAL ){
		p_wk->seq = WEATHER_TASK_SEQ_INIT_ALL;
	}
	else if( p_wk->init_mode == WEATHER_TASK_INIT_DIV ){
		p_wk->seq = WEATHER_TASK_SEQ_INIT_DIV_OAM;
	}
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	天気破棄
 *
 *	@param	p_wk			ワーク
 *	@param	fade			フェードの有無	　TRUE：あり　FALSE：なし
 *	@param	fog_cont		フォグ管理のモード
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_End( WEATHER_TASK* p_wk, BOOL fade, WEATHER_TASK_FOG_MODE fog_cont )
{
	u32 info;

	info = WEATHER_TASK_WK_GetInfo( p_wk );

	// 動作中かチェック
	if( p_wk->seq == WEATHER_TASK_SEQ_NONE ){
		return ;
	}
	
	// 破棄中チェック
	if( (info == WEATHER_TASK_INFO_DEST) || 
		(info == WEATHER_TASK_INFO_FADEOUT) ){

		// もう破棄中
		GF_ASSERT( 0 );
		return ;
	}

	// それぞれの破棄へ
	p_wk->fade		= fade;
	p_wk->fog_cont	= fog_cont;

	// 初期化中なら直ぐ破棄へ
	if( p_wk->seq <= WEATHER_TASK_SEQ_CALL_INIT ){
		p_wk->seq = WEATHER_TASK_SEQ_CALL_DEST;
	}else{

		if( p_wk->fade ){
			p_wk->seq = WEATHER_TASK_SEQ_CALL_FADEOUT_INIT;
		}else{
			p_wk->seq = WEATHER_TASK_SEQ_CALL_DEST;
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	強制終了	（この関数を呼んだ瞬間に全破棄します。）
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_ForceEnd( WEATHER_TASK* p_wk )
{
	if( p_wk->seq != WEATHER_TASK_SEQ_NONE ){
		// callback dest
		if( p_wk->seq <= WEATHER_TASK_SEQ_CALL_DEST ){
			WEATHER_TASK_FUNC_RESULT result;
			// さすがに破棄処理でヒープ確保はしないだろう
			result = WEATHER_TASK_WK_CallFunc( p_wk, p_wk->cp_data->p_f_dest, 0 );

			// 破棄処理は１シンクで終わる必要があります。
			GF_ASSERT( result == WEATHER_TASK_FUNC_RESULT_FINISH );
		}

		// delete all 
		if( p_wk->seq <= WEATHER_TASK_SEQ_DELETE_ALL ){
			WEATHER_TASK_WK_ExitOther( p_wk );
			WEATHER_TASK_WK_ExitBg( p_wk );
			WEATHER_TASK_WK_ExitOam( p_wk );
		}

		WEATHER_TASK_WK_Clear( p_wk );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	天気用ライト反映
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_LIGHT_Change( WEATHER_TASK* p_wk, u32 arcid, u32 dataid )
{
	FIELD_LIGHT_ChangeEx( p_wk->p_light, arcid, dataid, LIGHT_FADE_COUNT_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  天気用ライト反映その場で変わります。
 *
 *	@param	p_wk
 *	@param	arcid
 *	@param	dataid
 *	@param	heapID 
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_LIGHT_Set( WEATHER_TASK* p_wk, u32 arcid, u32 dataid )
{
	FIELD_LIGHT_ChangeEx( p_wk->p_light, arcid, dataid, 1 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フィールド用ライトに戻す
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_LIGHT_Back( WEATHER_TASK* p_wk )
{
	FIELD_LIGHT_ReLoadDefault( p_wk->p_light );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カラーフェード開始
 *
 *	@param	p_wk		ワーク
 *	@param	color		カラー
 *	@param	insync		インシンク
 *	@param	outsync		アウトシンク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_LIGHT_StartColorFade( WEATHER_TASK* p_wk, GXRgb color, u16 insync, u16 outsync )
{
	FIELD_LIGHT_COLORFADE_Start( p_wk->p_light, color, insync, outsync );
}

//----------------------------------------------------------------------------
/**
 *	@brief	カラーフェードチェック
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_LIGHT_IsColorFade( const WEATHER_TASK* cp_wk )
{
	return FIELD_LIGHT_COLORFADE_IsFade( cp_wk->p_light );
}

//----------------------------------------------------------------------------
/**
 *	@brief  カラーフェード　1方向のみ
 *
 *	@param	p_wk      ワーク
 *	@param	color     色
 *	@param	sync      シンク数
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_LIGHT_StartColorFadeOneWay( WEATHER_TASK* p_wk, GXRgb color, u16 sync )
{
  FIELD_LIGHT_COLORFADE_StartOneWay( p_wk->p_light, color, sync );
}


//----------------------------------------------------------------------------
/**
 *	@brief	3DBGシステム　描画設定
 *
 *	@param	p_wk		ワーク		
 *	@param	visible		フラグ
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_3DBG_SetVisible( WEATHER_TASK* p_wk, BOOL visible, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	FIELD_3DBG_SetVisible( p_wk->pp_3dbg[type], visible );
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBGシステム	描画状態取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	表示
 *	@retval	FALSE	非表示
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_3DBG_GetVisible( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	return FIELD_3DBG_GetVisible( cp_wk->pp_3dbg[type] );
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBGシステム	アルファ設定
 *
 *	@param	p_wk		ワーク
 *	@param	alpha		アルファ
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_3DBG_SetAlpha( WEATHER_TASK* p_wk, u8 alpha, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	FIELD_3DBG_SetAlpha( p_wk->pp_3dbg[type], alpha );
}

//----------------------------------------------------------------------------
/**
 *	@brief	3DBGシステム	アルファ取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	アルファ値
 */
//-----------------------------------------------------------------------------
u8 WEATHER_TASK_3DBG_GetAlpha( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	return FIELD_3DBG_GetAlpha( cp_wk->pp_3dbg[type] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ３DBG　Xスケール設定
 *
 *	@param	p_wk  ワーク
 *	@param	scale スケール値
 *	@param	type  タイプ
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_3DBG_SetScaleX( WEATHER_TASK* p_wk, fx32 scale, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
  FIELD_3DBG_SetScaleX( p_wk->pp_3dbg[type], scale );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ３DBG　Yスケール設定
 *
 *	@param	p_wk    ワーク
 *	@param	scale   スケール値
 *	@param	type    タイプ
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_3DBG_SetScaleY( WEATHER_TASK* p_wk, fx32 scale, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
  FIELD_3DBG_SetScaleY( p_wk->pp_3dbg[type], scale );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ３DBG　Xスケール値取得
 *
 *	@param	cp_wk ワーク
 *	@param	type  タイプ
 *
 *	@return スケール値
 */
//-----------------------------------------------------------------------------
fx32 WEATHER_TASK_3DBG_GetScaleX( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
  return FIELD_3DBG_GetScaleX( cp_wk->pp_3dbg[type] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ３DBG　Yスケール値取得
 *
 *	@param	cp_wk   ワーク
 *	@param	type    タイプ
 *
 *	@return スケール値
 */
//-----------------------------------------------------------------------------
fx32 WEATHER_TASK_3DBG_GetScaleY( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
  return FIELD_3DBG_GetScaleY( cp_wk->pp_3dbg[type] );
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロール座標の設定・取得
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_3DBG_SetScrollX( WEATHER_TASK* p_wk, s32 x, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	FIELD_3DBG_SetScrollX( p_wk->pp_3dbg[type], x );
}
void WEATHER_TASK_3DBG_SetScrollY( WEATHER_TASK* p_wk, s32 y, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	FIELD_3DBG_SetScrollY( p_wk->pp_3dbg[type], y );
}
s32 WEATHER_TASK_3DBG_GetScrollX( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	return FIELD_3DBG_GetScrollX( cp_wk->pp_3dbg[type] );
}
s32 WEATHER_TASK_3DBG_GetScrollY( const WEATHER_TASK* cp_wk, WEATHER_TASK_3DBG_TYPE type )
{
  GF_ASSERT( type < WEATHER_TASK_3DBG_NUM );
	return FIELD_3DBG_GetScrollY( cp_wk->pp_3dbg[type] );
}



//----------------------------------------------------------------------------
/**
 *	@brief	天気情報の取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	WEATHER_TASK_INFO_NONE,				// 情報なし
 *	@retval	WEATHER_TASK_INFO_LOADING,			// 読み込み中
 *	@retval	WEATHER_TASK_INFO_FADEIN,			// フェードイン中
 *	@retval	WEATHER_TASK_INFO_MAIN,				// メイン動作中
 *	@retval	WEATHER_TASK_INFO_FADEOUT,			// フェードアウト中
 *	@retval	WEATHER_TASK_INFO_DEST,				// 破棄中
 */
//-----------------------------------------------------------------------------
WEATHER_TASK_INFO WEATHER_TASK_GetInfo( const WEATHER_TASK* cp_wk )
{
	return WEATHER_TASK_WK_GetInfo( cp_wk );	
}

//----------------------------------------------------------------------------
/**
 *	@brief	天気	ユーザワークの取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	ユーザワーク	WEATHER_TASK_DATA.work_byte == 0の場合NULL
 */
//-----------------------------------------------------------------------------
void* WEATHER_TASK_GetWorkData( const WEATHER_TASK* cp_wk )
{
	return  cp_wk->p_user_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief	動作しているオブジェクトの数を取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	動作しているオブジェクトの数
 */
//-----------------------------------------------------------------------------
u32 WEATHER_TASK_GetActiveObjNum( const WEATHER_TASK* cp_wk )
{
	return cp_wk->active_objnum;
}


//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加処理
 *
 *	@param	p_wk			ワーク
 *	@param	heapID			ヒープID
 *
 *	@retval	追加オブジェ
 *	@retval	NULL			もう追加できない
 */
//-----------------------------------------------------------------------------
WEATHER_OBJ_WORK* WEATHER_TASK_CreateObj( WEATHER_TASK* p_wk, u32 heapID )
{
	WEATHER_OBJ_WORK* p_obj;

	p_obj = WEATHER_TASK_WK_GetClearObj( p_wk );
	if( p_obj ){

		// 初期化
//		WEATHER_OBJ_WK_Init( p_obj, p_wk, &p_wk->graphic, p_wk->p_unit, heapID );

		// リンクリストに設定
		WEATHER_TASK_WK_PushObjList( p_wk, p_obj );
	}
	
	return p_obj;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトの破棄処理
 *
 *	@param	p_obj	ワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_DeleteObj( WEATHER_OBJ_WORK* p_obj )
{
	if( WEATHER_OBJ_WK_IsUse( p_obj ) ){
		// リストから破棄
		WEATHER_TASK_WK_PopObjList( p_obj->p_parent, p_obj );

		// 破棄
//		WEATHER_OBJ_WK_Exit( p_obj );

		//  ワーククリア
//		WEATHER_OBJ_WK_Clear( p_obj );
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェード　フェードイン開始
 *
 *	@param	p_wk						システムワーク
 *	@param	objAddNum					オブジェ追加数
 *	@param	objAddTmgMax				オブジェ追加タイミングマックス
 *	@param	obj_add_num_end				オブジェ追加数　終了値
 *	@param	obj_add_tmg_end				オブジェ追加タイミング　終了値
 *	@param	obj_add_tmg_sum				オブジェ追加タイミング　足し算値
 *	@param	obj_add_num_sum_tmg			オブジェ追加タイミング　足し算タイミング
 *	@param	obj_add_num_sum				オブジェ追加数　加算値
 *	@param	p_addfunc					追加関数
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_ObjFade_Init( WEATHER_TASK* p_wk, s32 objAddNum, s32 objAddTmgMax, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum_tmg, s32 obj_add_num_sum, WEATHER_TASK_OBJADD_FUNC* p_addfunc )
{
	WEATHER_OBJ_FADE_InitFade( &p_wk->obj_fade, p_wk, objAddTmgMax, objAddNum, obj_add_num_end, obj_add_tmg_end, obj_add_tmg_sum, obj_add_num_sum_tmg, obj_add_num_sum, p_addfunc );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェード	フェードアウト開始
 *
 *	@param	p_wk					ワーク
 *	@param	obj_add_num_end				オブジェ追加数　終了値
 *	@param	obj_add_tmg_end				オブジェ追加タイミング　終了値
 *	@param	obj_add_tmg_sum				オブジェ追加タイミング　足し算値
 *	@param	obj_add_num_sum				オブジェ追加数　加算値
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_ObjFade_SetOut( WEATHER_TASK* p_wk, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum )
{
	WEATHER_OBJ_FADE_SetFadeOut( &p_wk->obj_fade, obj_add_num_end, obj_add_tmg_end, obj_add_tmg_sum, obj_add_num_sum );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェード	メイン
 *
 *	@param	p_wk				ワーク
 *
 *	@retval	TRUE	完了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_ObjFade_Main( WEATHER_TASK* p_wk, u32 heapID )
{
	return WEATHER_OBJ_FADE_Main( &p_wk->obj_fade, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードはせず、オブジェの同じ登録タイミングで、同じ数　オブジェを登録します。
 *
 *	@param	p_wk	ワーク
 *
 *	@retval	TRUE	登録した
 *	@retval	FALSE	登録してない
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_ObjFade_NoFadeMain( WEATHER_TASK* p_wk, u32 heapID )
{
	return WEATHER_TASK_OBJ_FADE_NoFadeMain( &p_wk->obj_fade, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェを分散させる
 *
 *	@param	p_wk				ワーク
 *	@param	o_add_func			追加関数
 *	@param	num					登録数
 *	@param	dust_div_num		何オブジェクトごとに動作数を変更するか
 *	@param	dust_div_move		各オブジェクトの動作数をどのくらい離すか
 *
 * num分のオブジェクトを登録し、0番目のオブジェクトからdust_div_move回数分
 * 多くオブジェクトを動作させる
 * 0番目のオブジェクト　0
 * 1番目のオブジェクト	dust_div_move
 * 2番目のオブジェクト	dust_div_move * 2
 * ....
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_DustObj( WEATHER_TASK* p_wk, WEATHER_TASK_OBJADD_FUNC* p_add_func, int num, int dust_div_num, int dust_div_move, u32 heapID )
{
	int i, j;
	int move_num;
	WEATHER_OBJ_WORK* p_obj;
	WEATHER_OBJ_WORK* p_obj_next;
	
	// オブジェクト登録
	p_add_func( p_wk, num, heapID );


	// オブジェクトを進める
	move_num = 0;
	p_obj = p_wk->p_objlist;
	p_obj_next = p_obj->p_next;
	for( i=0; i<num; i++ ){	
		
		for( j=0; j<move_num; j++ ){
			p_wk->cp_data->p_f_objmove( p_obj );

			// 破棄されたら終わり
			if( !WEATHER_OBJ_WK_IsUse(p_obj) ){
				break;
			}
		}

		// 次へ
		p_obj = p_obj_next;
		p_obj_next = p_obj->p_next;	// move_func内でオブジェクトが破棄されることもあるため、次のオブジェクトポインタを保存しておく

		// 動作数変更計算
		if( i >= dust_div_num ){
			if( (i % dust_div_num) == 0 ){
				move_num += dust_div_move;
			}
		}

		
		// 登録失敗しているときはnum分のオブジェクトが登録されていない
		if( (u32)p_obj == (u32)p_wk->p_objlist ){
			break;
		}
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	フォグ状態の設定
 *
 *	@param	p_wk			ワーク
 *	@param	fog_slope		フォグかかり具合
 *	@param	fog_offs		フォグオフセット
 *	@param	mode 
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_FogSet( WEATHER_TASK* p_wk, FIELD_FOG_SLOPE fog_slope, int fog_offs, WEATHER_TASK_FOG_MODE mode )
{
	if( mode == WEATHER_TASK_FOG_USE ){

		FIELD_FOG_SetBlendMode( p_wk->p_fog, FIELD_FOG_BLEND_COLOR_ALPHA );
		FIELD_FOG_SetColorA( p_wk->p_fog, 31 );
		FIELD_FOG_SetSlope( p_wk->p_fog, fog_slope );
		FIELD_FOG_SetOffset( p_wk->p_fog, fog_offs );
		FIELD_FOG_TBL_SetUpDefault( p_wk->p_fog );
		FIELD_FOG_SetFlag( p_wk->p_fog, TRUE );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグクリア処理
 *
 *	@param	p_wk	ワーク
 *	@param	mode	フォグ管理モード
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_FogClear( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE mode )
{
	if( mode == WEATHER_TASK_FOG_USE ){
		FIELD_FOG_SetFlag( p_wk->p_fog, FALSE );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグ	フェードイン開始
 *
 *	@param	p_wk			ワーク
 *	@param	fog_slope		フォグかかり具合
 *	@param	fog_offs		フォグオフセット
 *	@param	timing			使用するシンク数
 *	@param	mode			モード
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_FogFadeIn_Init( WEATHER_TASK* p_wk, FIELD_FOG_SLOPE fog_slope, int fog_offs, int timing, WEATHER_TASK_FOG_MODE mode )
{
	if( mode != WEATHER_TASK_FOG_NONE ){
		FIELD_FOG_FADE_Init( p_wk->p_fog, fog_offs, fog_slope, timing );
	}
}
	
//----------------------------------------------------------------------------
/**
 *	@brief	フォグフェードアウト	開始
 *
 *	@param	p_wk		ワーク
 *	@param	fog_offs	フォグオフセット
 *	@param	timing		使用するシンク数
 *	@param	mode		モード
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_FogFadeOut_Init( WEATHER_TASK* p_wk, int fog_offs, int timing, WEATHER_TASK_FOG_MODE mode )
{
	if( mode == WEATHER_TASK_FOG_USE ){
		FIELD_FOG_FADE_Init( p_wk->p_fog, fog_offs, FIELD_FOG_GetSlope( p_wk->p_fog ), timing );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	フォグフェードメイン
 *
 *	@param	p_wk		ワーク
 *
 *	@retval	TRUE	完了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
extern BOOL WEATHER_TASK_FogFade_IsFade( const WEATHER_TASK* cp_wk )
{
	if( FIELD_FOG_FADE_IsFade( cp_wk->p_fog ) ){
		return FALSE;	
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグ情報の有無
 *
 *	@param	cp_wk		ワーク
 *
 *	@retval	TRUE	ある
 *	@retval	FALSE	ない
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_IsZoneFog( const WEATHER_TASK* cp_wk )
{
	return FIELD_ZONEFOGLIGHT_IsFogData( cp_wk->cp_zonefog );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンライト情報の有無
 *
 *	@param	cp_wk		ワーク
 */
//-----------------------------------------------------------------------------
BOOL WEATHER_TASK_IsZoneLight( const WEATHER_TASK* cp_wk )
{
	return FIELD_ZONEFOGLIGHT_IsLightData( cp_wk->cp_zonefog );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ゾーンフォグオフセット情報の取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	オフセット情報
 */
//-----------------------------------------------------------------------------
s32 WEATHER_TASK_GetZoneFogOffset( const WEATHER_TASK* cp_wk )
{
	return FIELD_ZONEFOGLIGHT_GetOffset( cp_wk->cp_zonefog );
}

//----------------------------------------------------------------------------
/**
 *	@brief		ゾーンフォグスロープ情報の取得
 */
//-----------------------------------------------------------------------------
u32 WEATHER_TASK_GetZoneFogSlope( const WEATHER_TASK* cp_wk )
{
	return FIELD_ZONEFOGLIGHT_GetSlope( cp_wk->cp_zonefog );
}

//----------------------------------------------------------------------------
/**
 *	@brief		ゾーンライトデータIDの取得
 */
//-----------------------------------------------------------------------------
u32 WEATHER_TASK_GetZoneLight( const WEATHER_TASK* cp_wk )
{
	return FIELD_ZONEFOGLIGHT_GetLightIndex( cp_wk->cp_zonefog );
}


//----------------------------------------------------------------------------
/**
 *	@brief	FOGブレンドモード設定
 *
 *	@param	p_wk		ワーク
 *	@param	blend		ブレンド
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_Fog_SetBlendMode( WEATHER_TASK* p_wk, FIELD_FOG_BLEND blend )
{
	FIELD_FOG_SetBlendMode( p_wk->p_fog, blend );
}

//----------------------------------------------------------------------------
/**
 *	@brief	FOGブレンドモード取得
 *
 *	@param	cp_wk		ワーク
 *
 *	@return	ブレンドモード
 */
//-----------------------------------------------------------------------------
FIELD_FOG_BLEND WEATHER_TASK_Fog_GetBlendMode( const WEATHER_TASK* cp_wk )
{
	return FIELD_FOG_GetBlendMode( cp_wk->p_fog );
}

//----------------------------------------------------------------------------
/**
 *	@brief	FOGブレンドアルファ値設定
 *
 *	@param	p_wk		ワーク
 *	@param	alpha		アルファ値
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_Fog_SetAlpha( WEATHER_TASK* p_wk, u8 alpha )
{
	FIELD_FOG_SetColorA( p_wk->p_fog, alpha );
}

//----------------------------------------------------------------------------
/**
 *	@brief	FOGブレンドアルファ値取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	アルファ値
 */
//-----------------------------------------------------------------------------
u8 WEATHER_TASK_Fog_GetAlpha( const WEATHER_TASK* cp_wk )
{
	return FIELD_FOG_GetColorA( cp_wk->p_fog );
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロール距離　計算ワーク　初期化
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_InitScrollDist( WEATHER_TASK* p_wk )
{
	VecFx32	now_mat;				// 今の座標
	const GFL_G3D_CAMERA* cp_camera;// カメラ情報

	if( !p_wk->cp_camera ){
		return;
	}

	//  カメラ情報の取得
	cp_camera = FIELD_CAMERA_GetCameraPtr( p_wk->cp_camera );
	GFL_G3D_CAMERA_GetTarget( cp_camera, &now_mat );

	p_wk->scroll.last_target = now_mat;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGやOAMなど２D素材のスクロール距離を求める
 *
 *	@param	p_wk			ワーク
 *	@param	p_x				ｘ距離格納先
 *	@param	p_y				ｙ距離格納先
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_GetScrollDist( WEATHER_TASK* p_wk, int* p_x, int* p_y )
{
	VecFx32	now_mat;				// 今の座標
	VecFx32	now_camera_pos;			// 今のカメラ座標
	fx32 scl_x, scl_y;				// Scroll座標
	fx32 d_x, d_y;					// 2dで１の３ｄの値
	fx32 dist_x, dist_y;			// 移動した値
	int	mark;						// 符号
	const GFL_G3D_CAMERA* cp_camera;// カメラ情報
	fx32 camera_dist;			// カメラ距離

	if( !p_wk->cp_camera ){
		*p_x = 0;
		*p_y = 0;
		return;
	}

	//  カメラ情報の取得
	cp_camera = FIELD_CAMERA_GetCameraPtr( p_wk->cp_camera );

	// 動いたら動いた方向に０．５足す（小数点の誤差で値が変わらないため）
	GFL_G3D_CAMERA_GetTarget( cp_camera, &now_mat );
	GFL_G3D_CAMERA_GetPos( cp_camera, &now_camera_pos );
	
	dist_x = (now_mat.x - p_wk->scroll.last_target.x);
	dist_y = (now_mat.z - p_wk->scroll.last_target.z);

	
	// 今のカメラの2dで1の３ｄの値を求める
	camera_dist = VEC_Distance( &now_mat, &now_camera_pos );
	TOOL_GetPerspectiveScreenSize( 
			NNS_G3dGlbGetProjectionMtx(),
			camera_dist,
			&d_x, &d_y);

	// うまくいくようにちょうせい
	d_x = FX_Div(d_x, 256*FX32_ONE);

	// 上に進んでいるとき、
	// 下に進んでいるとき
	d_y = FX_Div(d_y, 192*FX32_ONE);


	// プラスで計算する
	mark = FX32_ONE;
	if(dist_x < 0){		// ーの時は＋の値にする
		mark = -FX32_ONE;
		dist_x = -dist_x;
	}
	scl_x = FX_Div(dist_x, d_x);	// スクロール座標を計算
	if( mark < 0 ){		// 元の符号に戻す
		scl_x = -scl_x;	// 元の符号に戻す

		// うまく見えるように調整
		scl_x += FX32_ONE;
	}

	mark = FX32_ONE;
	if(dist_y < 0){		// ーの時は＋の値にする
		mark = -FX32_ONE;
		dist_y = -dist_y;
	}	
	scl_y = FX_Div(dist_y, d_y);	// スクロール座標を求める
	if( mark < 0 ){
		scl_y = -scl_y;	// 元の符号に戻す

		// うまく見えるように調整
		scl_y += FX32_ONE;
	}
		
	// 今の座標を取得
	if((scl_x + scl_y) != 0){
		p_wk->scroll.last_target = now_mat;
	}

	*p_x = scl_x>>FX32_SHIFT;
	*p_y = scl_y>>FX32_SHIFT;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェクトを全部スクロールさせる
 *
 *	@param	p_wk		ワーク
 *	@param	x			スクロールｘ座標
 *	@param	y			スクロールｙ座標
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_ScrollObj( WEATHER_TASK* p_wk, int x, int y )
{
	WEATHER_OBJ_WORK* p_objwk;
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS pos;

	// オブジェ有無チェック
	if( !p_wk->p_objlist ){
		return ;
	}
	
	p_objwk = p_wk->p_objlist;
	do{
		p_clwk = WEATHER_OBJ_WK_GetClWk( p_objwk );
		GFL_CLACT_WK_GetWldPos( p_clwk, &pos );
		pos.x -= x;
		pos.y -= y;
		GFL_CLACT_WK_SetWldPos( p_clwk, &pos );

		p_objwk = p_objwk->p_next;
	}while( (u32)p_objwk != (u32)p_wk->p_objlist );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG面のスクロール
 *
 *	@param	p_wk		ワーク
 *	@param	x			スクロールｘ座標
 *	@param	y			スクロールｙ座標
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_ScrollBg( WEATHER_TASK* p_wk, int x, int y )
{
}



//----------------------------------------------------------------------------
/**
 *	@brief	ループSEの再生
 *
 *	@param	p_wk		ワーク
 *	@param	snd_no		SEナンバー
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_PlayLoopSnd( WEATHER_TASK* p_wk, int snd_no )
{
	WEATHER_SND_LOOP_Play( &p_wk->snd_loop, snd_no );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ループSEの停止
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
void WEATHER_TASK_StopLoopSnd( WEATHER_TASK* p_wk )
{
	WEATHER_SND_LOOP_Stop( &p_wk->snd_loop );
}





//-----------------------------------------------------------------------------
/**
 *		WEATHER_OBJ_WORK	管理関数
 *
 *		天気オブジェの持っているワークサイズは固定です。
 *		WEATHER_OBJ_WORK_USE_WORKSIZE
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	ユーザーワークの取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	ユーザワーク
 */
//-----------------------------------------------------------------------------
void* WEATHER_OBJ_WORK_GetWork( const WEATHER_OBJ_WORK* cp_wk )
{
	return WEATHER_OBJ_WK_GetWork( cp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターワークの取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	セルアクターワーク
 */
//-----------------------------------------------------------------------------
GFL_CLWK* WEATHER_OBJ_WORK_GetClWk( const WEATHER_OBJ_WORK* cp_wk )
{
	return WEATHER_OBJ_WK_GetClWk( cp_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	おやワークの取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	親の天気ワーク
 */
//-----------------------------------------------------------------------------
const WEATHER_TASK* WEATHER_OBJ_WORK_GetParent( const WEATHER_OBJ_WORK* cp_wk )
{
	return cp_wk->p_parent;
}


//----------------------------------------------------------------------------
/**
 *	@brief	座標の取得関数
 *
 *	@param	cp_wk	ワーク
 *	@param	p_pos	位置格納先
 */
//-----------------------------------------------------------------------------
void WEATHER_OBJ_WORK_GetPos( const WEATHER_OBJ_WORK* cp_wk, GFL_CLACTPOS* p_pos )
{
	GFL_CLWK* p_clwk;

	p_clwk = WEATHER_OBJ_WK_GetClWk( cp_wk );
	GFL_CLACT_WK_GetPos( p_clwk, p_pos, CLSYS_DEFREND_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief	座標の設定	（サーフェース内回り込みあり）
 *
 *	@param	p_wk		ワーク
 *	@param	cp_pos		位置
 */
//-----------------------------------------------------------------------------
void WEATHER_OBJ_WORK_SetPos( WEATHER_OBJ_WORK* p_wk, const GFL_CLACTPOS* cp_pos )
{
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS pos;

	pos = *cp_pos;

	// 回り込み
	if(pos.x > WEATHER_OBJ_MAXTURN_X){
		pos.x %= WEATHER_OBJ_MAXTURN_X;
	}else{

		if( pos.x < WEATHER_OBJ_MINTURN_X ){	
			pos.x += WEATHER_OBJ_MAXTURN_X;
		}
	}

	if(pos.y > WEATHER_OBJ_MAXTURN_Y){
		pos.y %= WEATHER_OBJ_MAXTURN_Y;
	}else{

		if( pos.y < WEATHER_OBJ_MINTURN_Y ){	
			pos.y += WEATHER_OBJ_MAXTURN_Y;
		}
	}

	p_clwk = WEATHER_OBJ_WK_GetClWk( p_wk );
	GFL_CLACT_WK_SetPos( p_clwk, &pos, CLSYS_DEFREND_MAIN );
}

//----------------------------------------------------------------------------
/**
 *	@brief  回り込みなしの座標設定関数
 *
 *	@param	p_wk      ワーク
 *	@param	cp_pos    位置
 */
//-----------------------------------------------------------------------------
void WEATHER_OBJ_WORK_SetPosNoTurn( WEATHER_OBJ_WORK* p_wk, const GFL_CLACTPOS* cp_pos )
{
	GFL_CLWK* p_clwk;

	p_clwk = WEATHER_OBJ_WK_GetClWk( p_wk );
	GFL_CLACT_WK_SetPos( p_clwk, cp_pos, CLSYS_DEFREND_MAIN );
}







//----------------------------------------------------------------------------
/**
 *	@brief	線形動作ワーク	初期化
 *
 *	@param	p_wk			ワーク
 *	@param	s_x				開始位置
 *	@param	e_x				終了位置
 *	@param	count_max		カウント最大値
 */
//-----------------------------------------------------------------------------
void WT_MOVE_WORK_Init( WT_MOVE_WORK* p_wk, int s_x, int e_x, u32 count_max )
{
	p_wk->x			= s_x;
	p_wk->s_x		= s_x;
	p_wk->d_x		= e_x - s_x;
	p_wk->count_max = count_max;
}

//----------------------------------------------------------------------------
/**
 *	@brief	線形動作メイン
 *
 *	@param	p_wk		ワーク
 *	@param	count		カウンタ
 *
 *	@retval	TRUE	動作完了
 *	@retval	FALSE	動作途中
 */
//-----------------------------------------------------------------------------
BOOL WT_MOVE_WORK_Main( WT_MOVE_WORK* p_wk, u32 count )
{
	BOOL ret = FALSE;
	u32 calc_count;
	
	// 計算カウント値を求める
	if( (count >= p_wk->count_max) ){
		ret = TRUE;
		calc_count = p_wk->count_max;
	}else{
		calc_count = count;
	}


	p_wk->x = (p_wk->d_x * calc_count) / p_wk->count_max;

	return ret;
}





//-----------------------------------------------------------------------------
/**
 *			プライベート関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	天気タスクワークのクリア
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_Clear( WEATHER_TASK* p_wk )
{
	GFL_CLUNIT* p_unit;
	FIELD_FOG_WORK*	p_fog;
	const FIELD_ZONEFOGLIGHT*	cp_zonefog;
	FIELD_LIGHT* p_light;
	FIELD_3DBG** pp_3dbg;
	const FIELD_CAMERA* cp_camera;
	const FIELD_SOUND* cp_sound;

	// 一時退避
	p_unit		= p_wk->p_unit;
	p_fog			= p_wk->p_fog;
	cp_zonefog= p_wk->cp_zonefog;
	p_light		= p_wk->p_light;
	cp_camera	= p_wk->cp_camera;
	pp_3dbg		= p_wk->pp_3dbg;
	cp_sound  = p_wk->cp_sound;

	// クリア
	GFL_STD_MemClear( p_wk, sizeof(WEATHER_TASK) );

	// 復帰
	p_wk->p_unit	= p_unit;
	p_wk->p_fog		= p_fog;
	p_wk->p_light	= p_light;
	p_wk->cp_camera	= cp_camera;
	p_wk->pp_3dbg	= pp_3dbg;
	p_wk->cp_zonefog = cp_zonefog;
	p_wk->cp_sound = cp_sound;
}

//----------------------------------------------------------------------------
/**
 *	@brief	天気タスク	管理関数の呼び出し
 *
 *	@param	p_wk		ワーク
 *	@param	p_func		関数
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_TASK_WK_CallFunc( WEATHER_TASK* p_wk, WEATHER_TASK_FUNC* p_func, u32 heapID )
{
	if( p_func ){
		return p_func( p_wk, p_wk->fog_cont, heapID );
	}
	return WEATHER_TASK_FUNC_RESULT_FINISH;
}


//----------------------------------------------------------------------------
/**
 *	@brief	天気タスク	インフォの取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	インフォ情報
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_INFO WEATHER_TASK_WK_GetInfo( const WEATHER_TASK* cp_wk )
{
	static const u8 sc_WEATHER_TASK_WK_SEQ_TO_INFO[WEATHER_TASK_SEQ_NUM]={
		WEATHER_TASK_INFO_NONE,		// WEATHER_TASK_SEQ_NONE,				// なし
		WEATHER_TASK_INFO_LOADING,	// WEATHER_TASK_SEQ_INIT_ALL,			// 一気に初期化
		WEATHER_TASK_INFO_LOADING,	// WEATHER_TASK_SEQ_INIT_DIV_OAM,		// 分割読み込み OAM
		WEATHER_TASK_INFO_LOADING,	// WEATHER_TASK_SEQ_INIT_DIV_BG,		// 分割読み込み BG
		WEATHER_TASK_INFO_LOADING,	// WEATHER_TASK_SEQ_INIT_DIV_CREATE,	// 分割読み込み 構築
		WEATHER_TASK_INFO_FADEIN,	// WEATHER_TASK_SEQ_CALL_INIT,			// 管理関数　初期化	　		呼び出し
		WEATHER_TASK_INFO_FADEIN,	// WEATHER_TASK_SEQ_CALL_FADEIN,		// 管理関数  フェードイン	呼び出し
		WEATHER_TASK_INFO_FADEIN,	// WEATHER_TASK_SEQ_CALL_NOFADE,		// 管理関数	 フェードなし	呼び出し
		WEATHER_TASK_INFO_MAIN,		// WEATHER_TASK_SEQ_CALL_MAIN,			// 管理関数　メイン			呼び出し
		WEATHER_TASK_INFO_FADEOUT,	// WEATHER_TASK_SEQ_CALL_FADEOUT_INIT,		// 管理関数　フェードアウト 呼び出し
		WEATHER_TASK_INFO_FADEOUT,	// WEATHER_TASK_SEQ_CALL_FADEOUT,		// 管理関数　フェードアウト 呼び出し
		WEATHER_TASK_INFO_DEST,		// WEATHER_TASK_SEQ_CALL_DEST,			// 管理関数　破棄			呼び出し
		WEATHER_TASK_INFO_DEST,		// WEATHER_TASK_SEQ_DELETE_ALL,		// 全情報の破棄
	};
	
	GF_ASSERT( cp_wk->seq < NELEMS(sc_WEATHER_TASK_WK_SEQ_TO_INFO) );
	return sc_WEATHER_TASK_WK_SEQ_TO_INFO[ cp_wk->seq ];
}


//----------------------------------------------------------------------------
/**
 *	@brief	空のオブジェを取得
 *
 *	@param	p_wk	ワーク
 *
 *	@return	空のオブジェワーク
 */
//-----------------------------------------------------------------------------
static WEATHER_OBJ_WORK* WEATHER_TASK_WK_GetClearObj( WEATHER_TASK* p_wk )
{
	int i;

	for( i=0; i<WEATHER_TASK_OBJBUFF_MAX; i++ ){

		if( WEATHER_OBJ_WK_IsUse( &p_wk->objbuff[i] ) == FALSE ){
			return &p_wk->objbuff[i];
		}
	}

	return NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief	オブジェリストへの登録
 *
 *	@param	p_wk		ワーク
 *	@param	p_obj		オブジェワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_PushObjList( WEATHER_TASK* p_wk, WEATHER_OBJ_WORK* p_obj )
{
	WEATHER_OBJ_WORK* p_last;

	p_wk->active_objnum ++;
	
	// 先頭チェック
	if( !p_wk->p_objlist ){
		p_wk->p_objlist = p_obj;
		p_wk->p_objlist->p_next = p_obj;
		p_wk->p_objlist->p_last = p_obj;
		return ;
	}

	// 最終に追加
	p_last = p_wk->p_objlist->p_last;

	// p_lastの次に登録
	p_obj->p_last			= p_last;
	p_obj->p_next			= p_last->p_next;
	p_last->p_next->p_last	= p_obj;
	p_last->p_next			= p_obj;


	// 描画ON
	GFL_CLACT_WK_SetDrawEnable( p_obj->p_clwk, TRUE );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェリストからはずす
 *
 *	@param	p_wk		ワーク	
 *	@param	p_obj		オブジェワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_PopObjList( WEATHER_TASK* p_wk, WEATHER_OBJ_WORK* p_obj )
{

	p_wk->active_objnum --;
	
	// 自分が先頭かチェック
	if( p_wk->p_objlist == p_obj ){
		// 自分の次も自分だったら終わり
		if( p_obj->p_next == p_obj ){
			p_wk->p_objlist = NULL;
		}else{
			// 先頭を自分の次のやつに譲る
			p_wk->p_objlist = p_obj->p_next;
		}
	}
	// はずす
	p_obj->p_next->p_last = p_obj->p_last;
	p_obj->p_last->p_next = p_obj->p_next;
	// 自分のリストポインタをクリア
	p_obj->p_next = NULL;
	p_obj->p_last = NULL;

	// 描画OFF
	GFL_CLACT_WK_SetDrawEnable( p_obj->p_clwk, FALSE );

}


//----------------------------------------------------------------------------
/**
 *	@brief	オブジェリスト　メイン処理
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_MainObjList( WEATHER_TASK* p_wk )
{
	WEATHER_OBJ_WORK* p_objwk;
	WEATHER_OBJ_WORK* p_objwk_next;

	// オブジェ有無チェック
	if( !p_wk->p_objlist ){
		return ;
	}

	// 関数があるか？
	if( !p_wk->cp_data->p_f_objmove ){
		return ;
	}
	
	p_objwk = p_wk->p_objlist;
	p_objwk_next = p_objwk->p_next;
	do{

		// メイン関数をまわす
		p_wk->cp_data->p_f_objmove( p_objwk );

		p_objwk = p_objwk_next;
		p_objwk_next = p_objwk->p_next;
	}while( ((u32)p_objwk != (u32)p_wk->p_objlist) && (p_wk->p_objlist != NULL) );
}


//----------------------------------------------------------------------------
/**
 *	@brief	OAMの読み込み
 *
 *	@param	p_wk	ワーク
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_InitOam( WEATHER_TASK* p_wk, u32 heapID )
{
	// グラフィック読み込み
	WEATHER_TASK_GRAPHIC_InitHandle( &p_wk->graphic, p_wk->cp_data, heapID );
	WEATHER_TASK_GRAPHIC_InitOam( &p_wk->graphic, p_wk->cp_data, heapID );
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMリソースの破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_ExitOam( WEATHER_TASK* p_wk )
{
	WEATHER_TASK_GRAPHIC_ExitOam( &p_wk->graphic );
	WEATHER_TASK_GRAPHIC_ExitHandle( &p_wk->graphic );
}
	
//----------------------------------------------------------------------------
/**
 *	@brief	BGリソースの読み込み
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_InitBg( WEATHER_TASK* p_wk, u32 heapID )
{
  int i;
  for( i=0; i<p_wk->cp_data->use_bg; i++ ){
   	WEATHER_TASK_GRAPHIC_InitBg( &p_wk->graphic, i, p_wk->pp_3dbg[i], &p_wk->cp_data->bg_data[i], heapID );
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGリソースの破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_ExitBg( WEATHER_TASK* p_wk )
{
  int i;
  for( i=0; i<p_wk->cp_data->use_bg; i++ ){
    WEATHER_TASK_GRAPHIC_ExitBg( &p_wk->graphic, i, p_wk->pp_3dbg[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	その他の初期化
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_InitOther( WEATHER_TASK* p_wk, u32 heapID )
{
	int i;
	
	GF_ASSERT( !p_wk->p_user_work );
	p_wk->p_user_work = GFL_HEAP_AllocClearMemory( heapID, p_wk->cp_data->work_byte );

	// オブジェワークのアクターをクリエイト
	if( p_wk->cp_data->use_oam ){
		for( i=0; i<WEATHER_TASK_OBJBUFF_MAX; i++ ){
			WEATHER_OBJ_WK_Init( &p_wk->objbuff[i], p_wk, &p_wk->graphic, p_wk->p_unit, heapID );
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	その他の破棄
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_WK_ExitOther( WEATHER_TASK* p_wk )
{
	int i;
	
	if( p_wk->p_user_work ){
		GFL_HEAP_FreeMemory( p_wk->p_user_work );
		p_wk->p_user_work = NULL;
	}

	// オブジェワークのアクターを破棄
	if( p_wk->cp_data->use_oam ){
		for( i=0; i<WEATHER_TASK_OBJBUFF_MAX; i++ ){
			WEATHER_OBJ_WK_Exit( &p_wk->objbuff[i] );
			WEATHER_OBJ_WK_Clear( &p_wk->objbuff[i] );
		}
	}

  // SEの停止
  WEATHER_TASK_StopLoopSnd( p_wk );
}




//----------------------------------------------------------------------------
/**
 *	@brief	グラフィックワークをクリアする
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_Clear( WEATHER_TASK_GRAPHIC* p_wk )
{
	GF_ASSERT( !p_wk->oam_load );
	GF_ASSERT( !p_wk->p_handle );

	GFL_STD_MemClear( p_wk, sizeof(WEATHER_TASK_GRAPHIC) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アーカイブハンドル初期化
 *
 *	@param	p_wk			ワーク
 *	@param	cp_data			天気情報
 *	@param	heapID			ヒープID
 */	
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_InitHandle( WEATHER_TASK_GRAPHIC* p_wk, const WEATHER_TASK_DATA* cp_data, u32 heapID )
{
	GF_ASSERT( !p_wk->p_handle );
	if( cp_data->use_oam || cp_data->use_bg ){
		p_wk->p_handle = GFL_ARC_OpenDataHandle( cp_data->arc_id, heapID );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	アーカイブハンドル破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_ExitHandle( WEATHER_TASK_GRAPHIC* p_wk )
{
	if( p_wk->p_handle ){
		GFL_ARC_CloseDataHandle( p_wk->p_handle );
		p_wk->p_handle = NULL;
	}
}
	
//----------------------------------------------------------------------------
/**
 *	@brief	OAMリソース読み込み
 *
 *	@param	p_wk		ワーク
 *	@param	cp_data		天気情報
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_InitOam( WEATHER_TASK_GRAPHIC* p_wk, const WEATHER_TASK_DATA* cp_data, u32 heapID )
{
	GF_ASSERT( !p_wk->oam_load );

	if( cp_data->use_oam ){
		p_wk->oam_cgx_id = GFL_CLGRP_CGR_Register( p_wk->p_handle, cp_data->oam_cg, FALSE, 
				WEATHER_GRAPHIC_OAM_LOAD_DRAW_TYPE, heapID );
		p_wk->oam_pltt_id = GFL_CLGRP_PLTT_RegisterEx( p_wk->p_handle, cp_data->oam_pltt, 
				WEATHER_GRAPHIC_OAM_LOAD_DRAW_TYPE, WEATHER_TASK_GRAPHIC_OAM_PLTT_OFS, 0, 
				WEATHER_TASK_GRAPHIC_OAM_PLTT_NUM_MAX, heapID );
		p_wk->oam_cell_id = GFL_CLGRP_CELLANIM_Register( p_wk->p_handle, cp_data->oam_cell, cp_data->oam_cellanm, heapID );

		p_wk->oam_load = TRUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	OAMリソース破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_ExitOam( WEATHER_TASK_GRAPHIC* p_wk )
{
	if( p_wk->oam_load ){

		GFL_CLGRP_CGR_Release( p_wk->oam_cgx_id );
		GFL_CLGRP_PLTT_Release( p_wk->oam_pltt_id );
		GFL_CLGRP_CELLANIM_Release( p_wk->oam_cell_id );

		p_wk->oam_load = FALSE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGリソース読み込み
 *
 *	@param	p_wk		ワーク
 *	@param  type
 *	@param	cp_data		データ
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_InitBg( WEATHER_TASK_GRAPHIC* p_wk, WEATHER_TASK_3DBG_TYPE type, FIELD_3DBG* p_3dbg, const FIELD_3DBG_WRITE_DATA* cp_data, u32 heapID )
{
  static FIELD_3DBG_WRITE_DATA s_FIELD_3DBG_WRITE_DATA;

  GFL_STD_MemCopy( cp_data, &s_FIELD_3DBG_WRITE_DATA, sizeof(FIELD_3DBG_WRITE_DATA) );
  
  if( s_FIELD_3DBG_WRITE_DATA.alpha != 31 )
  {
    TOMOYA_Printf( "WEATHER 3DBG ALPHA 31 SetUp\n" );
    s_FIELD_3DBG_WRITE_DATA.alpha = 31;
  }
  FIELD_3DBG_SetWriteData( p_3dbg, p_wk->p_handle, &s_FIELD_3DBG_WRITE_DATA, heapID );
  FIELD_3DBG_SetVisible( p_3dbg, FALSE );

  FIELD_3DBG_SetPolygonID( p_3dbg, 32 );

  p_wk->bg_load[type] = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	BGリソース破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_TASK_GRAPHIC_ExitBg( WEATHER_TASK_GRAPHIC* p_wk, WEATHER_TASK_3DBG_TYPE type, FIELD_3DBG* p_3dbg )
{
  if( p_wk->bg_load[type] ){
	  FIELD_3DBG_ClearWriteData( p_3dbg );
    p_wk->bg_load[type] = FALSE;
  }
}





//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェードシステム	初期化（フェードイン）
 *
 *	@param	p_wk						ワーク
 *	@param	p_sys						システム
 *	@param	objAddTmgMax				オブジェ追加タイミングマックス
 *	@param	objAddNum					オブジェ追加数
 *	@param	obj_add_num_end				オブジェ追加数　終了値
 *	@param	obj_add_tmg_end				オブジェ追加タイミング　終了値
 *	@param	obj_add_tmg_sum				オブジェ追加タイミング　足し算値
 *	@param	obj_add_num_sum_tmg			オブジェ追加タイミング　足し算タイミング
 *	@param	obj_add_num_sum				オブジェ追加数　加算値
 *	@param	p_addfunc					追加関数
 */
//-----------------------------------------------------------------------------
static void WEATHER_OBJ_FADE_InitFade( WEATHER_TASK_OBJ_FADE* p_wk, WEATHER_TASK* p_sys, s32 objAddTmgMax, s32 objAddNum, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum_tmg, s32 obj_add_num_sum, WEATHER_TASK_OBJADD_FUNC* p_addfunc )
{
	p_wk->p_parent			= p_sys;
	
	p_wk->obj_addnum			= objAddNum;
	p_wk->obj_addtmg			= 0;
	p_wk->obj_addtmg_max		= objAddTmgMax;
	p_wk->obj_addnum_sum		= 0;
	p_wk->obj_add_num_end		= obj_add_num_end;
	p_wk->obj_add_tmg_end		= obj_add_tmg_end;
	p_wk->obj_add_tmg_sum		= obj_add_tmg_sum;
	p_wk->obj_add_num_sum_tmg	= obj_add_num_sum_tmg;
	p_wk->obj_add_num_sum		= obj_add_num_sum;

	p_wk->p_addfunc				= p_addfunc;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェードシステム	フェードアウト設定
 *
 *	@param	p_wk					ワーク
 *	@param	obj_add_num_end			オブジェ追加数　終了値
 *	@param	obj_add_tmg_end			オブジェ追加タイミング　終了値
 *	@param	obj_add_tmg_sum			オブジェ追加タイミング　変化値
 *	@param	obj_add_num_sum			オブジェ追加数　変化値
 */
//-----------------------------------------------------------------------------
static void WEATHER_OBJ_FADE_SetFadeOut( WEATHER_TASK_OBJ_FADE* p_wk, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum )
{
	p_wk->obj_add_num_end	= obj_add_num_end;
	p_wk->obj_add_tmg_end	= obj_add_tmg_end;
	p_wk->obj_add_tmg_sum	= obj_add_tmg_sum;
	p_wk->obj_add_num_sum	= obj_add_num_sum;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェ追加フェードシステム　メイン
 *
 *	@param	p_wk				ワーク
 *
 *	@retval	TRUE	フェードイン or フェードアウトの完了
 *	@retval	FALSE	途中
 */
//-----------------------------------------------------------------------------
static BOOL WEATHER_OBJ_FADE_Main( WEATHER_TASK_OBJ_FADE* p_wk, u32 heapID )
{
	int fade_flag;
	int ret = 0;
	

	// フェード終了チェック
	// タイミング-------------------------------------------
	// 足す値が０ならタイミングフェード終了
	if(p_wk->obj_add_tmg_sum == 0){
		ret |= 1;
	}else{
		// フェードイン、アウトチェック
		if(p_wk->obj_add_tmg_sum > 0){
			fade_flag = 0;		// 変更値がプラスなのでフェードイン
		}else{
			fade_flag = 1;		// 変更値がマイナスなのでフェードアウト
		}
		// 登録タイミング終了点にきていないかチェック
		if(((p_wk->obj_addtmg_max >= p_wk->obj_add_tmg_end)&&(fade_flag == 0)) ||
			((p_wk->obj_addtmg_max <= p_wk->obj_add_tmg_end)&&(fade_flag == 1))){
			ret |= 1;

			p_wk->obj_addtmg_max = p_wk->obj_add_tmg_end;
		}
	}

	// 登録数---------------------------------------------------
	// 足す値が０なら登録数フェード終了
	if(p_wk->obj_add_num_sum == 0){
		ret |= 2;
	}else{
		
		// フェードイン、アウトチェック
		if(p_wk->obj_add_num_sum > 0){
			fade_flag = 0;		// 変更値がプラスなのでフェードイン
		}else{
			fade_flag = 1;		// 変更値がマイナスなのでフェードアウト
		}
		// オブジェクト登録終了点にきていないかチェック
		if(((p_wk->obj_addnum >= p_wk->obj_add_num_end)&&(fade_flag == 0)) ||
			((p_wk->obj_addnum <= p_wk->obj_add_num_end)&&(fade_flag == 1))){
		
			ret |= 2;
			p_wk->obj_addnum = p_wk->obj_add_num_end;
		}
	}
	

	// タイミングカウンタが最小になるまでカウント作業
	(p_wk->obj_addtmg)--;
	if(p_wk->obj_addtmg <= 0){

		// 登録
		p_wk->p_addfunc(p_wk->p_parent, p_wk->obj_addnum, heapID);

		p_wk->obj_addtmg = p_wk->obj_addtmg_max;			// 登録タイミングセット
		
		// タイミング-------------------------------------------
		if((ret & 1) == 0){
			p_wk->obj_addtmg_max += p_wk->obj_add_tmg_sum;	// 雨タイミング計算
		}
		
	
		// 登録数---------------------------------------------------
		if((ret & 2) == 0){
			(p_wk->obj_addnum_sum)++;
			if(p_wk->obj_addnum_sum >= p_wk->obj_add_num_sum_tmg){
				p_wk->obj_addnum_sum = 0;
				p_wk->obj_addnum += p_wk->obj_add_num_sum;	// 登録数アッド
			}
		}
	}

  if( ret & (1|2) == (1|2) )
  {
    return TRUE;
  }
	return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードはせず、オブジェの同じ登録タイミングで、同じ数　オブジェを登録します。
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static BOOL WEATHER_TASK_OBJ_FADE_NoFadeMain( WEATHER_TASK_OBJ_FADE* p_wk, u32 heapID )
{

	// タイミングカウンタが最小になるまでカウント作業
	p_wk->obj_addtmg--;
	if(p_wk->obj_addtmg <= 0){

		// 登録
		p_wk->p_addfunc(p_wk->p_parent, p_wk->obj_addnum, heapID);

		p_wk->obj_addtmg = p_wk->obj_addtmg_max;			// 登録タイミングセット
		return TRUE;
	}
	return FALSE;
}








//----------------------------------------------------------------------------
/**
 *	@brief	オブジェワーククリア
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_OBJ_WK_Clear( WEATHER_OBJ_WORK* p_wk )
{
	GFL_STD_MemClear( p_wk, sizeof(WEATHER_OBJ_WORK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェワークが使われているかチェック
 *
 *	@param	cp_wk	ワーク
 *
 *	@retval	TRUE	使われている
 *	@retval	FALSE	使われていない
 */
//-----------------------------------------------------------------------------
static BOOL WEATHER_OBJ_WK_IsUse( const WEATHER_OBJ_WORK* cp_wk )
{
	if( cp_wk->p_last == NULL ){
		return FALSE;
	}
	return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェワーク	初期化処理
 *
 *	@param	p_wk			ワーク
 *	@param	p_parent		親の天気タスクシステム
 *	@param	cp_graphic		グラフィックワーク
 *	@param	p_unit			セルユニット
 *	@param	heapID			ヒープID
 */
//-----------------------------------------------------------------------------
static void WEATHER_OBJ_WK_Init( WEATHER_OBJ_WORK* p_wk, WEATHER_TASK* p_parent, const WEATHER_TASK_GRAPHIC* cp_graphic, GFL_CLUNIT* p_unit, u32 heapID )
{
	static const GFL_CLWK_DATA cladd = {
		0, 0,
		0, 
		WEATHER_OBJ_WORK_SOFT_PRI,
		WEATHER_OBJ_WORK_BG_PRI
	};

	// グラフィックは読み込み済みか？
	GF_ASSERT( cp_graphic->oam_load );
	
	
	// おや設定
	p_wk->p_parent = p_parent; 

	// アクター生成
	p_wk->p_clwk = GFL_CLACT_WK_Create( p_unit, 
			cp_graphic->oam_cgx_id, cp_graphic->oam_pltt_id, 
			cp_graphic->oam_cell_id, &cladd, 
			WEATHER_OBJ_WORK_SERFACE_NO, heapID );

	// 描画OFF
	GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );

}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェワーク	破棄処理
 *
 *	@param	p_wk			ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_OBJ_WK_Exit( WEATHER_OBJ_WORK* p_wk )
{
	if( p_wk->p_clwk ){
		// アクター破棄
		GFL_CLACT_WK_Remove( p_wk->p_clwk );
		p_wk->p_clwk = NULL;
	}
	
	
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェのユーザワークを取得
 *
 *	@param	cp_wk	ワーク
 *	
 *	@return	ユーザワーク
 */
//-----------------------------------------------------------------------------
static void* WEATHER_OBJ_WK_GetWork( const WEATHER_OBJ_WORK* cp_wk )
{
	return (void*)cp_wk->user_work;
}

//----------------------------------------------------------------------------
/**
 *	@brief	オブジェのアクターワーク取得
 *
 *	@param	cp_wk	ワーク
 *
 *	@return	アクターワーク
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* WEATHER_OBJ_WK_GetClWk( const WEATHER_OBJ_WORK* cp_wk )
{
	return cp_wk->p_clwk;
}





//----------------------------------------------------------------------------
/**
 *	@brief	ループサウンド再生
 *
 *	@param	p_wk		ワーク
 *	@param	snd_no		サウンド定数
 */
//-----------------------------------------------------------------------------
static void WEATHER_SND_LOOP_Play( WEATHER_TASK_SND_LOOP* p_wk, int snd_no )
{
	if( p_wk->play ){
		WEATHER_SND_LOOP_Stop( p_wk );
	}

	p_wk->play		= TRUE;
	p_wk->snd_no	= snd_no;

  PMSND_PlaySE( p_wk->snd_no );
}

//----------------------------------------------------------------------------
/**
 *	@brief	ループサウンド停止
 *
 *	@param	p_wk		ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_SND_LOOP_Stop( WEATHER_TASK_SND_LOOP* p_wk )
{
  if(p_wk->play)
  {
    p_wk->play = FALSE;

    PMSND_StopSE_byPlayerID( PMSND_GetSE_DefaultPlayerID( p_wk->snd_no ) );
  }
}




//----------------------------------------------------------------------------
/**
 *	@brief	射影行列の情報から、高さと幅を返す
 *
 *	@param	cp_pers_mtx			パースマトリックス
 *	@param	dist				ターゲットまでの距離
 *	@param	p_width				画面の幅格納先
 *	@param	p_height			画面の高さ格納先
 */
//-----------------------------------------------------------------------------
static void TOOL_GetPerspectiveScreenSize( const MtxFx44* cp_pers_mtx, fx32 dist, fx32* p_width, fx32* p_height )
{
	// 高さを求める
	*p_height = FX_Div(dist, cp_pers_mtx->_11);				// (fovySin / fovyCos)*TargetDist
	*p_height = FX_Mul(*p_height, 2*FX32_ONE);		// ２かけると画面の高さになる
	
	// 幅を求める
	*p_width  = FX_Div(dist, cp_pers_mtx->_00);
	*p_width = FX_Mul(*p_width, 2*FX32_ONE);		// ２かけると画面の幅になる
}



