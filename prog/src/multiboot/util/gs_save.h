
#define CRC_LOADCHECK (1)
//---------------------------------------------------------------------------
/// セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define GS_SAVE_PAGE_MAX   (35)

#define GS_SAVE_SECTOR_SIZE  (0x1000)
//---------------------------------------------------------------------------
/// セーブ項目の定義
//---------------------------------------------------------------------------
typedef enum {
  //ノーマルデータグループ
  GS_GMDATA_ID_SYSTEM_DATA,
  GS_GMDATA_ID_PLAYER_DATA,
  GS_GMDATA_ID_TEMOTI_POKE,
  GS_GMDATA_ID_TEMOTI_ITEM,
  GS_GMDATA_ID_EVENT_WORK,
  GS_GMDATA_ID_SITUATION,
  GS_GMDATA_ID_ZUKANWORK,
  GS_GMDATA_ID_SODATEYA,
  GS_GMDATA_ID_FRIEND,
  GS_GMDATA_ID_MISC,
  GS_GMDATA_ID_FIELDOBJSV,
  GS_GMDATA_ID_REGULATION,
  GS_GMDATA_ID_IMAGECLIPDATA,
  GS_GMDATA_ID_MAILDATA,
  GS_GMDATA_ID_RANDOMGROUP,
  GS_GMDATA_ID_TRCARD,
  GS_GMDATA_ID_RECORD,
  GS_GMDATA_ID_CUSTOM_BALL,
  GS_GMDATA_ID_PERAPVOICE,
  GS_GMDATA_ID_FRONTIER,
  GS_GMDATA_ID_SP_RIBBON,
  GS_GMDATA_ID_ENCOUNT,
  GS_GMDATA_ID_WORLDTRADEDATA,
  GS_GMDATA_ID_TVWORK,
  GS_GMDATA_ID_GUINNESS,
  GS_GMDATA_ID_WIFILIST,
  GS_GMDATA_ID_WIFIHISTORY,
  GS_GMDATA_ID_FUSHIGIDATA,
  GS_GMDATA_ID_POKEPARKDATA,
  GS_GMDATA_ID_PKTH_WPOKE_SAVE,
  GS_GMDATA_ID_PMS,
  GS_GMDATA_ID_EMAIL,
  GS_GMDATA_ID_WFHIROBA,
  GS_GMDATA_ID_PAIR_POKE,
  GS_GMDATA_ID_PGEAR_SAVE,
  GS_GMDATA_ID_SAFARI_SAVE,
  GS_GMDATA_ID_PICTURE_SAVE,
  GS_GMDATA_ID_POKETHLON_SAVE,
  GS_GMDATA_ID_BONGURI_SAVE,
  GS_GMDATA_ID_PHC_SAVE,
  GS_GMDATA_ID_TR_HOUSE_SAVE,
  GS_GMDATA_NORMAL_FOOTER,  //WBで追加。セーブのCRCとかカウンタとか入ってる
  GS_GMDATA_PADDING,        //WBで追加。ボックスの前に256アライメントやっているので
  
  //ボックスデータグループ
  GS_GMDATA_ID_BOXDATA,
  GS_GMDATA_BOX_FOOTER,     //WBで追加。ボックスのCRCとかカウンタとか入ってる

  GS_GMDATA_ID_MAX,
}GS_GMDATA_ID;
//=============================================================================
//=============================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MAGIC_NUMBER_GS (0x20060623)
#define GS_SECTOR_SIZE   (GS_SAVE_SECTOR_SIZE)
#define GS_SECTOR_MAX    (GS_SAVE_PAGE_MAX)

#define FIRST_MIRROR_START  (0)
#define SECOND_MIRROR_START (64)

#define MIRROR1ST (0)
#define MIRROR2ND (1)
#define MIRRORERROR (2)

#define HEAPID_SAVE_TEMP  (HEAPID_BASE_APP)

#define GS_SEC_DATA_SIZE   GS_SECTOR_SIZE

#define GS_SAVE_SIZE (GS_SECTOR_SIZE * GS_SECTOR_MAX)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef struct {
  u32 g_count;    ///<グローバルカウンタ（MYデータ、BOXデータ共有）
  u32 size;     ///<データサイズ（フッタサイズ含む）
  u32 magic_number; ///<マジックナンバー
  u8 blk_id;      ///<対象のブロック指定ID
  u16 crc;      ///<データ全体のCRC値
}GS_SAVE_FOOTER;

//---------------------------------------------------------------------------
/**
 * @brief セーブワーク構造体
 *
 * 実際のセーブされる部分の構造
 */
//---------------------------------------------------------------------------
/*
typedef struct {
  u8 data[GS_SECTOR_SIZE * GS_SECTOR_MAX];  ///<実際のデータ保持領域
}SAVEWORK;
*/
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define GS_BOX_MAX_TRAY      (18)
#define GS_BOX_MAX_POS       (5*6)
#define GS_BOX_TRAYNAME_BUFSIZE  (20)  // 日本語８文字＋EOM。海外版用の余裕も見てこの程度。

typedef struct
{
  u32 dummy[0x88/4];
}GS_POKEMON_PARAM;


//フラッシュのページをまたがないように1トレー分のデータを256バイトアライメントする
typedef struct
{
  GS_POKEMON_PARAM  ppp[GS_BOX_MAX_POS];
  u8 dummy[16];
}GS_BOX_TRAY_DATA;

typedef struct {
/// POKEMON_PASO_PARAM  ppp[BOX_MAX_TRAY][BOX_MAX_POS];
  GS_BOX_TRAY_DATA   btd[GS_BOX_MAX_TRAY];
  //ここより下のデータが２５６バイトアライメントされたところにマッピングされる
  u32         currentTrayNumber;                //4
  u32         UseBoxBits;                   //4
  STRCODE     trayName[GS_BOX_MAX_TRAY][GS_BOX_TRAYNAME_BUFSIZE]; //2*20*18 = 720
  u8          wallPaper[GS_BOX_MAX_TRAY];            //18
  u8          daisukiBitFlag;                 //1
  u8          dummy[17];    //アライメントダミー      //17
  
  //ポケモンデータが0x1000*18 バイト = 0x12000
  //ポケモンデータ以外で764バイト = 0x2fc
  //計0x122fcバイト　
  //これにＣＲＣ４バイトを追加すると、0x12300となり、256バイトアライメントされ、
  //フッター情報のみの書き込みの際に、フラッシュページをまたがなくなる
}GS_BOX_DATA;


//---------------------------------------------------------------------------
//  アイテム関係
//---------------------------------------------------------------------------
typedef struct
{
  u16 id;   // アイテム番号
  u16 no;   // 個数
}GS_MINEITEM;

#define GS_BAG_NORMAL_ITEM_MAX    ( 165 )   // 道具ポケット最大数
#define GS_BAG_EVENT_ITEM_MAX     ( 50 )    // 大切な物ポケット最大数
#define GS_BAG_WAZA_ITEM_MAX      ( 101 )   // 技マシンポケット最大数
#define GS_BAG_SEAL_ITEM_MAX      ( 12 )    // シールポケット最大数
#define GS_BAG_DRUG_ITEM_MAX      ( 40 )    // 薬ポケット最大数
#define GS_BAG_NUTS_ITEM_MAX      ( 64 )    // 木の実ポケット最大数
#define GS_BAG_BALL_ITEM_MAX      ( 24 )    // モンスターボールポケット最大数
#define GS_BAG_BATTLE_ITEM_MAX    ( 30 )    // 戦闘用アイテムポケット最大数

typedef struct {
  GS_MINEITEM MyNormalItem[ GS_BAG_NORMAL_ITEM_MAX ]; // 手持ちの普通の道具
  GS_MINEITEM MyEventItem[ GS_BAG_EVENT_ITEM_MAX ];   // 手持ちの大切な道具
  GS_MINEITEM MySkillItem[ GS_BAG_WAZA_ITEM_MAX ];    // 手持ちの技マシン
  GS_MINEITEM MySealItem[ GS_BAG_SEAL_ITEM_MAX ];   // 手持ちのシール(実際はメール
  GS_MINEITEM MyDrugItem[ GS_BAG_DRUG_ITEM_MAX ];   // 手持ちの薬
  GS_MINEITEM MyNutsItem[ GS_BAG_NUTS_ITEM_MAX ];   // 手持ちの木の実
  GS_MINEITEM MyBallItem[ GS_BAG_BALL_ITEM_MAX ];   // 手持ちのモンスターボール
  GS_MINEITEM MyBattleItem[ GS_BAG_BATTLE_ITEM_MAX ]; // 手持ちの戦闘用アイテム
  u16 cnv_button1;                // 便利ボタン1(Yボタン）
  u16 cnv_button2;                // 便利ボタン2
}GS_MYITEM;

