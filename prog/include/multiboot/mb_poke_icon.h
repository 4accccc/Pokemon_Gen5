//======================================================================
/**
 * @file	mb_poke_icon.h
 * @brief	マルチブート・ポケアイコン表示(子機は刺さっているROMからデータを抜く
 * @author	ariizumi
 * @data	09/11/20
 *
 * モジュール名：MB_ICON
 */
//======================================================================
#pragma once
#include "multiboot/mb_data_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern ARCHANDLE* MB_ICON_GetArcHandle( HEAPID heapId , const DLPLAY_CARD_TYPE cardType );
extern u32 MB_ICON_GetPltResId( const DLPLAY_CARD_TYPE cardType );
extern u32 MB_ICON_GetCharResId( POKEMON_PASO_PARAM *ppp , const DLPLAY_CARD_TYPE cardType );
extern u32 MB_ICON_GetCellResId( const DLPLAY_CARD_TYPE cardType );
extern u32 MB_ICON_GetAnmResId( const DLPLAY_CARD_TYPE cardType );
extern const u8 MB_ICON_GetPalNumber( POKEMON_PASO_PARAM *ppp , const DLPLAY_CARD_TYPE cardType );

