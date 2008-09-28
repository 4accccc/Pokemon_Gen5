//============================================================================================
/**
 * @file	field_actor.h
 * @date	2008.9.29
 * @brief	（仮）フィールドアクター制御
 */
//============================================================================================
#pragma once

#include "field_common.h"


typedef struct _FLD_ACTCONT		FLD_ACTCONT;
extern FLD_ACTCONT*		FLD_CreateFieldActSys( FIELD_SETUP* gs, HEAPID heapID );
extern void				FLD_DeleteFieldActSys( FLD_ACTCONT* fldActCont );
extern void				FLD_MainFieldActSys( FLD_ACTCONT* fldActCont );

