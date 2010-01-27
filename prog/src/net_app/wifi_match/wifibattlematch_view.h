//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_view.h
 *	@brief	ランダムマッチ表示関係
 *	@author	Toru=Nagihashi
 *	@date		2009.11.04
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
#include "net_app/wifibattlematch.h"
#include "system/pms_data.h"
#include "pm_define.h"
#include "pokeicon/pokeicon.h"
#include "system/pms_draw.h"
#include "wifibattlematch_data.h"
#include "savedata/regulation.h"
#include "savedata/battle_box_save.h"
//-------------------------------------
///	デバッグ
//=====================================
#ifdef PM_DEBUG
//#define DEBUG_DATA_CREATE
#endif //PM_DEBUGa

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					表示関係の定数
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	BGフレーム
//=====================================
enum
{
	//メイン画面
	BG_FRAME_M_TEXT	=	GFL_BG_FRAME0_M, 
	BG_FRAME_M_FONT	=	GFL_BG_FRAME1_M, 
	BG_FRAME_M_CARD	=	GFL_BG_FRAME2_M, 
	BG_FRAME_M_BACK	=	GFL_BG_FRAME3_M, 

	//サブ画面
	BG_FRAME_S_TEXT	=	GFL_BG_FRAME0_S, 
	BG_FRAME_S_FONT	=	GFL_BG_FRAME1_S, 
	BG_FRAME_S_CARD	=	GFL_BG_FRAME2_S, 
	BG_FRAME_S_BACK	=	GFL_BG_FRAME3_S, 
} ;

//-------------------------------------
///	パレット
//=====================================
enum
{
	//メイン画面BG
	PLT_BG_M	  =	0,
  PLT_LIST_M  = 13,
  PLT_TEXT_M  = 14,
	PLT_FONT_M	= 15,

	//サブ画面BG
	PLT_BG_S	  = 0,
	PLT_FONT_S	= 15,

	//メイン画面OBJ
  PLT_OBJ_M   = 0,
  PLT_OBJ_PMS_M = 5,

  PLT_OBJ_POKEMON_M = PLT_OBJ_PMS_M,  //同時に表示しないので同じ位置
  PLT_OBJ_POKEITEM_M  = PLT_OBJ_POKEMON_M + POKEICON_PAL_MAX,
  PLT_OBJ_TRAINER_M,
	
	//サブ画面OBJ
  PLT_OBJ_S   = 0,  //wifi_match common 3line
  PLT_OBJ_POKEMON_S = 3,  //pokemon 3line
  PLT_OBJ_TRAINER_S = PLT_OBJ_POKEMON_S + POKEICON_PAL_MAX,  //pokemon 1tainer 
} ;

//-------------------------------------
///	キャラ
//=====================================
enum
{ 
  //メイン画面  BG_FRAME_M_TEXT
  CGR_OFS_M_CLEAR = 0,
  CGR_OFS_M_LIST  = 1,
  CGR_OFS_M_TEXT  = 10,
};

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					リソース読み込み
 *					  共通素材を読み込みます
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	定数
//=====================================
typedef enum
{ 
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_M,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_PLT_S,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_M,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CHR_S,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_M,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_CEL_S,
  WIFIBATTLEMATCH_VIEW_RES_TYPE_OBJ_MAX,
}WIFIBATTLEMATCH_VIEW_RES_TYPE;

//-------------------------------------
///	リソース管理ワーク
//=====================================
typedef struct _WIFIBATTLEMATCH_VIEW_RESOURCE WIFIBATTLEMATCH_VIEW_RESOURCE;

//-------------------------------------
///	パブリック
//=====================================
extern WIFIBATTLEMATCH_VIEW_RESOURCE *WIFIBATTLEMATCH_VIEW_LoadResource( GFL_CLUNIT *p_unit, WIFIBATTLEMATCH_MODE mode, HEAPID heapID );
extern void WIFIBATTLEMATCH_VIEW_UnLoadResource( WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk );

extern u32 WIFIBATTLEMATCH_VIEW_GetResource( const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_wk, WIFIBATTLEMATCH_VIEW_RES_TYPE type );

extern void WIFIBATTLEMATCH_VIEW_Main( WIFIBATTLEMATCH_VIEW_RESOURCE *p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					プレイヤー情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数
*/
//=============================================================================
//サブの場合
#define PLAYERINFO_BG_FRAME	        (BG_FRAME_S_FONT)
#define PLAYERINFO_PLT_BG_FONT	    (PLT_FONT_S)
#define PLAYERINFO_PLT_OBJ_POKEICON	(PLT_OBJ_POKEMON_S)
#define PLAYERINFO_PLT_OBJ_TRAINER	(PLT_OBJ_TRAINER_S)

//メインの場合
#define PLAYERINFO_BG_FRAME_MAIN	        (BG_FRAME_M_FONT)
#define PLAYERINFO_PLT_BG_FONT_MAIN	      (PLT_FONT_M)
#define PLAYERINFO_PLT_OBJ_POKEICON_MAIN	(PLT_OBJ_POKEMON_M)
#define PLAYERINFO_PLT_OBJ_TRAINER_MAIN 	(PLT_OBJ_TRAINER_M)


#define PLAYERINFO_POKEICON_START_X		(32)
#define PLAYERINFO_POKEICON_DIFF_X		(32)
#define PLAYERINFO_POKEICON_Y					(168)

#define PLAYERINFO_TRAINER_CUP_X			(32)
#define PLAYERINFO_TRAINER_CUP_Y			(112-6)
#define PLAYERINFO_TRAINER_RND_X			(32)
#define PLAYERINFO_TRAINER_RND_Y			(74-6)

#define WBM_CARD_LOCK_POS_X           (24)
#define WBM_CARD_LOCK_POS_Y           (152)

#define PLAYERINFO_ALPHA_EV1          (15)
#define PLAYERINFO_ALPHA_EV2          (3)

#define PLAYERINFO_STR_COLOR_BLACK    (PRINTSYS_LSB_Make( 1,2,0 ))
#define PLAYERINFO_STR_COLOR_WHITE    (PRINTSYS_LSB_Make( 0xF,2,0 ))

#define WBM_CARD_WIN_BLACK  (500)
#define WBM_CARD_WIN_GOLD   (100)
#define WBM_CARD_WIN_SILVER (50)
#define WBM_CARD_WIN_BRONZE (20)
#define WBM_CARD_WIN_COPPER (10)
#define WBM_CARD_WIN_NORMAL (0)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	プレイヤー情報表示ワーク
//=====================================
typedef struct _PLAYERINFO_WORK PLAYERINFO_WORK;

//-------------------------------------
///	ランダムマッチを表示するときに必要なデータ
//=====================================
typedef struct
{
	WIFIBATTLEMATCH_BTLRULE		btl_rule;				//何のバトルか
	u16										rate;						//レーティング
	u32										trainerID;			//自分の見た目
	u16										btl_cnt;				//対戦回数
	u16										win_cnt;				//勝ち回数
	u16										lose_cnt;				//負け回数
} PLAYERINFO_RANDOMMATCH_DATA;
//-------------------------------------
///	WIFI大会の上画面に表示するデータ
//=====================================
typedef struct
{
	STRCODE								cup_name[WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE];	//大会名文字列
	GFDATE								start_date;			//大会開始
	GFDATE								end_date;				//大会終了
	u32										trainerID;			//自分の見た目
	u16										rate;						//レーティング
	u16										btl_cnt;				//対戦回数
	u16										btl_max;				//対戦回数上限
} PLAYERINFO_WIFICUP_DATA;
//-------------------------------------
///	ライブ大会の上画面に表示するデータ
//=====================================
typedef struct
{
	STRCODE								cup_name[WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE];	//大会名文字列
	GFDATE								start_date;			//大会開始
	GFDATE								end_date;				//大会終了
	u32										trainerID;			//自分の見た目
	u16										win_cnt;				//勝ち回数
	u16										lose_cnt;				//負け回数
	u16										btl_cnt;				//対戦回数
	u16										btl_max;				//対戦回数上限
} PLAYERINFO_LIVECUP_DATA;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//-------------------------------------
///	ランダムマッチ
//=====================================
extern PLAYERINFO_WORK *PLAYERINFO_RND_Init( const PLAYERINFO_RANDOMMATCH_DATA *cp_data, BOOL is_rate, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT	*p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, HEAPID heapID );
extern void PLAYERINFO_RND_Exit( PLAYERINFO_WORK *p_wk );
//-------------------------------------
///	WIFI大会
//=====================================
extern PLAYERINFO_WORK *PLAYERINFO_WIFI_Init( const PLAYERINFO_WIFICUP_DATA *cp_data, BOOL is_limit, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT	*p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BATTLE_BOX_SAVE * p_btlbox_sv, BOOL is_main, HEAPID heapID );
extern void PLAYERINFO_WIFI_Exit( PLAYERINFO_WORK *p_wk );

typedef enum
{
  PLAYERINFO_WIFI_UPDATE_TYPE_LOCK,
  PLAYERINFO_WIFI_UPDATE_TYPE_UNLOCK,
  PLAYERINFO_WIFI_UPDATE_TYPE_UNREGISTER,
} PLAYERINFO_WIFI_UPDATE_TYPE;
extern void PLAYERINFO_WIFI_RenewalData( PLAYERINFO_WORK *p_wk, PLAYERINFO_WIFI_UPDATE_TYPE type, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, GFL_FONT *p_font, HEAPID heapID );
//-------------------------------------
///	LIVE大会
//=====================================
extern PLAYERINFO_WORK *PLAYERINFO_LIVE_Init( const PLAYERINFO_LIVECUP_DATA *cp_data, const MYSTATUS* p_my, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, BATTLE_BOX_SAVE * p_btlbox_sv, HEAPID heapID );
extern void PLAYERINFO_LIVE_Exit( PLAYERINFO_WORK *p_wk );
//-------------------------------------
///	共通関数
//=====================================
extern BOOL PLAYERINFO_PrintMain( PLAYERINFO_WORK * p_wk, PRINT_QUE *p_que );
extern BOOL PLAYERINFO_MoveMain( PLAYERINFO_WORK * p_wk );

#ifdef DEBUG_DATA_CREATE
extern void PLAYERINFO_DEBUG_CreateRndData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs );
extern void PLAYERINFO_DEBUG_CreateWifiData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs );
extern void PLAYERINFO_DEBUG_CreateLiveData( WIFIBATTLEMATCH_MODE mode, void *p_data_adrs );
#else
#define PLAYERINFO_DEBUG_CreateRndData(a,b)		/*  */
#define PLAYERINFO_DEBUG_CreateWifiData(a,b)		/*  */
#define PLAYERINFO_DEBUG_CreateLiveData(a,b)		/*  */
#endif //DEBUG_DATA_CREATE

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					対戦者情報表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//=============================================================================
/**
 *					定数
*/
//=============================================================================
#define MATCHINFO_BG_FRAME	  (BG_FRAME_M_FONT)
#define MATCHINFO_PLT_BG_FONT	(PLT_FONT_M)
#define MATCHINFO_PLT_OBJ_PMS	(PLT_OBJ_PMS_M)
#define MATCHINFO_PLT_OBJ_TRAINER	(MATCHINFO_PLT_OBJ_PMS+PMS_DRAW_OBJ_PLTT_NUM)

#define MATCHINFO_TRAINER_X			(32)
#define MATCHINFO_TRAINER_Y			(48 - 6)
//=============================================================================
/**
 *					構造体宣言
 */
//=============================================================================
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
typedef struct _MATCHINFO_WORK MATCHINFO_WORK;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
extern MATCHINFO_WORK * MATCHINFO_Init( const WIFIBATTLEMATCH_ENEMYDATA *cp_data, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, WORDSET *p_word, WIFIBATTLEMATCH_MODE mode, HEAPID heapID );
extern void MATCHINFO_Exit( MATCHINFO_WORK *p_wk );
extern BOOL MATCHINFO_PrintMain( MATCHINFO_WORK *p_wk, PRINT_QUE *p_que );
extern BOOL MATCHINFO_MoveMain( MATCHINFO_WORK * p_wk );

//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *					待機中OBJ
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
typedef struct _WBM_WAITICON_WORK WBM_WAITICON_WORK;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
extern WBM_WAITICON_WORK	* WBM_WAITICON_Init( GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, HEAPID heapID );
extern void WBM_WAITICON_Exit( WBM_WAITICON_WORK *p_wk );
extern void WBM_WAITICON_Main( WBM_WAITICON_WORK *p_wk );
extern void WBM_WAITICON_SetDrawEnable( WBM_WAITICON_WORK *p_wk, BOOL on_off );


//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
/**
 *				  バトルボックス表示
*/
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
//-------------------------------------
///	対戦者情報表示ワーク
//=====================================
typedef struct _WBM_BTLBOX_WORK WBM_BTLBOX_WORK;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
extern WBM_BTLBOX_WORK	* WBM_BTLBOX_Init( BATTLE_BOX_SAVE * cp_btl_box, GFL_CLUNIT *p_unit, const WIFIBATTLEMATCH_VIEW_RESOURCE *cp_res, GFL_FONT *p_font, PRINT_QUE *p_que, GFL_MSGDATA *p_msg, HEAPID heapID );
extern void WBM_BTLBOX_Exit( WBM_BTLBOX_WORK *p_wk );
extern BOOL WBM_BTLBOX_MoveInMain( WBM_BTLBOX_WORK *p_wk );
extern BOOL WBM_BTLBOX_MoveOutMain( WBM_BTLBOX_WORK *p_wk );
extern BOOL WBM_BTLBOX_PrintMain( WBM_BTLBOX_WORK *p_wk, PRINT_QUE *p_que );


#ifdef DEBUG_DATA_CREATE
//extern void MATCHINFO_DEBUG_CreateData( MATCHINFO_DATA *p_data );
#else
#define MATCHINFO_DEBUG_CreateData(a)		/*  */
#endif //DEBUG_DATA_CREATE
