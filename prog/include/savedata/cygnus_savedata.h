//============================================================================================
/**
 * @file    cygnus_savedata.h
 * @brief   3DS連動：swan-cygnus　データ交換領域
 * @date    2012.01.16
 * @author  tamada
 */
//============================================================================================
#pragma once

#include <gflib.h>

//----------------------------------------------------------
/// CYGNUSセーブデータの状態
//----------------------------------------------------------
typedef enum {
  CYGSAVE_STATE_NON = 0,  ///<データがない
  CYGSAVE_STATE_SEND,     ///<CYGNUSから書き込み可能
  CYGSAVE_STATE_RECEIVE,  ///<SWANで受け取り可能
}CYGSAVE_STATE;

//----------------------------------------------------------
//----------------------------------------------------------
enum {
  MAGICNUMBER_SP_POKE01 = 0x80808080,
  MAGICNUMBER_SP_POKE02,
  MAGICNUMBER_SP_POKE03,
  MAGICNUMBER_SP_POKE04,
  MAGICNUMBER_SP_POKE05,
  MAGICNUMBER_SP_POKE06,
  MAGICNUMBER_SP_POKE07,
  MAGICNUMBER_SP_POKE08,
};

//----------------------------------------------------------
//----------------------------------------------------------
enum {
  CYGDATA_SP_POKE01 = 0,
  CYGDATA_SP_POKE02,
  CYGDATA_SP_POKE03,
  CYGDATA_SP_POKE04,
  CYGDATA_SP_POKE05,
  CYGDATA_SP_POKE06,
  CYGDATA_SP_POKE07,
  CYGDATA_SP_POKE08,

  CYGDATA_POKE01,
  CYGDATA_POKE02,
  CYGDATA_POKE03,
  CYGDATA_POKE04,
  CYGDATA_POKE05,
  CYGDATA_POKE06,

  CYGDATA_ITEMID01,
  CYGDATA_ITEMID02,
  CYGDATA_ITEMID03,
  CYGDATA_ITEMID04,
  CYGDATA_ITEMID05,
  CYGDATA_ITEMID06,

  CYGDATA_ITEMNUM01,
  CYGDATA_ITEMNUM02,
  CYGDATA_ITEMNUM03,
  CYGDATA_ITEMNUM04,
  CYGDATA_ITEMNUM05,
  CYGDATA_ITEMNUM06,

  CYGDATA_CRC = 31,
  CYGDATA_AREA_SIZE = 32
};

//----------------------------------------------------------
/// CYGNUSセーブデータの位置
//----------------------------------------------------------
enum { CYGSAVE_FLASH_ADRS = 0x7f000 };

//----------------------------------------------------------
//----------------------------------------------------------
struct _CYGNUS_SAVEDATA {
  u32 overwrite_enable_flag;          ///<swanRW  cyg RW  CYGNUSからの書き込みを許可するフラグ
  u32 onetime_key;                    ///<swanRW  cyg RO  暗号化キー（毎回変化させること）
  u32 magic_number;                   ///<swanRW  cyg RO  マジックナンバー
  u32 flags;                          ///<swanRW  cyg RO  受け取り状況フラグ

  u32 data_area[ CYGDATA_AREA_SIZE ]; ///<swanRO  cyg WO  実データ領域
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

//----------------------------------------------------------
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_GetStatus( const CYGNUS_SAVEDATA * sv );

//----------------------------------------------------------
//----------------------------------------------------------
extern void CYGNUS_SAVEDATA_SetSend( CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 flags );


//----------------------------------------------------------
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_Receive(
    const CYGNUS_SAVEDATA * sv, u32 onetime_key, u32 * rec_buf );


#ifdef  PM_DEBUG
//----------------------------------------------------------
//----------------------------------------------------------
extern CYGSAVE_STATE CYGNUS_SAVEDATA_DEBUG_Write( CYGNUS_SAVEDATA * sv, u32 * rec_buf );

//----------------------------------------------------------
//----------------------------------------------------------
extern u32 CYGNUS_SAVEDATA_DEBUG_GetFlags( const CYGNUS_SAVEDATA * sv );

#endif

