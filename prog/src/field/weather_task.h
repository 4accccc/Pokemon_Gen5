//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_task.h
 *	@brief		１天気　動作管理
 *	@author		tomoya takahshi
 *	@date		2009.03.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef _cplusplus
extern "C"{
#endif

#ifndef __WEATHER_TASK_H__
#define __WEATHER_TASK_H__

#include <gflib.h>

#include "fieldmap.h"
#include "field_common.h"
#include "field_camera.h"

#include "field_light.h"
#include "field_fog.h"
#include "field_zonefog.h"
#include "field_3dbg.h"
#include "field_oam_pal.h"

#include "weather_snd.h"



//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	初期化モード
//=====================================
typedef enum {
	WEATHER_TASK_INIT_NORMAL,		// 通常読み込み
	WEATHER_TASK_INIT_DIV,			// 分割読み込み

	WEATHER_TASK_INIT_MODE_NUM,		// システム内部で使用
} WEATEHR_TASK_INIT_MODE;


//-------------------------------------
///	タスク情報
//=====================================
typedef enum {
	WEATHER_TASK_INFO_NONE,				// 情報なし
	WEATHER_TASK_INFO_LOADING,			// 読み込み中
	WEATHER_TASK_INFO_FADEIN,			// フェードイン中
	WEATHER_TASK_INFO_MAIN,				// メイン動作中
	WEATHER_TASK_INFO_FADEOUT,			// フェードアウト中
	WEATHER_TASK_INFO_DEST,				// 破棄中


	WEATHER_TASK_INFO_NUM,				// システム内で使用
} WEATHER_TASK_INFO;

//-------------------------------------
///	管理関数の戻り値
//=====================================
typedef enum {
	WEATHER_TASK_FUNC_RESULT_CONTINUE,	// つづく
	WEATHER_TASK_FUNC_RESULT_FINISH,	// 次へ


	WEATHER_TASK_FUNC_RESULT_NUM,		// システム内で使用
} WEATHER_TASK_FUNC_RESULT;

//-------------------------------------
///	FOG管理フラグ
//=====================================
typedef enum {
	WEATHER_TASK_FOG_NONE,		// フォグ操作を行わない
	WEATHER_TASK_FOG_USE,		// 新しくフォグを設定する
	WEATHER_TASK_FOG_WITH,		// 前の設定を引き継ぎつつ設定する

	WEATHER_TASK_FOG_MODE_MAX,	// プログラム内で使用
} WEATHER_TASK_FOG_MODE;



//-------------------------------------
///	リソース情報
//=====================================
#define WEATHER_TASK_GRAPHIC_OAM_PLTT_NUM_MAX	( 1 )		// パレットのサイズ
#define WEATHER_TASK_GRAPHIC_OAM_PLTT_OFS		( FLDOAM_PALNO_WEATHER*32 )	// パレットの転送位置


//-------------------------------------
///	天気オブジェ保持、ワークサイズ
//=====================================
#define WEATHER_OBJ_WORK_USE_WORKSIZE	( sizeof(s32) * 10 )


//-------------------------------------
/// DP　から移植　基本情報	
//=====================================
#define		WEATHER_FOG_DEPTH_DEFAULT	      ( 0x7880 )	// フォグデフォルト値
#define		WEATHER_FOG_DEPTH_DEFAULT_START	( 0x7FFF )	// フォグデフォルトフェード開始値
//#define		WEATHER_FOG_DEPTH_DEFAULT	( 0x200 )	// フォグデフォルト値
#define		WEATHER_FOG_SLOPE_DEFAULT	( 4 )



//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	天気タスク
//=====================================
typedef struct _WEATHER_TASK WEATHER_TASK;

//-------------------------------------
///	天気１オブジェ（雪の粒など用　OAM）
//=====================================
typedef struct _WEATHER_OBJ_WORK WEATHER_OBJ_WORK;


//-------------------------------------
///	天気管理関数
//=====================================
typedef WEATHER_TASK_FUNC_RESULT (WEATHER_TASK_FUNC)( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID );


//-------------------------------------
///	１オブジェ動作関数
//=====================================
typedef void (WEATHER_OBJ_WORK_FUNC)( WEATHER_OBJ_WORK* p_wk );

//-------------------------------------
///	天気追加情報
//=====================================
typedef struct {
	// アークID
	// BGとOAM共用
	u16	arc_id;				
	u16	use_oam;			// OAMを使用するか？ TRUE or FALSE
	u16	use_bg;			// BGを使用するか？ TRUE or FALSE
	
	// OAM情報
	u16 oam_cg;				
	u16 oam_pltt;			// パレットは1本です。
	u16 oam_cell;
	u16 oam_cellanm;
	
	// BG情報
	// （２つの天気を同時に管理する場合、BGの反映は後勝ち）
	u16 bg_tex;		// BGTEX
	u8 texwidth;	// GXTexSizeS
	u8 texheight;	// GXTexSizeT
	u8 repeat;		// GXTexRepeat
	u8 flip;		// GXTexFlip
	u8 texfmt;		// GXTexFmt
	u8 texpltt;		// GXTexPlttColor0
	
	// ワーク領域サイズ
	u32 work_byte;
	
	// 管理関数
	WEATHER_TASK_FUNC* p_f_init;			// 初期化処理
	WEATHER_TASK_FUNC* p_f_fadein;			// フェードイン処理
	WEATHER_TASK_FUNC* p_f_nofade;			// フェードなしの場合の処理
	WEATHER_TASK_FUNC* p_f_main;			// メイン処理
	WEATHER_TASK_FUNC* p_f_init_fadeout;	// フェードアウト初期化処理
	WEATHER_TASK_FUNC* p_f_fadeout;			// フェードアウト処理
	WEATHER_TASK_FUNC* p_f_dest;			// 破棄処理

	
	//  動作オブジェ動作関数
	WEATHER_OBJ_WORK_FUNC* p_f_objmove;	
	
} WEATHER_TASK_DATA;


//-------------------------------------
///	線形動作ワーク
//=====================================
typedef struct {
	int x;
	int s_x;
	int d_x;

	u32 count_max;
} WT_MOVE_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *		WEATHER_TASK	管理関数
 */
//-----------------------------------------------------------------------------
//-------------------------------------
///	システム生成・破棄・メイン
//=====================================
extern WEATHER_TASK* WEATHER_TASK_Init( GFL_CLUNIT* p_clunit, const FIELD_CAMERA* cp_camera, FIELD_LIGHT* p_light, FIELD_FOG_WORK* p_fog, const FIELD_ZONEFOGLIGHT* cp_zonefog, FIELD_3DBG* p_3dbg, u32 heapID );
extern void WEATHER_TASK_Exit( WEATHER_TASK* p_wk );
extern void WEATHER_TASK_Main( WEATHER_TASK* p_wk, u32 heapID );

//-------------------------------------
///	発動・停止管理
//=====================================
extern void WEATHER_TASK_Start( WEATHER_TASK* p_wk, const WEATHER_TASK_DATA* cp_data, WEATEHR_TASK_INIT_MODE init_mode, BOOL fade, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID );
extern void WEATHER_TASK_End( WEATHER_TASK* p_wk, BOOL fade, WEATHER_TASK_FOG_MODE fog_cont );
extern void WEATHER_TASK_ForceEnd( WEATHER_TASK* p_wk );

//-------------------------------------
///	ライト操作
//	Change	light_idのライトに変更
//	Back	基本のライトに変更
//=====================================
extern void WEATHER_TASK_LIGHT_Change( WEATHER_TASK* p_wk, u32 arcid, u32 dataid, u32 heapID );
extern void WEATHER_TASK_LIGHT_Back( WEATHER_TASK* p_wk, u32 heapID );

extern void WEATHER_TASK_LIGHT_StartColorFade( WEATHER_TASK* p_wk, GXRgb color, u16 insync, u16 outsync );
extern BOOL WEATHER_TASK_LIGHT_IsColorFade( const WEATHER_TASK* cp_wk );


//-------------------------------------
///	3DBG操作
//=====================================
extern void WEATHER_TASK_3DBG_SetVisible( WEATHER_TASK* p_wk, BOOL visible );
extern BOOL WEATHER_TASK_3DBG_GetVisible( const WEATHER_TASK* cp_wk );
extern void WEATHER_TASK_3DBG_SetAlpha( WEATHER_TASK* p_wk, u8 alpha );
extern u8 WEATHER_TASK_3DBG_GetAlpha( const WEATHER_TASK* cp_wk );

extern void WEATHER_TASK_3DBG_SetScrollX( WEATHER_TASK* p_wk, s32 x );
extern void WEATHER_TASK_3DBG_SetScrollY( WEATHER_TASK* p_wk, s32 y );
extern s32 WEATHER_TASK_3DBG_GetScrollX( const WEATHER_TASK* cp_wk );
extern s32 WEATHER_TASK_3DBG_GetScrollY( const WEATHER_TASK* cp_wk );


//-------------------------------------
///	情報の取得
//=====================================
extern WEATHER_TASK_INFO WEATHER_TASK_GetInfo( const WEATHER_TASK* cp_wk );
extern void* WEATHER_TASK_GetWorkData( const WEATHER_TASK* cp_wk );		//WEATHER_TASK_DATA.work_byte分のワーク
extern u32 WEATHER_TASK_GetActiveObjNum( const WEATHER_TASK* cp_wk );


//-------------------------------------
///	動作オブジェの追加・破棄
//=====================================
extern WEATHER_OBJ_WORK* WEATHER_TASK_CreateObj( WEATHER_TASK* p_wk, u32 heapID );
extern void WEATHER_TASK_DeleteObj( WEATHER_OBJ_WORK* p_obj );




//-------------------------------------
///	動作オブジェの追加フェード処理
//=====================================
typedef void (WEATHER_TASK_OBJADD_FUNC)( WEATHER_TASK* p_wk, int num, u32 heapID );
extern void WEATHER_TASK_ObjFade_Init( WEATHER_TASK* p_wk, s32 objAddNum, s32 objAddTmgMax, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum_tmg, s32 obj_add_num_sum, WEATHER_TASK_OBJADD_FUNC* p_addfunc );
extern void WEATHER_TASK_ObjFade_SetOut( WEATHER_TASK* p_wk, s32 obj_add_num_end, s32 obj_add_tmg_end, s32 obj_add_tmg_sum, s32 obj_add_num_sum );
extern BOOL WEATHER_TASK_ObjFade_Main( WEATHER_TASK* p_wk, u32 heapID );

// フェードはせず、オブジェの同じ登録タイミングで、同じ数　オブジェを登録します。
extern BOOL WEATHER_TASK_ObjFade_NoFadeMain( WEATHER_TASK* p_wk, u32 heapID );

//-------------------------------------
///	動作オブジェを乱数で分布させる
//=====================================
extern void WEATHER_TASK_DustObj( WEATHER_TASK* p_wk, WEATHER_TASK_OBJADD_FUNC* p_add_func, int num, int dust_div_num, int dust_div_move, u32 heapID );

//-------------------------------------
///	フォグフェード処理
//=====================================
extern void WEATHER_TASK_FogSet( WEATHER_TASK* p_wk, FIELD_FOG_SLOPE fog_slope, int fog_offs, WEATHER_TASK_FOG_MODE mode );
extern void WEATHER_TASK_FogClear( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE mode );
extern void WEATHER_TASK_FogFadeIn_Init( WEATHER_TASK* p_wk, FIELD_FOG_SLOPE fog_slope, int fog_offs, int timing, WEATHER_TASK_FOG_MODE mode );
extern void WEATHER_TASK_FogFadeOut_Init( WEATHER_TASK* p_wk, int fog_offs, int timing, WEATHER_TASK_FOG_MODE mode );
extern BOOL WEATHER_TASK_FogFade_IsFade( const WEATHER_TASK* cp_wk );


//-------------------------------------
///	ゾーン用設定の取得
//=====================================
extern BOOL WEATHER_TASK_IsZoneFog( const WEATHER_TASK* cp_wk );
extern BOOL WEATHER_TASK_IsZoneLight( const WEATHER_TASK* cp_wk );
extern s32 WEATHER_TASK_GetZoneFogOffset( const WEATHER_TASK* cp_wk );
extern u32 WEATHER_TASK_GetZoneFogSlope( const WEATHER_TASK* cp_wk );
extern u32 WEATHER_TASK_GetZoneLight( const WEATHER_TASK* cp_wk );

//-------------------------------------
///	フォグブレンド操作
//=====================================
extern void WEATHER_TASK_Fog_SetBlendMode( WEATHER_TASK* p_wk, FIELD_FOG_BLEND blend );
extern FIELD_FOG_BLEND WEATHER_TASK_Fog_GetBlendMode( const WEATHER_TASK* cp_wk );
extern void WEATHER_TASK_Fog_SetAlpha( WEATHER_TASK* p_wk, u8 alpha );
extern u8 WEATHER_TASK_Fog_GetAlpha( const WEATHER_TASK* cp_wk );



//-------------------------------------
///　スクロール管理
//=====================================
extern void WEATHER_TASK_InitScrollDist( WEATHER_TASK* p_wk );
extern void WEATHER_TASK_GetScrollDist( WEATHER_TASK* p_wk, int* p_x, int* p_y );
extern void WEATHER_TASK_ScrollObj( WEATHER_TASK* p_wk, int x, int y );
extern void WEATHER_TASK_ScrollBg( WEATHER_TASK* p_wk, int x, int y );

//-------------------------------------
///	サウンド管理
//=====================================
extern void WEATHER_TASK_PlayLoopSnd( WEATHER_TASK* p_wk, int snd_no );
extern void WEATHER_TASK_StopLoopSnd( WEATHER_TASK* p_wk );




//-----------------------------------------------------------------------------
/**
 *		WEATHER_OBJ_WORK	管理関数
 *
 *		天気オブジェの持っているワークサイズは固定です。
 *		WEATHER_OBJ_WORK_USE_WORKSIZE
 */
//-----------------------------------------------------------------------------
extern void* WEATHER_OBJ_WORK_GetWork( const WEATHER_OBJ_WORK* cp_wk );
extern GFL_CLWK* WEATHER_OBJ_WORK_GetClWk( const WEATHER_OBJ_WORK* cp_wk );
extern const WEATHER_TASK* WEATHER_OBJ_WORK_GetParent( const WEATHER_OBJ_WORK* cp_wk );
extern void WEATHER_OBJ_WORK_GetPos( const WEATHER_OBJ_WORK* cp_wk, GFL_CLACTPOS* p_pos );
extern void WEATHER_OBJ_WORK_SetPos( WEATHER_OBJ_WORK* p_wk, const GFL_CLACTPOS* cp_pos );
extern void WEATHER_OBJ_WORK_SetPosNoTurn( WEATHER_OBJ_WORK* p_wk, const GFL_CLACTPOS* cp_pos );


//-----------------------------------------------------------------------------
/**
 *		動作計算ツール
 */
//-----------------------------------------------------------------------------
extern void WT_MOVE_WORK_Init( WT_MOVE_WORK* p_wk, int s_x, int e_x, u32 count_max );
extern BOOL	WT_MOVE_WORK_Main( WT_MOVE_WORK* p_wk, u32 count );



#endif		// __WEATHER_TASK_H__

#ifdef _cplusplus
}/* extern "C" */
#endif

