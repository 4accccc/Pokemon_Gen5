//============================================================================================
/**
 * @file  wbt_system.h
 * @brief WBTシステムヘッダ
 * @author  tamada GAME FREAK inc.
 * @date  2011.02.13
 *
 */
//============================================================================================
#pragma once

#include "field/wbt.h"

#include "battle/battle.h"

#include "poke_tool/poke_regulation.h"  //REGULATION

#include "app/pokelist.h" //PLIST_DATA
#include "app/p_status.h" //PSTATUS_DATA


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
/// 対戦トレーナーデータの不完全型
//------------------------------------------------------------------
typedef struct _WBTTRAINER WBTTRAINER;

//------------------------------------------------------------------
/// 対戦結果データの不完全型
//------------------------------------------------------------------
typedef struct _WBTRESULT WBTRESULT;

//------------------------------------------------------------------
/// 対戦組み合わせ生成条件の不完全型
//------------------------------------------------------------------
typedef struct _WBT_MATCH_CONDITION WBT_MATCH_CONDITION;

//------------------------------------------------------------------
///
//------------------------------------------------------------------
typedef struct _WBT_TRCODE WBT_TRCODE;

//------------------------------------------------------------------
/// ポケモン抽選用NG指定オブジェクトの不完全型
//------------------------------------------------------------------
typedef struct _WBT_NGPMINFO WBT_NGPMINFO;

//------------------------------------------------------------------
///ミックストーナメント用ウィンドウの不完全型
//------------------------------------------------------------------
typedef struct _WBT_MIXWIN WBT_MIXWIN;

//------------------------------------------------------------------
/// ミックストーナメントでの選択結果の保存指定
//------------------------------------------------------------------
typedef enum {
  WBTMIX_TARGET_MINE1 = 0,  ///<自分手持ちからの選択その１（＝相手の選択）
  WBTMIX_TARGET_MINE2,      ///<自分手持ちからの選択その２（＝相手の選択）
  WBTMIX_TARGET_ENEMY1,     ///<相手手持ちからの選択その１（＝自分の選択）
  WBTMIX_TARGET_ENEMY2,     ///<相手手持ちからの選択その２（＝自分の選択）

  WBTMIX_TARGET_NUM,
}WBTMIX_TARGET;

//------------------------------------------------------------------
/// レンタルトーナメントでの抽選タイプ
//------------------------------------------------------------------
typedef enum {
  WBTRENT_TYPE_WEAK = 0,    ///<レンタル抽選：弱いグループから
  WBTRENT_TYPE_STRONG,      ///<レンタル抽選：強いグループから
  WBTRENT_TYPE_ALL,         ///<レンタル抽選：強弱両方のグループから

  WBTRENT_TYPE_NUM,
}WBTRENT_TYPE;

//------------------------------------------------------------------
//  WBT対戦相手用のメッセージオフセット
//------------------------------------------------------------------
typedef enum {
  WBT_TRMSG_1ST_FIGHT_START = 0,  ///<バトル前、対戦相手の1回戦時
  WBT_TRMSG_2ND_FIGHT_START,      ///<バトル前、対戦相手の2回戦時
  WBT_TRMSG_3RD_FIGHT_START,      ///<バトル前、対戦相手の決勝戦時
  WBT_TRMSG_FIGHT_LOSE,           ///<バトル内、対戦相手の負けMSG
  WBT_TRMSG_FIGHT_WIN,            ///<バトル内、対戦相手の勝ちMSG
  WBT_TRMSG_PLAYER_WIN,           ///<バトル後、対戦相手の負けMSG
  WBT_TRMSG_PLAYER_LOSE,          ///<バトル後、対戦相手の勝ちMSG
  WBT_TRMSG_ANNOUNCE,             ///<バトル前、決勝戦の前口上

  WBT_TRMSG_TYPE_NUM,
}WBT_TRMSG;

//------------------------------------------------------------------
//  WBT対戦相手のビット指定
//------------------------------------------------------------------
enum {
  WBT_AI_ID_BASIC = 0,  ///<基本AI
  WBT_AI_ID_STRONG,     ///<威力強AI
  WBT_AI_ID_EXPERT,     ///<エキスパートAI
  WBT_AI_ID_DOUBLE,     ///<ダブルバトルAI

  WBT_AI_BIT_BASIC = 1 << WBT_AI_ID_BASIC,
  WBT_AI_BIT_STRONG = 1 << WBT_AI_ID_STRONG,
  WBT_AI_BIT_EXPERT = 1 << WBT_AI_ID_EXPERT,
  WBT_AI_BIT_DOUBLE = 1 << WBT_AI_ID_DOUBLE,
};

//------------------------------------------------------------------
/// WBT会場の観客パターン数
//------------------------------------------------------------------
enum { WBT_AUDIENCE_TYPE_MAX = 4 };

//------------------------------------------------------------------
/// トーナメント名称の長さ
//------------------------------------------------------------------
#define BUFLEN_TOURNAMENT_NAME  ( (18 * GLOBAL_MSGLEN) + BUFLEN_EOM_SIZE )	// 16 -> 18

//------------------------------------------------------------------
//------------------------------------------------------------------
enum {
  ///  対戦したかどうかが記録されるWBTROMトレーナーの数
  WBTROM_TRID_SWLEADER1 = 0,              ///<チェレン
  WBTROM_TRID_RED = 53,                   ///<レッド
  WBTROM_TRID_MEM_MAX = 54,
  WBTROM_TRID_RIVAL = 55,                 ///<ライバル
  WBTROM_TRID_SWLEADER1_EVENT = 56,       ///<チェレン（ホドモエイベント戦）
  WBTROM_TRID_TYPEBATTLE_MOB_START = 58,
  WBTROM_TRID_BATTLE_MOB_START = 68,
  WBTROM_TRID_NOBATTLE_MOB_START = 78,
};

//============================================================================================
//============================================================================================
extern WBTSYS * WBTSYS_Create( HEAPID heapID, BOOL isTutorial );
extern void WBTSYS_Delete( WBTSYS * wbtsys );

//------------------------------------------------------------------
//  settter/getter
//------------------------------------------------------------------
extern void WBTSYS_SetWBTCup( WBTSYS * wbtsys, WBTCUP wbtcup );
extern WBTCUP WBTSYS_GetWBTCup( const WBTSYS * wbtsys );

extern void WBTSYS_SetRule( WBTSYS * wbtsys, WBTRULE rule );
extern WBTRULE WBTSYS_GetRule( const WBTSYS * wbtsys );

extern void WBTSYS_SetPokeType( WBTSYS * wbtsys, PokeType poketype );
extern PokeType WBTSYS_GetPokeType( const WBTSYS * wbtsys );

extern WBTRECEPT WBTSYS_GetReceptionID( const WBTSYS * wbtsys );
extern void WBTSYS_SetReceptionID( WBTSYS * wbtsys, WBTRECEPT reception_id );

extern void WBTSYS_SetStage( WBTSYS * wbtsys, WBTSTAGE stage );
extern WBTSTAGE WBTSYS_GetStage( const WBTSYS * wbtsys );

extern u64 WBTSYS_GetRandomSeed( const WBTSYS * wbtsys );
extern u8 WBTSYS_GetAudienceType( const WBTSYS * wbtsys );

extern POKEPARTY * WBTSYS_GetMyParty( WBTSYS * wbtsys );
extern POKEPARTY * WBTSYS_GetEnemyParty( WBTSYS * wbtsys );

extern void WBTSYS_SetSelectDLData( WBTSYS * wbtsys, u8 no );
extern u8 WBTSYS_GetSelectDLData( const WBTSYS * wbtsys );

extern void WBTSYS_SetScriptAnswerWork( WBTSYS * wbtsys, u16 * scr_answer_work );
extern u16 * WBTSYS_GetScriptAnswerWork( WBTSYS * wbtsys );

extern void WBTSYS_SetBattleBoxUse( WBTSYS * wbtsys, BOOL use_flag );

extern void WBTSYS_SetMixInfoWinPointer( WBTSYS * wbtsys, WBT_MIXWIN * www );
extern WBT_MIXWIN * WBTSYS_GetMixInfoWinPointer( WBTSYS * wbtsys );

extern void WBTSYS_SetRentalType( WBTSYS * wbtsys, WBTRENT_TYPE rental_type );
extern WBTRENT_TYPE WBTSYS_GetRentalType( const WBTSYS * wbtsys );

extern void WBTSYS_GetTournamentName( const WBTSYS * wbtsys, WBTCUP wbtcup, STRBUF * strbuf );

//------------------------------------------------------------------
//  レギュレーション関連
//------------------------------------------------------------------
extern u32 WBTSYS_GetRegulationID( WBTSYS * wbtsys );
extern const REGULATION * WBTSYS_GetRegulation( WBTSYS * wbtsys );
extern const REGULATIONEX * WBTSYS_GetRegulationEx( WBTSYS * wbtsys );
extern u8 WBTSYS_GetPartyMax( WBTSYS * wbtsys );

//------------------------------------------------------------------
/// 対戦トレーナーデータ取得関連
//------------------------------------------------------------------
extern const WBTTRAINER * WBTSYS_GetTrainer( const WBTSYS * wbtsys, u32 pos );
extern const WBTTRAINER * WBTSYS_GetPlayerTrainer( const WBTSYS * wbtsys );

//------------------------------------------------------------------
//  WBTTRAINER関連
//------------------------------------------------------------------
extern u8 WBTTRAINER_GetID( const WBTTRAINER * wbt_tr );
extern u8 WBTTRAINER_GetDLID( const WBTTRAINER * wbt_tr );
extern u8 WBTTRAINER_GetPriority( const WBTTRAINER * wbt_tr );
extern u8 WBTTRAINER_GetPowerRnd( const WBTTRAINER * wbt_tr );
extern u16 WBTTRAINER_GetBattleTrainerType( const WBTTRAINER * wbt_tr );
extern WBT_TRTYPE WBTTRAINER_GetWBTTrainerType( const WBTTRAINER * wbt_tr );
extern void WBTTRAINER_CopyName( const WBTTRAINER * wbt_tr, STRBUF * buf );
extern void WBTTRAINER_CopyTitle( const WBTTRAINER * wbt_tr, STRBUF * buf );
extern BOOL WBTTRAINER_HasAce( const WBTTRAINER * wbt_tr );

//============================================================================================
//============================================================================================
#ifdef  PM_DEBUG
extern void WBTTRAINER_DEBUG_dump( const WBTTRAINER * wbt_tr );
extern void WBTSYS_DEBUG_dumpTrainer( const WBTSYS * wbtsys );
extern void WBTSYS_DEBUG_PrintRegulation( const REGULATION * cp_data );
extern void WBTSYS_DEBUG_PrintRegulationEx( const REGULATIONEX * reg_ex );
#endif

//============================================================================================
//
//
//
//============================================================================================
//------------------------------------------------------------------
//  in wbt_system_lobby.c
//------------------------------------------------------------------
extern void WBTSYS_MakeMatch( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_SetupTrainer( WBTSYS * wbtsys, GAMEDATA * gamedata, const WBT_TRCODE * tr_list );

extern POKEPARTY * WBTSYS_GetUsePokeParty( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_SetupBattleBox( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern u8 WBTSYS_GetRewardCount( const WBTSYS * wbtsys );

extern u32 WBTSYS_GetDownLoadCount( const WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_LoadDLData( WBTSYS * wbtsys, GAMEDATA * gamedata );
extern void WBTSYS_UnloadDLData( WBTSYS * wbtsys );

extern void WBTSYS_SetupPokeListData( WBTSYS * wbtsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void WBTSYS_SetupPokeStatusData( WBTSYS * wbtsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );
extern void WBTSYS_SetPokelistResult( WBTSYS * wbtsys, const PLIST_DATA * plist_data );

extern void WBTSYS_SetupMyParty( WBTSYS * wbtsys, GAMEDATA * gamedata );

//------------------------------------------------------------------
//  in wbt_makematch.c
//------------------------------------------------------------------
extern void WBTSYS_MakeEntryList(
    GAMEDATA * gamedata, const WBT_MATCH_CONDITION * cond, WBT_TRCODE * tr_list );
extern void WBTSYS_SortTrainer( WBTTRAINER * wbt_tr_array, u64 seed64, HEAPID heapID );

//------------------------------------------------------------------
//  in wbt_party.c
//------------------------------------------------------------------
extern void WBTSYS_ChooseBSubwayData( u16 * index_array, WBT_NGPMINFO * ng_info,
    HEAPID tempHeapID,
    u16 use_poke_cnt, const u16 * use_poke_table,
    ARCID arc_id, u8 num, GFL_STD_RandContext * rand );
extern void WBTSYS_SetupMyRentalParty(
    WBTSYS * wbtsys, WBTRENT_TYPE rent_type, const MYSTATUS * mystatus, u16 place_id );
extern void WBTSYS_SetupEnemyRentalParty( WBTSYS * wbtsys );

extern WBT_NGPMINFO * NGINFO_Create( HEAPID heapID, WBTCUP wbtcup );
extern void WBT_NGINFO_Delete( WBT_NGPMINFO * ng_info );
extern void WBT_NGINFO_AddNGItem( WBT_NGPMINFO * ng_info, u16 item_no );
extern void WBT_NGINFO_AddNGMons( WBT_NGPMINFO * ng_info, u16 mons_no );
extern void WBT_NGINFO_AddNGIndex( WBT_NGPMINFO * ng_info, s16 ng_index );

//------------------------------------------------------------------
//  in wbt_calc_result.c
//------------------------------------------------------------------
extern u32 WBTSYS_GetOpponentPos( const WBTSYS * wbtsys );
extern const WBTTRAINER * WBTSYS_GetOpponentTrainer( const WBTSYS * wbtsys );
extern void WBTSYS_CalcResult( WBTSYS * wbtsys, GFL_STD_RandContext * context );

//------------------------------------------------------------------
//  in wbt_battle.c
//------------------------------------------------------------------
extern void WBTSYS_GetTrainerMsg( const WBTSYS * wbtsys, WBT_TRMSG msg_type, STRBUF * strbuf );
extern void WBTSYS_GetOpponentName( const WBTSYS * wbtsys, STRBUF * strbuf );
extern u8 WBTSYS_GetOpponentSex( const WBTSYS * wbtsys );
extern u16 WBTSYS_GetOpponentMMdlObjCode( const WBTSYS * wbtsys );
extern u16 WBTSYS_GetOpponentTrainerType( const WBTSYS * wbtsys );
extern void WBTSYS_GetOpponentBGM( const WBTSYS * wbtsys, u16 *btl_bgm, u16 *win_bgm );

extern void WBTSYS_MixInit( WBTSYS * wbtsys );
extern void WBTSYS_MixSetSelect( WBTSYS * wbtsys, WBTMIX_TARGET target, u8 select_pos );
extern void WBTSYS_MixSetRandomSelect( WBTSYS * wbtsys, WBTMIX_TARGET target );
extern u8 WBTSYS_MixGetSelected( const WBTSYS * wbtsys, WBTMIX_TARGET target );
extern void WBTSYS_SetupMixParty( WBTSYS * wbtsys );

extern WBT_MIXWIN * WBT_MIXWIN_Create( void * wk, WBTSYS * wbtsys );
extern void WBT_MIXWIN_Draw( WBT_MIXWIN * wmw );
extern void WBT_MIXWIN_Erase( WBT_MIXWIN * wmw );
extern void WBT_MIXWIN_Delete( WBT_MIXWIN * wmw );

extern void WBTSYS_SetupEnemyParty( WBTSYS * wbtsys, GAMESYS_WORK * gsys, const WBTTRAINER * wbt_tr );
extern BATTLE_SETUP_PARAM * WBTSYS_CreateBattleSetupParam( WBTSYS * wbtsys, GAMESYS_WORK * gsys );
extern void WBTSYS_DeleteBattleSetupParam( WBTSYS * wbtsys, BATTLE_SETUP_PARAM * bsp );

extern void WBTSYS_ReflectBattleResult(
    WBTSYS * wbtsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp );

extern u8 WBTSYS_GetSurvivalCount( const WBTSYS * wbtsys, WBTSTAGE stage );

