//======================================================================
/**
 *
 * @file  mb_data_pt.c 
 * @brief wbダウンロードプレイ　プラチナ用機種別コード
 * @author  ariizumi
 * @data  09/11/20
 */
//======================================================================

#pragma once

extern BOOL MB_DATA_PT_LoadData( MB_DATA_WORK *dataWork );
extern BOOL MB_DATA_PT_SaveData( MB_DATA_WORK *dataWork );

extern void* MB_DATA_PT_GetBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx );
extern u16*  MB_DATA_PT_GetBoxName( MB_DATA_WORK *dataWork , const u8 tray );
extern void  MB_DATA_PT_ClearBoxPPP( MB_DATA_WORK *dataWork , const u8 tray , const u8 idx );
