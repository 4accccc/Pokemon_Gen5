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
extern BOOL Intrude_SearchPalaceTown(ZONEID zone_id, PALACE_TOWN_RESULT *result);
extern CHECKSAME Intrude_CheckSameZoneID(const PALACE_TOWN_RESULT *result_a, const PALACE_TOWN_RESULT *result_b);
extern void Intrude_PlayerPosRecalc(INTRUDE_COMM_SYS_PTR intcomm, fx32 mine_x);
extern void Intrude_UpdatePlayerStatusAll(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_SetSendMyPositionStopFlag(INTRUDE_COMM_SYS_PTR intcomm, BOOL stop);

//--------------------------------------------------------------
//  ワークアクセス関数
//--------------------------------------------------------------
extern void Intrude_SetActionStatus(INTRUDE_COMM_SYS_PTR intcomm, INTRUDE_ACTION action);
extern void Intrude_SetSendProfileBuffer(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_SetSendStatus(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_InitTalkCertification(INTRUDE_COMM_SYS_PTR intcomm);
extern INTRUDE_TALK_CERTIFICATION * Intrude_GetTalkCertification(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_SetProfile(
  INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_PROFILE *profile);
extern void Intrude_SetPlayerStatus(
  INTRUDE_COMM_SYS_PTR intcomm, int net_id, const INTRUDE_STATUS *sta, BOOL first_status);
extern void Intrude_InitTalkWork(INTRUDE_COMM_SYS_PTR intcomm, int talk_netid);
extern void Intrude_TalkRandClose(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_SetTalkReq(INTRUDE_COMM_SYS_PTR intcomm, int net_id, u8 talk_rand);
extern void Intrude_SetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm, int net_id, INTRUDE_TALK_STATUS talk_status, u8 talk_rand);
extern void Intrude_SetTalkCancel(INTRUDE_COMM_SYS_PTR intcomm, int net_id, u8 talk_rand);
extern INTRUDE_TALK_STATUS Intrude_GetTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_ClearTalkAnswer(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_SetTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_ClearTalkedEventReserve(INTRUDE_COMM_SYS_PTR intcomm);
extern BOOL Intrude_CheckTalkedTo(INTRUDE_COMM_SYS_PTR intcomm, u32 *hit_netid);
extern void Intrude_GetNormalDisguiseObjCode(const MYSTATUS *myst, u16 *objcode, u8 *disguise_type, u8 *disguise_sex);
extern u16 Intrude_GetObjCode(const INTRUDE_STATUS *sta, const MYSTATUS *myst);
extern BOOL Intrude_OtherPlayerExistence(void);
extern void Intrude_MyMonolithStatusSet(GAMEDATA *gamedata, MONOLITH_STATUS *monost);
extern BOOL Intrude_CheckRecvOccupyResult(INTRUDE_COMM_SYS_PTR intcomm);
extern void Intrude_SetTimeOutStopFlag(INTRUDE_COMM_SYS_PTR intcomm, BOOL stop);
extern void Intrude_SetMinePalaceArea(INTRUDE_COMM_SYS_PTR intcomm, int palace_area);

//--------------------------------------------------------------
//  不正チェック
//--------------------------------------------------------------
extern BOOL Intrude_CheckNG_OBJID(u16 obj_id);
extern BOOL Intrude_CheckNG_Item(u16 item_no);

//==============================================================================
//  データ
//==============================================================================
extern const PALACE_TOWN_DATA PalaceTownData[];
extern u32 DisguiseObjCodeTblMax;

