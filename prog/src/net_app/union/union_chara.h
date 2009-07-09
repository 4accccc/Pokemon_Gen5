//==============================================================================
/**
 * @file    union_chara.h
 * @brief   ユニオンルームでの人物関連のヘッダ
 * @author  matsuda
 * @date    2009.07.06(月)
 */
//==============================================================================
#pragma once

//==============================================================================
//  定数定義
//==============================================================================
///ビーコンPC：イベントステータス
enum{
  BPC_EVENT_STATUS_NULL,      ///<何もしていない
  BPC_EVENT_STATUS_ENTER,     ///<ユニオンルームへ進入
  BPC_EVENT_STATUS_LEAVE,     ///<ユニオンルームから退出
  
  BPC_EVENT_STATUS_MAX,
};

//==============================================================================
//  外部関数宣言
//==============================================================================
extern MMDL * UNION_CHAR_AddOBJ(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata, u8 trainer_view, u16 char_index);
extern void UNION_CHAR_Update(UNION_SYSTEM_PTR unisys, GAMEDATA *gdata);
extern BOOL UNION_CHAR_EventReq(UNION_BEACON_PC *bpc, int event_status);

