
#pragma once

#define CRC_LOADCHECK (1)
//---------------------------------------------------------------------------
/// セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define PT_SAVE_PAGE_MAX   (32)

#define PT_SAVE_SECTOR_SIZE  (0x1000)

//---------------------------------------------------------------------------
/// セーブ項目の定義
//---------------------------------------------------------------------------
typedef enum {
  //ノーマルデータグループ
  PT_GMDATA_ID_SYSTEM_DATA,
  PT_GMDATA_ID_PLAYER_DATA,
  PT_GMDATA_ID_TEMOTI_POKE,
  PT_GMDATA_ID_TEMOTI_ITEM,
  PT_GMDATA_ID_EVENT_WORK,
  PT_GMDATA_ID_POKETCH_DATA,
  PT_GMDATA_ID_SITUATION,
  PT_GMDATA_ID_ZUKANWORK,
  PT_GMDATA_ID_SODATEYA,
  PT_GMDATA_ID_FRIEND,
  PT_GMDATA_ID_MISC,
  PT_GMDATA_ID_FIELDOBJSV,
  PT_GMDATA_ID_UNDERGROUNDDATA,
  PT_GMDATA_ID_REGULATION,
  PT_GMDATA_ID_IMAGECLIPDATA,
  PT_GMDATA_ID_MAILDATA,
  PT_GMDATA_ID_PORUTODATA,
  PT_GMDATA_ID_RANDOMGROUP,
  PT_GMDATA_ID_FNOTE,
  PT_GMDATA_ID_TRCARD,
  PT_GMDATA_ID_RECORD,
  PT_GMDATA_ID_CUSTOM_BALL,
  PT_GMDATA_ID_PERAPVOICE,
  PT_GMDATA_ID_FRONTIER,
  PT_GMDATA_ID_SP_RIBBON,
  PT_GMDATA_ID_ENCOUNT,
  PT_GMDATA_ID_WORLDTRADEDATA,
  PT_GMDATA_ID_TVWORK,
  PT_GMDATA_ID_GUINNESS,
  PT_GMDATA_ID_WIFILIST,
  PT_GMDATA_ID_WIFIHISTORY,
  PT_GMDATA_ID_FUSHIGIDATA,
  PT_GMDATA_ID_POKEPARKDATA,
  PT_GMDATA_ID_CONTEST,
  PT_GMDATA_ID_PMS,
  PT_GMDATA_ID_EMAIL,   //PTのみ
  PT_GMDATA_ID_WFHIROBA,    //PTのみ

  PT_GMDATA_NORMAL_FOOTER,  //WBで追加。セーブのCRCとかカウンタとか入ってる
  //ボックスデータグループ
  PT_GMDATA_ID_BOXDATA,

  PT_GMDATA_BOX_FOOTER,   //WBで追加。ボックスのCRCとかカウンタとか入ってる

  PT_GMDATA_ID_MAX,
}PT_GMDATA_ID;
//=============================================================================
//=============================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MAGIC_NUMBER_PT (0x20060623)
#define PT_SECTOR_SIZE   (PT_SAVE_SECTOR_SIZE)
#define PT_SECTOR_MAX    (PT_SAVE_PAGE_MAX)

#define FIRST_MIRROR_START  (0)
#define SECOND_MIRROR_START (64)

#define MIRROR1ST (0)
#define MIRROR2ND (1)
#define MIRRORERROR (2)

#define HEAPID_SAVE_TEMP  (HEAPID_BASE_APP)

#define SEC_DATA_SIZE   PT_SECTOR_SIZE

#define PT_SAVE_SIZE (PT_SECTOR_SIZE * PT_SECTOR_MAX)


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef struct {
  u32 g_count;    ///<グローバルカウンタ（MYデータ、BOXデータ共有）
  u32 b_count;    ///<ブロック内カウンタ（MYデータとBOXデータとで独立）
  u32 size;     ///<データサイズ（フッタサイズ含む）
  u32 magic_number; ///<マジックナンバー
  u8 blk_id;      ///<対象のブロック指定ID
  u16 crc;      ///<データ全体のCRC値
}PT_SAVE_FOOTER;

//---------------------------------------------------------------------------
/**
 * @brief セーブワーク構造体
 *
 * 実際のセーブされる部分の構造
 */
//---------------------------------------------------------------------------
/*
typedef struct {
  u8 data[PT_SECTOR_SIZE * PT_SECTOR_MAX];  ///<実際のデータ保持領域
}SAVEWORK;
*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define PT_BOX_MAX_TRAY      (18)
#define PT_BOX_MAX_POS       (5*6)
#define PT_BOX_TRAYNAME_BUFSIZE  (20)  // 日本語８文字＋EOM。海外版用の余裕も見てこの程度。

typedef struct pt_box_data PT_BOX_DATA;
typedef struct
{
  u32 dummy[0x88/4];
}PT_POKEMON_PARAM;


struct pt_box_data
{
  u32         currentTrayNumber;
  PT_POKEMON_PARAM  ppp[PT_BOX_MAX_TRAY][PT_BOX_MAX_POS];
  STRCODE       trayName[PT_BOX_MAX_TRAY][PT_BOX_TRAYNAME_BUFSIZE];
  u8          wallPaper[PT_BOX_MAX_TRAY];
  u8          daisukiBitFlag;
};

