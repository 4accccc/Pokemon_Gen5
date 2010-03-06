//==============================================================================
/**
 * @file    symbol_save_field.h
 * @brief   シンボルエンカウント用セーブデータでフィールド上でしか使用しないもの(FIELDMAPオーバーレイに配置)
 * @author  matsuda
 * @date    2010.03.06(土)
 */
//==============================================================================
#pragma once


//==============================================================================
//  外部関数宣言
//==============================================================================
extern u32 SymbolSave_Field_CheckKeepZoneSpace(SYMBOL_SAVE_WORK *symbol_save, SYMBOL_ZONE_TYPE zone_type);
extern void SymbolSave_Field_Move_FreeToKeep(SYMBOL_SAVE_WORK *symbol_save, u32 now_no);
extern void SymbolSave_Field_Move_KeepToFree(SYMBOL_SAVE_WORK *symbol_save, u32 keep_no);

