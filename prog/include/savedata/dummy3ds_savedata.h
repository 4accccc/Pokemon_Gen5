
//============================================================================================
/**
 * @file    dummy3ds_savedata.h
 * @brief   3DS連動：内部確保領域
 * @date    2012.01.16
 * @author  tamada
 *
 */
//============================================================================================
#pragma once

//----------------------------------------------------------
/**
 * @brief DUMMY3DS_SAVEDATAへの不完全型定義
 */
//----------------------------------------------------------
typedef struct _DUMMY3DS_SAVEDATA DUMMY3DS_SAVEDATA;


//----------------------------------------------------------
//----------------------------------------------------------
extern int DUMMY3DS_SAVEDATA_GetWorkSize( void );

//----------------------------------------------------------
//----------------------------------------------------------
extern void DUMMY3DS_SAVEDATA_Init( DUMMY3DS_SAVEDATA * sv );

typedef enum
{
  DUMMY3DS_SAVEDATA_PARAM_IS_PLAYED,
  DUMMY3DS_SAVEDATA_PARAM_ONETIME_KEY,
  DUMMY3DS_SAVEDATA_PARAM_FLAG,
}DUMMY3DS_SAVEDATA_PARAM;
extern u32 DUMMY3DS_SAVEDATA_GetData( const DUMMY3DS_SAVEDATA * sv, DUMMY3DS_SAVEDATA_PARAM param );
extern void DUMMY3DS_SAVEDATA_SetData( DUMMY3DS_SAVEDATA * sv, DUMMY3DS_SAVEDATA_PARAM param, u32 data );

extern DUMMY3DS_SAVEDATA * SaveData_GetDummy3ds( SAVE_CONTROL_WORK * p_sv );
