//============================================================================================
/**
 * @file  trainer_card.h
 * @bfief トレーナーカードアプリ呼出用ワーク
 * @author  Akito Mori / Nozomu Saito (WB移植は有泉くん)
 * @date  10.03.09
 */
//============================================================================================
#ifndef _TRAINER_CARD_H_
#define _TRAINER_CARD_H_

#include "gamesystem/game_data.h"
#include "buflen.h"
#include "savedata/playtime.h"
#include "system/pms_data.h"

#define SIGN_SIZE_X (24)  ///< トレーナーサインの書き込み領域開始Ｘ
#define SIGN_SIZE_Y ( 8)  ///< トレーナーサインの書き込み領域開始Ｙ

#define TRAINERCARD_NEXTPROC_RETURN   ( 0 ) ///< フィールドメニューに戻る
#define TRAINERCARD_NEXTPROC_EXIT     ( 1 ) ///< フィールドに直接戻る

///< ショートカット起動用定義
#define TRCARD_SHORTCUT_NONE   ( 0 )  ///< 指定なし
#define TRCARD_SHORTCUT_FRONT  ( 1 )  ///< 表
#define TRCARD_SHORTCUT_BACK   ( 2 )  ///< 裏
#define TRCARD_SHORTCUT_BADGE  ( 3 )  ///< バッジ画面
  
//================================================================================
/**! @enum TR_CARD_RANK
  *        トレーナーカードのランク順のカラー指定
  */
//================================================================================
typedef enum{
  TR_CARD_RANK_NORMAL = 0,
  TR_CARD_RANK_BRO,
  TR_CARD_RANK_KAP,
  TR_CARD_RANK_SIL,
  TR_CARD_RANK_GOL,
  TR_CARD_RANK_BLACK,
}TR_CARD_RANK;

//================================================================================
/**! @typedef TR_CARD_DATA
  *           トレーナーカード表示データ構造体。自分のトレーナーカード閲覧の
  *           場合は情報収集を行った結果が入る。通信の場合はこのワークがまるごと
  *           送信されてくるのでこのデータでトレーナーカードを表示する
  */
//================================================================================
typedef struct TR_CARD_DATA_tag
{
  u8 Version;       ///< バージョン
  u8 CountryCode;   ///< 国コード
  u8 GymReaderMask; ///< ジムリーダー８人顔マスクフラグ
  u8 CardRank;      ///< カードランク
  
  u8 TimeUpdate:1;  ///< 時間更新フラグ
  u8 TrSex:1;       ///< 性別
  u8 PokeBookFlg:1; ///< 図鑑所持フラグ
  u8 MySignValid:1; ///< サインデータ有効/無効フラグ
  u8 SignAnimeOn:1; ///<  サインデータはアニメしているか？（0:してない　1:してる）
  u8 SignDisenable:1; ///< サインを書けなくする？（0:サインは書ける 1:書け無くする ）
  u8 EditPossible:1;  ///< 編集可能か？（TRUE:OK　FALSE:NG)
  u8 OtherTrCard:1;   ///< 他人のカードを見ている（TRUE:他人のカード FALSE：自分のカード)
  u8 Padding:1;
  u8 UnionTrNo;     ///< ユニオントレーナーナンバー（0～15）指定無しのときはUNION_TR_NONE(0xff)

  u16 BadgeFlag;    ///< バッジ入手フラグ(16bit)
  
  STRCODE TrainerName[PERSON_NAME_SIZE+EOM_SIZE]; ///< トレーナー名
  const PLAYTIME *PlayTime; ///< プレイ時間構造体（通信時は時間更新が行われないのでNULLをセットする）
  
  u32 Money;        ///< おこづかい
  u32 PokeBook;     ///< 図鑑モンスター数
  u16 TrainerID;    ///< トレーナーID
  
  u16 PlayTime_h;   ///< プレイ時間（時）
  u16 ClearTime_h;  ///< クリア時間（時）
  
  u8 PlayTime_m;    ///< プレイ時間（分）
  u8 Start_y;       ///< 開始年
  u8 Start_m;       ///< 開始月
  u8 Start_d;       ///< 開始日
  
  u8 Clear_y;       ///< クリア年
  u8 Clear_m;       ///< クリア月
  u8 Clear_d;       ///< クリア日
  u8 ClearTime_m;   ///< クリア時間（分）
  
  u16 CommNum;        ///< 通信回数
  u16 CommBattleNum;  ///< 通信対戦回数 
  u16 CommBattleWin;  ///< 対戦勝ち数
  u16 CommBattleLose; ///< 対戦負け数
  u32 Personality;    ///< 性格
  u32 CommTrade;      ///< 交換回数
  
  u32 EncountNum;        ///< 野生エンカウント回数
  u32 TrainerEncountNum; ///< トレーナー戦をした回数
  u32 SuretigaiNum;      ///< すれ違い通信をした回数
  u16 FeelingCheckNum;   ///< フィーリングチェックをした回数
  u16 gPowerNum;         ///< 発動できる持っているデルパワーの回数
  u16 MusicalNum;        ///< ミュージカルをした回数
  u16 PokeshifterHigh;   ///< ポケシフターのハイスコア
  u16 TrialHouseHigh;    ///< トライアウハウスの最高得点
  u16 TrialHouseRank;    ///< トライアルハウスのランク

  u32 CgearGetFlag:1;    ///< Cギアを持っているか
  u32 PaleceFlag:1;      ///< パレスに行った事があるか
  u32 MusicalFlag:1;     ///< ミュージカルをした事があるか
  u32 PokeshifterFlag:1; ///< ポケシフターをした事があるか
  u32 TrianHouseFlag:1;  ///< トライアルハウスに参加した事があるか
  u32 Padding2:27;       


  PMS_DATA Pms;       ///< トレーナーカード内簡易会話
  
  u8  SignRawData[SIGN_SIZE_X*SIGN_SIZE_Y*8]; ///< サインデータ
  u16 _xor;         ///< 検証用xor
  u16 reached;      ///< 到着フラグ(通信時に使用）
}TR_CARD_DATA;

//================================================================================
/**! @typedef TRCARD_CALL_PARAM
  *           トレーナーカード呼び出しワーク。トレーナーカード画面に
  *           PROC切り替えする際に必要。
  */          
//================================================================================
typedef struct TRCARD_CALL_PARAM_tag{
  TR_CARD_DATA  *TrCardData;    ///< トレーナーカードデータが入っている場合のポインタ(通信中など）
  GAMEDATA      *gameData;      ///< ゲームデータアクセス用ポインタ
  BOOL          edit_possible;  ///< 編集可能か？（TRUE:可能・FALSE：不可能）
  int           value;          ///< リターン値
  int           next_proc;      ///< トレーナーカード終了時の戻り値
  int           mode;           ///< ショートカットからの呼び出し( TRCARD_SHORTCUT_NONE,
                                ///< TRCARD_SHORTCUT_FRONT, TRCARD_SHORTCUT_BACK, TRCARD_SHORTCUT_BADGE )
}TRCARD_CALL_PARAM;

FS_EXTERN_OVERLAY(trainercard);
#define TRCARD_OVERLAY_ID (FS_OVERLAY_ID(trainercard))

extern void TRAINERCARD_GetSelfData( 
            TR_CARD_DATA *cardData , GAMEDATA *gameData , const BOOL isSendData, BOOL edit, HEAPID heapId );
extern TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_SelfData( GAMEDATA *gameData , HEAPID heapId, BOOL edit );
extern TRCARD_CALL_PARAM* TRAINERCASR_CreateCallParam_CommData( GAMEDATA *gameData , void* pCardData , HEAPID heapId );
extern const GFL_PROC_DATA TrCardSysProcData;
extern const GFL_PROC_DATA TrCardSysCommProcData;

#endif //_TRAINER_CARD_H_
