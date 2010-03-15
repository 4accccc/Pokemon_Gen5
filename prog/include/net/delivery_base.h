//======================================================================
/**
 * @file	  delivery_base.h
 * @brief	  配信の初期化構造体と基本情報
 * @author	k.ohno
 * @data	  10/02/28
 */
//======================================================================

#pragma once

#include "net/network_define.h"

#define DELIVERY_BEACON_ONCE_NUM  (88)  //ビーコン一回の送信サイズ
#define DELIVERY_BEACON_MAX_NUM  (17)    //DELIVERY_BEACON_MAX_NUM*DELIVERY_BEACON_ONCE_NUMバイトの送信が可能

#define DELIVERY_IRC_SEND_DATA_MAX  (7)   //日、英、フランス、イタリア、ドイツ、スペイン、韓国の７つ分

//IRC送信サイズ
//赤外線は内部で分割送信
#define DELIVERY_IRC_MAX_NUM  (DELIVERY_BEACON_ONCE_NUM*DELIVERY_BEACON_MAX_NUM)


typedef struct {
  int NetDevID;   // //通信種類 network_define.h参照
  int datasize;   //データ全体サイズ
  u8* pData;     // データ
  u16 ConfusionID;   //混線しないためのID 送信側で必要
  HEAPID heapID;    //HEAP
} DELIVERY_BEACON_INIT;

typedef struct {
  int datasize;   //データ全体サイズ
  u8* pData;     // データ  受信バッファ 送信バッファ
  u32 LangCode;    //言語コード
} DELIVERY_IRC_DATA;

typedef struct {
  int NetDevID;   // //通信種類  network_define.h参照
  u16 ConfusionID;   //混線しないためのID 送信側で必要
  HEAPID heapID;    //HEAPD
  DELIVERY_IRC_DATA  data[DELIVERY_IRC_SEND_DATA_MAX];
  u32               dataNum;   //送信側は複数、受信側は１つだけ設定すること
} DELIVERY_IRC_INIT;



