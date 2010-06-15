//============================================================================================
/**
 * @file	  mystery_data.h
 * @date	  2009.12.04
 * @author	k.ohno
 * @brief	  ふしぎ通信用セーブデータ用ヘッダ
 *
 * @note    2010412 不思議な贈り物がゲーム開始前に保存できるようになりました
 *                  ゲーム開始前は管理外のセーブエリアを使用し、３件まで保存可能です。
 *                  ゲーム開始後は内部セーブを使用し１２件まで保存可能です。
 *                  上記を共通して使うため、以下の関数はセーブデータから参照されるものをのぞき、
 *                  すべて、内部でゲーム開始前（＝内部セーブが作られていない）か、ゲーム開始後かを
 *                  判別し、自動的にそちらのデータへアクセスするようになっています。
 */
//============================================================================================

#pragma once

#include "include/buflen.h"
#include "savedata/save_control.h"
#include "poke_tool/poke_tool.h"
#include "savedata/regulation.h"

#include "gamesystem/game_data.h"

//============================================================================================
//
//			定義
//
//============================================================================================

//------------------------------------------------------------------
///		贈り物データの個数
//------------------------------------------------------------------
#define	GIFT_DATA_MAX		(12)	///※この個数はあくまで内部セーブ版の最大数。
                              ///内部セーブと管理外セーブを同時に使用する箇所ではMYSTERY_DATA_GetCardMaxを使用すること
#define MYSTERY_DATA_MAX_EVENT		(2048)  //イベント最大

//-------------------------------------
///	特殊イベント
//=====================================
#define MYSTERY_DATA_EVENT_LIBERTY	(2046)  //早期購入特典リバティチケット

//------------------------------------------------------------------
///		カード関係のサイズ定義
//------------------------------------------------------------------
#define GIFT_DATA_CARD_TITLE_MAX	(36)    //配布タイトル

#define GIFT_DATA_CARD_TEXT_MAX		(252)   //説明テキスト


//------------------------------------------------------------------
///		贈り物データ構造体
//------------------------------------------------------------------
#define MYSTERYGIFT_TYPE_NONE		(0)	// 何も無い
#define MYSTERYGIFT_TYPE_POKEMON	(1)	// ポケモン
#define MYSTERYGIFT_TYPE_ITEM		(2)	// どうぐ
#define MYSTERYGIFT_TYPE_POWER		(3)	// パワー
#define MYSTERYGIFT_TYPE_MAX	(4)	// 総数

#define MYSTERYGIFT_TYPE_CLEAR		(255)	// ふしぎ領域の強制クリア


//------------------------------------------------------------------
///		受信日時
//------------------------------------------------------------------
#define MYSTERYGIFT_DATE_MASK_YEAR    0xFFFF
#define MYSTERYGIFT_DATE_MASK_MONTH   0xFF
#define MYSTERYGIFT_DATE_MASK_DAY     0xFF
#define MYSTERYGIFT_DATE_SHIFT_YEAR   16
#define MYSTERYGIFT_DATE_SHIFT_MONTH  8
#define MYSTERYGIFT_DATE_SHIFT_DAY    0

//-------------------------------------
///	データのタイプ
//=====================================
typedef enum {
  MYSTERY_DATA_TYPE_OUTSIDE,  //管理外セーブ
  MYSTERY_DATA_TYPE_ORIGINAL, //通常のセーブ
} MYSTERY_DATA_TYPE;



// サイズ固定用構造体
typedef struct {
  u8 data[256];
} GIFT_PRESENT_ALL;

// ポケモン
typedef struct {
	u32	id_no;			// 親のID 0ならばプレイヤーのIDを入れる
  u32 version;    // ROMバージョン  0ならば自分のROMバージョンが入る
  u32 rnd;        // 固体乱数　0ならば性別、特性、レアのパラメータを使う。0以外ならばそれらを使わず個体乱数直指定
  u16 ribbon_no;        //リボンビット 16本
  u16 get_ball;  //捕まえたボール 0ならば指定しない
  u16 item;      //もちもの
  u16 waza1;     //技1
  u16 waza2;     //技2
  u16 waza3;     //技3
  u16 waza4;     //技4
  u16 mons_no;   //モンスターNo
  u8 form_no;    //フォルムNo
  u8 country_code;  //国コード 0ならばROMの言語コードを入れる
	STRCODE	nickname[MONS_NAME_SIZE+EOM_SIZE];	//16h	ニックネーム(MONS_NAME_SIZE=10)+(EOM_SIZE=1)=11  最初がEOMならデフォルト
  u8 seikaku;          //性格 0xFFでランダム
  u8 sex;              //性別 0xFFでランダム
  u8 speabino;         //とくせい 0=特性１,1=特性２,2=特性３,3=特性1or2ランダム,4=特性1or2or3ランダム
  u8 rare;             //レアにするかどうか 0=レアない  1=ランダム 2=レア
  u16 get_place;        //捕まえた場所
  u16 birth_place;      //生まれた場所
  u8 get_level;         //捕まえたLEVEL
  u8 style;                    //かっこよさ
  u8 beautiful;                //うつくしさ
  u8 cute;                     //かわいさ
  u8 clever;                   //かしこさ
  u8 strong;                   //たくましさ
  u8 fur;                      //毛艶
  u8 hp_rnd;               //HP乱数        0xFFでランダム 
  u8 pow_rnd;              //攻撃力乱数    0xFFでランダム 
  u8 def_rnd;              //防御力乱数    0xFFでランダム 
  u8 agi_rnd;              //素早さ乱数    0xFFでランダム 
  u8 spepow_rnd;             //特攻乱数    0xFFでランダム 
  u8 spedef_rnd;             //特防乱数    0xFFでランダム 
  STRCODE	oyaname[PERSON_NAME_SIZE+EOM_SIZE];	//10h	親の名前(PERSON_NAME_SIZE=7)+(EOM_SIZE_=1)=8*2(STRCODE=u16)　1文字目がEOMならばプレイヤーの名前をいれる
  u8 oyasex;      //親の性別 0は男性 1は女性 2 はプレイヤーの性別をいれる
  u8 level;     //ポケモンレベル　0ならばランダム
  u8 egg;       //タマゴかどうか TRUE＝たまご
  u8 speabino_flag;   //特性固定フラグ(未使用です)
} GIFT_PRESENT_POKEMON;


// どうぐ
typedef struct {
  int itemNo;        ///どうぐ番号
  int movieflag;    ///映画配信かどうか
} GIFT_PRESENT_ITEM;


// Ｇパワー
typedef struct {
  int type;        ///開放するパワータイプ一種類
} GIFT_PRESENT_POWER;



// ふしぎなおくりもの強制クリア
typedef struct {
  int dummy;
} GIFT_PRESENT_REMOVE;

typedef union {
  GIFT_PRESENT_POKEMON	pokemon;
  GIFT_PRESENT_ITEM		  item;
  GIFT_PRESENT_POWER		gpower;
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
  u16 crc;             //CRCデータ（現状、以下のDOWNLOAD_GIFT_DATAのものを使用しております）
  u16 encryption;             //暗号複合キー(現状未使用)
} GIFT_PACK_DATA;


// サーバから送られるデータ
typedef struct{
  GIFT_PACK_DATA data;
  u32 version;					// 対象バージョン(０の場合は制限無しで配布)
  STRCODE event_text[GIFT_DATA_CARD_TEXT_MAX+EOM_SIZE];	// 説明テキスト
  u8 movie_flag;         //映画配信かどうか
  u8 LangCode;          //言語コード
  u8 dummy[2];          //ダミー
  u16 crc;              //CRCデータ
} DOWNLOAD_GIFT_DATA;

//------------------------------------------------------------------
/**
 * @brief	ふしぎセーブデータへの不完全型定義
 */
//------------------------------------------------------------------
typedef struct _MYSTERY_DATA MYSTERY_DATA;


//============================================================================================
//
//			外部参照
//
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	ふしぎデータへのポインタ取得  …はなくなって、ALLOC＋FREE方式になりました
 *
 *        ふしぎデータは内部セーブ版と管理外セーブ版を内包しているため
 *        直接セーブデータへのアクセスを禁止するため、非公開
 *
 *        以下関数を使い、ALLOCとFREEをすること。
 *
 */
//------------------------------------------------------------------
//extern MYSTERY_DATA * SaveData_GetMysteryData(SAVE_CONTROL_WORK * sv);

typedef enum
{ 
  MYSTERYDATA_LOADTYPE_NORMAL,  //通常セーブを見に行く
  MYSTERYDATA_LOADTYPE_AUTO,    //通常セーブと管理外セーブの自動判別（ここは不思議なおくりもの画面でしか使わない）
  //管理外だけみに行くモードは不必要なので作成していない
}MYSTERYDATA_LOADTYPE;
extern MYSTERY_DATA * MYSTERY_DATA_Load( SAVE_CONTROL_WORK *p_sv, MYSTERYDATA_LOADTYPE type, HEAPID heapID );
extern void MYSTERY_DATA_UnLoad( MYSTERY_DATA *p_wk );

//------------------------------------------------------------------
/// データのシステム値取得
//------------------------------------------------------------------
extern u32 MYSTERY_DATA_GetCardMax( const MYSTERY_DATA *fd );
extern MYSTERY_DATA_TYPE MYSTERY_DATA_GetDataType( const MYSTERY_DATA *fd );

//------------------------------------------------------------------
/// カードデータの取得
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_GetCardData(MYSTERY_DATA *fd, u32 index,GIFT_PACK_DATA *pData);

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
extern BOOL MYSTERYDATA_IsExistsCard( MYSTERY_DATA * fd, u32 cardindex);
//------------------------------------------------------------------
/// セーブデータ内にカードデータが存在するか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsExistsCardAll( MYSTERY_DATA *fd);
//------------------------------------------------------------------
//	 指定のカードからデータを受け取り済みか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsHavePresent( MYSTERY_DATA * fd, u32 cardindex);
//------------------------------------------------------------------
//   指定のカードからデータを受け取ったことにする
//------------------------------------------------------------------
extern void MYSTERYDATA_SetHavePresent(MYSTERY_DATA * fd, u32 index);
//------------------------------------------------------------------
/// 指定のイベントはすでにもらったか返す
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_IsEventRecvFlag(MYSTERY_DATA * fd, u32 eventno);
//------------------------------------------------------------------
/// 指定のイベントもらったよフラグを立てる
//------------------------------------------------------------------
extern void MYSTERYDATA_SetEventRecvFlag(MYSTERY_DATA * fd, u32 eventno);

//------------------------------------------------------------------
/// CRCをチェックする
//------------------------------------------------------------------
extern BOOL MYSTERYDATA_CheckCrc( const DOWNLOAD_GIFT_DATA * fd );

//------------------------------------------------------------------
/// 受信日時　年月日
//------------------------------------------------------------------
#define MYSTERYDATA_GetYear( recv_date )  \
  (((recv_date) >> MYSTERYGIFT_DATE_SHIFT_YEAR) & MYSTERYGIFT_DATE_MASK_YEAR)

#define MYSTERYDATA_GetMonth( recv_date ) \
  (((recv_date) >> MYSTERYGIFT_DATE_SHIFT_MONTH) & MYSTERYGIFT_DATE_MASK_MONTH)

#define MYSTERYDATA_GetDay( recv_date ) \
  (((recv_date) >> MYSTERYGIFT_DATE_SHIFT_DAY) & MYSTERYGIFT_DATE_MASK_DAY)

#define MYSTERYDATA_PackDate( cp_date ) \
  ((((2000 + (cp_date)->year) & MYSTERYGIFT_DATE_MASK_YEAR ) << MYSTERYGIFT_DATE_SHIFT_YEAR) | \
   (((cp_date)->month & MYSTERYGIFT_DATE_MASK_MONTH) << MYSTERYGIFT_DATE_SHIFT_MONTH) | \
   (((cp_date)->day & MYSTERYGIFT_DATE_MASK_DAY ) << MYSTERYGIFT_DATE_SHIFT_DAY)) \

//------------------------------------------------------------------
//	 指定のカードを入れ替える
//------------------------------------------------------------------
extern void MYSTERYDATA_SwapCard( MYSTERY_DATA * fd, u32 cardindex1, u32 cardindex2 );

//------------------------------------------------------------------
//	 内部セーブと管理外セーブの共通化のため
//------------------------------------------------------------------
extern void MYSTERYDATA_SaveAsyncStart( MYSTERY_DATA * fd, GAMEDATA *p_gamedata );
extern SAVE_RESULT MYSTERYDATA_SaveAsyncMain( MYSTERY_DATA * fd, GAMEDATA *p_gamedata );

//=============================================================================
/**
 *  セーブデータからアクセスされる関数
 *    それ以外の用途には使わないこと！
 */
//=============================================================================
//------------------------------------------------------------------
//データサイズ取得
//------------------------------------------------------------------
extern int MYSTERYDATA_GetWorkSize(void);

//------------------------------------------------------------------
//初期化処理
//------------------------------------------------------------------
extern void MYSTERYDATA_Init(void * fd);

//=============================================================================
/**
 *  save_outside.cでのみ使用される暗号・複合化関数
 *    それ以外の部分では使わないこと！！
 */
//=============================================================================
//暗号化
void MYSTERYDATA_Coded( void *p_data_adrs, MYSTERY_DATA_TYPE type );
//復号化
void MYSTERYDATA_Decoded( void *p_data_adrs, MYSTERY_DATA_TYPE type );

//=============================================================================
/**
 *  デバッグ用
 */
//=============================================================================
//------------------------------------------------------------------
/// デバッグ用にポケモンデータをセットする関数
//------------------------------------------------------------------
extern void MYSTERYDATA_DebugSetPokemon(void);

