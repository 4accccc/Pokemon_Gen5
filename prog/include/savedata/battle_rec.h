//==============================================================================
/**
 * @file    battle_rec.h
 * @brief   戦闘録画セーブデータのヘッダ
 * @author  matsuda
 * @date    2009.09.01(火)
 */
//==============================================================================
#pragma once

#include "savedata/gds_profile.h" //GDS_PROFILE_PTR定義
#include "battle/battle.h"

//==============================================================================
//  定数定義
//==============================================================================
enum {
  BTLREC_OPERATION_BUFFER_SIZE = 0xc00,   ///< 全クライアントの操作内容保持バッファサイズ
};

///録画ヘッダに格納するポケモン番号の最大要素数
#define HEADER_MONSNO_MAX   (12)

///データナンバーの型
typedef u64 DATA_NUMBER;

//ロードデータ用インデックス
#define LOADDATA_MYREC    (0)
#define LOADDATA_DOWNLOAD0  (1)
#define LOADDATA_DOWNLOAD1  (2)
#define LOADDATA_DOWNLOAD2  (3)

///録画データロード時の結果値
#define RECLOAD_RESULT_NULL   (0) ///<データなし
#define RECLOAD_RESULT_OK   (1) ///<正常読み込み
#define RECLOAD_RESULT_NG   (2) ///<データ不正
#define RECLOAD_RESULT_ERROR  (3) ///<ロードで読み出し失敗(セーブデータが破壊されている場合など)

#define REC_COUNTER_MAX     ( 9999 )  ///< 連勝記録最大値


//--------------------------------------------------------------
//  戦闘モード(施設一覧)
//--------------------------------------------------------------
///バトルモード
typedef enum{
  //コロシアム：シングル
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE,
  BATTLE_MODE_COLOSSEUM_SINGLE_STANDARD,
  BATTLE_MODE_COLOSSEUM_SINGLE_50,
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE_SHOOTER,
  BATTLE_MODE_COLOSSEUM_SINGLE_STANDARD_SHOOTER,
  BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER,
  //コロシアム：ダブル
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE,
  BATTLE_MODE_COLOSSEUM_DOUBLE_STANDARD,
  BATTLE_MODE_COLOSSEUM_DOUBLE_50,
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE_SHOOTER,
  BATTLE_MODE_COLOSSEUM_DOUBLE_STANDARD_SHOOTER,
  BATTLE_MODE_COLOSSEUM_DOUBLE_50_SHOOTER,
  //コロシアム：トリプル
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE,
  BATTLE_MODE_COLOSSEUM_TRIPLE_STANDARD,
  BATTLE_MODE_COLOSSEUM_TRIPLE_50,
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE_SHOOTER,
  BATTLE_MODE_COLOSSEUM_TRIPLE_STANDARD_SHOOTER,
  BATTLE_MODE_COLOSSEUM_TRIPLE_50_SHOOTER,
  //コロシアム：ローテーション
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE,
  BATTLE_MODE_COLOSSEUM_ROTATION_STANDARD,
  BATTLE_MODE_COLOSSEUM_ROTATION_50,
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE_SHOOTER,
  BATTLE_MODE_COLOSSEUM_ROTATION_STANDARD_SHOOTER,
  BATTLE_MODE_COLOSSEUM_ROTATION_50_SHOOTER,
  //コロシアム：マルチ
  BATTLE_MODE_COLOSSEUM_MULTI_FREE,
  BATTLE_MODE_COLOSSEUM_MULTI_STANDARD,
  BATTLE_MODE_COLOSSEUM_MULTI_50,
  BATTLE_MODE_COLOSSEUM_MULTI_FREE_SHOOTER,
  BATTLE_MODE_COLOSSEUM_MULTI_STANDARD_SHOOTER,
  BATTLE_MODE_COLOSSEUM_MULTI_50_SHOOTER,
  //地下鉄
  BATTLE_MODE_SUBWAY_SINGLE,     //WIFI DL含む
  BATTLE_MODE_SUBWAY_DOUBLE,
  BATTLE_MODE_SUBWAY_MULTI,      //NPC, COMM, WIFI
  //ランダムマッチ：フリー
  BATTLE_MODE_RANDOM_FREE_SINGLE,
  BATTLE_MODE_RANDOM_FREE_DOUBLE,
  BATTLE_MODE_RANDOM_FREE_TRIPLE,
  BATTLE_MODE_RANDOM_FREE_ROTATION,
  BATTLE_MODE_RANDOM_FREE_SHOOTER,
  //ランダムマッチ：レーティング
  BATTLE_MODE_RANDOM_RATING_SINGLE,
  BATTLE_MODE_RANDOM_RATING_DOUBLE,
  BATTLE_MODE_RANDOM_RATING_TRIPLE,
  BATTLE_MODE_RANDOM_RATING_ROTATION,
  BATTLE_MODE_RANDOM_RATING_SHOOTER,
  //大会
  BATTLE_MODE_COMPETITION_SINGLE,
  BATTLE_MODE_COMPETITION_DOUBLE,
  BATTLE_MODE_COMPETITION_TRIPLE,
  BATTLE_MODE_COMPETITION_ROTATION,
  BATTLE_MODE_COMPETITION_SINGLE_SHOOTER,
  BATTLE_MODE_COMPETITION_DOUBLE_SHOOTER,
  BATTLE_MODE_COMPETITION_TRIPLE_SHOOTER,
  BATTLE_MODE_COMPETITION_ROTATION_SHOOTER,

  //--------------------------------------------------------------
  //  複数検索
  //--------------------------------------------------------------
  BATTLE_MODE_COLOSSEUM_SINGLE_NO_SHOOTER = 0xef,    //コロシアム/シングル　シューター無し
  BATTLE_MODE_COLOSSEUM_SINGLE_SHOOTER = 0xf0,       //コロシアム/シングル　シューター有
  BATTLE_MODE_COLOSSEUM_DOUBLE_NO_SHOOTER = 0xf1,    //コロシアム/ダブル　シューター無し
  BATTLE_MODE_COLOSSEUM_DOUBLE_SHOOTER = 0xf2,       //コロシアム/ダブル　シューター有
  BATTLE_MODE_COLOSSEUM_TRIPLE_NO_SHOOTER = 0xf3,    //コロシアム/トリプル　シューター無し
  BATTLE_MODE_COLOSSEUM_TRIPLE_SHOOTER = 0xf4,       //コロシアム/トリプル　シューター有
  BATTLE_MODE_COLOSSEUM_ROTATION_NO_SHOOTER = 0xf5,  //コロシアム/ローテーション　シューター無し
  BATTLE_MODE_COLOSSEUM_ROTATION_SHOOTER = 0xf6,     //コロシアム/ローテーション　シューター有
  BATTLE_MODE_COLOSSEUM_MULTI_NO_SHOOTER = 0xf7,     //コロシアム/ローテーション　シューター無し
  BATTLE_MODE_COLOSSEUM_MULTI_SHOOTER = 0xf8,        //コロシアム/ローテーション　シューター有
  BATTLE_MODE_RANDOM_SINGLE = 0xf9,                  //ランダムマッチ/シングル
  BATTLE_MODE_RANDOM_DOUBLE = 0xfa,                  //ランダムマッチ/ダブル
  BATTLE_MODE_RANDOM_TRIPLE = 0xfb,                  //ランダムマッチ/トリプル
  BATTLE_MODE_RANDOM_ROTATION = 0xfc,                //ランダムマッチ/ローテーション
  BATTLE_MODE_RANDOM_SHOOTER = 0xfd,                 //ランダムマッチ/シューター
  BATTLE_MODE_COMPETITION = 0xfe,                    //バトル大会
  BATTLE_MODE_NONE = 0xff,                           //指定なし
}BATTLE_MODE;

//終端
#define BATTLE_MODE_MAX           (BATTLE_MODE_COMPETITION_ROTATION_SHOOTER + 1)


//--------------------------------------------------------------
//  録画ヘッダのパラメータ取得要INDEX
//--------------------------------------------------------------
#define RECHEAD_IDX_MONSNO      (0)   ///<ポケモン番号
#define RECHEAD_IDX_FORM_NO     (1)   ///<フォルム番号
#define RECHEAD_IDX_COUNTER     (2)   ///<何戦目か
#define RECHEAD_IDX_MODE      (3)   ///<録画施設
#define RECHEAD_IDX_DATA_NUMBER   (4)   ///<データナンバー
#define RECHEAD_IDX_SECURE      (5)   ///<安全視聴済み



//--------------------------------------------------------------
/**
 *  戦闘録画セーブデータの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_SAVEDATA   BATTLE_REC_SAVEDATA;

//--------------------------------------------------------------
/**
 *  戦闘録画本体の不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_WORK *   BATTLE_REC_WORK_PTR;

//--------------------------------------------------------------
/**
 *  戦闘録画ヘッダの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_HEADER *   BATTLE_REC_HEADER_PTR;



//==============================================================================
//  ※check　makeを通す為の暫定定義
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   戦闘開始パラメータ
 *      336 byte
 */
//--------------------------------------------------------------
typedef struct{
  u8 work[336];
}BATTLE_PARAM;

//----------------------------------------------------------
/**
 * @brief バトルレギュレーションデータ型定義  fushigi_data.h参照の為外部公開に
 */
//----------------------------------------------------------
typedef struct {
  STRCODE cupName[12];
  u16 totalLevel;
  u8 num;
  u8 level;
    s8 height;
    s8 weight;
    u8 evolution:1;    //
    u8 bLegend:1;
    u8 bBothMonster:1;
    u8 bBothItem:1;
    u8 bFixDamage:1;
}AAA_REGULATION;

//----------------------------------------------------------------------------
/**
  *  不完全型定義
 */
//----------------------------------------------------------------------------
typedef struct _BTLREC_OPERATION_BUFFER   BTLREC_OPERATION_BUFFER;  ///< クライアント操作内容保持バッファ
typedef struct _BTLREC_SETUP_SUBSET       BTLREC_SETUP_SUBSET;      ///< バトルセットアップパラメータのサブセット


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 BattleRec_GetWorkSize( void );
extern void BattleRec_WorkInit(void *rec);
extern void BattleRec_Init(HEAPID heapID);
extern void BattleRec_Exit(void);
extern BOOL BattleRec_DataExistCheck(void);
extern void * BattleRec_RecWorkAdrsGet( void );
extern BOOL BattleRec_Load( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num );
extern BOOL BattleRec_DataOccCheck(SAVE_CONTROL_WORK *sv,HEAPID heapID,LOAD_RESULT *result,int num);
extern SAVE_RESULT BattleRec_Save(SAVE_CONTROL_WORK *sv, HEAPID heap_id, int rec_mode, int fight_count, int num, u16 *work0, u16 *work1);
extern SAVE_RESULT BattleRec_SaveDataErase(SAVE_CONTROL_WORK *sv, HEAPID heap_id, int num);
extern void BattleRec_ClientTemotiGet(const BATTLE_REC_WORK_PTR rec, int *client_max, int *temoti_max);
extern void BattleRec_BattleParamRec(BATTLE_PARAM *bp);
extern void BattleRec_ServerVersionUpdate(int id_no, u32 server_version);
extern BOOL BattleRec_ServerVersionCheck(void);
extern void BattleRec_BattleParamCreate(BATTLE_PARAM *bp,SAVE_CONTROL_WORK *sv);
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderAllocCopy(HEAPID heap_id);
extern GDS_PROFILE_PTR BattleRec_GDSProfileAllocCopy(HEAPID heap_id);
extern GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void);
extern BATTLE_REC_WORK_PTR BattleRec_WorkPtrGet(void);
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGet(void);
extern BTLREC_OPERATION_BUFFER* BattleRec_GetOperationBufferPtr( void );
extern BTLREC_SETUP_SUBSET*     BattleRec_GetSetupSubsetPtr( void );

extern void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, BATTLE_REC_WORK_PTR rec, BATTLE_PARAM *bp, SAVE_CONTROL_WORK *sv);
extern u64 RecHeader_ParamGet(BATTLE_REC_HEADER_PTR head, int index, int param);
extern BATTLE_REC_HEADER_PTR BattleRec_Header_AllocMemory(HEAPID heap_id);
extern void BattleRec_Header_FreeMemory(BATTLE_REC_HEADER_PTR brhp);


extern void BattleRec_LoadToolModule( void );
extern void BattleRec_UnloadToolModule( void );
extern void BattleRec_StoreSetupParam( const BATTLE_SETUP_PARAM* setup );
extern void BattleRec_RestoreSetupParam( BATTLE_SETUP_PARAM* setup, HEAPID heapID );

//==============================================================================
//  一部関数をbattle_rec_ov.cに移動した為、外部公開する必要になったもの
//==============================================================================
extern SAVE_RESULT Local_BattleRecSave(SAVE_CONTROL_WORK *sv, BATTLE_REC_SAVEDATA *work, int num, u16 *seq, HEAPID heap_id);
extern void BattleRec_Coded(void *data,u32 size,u32 code);

//==============================================================================
//  battl_rec_ov.c
//      ※battle_recorderオーバーレイ領域に配置されているので注意!
//==============================================================================
extern void BattleRecTool_ErrorStrCheck(BATTLE_REC_SAVEDATA *src, BATTLE_PARAM *bp, int heap_id);
extern SAVE_RESULT BattleRec_GDS_RecvData_Save(SAVE_CONTROL_WORK *sv, int num, u8 secure, u16 *work0, u16 *work1);
extern void BattleRec_GDS_SendData_Conv(SAVE_CONTROL_WORK *sv);
extern SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(SAVE_CONTROL_WORK *sv, u64 data_number, u16 *work0, u16 *work1);
extern SAVE_RESULT BattleRec_SecureSetSave(SAVE_CONTROL_WORK *sv, int num);

#ifdef PM_DEBUG
extern void DEBUG_BattleRec_SecureFlagSet(SAVE_CONTROL_WORK *sv);
#endif

