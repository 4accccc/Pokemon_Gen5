//==============================================================================
/**
 * @file    intrude_save.h
 * @brief   侵入セーブデータ
 * @author  matsuda
 * @date    2009.10.18(日)
 */
//==============================================================================
#pragma once

#include "gamesystem/gamedata_def.h"  //GAMEDATA
#include "field/intrude_common.h"
#include "buflen.h"
#include "field/gpower_id.h"
#include "field/field_comm/mission_types.h"


//==============================================================================
//  定数定義
//==============================================================================
enum {
  OCCUPY_ID_MAX = FIELD_COMM_MEMBER_MAX + 1,

  OCCUPY_ID_MINE = OCCUPY_ID_MAX - 1,   ///<自分自身の占拠情報ID
};

///占拠レベルの最大値
#define OCCUPY_LEVEL_MAX    (999)

///ミッションクリア状況
enum{
  MISSION_CLEAR_NONE,       ///<クリアしていない
  MISSION_CLEAR_WHITE,      ///<白でクリア達成
  MISSION_CLEAR_BLACK,      ///<黒でクリア達成
};

//==============================================================================
//  構造体定義
//==============================================================================
///侵入セーブ構造体の不完全型
typedef struct _INTRUDE_SAVE_WORK INTRUDE_SAVE_WORK;

//--------------------------------------------------------------
//  占拠
//--------------------------------------------------------------
///ミッションリストステータス
typedef struct{
  u8 mission_no[MISSION_TYPE_MAX];      ///<ミッション番号
  u8 mission_clear[MISSION_TYPE_MAX];   ///<ミッションクリア状況(MISSION_CLEAR_xxx)
}MISSION_LIST_STATUS;

///占拠情報
typedef struct{
  MISSION_LIST_STATUS mlst;              ///<ミッションリストステータス
  u16 white_level;                       ///<白レベル
  u16 black_level;                       ///<黒レベル
}OCCUPY_INFO;

//--------------------------------------------------------------
//  隠しアイテム
//--------------------------------------------------------------
///侵入隠しアイテムセーブワーク
typedef struct{
  STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];  ///<隠した人の名前
  u16 item;                                   ///<アイテム番号
  u8 tbl_no;                                  ///<隠されている場所を示すテーブル番号
  u8 padding;
}INTRUDE_SECRET_ITEM_SAVE;


//==============================================================================
//  
//==============================================================================
extern INTRUDE_SAVE_WORK * SaveData_GetIntrude( SAVE_CONTROL_WORK * p_sv);


//============================================================================================
//============================================================================================
//--------------------------------------------------------------
/**
 * @brief   OCCUPY_INFOへのポインタ取得(プレイヤーID指定)
 * @param   gamedata		GAMEDATAへのポインタ
 * @param   player_id   プレイヤーID
 * @retval  OCCUPY_INFOへのポインタ
 * @note
 * 実態はsrc/gamesystem/game_data.cにあるが、情報隠蔽を
 * 考慮してアクセス関数群を扱うヘッダに配置している
 */
//--------------------------------------------------------------
extern OCCUPY_INFO * GAMEDATA_GetOccupyInfo(GAMEDATA * gamedata, u32 player_id);
//--------------------------------------------------------------
/**
 * @brief   OCCUPY_INFOへのポインタ取得
 * @param   gamedata		GAMEDATAへのポインタ
 * @retval  OCCUPY_INFOへのポインタ
 * @note
 * 実態はsrc/gamesystem/game_data.cにあるが、情報隠蔽を
 * 考慮してBGM_INFO_SYSを扱うヘッダに配置している
 */
//--------------------------------------------------------------
extern OCCUPY_INFO * GAMEDATA_GetMyOccupyInfo(GAMEDATA * gamedata);

//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 IntrudeSave_GetWorkSize( void );
extern void IntrudeSave_WorkInit(void *work);

//--------------------------------------------------------------
//  占拠情報
//--------------------------------------------------------------
extern void OccupyInfo_WorkInit(OCCUPY_INFO *occupy);
extern void SaveData_OccupyInfoUpdate(SAVE_CONTROL_WORK * sv, const OCCUPY_INFO *occupy);
extern void SaveData_OccupyInfoLoad(SAVE_CONTROL_WORK * sv, OCCUPY_INFO *dest_occupy);

//--------------------------------------------------------------
//  隠しアイテム
//--------------------------------------------------------------
extern void ISC_SAVE_SetItem(INTRUDE_SAVE_WORK *intsave, const INTRUDE_SECRET_ITEM_SAVE *src);

//--------------------------------------------------------------
//  Gパワー
//--------------------------------------------------------------
extern void ISC_SAVE_SetGPowerID(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id);
extern GPOWER_ID ISC_SAVE_GetGPowerID(INTRUDE_SAVE_WORK *intsave);
extern void ISC_SAVE_SetDistributionGPower(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id);
extern BOOL ISC_SAVE_GetDistributionGPower(INTRUDE_SAVE_WORK *intsave, GPOWER_ID gpower_id);
