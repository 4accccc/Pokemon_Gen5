//============================================================================================
/**
 * @file	  mystery_data.h
 * @date	  2009.12.04
 * @author	k.ohno
 * @brief	  ふしぎ通信用セーブデータ用ヘッダ
 */
//============================================================================================

#pragma once

#include "include/buflen.h"
#include "savedata/save_control.h"
#include "poke_tool/poke_tool.h"
#include "savedata/regulation.h"

//============================================================================================
//
//			定義
//
//============================================================================================

//------------------------------------------------------------------
///		贈り物データの個数
//------------------------------------------------------------------
#define	GIFT_DATA_MAX		12	//カード12

#define MYSTERYGIFT_NUTSET_MAX 4 //木の実セットの数

//------------------------------------------------------------------
///		カード関係のサイズ定義
//------------------------------------------------------------------
#define GIFT_DATA_CARD_TITLE_MAX	36    //配布タイトル

#define GIFT_DATA_CARD_TEXT_MAX		504   //説明テキスト


//------------------------------------------------------------------
///		贈り物データ構造体
//------------------------------------------------------------------
#define MYSTERYGIFT_TYPE_NONE		0	// 何も無い
#define MYSTERYGIFT_TYPE_POKEMON	1	// ポケモン
#define MYSTERYGIFT_TYPE_ITEM		2	// どうぐ
#define MYSTERYGIFT_TYPE_RULE		3	// ルール
#define MYSTERYGIFT_TYPE_NUTSET		4	// 木の実セット
#define MYSTERYGIFT_TYPE_MAX	5	// 総数

#define MYSTERYGIFT_TYPE_CLEAR		255	// ふしぎ領域の強制クリア


//------------------------------------------------------------------
///		受信日時
//------------------------------------------------------------------
#define MYSTERYGIFT_DATE_MASK_YEAR    0xFFFF
#define MYSTERYGIFT_DATE_MASK_MONTH   0xFF
#define MYSTERYGIFT_DATE_MASK_DAY     0xFF
#define MYSTERYGIFT_DATE_SHIFT_YEAR   16
#define MYSTERYGIFT_DATE_SHIFT_MONTH  8
#define MYSTERYGIFT_DATE_SHIFT_DAY    0


// サイズ固定用構造体
typedef struct {
  u8 data[256];
} GIFT_PRESENT_ALL;

// ポケモン
typedef struct {
	u32	id_no;			// 親のID
  u32 version;    // ROMバージョン
  u32 rnd;        // 固体乱数
  u16 ribbon_no;        //リボンビット 16本
  u16 get_ball;  //捕まえたボール
  u16 item;      //もちもの
  u16 waza1;     //技1
  u16 waza2;     //技2
  u16 waza3;     //技3
  u16 waza4;     //技4
  u16 mons_no;   //モンスターNo
  u8 form_no;    //フォルムNo
  u8 country_code;  //国コード
	STRCODE	nickname[MONS_NAME_SIZE+EOM_SIZE];	//16h	ニックネーム(MONS_NAME_SIZE=10)+(EOM_SIZE=1)=11  最初がEOMならデフォルト
  u8 seikaku;          //性格
  u8 sex;              //性別
  u8 speabino;         //とくせい
  u8 rare;             //レアにするかどうか 0=ランダムレアない  1=ランダム 2=レア
  u8 get_place;        //捕まえた場所
  u8 birth_place;      //生まれた場所
  u8 get_level;        //捕まえたLEVEL
  u8 style;                    //かっこよさ
  u8 beautiful;                //うつくしさ
  u8 cute;                     //かわいさ
  u8 clever;                   //かしこさ
  u8 strong;                   //たくましさ
  u8 fur;                      //毛艶
  u8 hp_rnd;               //HP乱数
  u8 pow_rnd;              //攻撃力乱数
  u8 def_rnd;              //防御力乱数
  u8 agi_rnd;              //素早さ乱数
  u8 spepow_rnd;             //特攻乱数
  u8 spedef_rnd;             //特防乱数
  STRCODE	oyaname[PERSON_NAME_SIZE+EOM_SIZE];	//10h	親の名前(PERSON_NAME_SIZE=7)+(EOM_SIZE_=1)=8*2(STRCODE=u16)
  u8 oyasex;      //親の性別 0 1 2 
  u8 level;     //ポケモンレベル
  u8 egg;       //タマゴかどうか TRUE＝たまご
  u8 dummy;
} GIFT_PRESENT_POKEMON;


// どうぐ
typedef struct {
  int itemNo;        ///どうぐ番号
  int movieflag;    ///映画配信かどうか
} GIFT_PRESENT_ITEM;

// ルール(レギュレーション)
typedef struct {
  REGULATION regu_data;  ///デジタル選手証
} GIFT_PRESENT_RULE;

// 木の実セット  PDW用
typedef struct {
  int itemNo[MYSTERYGIFT_NUTSET_MAX];
  int itemMax[MYSTERYGIFT_NUTSET_MAX];
} GIFT_PRESENT_NUTSET;

// ポケモンエンカウント PDW用
typedef struct {
  int pokemon;
} GIFT_PRESENT_ENCOUNT;


// ふしぎなおくりもの強制クリア
typedef struct {
  int dummy;
} GIFT_PRESENT_REMOVE;

typedef union {
  GIFT_PRESENT_ALL 		all;
  GIFT_PRESENT_POKEMON		pokemon;
  GIFT_PRESENT_ITEM		item;
  GIFT_PRESENT_RULE		rule;
  GIFT_PRESENT_NUTSET nutset;
  GIFT_PRESENT_ENCOUNT encount;
  GIFT_PRESENT_REMOVE		remove;
} GIFT_PRESENT;



// 保存するデータ
typedef struct{
  GIFT_PRESENT data;
  STRCODE event_name[GIFT_DATA_CARD_TITLE_MAX+EOM_SIZE];	// イベントタイトル
  s32 recv_date;				// 受信した時間
  u16 event_id;					// イベントＩＤ(最大2048件まで)
  u8 card_message;      //  カードのメッセージタイプ
  u8 gift_type;         // 送られてくるタイプ
  u8 only_one_flag:1;	  // １度だけ受信フラグ(0..何度でも受信可能 1..１回のみ)
  u8 have:1;				    // データを受け取った場合 1
  u8 padding:6;
  u8 padding2;
  u32 dummy[4];            //予備データ16byte
} GIFT_PACK_DATA;


// サーバから送られるデータ
typedef struct{
  GIFT_PACK_DATA data;
  u32 version;					// 対象バージョン(０の場合は制限無しで配布)
  STRCODE event_text[GIFT_DATA_CARD_TEXT_MAX+EOM_SIZE];	// 説明テキスト
} DOWNLOAD_GIFT_DATA;




//------------------------------------------------------------------
/**
 * @brief	ふしぎセーブデータへの不完全型定義
 */
//------------------------------------------------------------------
typedef struct MYSTERY_DATA MYSTERY_DATA;


//============================================================================================
//
//			外部参照
//
//============================================================================================
//------------------------------------------------------------------
//データサイズ取得
//------------------------------------------------------------------
extern int MYSTERYDATA_GetWorkSize(void);

//------------------------------------------------------------------
//初期化処理
//------------------------------------------------------------------
extern void MYSTERYDATA_Init(MYSTERY_DATA * fd);

//------------------------------------------------------------------
/**
 * @brief	ふしぎデータへのポインタ取得
 */
//------------------------------------------------------------------
extern MYSTERY_DATA * SaveData_GetMysteryData(SAVE_CONTROL_WORK * sv);


//------------------------------------------------------------------
/// カードデータの取得
//------------------------------------------------------------------
extern GIFT_PACK_DATA *MYSTERYDATA_GetCardData(MYSTERY_DATA *fd, u32 cardindex);
//------------------------------------------------------------------
/// カードデータをセーブデータ登録
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_SetCardData(MYSTERY_DATA *fd, const void *p);
//------------------------------------------------------------------
/// カードデータを抹消する
//------------------------------------------------------------------
extern void MYSTERYDATA_RemoveCardData(MYSTERY_DATA *fd, u32 cardindex);
//------------------------------------------------------------------
/// カードデータがセーブできるかチェック
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_CheckCardDataSpace(MYSTERY_DATA *fd);
//------------------------------------------------------------------
/// カードデータが存在するか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsExistsCard(const MYSTERY_DATA * fd, u32 cardindex);
//------------------------------------------------------------------
/// セーブデータ内にカードデータが存在するか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsExistsCardAll(const MYSTERY_DATA *fd);
//------------------------------------------------------------------
//	 指定のカードからデータを受け取り済みか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsHavePresent(const MYSTERY_DATA * fd, u32 cardindex);
//------------------------------------------------------------------
/// 指定のイベントはすでにもらったか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsEventRecvFlag(MYSTERY_DATA * fd, u32 eventno);
//------------------------------------------------------------------
/// 指定のイベントもらったよフラグを立てる
//------------------------------------------------------------------
extern void MYSTERYDATA_SetEventRecvFlag(MYSTERY_DATA * fd, u32 eventno);


//------------------------------------------------------------------
/// 受信日時　年月日受け取り
//------------------------------------------------------------------
inline s32 MYSTERYDATA_GetYear( s32 recv_date )
{ 
  return (recv_date >> MYSTERYGIFT_DATE_SHIFT_YEAR) & MYSTERYGIFT_DATE_MASK_YEAR;
}
inline s32 MYSTERYDATA_GetMonth( s32 recv_date )
{ 
  return (recv_date >> MYSTERYGIFT_DATE_SHIFT_MONTH) & MYSTERYGIFT_DATE_MASK_MONTH;
}
inline s32 MYSTERYDATA_GetDay( s32 recv_date )
{ 
  return (recv_date >> MYSTERYGIFT_DATE_SHIFT_DAY) & MYSTERYGIFT_DATE_MASK_DAY;
}

//------------------------------------------------------------------
//	 指定のカードを入れ替える
//------------------------------------------------------------------
extern void MYSTERYDATA_SwapCard( MYSTERY_DATA * fd, u32 cardindex1, u32 cardindex2 );

//------------------------------------------------------------------
/// 不正修正関数  (現在名前のEOMチェックとEVENT_IDの範囲チェックのみです)
//------------------------------------------------------------------
extern u32 MYSTERYDATA_ModifyDownloadData( DOWNLOAD_GIFT_DATA *p_data );
extern u32 MYSTERYDATA_ModifyGiftData( GIFT_PACK_DATA *p_data );

//------------------------------------------------------------------
/// デバッグ用にポケモンデータをセットする関数
//------------------------------------------------------------------
extern void MYSTERYDATA_DebugSetPokemon(void);

