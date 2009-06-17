//======================================================================
/**
 * @file	plist_plate.c
 * @brief	ポケモンリスト プレート
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST_PLATE
 */
//======================================================================
#pragma once

#include "plist_local_def.h"
#include "poke_tool\poke_tool.h"

//プレートの色の種類(そのままパレット番号に対応
typedef enum
{
  PPC_NORMAL        = 3,  //通常
  PPC_NORMAL_SELECT = 7,  //通常(選択
  PPC_CHANGE        = 6,  //入れ替え
}PLIST_PLATE_COLTYPE;

extern PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate( PLIST_WORK *work , const u8 idx , POKEMON_PARAM *pp );
extern PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate_Blank( PLIST_WORK *work , const u8 idx );

extern void PLIST_PLATE_DeletePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
extern void PLIST_PLATE_UpdatePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

extern void PLIST_PLATE_ChangeColor( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , PLIST_PLATE_COLTYPE col );

