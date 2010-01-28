//==============================================================================
/**
 * @file    union_msg.c
 * @brief   ユニオンルーム：ツール類(オーバーレイ領域に配置されます)
 * @author  matsuda
 * @date    2009.07.13(月)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "net_app/union/union_msg.h"
#include "message.naix"
#include "msg/msg_union_room.h"
#include "system/main.h"
#include "savedata/wifihistory.h"


//==============================================================================
//  定数定義
//==============================================================================
///レギュレーション：決定
#define UNION_BATTLE_REGULATION_DECIDE    (100)
///レギュレーション：ルールをみる
#define UNION_BATTLE_REGULATION_RULE      (150)

///レギュレーション：対戦形式
enum{
  UNION_BATTLE_REGULATION_MODE_SINGLE,      ///<シングル
  UNION_BATTLE_REGULATION_MODE_DOUBLE,      ///<ダブル
  UNION_BATTLE_REGULATION_MODE_TRIPLE,      ///<トリプル
  UNION_BATTLE_REGULATION_MODE_ROTATION,    ///<ローテーション
  UNION_BATTLE_REGULATION_MODE_MULTI,       ///<マルチ
  
  UNION_BATTLE_REGULATION_MODE_MAX,
};

///レギュレーション：ルール
enum{
  UNION_BATTLE_REGULATION_RULE_FREE,        ///<制限無し
  UNION_BATTLE_REGULATION_RULE_FLAT,    ///<フラット
  
  UNION_BATTLE_REGULATION_RULE_MAX,
};

///レギュレーション：シューター
enum{
  UNION_BATTLE_REGULATION_SHOOTER_TRUE,     ///<シューター使用する
  UNION_BATTLE_REGULATION_SHOOTER_FALSE,    ///<シューター禁止
  
  UNION_BATTLE_REGULATION_SHOOTER_MAX,
};

///レギュレーション：ノーマル色
#define REG_NORMAL_COLOR      PRINTSYS_MACRO_LSB(1, 2, 0)
///レギュレーション：NG色
#define REG_FAIL_COLOR        PRINTSYS_MACRO_LSB(4, 2, 0)

///union_msg上でのWORDSETのbuf_id管理番号
enum{
  _WORDSET_BUFID_TARGET_NAME,     ///<接続相手の名前
  _WORDSET_BUFID_MINE_NAME,       ///<自分の名前
  _WORDSET_BUFID_TARGET_AISATSU,  ///<言語に合わせた挨拶
  _WORDSET_BUFID_NATION,          ///<国
  _WORDSET_BUFID_AREA,            ///<地域
};


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _SetPmsAisatsu(GAMEDATA * gamedata, u8 region);

//==============================================================================
//  プロトタイプ宣言：データ
//==============================================================================
static const FLDMENUFUNC_LIST BattleMenuList_Number[3];
static const FLDMENUFUNC_LIST BattleMenuList_Custom[6];
static const FLDMENUFUNC_LIST BattleMenuList_Mode[5];
static const FLDMENUFUNC_LIST BattleMenuList_Rule[3];
static const FLDMENUFUNC_LIST BattleMenuList_Shooter[3];
static const FLDMENUFUNC_LIST BattleMenuList_CustomMulti[5];
static const FLDMENUFUNC_LIST BattleMenuList_RuleMulti[3];
static const FLDMENUFUNC_LIST BattleMenuList_ShooterMulti[3];


//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  各ゲームの最大参加人数
//--------------------------------------------------------------
static const struct{
  u8 member_max;
  u8 padding[3];
}MenuMemberMax[] = {
  {2},      //UNION_PLAY_CATEGORY_UNION,          ///<ユニオンルーム
  {2},      //UNION_PLAY_CATEGORY_TALK,           ///<会話中
  {2},      //UNION_PLAY_CATEGORY_TRAINERCARD,    ///<トレーナーカード
  {4},      //UNION_PLAY_CATEGORY_PICTURE,        ///<お絵かき
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT,   ///<コロシアム
  {4},      //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER,
  {4},      //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE,
  {4},      //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER,
  {4},      //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT,
  {2},      //UNION_PLAY_CATEGORY_TRADE,          ///<ポケモン交換
  {4},      //UNION_PLAY_CATEGORY_GURUGURU,       ///<ぐるぐる交換
  {5},      //UNION_PLAY_CATEGORY_RECORD,         ///<レコードコーナー
};

//--------------------------------------------------------------
//  メニューヘッダー
//--------------------------------------------------------------
///メインメニューリスト
static const FLDMENUFUNC_LIST MainMenuList[] =
{
  {msg_union_select_01, (void*)UNION_PLAY_CATEGORY_TRAINERCARD},   //挨拶
  {msg_union_select_04, (void*)UNION_PLAY_CATEGORY_TRADE},   //交換
  {msg_union_select_03, (void*)UNION_MENU_NO_SEND_BATTLE},   //対戦
  {msg_union_select_06, (void*)UNION_PLAY_CATEGORY_GURUGURU}, //ぐるぐる交換
  {msg_union_select_02, (void*)UNION_PLAY_CATEGORY_PICTURE},   //お絵かき
  {msg_union_select_05, (void*)UNION_MENU_SELECT_CANCEL},   //やめる
};

///メニューヘッダー(メインメニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_MainMenu =
{
	6,		//リスト項目数
	6,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	0,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	1,//32-14,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	13,		//表示サイズX キャラ単位
	12,		//表示サイズY キャラ単位
};
//SDK_COMPILER_ASSERT(NELEMS(MainMenuList) == MenuHeader_MainMenu.line);

//--------------------------------------------------------------
//  対戦メニュー
//--------------------------------------------------------------
///二人対戦、四人対戦、やめる
static const FLDMENUFUNC_LIST BattleMenuList_Number[] =
{
  {msg_union_battle_01_01, (void*)BattleMenuList_Custom},   //二人対戦
  {msg_union_battle_01_02, (void*)BattleMenuList_CustomMulti},    //四人対戦
  {msg_union_battle_01_03, (void*)FLDMENUFUNC_CANCEL},    //やめる
};

///二人対戦：カスタムメニュー
static const FLDMENUFUNC_LIST BattleMenuList_Custom[] =
{
  {msg_union_battle_01_04, (void*)BattleMenuList_Mode},      //対戦形式(シングル、ダブル…)
  {msg_union_battle_01_05, (void*)BattleMenuList_Rule},       //ルール(レベル50、制限無し…)
  {msg_union_battle_01_06, (void*)BattleMenuList_Shooter},   //シューター有無
  {msg_union_battle_01_07_01, (void*)UNION_BATTLE_REGULATION_DECIDE},    //決定
  {msg_union_battle_01_07_02, (void*)UNION_BATTLE_REGULATION_RULE},    //ルールをみる
  {msg_union_battle_01_08, (void*)BattleMenuList_Number},    //もどる
};

///二人対戦：シングル、ダブル、トリプル、ローテーション、もどる
static const FLDMENUFUNC_LIST BattleMenuList_Mode[] =
{
  {msg_union_battle_01_04, (void*)UNION_BATTLE_REGULATION_MODE_SINGLE},   //シングル
  {msg_union_battle_01_05, (void*)UNION_BATTLE_REGULATION_MODE_DOUBLE},   //ダブル
  {msg_union_battle_01_06, (void*)UNION_BATTLE_REGULATION_MODE_TRIPLE},   //トリプル
  {msg_union_battle_01_07, (void*)UNION_BATTLE_REGULATION_MODE_ROTATION}, //ローテーション
  {msg_union_battle_01_08, (void*)BattleMenuList_Custom},   //もどる
};

///二人対戦：制限なし、フラット、もどる
static const FLDMENUFUNC_LIST BattleMenuList_Rule[] =
{
  {msg_union_battle_01_10, (void*)UNION_BATTLE_REGULATION_RULE_FREE},     //制限なし
  {msg_union_battle_01_11, (void*)UNION_BATTLE_REGULATION_RULE_FLAT}, //フラット
  {msg_union_battle_01_12, (void*)BattleMenuList_Custom},   //もどる
};

///二人対戦：シューターあり、シューター無し
static const FLDMENUFUNC_LIST BattleMenuList_Shooter[] =
{
  {msg_union_battle_01_11_01, (void*)UNION_BATTLE_REGULATION_SHOOTER_TRUE}, //有
  {msg_union_battle_01_11_02, (void*)UNION_BATTLE_REGULATION_SHOOTER_FALSE}, //無
  {msg_union_battle_01_12,    (void*)BattleMenuList_Custom},   //もどる
};

///マルチ：カスタムメニュー
static const FLDMENUFUNC_LIST BattleMenuList_CustomMulti[] =
{
  {msg_union_battle_01_05, (void*)BattleMenuList_RuleMulti},       //ルール(レベル50、制限無し…)
  {msg_union_battle_01_06, (void*)BattleMenuList_ShooterMulti},   //シューター有無
  {msg_union_battle_01_07_01, (void*)UNION_BATTLE_REGULATION_DECIDE},    //決定
  {msg_union_battle_01_07_02, (void*)UNION_BATTLE_REGULATION_RULE},    //ルールをみる
  {msg_union_battle_01_08, (void*)BattleMenuList_Number},    //もどる
};

///マルチ：制限なし、フラット、もどる
static const FLDMENUFUNC_LIST BattleMenuList_RuleMulti[] =
{
  {msg_union_battle_01_10, (void*)UNION_BATTLE_REGULATION_RULE_FREE},     //制限なし
  {msg_union_battle_01_11, (void*)UNION_BATTLE_REGULATION_RULE_FLAT}, //フラット
  {msg_union_battle_01_12, (void*)BattleMenuList_CustomMulti},   //もどる
};

///マルチ：シューターあり、シューター無し
static const FLDMENUFUNC_LIST BattleMenuList_ShooterMulti[] =
{
  {msg_union_battle_01_11_01, (void*)UNION_BATTLE_REGULATION_SHOOTER_TRUE}, //有
  {msg_union_battle_01_11_02, (void*)UNION_BATTLE_REGULATION_SHOOTER_FALSE}, //無
  {msg_union_battle_01_12,    (void*)BattleMenuList_CustomMulti},   //もどる
};

///対戦メニューのデータテーブル ※BATTLE_MENU_INDEX_???と並びを同じにしておくこと！
static const struct{
  const FLDMENUFUNC_LIST *list;
  u8 list_max;
}BattleMenuDataTbl[] = {
  {
    BattleMenuList_Number,
    3,
  },
  {
    BattleMenuList_Custom,
    6,
  },
  {
    BattleMenuList_Mode,
    5,
  },
  {
    BattleMenuList_Rule,
    3,
  },
  {
    BattleMenuList_Shooter,
    3,
  },
  
  //マルチ用メニュー
  {
    BattleMenuList_CustomMulti,
    5,
  },
  {
    BattleMenuList_RuleMulti,
    3,
  },
  {
    BattleMenuList_ShooterMulti,
    3,
  },
};

///メニューヘッダー(対戦メニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_Battle =
{
	3,		//リスト項目数
	3,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	0,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	1,//32-20,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	19,		//表示サイズX キャラ単位
	14,		//表示サイズY キャラ単位
};
//SDK_COMPILER_ASSERT(NELEMS(MainMenuList) == MenuHeader_MainMenu.line);

//--------------------------------------------------------------
//  POKEPARTY選択メニューヘッダー
//--------------------------------------------------------------
///POKEPARTY選択メニューリスト
static const FLDMENUFUNC_LIST PokePartySelectMenuList[] =
{
  {msg_union_battle_01_23, (void*)COLOSSEUM_SELECT_PARTY_TEMOTI},  //手持ち
  {msg_union_battle_01_24, (void*)COLOSSEUM_SELECT_PARTY_BOX},     //バトルボックス
  {msg_union_battle_01_25, (void*)COLOSSEUM_SELECT_PARTY_CANCEL},  //やめる
};

///メニューヘッダー(POKEPARTY選択メニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_PokePartySelect =
{
	3,		//リスト項目数
	3,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	32-14,		//表示座標X キャラ単位
	24-6-7,		//表示座標Y キャラ単位
	13,		//表示サイズX キャラ単位
	6,		//表示サイズY キャラ単位
};

//--------------------------------------------------------------
///  メニュー選択レギュレーションをゲームカテゴリーに変換するテーブル
//--------------------------------------------------------------
ALIGN4 static const u8 MenuReg_to_PlayCategory[UNION_BATTLE_REGULATION_MODE_MAX][UNION_BATTLE_REGULATION_RULE_MAX][UNION_BATTLE_REGULATION_SHOOTER_MAX] = {
  {//UNION_BATTLE_REGULATION_MODE_SINGLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_DOUBLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_TRIPLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_ROTATION
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_MULTI
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT,
    },
  },
};


//--------------------------------------------------------------
//  
//--------------------------------------------------------------
#define TALK_CARD_MAX		( 2 )
#define TALK_TRADE_MAX		( 2 )
#define TALK_FIGHT_MAX    (4)
#define TALK_BATTLE_MAX		( 4 )
#define TALK_OEKAKI_MAX		( 4 )
#define TALK_RECORD_MAX		( 4 )
#define TALK_GURUGURU_MAX	( 4 )

///挨拶中のOBJに話しかけた時のランダムメッセージ(トレーナーカード中)
ALIGN4 static const u16 talk_table_card[TALK_CARD_MAX][2]={
	{	msg_union_talkboy_01_03,msg_union_talkgirl_01_03},
	{	msg_union_talkboy_01_04,msg_union_talkgirl_01_04},
};
///お絵かき中のOBJに話しかけた時のランダムメッセージ
ALIGN4 static const u16 talk_table_oekaki[TALK_OEKAKI_MAX][2]={
	{	msg_union_talkboy_02_09,msg_union_talkgirl_02_09},
	{	msg_union_talkboy_02_10,msg_union_talkgirl_02_10},
	{	msg_union_talkboy_02_11,msg_union_talkgirl_02_11},
	{	msg_union_talkboy_02_12,msg_union_talkgirl_02_12},
};
///バトル中のOBJに話しかけた時のランダムメッセージ
ALIGN4 static const u16 talk_table_fight[TALK_FIGHT_MAX][2] = {
  {msg_union_talkboy_03_10, msg_union_talkgirl_03_10,},
  {msg_union_talkboy_03_11, msg_union_talkgirl_03_11},
  {msg_union_talkboy_03_12, msg_union_talkgirl_03_12},
  {msg_union_talkboy_03_13, msg_union_talkgirl_03_13},
};

//マルチバトルの途中参加、開始前の待ち「よーし、ちょっとまってて！」
ALIGN4 static const u16 talk_table_battle_wait[2]={
	msg_union_talkboy_03_06,msg_union_talkgirl_03_06,
};
//マルチバトルの途中参加が成立「またせたね！それじゃいっしょにマルチバトルしよう！」
ALIGN4 static const u16 talk_table_battle_start[2] = {
	msg_union_talkboy_03_07,msg_union_talkgirl_03_07,
};
//マルチバトルの途中参加が不成立だった「あっごめん、やっぱりいま都合が悪い」
ALIGN4 static const u16 talk_table_battle_cancel[2] = {
	msg_union_talkboy_03_08,msg_union_talkgirl_03_08,
};

//お絵かきの途中参加、開始前の待ち「よーし、ちょっとまってて！」
ALIGN4 static const u16 talk_table_picture_wait[2]={
	msg_union_talkboy_02_06_01,msg_union_talkgirl_02_06_01,
};
//お絵かきの途中参加が成立
ALIGN4 static const u16 talk_table_picture_start[2] = {
	msg_union_talkboy_02_07,msg_union_talkgirl_02_07,
};
//お絵かきの途中参加が不成立だった「あっごめん、やっぱりいま都合が悪い」
ALIGN4 static const u16 talk_table_picture_cancel[2] = {
	msg_union_talkboy_02_08,msg_union_talkgirl_02_08,
};

//ぐるぐる交換の途中参加、開始前の待ち「よーし、ちょっとまってて！」
ALIGN4 static const u16 talk_table_guruguru_wait[2]={
	msg_union_talkboy_07_06_01,msg_union_talkgirl_07_06_01,
};
//ぐるぐる交換の途中参加が成立
ALIGN4 static const u16 talk_table_guruguru_start[2] = {
	msg_union_talkboy_07_07,msg_union_talkgirl_07_07,
};
//ぐるぐる交換の途中参加が不成立だった「あっごめん、やっぱりいま都合が悪い」
ALIGN4 static const u16 talk_table_guruguru_cancel[2] = {
	msg_union_talkboy_07_09,msg_union_talkgirl_07_09,
};
//ポケモン交換中のOBJに話しかけた時のランダムメッセージ
ALIGN4 static const u16 talk_table_trade[TALK_TRADE_MAX][2]={
	{	msg_union_talkboy_04_04,msg_union_talkgirl_04_04},
	{	msg_union_talkboy_04_05,msg_union_talkgirl_04_05},
};
///ぐるぐる交換中のOBJに話しかけた時のランダムメッセージ
ALIGN4 static const u16 talk_table_guruguru[TALK_GURUGURU_MAX][2]={
	{	msg_union_talkboy_07_10,msg_union_talkgirl_07_10},
	{	msg_union_talkboy_07_11,msg_union_talkgirl_07_11},
	{	msg_union_talkboy_07_12,msg_union_talkgirl_07_12},
	{	msg_union_talkboy_07_13,msg_union_talkgirl_07_13},
};

// メインメニュー決定項目が親から送られてきた後の内容表示
ALIGN4 static const u16 talk_child_mainmenu_view[] = {
  msg_union_connect_01_01, //UNION_PLAY_CATEGORY_TRAINERCARD
  msg_union_connect_02_01, //UNION_PLAY_CATEGORY_PICTURE
  msg_union_connect_03_01_02, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER
  msg_union_connect_03_01_02, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE
  msg_union_connect_03_01_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER
  msg_union_connect_03_01_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT
  msg_union_connect_03_01_05, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER
  msg_union_connect_03_01_05, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE
  msg_union_connect_03_01_06, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER
  msg_union_connect_03_01_06, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT
  msg_union_connect_03_01_08, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER
  msg_union_connect_03_01_08, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE
  msg_union_connect_03_01_09, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER
  msg_union_connect_03_01_09, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT
  msg_union_connect_03_01_11, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER
  msg_union_connect_03_01_11, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE
  msg_union_connect_03_01_12, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER
  msg_union_connect_03_01_12, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT
  msg_union_connect_03_01_13, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER
  msg_union_connect_03_01_13, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE
  msg_union_connect_03_01_14, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER
  msg_union_connect_03_01_14, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT
  msg_union_connect_04_01,    //UNION_PLAY_CATEGORY_TRADE
  msg_union_connect_06_01,    //UNION_PLAY_CATEGORY_GURUGURU
  msg_union_connect_05_01,    //UNION_PLAY_CATEGORY_RECORD
};

// 話しかけが成功したとき
ALIGN4 static const u16 talk_start_child_table[2]={
	msg_union_talkboy_00_02,msg_union_talkgirl_00_02  //「ｘｘｘです！　なにかするかい？」
};
// 話しかけがしっぱいしたとき(メインメニュー選択中の相手に話しかけた)
ALIGN4 static const u16 talk_failed_child_table[2]={
	msg_union_talkboy_00_05,msg_union_talkgirl_00_05
};
// 話しかけタイムアウト
ALIGN4 static const u16 talk_parent_timeout[2] = {
  msg_union_talkboy_00_06, msg_union_talkgirl_00_06,
};

///会話を継続して続ける時のメッセージ「他にも何かする？」
ALIGN4 static const u16 talk_continue[2] = {
  msg_union_talkboy_00_04, msg_union_talkgirl_00_04,
};

// 遊びを断られたとき
ALIGN4 static const u16 talk_failed_table[][2]={
  {msg_union_talkboy_01_02,msg_union_talkgirl_01_02}, //UNION_PLAY_CATEGORY_TRAINERCARD
  {msg_union_talkboy_02_03,msg_union_talkgirl_02_03}, //UNION_PLAY_CATEGORY_PICTURE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER
  {msg_union_talkboy_03_03,msg_union_talkgirl_03_03}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT
  {msg_union_talkboy_04_03,msg_union_talkgirl_04_03}, //UNION_PLAY_CATEGORY_TRADE
  {msg_union_talkboy_07_03,msg_union_talkgirl_07_03,}, //UNION_PLAY_CATEGORY_GURUGURU
  {msg_union_talkboy_06_03,msg_union_talkgirl_06_03,}, //UNION_PLAY_CATEGORY_RECORD
};

// 子MSG:遊びを断ったとき
ALIGN4 static const u16 talk_child_no_table[]={
  msg_union_connect_01_03, //UNION_PLAY_CATEGORY_TRAINERCARD
  msg_union_connect_02_03, //UNION_PLAY_CATEGORY_PICTURE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER
  msg_union_connect_03_03, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT
  msg_union_connect_04_03, //UNION_PLAY_CATEGORY_TRADE
  msg_union_connect_06_03, //UNION_PLAY_CATEGORY_GURUGURU
  msg_union_connect_06_03, //UNION_PLAY_CATEGORY_RECORD
};

///マルチバトル開始(最初の二人)
ALIGN4 static const u16 talk_multi_start[2] = {
  msg_union_talkboy_03_02, msg_union_talkgirl_03_02,
};

// ぐるぐる交換：手持ちにダメタマゴがある場合のNGメッセージ
ALIGN4 static const u16 guruguru_ng_dametamago[2] = {
  msg_union_talkboy_07_14, msg_union_talkgirl_07_14,
};

// ポケモン交換を選んだが自分の手持ちが1匹しかいない
ALIGN4 static const u16 talk_trade_mine_poke_ng[2] = {
	msg_union_talkboy_04_06,msg_union_talkgirl_04_06,	// 2匹以上てもちがいないので交換できない
};
// ぐるぐる交換を選んだが自分がタマゴを持っていない
ALIGN4 static const u16 talk_guruguru_mine_egg_ng[2] = {
	msg_union_talkboy_07_04,msg_union_talkgirl_07_04,
};

// メインメニュー選択結果を相手に送信中の待ち
ALIGN4 static const u16 talk_answer_wait_table[][2]={
  { msg_union_talkboy_01_01,msg_union_talkgirl_01_01,}, //UNION_PLAY_CATEGORY_TRAINERCARD
  { msg_union_talkboy_02_01,msg_union_talkgirl_02_01,}, //UNION_PLAY_CATEGORY_PICTURE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FREE
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT_SHOOTER
  { msg_union_talkboy_03_01,msg_union_talkgirl_03_01,}, //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI_FLAT
  { msg_union_talkboy_04_01,msg_union_talkgirl_04_01,}, //UNION_PLAY_CATEGORY_TRADE
  { msg_union_talkboy_07_01,msg_union_talkgirl_07_01,}, //UNION_PLAY_CATEGORY_GURUGURU
  { msg_union_talkboy_07_01,msg_union_talkgirl_07_01,}, //UNION_PLAY_CATEGORY_RECORD
};

///ぐるぐる交換に途中参加するかい？
ALIGN4 static const u16 talk_guruguru_intrude[2] = {
  msg_union_talkboy_07_05, msg_union_talkgirl_07_05,
};
///お絵かきに途中参加するかい？
ALIGN4 static const u16 talk_picture_intrude[2] = {
  msg_union_talkboy_02_05,msg_union_talkgirl_02_05,
};
///お絵かきの途中参加を断った
ALIGN4 static const u16 talk_picture_intrude_no_table[2]={
  msg_union_talkboy_02_06,msg_union_talkgirl_02_06,
};
///ぐるぐる交換の途中参加を断った
ALIGN4 static const u16 talk_guruguru_intrude_no_table[2]={
  msg_union_talkboy_07_06,msg_union_talkgirl_07_06,
};
///マルチバトルの途中参加を断った
ALIGN4 static const u16 talk_multi_intrude_no_table[2]={
  msg_union_talkboy_03_05,msg_union_talkgirl_03_05,
};
///ぐるぐる交換の途中参加でタマゴをもっていなかった
ALIGN4 static const u16 talk_guruguru_intrude_no_egg[2] = {
  msg_union_talkboy_07_08, msg_union_talkgirl_07_08,
};
///ぐるぐる交換の途中参加でダメタマゴが手持ちにあった
ALIGN4 static const u16 talk_guruguru_intrude_dametamago[2] = {
  msg_union_talkboy_07_14, msg_union_talkgirl_07_14,
};
//またこえをかけてくれよ
ALIGN4 static const u16 talk_connect_cancel_table[2]={
	msg_union_talkboy_05_01,msg_union_talkgirl_05_01
};
//あ！ごめん　他に用事があるからまた今度ね
ALIGN4 static const u16 talk_connect_canceled_table[2]={
	msg_union_talkboy_00_03,msg_union_talkgirl_00_03
};

///マルチバトルに途中参加するかい？
ALIGN4 static const u16 talk_multil_intrude_table[2] = {
  msg_union_talkboy_03_04, msg_union_talkgirl_03_04,
};



//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 話しかけたがタイムアウトになったとき
 * @param   unisys		
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TalkTimeout(UNION_SYSTEM_PTR unisys)
{
  return talk_parent_timeout[MyStatus_GetMySex(unisys->uniparent->mystatus)];
}

//==================================================================
/**
 * 話しかけた時
 * @param   sex		話しかけた相手の性別
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_ParentConnectStart(int target_sex)
{
  return talk_start_child_table[target_sex];
}

//==================================================================
/**
 * 自分が話をやめるを実行した時
 * @param   target_sex		
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TalkCancel(int target_sex)
{
  return talk_connect_cancel_table[target_sex];
}

//==================================================================
/**
 * 相手から話をやめるを実行された時
 * @param   target_sex		
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TalkCancelPartner(int target_sex)
{
  return talk_connect_canceled_table[target_sex];
}

//==================================================================
/**
 * 親MSG：メインメニュー選択後、子の返事待ち中のメッセージ
 * @param   mainmenu_select		メインメニュー選択結果
 * @retval  u16		表示MsgID
 */
//==================================================================
u16 UnionMsg_GetMsgID_ParentMainMenuSelectAnswerWait(u32 mainmenu_select, int target_sex)
{
  return talk_answer_wait_table[mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD][target_sex];
}

//==================================================================
/**
 * 親MSG：メインメニュー選択後、子から遊びを断られた
 * @param   mainmenu_select		メインメニュー選択結果
 * @retval  u16		表示MsgID
 */
//==================================================================
u16 UnionMsg_GetMsgID_ParentMainMenuSelectAnswerNG(u32 mainmenu_select, int target_sex)
{
  return talk_failed_table[mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD][target_sex];
}

//==================================================================
/**
 * 子MSG：メインメニュー選択が親から送られてきた時の内容表示
 * @param   mainmenu_select		メインメニュー選択結果
 * @retval  u16		表示MsgID
 */
//==================================================================
u16 UnionMsg_GetMsgID_ChildMainMenuSelectView(u32 mainmenu_select)
{
  return talk_child_mainmenu_view[mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD];
}

//==================================================================
/**
 * 子MSG：親から送られてきたメインメニューの項目を断った
 * @param   mainmenu_select		メインメニュー選択結果
 * @retval  u16		表示MsgID
 */
//==================================================================
u16 UnionMsg_GetMsgID_ChildMainMenuSelectRefuse(u32 mainmenu_select)
{
  return talk_child_no_table[mainmenu_select - UNION_PLAY_CATEGORY_TRAINERCARD];
}

//==================================================================
/**
 * 会話を継続するとき
 * @param   sex		話しかけた相手の性別
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TalkContinue(int target_sex)
{
  return talk_continue[target_sex];
}

//==================================================================
/**
 * マルチバトル開始(最初の二人)
 * @param   target_sex		
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiStart(int target_sex)
{
  return talk_multi_start[target_sex];
}

//==================================================================
/**
 * ポケモン交換を選んだが自分の手持ちが1匹しかいない
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TradeMinePokeNG(int target_sex)
{
  return talk_trade_mine_poke_ng[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換を選んだが自分がタマゴを持っていない
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruMineEggNG(int target_sex)
{
  return talk_guruguru_mine_egg_ng[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換を選んだが自分がダメタマゴを持っている
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruMineDametamago(UNION_SYSTEM_PTR unisys)
{
  return talk_guruguru_intrude_dametamago[MyStatus_GetMySex(unisys->uniparent->mystatus)];
}

//==================================================================
/**
 * ぐるぐる交換をしようとしたが手持ちにダメタマゴがあった場合のNGメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruDameTamagoNG(UNION_SYSTEM_PTR unisys)
{
  return guruguru_ng_dametamago[MyStatus_GetMySex(unisys->uniparent->mystatus)];
}

//==================================================================
/**
 * メインメニュー選択中の相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGameBattle(int target_sex)
{
  return talk_failed_child_table[target_sex];
}

//==================================================================
/**
 * バトル中の相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGameMainMenuSelect(int target_sex)
{
  return talk_table_fight[GFUser_GetPublicRand(TALK_FIGHT_MAX)][target_sex];
}

//==================================================================
/**
 * トレーナーカードしている相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGameTrainerCard(int target_sex)
{
  return talk_table_card[GFUser_GetPublicRand(TALK_CARD_MAX)][target_sex];
}

//==================================================================
/**
 * お絵かきしている相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGamePicture(int target_sex)
{
  return talk_table_oekaki[GFUser_GetPublicRand(TALK_OEKAKI_MAX)][target_sex];
}

//==================================================================
/**
 * ポケモン交換している相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGameTrade(int target_sex)
{
  return talk_table_trade[GFUser_GetPublicRand(TALK_TRADE_MAX)][target_sex];
}

//==================================================================
/**
 * ぐるぐる交換している相手に話しかけた時のランダムメッセージ
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PlayGameGuruguru(int target_sex)
{
  return talk_table_guruguru[GFUser_GetPublicRand(TALK_GURUGURU_MAX)][target_sex];
}

//==================================================================
/**
 * マルチバトルに途中参加するかい？
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiIntrude(int target_sex)
{
  return talk_multil_intrude_table[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換に途中参加するかい？
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GurugurIntrude(int target_sex)
{
  return talk_guruguru_intrude[target_sex];
}

//==================================================================
/**
 * お絵かきに途中参加するかい？
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PictureIntrude(int target_sex)
{
  return talk_picture_intrude[target_sex];
}

//==================================================================
/**
 * マルチバトルの途中参加を断った
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiIntrudeRefuses(int target_sex)
{
  return talk_multi_intrude_no_table[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加を断った
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeRefuses(int target_sex)
{
  return talk_guruguru_intrude_no_table[target_sex];
}

//==================================================================
/**
 * お絵かきの途中参加を断った
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PictureIntrudeRefuses(int target_sex)
{
  return talk_picture_intrude_no_table[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加でタマゴを持っていなかった
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeNoEgg(int target_sex)
{
  return talk_guruguru_intrude_no_egg[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加でダメタマゴが手持ちにあった
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeDametamago(int target_sex)
{
  return talk_guruguru_intrude_dametamago[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加、親の承諾待ち
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeWait(int target_sex)
{
  return talk_table_guruguru_wait[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加、途中参加が成立
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeSuccess(int target_sex)
{
  return talk_table_guruguru_start[target_sex];
}

//==================================================================
/**
 * ぐるぐる交換の途中参加、不成立(親からNG)
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_GuruguruIntrudeParentNG(int target_sex)
{
  return talk_table_guruguru_cancel[target_sex];
}

//==================================================================
/**
 * お絵かきの途中参加、親の承諾待ち
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PictureIntrudeWait(int target_sex)
{
  return talk_table_picture_wait[target_sex];
}

//==================================================================
/**
 * お絵かきの途中参加、途中参加が成立
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PictureIntrudeSuccess(int target_sex)
{
  return talk_table_picture_start[target_sex];
}

//==================================================================
/**
 * お絵かきの途中参加、不成立(親からNG)
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_PictureIntrudeParentNG(int target_sex)
{
  return talk_table_picture_cancel[target_sex];
}

//==================================================================
/**
 * マルチバトルの途中参加、親の承諾待ち
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiIntrudeWait(int target_sex)
{
  return talk_table_battle_wait[target_sex];
}

//==================================================================
/**
 * マルチバトルの途中参加、途中参加が成立
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiIntrudeSuccess(int target_sex)
{
  return talk_table_battle_start[target_sex];
}

//==================================================================
/**
 * マルチバトルの途中参加、不成立(親からNG)
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_MultiIntrudeParentNG(int target_sex)
{
  return talk_table_battle_cancel[target_sex];
}

//==================================================================
/**
 * トレーナーカード開始時のメッセージID取得
 *
 * @param   unisys		
 * @param   target_nation		接続相手の国
 * @param   target_area		  接続相手の地域
 *
 * @retval  u16		
 */
//==================================================================
u16 UnionMsg_GetMsgID_TrainerCardStart(UNION_SYSTEM_PTR unisys, int target_nation, int target_area)
{
  MYSTATUS *mystatus;
  int my_nation, my_area;
  
  mystatus = GAMEDATA_GetMyStatus(unisys->uniparent->game_data);
  my_nation = MyStatus_GetMyNation(mystatus);
  my_area = MyStatus_GetMyArea(mystatus);

  if(target_nation == 0){ //国がない
  	return msg_union_connect_01_02_3;
  }
  
	// 国が違う
	if(my_nation != target_nation){
		// 地域がない
		if(target_area == 0){
			return msg_union_connect_01_02_1;
		}
		// 地域がある
		return msg_union_connect_01_02;
	}

	// 国が同じ
	if(target_area == 0 || my_area == target_area){
  	return msg_union_connect_01_02_3; //地域が同じが指定が無い
  }
  return msg_union_connect_01_02_2; //地域が違う
}


//==============================================================================
//
//  
//
//==============================================================================
//--------------------------------------------------------------
/**
 * WORDSETにデフォルトでセットする単語を登録する
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void _WordsetSetDefaultData(UNION_SYSTEM_PTR unisys)
{
  UNION_MY_SITUATION *situ = &unisys->my_situation;
  const MYSTATUS *mine_myst = unisys->uniparent->mystatus;
  const STRCODE *target_name;
  u8 target_region, target_sex;
  STRBUF *strbuf_targetname, *greeting;
  u16 msgno;
  
  GF_ASSERT(unisys->msgdata != NULL);
  GF_ASSERT(unisys->wordset != NULL);
  
  if(situ->mycomm.talk_pc != NULL){
    target_name = situ->mycomm.talk_pc->beacon.name;
    target_region = situ->mycomm.talk_pc->beacon.language;
    target_sex = situ->mycomm.talk_pc->beacon.sex;
  }
  else{
    GF_ASSERT(0);
    //吹っ飛ばないように自分のデータを入れておく
    target_name = MyStatus_GetMyName(mine_myst);
    target_region = PM_LANG;
    target_sex = MyStatus_GetMySex(mine_myst);
  }
  
  //相手の名前
  strbuf_targetname = GFL_STR_CreateBuffer(32, HEAPID_UNION);
  GFL_STR_SetStringCode(strbuf_targetname, target_name);
	WORDSET_RegisterWord( unisys->wordset, _WORDSET_BUFID_TARGET_NAME, 
	  strbuf_targetname, target_sex, TRUE, target_region );
	GFL_STR_DeleteBuffer(strbuf_targetname);
	
	//自分の名前
	WORDSET_RegisterPlayerName( 
	  unisys->wordset, _WORDSET_BUFID_MINE_NAME, mine_myst );

	// 相手の言語によって挨拶が変わる
	switch(target_region){
	case LANG_JAPAN: 	msgno = msg_union_greeting_01;	break;
	case LANG_ENGLISH: 	msgno = msg_union_greeting_02;	break;
	case LANG_FRANCE: 	msgno = msg_union_greeting_03;	break;
	case LANG_ITALY	: 	msgno = msg_union_greeting_04;	break;
	case LANG_GERMANY: 	msgno = msg_union_greeting_05;	break;
	case LANG_SPAIN	: 	msgno = msg_union_greeting_06;	break;
	case LANG_KOREA :   msgno = msg_union_greeting_08;  break;
	default:
		msgno = msg_union_greeting_07;
		break;
	}
  greeting = GFL_MSG_CreateString( unisys->msgdata, msgno );
	// 各国の「こんにちは」を格納する
	WORDSET_RegisterWord( unisys->wordset, _WORDSET_BUFID_TARGET_AISATSU, greeting, 
	  target_sex, TRUE, target_region );
	GFL_STR_DeleteBuffer(greeting);
	
	if(situ->mycomm.talk_pc != NULL && situ->mycomm.talk_pc->beacon.my_nation > 0){
  	//国
  	WORDSET_RegisterCountryName( unisys->wordset, _WORDSET_BUFID_NATION, 
  	  situ->mycomm.talk_pc->beacon.my_nation );
  	if(situ->mycomm.talk_pc->beacon.my_area > 0){
    	//地域
    	WORDSET_RegisterLocalPlaceName(unisys->wordset, _WORDSET_BUFID_AREA, 
    	  situ->mycomm.talk_pc->beacon.my_nation, situ->mycomm.talk_pc->beacon.my_area);
    }
  }
	
	//簡易会話システムフラグセット
	_SetPmsAisatsu( unisys->uniparent->game_data, target_region );
}

//--------------------------------------------------------------
/**
 * 各言語カートリッジにあわせて簡易会話フラグを追加する
 *
 * @param   gamedata		
 * @param   region		  言語コード
 */
//--------------------------------------------------------------
static void _SetPmsAisatsu(GAMEDATA * gamedata, u8 region)
{
	if(region>=LANG_JAPAN && region <=LANG_KOREA){
		static const s32 AisatsuFlagTable[] = {
			PMSW_AISATSU_JP,	// 日本語
			PMSW_AISATSU_EN,	// 英語
			PMSW_AISATSU_FR,	// フランス語
			PMSW_AISATSU_IT,	// イタリア語
			PMSW_AISATSU_GE,	// ドイツ語
			-1,					// 無効言語ナンバー
			PMSW_AISATSU_SP,	// スペイン語
			-1,               // 韓国語
		};

		u16 lang = region - LANG_JAPAN;
		if( (lang < NELEMS(AisatsuFlagTable)) && (AisatsuFlagTable[lang] >= 0) )
		{
      SAVE_CONTROL_WORK *sv_ctrl = GAMEDATA_GetSaveControlWork(gamedata);
      PMSW_SAVEDATA *pmssave = SaveData_GetPMSW( sv_ctrl );
      
			PMSW_SetAisatsuFlag( pmssave,  AisatsuFlagTable[lang] );
			OS_Printf("簡易会話に %d　を追加\n", PMSW_AISATSU_JP+( region - LANG_JAPAN ) );
		}
	}
}

//--------------------------------------------------------------
/**
 * WORDSETを作成し、デフォルトデータの登録も行う
 *
 * @param   unisys		
 *
 * msgdataをセットしてから呼び出すこと
 */
//--------------------------------------------------------------
static void _WordsetCreate( UNION_SYSTEM_PTR unisys )
{
  if(unisys->wordset != NULL){
    return;
  }
  unisys->wordset = WORDSET_Create( HEAPID_UNION );
  _WordsetSetDefaultData(unisys);
}

//--------------------------------------------------------------
/**
 * WORDSETを削除
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void _WordsetDelete( UNION_SYSTEM_PTR unisys )
{
  if(unisys->wordset != NULL){
    WORDSET_Delete(unisys->wordset);
    unisys->wordset = NULL;
  }
}


//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 各メニューで選択したゲームの通信最大参加人数を取得する
 *
 * @param   menu_index		UNION_MSG_MENU_SELECT_???
 *
 * @retval  int		最大参加人数
 */
//==================================================================
int UnionMsg_GetMemberMax(UNION_PLAY_CATEGORY menu_index)
{
  if(menu_index >= UNION_PLAY_CATEGORY_MAX){
    GF_ASSERT(menu_index < UNION_PLAY_CATEGORY_MAX);
    return 0;
  }
  return MenuMemberMax[menu_index].member_max;
}

//==================================================================
/**
 * UnionMsg系のものを全て一括削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_AllDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_YesNo_Del(unisys);
  UnionMsg_Menu_MainMenuDel(unisys);
  UnionMsg_TalkStream_WindowDel(unisys);
}

//==================================================================
/**
 * 会話ウィンドウ：セットアップ
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;
  
  if(unisys->fld_msgwin_stream == NULL){
    GF_ASSERT(unisys->msgdata == NULL);
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->msgdata = FLDMSGBG_CreateMSGDATA( fldmsg_bg, NARC_message_union_room_dat );
    unisys->strbuf_temp = GFL_STR_CreateBuffer(256, HEAPID_UNION);
    unisys->strbuf_talk_expand = GFL_STR_CreateBuffer(256, HEAPID_UNION);
    _WordsetCreate(unisys);
    unisys->fld_msgwin_stream = FLDPLAINMSGWIN_Add(fldmsg_bg, unisys->msgdata,
      1, 19, 30, 4);
  }
  else{ //既にウィンドウが表示されている場合はメッセージエリアのクリアを行う
    FLDPLAINMSGWIN_ClearMessage(unisys->fld_msgwin_stream);
  }
}

//==================================================================
/**
 * 会話ウィンドウ：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream != NULL){
    FLDPLAINMSGWIN_ClearWindow(unisys->fld_msgwin_stream);
    FLDPLAINMSGWIN_Delete(unisys->fld_msgwin_stream);
    unisys->fld_msgwin_stream = NULL;
    unisys->fld_msgwin_stream_print_on = FALSE;
    
    GFL_STR_DeleteBuffer(unisys->strbuf_temp);
    GFL_STR_DeleteBuffer(unisys->strbuf_talk_expand);
    unisys->strbuf_temp = NULL;
    unisys->strbuf_talk_expand = NULL;
  }

  if(unisys->msgdata != NULL){
    _WordsetDelete(unisys);
    FLDMSGBG_DeleteMSGDATA(unisys->msgdata);
    unisys->msgdata = NULL;
  }
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始
 *
 * @param   unisys		
 * @param   str_id		メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_Print(UNION_SYSTEM_PTR unisys, u32 str_id)
{
  GF_ASSERT(unisys->fld_msgwin_stream != NULL);

  GFL_MSG_GetString(unisys->msgdata, str_id, unisys->strbuf_temp);
  WORDSET_ExpandStr(unisys->wordset, unisys->strbuf_talk_expand, unisys->strbuf_temp);
  FLDPLAINMSGWIN_PrintStreamStartStrBuf(
    unisys->fld_msgwin_stream, 0, 0, unisys->strbuf_talk_expand);

  unisys->fld_msgwin_stream_print_on = TRUE;
}

//==================================================================
/**
 * 会話ウィンドウ：終了チェック
 *
 * @param   unisys		
 *
 * @retval  BOOL		TRUE:終了。　FALSE:継続中
 */
//==================================================================
BOOL UnionMsg_TalkStream_Check(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream == NULL || unisys->fld_msgwin_stream_print_on == FALSE){
    return TRUE;
  }
  return FLDPLAINMSGWIN_PrintStream(unisys->fld_msgwin_stream);
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始に加え、メッセージウィンドウが無い場合は生成も行う
 *
 * @param   unisys		  
 * @param   fieldWork		
 * @param   str_id		  メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_PrintPack(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, u32 str_id)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  _WordsetSetDefaultData(unisys);
  UnionMsg_TalkStream_Print(unisys, str_id);
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウセットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_YesNo_Setup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;

  if(unisys->fldmenu_yesno_func == NULL){
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->fldmenu_yesno_func = FLDMENUFUNC_AddYesNoMenu(fldmsg_bg, FLDMENUFUNC_YESNO_YES);
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウ削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_YesNo_Del(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_yesno_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_yesno_func);
    unisys->fldmenu_yesno_func = NULL;
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択待ち
 *
 * @param   unisys		
 * @param   result		結果代入先(TRUE:はい、FALSE:いいえ)
 *
 * @retval  BOOL		TRUE:選択した。　FALSE:選択中
 */
//==================================================================
BOOL UnionMsg_YesNo_SelectLoop(UNION_SYSTEM_PTR unisys, BOOL *result)
{
  FLDMENUFUNC_YESNO yes_no;
  
  GF_ASSERT(unisys->fldmenu_yesno_func != NULL);
  
  yes_no = FLDMENUFUNC_ProcYesNoMenu(unisys->fldmenu_yesno_func);
  if(yes_no == FLDMENUFUNC_YESNO_NULL){
    return FALSE;
  }
  else if(yes_no == FLDMENUFUNC_YESNO_YES){
    *result = TRUE;
  }
  else{
    *result = FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * メニューリスト：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   menulist		
 * @param   list_max		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, const FLDMENUFUNC_LIST *menulist, int list_max, const FLDMENUFUNC_HEADER *menuhead)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;
  
  if(unisys->fldmenu_func == NULL){
    fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
      menulist, list_max, unisys->msgdata, HEAPID_UNION);
    unisys->fldmenu_func = FLDMENUFUNC_AddEventMenuList( fldmsg_bg, menuhead, fldmenu_listdata,
      NULL, NULL, 0, 0, TRUE);
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：削除
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_func);
    unisys->fldmenu_func = NULL;
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：選択待ち
 *
 * @param   unisys		
 *
 * @retval  u32		結果
 */
//--------------------------------------------------------------
static u32 UnionMsg_Menu_SelectLoop(UNION_SYSTEM_PTR unisys)
{
  GF_ASSERT(unisys->fldmenu_func != NULL);
  return FLDMENUFUNC_ProcMenu(unisys->fldmenu_func);
}

//==================================================================
/**
 * メインメニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_MainMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
    MainMenuList, NELEMS(MainMenuList), &MenuHeader_MainMenu);
}

//==================================================================
/**
 * メインメニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_MainMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * メインメニュー：選択待ち
 *
 * @param   unisys		
 * 
 * @retval  u32		結果
 */
//==================================================================
u32 UnionMsg_Menu_MainMenuSelectLoop(UNION_SYSTEM_PTR unisys)
{
  return UnionMsg_Menu_SelectLoop(unisys);
}

//==================================================================
/**
 * 対戦メニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, int menu_index, UNION_MENU_REGULATION *menu_reg)
{
  FLDMENUFUNC_HEADER head;
  
  menu_reg->menu_index = menu_index;
  
  head = MenuHeader_Battle;
  head.count = BattleMenuDataTbl[menu_index].list_max;
  head.line = BattleMenuDataTbl[menu_index].list_max;
  head.bmpsize_y = head.count * 2;
  
  if(menu_index == BATTLE_MENU_INDEX_CUSTOM || menu_index == BATTLE_MENU_INDEX_CUSTOM_MULTI){
    head.bmpsize_x = 30;
  }
  if(menu_index >= BATTLE_MENU_INDEX_CUSTOM_MULTI){
    head.bmppos_y = 5;
  }

  if(menu_index == BATTLE_MENU_INDEX_CUSTOM){
    FLDMENUFUNC_LIST *list;
    
    list = GFL_HEAP_AllocClearMemory(
      HEAPID_UNION, sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_Custom));
    GFL_STD_MemCopy(BattleMenuList_Custom, list, 
      sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_Custom));
    
    list[0].str_id = msg_union_battle_01_12_01 + menu_reg->mode;     //対戦形式(シングル、ダブル…)
    list[1].str_id = msg_union_battle_01_12_06 + menu_reg->rule;     //ルール(レベル50、制限無し…)
    list[2].str_id = msg_union_battle_01_12_08 + menu_reg->shooter;  //シューター有無
    
    UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
      list, BattleMenuDataTbl[menu_index].list_max, &head);
    GFL_HEAP_FreeMemory(list);
  }
  else if(menu_index == BATTLE_MENU_INDEX_CUSTOM_MULTI){
    FLDMENUFUNC_LIST *list;
    
    list = GFL_HEAP_AllocClearMemory(
      HEAPID_UNION, sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_CustomMulti));
    GFL_STD_MemCopy(BattleMenuList_CustomMulti, list, 
      sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_CustomMulti));
    
    list[0].str_id = msg_union_battle_01_12_06 + menu_reg->rule;     //ルール(レベル50、制限無し…)
    list[1].str_id = msg_union_battle_01_12_08 + menu_reg->shooter;  //シューター有無
    
    UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
      list, BattleMenuDataTbl[menu_index].list_max, &head);
    GFL_HEAP_FreeMemory(list);
  }
  else{
    UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
      BattleMenuDataTbl[menu_index].list, BattleMenuDataTbl[menu_index].list_max, &head);
  }
}

//==================================================================
/**
 * 対戦メニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * 対戦メニュー：選択待ち
 *
 * @param   unisys		
 * @param   next_sub_menu		TRUE:次のサブメニューがある　FALSE:最終結果
 *
 * @retval  u32		next_sub_menuがTRUEの場合、次のメニューIndex
 * @retval  u32		next_sub_menuがFALSEの場合、最終結果番号(これを通信で送る)
 */
//==================================================================
u32 UnionMsg_Menu_BattleMenuSelectLoop(UNION_SYSTEM_PTR unisys, BOOL *next_sub_menu, UNION_MENU_REGULATION *menu_reg, BOOL *reg_look)
{
  u32 menu_ret;
  int i;
  
  *next_sub_menu = FALSE;
  *reg_look = FALSE;
  
  menu_ret = UnionMsg_Menu_SelectLoop(unisys);
  switch(menu_ret){
  case FLDMENUFUNC_NULL:
    return menu_ret;
  case FLDMENUFUNC_CANCEL:
    if(menu_reg->menu_index == BATTLE_MENU_INDEX_NUMBER){
      return menu_ret;
    }
    //一番下のメニューがキャンセル動作のはずなので、それを返す
    menu_ret = (u32)(BattleMenuDataTbl[menu_reg->menu_index].list[BattleMenuDataTbl[menu_reg->menu_index].list_max-1].selectParam);
    break;
  case UNION_BATTLE_REGULATION_DECIDE:
    return MenuReg_to_PlayCategory[menu_reg->mode][menu_reg->rule][menu_reg->shooter];
  case UNION_BATTLE_REGULATION_RULE:
    *reg_look = TRUE; //レギュレーションを見る
    return MenuReg_to_PlayCategory[menu_reg->mode][menu_reg->rule][menu_reg->shooter];
  }
  if(menu_reg->menu_index == BATTLE_MENU_INDEX_NUMBER){
    if(menu_ret == (u32)(&BattleMenuList_Custom)){  //二人対戦
      if(menu_reg->mode == UNION_BATTLE_REGULATION_MODE_MULTI){
        menu_reg->mode = UNION_BATTLE_REGULATION_MODE_SINGLE;
      }
    }
    else if(menu_ret == (u32)(&BattleMenuList_CustomMulti)){  //マルチ
      menu_reg->mode = UNION_BATTLE_REGULATION_MODE_MULTI;
    }
  }

  for(i = 0; i < NELEMS(BattleMenuDataTbl); i++){
    if(BattleMenuDataTbl[i].list == (void*)menu_ret){
      *next_sub_menu = TRUE;
      return i;
    }
    else{
      switch(menu_reg->menu_index){
      case BATTLE_MENU_INDEX_MODE:
        if(menu_ret < UNION_BATTLE_REGULATION_MODE_MAX){
          menu_reg->mode = menu_ret;
          *next_sub_menu = TRUE;
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      case BATTLE_MENU_INDEX_RULE:
      case BATTLE_MENU_INDEX_RULE_MULTI:
        if(menu_ret < UNION_BATTLE_REGULATION_RULE_MAX){
          menu_reg->rule = menu_ret;
          *next_sub_menu = TRUE;
          if(menu_reg->menu_index == BATTLE_MENU_INDEX_RULE_MULTI){
            return BATTLE_MENU_INDEX_CUSTOM_MULTI;
          }
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      case BATTLE_MENU_INDEX_SHOOTER:
      case BATTLE_MENU_INDEX_SHOOTER_MULTI:
        if(menu_ret < UNION_BATTLE_REGULATION_SHOOTER_MAX){
          menu_reg->shooter = menu_ret;
          *next_sub_menu = TRUE;
          if(menu_reg->menu_index == BATTLE_MENU_INDEX_SHOOTER_MULTI){
            return BATTLE_MENU_INDEX_CUSTOM_MULTI;
          }
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      
      }
    }
  }
  
  GF_ASSERT_MSG(0, "menu_ret=%d\n", menu_ret);
  return 0;
}

//==================================================================
/**
 * 対戦メニューでマルチバトルを選んだときに出しておく表題をセットアップ
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuMultiTitleSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  if(unisys->fldmsgwin == NULL){
    FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->fldmsgwin = FLDPLAINMSGWIN_Add(fldmsg_bg, unisys->msgdata, 1, 1, 30, 2);
    FLDPLAINMSGWIN_Print( unisys->fldmsgwin, 0, 0, msg_union_battle_01_12_05 );
  }
}

//==================================================================
/**
 * 対戦メニューでマルチバトルを選んだときに出しておく表題を削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuMultiTitleDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmsgwin != NULL){
    FLDPLAINMSGWIN_Delete(unisys->fldmsgwin);
    unisys->fldmsgwin = NULL;
  }
}

//==================================================================
/**
 * POKEPARTY選択メニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_PokePartySelectMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
    PokePartySelectMenuList, NELEMS(PokePartySelectMenuList), &MenuHeader_PokePartySelect);
}

//==================================================================
/**
 * POKEPARTY選択メニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_PokePartySelectMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * POKEPARTY選択メニュー：選択待ち
 *
 * @param   unisys		
 *
 * @retval  u32		結果
 */
//==================================================================
u32 UnionMsg_Menu_PokePartySelectMenuSelectLoop(UNION_SYSTEM_PTR unisys)
{
  return UnionMsg_Menu_SelectLoop(unisys);
}

//==================================================================
/**
 * NGレギュレーション：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   fail_bit    レギュレーションNGbit
 * @param   shooter_type  TRUE:シューター有　FALSE:シューター無し
 * @param   regwin_type    NGREG_TYPE_???
 */
//==================================================================
void UnionMsg_Menu_RegulationSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, u32 fail_bit, BOOL shooter_type, REGWIN_TYPE regwin_type)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  int category;
  PRINTSYS_LSB color;
  u16 title_msgid;
  
  GF_ASSERT(unisys->wordset != NULL);
  GF_ASSERT(unisys->fldmsgwin == NULL);
  GF_ASSERT(unisys->alloc.rpm == NULL);
  
  if(shooter_type == TRUE){
    shooter_type = REGULATION_SHOOTER_VALID;
  }
  else{
    shooter_type = REGULATION_SHOOTER_INVALID;
  }
  
//  _WordsetCreate(unisys);
  unisys->fldmsgwin = FLDPLAINMSGWIN_Add(fldmsg_bg, unisys->msgdata, 
    1, 1, 26, (REGULATION_CATEGORY_MAX+2)*2);
  unisys->alloc.rpm = PokeRegulation_CreatePrintMsg(unisys->alloc.regulation,
    unisys->wordset, HEAPID_UNION, shooter_type);
  _WordsetSetDefaultData(unisys); //レギュレーションでWORDSETが上書きされたので復帰
  
  if(regwin_type == REGWIN_TYPE_RULE){
    STRBUF* cupname = Regulation_CreateCupName(unisys->alloc.regulation, HEAPID_UNION);
    FLDPLAINMSGWIN_PrintStrBuf( unisys->fldmsgwin, 0, 0, cupname );
    GFL_STR_DeleteBuffer(cupname);
  }
  else{
    if(regwin_type == REGWIN_TYPE_NG_TEMOTI){
      title_msgid = msg_union_ng_temoti;
    }
    else{
      title_msgid = msg_union_ng_bbox;
    }
    FLDPLAINMSGWIN_Print( unisys->fldmsgwin, 0, 0, title_msgid );
  }
  
  for(category = 0; category < REGULATION_CATEGORY_MAX; category++){
    if(fail_bit & (1 << category)){
      color = REG_FAIL_COLOR;
    }
    else{
      color = REG_NORMAL_COLOR;
    }
    FLDPLAINMSGWIN_PrintStrBufColor( unisys->fldmsgwin, 0, (4 + 2*category)*8, 
      unisys->alloc.rpm->category[category], color );
    FLDPLAINMSGWIN_PrintStrBufColor( unisys->fldmsgwin, 14*8, (4 + 2*category)*8, 
      unisys->alloc.rpm->prerequisite[category], color );
  }
}

//==================================================================
/**
 * NGレギュレーション：描画完了待ち
 *
 * @param   unisys		
 * @param   fieldWork		
 *
 * @retval  BOOL		TRUE:描画完了　FALSE:描画中
 */
//==================================================================
BOOL UnionMsg_Menu_RegulationWait(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  PRINT_QUE *print_que = FLDMSGBG_GetPrintQue( fldmsg_bg );

  if(unisys->alloc.rpm == NULL){
    return TRUE;
  }

  return PRINTSYS_QUE_IsFinished( print_que );
}

//==================================================================
/**
 * NGレギュレーション：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_RegulationDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->alloc.rpm == NULL){
    return;
  }
  
//  _WordsetDelete(unisys);
  
  FLDPLAINMSGWIN_Delete(unisys->fldmsgwin);
  unisys->fldmsgwin = NULL;

  PokeRegulation_DeletePrintMsg(unisys->alloc.rpm);
  unisys->alloc.rpm = NULL;
}

