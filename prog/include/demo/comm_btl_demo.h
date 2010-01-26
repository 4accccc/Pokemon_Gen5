//============================================================================
/**
 *
 *	@file		comm_battle_demo.h
 *	@brief  通信バトルデモ 
 *	@author hosaka genya
 *	@data		2010.01.20
 *
 */
//============================================================================
#pragma once

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//--------------------------------------------------------------
///	通信バトルデモ 起動タイプ
//==============================================================
typedef enum
{ 
  COMM_BTL_DEMO_TYPE_NORMAL_START,  ///< ノーマル開始
  COMM_BTL_DEMO_TYPE_NORMAL_END,    ///< ノーマル終了
  COMM_BTL_DEMO_TYPE_MULTI_START,   ///< マルチバトル開始
  COMM_BTL_DEMO_TYPE_MULTI_END,     ///< マルチバトル終了
} COMM_BTL_DEMO_TYPE;

//--------------------------------------------------------------
/// バトルの勝敗
//==============================================================
typedef enum {
  COMM_BTL_DEMO_RESULT_WIN = 0, ///< 勝ち
  COMM_BTL_DEMO_RESULT_LOSE,    ///< 負け
  COMM_BTL_DEMO_RESULT_DRAW,    ///< 引き分け
  COMM_BTL_DEMO_RESULT_MAX,
} COMM_BTL_DEMO_RESULT;

//--------------------------------------------------------------
///	トレーナーID配列のID
//==============================================================
enum
{ 
  COMM_BTL_DEMO_TRDATA_A = 0,    ///< マルチバトルの時、ユニオンルームで左上の位置 / 通常時は自分
  COMM_BTL_DEMO_TRDATA_B,        ///< マルチバトルの時、ユニオンルームで左下の位置 / 通常時は相手
  COMM_BTL_DEMO_TRDATA_C,        ///< マルチバトルの時、ユニオンルームで右上の位置
  COMM_BTL_DEMO_TRDATA_D,        ///< マルチバトルの時、ユニオンルームで右下の位置

  COMM_BTL_DEMO_TRDATA_MAX,
};

#define TRAINER_NAME_BUF_LEN ( PERSON_NAME_SIZE*2+EOM_SIZE ) ///< プレイヤー名BUFの長さ

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//--------------------------------------------------------------
///	トレーナー毎のデータ
//==============================================================
typedef struct {
  const POKEPARTY* party; ///< 所有ポケモンデータ
  //@TODO STRCODEかSTRBUFか決める
  STRCODE trname[ PERSON_NAME_SIZE + EOM_SIZE ];  // 16
  STRBUF* str_trname;     ///< トレーナー名
  u8 trsex;               ///< トレーナーの性別:PM_MALE or PM_FEMALE
  u8 server_version;      ///< ROMのサーババージョン
  u8 padding[2];
} COMM_BTL_DEMO_TRAINER_DATA;

//--------------------------------------------------------------
///	初期化パラメータ
//==============================================================
typedef struct {
  // [IN]
  COMM_BTL_DEMO_TYPE type;      ///< 通信バトルデモ 起動タイプ
  COMM_BTL_DEMO_RESULT result;  ///< バトルの勝敗
  COMM_BTL_DEMO_TRAINER_DATA trainer_data[ COMM_BTL_DEMO_TRDATA_MAX ];  ///< トレーナー毎のデータ
} COMM_BTL_DEMO_PARAM;

//=============================================================================
/**
 *								EXTERN宣言
 */
//=============================================================================

FS_EXTERN_OVERLAY(comm_btl_demo);
extern const GFL_PROC_DATA CommBtlDemoProcData;

