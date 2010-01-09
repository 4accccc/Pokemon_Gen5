//==============================================================================
/**
 * @file    union_app_local.h
 * @brief   ユニオンルームでのミニゲーム制御系のローカルヘッダ
 * @author  matsuda
 * @date    2010.01.08(金)
 */
//==============================================================================
#pragma once

//==============================================================================
//  定数定義
//==============================================================================
///ミニゲーム最大参加人数
#define UNION_APP_MEMBER_MAX    (5)

//==============================================================================
//  構造体定義
//==============================================================================
///基本情報：全プレイヤーが参照するパラメータ
typedef struct _UNION_APP_BASIC_STATUS{
  u8 member_max;        ///<参加最大数
  u8 member_bit;        ///<ゲームに参加しているユーザーのNetID管理(bit単位)
  u8 padding[2];
}UNION_APP_BASIC_STATUS;

struct _UNION_APP_WORK{
  UNION_APP_BASIC_STATUS basic_status;  ///<基本情報
  
  //親機だけが所持するパラメータ
  u8 entry_reserve_bit; ///<乱入予約者のNetID(bit管理) 乱入コールバック発生直前にmember_bitに登録され、bitはOFFされます
  u8 send_entry_answer_ok_bit;  ///<乱入希望者への返事：OK(bit管理)
  u8 send_entry_answer_ng_bit;  ///<乱入希望者への返事：NG(bit管理)
  u8 recv_intrude_ready_bit;    ///<乱入準備完了者のNetID(bit管理)
  u8 recv_leave_bit;            ///<離脱者のNetID(bit管理)
  
  BOOL entry_block;             ///<TRUE:乱入禁止
  BOOL leave_block;             ///<TRUE:退出禁止
  BOOL send_leave_req;          ///<TRUE:退出許可リクエストが発生している
  UNION_APP_LEAVE recv_leave_req_result;   ///<退出許可リクエストの返事
  BOOL shutdown_req;            ///<TRUE:切断リクエスト
  BOOL shutdown_exe;            ///<TRUE:切断実行中
  BOOL shutdown;                ///<TRUE:切断した
  
  MYSTATUS *mystatus[UNION_APP_MEMBER_MAX];
  
  u8 recv_basic_status_req_bit;   ///<基本情報要求リクエスト発生(bit管理)
  u8 recv_mystatus_req_bit;       ///<MYSTATUS要求リクエスト発生(bit管理)
  u8 recv_mystatus_bit;           ///<MYSTATUS受信済み(bit管理)
  
  //アプリ側からセットしてもらうパラメータ
  UNION_APP_CALLBACK_ENTRY_FUNC entry_callback;   ///<乱入者発生時のコールバック
  UNION_APP_CALLBACK_LEAVE_FUNC leave_callback;   ///<退出者発生時のコールバック
  void *userwork;
};
