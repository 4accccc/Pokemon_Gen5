//======================================================================
/**
 * @file  field_encount.h
 * @brief	フィールド　エンカウント処理
 * @author kagaya
 */
//======================================================================
#pragma once
#include <gflib.h>
#include "system/gfl_use.h"
#include "battle/battle.h"
#include "field/fieldmap_proc.h"

#include "field/encount_data.h"   //ENCOUNT_TYPE
#include "field/field_wfbc.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  proto
//======================================================================
///FIELD_ENCOUNT
typedef struct _TAG_FIELD_ENCOUNT FIELD_ENCOUNT;
typedef struct _TAG_ENCOUNT_WORK ENCOUNT_WORK;

//======================================================================
//  extern
//======================================================================
//--------------------------------------------------------------
/**
 * @brief   ENCOUNT_WORK取得
 * @param   gamedata	GAMEDATAへのポインタ
 * @return  ENCOUNT_WORK*
 *
 * 実態はsrc/gamesystem/game_data.cにあるが、情報隠蔽を
 * 考慮してアクセス関数群を扱うヘッダに配置している
 */
//--------------------------------------------------------------
extern ENCOUNT_WORK* GAMEDATA_GetEncountWork( GAMEDATA *gamedata );

/////////////////////////////////////////////////////////////
//通常エンカウント系 field_encount.c

//生成、削除
extern FIELD_ENCOUNT * FIELD_ENCOUNT_Create( FIELDMAP_WORK *fwork );
extern void FIELD_ENCOUNT_Delete( FIELD_ENCOUNT *enc );
extern ENCOUNT_WORK * ENCOUNT_WORK_Create( HEAPID heapID );
extern void ENCOUNT_WORK_Delete( ENCOUNT_WORK* wp );

//チェック
extern void* FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type );
extern void* FIELD_ENCOUNT_SetWildEncount( FIELD_ENCOUNT *enc, u16 mons_no, u8 mons_lv, u16 flags );
extern void* FIELD_ENCOUNT_CheckFishingEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type );
extern void* FIELD_ENCOUNT_CheckWfbcEncount( FIELD_ENCOUNT *enc, const FIELD_WFBC* cp_wfbcdata );

extern void FIELD_ENCOUNT_SetTrainerBattleParam(
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup, int partner_id, int tr_id0, int tr_id1, HEAPID heapID );


/////////////////////////////////////////////////////////////
//エフェクトエンカウント系 effect_encount.c
extern void EFFECT_ENC_CheckEffectEncountStart( FIELD_ENCOUNT* enc);


