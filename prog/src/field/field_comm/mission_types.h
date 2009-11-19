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
  u16 zone_id;                ///<ミッション起動に使用したミニモノリスがあったゾーンID
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 padding;
}MISSION_REQ;

///ミッションデータ
typedef struct{
  u8 monolith_type;           ///<石版タイプ  MONOLITH_TYPE_???
  u8 mission_no;              ///<ミッション番号(ミッションが無い場合はMISSION_NO_NULL)
  u8 accept_netid;            ///<ミッション受注者のNetID
  u8 target_netid;            ///<ミッション内容によってターゲットとなるプレイヤーのNetID
  
  u16 zone_id;                ///<ミッション起動に使用したミニモノリスがあったゾーンID
  u8 padding[2];
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
  u32 timer;                  ///<ミッション失敗までのタイマー
  u8 data_send_req;           ///<TRUE:ミッションデータの送信を行う
  u8 result_send_req;         ///<TRUE:ミッションデータの送信を行う
  u8 parent_data_recv;        ///<TRUE:親からミッションデータを受信
  u8 parent_result_recv;      ///<TRUE:親から結果を受信
  u8 padding;
}MISSION_SYSTEM;


//--------------------------------------------------------------
//  コンバータから出力されるデータ
//--------------------------------------------------------------
///ミッションデータ
typedef struct{
  u8 mission_no;                  ///<ミッション番号
  u8 level;                       ///<発生レベル
  u8 type;                        ///<ミッション系統
  u8 odds;                        ///<発生率
  u16 msg_id_contents;            ///<ミッション内容gmmのmsg_id
  u16 msg_id_contents_monolith;   ///<ミッション内容モノリスgmmのmsg_id
  u16 obj_id[4];      ///<変化OBJ ID
  u16 data[3];        ///<データ(ミッション系統毎に扱いが変化)
  u16 time;           ///<時間(秒)
  u16 reward[4];      ///<報酬 0(1位) ～ 3(4位)
  u8  confirm_type;   ///<ミッション確認タイプ
  u8 limit_run;       ///<制限走り　TRUE:走り禁止
  u8 limit_talk;      ///<制限話　TRUE:話禁止
  u8 padding;
}MISSION_CONV_DATA;
