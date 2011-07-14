//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wb_save.h
 *	@brief  WBのセーブ解析
 *	@author	Toru=Nagihashi
 *	@date		2011.07.06
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

//=============================================================================
/**
 *
 */
//=============================================================================

extern void WB_SAVE_Init( void );
extern u32  WB_SAVE_GetBlockOffset( const u16 id );
extern u32  WB_SAVE_GetBlockSize( const u16 id );
extern void * WB_SAVE_GetBlockAddress( void *p_data_adrs, const u16 id );
extern BOOL WB_SAVE_CheckIllegal( const void *cp_data_adrs );

//---------------------------------------------------------------------------
/// セーブに使用しているセクタ数
//---------------------------------------------------------------------------
#define WB_SAVE_PAGE_MAX   (36)

#define WB_SAVE_SECTOR_SIZE  (0x1000)

///ノーマル領域で使用するセーブサイズ
#define WB_SAVE_SIZE  (WB_SAVE_SECTOR_SIZE * WB_SAVE_PAGE_MAX)

///ミラーリング領域で使用するセーブ開始アドレス
#define WB_MIRROR_SAVE_ADDRESS   (0x24000)

///マジックナンバー
#define WB_MAGIC_NUMBER  (0x31053527)

///セーブデータ識別ID
enum{
  WB_GMDATA_ID_BOXDATA,        //ボックスデータグループ
  WB_GMDATA_ID_BOXTRAY_01,     //ボックストレーデータ
  WB_GMDATA_ID_BOXTRAY_02,
  WB_GMDATA_ID_BOXTRAY_03,
  WB_GMDATA_ID_BOXTRAY_04,
  WB_GMDATA_ID_BOXTRAY_05,
  WB_GMDATA_ID_BOXTRAY_06,
  WB_GMDATA_ID_BOXTRAY_07,
  WB_GMDATA_ID_BOXTRAY_08,
  WB_GMDATA_ID_BOXTRAY_09,
  WB_GMDATA_ID_BOXTRAY_10,
  WB_GMDATA_ID_BOXTRAY_11,
  WB_GMDATA_ID_BOXTRAY_12,
  WB_GMDATA_ID_BOXTRAY_13,
  WB_GMDATA_ID_BOXTRAY_14,
  WB_GMDATA_ID_BOXTRAY_15,
  WB_GMDATA_ID_BOXTRAY_16,
  WB_GMDATA_ID_BOXTRAY_17,
  WB_GMDATA_ID_BOXTRAY_18,
  WB_GMDATA_ID_BOXTRAY_19,
  WB_GMDATA_ID_BOXTRAY_20,
  WB_GMDATA_ID_BOXTRAY_21,
  WB_GMDATA_ID_BOXTRAY_22,
  WB_GMDATA_ID_BOXTRAY_23,
  WB_GMDATA_ID_BOXTRAY_24,
  WB_GMDATA_ID_MYITEM,           // どうぐ
  WB_GMDATA_ID_MYPOKE,           // てもちポケモン
  WB_GMDATA_ID_PLAYER_DATA,      // プレイヤー関連セーブデータ
  WB_GMDATA_ID_SITUATION,        // フィールド状況データ
  WB_GMDATA_ID_WIFIHISTORY,      // Wi-Fi遊んだ履歴
  WB_GMDATA_ID_WIFILIST,         // ともだちコード
  WB_GMDATA_ID_WIFI_NEGOTIATION, // Wi-Fiネゴシエーション用交換した人のPID
  WB_GMDATA_ID_CGEAR,            // C-GEAR
  WB_GMDATA_ID_TRCARD,           // トレーナーカード(サイン
  WB_GMDATA_ID_MYSTERYDATA,      // 不思議な贈り物
  WB_GMDATA_ID_DREAMWORLD,       // PDW
  WB_GMDATA_ID_PERAPVOICE,       // ぺラップヴォイス
  WB_GMDATA_ID_SYSTEM_DATA,      // システムデータ
  WB_GMDATA_ID_RECORD,           // レコード(スコア
  WB_GMDATA_ID_PMS,              // 簡易文
  WB_GMDATA_ID_MAILDATA,         // メールデータ
  WB_GMDATA_ID_MMDL,             // 動作モデル
  WB_GMDATA_ID_MUSICAL,          // ミュージカル
  WB_GMDATA_ID_RANDOMMAP,        // ランダム生成マップ
  WB_GMDATA_ID_IRCCOMPATIBLE,    // 相性チェック
  WB_GMDATA_ID_EVENT_WORK,       // イベントワーク
  WB_GMDATA_ID_WORLDTRADEDATA,   // GTS
  WB_GMDATA_ID_REGULATION_DATA,  // レギュレーション
  WB_GMDATA_ID_GIMMICK_WORK,     // ギミック
  WB_GMDATA_ID_BATTLE_BOX,       // バトルボックス
  WB_GMDATA_ID_SODATEYA_WORK,    // 育て屋ワーク
  WB_GMDATA_ID_ROCKPOS,          // かいりき岩
  WB_GMDATA_ID_MISC,             // 未分類(小さいデータの集まり）
  WB_GMDATA_ID_INTRUDE,          // 侵入
  WB_GMDATA_ID_SHORTCUT,         // Yボタン登録
  WB_GMDATA_ID_ZUKAN,            // ポケモン図鑑
  WB_GMDATA_ID_ENCOUNT,          // エンカウント関連セーブデータ
  WB_GMDATA_ID_BSUBWAY_PLAYDATA, // バトルサブウェイ　プレイデータ
  WB_GMDATA_ID_BSUBWAY_SCOREDATA,// バトルサブウェイ　スコアデータ
  WB_GMDATA_ID_BSUBWAY_WIFIDATA, // バトルサブウェイ WiFiデータ
  WB_GMDATA_ID_BATTLEMATCH,      // バトル大会
  WB_GMDATA_ID_SYMBOL,           // シンボルエンカウント
  WB_GMDATA_ID_MYPMS,            // 主人公の挨拶文データ
  WB_GMDATA_ID_QUESTIONNAIRE,    // すれ違いアンケート
  WB_GMDATA_ID_UNSV,             // 国連
  WB_GMDATA_ID_THSV,             // トライアルハウスランキング表示用
  WB_GMDATA_ID_GAMECLEAR_POKE,   // ゲームクリアてもちポケモンデータ
  WB_GMDATA_ID_TRPOKE_AFTER,     // 交換ポケモンのその後
  WB_GMDATA_ID_ETC,              // 未分類ワークその２
  WB_GMDATA_ID_MAX,
  WB_GMDATA_ID_CRC = WB_GMDATA_ID_MAX,  //SWANで追加：各ブロックのCRCテーブル
  WB_GMDATA_ID_FOOTER,              // SWANで追加：最後尾フッターデータが存在する
  WB_GMDATA_ID_END,
};

//--------------------------------------------------------------
/**
 * @brief   ブロックの配列全てを管理するフッター
 */
//--------------------------------------------------------------
typedef struct {
	u32 g_count;		///<グローバルカウンタ　
	u32 size;			///<データサイズ（フッタサイズ含む）
	u32 magic_number;	///<マジックナンバー
	u16 padding;		///<パディング
	u16 save_block_crc_tbl_crc;	///セーブブロックのCRC管理ワークのCRC
}WB_SAVE_FOOTER;

//--------------------------------------------------------------
/**
 * @brief   ブロック毎に持つフッター
 */
//--------------------------------------------------------------
typedef struct {
	u16 block_count;	///<ブロック毎のセーブカウンタ
	u16 crc;			///<ブロックのCRC値
}WB_SAVE_BLOCK_FOOTER;

//=============================================================================
/**
 *  各セーブワーク関連
 */
//=============================================================================
//----------------------------------------------------------
/**
 * @brief 自分状態データ型定義
 */
//----------------------------------------------------------
typedef struct {
  STRCODE name[7 + 1];    // 16
  u32 id;               ///< 20 PlayerID
  u32 profileID;        ///< 24 GameSyncID
  u8 nation;            ///< 国  25
  u8 area;              ///< 地域 26
  u8 region_code;       ///< PM_LANG   27
  u8 rom_code;          ///< PM_VERSION   28
  u8 trainer_view;      ///< ユニオンルーム内での見た目フラグ29
  u8 sex;               ///< 性別 30
  u8 dummy2;            // 31
  u8 dummy3;            // 32
}WB_MYSTATUS;

//----------------------------------------------------------
/**
 * @brief イベントワーク構造体
 */
//----------------------------------------------------------
#include "wb_flag_define.h"
#include "wb_work_define.h"
enum 
{
  //  全セーブフラグ占有ワークサイズ
  WB_EVENT_FLAG_AREA_MAX =  ( MATH_ROUNDUP(FLAGS_TOTAL_MAX, 8) / 8),

  //  全フラグエリア占有フラグサイズ
  WB_EVENT_WORK_AREA_MAX = WK_AREA_END - WORKS_TOTAL_MIN,

};
typedef struct {
	u16 work[WB_EVENT_WORK_AREA_MAX];	//ワーク
	u8	flag[WB_EVENT_FLAG_AREA_MAX];	//フラグ
  u8  time_request;
}WB_EVENTWORK;

//----------------------------------------------------------
/**
 * @brief ミュージカル
 */
//----------------------------------------------------------
typedef struct {
  //ミュージカルショット
  struct {
    u32 bgNo   :5;  //背景番号
    u32 spotBit:4;  //スポットライト対象(bit)(トップだったポケモン
    u32 year   :7;  //年
    u32 month  :5;  //月  これが０だったら無効データとみなす
    u32 day    :6;  //日
    u32 player :2;  //自分の番号
    u32 musVer :3;  //ミュージカルバージョン

    struct
    {
      u16  monsno; //ポケモン番号
      u16  sex :2; //性別
      u16  rare:1; //レアフラグ
      u16  form:5; //フォルム番号
      u16  pad :8;
      u32  perRand; //個性乱数  +8byte

      STRCODE trainerName[8];  //トレーナー名              +16byte

      struct
      {
        u16 itemNo;   //グッズ番号
        s16 angle;    //角度
        u8  equipPos; //装備箇所
        u8  pad[3];
      }equip[8];  //装備グッズデータ(８個 +64byte;

    }shotPoke[4];

    STRCODE title[37];  //ミュージカルタイトル(日本18・海外36＋EOM  +74byte
    u8 pmVersion;
    u8 pmLang;
  } musicalShotData;
	//前回装備
  struct
  {
    struct
    {
      u8	pos;					//装備箇所(MUS_POKE_EQUIP_POS）
      u16	itemNo;	      //アイテムの種類
      s16	angle;	      //アイテムの傾き
    }equipData[8];
  }befEquip;
	
  //ファン状態
  struct
  {
    u8 type;          //種類(MUSICAL_CONDITION_TYPE)
    u8 giftType;   //プレゼント種類
    u16 giftValue; //アイテム番号・グッズ番号
  }fanState[10];
  
  //配信演目名
  STRCODE distTitle[36+1];  //37
  
  //参加回数
  u16 entryNum; 

  //トップ回数
  u16 topNum;
  //累計得点
  u16 sumPoint;

  //アイテム所持bit
  u8 itemBit[256/8];
  //Newアイテムbit
  u8 itemNewBit[256/8];
  
  //前回コンディション
  u8 befCondition[4];
  //前回評価点
  u8 befPoint;

  //選択している演目番号
  u8 programNumber;
  //配信データのチェック
  u8 enableDistData;
  //デモを見たか？
  u8 isLookDemo;
  u8 padding[1];
}WB_MUSICAL_SAVE;
