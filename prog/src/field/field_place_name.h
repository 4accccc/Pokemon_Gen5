////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @file   field_place_name.h
 * @brief  地名表示ウィンドウ
 * @author obata_toshihiro
 * @data   
 */
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>

//------------------------------------------------------------------------------
/**
 * データ型宣言
 */
//------------------------------------------------------------------------------
typedef struct _FIELD_PLACE_NAME FIELD_PLACE_NAME;


//------------------------------------------------------------------------------
/**
 * プロトタイプ宣言
 */
//------------------------------------------------------------------------------
extern FIELD_PLACE_NAME* FIELD_PLACE_NAME_Create( HEAPID heap_id );
extern void FIELD_PLACE_NAME_Process( FIELD_PLACE_NAME* p_sys );
extern void FIELD_PLACE_NAME_Draw( FIELD_PLACE_NAME* p_sys );
extern void FIELD_PLACE_NAME_Destroy( FIELD_PLACE_NAME* p_sys );
