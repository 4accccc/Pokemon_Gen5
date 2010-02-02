//======================================================================
/**
 * @file	plist_sys.c
 * @brief	ポケモンリスト メイン処理
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST
 * モジュール名：PLIST_UTIL
 */
//======================================================================
#pragma once

#include "plist_local_def.h"
#include "print/wordset.h"

typedef enum
{
  LSCL_OK,       //覚えられる
  LSCL_OK_FULL,  //覚えられる(でも技いっぱい
  LSCL_NG,    //覚えられない
  LSCL_LEARN, //すでに覚えている
}PLIST_SKILL_CAN_LEARN;

extern const BOOL PLIST_InitPokeList( PLIST_WORK *work );
extern const BOOL PLIST_TermPokeList( PLIST_WORK *work );
extern const BOOL PLIST_UpdatePokeList( PLIST_WORK *work );

//--------------------------------------------------------------
//	指定技箇所にフィールド技があるか？
//  戻り値はPL_RET_***** 無いときは０
//--------------------------------------------------------------
extern u32 PLIST_UTIL_CheckFieldWaza( const POKEMON_PARAM *pp , const u8 wazaPos );
extern const BOOL PLIST_UTIL_IsBattleMenu( const PLIST_WORK *work );
extern const BOOL PLIST_UTIL_IsBattleMenu_CanReturn( const PLIST_WORK *work );
extern const PLIST_SKILL_CAN_LEARN PLIST_UTIL_CheckLearnSkill( PLIST_WORK *work , const POKEMON_PARAM *pp , const u8 idx );
//monsno返し
extern const u16 PLIST_UTIL_CheckItemEvolution( PLIST_WORK *work , POKEMON_PARAM *pp , const u16 itemNo );

//文字描画用
extern void PLIST_UTIL_DrawStrFunc( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawStrFuncSys( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawValueStrFunc( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col );
extern void PLIST_UTIL_DrawValueStrFuncSys( PLIST_WORK *work , GFL_BMPWIN *bmpWin , 
                                      WORDSET *wordSet, const u16 strId , const u16 posX , const u16 posY , const u16 col );

//アイテムに渡す用
extern void PLIST_MessageWaitInit( PLIST_WORK *work , u32 msgId , const BOOL isWaitKey , PLIST_CallbackFunc msgCallBack );
extern void PLIST_MSGCB_ExitCommon( PLIST_WORK *work );
extern void PLIST_MSGCB_ForgetSkill_ForgetCheck( PLIST_WORK *work );

//時間切れ強制終了
extern void PLIST_ForceExit_Timeup( PLIST_WORK *work );
