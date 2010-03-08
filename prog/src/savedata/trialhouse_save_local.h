//==============================================================================
/**
 * @file    trialhouse_save.h
 * @brief   トライアルハウスセーブデータ
 * @author  saito
 */
//==============================================================================
#pragma once


#define TH_MONS_DATA_MAX  (4)
#define TH_TITLE_MSG_MAX (16+1)   ///< メッセージの数 battle_exmination.hのBATTLE_EXAMINATION_TITLE_MSG_MAXと同じサイズ

//==============================================================================
//  構造体定義
//==============================================================================
///セーブワーク
typedef struct TH_MONS_DATA_tag
{
  u16 MonsNo;
  u8 FormNo;
  u8 Sex;
}TH_MONS_DATA;      //4byte

typedef struct TH_SV_COMMON_WK_tag
{
  u8 Valid;         //0　or 1  1で有効データ
  u8 IsDouble;      //ダブルバトルデータか？    0:シングル　1；ダブル
  u16 Point;        //検定ポイント
  TH_MONS_DATA MonsData[TH_MONS_DATA_MAX];   //最大４匹
}TH_SV_COMMON_WK;   //4+4x4 = 20byte
 
struct _THSV_WORK
{
  TH_SV_COMMON_WK CommonData[2];      //添え字　0；ＲＯＭデータ　1：ダウンロードデータ
  u8 DownloadBits[16];                   //ダウンロード用ビット群
  STRCODE Name[TH_TITLE_MSG_MAX];                        //検定名　ダウンロード検定名
  u8 padding[2];
};    //20x2+16+34+2 = 92byte
