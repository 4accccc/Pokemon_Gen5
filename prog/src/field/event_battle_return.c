//============================================================================================
/**
 * @file  event_battle_return.c
 * @brief バトルからフィールドへの復帰管理
 * @author  taya
 * @date  2009.10.23
 */
//============================================================================================

// lib includes -----------------------
#include <gflib.h>
#include <procsys.h>

// global includes --------------------
#include "system\main.h"

// local includes --------------------
#include "event_battle_return.h"



/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT BtlRet_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BtlRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BtlRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );



/*--------------------------------------------------------------------------*/
/* Proc Table                                                               */
/*--------------------------------------------------------------------------*/
const GFL_PROC_DATA   BtlRet_ProcData = {
  BtlRet_ProcInit,
  BtlRet_ProcMain,
  BtlRet_ProcQuit,
};


//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT BtlRet_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT BtlRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT BtlRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  BTLRET_PARAM* param = pwk;

  switch( *seq ){
  case 0:
    // @todo 図鑑（見たフラグ）をセットする

    // 捕獲した
    if( param->btlResult->result == BTL_RESULT_CAPTURE )
    {
      POKEPARTY* party  = GAMEDATA_GetMyPokemon( param->gameData );
      POKEMON_PARAM* pp = PokeParty_GetMemberPointer(
                            param->btlResult->partyEnemy1, param->btlResult->capturedPokeIdx );

      if( PokeParty_GetPokeCount(party) < PokeParty_GetPokeCountMax(party) ){
        PokeParty_Add( party, pp );
      }
    }
    break;
  }

  return GFL_PROC_RES_FINISH;
}
