//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		event_fieldmap_control_local.h
 *  @brief	イベント：フィールドマップ制御ツール  event_mapchange用関数
 *	@author	tomoya takahashi
 *	@date		2010.01.12
 *
 *	モジュール名：
 *
 *	event_mapchangeの
 *	field_procが動作する前に使用するフィールド用のソースを
 *	常駐からはずすために、『FIELD_INIT』オーバーレイを作成した。
 *
 *	gamesystem 『常駐』
 *	  event_mapchange 『FIELD_INIT』
 *	    field_proc      『FIELDMAP』
 *
 *	通常のEVENT_FieldOpen、EVENT_FieldCloseはFIELD_INITとFIELDMAP両方の
 *	読み込み、破棄を行うが、
 *  event_mapchangeでは、『FIELDMAP』のみの読み込み破棄でよいので、
 *  専用の関数を用意する。
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef _cplusplus
extern "C"{
#endif


//============================================================================================
// event_mapchange内限定のFieldOpen Close
//============================================================================================
extern GMEVENT * EVENT_FieldOpen_FieldProcOnly(GAMESYS_WORK *gsys);
extern GMEVENT * EVENT_FieldClose_FieldProcOnly(GAMESYS_WORK *gsys, FIELDMAP_WORK * fieldmap);


#ifdef _cplusplus
}	// extern "C"{
#endif



