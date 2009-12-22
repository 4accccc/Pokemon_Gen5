//==============================================================================
/**
 * @file    intrude_main.h
 * @brief   侵入メインのヘッダ
 * @author  matsuda
 * @date    2009.09.03(木)
 */
//==============================================================================
#pragma once

#include "field/field_subscreen.h"
#include "intrude_types.h"


//==============================================================================
//  定数定義
//==============================================================================
///パレスタウンではないゾーンだという事を示すID
#define PALACE_TOWN_TBLNO_NULL    (0xff)

///Intrude_CheckSameZoneID関数の戻り値
typedef enum{
  CHECKSAME_SAME,           ///<同じゾーンにいる
  CHECKSAME_SAME_REVERSE,   ///<表裏のゾーンにいる
  CHECKSAME_NOT,            ///<違うゾーンにいる
}CHECKSAME;

enum{
  PALACE_TOWN_SIDE_FRONT,       ///<表フィールド
  PALACE_TOWN_SIDE_REVERSE,     ///<裏フィールド
};

///WFBCの街番号
#define PALACE_TOWN_WFBC          (7)
///PalaceTownDataのデータ数
#define PALACE_TOWN_DATA_MAX      (8)
///PalaceTownDataのパレス島を指定
#define PALACE_TOWN_DATA_PALACE   (PALACE_TOWN_DATA_MAX)
///PalaceTownDataの無効値
#define PALACE_TOWN_DATA_NULL     (0xff)

///パレスの裏フィールドへワープする時のワープ先座標のパターン数
#define PALACE_WARP_POS_PATERN    (4)

enum{
  DISGUISE_NO_NULL = 0,         ///<変装無し
  DISGUISE_NO_NORMAL = 0xffff,  ///<パレス標準の変装姿
};

//==============================================================================
//  構造体定義
//==============================================================================
///パレスの街データ構造体
typedef struct{
  u16 front_zone_id;        ///<表フィールドのゾーンID
  u16 reverse_zone_id;      ///<裏フィールドのゾーンID
  u8 subscreen_x;           ///<サブスクリーンの街の位置X
  u8 subscreen_y;           ///<サブスクリーンの街の位置Y
//  u8 padding[2];
  struct{
    u16 gx;
    u16 gz;
    u16 y;
  }warp_pos[PALACE_WARP_POS_PATERN];
}PALACE_TOWN_DATA;

///パレスの街データ、検索結果
typedef struct{
  u16 zone_id;
  u8 front_reverse;         ///<表、裏、どちらにいるか(PALACE_TOWN_SIDE_???)
  u8 tblno;
}PALACE_TOWN_RESULT;

//==============================================================================
//  外部関数宣言
//==============================================================================
extern void Intrude_Main(INTRUDE_COMM_SYS_PTR intcomm);
extern FIELD_SUBSCREEN_MODE Intrude_SUBSCREEN_Watch(GAME_COMM_SYS_PTR game_comm, FIELD_SUBSCREEN_WORK *subscreen);
extern BOOL Intrude_SearchPalaceTown(ZONEID zone_id, PALACE_TOWN_RESULT *result);
extern CHECKSAME Intrude_CheckSameZoneID(const PALACE_TOWN_RESULT *result_a, const PALACE_TOWN_RESULT *result_b);
extern void Intrude_PlayerPosRecalc(INTRUDE_COMM_SYS_PTR intcomm, fx32 mine_x);

//--------------------------------------------------------------
//  ワークアクセス関数
//--------------------------------------------------------------
extern void Intrude_SetActionStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_ACTION action);
extern void Intrude_SetSendProfileBuffer(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_SetSendStatus(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_SetProfile(
  INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_PROFILE *profile);
extern void Intrude_SetPlayerStatus(
  INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_STATUS *sta);
extern void Intrude_InitTalkWork(INTRUDE_COMM_SYS_PTR intcomm, int talk_netid);
extern BOOL Intrude_SetTalkReq(INTRUDE_COMM_SYS_PTR intcomm, int net_id);
extern void Intrude_SetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int net_id, INTRUDE_TALK_STATUS talk_status);
extern void Intrude_SetTalkCancel(INTRUDE_COMM_SYS_PTR intcomm, int net_id);
extern INTRUDE_TALK_STATUS Intrude_GetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm);
extern int Intrude_GetPalaceTownZoneID(int town_tblno);
extern void Intrude_GetPalaceTownRandPos(int town_tblno, VecFx32 *vec);
extern INTRUDE_COMM_SYS_PTR Intrude_Check_CommConnect(GAME_COMM_SYS_PTR game_comm);
extern u16 Intrude_GetNormalDisguiseObjCode(const MYSTATUS *myst);
extern u16 Intrude_GetObjCode(const INTRUDE_STATUS *sta, const MYSTATUS *myst);
extern BOOL Intrude_OtherPlayerExistence(void);
extern BOOL Intrude_ObjPauseCheck(GAMESYS_WORK *gsys);

//==============================================================================
//  データ
//==============================================================================
extern const PALACE_TOWN_DATA PalaceTownData[];
extern u32 DisguiseObjCodeTblMax;

