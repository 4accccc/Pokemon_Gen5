//============================================================================================
/**
 * @file  pokewood_system.h
 * @brief フィールドでポケウッドを開始する際の処理
 * @author  mori GAME FREAK inc.
 * @date  2011.07.11
 *
 * @todo  ポケウッド状態の場合の数などの検討、スタジオ内で変化するNPCなど
 */
//============================================================================================
#pragma once

#include "field/pokewood.h"

#include "battle/battle.h"

#include "poke_tool/poke_regulation.h"  //REGULATION
#include "app/pokelist.h" //PLIST_DATA
#include "app/p_status.h" //PSTATUS_DATA


//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
//============================================================================================
//============================================================================================
extern PKWDSYS * PKWDSYS_Create( HEAPID heapID );
extern void PKWDSYS_Delete( PKWDSYS * pkwdsys );

//------------------------------------------------------------------
//------------------------------------------------------------------
//extern void PKWDSYS_GetOpponentName( PKWDSYS * pkwdsys, STRBUF * strbuf );
//extern u8 PKWDSYS_GetOpponentSex( PKWDSYS * pkwdsys );
//extern u16 PKWDSYS_GetOpponentMMdlObjCode( PKWDSYS * pkwdsys );

//============================================================================================
//============================================================================================
//------------------------------------------------------------------
//------------------------------------------------------------------
extern void PKWDSYS_SetupPokeListData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PLIST_DATA * plist_data );
extern void PKWDSYS_SetupPokeStatusData( PKWDSYS * pkwdsys, GAMEDATA * gamedata, PSTATUS_DATA * pstatus );

extern void PKWDSYS_SetPokelistResult( PKWDSYS * pkwdsys, PLIST_DATA * plist_data );
extern void PKWDSYS_SetScriptAnswerWork( PKWDSYS * pkwdsys, u16 * scr_answer_work );
extern u16* PKWDSYS_GetScriptAnswerWork( PKWDSYS * pkwdsys );


extern void PKWDSYS_SetJoinMypoke( PKWDSYS *pkwdsys, u16 flag );
extern  int PKWDSYS_GetJoinMypoke( PKWDSYS *pkwdsys );

extern void PKWDSYS_SetScenarioNo( PKWDSYS *pkwdsys, int no );
extern  u16 PKWDSYS_GetScenarioNo( PKWDSYS *pkwdsys );

extern void PKWDSYS_SetPlayNo( PKWDSYS *pkwdsys, int no );
extern  u16 PKWDSYS_GetPlayNo( PKWDSYS *pkwdsys );


extern void PKWDSYS_CreateBattleParty( PKWDSYS * pkwdsys, GAMEDATA * gamedata, POKEPARTY *entry_party );
//extern BATTLE_SETUP_PARAM * PKWDSYS_CreateBattleSetupParam( PKWDSYS * pkwdsys, GAMESYS_WORK * gsys );

//extern void PKWDSYS_ReflectBattleResult(
//    PKWDSYS * pkwdsys, GAMEDATA * gamedata, BATTLE_SETUP_PARAM * bsp );

