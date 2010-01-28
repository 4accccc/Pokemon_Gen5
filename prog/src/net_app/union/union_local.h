//==============================================================================
/**
 * @file    union_local.h
 * @brief   ユニオンルーム関連のシステム部分を扱うソースのみで使用される定義類
 * @author  matsuda
 * @date    2009.07.04(土)
 */
//==============================================================================
#pragma once

#include "net_app/union/union_main.h"
#include "union_types.h"
#include "field/field_msgbg.h"
#include "net_app/union/comm_player.h"
#include "net_app/union/colosseum_types.h"


//==============================================================================
//  定数定義
//==============================================================================
///一度に受信できる最大ビーコン数
#define UNION_RECEIVE_BEACON_MAX      (10)

///キャラクタ管理数(一度に出せる最大キャラクタ数)
#define UNION_CHARACTER_MAX           (UNION_RECEIVE_BEACON_MAX * UNION_CONNECT_PLAYER_NUM)

///巨大データ受信バッファサイズ
#define UNION_HUGE_RECEIVE_BUF_SIZE   (0x800)

///リスタートモード
typedef enum{
  UNION_RESTART_SHUTDOWN_RESTARTS,    ///<切断→再開
  UNION_RESTART_SHUTDOWN,             ///<切断のみ
  UNION_RESTART_RESTARTS,             ///<再開のみ
}UNION_RESTART;


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _UNION_SYSTEM{
  UNION_PARENT_WORK *uniparent;
  u8 huge_receive_buf[UNION_CONNECT_PLAYER_NUM][UNION_HUGE_RECEIVE_BUF_SIZE];
  
  WORDSET *wordset;
  GFL_MSGDATA *msgdata;
  FLDPLAINMSGWIN *fld_msgwin_stream;
  FLDMENUFUNC *fldmenu_func;
  FLDMENUFUNC *fldmenu_yesno_func;

  FLDPLAINMSGWIN *fldmsgwin;
  FLDMSGPRINT *fldmsgprint;
  STRBUF *strbuf_temp;
  STRBUF *strbuf_talk_expand;
  u8 fld_msgwin_stream_print_on;      ///<TRUE:stream出力中
  u8 send_beacon_update;              ///<TRUE:送信ビーコン更新リクエストがある
  u8 overlay_load;                    ///<TRUE:ユニオンルームOVERLAYがロードされている
  u8 minigame_entry_answer;           ///<乱入希望の返事(UNION_MINIGAME_ENTRY_ANSWER)
  u8 minigame_entry_req_answer_ok_bit;  ///<乱入希望の返事(OK) ビット管理
  u8 minigame_entry_req_answer_ng_bit;  ///<乱入希望の返事(NG) ビット管理
  u8 padding[2];
  
  UNION_MY_SITUATION my_situation;    ///<自分の状況
  UNION_BEACON my_beacon;             ///<自分の送信ビーコン
  UNION_BEACON_PC receive_beacon[UNION_RECEIVE_BEACON_MAX];  ///<受信ビーコン
  UNION_CHARACTER character[UNION_CHARACTER_MAX];   ///<キャラクタ管理バッファ
  UNION_CHAT_LOG chat_log;            ///<チャットログ管理データ
  UNION_ALLOC alloc;                  ///<ユニオン内で別途Allocされるワーク管理
  
  UNION_SUB_PROC subproc;             ///<サブPROC呼び出し制御
  void *parent_work;                  ///<サブPROC用に使用する一時的なワーク管理用
  
  COLOSSEUM_SYSTEM *colosseum_sys;    ///<コロシアムシステムワーク
  
  u8 comm_status;
  u8 restart_seq;                     ///<「切断→再開」処理シーケンスNo
  u8 restart_mode;                    ///< UNION_RESTART_???
  u8 player_pause;                    ///<自機のポーズ状況
  
  BOOL debug_wayout_walk;             ///<※check TRUE:出口座標から一度でも出た
  BOOL finish;                        ///<TRUE:ユニオン終了リクエスト
  BOOL finish_exe;                    ///<TRUE:ユニオン終了処理実行中
}UNION_SYSTEM;


