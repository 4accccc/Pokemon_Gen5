//===============================================================================
/*
 *  @file   fes_gimmick_npc.h
 *  @brief  フェスミッション NPCミッションギミック
 *  @author Miyuki Iwasawa
 *  @date   11.09.09
 */
//===============================================================================

#pragma once

#define FES_GIMMICK_NPC_DATA_MAX (20) ///<1ゾーンでmax20箇所候補にできる

#define FES_GIMMICK_NPC_POINT_LOW_MIN (0)     ///<ポイント抽選数下限 少なめ(街)
#define FES_GIMMICK_NPC_POINT_LOW_MAX (3)     ///<ポイント抽選数上限 少なめ(街)
#define FES_GIMMICK_NPC_POINT_MIDDLE_MIN (1)  ///<ポイント抽選数下限 中(道路)
#define FES_GIMMICK_NPC_POINT_MIDDLE_MAX (3)  ///<ポイント抽選数上限 中(道路)
#define FES_GIMMICK_NPC_POINT_HIGH_MIN (3)  ///<ポイント抽選数下限 中(ダンジョン)
#define FES_GIMMICK_NPC_POINT_HIGH_MAX (3)  ///<ポイント抽選数上限 中(ダンジョン)

#define FES_GIMMICK_NPC_POINT_MAX (FES_GIMMICK_NPC_POINT_HIGH_MAX) ///<1ゾーンでmax3箇所抽選される

#define FES_GIMMICK_NPC_FLAG_WORK_NUM  (6)   ///<過去6ゾーン分確保

#define FES_GIMMICK_NPC_POINT_PARAM_MAX (3) //ポイントごとに保存するパラメータ数

#define FES_GIMMICK_NPC_TMPDATA_BUF_SIZE  (0x500) //テンポラリデータ領域500h

///MMDL ParamNo
#define NITEM_PRESENT_PARAM_ITEM  (0)

#define NITEM_TRADE_PARAM_MY_CATEGORY (0)
#define NITEM_TRADE_PARAM_NPC_CATEGORY  (1)
#define NITEM_TRADE_PARAM_TBLNO (2)

#define NITEM_SHOPPING_PARAM_ITEM (0)
#define NITEM_SHOPPING_PARAM_PRICE  (2)

#define NITEM_GPRESENT_PARAM_ITEM (0)

#define NPC_TRAINER_PARAM_TRID  (0)

#define NPC_POKE_QUIZ_PARAM_MONSNO    (0)

//記憶クイズ出題レンジ
#define NPC_MEMORY_QUIZ_QUESTION_MIN  (3)   //最小3題
#define NPC_MEMORY_QUIZ_QUESTION_MAX  (10)  //最大10題
#define NPC_MEMORY_QUIZ_ANSWER_MAX    (5)   //答え候補数


#ifndef __ASM_NO_DEF_

#include "field/eventdata_local.h"
#include "field/eventdata_type.h"

//フェスミッションNPCギミックフラグ
typedef struct _FES_GIMMICK_NPC_FLAG{
  u16 zone_id;
  u8  flag[FES_GIMMICK_NPC_POINT_MAX];
}FES_GIMMICK_NPC_FLAG;

//フェスミッションNPCギミックフラグ領域
typedef struct _FES_GIMMICK_NPC_FLAG_WORK{
  int tbl_num;
  FES_GIMMICK_NPC_FLAG tbl[FES_GIMMICK_NPC_FLAG_WORK_NUM];
}FES_GIMMICK_NPC_FLAG_WORK;

//フェスミッションNPCギミック 記憶クイズ用ワーク領域
typedef struct _FES_GIMMICK_NPC_MEMORY_QUIZ_WORK{
  u8  q_num;  //出題数
  u8  question_idx; //出題No
  u8  answer_idx; //正解index

  u16 q_poke[NPC_MEMORY_QUIZ_QUESTION_MAX];  //出題ポケモンリスト
  u16 a_poke[NPC_MEMORY_QUIZ_ANSWER_MAX];    //回答ポケモンリスト
}FES_GIMMICK_NPC_MEMORY_QUIZ_WORK;

//フェスアイテムミッションギミックワーク
typedef struct _FES_GIMMICK_NPC_WORK{
  LINK_FESTIVAL* fest_sys;

  u16 zone_id; ///<現在のゾーンID

  u8  data_max;  ///<ゾーンに登録されたデータmax
  u8  point_max; ///<抽選ポイント数
  BG_TALK_DATA  data[FES_GIMMICK_NPC_DATA_MAX];  ///<ポジション候補データテーブル
  u8  idx[FES_GIMMICK_NPC_POINT_MAX];  ///<抽選されたデータの参照indexテーブル
  u16 param[FES_GIMMICK_NPC_POINT_MAX][FES_GIMMICK_NPC_POINT_PARAM_MAX]; ///<抽選されたパラメータテーブル
  MMDL* mmdl_tbl[FES_GIMMICK_NPC_POINT_MAX];
  

  //取得済みログデータバッファ
  FES_GIMMICK_NPC_FLAG_WORK flag_wk;

  int   tmp_data_size;  //テンポラリバッファサイズ保持領域
  u8    tmp_data_buf[FES_GIMMICK_NPC_TMPDATA_BUF_SIZE];

}FES_GIMMICK_NPC_WORK;

//NOTWIFI
extern void FES_GIMMICK_NPC_ZoneChangeSetup( FES_GIMMICK_NPC_WORK* wk, GAMESYS_WORK* gsys, LINK_FESTIVAL* fest_sys, FEST_MISSION* fmwk, u16 zone_id );
extern u8 FES_GIMMICK_NPC_SUB_GetObjType( const FEST_MISSION* fmwk );
extern u16 FES_GIMMICK_NPC_SUB_GetObjCode( FES_GIMMICK_NPC_WORK* wk, const FEST_MISSION* fmwk );
extern FES_GIMMICK_NPC_FLAG* FES_GIMMICK_NPC_SUB_FlagTableGet( FES_GIMMICK_NPC_WORK* wk, u16 zone_id );

//FIELDMAP
extern void FES_GIMMICK_NPC_MMdlSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
extern void FES_GIMMICK_NPC_FieldRecoverSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
extern void FES_GIMMICK_NPC_FlagSet( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk, u16 zone_id, u16 idx );
extern void FES_GIMMICK_NPC_MissionInitSetup( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );
extern BOOL FES_GIMMICK_NPC_Delete( FES_GIMMICK* sys_wk, FES_GIMMICK_NPC_WORK* wk );

extern void FES_GIMMICK_NPC_FesTrainerPartySet( FES_GIMMICK* sys_wk, POKEPARTY* party );
extern void FES_GIMMICK_NPC_TradeItemGet( FES_GIMMICK* sys_wk, MMDL* mmdl, u16 my_rank, u16 npc_rank, u16* my_item, u16* npc_item );
extern void FES_GIMMICK_NPC_ShoppingItemGet( FES_GIMMICK* sys_wk, MMDL* mmdl, u16* item, u16* price );
extern void FES_GIMMICK_NPC_MemoryQuizParamGet( FES_GIMMICK* sys_wk,u16* q_num, u16* q_idx, u16* a_idx );
extern u16 FES_GIMMICK_NPC_MemoryQuizParamGetQuestionPoke( FES_GIMMICK* sys_wk, u8 idx );
extern u16 FES_GIMMICK_NPC_MemoryQuizParamGetAnswerPoke( FES_GIMMICK* sys_wk, u8 idx );

#endif //__ASM_NO_DEF_

