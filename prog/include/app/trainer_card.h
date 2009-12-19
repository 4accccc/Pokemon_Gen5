#ifndef _TRAINER_CARD_H_
#define _TRAINER_CARD_H_

#include "gamesystem/game_data.h"
#include "buflen.h"
#include "savedata/playtime.h"
#include "system/pms_data.h"

#define SIGN_SIZE_X (24)
#define SIGN_SIZE_Y (8)

#define COUNT_LV0 (100)
#define COUNT_LV1 (140)
#define COUNT_LV2 (170)
#define COUNT_LV3 (190)
#define COUNT_LV4 (200)

#define UNION_TR_NONE (0xff)

typedef enum{
  TR_CARD_RANK_NORMAL = 0,
  TR_CARD_RANK_BRO,
  TR_CARD_RANK_KAP,
  TR_CARD_RANK_SIL,
  TR_CARD_RANK_GOL,
  TR_CARD_RANK_BLACK,
}TR_CARD_RANK;

typedef struct TR_CARD_DATA_tag
{
  u8 Version;     //バージョン
  u8 CountryCode;   //国コード
  u8 GymReaderMask; //ジムリーダー８人顔マスクフラグ
  u8 CardRank;    //カードランク
  
  u8 TimeUpdate:1;  //時間更新フラグ
  u8 TrSex:1;     //性別
  u8 PokeBookFlg:1; //図鑑所持フラグ
  u8 MySignValid:1; //サインデータ有効/無効フラグ
  u8 SignAnimeOn:1; // サインデータはアニメしているか？（0:してない　1:してる）
  u8 Padding:3;
  u8 UnionTrNo;   //ユニオントレーナーナンバー（0～15）指定無しのときはUNION_TR_NONE(0xff)

  u16 BadgeFlag;    //バッジ入手フラグ(16bit)
  
  STRCODE TrainerName[PERSON_NAME_SIZE+EOM_SIZE]; //トレーナー名
  const PLAYTIME *PlayTime; //プレイ時間構造体（通信時は時間更新が行われないのでNULLをセットする）
  
  u32 Money;      //おこづかい
  u32 PokeBook;   //図鑑モンスター数
  u32 Score;      //スコア
  u16 TrainerID;    //トレーナーID
  
  u16 PlayTime_h;   //プレイ時間（時）
  u16 ClearTime_h;  //クリア時間（時）
  
  u8 PlayTime_m;    //プレイ時間（分）
  u8 Start_y;     //開始年
  u8 Start_m;     //開始月
  u8 Start_d;     //開始日
  
  u8 Clear_y;     //クリア年
  u8 Clear_m;     //クリア月
  u8 Clear_d;     //クリア日
  u8 ClearTime_m; //クリア時間（分）
  
  u16 CommNum;        //通信回数
  u16 CommBattleNum;  // 
  u16 CommBattleWin;  //対戦勝ち数
  u16 CommBattleLose; //対戦負け数
  u32 Personality;    //性格
  u32 CommTrade;      //交換回数
  PMS_DATA Pms;       //トレーナーカード内簡易会話
  
  u8  SignRawData[SIGN_SIZE_X*SIGN_SIZE_Y*8]; //サインデータ
  u16 _xor;     // 検証用
  u16 reached;    // 到着フラグ(通信時に使用）
}TR_CARD_DATA;

typedef struct TRCARD_CALL_PARAM_tag{
  TR_CARD_DATA    *TrCardData;
  GAMEDATA   *gameData;
  int         value;    ///<リターン値
}TRCARD_CALL_PARAM;

FS_EXTERN_OVERLAY(trainercard);
#define TRCARD_OVERLAY_ID (FS_OVERLAY_ID(trainercard))

//データの取得
extern void TRAINERCARD_GetSelfData( TR_CARD_DATA *cardData , GAMEDATA *gameData , const BOOL isSendData );


//通信用と自分用のCallParam作成
TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_SelfData( GAMEDATA *gameData , HEAPID heapId );
TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_CommData( GAMEDATA *gameData , void* pCardData , HEAPID heapId );

extern const GFL_PROC_DATA TrCardSysProcData;
//初期化が違う通信用Procデータ
extern const GFL_PROC_DATA TrCardSysCommProcData;

#endif //_TRAINER_CARD_H_
