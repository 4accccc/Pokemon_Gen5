//======================================================================
/**
 * @file  mb_data_def.c 
 * @brief wbダウンロードプレイ定義類
 * @author  ariizumi
 * @data  09/11/20
 */
//======================================================================

#pragma once

#if 1 //デバッグ出力切り替え

#ifndef MULTI_BOOT_MAKE
#include "test/ariizumi/ari_debug.h"
#define MB_DATA_TPrintf(...) (void)((ARI_TPrintf(__VA_ARGS__)))
#else
#include "../../src/test/ariizumi/ari_debug.h"
#define MB_DATA_TPrintf(...) (void)((ARI_TPrintf(__VA_ARGS__)))
#endif //MULTI_BOOT_MAKE
#else

#define MB_DATA_TPrintf(...) (void)((OS_TPrintf(__VA_ARGS__)))
#endif//デバッグ出力切り替え


//本体に刺さっているカードの種類
typedef enum {
  CARD_TYPE_DP,   //ダイアモンド＆パール
  CARD_TYPE_PT,   //プラチナ
  CARD_TYPE_GS,   //ゴールド＆シルバー
  CARD_TYPE_DUMMY,  //MBテストダミー
  CARD_TYPE_INVALID,
}DLPLAY_CARD_TYPE;

enum DLPLAY_ERROR_STATE
{
  DES_NONE,
  DES_DISCONNECT,
  DES_MISS_LOAD_BACKUP,

  DES_MAX,
};

enum DLPLAY_DATA_SAVEPOS
{
  DDS_FIRST,  //1番データ
  DDS_SECOND, //2番データ
  DDS_ERROR,  //エラー
};

struct _MB_DATA_WORK
{
  int heapId;
  u8  mainSeq;
  u8  subSeq;
  u8  errorState;
  u8  selectBoxNumber_;
  BOOL  isFinishSaveFirst;
  BOOL  isFinishSaveSecond;
  BOOL  permitLastSaveFirst;
  BOOL  permitLastSaveSecond;
  BOOL  isFinishSaveAll_;

  s32 lockID_;  //カードロック用ID
  MATHCRC16Table  crcTable_;  //CRCチェック用テーブル
  
  BOOL  isDummyCard;
  DLPLAY_CARD_TYPE  cardType;     //本体に刺さっているカードの種類(デバッグ時に選択したROMがのる
  u8          boxSavePos;  //ボックスデータの新しい方
  u8          mainSavePos; //メインデータの新しい方

  u8  *pData;  //読み出したデータ
  u8  *pDataMirror;  //ミラー用データ
  u8  *pBoxData;    //読み込み対象のBOXデータアドレス(まだポケモンデータじゃない！
};

