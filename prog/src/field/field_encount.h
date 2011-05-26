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
//通常エンカウント系 field_encount_st.c

//生成、削除
extern FIELD_ENCOUNT * FIELD_ENCOUNT_Create( FIELDMAP_WORK *fwork );
extern void FIELD_ENCOUNT_Delete( FIELD_ENCOUNT *enc );
extern ENCOUNT_WORK * ENCOUNT_WORK_Create( HEAPID heapID );
extern void ENCOUNT_WORK_Delete( ENCOUNT_WORK* wp );

extern void ENCOUNT_WORK_MapWalkStepOverUpdate( GAMEDATA* gdata, FIELDMAP_WORK* fieldWork );
extern void ENCOUNT_WORK_SpMapJumpUpdate( GAMEDATA* gdata );
extern void ENCOUNT_WORK_MapJumpUpdate( GAMEDATA* gdata, FIELDMAP_WORK* fieldWork );
extern void ENCOUNT_WORK_EncountProbReset( ENCOUNT_WORK* ewk, MAPATTR attr );

/////////////////////////////////////////////////////////////
//通常エンカウント系 field_encount.c
//チェック
extern void FIELD_ENCOUNT_EncountProbCalc( FIELD_ENCOUNT* enc, BOOL move_f, BOOL ev_move_f );
extern void* FIELD_ENCOUNT_CheckEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type );
extern void* FIELD_ENCOUNT_SetWildEncount( FIELD_ENCOUNT *enc, u16 mons_no, u8 mons_lv, u8 formno ,u16 flags );
extern void* FIELD_ENCOUNT_CheckFishingEncount( FIELD_ENCOUNT *enc, ENCOUNT_TYPE enc_type );
extern void* FIELD_ENCOUNT_CheckWfbcEncount( FIELD_ENCOUNT *enc, const FIELD_WFBC* cp_wfbcdata );

extern void FIELD_ENCOUNT_SetTrainerBattleParam(
    FIELD_ENCOUNT *enc, BATTLE_SETUP_PARAM *setup, int rule, int partner_id, int tr_id0, int tr_id1, HEAPID heapID );

extern void BTL_FIELD_SITUATION_SetFromFieldStatus( BTL_FIELD_SITUATION* sit, GAMEDATA* gdata, FIELDMAP_WORK* fieldWork );

/////////////////////////////////////////////////////////////
//エフェクトエンカウント系 effect_encount.c
extern void EFFECT_ENC_CheckEffectEncountStart( FIELD_ENCOUNT* enc);

/////////////////////////////////////////////////////////////////////////
//デバッグ用ルーチン
#ifdef PM_DEBUG
#define FLDENC_DNI_NPOKE_ENC_PROB (255)

extern u32 FLDENC_DEB_NumInputNPokeGet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param );
extern void FLDENC_DEB_NumInputNPokeSet( GAMESYS_WORK * gsys, GAMEDATA * gamedata, u32 param, u32 value );

#endif  //PM_DEBUG

