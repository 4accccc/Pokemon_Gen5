//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		scrcmd_wfbc.c
 *	@brief  WFBC用スクリプトコマンド郡
 *	@author	tomoya takahashi
 *	@date		2009.12.17
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc/arc_def.h"
#include "arc/script_message.naix"

#include "print/wordset.h"

#include "field/field_msgbg.h"

#include "field_comm/intrude_work.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "event_field_fade.h"

#include "fieldmap.h"
#include "field_wfbc.h"
#include "fldmmdl.h"
#include "field_status_local.h"

#include "scrcmd_wfbc.h"

#include "scrcmd_wfbc_define.h" 

#include "msg/msg_place_name.h"  // for MAPNAME_xxxx
#include "msg/script/msg_plc10.h"  
#include "msg/script/msg_bc10.h"  
#include "msg/script/msg_wc10.h"  

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
