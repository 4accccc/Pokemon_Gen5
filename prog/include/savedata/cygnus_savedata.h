//============================================================================================
/**
 * @file    cygnus_savedata.h
 * @brief   3DS連動：swan-cygnus　データ交換領域
 * @date    2012.01.16
 * @author  tamada
 *
 * ここに定義されているのはデータ構造のみ。
 * 実際のフラッシュアクセス関数などは別定義される。
 *
 */
//============================================================================================
//
#pragma once

#include <gflib.h>

//----------------------------------------------------------
/// CYGNUSセーブデータの状態
//----------------------------------------------------------
typedef enum {
  CYGSAVE_STATE_NON = 0,  ///<データがない(MAGICNUMBER/CTRLCRCの異常）
  CYGSAVE_STATE_SEND,     ///<CYGNUSから書き込み可能(OWFlag=TRUE)
  CYGSAVE_STATE_RECEIVE,  ///<SWANで受け取り可能(OWFlag=FALSE)
  CYGSAVE_STATE_BREAK,    ///<セーブ失敗で壊れている(DATACRCの異常）

  CYGSAVE_STATE_NUM,
}CYGSAVE_STATE;

//----------------------------------------------------------
/**
 * @brief CYGNUS送信データ領域のインデックス
 */
//----------------------------------------------------------
enum {
  CYGDATA_AREA_SIZE = 32    ///<領域最大長
};

//----------------------------------------------------------
/**
 * @brief CYGNUSセーブデータの位置
 *
 * 4Mフラッシュの末尾0x1000の領域を予約している。
 */
//----------------------------------------------------------
enum { CYGSAVE_FLASH_ADRS = 0x7f000 };


//----------------------------------------------------------
/**
 * @brief CYGNUS-SWAN 共有領域の定義
 */
//----------------------------------------------------------
struct _CYGNUS_SAVEDATA {
  u32 overwrite_enable_flag;          ///<swanRW  cyg RW  CYGNUSからの書き込みを許可するフラグ
  u32 onetime_key;                    ///<swanRW  cyg RO  暗号化キー（毎回変化させること）
  u32 magic_number;                   ///<swanRW  cyg RO  マジックナンバー
  u32 flags;                          ///<swanRW  cyg RO  受け取り状況フラグ
  u16 ctrl_crc;                       ///<swanRW  cyg RW  上記領域のためのCRC
  u16 pad1;                           ///< 4バイトアライメントのためのパディング

  u32 data_area[ CYGDATA_AREA_SIZE ]; ///<swanRO  cyg WO  実データ領域
  u16 data_crc;                       ///<swanRO  cyg WO  実データ領域のためのCRC
  u16 pad2;                           ///< 4バイトアライメントのためのパディング
};

//----------------------------------------------------------
/**
 * @brief CYGNUS_SAVEDATAへの不完全型定義
 */
//----------------------------------------------------------
typedef struct _CYGNUS_SAVEDATA CYGNUS_SAVEDATA;


//============================================================================================
//============================================================================================
//----------------------------------------------------------
//----------------------------------------------------------
extern int CYGNUS_SAVEDATA_GetWorkSize( void );

//----------------------------------------------------------
//----------------------------------------------------------
extern void CYGNUS_SAVEDATA_Init( CYGNUS_SAVEDATA * sv );


//============================================================================================
//============================================================================================
//----------------------------------------------------------
/**
 * @brief   セーブデータの状態を取得する
 * @param sv            CYGNUS_SAVEDATAへのポインタ
 * @return  CYGSAVE_STATE
 */
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_GetStatus( const CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
/**
 * @brief SWANからCYGNUS書き込み可能許可を設定する
 * @param sv            CYGNUS_SAVEDATAへのポインタ
 * @param onetime_key   暗号化に使用するワンタイムキー
 * @param flags         CYGNUSに伝える情報
 */
//----------------------------------------------------------
extern void CYGNUS_SAVEDATA_SetSend( CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 flags );


//----------------------------------------------------------
/**
 * @brief CYGNUSが書き込んだデータをデコード、SWANで受け取る
 * @param sv            CYGNUS_SAVEDATAへのポインタ
 * @param onetime_key   解読に使用するワンタイムキー
 * @param rec_buf       結果を受け取るためのバッファ（CYGDATA_AREA_SIZEの長さが必要）
 * @return  状態指定（CYGSAVE_STATEを参照）
 */
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_Receive(
    const CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 * rec_buf );


#ifdef  PM_DEBUG
//----------------------------------------------------------
/**
 * @brief デバッグ用：CYGNUSのふり：書き込み
 */
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_DEBUG_Write( CYGNUS_SAVEDATA * sv, u32 * rec_buf );

//----------------------------------------------------------
/**
 * @brief デバッグ用：CYGNUSのふり：情報フラグの取得
 */
//----------------------------------------------------------
extern u32 CYGNUS_SAVEDATA_DEBUG_GetFlags( const CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
/**
 * @brief デバッグ用：管理領域用CRCを破壊する
 */
//----------------------------------------------------------
extern void CYGNUS_SAVEDATA_DEBUG_BreakCtrlCRC( CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
/**
 * @brief デバッグ用：データ領域用CRCを破壊する
 */
//----------------------------------------------------------
extern void CYGNUS_SAVEDATA_DEBUG_BreakDataCRC( CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
/**
 * @brief デバッグ用：管理領域用CRCのチェック
 * @param BOOL  TRUEのとき、CRCは正しい
 */
//----------------------------------------------------------
extern BOOL CYGNUS_SAVEDATA_DEBUG_CheckCtrlCRC( CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
/**
 * @brief デバッグ用：データ域用CRCのチェック
 * @param BOOL  TRUEのとき、CRCは正しい
 */
//----------------------------------------------------------
extern BOOL CYGNUS_SAVEDATA_DEBUG_CheckDataCRC( CYGNUS_SAVEDATA * sv );
#endif

