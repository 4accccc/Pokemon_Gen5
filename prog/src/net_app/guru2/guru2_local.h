//==============================================================================
/**
 * @file    guru2_local.h
 * @brief   ぐるぐる交換　ローカル用ヘッダーファイル
 * @author  Akito Mori(移植） / kagaya 
 * @data    2010.01.20
 */
//==============================================================================
#ifndef GURU2_LH_FILE
#define GURU2_LH_FILE

#include <gflib.h>
#include "net_app/union_app.h"
#include "net_app/guru2.h"
#include "app/pokelist.h"
#include "app/p_status.h"


#include "guru2_comm.h"
#include "guru2.naix"

//==============================================================================
//  define
//==============================================================================
//--------------------------------------------------------------
//  debug
//--------------------------------------------------------------
//----
#ifdef PM_DEBUG

#ifdef DEBUG_ONLY_FOR_kagaya
//#define GURU2_DEBUG_ON          //定義でデバッグ機能ON
#endif

#ifdef GURU2_DEBUG_ON
//#define DEBUG_GAME_SEC (10)     //定義でゲーム時間変更
//#define DEBUG_DISP_CHECK        //定義でカメラ、皿の画面チェック
#endif

#endif
//----

#define GURU2_RC_MCCELL_OFF

//--------------------------------------------------------------
//  
//--------------------------------------------------------------
#define NUM_FX32(a) (FX32_ONE*(a))      ///<整数->FX32型に
#define FX32_NUM(a) ((a)/FX32_ONE)      ///<FX32->整数に

//--------------------------------------------------------------
//  ヒープ
//--------------------------------------------------------------
#define GURU2_HEAPSIZE  (0x80000)     //<ヒープサイズ

//--------------------------------------------------------------
/// GURU2RET
//--------------------------------------------------------------
typedef enum
{
  GURU2RET_NON,
  GURU2RET_NEXT,
  GURU2RET_END,
}GURU2RET;

//--------------------------------------------------------------
/// ぐるぐる交換モード
//--------------------------------------------------------------
enum
{
  GURU2MODE_RECEIPT,
  GURU2MODE_POKESEL,
  GURU2MODE_POKESEL_CANCEL,
  GURU2MODE_GAME_MAIN,
};

//--------------------------------------------------------------
/// ぐるぐる通信接続状態
//--------------------------------------------------------------
enum
{
  GURU2CONNECT_ON = 0,    //接続状態
  GURU2CONNECT_CUT_EXE,   //切断中
  GURU2CONNECT_CUT,     //切断完了
  GURU2CONNECT_TIMING,    //同期中
  GURU2CONNECT_MAX,     //最大
};

//--------------------------------------------------------------
/// ぐるぐる通信シーケンス
//--------------------------------------------------------------
enum
{
  GURU2CONNECTGFL_PROC_NON = 0, //特に無し
  GURU2CONNECTGFL_PROC_TIMING,  //同期処理
  GURU2CONNECTGFL_PROC_CUT,   //切断
  GURU2CONNECTGFL_PROC_MAX,   //最大
};

//==============================================================================
//  typedef
//==============================================================================
///ゲームメイン用ワーク
typedef struct GURU2MAIN_WORK GURU2MAIN_WORK;
///受付用ワーク
typedef struct GURU2RC_WORK GURU2RC_WORK;

//--------------------------------------------------------------
/// GURU2_PROC_WORK
//--------------------------------------------------------------
struct GURU2PROC_WORK
{
  int guru2_mode;
  int trade_no;
  int receipt_num;    //受付人数
  u32 receipt_bit;    //受付ビット
  BOOL receipt_ret;
  
  GURU2_PARAM param;
  GURU2COMM_WORK *g2c;
  GURU2MAIN_WORK *g2m;
  GURU2RC_WORK *g2r;
};

//==============================================================================
//  extern
//==============================================================================
/* guru2_main.c */
extern void Guru2Main_CommButtonSet( GURU2MAIN_WORK *g2m, u32 btn );
extern void Guru2Main_CommGameDataSet(
    GURU2MAIN_WORK *g2m, const GURU2COMM_GAMEDATA *data );
extern void Guru2Main_CommPlayNoDataSet(
    GURU2MAIN_WORK *g2m, const GURU2COMM_PLAYNO *no );
extern void Guru2Main_CommPlayMaxSet( GURU2MAIN_WORK *g2m, int max );
extern void Guru2Main_TradeNoSet( GURU2MAIN_WORK *g2m, int id, int no );
extern void Guru2Main_GameResultSet(
  GURU2MAIN_WORK *g2m, const GURU2COMM_GAMERESULT *result );

#endif //GURU2_LH_FILE
