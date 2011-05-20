//======================================================================
/**
 * @file	plist_demo.c
 * @brief	ポケモンリスト：フォルムチェンジ演出
 * @author	ariizumi
 * @data	10/02/22
 *
 * モジュール名：PLIST_DEMO
 */
//======================================================================
#pragma once

#include "plist_local_def.h"
#include "savedata/randommap_save.h"  //キュレム合体相手のPP保存のため
#include "waza_tool/wazano_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

typedef enum{
  KYUREMU_CHECK_OK, //フォルムチェンジOK
  KYUREMU_CHECK_NG, //NGデフォルトコード
  KYUREMU_CHECK_NG_DETH,  //NGひんし
}KYUREMU_CHECK_CODE;

typedef enum{
  KYUREMU_UNION_RESIRAMU, //レシラムと合体
  KYUREMU_UNION_ZEKUROMU, //ゼクロムと合体
  KYUREMU_UNION_NG_POKE,  //レシゼクでないのでNG
  KYUREMU_UNION_NG_DETH,  //瀕死なのでNG
  KYUREMU_UNION_NG_EGG,   //タマゴはNG
}KYUREMU_UNION_CHECK;

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
extern void PLIST_DEMO_DemoInit( PLIST_WORK *work );
extern void PLIST_DEMO_DemoTerm( PLIST_WORK *work );
extern void PLIST_DEMO_DemoMain( PLIST_WORK *work );

//本当はDEMOじゃないけど、ここにおく。
extern const BOOL PLIST_DEMO_CheckGirathnaToOrigin( PLIST_WORK *work , POKEMON_PARAM *pp );
extern void PLIST_DEMO_ChangeGirathinaToOrigin( PLIST_WORK *work , POKEMON_PARAM *pp );
extern const BOOL PLIST_DEMO_CheckGirathnaToAnother( PLIST_WORK *work , POKEMON_PARAM *pp );
extern void PLIST_DEMO_ChangeGirathinaToAnother( PLIST_WORK *work , POKEMON_PARAM *pp );
extern const BOOL PLIST_DEMO_CheckSheimiToSky( PLIST_WORK *work , POKEMON_PARAM *pp );
extern void PLIST_DEMO_ChangeSheimiToSky( PLIST_WORK *work , POKEMON_PARAM *pp );
extern const KYUREMU_CHECK_CODE PLIST_DEMO_CheckKyuremuToColor( PLIST_WORK *work , POKEMON_PARAM *pp );
extern const BOOL PLIST_DEMO_CheckKyuremuToNormal( PLIST_WORK *work , POKEMON_PARAM *pp );
extern void PLIST_DEMO_ChangeKyuremuColor( PLIST_WORK *work, KYUREMU_UNION_CHECK mode, POKEMON_PARAM* pp_k, POKEMON_PARAM* pp_t );
extern void PLIST_DEMO_ChangeKyuremuNormal( PLIST_WORK *work, KYUREMU_UNION_CHECK mode, POKEMON_PARAM* pp_k );

