//============================================================================================
/**
 * @file    event_battle_return.c
 * @brief   バトルからフィールドへの復帰管理
 * @author  taya
 * @date    2009.10.23
 */
//============================================================================================

// lib includes -----------------------
#include <gflib.h>
#include <procsys.h>

// global includes --------------------
#include "system\main.h"
#include "savedata\box_savedata.h"
#include "app\name_input.h"
#include "poke_tool\pokerus.h"

// local includes --------------------
#include "event_battle_return.h"


/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  STRBUF_SIZE = 32,   ///< プレイヤー名を一時的に格納するためのバッファサイズ
};

/*--------------------------------------------------------------------------*/
/* Main Work                                                                */
/*--------------------------------------------------------------------------*/
typedef struct {

  NAMEIN_PARAM*     nameinParam;
  STRBUF*           strbuf;
  POKEMON_PARAM*    pp;

  HEAPID  heapID;

}BTLRET_WORK;

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
  BTLRET_WORK* wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_BTLRET_SYS, 0x1000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(BTLRET_WORK), HEAPID_BTLRET_SYS );

  wk->strbuf = GFL_STR_CreateBuffer( STRBUF_SIZE, HEAPID_BTLRET_SYS );
  wk->nameinParam = NULL;
  wk->pp = NULL;
  wk->heapID = HEAPID_BTLRET_SYS;

  return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT BtlRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  BTLRET_WORK* wk = mywk;

  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_BTLRET_SYS );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT BtlRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  BTLRET_WORK*  wk = mywk;
  BTLRET_PARAM* param = pwk;

  switch( *seq ){
  case 0:
    {
      POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );
      MYSTATUS*  myStatus = GAMEDATA_GetMyStatus( param->gameData );

      PokeParty_Copy( param->btlResult->party[ BTL_CLIENT_PLAYER ], party );

      //ポケルス感染チェック
      //通信対戦では感染チェックをしない
      if( param->btlResult->competitor != BTL_COMPETITOR_COMM )
      {
        //感染チェック
        POKERUS_CheckCatchPokerus( party );
        //伝染チェック
        POKERUS_CheckContagion( party );
      }

      // @todo 図鑑フラグをセットする

      // 捕獲した
      if( param->btlResult->result == BTL_RESULT_CAPTURE )
      {
        wk->pp = PokeParty_GetMemberPointer(
                                param->btlResult->party[ BTL_CLIENT_ENEMY1 ], param->btlResult->capturedPokeIdx );

        // 親名セットしてるけど、本来はエンカウント前にフィールド側で設定すべき？
        MyStatus_CopyNameString( myStatus, wk->strbuf );
        PP_Put( wk->pp, ID_PARA_oyaname, (u32)(wk->strbuf) );

        // @todo 今は必ず名前入力させているが、いずれ確認画面を作る。
        GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
        wk->nameinParam = NAMEIN_AllocParamPokemonByPP( wk->heapID, wk->pp, NAMEIN_POKEMON_LENGTH, NULL );

        GFL_PROC_SysCallProc( NO_OVERLAY_ID, &NameInputProcData, wk->nameinParam );
        (*seq)++;
      }else{
        (*seq) = 3;
      }
    }
    break;

  case 1:
    if( !NAMEIN_IsCancel(wk->nameinParam) ){
      NAMEIN_CopyStr( wk->nameinParam, wk->strbuf );
      PP_Put( wk->pp, ID_PARA_nickname, (u32)(wk->strbuf) );
    }
    NAMEIN_FreeParam( wk->nameinParam );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    wk->nameinParam = NULL;
    (*seq)++;
    break;

  case 2:
    {
      POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );

      if( PokeParty_GetPokeCount(party) < PokeParty_GetPokeCountMax(party) ){
        PokeParty_Add( party, wk->pp );
      }else{
        BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );
        BOXDAT_PutPokemon( boxman, PP_GetPPPPointer(wk->pp) );
      }
    }
    (*seq)++;
    break;

  default:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

