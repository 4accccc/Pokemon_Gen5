//==============================================================================
/**
 * @file    mission_types.h
 * @brief   ミッション関連の定義
 * @author  matsuda
 * @date    2009.10.27(火)
 */
//==============================================================================
#pragma once


//==============================================================================
//  構造体定義
//==============================================================================
///ミッション要求データ
typedef struct{
  u8 monolith_type;   ///<石版タイプ  MONOLITH_TYPE_???
  u8 padding[3];
}MISSION_REQ;

///ミッションデータ
typedef struct{
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 mission_no;              ///<ミッション番号(ミッションが無い場合はMISSION_NO_NULL)
  u8 accept_netid;            ///<ミッション受注者のNetID
  u8 target_netid;            ///<ミッション内容によってターゲットとなるプレイヤーのNetID
}MISSION_DATA;

///ミッション結果
typedef struct{
  MISSION_DATA mission_data;  ///<達成したミッションデータ
  u8 achieve_netid;           ///<達成者のNetID
  u8 padding[3];
}MISSION_RESULT;

///ミッションシステム構造体
typedef struct{
  MISSION_DATA data;          ///<実行しているミッション
  MISSION_RESULT result;      ///<ミッション結果
  u8 data_send_req;           ///<TRUE:ミッションデータの送信を行う
  u8 result_send_req;         ///<TRUE:ミッションデータの送信を行う
  u8 parent_data_recv;        ///<TRUE:親からミッションデータを受信
  u8 parent_result_recv;      ///<TRUE:親から結果を受信
  u8 padding;
}MISSION_SYSTEM;

