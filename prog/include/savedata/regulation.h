//============================================================================================
/**
 * @file	regulation.h
 * @brief	バトルレギュレーションデータアクセス用ヘッダ
 * @author	k.ohno
 * @date	2006.1.20
 */
//============================================================================================
#pragma once


#include <gflib.h>

//============================================================================================
//============================================================================================


#define WIFI_PLAYER_TIX_CUPNAME_MOJINUM (72) //デジタル選手証大会名文字列文字数
#define REGULATION_CUP_MAX (7)  //regulation.rbやregulation_cup.gmmと連動
#define REGULATION_RULE_MAX (6) //regulation.rbやregulation_rule.gmmと連動

//----------------------------------------------------------
/**
 * @brief	バトルレギュレーションデータ型定義
 */
//----------------------------------------------------------


typedef enum {
  REGULATION_CUPNAME_SIZE = 12,      ///< カップ名の長さ 12文字24バイト +EOM2byte
  REGULATION_RULENAME_SIZE = 12,      ///< ルール名の長さ 12文字24バイト +EOM2byte
  REGULATION_MAX_NUM = 1,   ///< １本保存可能
  REGULATION_VS_TIME = 99,        ///< 対戦時間MAX 99分
  REGULATION_COMMAND_TIME = 99,   ///< コマンド入力時間 99秒
  REGULATION_SHOW_POKE_TIME_MAX = 99,   ///< 見せ合い時間 99秒
  REGULATION_SHOW_POKE_TIME_NONE = 0,   ///< 見せ合い無制限
  REGULATION_AGE_NONE = 0,   ///< 年齢無制限

} REGULATION_ETC_PARAM_TYPE;

typedef enum  {
  REGULATION_CUPNUM,   ///< カップ番号
  REGULATION_RULENUM,  ///< ルール番号
  REGULATION_NUM_LO, ///<    #参加数下限
  REGULATION_NUM_HI, ///<    #参加数上限
  REGULATION_LEVEL,  ///<    #参加レベル
  REGULATION_LEVEL_RANGE, ///<    #レベル範囲
  REGULATION_LEVEL_TOTAL, ///<    #レベル合計
  REGULATION_BOTH_POKE, ///<    #同じポケモン
  REGULATION_BOTH_ITEM,  ///<    #同じどうぐ
  REGULATION_VETO_POKE_BIT,  ///<    #参加禁止ポケモン
  REGULATION_VETO_ITEM, ///<    #持ち込み禁止道具
  REGULATION_MUST_POKE,   ///<    #必須ポケモン
  REGULATION_MUST_POKE_FORM,  ///<  #必須ポケモンフォルム
  REGULATION_SHOOTER,    ///<    #シューター
  REGULATION_TIME_VS,     ///<    #対戦時間
  REGULATION_TIME_COMMAND, ///<    #入力時間
  REGULATION_NICKNAME, ///<    #ニックネーム表示
  REGULATION_AGE_LO,  ///<    #年齢制限以上
  REGULATION_AGE_HI,  ///<    #年齢制限以下
  REGULATION_SHOW_POKE,  ///<    #ポケモン見せ合いONOFF
  REGULATION_SHOW_POKE_TIME,  ///<    #ポケモン見せ合い制限時間 0は無制限
  REGULATION_BATTLETYPE,  ///<  バトルタイプ
} REGULATION_PARAM_TYPE;

typedef enum  {
  REGULATION_LEVEL_RANGE_NORMAL,   ///<なし
  REGULATION_LEVEL_RANGE_OVER,     ///<以上
  REGULATION_LEVEL_RANGE_LESS,     ///<以下
  REGULATION_LEVEL_RANGE_DRAG_DOWN,  ///<引き下げ
  REGULATION_LEVEL_RANGE_SAME,         ///<全補正
  REGULATION_LEVEL_RANGE_PULL_UP,     ///<引き上げ
  REGULATION_LEVEL_RANGE_MAX   //
} REGULATION_LEVEL_RANGE_TYPE;

typedef enum  {
  REGULATION_BATTLE_SINGLE,   ///<シングル
  REGULATION_BATTLE_DOUBLE,   ///<ダブル
  REGULATION_BATTLE_TRIPLE,   ///<トリプル
  REGULATION_BATTLE_ROTATION,   ///<ローテーション
  REGULATION_BATTLE_MULTI,   ///<マルチ
  REGULATION_BATTLE_MAX   //
} REGULATION_BATTLE_TYPE;

typedef enum {
  REGULATION_SHOOTER_INVALID,   ///<無効(なし)
  REGULATION_SHOOTER_VALID,     ///<有効(あり)
  REGULATION_SHOOTER_MANUAL,    ///<マニュアル
}REGULATION_SHOOTER_TYPE;



typedef enum  {
  REGULATION_CARD_CUPNAME,     ///< カップ名
  REGULATION_CARD_ROMVER,      ///< カセットバージョン：
  REGULATION_CARD_CUPNO,       ///< 大会No.
  REGULATION_CARD_LANGCODE,    ///< 国コード：
  REGULATION_CARD_START_YEAR,  ///< 開始年：00-99
  REGULATION_CARD_START_MONTH, ///< 開始月：01-12
  REGULATION_CARD_START_DAY,   ///< 開始日：01-31
  REGULATION_CARD_END_YEAR,    ///< 終了年：00-99
  REGULATION_CARD_END_MONTH,   ///< 終了月：01-12
  REGULATION_CARD_END_DAY,     ///< 終了日：01-31
  REGULATION_CARD_STATUS,      ///< 大会状態：net/dreamworld_netdata.hのDREAM_WORLD_MATCHUP_TYPEの値
} REGULATION_CARD_PARAM_TYPE;

//----------------------------------------------------------
/**
 * @brief	バトルレギュレーションデータ型定義  fushigi_data.h参照の為外部公開に
 */
//----------------------------------------------------------

#define REG_POKENUM_MAX_BYTE (656/8)  //##このくらいに増えるかも ８２バイト
#define REG_ITEMNUM_MAX_BYTE (608/8)  //##このくらいにふえるかも


typedef struct{
  u8 cupNo;  ///< 大会カップ番号
  u8 ruleNo; ///< 大会ルール番号
  u8 NUM_LO; ///<    #参加数下限
  u8 NUM_HI; ///<    #参加数上限
  u8 LEVEL;  ///<    #参加レベル
  u8 LEVEL_RANGE; ///<    #レベル範囲
  u16 LEVEL_TOTAL; ///<    #レベル合計
  u8 BOTH_POKE; ///<    #同じポケモン
  u8 BOTH_ITEM;  ///<    #同じどうぐ             60
  u8 VETO_POKE_BIT[REG_POKENUM_MAX_BYTE];  ///<    #参加禁止ポケモン 142
  u8 VETO_ITEM[REG_ITEMNUM_MAX_BYTE]; ///<    #持ち込み禁止道具 218
  u16 MUST_POKE;   ///<    #必須ポケモン
  u8 MUST_POKE_FORM;   ///<    #必須ポケモンフォルム
  u8 SHOOTER;    ///<    #シューター
  u8 TIME_VS;     ///<    #対戦時間
  u8 TIME_COMMAND; ///<    #入力時間
  u8 NICKNAME; ///<    #ニックネーム表示
  u8 AGE_LO;  ///<    #年齢制限以上
  u8 AGE_HI;  ///<    #年齢制限以下
  u8 SHOW_POKE;  ///<    #ポケモン見せ合い
  u8 SHOW_POKE_TIME;  ///<ポケモン見せ合い時間
  u8 BATTLE_TYPE;    ///< バトルタイプ  230
} REGULATION;


//選手証
typedef struct {
  REGULATION regulation_buff;
  u32 ver;//カセットバージョン：
  STRCODE cupname[WIFI_PLAYER_TIX_CUPNAME_MOJINUM + EOM_SIZE];  //大会名（72文字＋EOM）
  short no;//大会No.
  char code;//国コード：
  char start_year;//開始年：00-99
  char start_month;//開始月：01-12
  char start_day;//開始日：01-31
  char end_year;//終了年：00-99
  char end_month;//終了月：01-12
  char end_day;//終了日：01-31
  char status;  //大会状態： net/dreamworld_netdata.hのDREAM_WORLD_MATCHUP_TYPEの値
  u16 crc;  //整合性検査
 // short rating;//大会用レーティング
//  short rd;//大会用RD
} REGULATION_CARDDATA;




//============================================================================================
//============================================================================================
//----------------------------------------------------------
//	セーブデータシステムや通信で使用する関数
//----------------------------------------------------------
extern int Regulation_GetWorkSize(void);
extern int RegulationData_GetWorkSize(void);
extern REGULATION * Regulation_AllocWork(HEAPID heapID);
extern void Regulation_Copy(const REGULATION * from, REGULATION * to);
extern int Regulation_Cmp(const REGULATION* pCmp1,const REGULATION* pCmp2);

//----------------------------------------------------------
//	REGULATION操作のための関数
//----------------------------------------------------------
extern void Regulation_Init(REGULATION * my);

//カップ名
extern void Regulation_GetCupName(const REGULATION* pReg,STRBUF* pReturnCupName);
extern STRBUF* Regulation_CreateCupName(const REGULATION* pReg, HEAPID heapID);

//ルール名
extern void Regulation_GetRuleName(const REGULATION* pReg,STRBUF* pReturnRuleName);
extern STRBUF* Regulation_CreateRuleName(const REGULATION* pReg, HEAPID heapID);

extern int Regulation_GetParam(const REGULATION* pReg, REGULATION_PARAM_TYPE type);
extern void Regulation_SetParam(REGULATION * pReg, REGULATION_PARAM_TYPE type, int param);

extern BOOL Regulation_CheckParamBit(const REGULATION* pReg, REGULATION_PARAM_TYPE type, int no);


//----------------------------------------------------------
//	REGULATION_CARDDATA操作のための関数 デジタル選手証の事
//----------------------------------------------------------
extern void RegulationData_Init(REGULATION_CARDDATA * my);

//カップ名
extern void Regulation_GetCardCupName(const REGULATION_CARDDATA* pReg,STRBUF* pReturnCupName);
extern STRBUF* Regulation_CreateCardCupName(const REGULATION_CARDDATA* pReg, HEAPID heapID);
extern const STRCODE *Regulation_GetCardCupNamePointer( const REGULATION_CARDDATA* pReg );

extern int Regulation_GetCardParam(const REGULATION_CARDDATA* pReg, REGULATION_CARD_PARAM_TYPE type);
extern void Regulation_SetCardParam(REGULATION_CARDDATA * pReg, REGULATION_CARD_PARAM_TYPE type, int param);



//----------------------------------------------------------
//	セーブデータ取得のための関数
//----------------------------------------------------------
extern REGULATION* SaveData_GetRegulation(SAVE_CONTROL_WORK* pSave,int regNo);
extern void SaveData_SetRegulation(SAVE_CONTROL_WORK* pSave, const REGULATION* pReg, const int regNo);

extern REGULATION_CARDDATA* SaveData_GetRegulationCardData(SAVE_CONTROL_WORK* pSave);



//----------------------------------------------------------
//	デバッグ用
//----------------------------------------------------------
#ifdef PM_DEBUG
extern void Regulation_SetDebugData( REGULATION_CARDDATA* pReg );
#endif //PM_DEBUG
