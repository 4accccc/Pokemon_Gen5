//==============================================================================
/**
 * @file    battle_rec.h
 * @brief   戦闘録画セーブデータのヘッダ
 * @author  matsuda
 * @date    2009.09.01(火)
 */
//==============================================================================
#pragma once

#include "savedata/gds_local_common.h"
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

#define HEADER_FORM_NO_MASK ( 0x7f )      ///<フォルムナンバーを取り出すためのマスク値
#define HEADER_GENDER_MASK  ( 0x80 )      ///<性別を取り出すためのマスク値
#define HEADER_GENDER_SHIFT ( 7 )         ///<性別を取り出すためのシフト値

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
///バトルモード   ※BattleRecModeBitTblと並びを同じにしておくこと！！
typedef enum{
  //コロシアム：シングル
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE,    //コロシアム　シングル　制限無し
  BATTLE_MODE_COLOSSEUM_SINGLE_50,      //コロシアム　シングル　フラット
  BATTLE_MODE_COLOSSEUM_SINGLE_FREE_SHOOTER,  //コロシアム　シングル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_SINGLE_50_SHOOTER,    //コロシアム　シングル　フラット　シューターあり
  //コロシアム：ダブル
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE,          //コロシアム　ダブル　制限無し
  BATTLE_MODE_COLOSSEUM_DOUBLE_50,            //コロシアム　ダブル　フラット
  BATTLE_MODE_COLOSSEUM_DOUBLE_FREE_SHOOTER,  //コロシアム　ダブル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_DOUBLE_50_SHOOTER,    //コロシアム　ダブル　フラット　シューターあり
  //コロシアム：トリプル
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE,          //コロシアム　トリプル　制限無し
  BATTLE_MODE_COLOSSEUM_TRIPLE_50,            //コロシアム　トリプル　フラット
  BATTLE_MODE_COLOSSEUM_TRIPLE_FREE_SHOOTER,  //コロシアム　トリプル　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_TRIPLE_50_SHOOTER,    //コロシアム　トリプル　フラット　シューターあり
  //コロシアム：ローテーション
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE,        //コロシアム　ローテーション　制限無し
  BATTLE_MODE_COLOSSEUM_ROTATION_50,          //コロシアム　ローテーション　フラット
  BATTLE_MODE_COLOSSEUM_ROTATION_FREE_SHOOTER,  //コロシアム　ローテ　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_ROTATION_50_SHOOTER,    //コロシアム　ローテ　フラット　シューターあり
  //コロシアム：マルチ
  BATTLE_MODE_COLOSSEUM_MULTI_FREE,           //コロシアム　マルチ　制限無し
  BATTLE_MODE_COLOSSEUM_MULTI_50,             //コロシアム　マルチ　フラット
  BATTLE_MODE_COLOSSEUM_MULTI_FREE_SHOOTER,   //コロシアム　マルチ　制限無し　シューターあり
  BATTLE_MODE_COLOSSEUM_MULTI_50_SHOOTER,     //コロシアム　マルチ　フラット　シューターあり
  //地下鉄
  BATTLE_MODE_SUBWAY_SINGLE,     //WIFI DL含む      地下鉄　シングル
  BATTLE_MODE_SUBWAY_DOUBLE,                      //地下鉄　ダブル
  BATTLE_MODE_SUBWAY_MULTI,      //NPC, COMM, WIFI  地下鉄　マルチ
  //ランダムマッチ：フリー
  BATTLE_MODE_RANDOM_FREE_SINGLE,             //ランダムマッチ　フリー　シングル
  BATTLE_MODE_RANDOM_FREE_DOUBLE,             //ランダムマッチ　フリー　ダブル
  BATTLE_MODE_RANDOM_FREE_TRIPLE,             //ランダムマッチ　フリー　トリプル
  BATTLE_MODE_RANDOM_FREE_ROTATION,           //ランダムマッチ　フリー　ローテーション
  BATTLE_MODE_RANDOM_FREE_SHOOTER,            //ランダムマッチ　フリー　シューターバトル
  //ランダムマッチ：レーティング
  BATTLE_MODE_RANDOM_RATING_SINGLE,           //ランダムマッチ　レーティング　シングル
  BATTLE_MODE_RANDOM_RATING_DOUBLE,           //ランダムマッチ　レーティング　ダブル
  BATTLE_MODE_RANDOM_RATING_TRIPLE,           //ランダムマッチ　レーティング　トリプル
  BATTLE_MODE_RANDOM_RATING_ROTATION,         //ランダムマッチ　レーティング　ローテーション
  BATTLE_MODE_RANDOM_RATING_SHOOTER,          //ランダムマッチ　レーティング　シューターバトル
  //大会
  BATTLE_MODE_COMPETITION_SINGLE,             //大会バトル　シングル　シューター無し
  BATTLE_MODE_COMPETITION_SINGLE_SHOOTER,     //大会バトル　シングル　シューターあり
  BATTLE_MODE_COMPETITION_DOUBLE,             //大会バトル　ダブル　シューター無し
  BATTLE_MODE_COMPETITION_DOUBLE_SHOOTER,     //大会バトル　ダブル　シューターあり
  BATTLE_MODE_COMPETITION_TRIPLE,             //大会バトル　トリプル　シューター無し
  BATTLE_MODE_COMPETITION_TRIPLE_SHOOTER,     //大会バトル　トリプル　シューターあり
  BATTLE_MODE_COMPETITION_ROTATION,           //大会バトル　ローテーション　シューター無し
  BATTLE_MODE_COMPETITION_ROTATION_SHOOTER,   //大会バトル　ローテーション　シューターあり
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
#define RECHEAD_IDX_GENDER      (6)   ///<性別
#define RECHEAD_IDX_SERVER_VER  (7)   ///<サーバーバージョン



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
extern BOOL BattleRec_DataSetCheck(void);
extern void BattleRec_DataClear(void);
extern void BattleRec_DataDecoded(void);
extern void BattleRec_DataCoded(void);

extern BOOL BattleRec_Load( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num );
extern BOOL BattleRec_DataOccCheck(SAVE_CONTROL_WORK *sv,HEAPID heapID,LOAD_RESULT *result,int num);
extern SAVE_RESULT BattleRec_Save(GAMEDATA *gamedata, HEAPID heap_id, BATTLE_MODE rec_mode, int fight_count, int num, u16 *work0, u16 *work1);
extern void BattleRec_ClientTemotiGet(u16 mode_bit, int *client_max, int *temoti_max);
extern GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void);
extern BATTLE_REC_WORK_PTR BattleRec_WorkPtrGet(void);
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGet(void);
extern BTLREC_OPERATION_BUFFER* BattleRec_GetOperationBufferPtr( void );

extern void BattleRec_ExitWork(BATTLE_REC_SAVEDATA *wk_brs);
extern BATTLE_REC_SAVEDATA * BattleRec_LoadAlloc( SAVE_CONTROL_WORK *sv, HEAPID heapID, LOAD_RESULT *result, int num );
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGetWork(BATTLE_REC_SAVEDATA *wk_brs);

extern void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, BATTLE_REC_WORK_PTR rec, SAVE_CONTROL_WORK *sv);
extern u64 RecHeader_ParamGet(BATTLE_REC_HEADER_PTR head, int index, int param);
extern BOOL BattleRec_ServerVersionCheck( u8 version );
extern void BattleRec_CalcCrcRec( BATTLE_REC_WORK_PTR rec );

//==============================================================================
//  BattleRec_LoadToolModuleを行わないと使えない関数（battle_rec_tool.オーバーレイに配置！）
//==============================================================================
extern void BattleRec_LoadToolModule( void );
extern void BattleRec_UnloadToolModule( void );
extern void BattleRec_StoreSetupParam( const BATTLE_SETUP_PARAM* setup );
extern void BattleRec_RestoreSetupParam( BATTLE_SETUP_PARAM* setup, HEAPID heapID );

//==============================================================================
//  一部関数をbattle_rec_ov.cに移動した為、外部公開する必要になったもの
//==============================================================================
extern SAVE_RESULT Local_BattleRecSave(GAMEDATA *gamedata, BATTLE_REC_SAVEDATA *work, int num, u16 *seq, HEAPID heap_id);
extern void BattleRec_Coded(void *data,u32 size,u32 code);

//==============================================================================
//  battl_rec_ov.c
//      ※battle_recorderオーバーレイ領域に配置されているので注意!
//==============================================================================
extern SAVE_RESULT BattleRec_GDS_RecvData_Save(GAMEDATA *gamedata, int num, u8 secure, u16 *work0, u16 *work1, HEAPID heap_id);
extern void BattleRec_GDS_SendData_Conv(SAVE_CONTROL_WORK *sv);
extern SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(GAMEDATA *gamedata, u64 data_number, u16 *work0, u16 *work1, HEAPID heap_id);
extern SAVE_RESULT BattleRec_SecureSetSave(GAMEDATA *gamedata, int num, u16 *work0, u16 *work1, HEAPID heap_id);
extern void BattleRec_SaveDataEraseStart(GAMEDATA *gamedata, HEAPID heap_id, int num);
extern SAVE_RESULT BattleRec_SaveDataEraseMain(GAMEDATA *gamedata, int num);

extern void * BattleRec_RecWorkAdrsGet( void );

#ifdef PM_DEBUG
extern void DEBUG_BattleRec_SecureFlagSet(SAVE_CONTROL_WORK *sv);
#endif

