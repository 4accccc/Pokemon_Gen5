//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		misc.h
 *	@brief	未分類セーブデータアクセス
 *	@author	Toru=Nagihashi
 *	@date		2009.10.14
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#include "savedata/save_control.h"
#include "system/pms_data.h"
#include "field/research_team_def.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
///すれ違い通信でお礼を言われた回数の最大数
#define CROSS_COMM_THANKS_RECV_COUNT_MAX    (99999)
///すれ違い通信した人数の最大数
#define CROSS_COMM_SURETIGAI_COUNT_MAX      (99999)

enum {
  MY_GOLD_MAX = 999999,
};

#define NAMEIN_SAVEMODE_MAX (8)

///タイトルメニューフラグ定義
typedef enum {
	MISC_STARTMENU_FLAG_OPEN = 1,		///<表示許可
	MISC_STARTMENU_FLAG_VIEW = 2,		///<表示済み
}MISC_STARTMENU_FLAG;

///タイトルメニューフラグ設定定義
typedef enum {
	MISC_STARTMENU_TYPE_HUSHIGI = 0,	///<「不思議な贈り物」メニュー表示
	MISC_STARTMENU_TYPE_BATTLE,				///<「バトル大会」メニュー表示
	MISC_STARTMENU_TYPE_GAMESYNC,			///<「ゲームシンク設定」メニュー表示
	MISC_STARTMENU_TYPE_MACHINE,			///<「転送マシンを使う」メニュー表示
}MISC_STARTMENU_TYPE;


//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	未分類セーブデータ不完全型
//=====================================
typedef struct _MISC MISC;

//=============================================================================
/**
 *					外部参照
*/
//=============================================================================
//----------------------------------------------------------
//セーブデータシステムに依存する関数
//----------------------------------------------------------
extern int MISC_GetWorkSize( void );
extern void MISC_Init( MISC *p_msc );

//----------------------------------------------------------
//セーブデータ取得のための関数
//----------------------------------------------------------
extern const MISC * SaveData_GetMiscConst( const SAVE_CONTROL_WORK * cp_sv);
extern MISC * SaveData_GetMisc( SAVE_CONTROL_WORK * p_sv);

//----------------------------------------------------------
//それぞれの取得関数
//----------------------------------------------------------
//名前入力
extern void MISC_SetNameInMode( MISC *p_misc, u32 mode, u8 input_type );
extern u8 MISC_GetNameInMode( const MISC *cp_misc, u32 mode );
//GDS
extern void MISC_SetFavoriteMonsno(MISC * misc, int monsno, int form_no, int egg_flag);
extern void MISC_GetFavoriteMonsno(const MISC * misc, int *monsno, int *form_no, int *egg_flag);
//palpark
extern const u32  MISC_GetPalparkHighscore(const MISC *misc);
extern void  MISC_SetPalparkHighscore(MISC *misc , u32 score);
extern const u8  MISC_GetPalparkFinishState(const MISC *misc);
extern void  MISC_SetPalparkFinishState(MISC *misc , u8 state);
//すれ違い
extern u32 MISC_CrossComm_GetThanksRecvCount(const MISC *misc);
extern u32 MISC_CrossComm_IncThanksRecvCount(MISC *misc);
extern u32 MISC_CrossComm_GetSuretigaiCount(const MISC *misc);
extern u32 MISC_CrossComm_IncSuretigaiCount(MISC *misc);
extern const STRCODE * MISC_CrossComm_GetSelfIntroduction(const MISC *misc);
extern void MISC_CrossComm_SetSelfIntroduction(MISC *misc, const STRBUF *srcbuf);
extern const STRCODE * MISC_CrossComm_GetThankyouMessage(const MISC *misc);
extern void MISC_CrossComm_SetThankyouMessage(MISC *misc, const STRBUF *srcbuf);
extern int MISC_CrossComm_GetResearchTeamRank(const MISC *misc);
extern void MISC_CrossComm_SetResearchTeamRank(MISC *misc, int rank);
//バッジ  
extern BOOL MISC_GetBadgeFlag(const MISC *misc, int badge_id);
extern void MISC_SetBadgeFlag(MISC *misc, int badge_id);
extern int MISC_GetBadgeCount(const MISC *misc);
//お金 
extern u32 MISC_GetGold(const MISC *misc);
extern u32 MISC_SetGold(MISC *misc, u32 gold);
extern u32 MISC_AddGold(MISC *misc, u32 add);
extern u32 MISC_SubGold(MISC *misc, u32 sub);
//ジム勝利時のデータ登録
extern void MISC_SetGymVictoryInfo( MISC * misc, int gym_id, const u16 * monsnos );
extern void MISC_GetGymVictoryInfo( const MISC * misc, int gym_id, u16 * monsnos );
//タイトルメニュー
extern void MISC_SetStartMenuFlag( MISC * misc, MISC_STARTMENU_TYPE type, MISC_STARTMENU_FLAG flg );
extern u8 MISC_GetStartMenuFlag( const MISC * misc, MISC_STARTMENU_TYPE type );
// すれ違い調査隊
extern u8 MISC_GetResearchRequestID( const MISC* misc );
extern void MISC_SetResearchRequestID( MISC* misc, u8 id );
extern u16 MISC_GetResearchStartCount( const MISC* misc, u8 idx );
extern void MISC_SetResearchStartCount( MISC* misc, u8 idx, u16 count );
extern s64 MISC_GetResearchStartTimeBySecond( const MISC* misc );
extern void MISC_SetResearchStartTimeBySecond( MISC* misc, s64 time );
