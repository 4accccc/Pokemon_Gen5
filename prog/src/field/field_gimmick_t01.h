//============================================================================================
/**
 * @file	field_gimmick_t01.h
 * @brief	T01ギミック
 * @author	Saito
 */
//============================================================================================

#pragma once

extern void T01_GIMMICK_Setup( FIELDMAP_WORK* fieldmap );
extern void T01_GIMMICK_End( FIELDMAP_WORK* fieldmap );
extern void T01_GIMMICK_Move( FIELDMAP_WORK* fieldmap );

extern void T01_GIMMICK_Start( FIELDMAP_WORK* fieldmap );
extern GMEVENT * T01_GIMMICK_CreateEndChkEvt( GAMESYS_WORK *gsys );

extern void T01_GIMMICK_StartDisp( GAMESYS_WORK *gsys );   //NOTWIFIオーバーレイ

