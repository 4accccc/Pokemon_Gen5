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

//ROMコード
#define MB_ROMCODE_DIAMOND  (0x4A414441)
#define MB_ROMCODE_PEARL    (0x4A415041)
#define MB_ROMCODE_PLATINUM (0x4A555043)
#define MB_ROMCODE_H_GOLD   (0x4A4B5049)
#define MB_ROMCODE_S_SILVER (0x4A475049)

//本体に刺さっているカードの種類
typedef enum {
  CARD_TYPE_DP,   //ダイアモンド＆パール
  CARD_TYPE_PT,   //プラチナ
  CARD_TYPE_GS,   //ゴールド＆シルバー
#if PM_DEBUG
  CARD_TYPE_DUMMY,  //MBテストダミー
#endif
  CARD_TYPE_INVALID,
}DLPLAY_CARD_TYPE;

enum DLPLAY_ERROR_STATE
{
  DES_NONE,
  DES_DISCONNECT,
  DES_MISS_LOAD_BACKUP,
  DES_ANOTHER_COUNTRY,

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
  u8          boxLoadPos;  //ボックスデータの新しい方
  u8          mainLoadPos; //メインデータの新しい方
  u8          boxSavePos;  //セーブするほう
  u8          mainSavePos; //セーブするほう

  u8  *pData;  //読み出したデータ
  u8  *pDataMirror;  //ミラー用データ
  void *pDataCrcCheck;  //CRCチェック用データ
  
  u16 cardCrcTable[4];  //現在のデータのCRC
  u16 loadCrcTable[4];  //読み込んだデータのCRC
  
  u8  *pBoxData;    //読み込み対象のBOXデータアドレス(まだポケモンデータじゃない！
  u8  *pItemData;   //読み込み対象のアイテムデータアドレス(まだアイテムデータじゃない！
  u8  *pMysteryData;//読み込み対象の不思議な贈り物データアドレス
  u8  *pMyStatus;   //読み込み対象のマイステータすデータアドレス(初回国コードのみ
};

