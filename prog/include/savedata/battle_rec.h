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
#include "net_app/gds/gds_battle_rec_sub.h"

//==============================================================================
//  定数定義
//==============================================================================
enum {
  BTLREC_OPERATION_BUFFER_SIZE = 0xc00,   ///< 全クライアントの操作内容保持バッファサイズ
};



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
///バトルタワー / シングル
#define RECMODE_TOWER_SINGLE    (GT_BATTLE_MODE_TOWER_SINGLE)
///バトルタワー / ダブル
#define RECMODE_TOWER_DOUBLE    (GT_BATTLE_MODE_TOWER_DOUBLE)
///バトルタワー / WIFIダウンロード
#define RECMODE_TOWER_WIFI_DL   (GT_BATTLE_MODE_TOWER_SINGLE)

///バトルファクトリー / シングル
#define RECMODE_FACTORY_SINGLE    (GT_BATTLE_MODE_FACTORY50_SINGLE)
///バトルファクトリー / ダブル
#define RECMODE_FACTORY_DOUBLE    (GT_BATTLE_MODE_FACTORY50_DOUBLE)

///バトルファクトリー / シングル100
#define RECMODE_FACTORY_SINGLE100 (GT_BATTLE_MODE_FACTORY100_SINGLE)
///バトルファクトリー / ダブル100
#define RECMODE_FACTORY_DOUBLE100 (GT_BATTLE_MODE_FACTORY100_DOUBLE)

///バトルステージ / シングル
#define RECMODE_STAGE_SINGLE    (GT_BATTLE_MODE_STAGE_SINGLE)
///バトルステージ / ダブル
#define RECMODE_STAGE_DOUBLE    (GT_BATTLE_MODE_STAGE_DOUBLE)

///バトルキャッスル / シングル
#define RECMODE_CASTLE_SINGLE   (GT_BATTLE_MODE_CASTLE_SINGLE)
///バトルキャッスル / ダブル
#define RECMODE_CASTLE_DOUBLE   (GT_BATTLE_MODE_CASTLE_DOUBLE)

///バトルルーレット / シングル
#define RECMODE_ROULETTE_SINGLE   (GT_BATTLE_MODE_ROULETTE_SINGLE)
///バトルルーレット / ダブル
#define RECMODE_ROULETTE_DOUBLE   (GT_BATTLE_MODE_ROULETTE_DOUBLE)

///コロシアム(通信対戦) / シングル(制限無し)
#define RECMODE_COLOSSEUM_SINGLE      (GT_BATTLE_MODE_COLOSSEUM_SINGLE)
///コロシアム(通信対戦) / シングル(スタンダードカップ)
#define RECMODE_COLOSSEUM_SINGLE_STANDARD (GT_BATTLE_MODE_COLOSSEUM_SINGLE_STANDARD)
///コロシアム(通信対戦) / シングル(ファンシーカップ)
#define RECMODE_COLOSSEUM_SINGLE_FANCY    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_FANCY)
///コロシアム(通信対戦) / シングル(リトルカップ)
#define RECMODE_COLOSSEUM_SINGLE_LITTLE   (GT_BATTLE_MODE_COLOSSEUM_SINGLE_LITTLE)
///コロシアム(通信対戦) / シングル(ライトカップ)
#define RECMODE_COLOSSEUM_SINGLE_LIGHT    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_LIGHT)
///コロシアム(通信対戦) / シングル(ダブルカップ)
#define RECMODE_COLOSSEUM_SINGLE_DOUBLE   (GT_BATTLE_MODE_COLOSSEUM_SINGLE_DOUBLE)
///コロシアム(通信対戦) / シングル(その他のカップ)
#define RECMODE_COLOSSEUM_SINGLE_ETC    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_ETC)

///コロシアム(通信対戦) / ダブル
#define RECMODE_COLOSSEUM_DOUBLE      (GT_BATTLE_MODE_COLOSSEUM_DOUBLE)
///コロシアム(通信対戦) / ダブル(スタンダードカップ)
#define RECMODE_COLOSSEUM_DOUBLE_STANDARD (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_STANDARD)
///コロシアム(通信対戦) / ダブル(ファンシーカップ)
#define RECMODE_COLOSSEUM_DOUBLE_FANCY    (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_FANCY)
///コロシアム(通信対戦) / ダブル(リトルカップ)
#define RECMODE_COLOSSEUM_DOUBLE_LITTLE   (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_LITTLE)
///コロシアム(通信対戦) / ダブル(ライトカップ)
#define RECMODE_COLOSSEUM_DOUBLE_LIGHT    (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_LIGHT)
///コロシアム(通信対戦) / ダブル(ダブルカップ)
#define RECMODE_COLOSSEUM_DOUBLE_DOUBLE   (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_DOUBLE)
///コロシアム(通信対戦) / ダブル(その他のカップ)
#define RECMODE_COLOSSEUM_DOUBLE_ETC    (GT_BATTLE_MODE_COLOSSEUM_DOUBLE_ETC)

///コロシアム(通信対戦) / ミックス(制限無し)
#define RECMODE_COLOSSEUM_MIX       (GT_BATTLE_MODE_COLOSSEUM_SINGLE)
///コロシアム(通信対戦) / ミックス(スタンダードカップ)
#define RECMODE_COLOSSEUM_MIX_STANDARD    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_STANDARD)
///コロシアム(通信対戦) / ミックス(ファンシーカップ)
#define RECMODE_COLOSSEUM_MIX_FANCY     (GT_BATTLE_MODE_COLOSSEUM_SINGLE_FANCY)
///コロシアム(通信対戦) / ミックス(リトルカップ)
#define RECMODE_COLOSSEUM_MIX_LITTLE    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_LITTLE)
///コロシアム(通信対戦) / ミックス(ライトカップ)
#define RECMODE_COLOSSEUM_MIX_LIGHT     (GT_BATTLE_MODE_COLOSSEUM_SINGLE_LIGHT)
///コロシアム(通信対戦) / ミックス(ダブルカップ)
#define RECMODE_COLOSSEUM_MIX_DOUBLE    (GT_BATTLE_MODE_COLOSSEUM_SINGLE_DOUBLE)
///コロシアム(通信対戦) / ミックス(その他のカップ)
#define RECMODE_COLOSSEUM_MIX_ETC     (GT_BATTLE_MODE_COLOSSEUM_SINGLE_ETC)

//以下、マルチモード
#define RECMODE_TOWER_MULTI     (GT_BATTLE_MODE_TOWER_MULTI)  ///<バトルタワー / マルチ
#define RECMODE_FACTORY_MULTI   (GT_BATTLE_MODE_FACTORY50_MULTI)  ///<バトルファクトリー / マルチ
#define RECMODE_FACTORY_MULTI100  (GT_BATTLE_MODE_FACTORY100_MULTI)///<バトルファクトリー / マルチ100
#define RECMODE_STAGE_MULTI     (GT_BATTLE_MODE_STAGE_MULTI)  ///<バトルステージ / マルチ
#define RECMODE_CASTLE_MULTI    (GT_BATTLE_MODE_CASTLE_MULTI) ///<バトルキャッスル / マルチ
#define RECMODE_ROULETTE_MULTI    (GT_BATTLE_MODE_ROULETTE_MULTI) ///<バトルルーレット / マルチ
#define RECMODE_COLOSSEUM_MULTI   (GT_BATTLE_MODE_COLOSSEUM_MULTI)///<コロシアム(通信対戦) / マルチ

//終端
#define RECMODE_MAX           (GT_BATTLE_MODE_ROULETTE_MULTI + 1)


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
extern void BattleRec_ClientTemotiGet(int rec_mode, int *client_max, int *temoti_max);
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

