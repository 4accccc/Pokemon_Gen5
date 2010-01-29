
//============================================================================================
/**
 * @file	btlv_field.h
 * @brief	戦闘画面描画用関数（フィールド）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================

#pragma once

#include "battle/battle.h"

typedef enum
{ 
  BTLV_FIELD_VANISH_OFF = 0,
  BTLV_FIELD_VANISH_ON,
}BTLV_FIELD_VANISH;

typedef struct _BTLV_FIELD_WORK BTLV_FIELD_WORK;

extern  BTLV_FIELD_WORK*  BTLV_FIELD_Init( BtlRule rule, int index, u8 season, HEAPID heapID );
extern	void              BTLV_FIELD_Exit( BTLV_FIELD_WORK *bsw );
extern	void              BTLV_FIELD_Main( BTLV_FIELD_WORK *bsw );
extern	void              BTLV_FIELD_Draw( BTLV_FIELD_WORK *bsw );
extern  void              BTLV_FIELD_SetPaletteFade( BTLV_FIELD_WORK *bfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb );
extern  BOOL              BTLV_FIELD_CheckExecutePaletteFade( BTLV_FIELD_WORK* bfw );
extern  void              BTLV_FIELD_SetVanishFlag( BTLV_FIELD_WORK* bfw, BTLV_FIELD_VANISH flag );

