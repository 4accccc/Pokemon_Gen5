//======================================================================
/**
 *
 * @file  mb_data_main.c
 * @brief セーブデータを読み込むところ
 * @author  ariizumi
 * @data  09/11/20
 */
//======================================================================

#pragma once

#include "multiboot/mb_data_def.h"

typedef struct _MB_DATA_WORK MB_DATA_WORK;  //DLPLAYDATAのDATA・・・ややこしい・・・

extern  MB_DATA_WORK* MB_DATA_InitSystem( int heapID );
extern  void        MB_DATA_TermSystem( MB_DATA_WORK *dataWork );

extern const  DLPLAY_CARD_TYPE MB_DATA_GetCardType( MB_DATA_WORK *dataWork );
extern void   MB_DATA_SetCardType( MB_DATA_WORK *dataWork , const DLPLAY_CARD_TYPE type );

extern  BOOL  MB_DATA_LoadDataFirst( MB_DATA_WORK *dataWork );
extern  BOOL  MB_DATA_SaveData( MB_DATA_WORK *dataWork );
extern  u8    MB_DATA_GetErrorState( MB_DATA_WORK *dataWork );

extern  BOOL  MB_DATA_IsFinishSaveFirst( MB_DATA_WORK *dataWork );
extern  BOOL  MB_DATA_IsFinishSaveSecond( MB_DATA_WORK *dataWork );
extern  BOOL  MB_DATA_IsFinishSaveAll( MB_DATA_WORK *dataWork );
extern  void  MB_DATA_PermitLastSaveFirst( MB_DATA_WORK *dataWork );
extern  void  MB_DATA_PermitLastSaveSecond( MB_DATA_WORK *dataWork );

