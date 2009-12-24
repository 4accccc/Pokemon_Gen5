//============================================================================================
/**
 * @file	move_pokemon.h
 * @brief	ˆÚ“®ƒ|ƒPƒ‚ƒ“
 * @author	Saito -> Iwasawa GAME FREAK inc.
 * @date	2006.05.12-09.12.14
 */
//============================================================================================
#pragma once

#include "gamesystem/game_data.h"
#include "savedata/save_control.h"
#include "savedata/encount_sv.h"
#include "field/move_pokemon_def.h"

extern void MP_JumpMovePokemonAll(ENC_SV_PTR inEncData);
extern void MP_JumpMovePokemon(ENC_SV_PTR inEncData, const u8 inTarget);
extern void MP_MovePokemonNeighboring(ENC_SV_PTR inEncData);
extern void MP_AddMovePoke( GAMEDATA* gdata, const u8 inTargetPoke);
extern void MP_RecoverMovePoke( GAMEDATA* gdata );
extern u16 MP_GetMovePokeZoneID( ENC_SV_PTR inEncData, const u8 inTarget);
extern BOOL MP_CheckMovePoke(ENC_SV_PTR inEncData);
extern void MP_UpdatePlayerZoneHist(ENC_SV_PTR inEncData, const int inZone);
extern void MP_SetAfterBattle(GAMEDATA * gdata, BATTLE_SETUP_PARAM *bsp);
extern u16 MP_CheckMovePokeWeather( GAMEDATA * gdata, u16 zone_id );

extern u8 MP_MonsNoToMovePokeID(u16 monsno);

#ifdef PM_DEBUG
extern void MP_DebugUpdateData(	ENC_SV_PTR data, const u8 inTargetPoke, const u8  inZoneIdx);
#endif

