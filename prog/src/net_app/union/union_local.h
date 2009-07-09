//==============================================================================
/**
 * @file    union_local.h
 * @brief   ユニオンルーム関連のシステム部分を扱うソースのみで使用される定義類
 * @author  matsuda
 * @date    2009.07.04(土)
 */
//==============================================================================
#include "net_app/union/union_main.h"
#include "union_types.h"

#pragma once


//==============================================================================
//  定数定義
//==============================================================================
///一度に受信できる最大ビーコン数
#define UNION_RECEIVE_BEACON_MAX      (SCAN_PARENT_COUNT_MAX)


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _UNION_SYSTEM{
  UNION_PARENT_WORK *uniparent;
  
  UNION_MY_SITUATION my_situation;    ///<自分の状況
  UNION_BEACON my_beacon;             ///<自分の送信ビーコン
  UNION_BEACON_PC receive_beacon[UNION_RECEIVE_BEACON_MAX];  ///<受信ビーコン
  
  u8 comm_status;
}UNION_SYSTEM;


