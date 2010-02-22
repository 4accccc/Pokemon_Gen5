//============================================================================================
/**
 * @file  field_diving_data.h
 * @brief ダイビング接続先チェック用データとそのアクセス関数
 * @date  2010.02.21
 * @author  tamada GAME FREAK inc.
 */
//============================================================================================
#pragma once

#include <gflib.h>

#include "field/fieldmap_proc.h"
//-----------------------------------------------------------------------------
/**
 * @brief ダイビング判定処理
 * @param fieldmap  FIELDMAP_WORKへのポインタ
 * @param zone_id   遷移先マップIDを受け取るためのポインタ
 * @return  BOOL  TRUEのとき、ダイビング可能
 */
//-----------------------------------------------------------------------------
extern BOOL DIVINGSPOT_Check( FIELDMAP_WORK * fieldmap, u16 * zone_id );

