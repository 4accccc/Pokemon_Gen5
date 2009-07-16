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



//==============================================================================
//  定数定義
//==============================================================================
///一度に接続できる最大人数
#define UNION_CONNECT_PLAYER_NUM      (5) //レコードコーナーが最大5人なので

///一度に受信できる最大ビーコン数
#define UNION_RECEIVE_BEACON_MAX      (SCAN_PARENT_COUNT_MAX)

///巨大データ受信バッファサイズ
#define UNION_HUGE_RECEIVE_BUF_SIZE   (0x200)

//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _UNION_SYSTEM{
  UNION_PARENT_WORK *uniparent;
  u8 huge_receive_buf[UNION_CONNECT_PLAYER_NUM][UNION_HUGE_RECEIVE_BUF_SIZE];
  
  GFL_MSGDATA *msgdata;
  FLDMSGWIN_STREAM *fld_msgwin_stream;
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;
  FLDMENUFUNC *fldmenu_func;
  FLDMENUFUNC *fldmenu_yesno_func;
  
  UNION_MY_SITUATION my_situation;    ///<自分の状況
  UNION_BEACON my_beacon;             ///<自分の送信ビーコン
  UNION_BEACON_PC receive_beacon[UNION_RECEIVE_BEACON_MAX];  ///<受信ビーコン
  
  UNION_SUB_PROC subproc;             ///<サブPROC呼び出し制御
  
  u8 comm_status;
}UNION_SYSTEM;


