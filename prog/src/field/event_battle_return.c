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
#include "poke_tool\shinka_check.h"
#include "demo\shinka_demo.h"
#include "app/zukan_toroku.h"

#include "arc_def.h"
#include "message.naix"
#include "msg/msg_zkn.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

// local includes --------------------
#include "event_battle_return.h"

// オーバーレイ
FS_EXTERN_OVERLAY(zukan_toroku);

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
  ZUKAN_TOROKU_PARAM zukan_toroku_param;
  STRBUF*            box_strbuf;  // NULLでないときポケモン捕獲でボックス転送になる
  u32                box_tray;

  SHINKA_DEMO_PARAM*  shinka_param;
  u16                 shinka_poke_pos;
  u16                 shinka_poke_bit;

  HEAPID  heapID;

}BTLRET_WORK;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT BtlRet_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BtlRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT BtlRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void check_lvup_poke( BTLRET_WORK* wk, BTLRET_PARAM* param );

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
  wk->box_strbuf = NULL;
  wk->box_tray = 0;
  wk->shinka_param = NULL;
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

  if( wk->box_strbuf )
  {
    GFL_STR_DeleteBuffer( wk->box_strbuf );
  }

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
      ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( param->gameData );
      BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );

      wk->shinka_poke_pos = 0;
      wk->shinka_poke_bit = 0;

      // 野生orゲーム内通常トレーナー（サブウェイ除く）の後のみ行う処理
      if( (param->btlResult->competitor == BTL_COMPETITOR_WILD)
      ||  (param->btlResult->competitor == BTL_COMPETITOR_TRAINER)
      ){
        // レベルアップ進化チェック
        check_lvup_poke( wk, param );
        // パーティデータ書き戻し
        PokeParty_Copy( param->btlResult->party[ BTL_CLIENT_PLAYER ], party );

        //ポケルス感染＆伝染チェック
        POKERUS_CheckCatchPokerus( party );
        POKERUS_CheckContagion( party );

        // おこづかい増やす
        MyStatus_SetGold( myStatus, MyStatus_GetGold(myStatus) + param->btlResult->getMoney );
      }

      // 捕獲した
      if( param->btlResult->result == BTL_RESULT_CAPTURE )
      {
        wk->pp = PokeParty_GetMemberPointer(
                                param->btlResult->party[ BTL_CLIENT_ENEMY1 ], param->btlResult->capturedPokeIdx );

        // 親名セットしてるけど、本来はエンカウント前にフィールド側で設定すべき？
        MyStatus_CopyNameString( myStatus, wk->strbuf );
        PP_Put( wk->pp, ID_PARA_oyaname, (u32)(wk->strbuf) );

        // 手持ちかボックスに置くか
        if( !( PokeParty_GetPokeCount(party) < PokeParty_GetPokeCountMax(party) ) )
        {
          int tray_num;
          int pos;
          BOOL empty;

          GFL_MSGDATA* msgdata;
          BOOL flag_pcname;

          // 転送されるボックスのトレイナンバーを確定する
          tray_num = (int)BOXDAT_GetCureentTrayNumber( boxman );
          pos = 0;
          empty = BOXDAT_GetEmptyTrayNumberAndPos( boxman, &tray_num, &pos );  // これの結果はBOXDAT_PutPokemonと同じはず
          GF_ASSERT_MSG( empty, "BtlRet_ProcMain : ボックスに空きがありませんでした。\n" );
          wk->box_tray = (u32)tray_num;

          // STRBUFを作成する
          msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_dat, HEAPID_BTLRET_SYS );
          flag_pcname = EVENTWORK_CheckEventFlag( GAMEDATA_GetEventWork(param->gameData), SYS_FLAG_PCNAME );
          wk->box_strbuf = GFL_MSG_CreateString( msgdata, (flag_pcname)?(ZKN_POKEGET_BOX_02):(ZKN_POKEGET_BOX_01) );
          GFL_MSG_Delete( msgdata );
        }

        // 図鑑登録画面 or ニックネーム命名確認画面 へ
        GFL_OVERLAY_Load( FS_OVERLAY_ID(zukan_toroku) );
        if( !ZUKANSAVE_GetPokeGetFlag( zukan_savedata, (u16)( PP_Get(wk->pp, ID_PARA_monsno, NULL) ) ) )
        {
          ZUKANSAVE_SetPokeGet( zukan_savedata, wk->pp );  // 図鑑フラグをセットする
          ZUKAN_TOROKU_SetParam( &(wk->zukan_toroku_param), ZUKAN_TOROKU_LAUNCH_TOROKU, wk->pp, wk->box_strbuf, boxman, wk->box_tray );
        }
        else
        {
          ZUKAN_TOROKU_SetParam( &(wk->zukan_toroku_param), ZUKAN_TOROKU_LAUNCH_NICKNAME, wk->pp, wk->box_strbuf, boxman, wk->box_tray );
        }
        GFL_PROC_SysCallProc( NO_OVERLAY_ID, &ZUKAN_TOROKU_ProcData, &(wk->zukan_toroku_param) );
        (*seq)++;
      }else{
        (*seq) = 4;
      }
    }
    break;

  case 1:
    {
      BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );

      BOOL nickname = FALSE;
      if( ZUKAN_TOROKU_GetResult(&(wk->zukan_toroku_param)) == ZUKAN_TOROKU_RESULT_NICKNAME )
      {
        nickname = TRUE;
      }
      GFL_OVERLAY_Unload( FS_OVERLAY_ID(zukan_toroku) );

      if( nickname )
      {
        // 名前入力画面へ
        GFL_OVERLAY_Load( FS_OVERLAY_ID(namein) );
        wk->nameinParam = NAMEIN_AllocParamPokemonCapture( wk->heapID, wk->pp, NAMEIN_POKEMON_LENGTH, NULL,
                                                           wk->box_strbuf, boxman, wk->box_tray );

        GFL_PROC_SysCallProc( NO_OVERLAY_ID, &NameInputProcData, wk->nameinParam );
        (*seq)++;
      }
      else
      {
        (*seq) = 3;
      }
    }
    break;

  case 2:
    if( !NAMEIN_IsCancel(wk->nameinParam) ){
      NAMEIN_CopyStr( wk->nameinParam, wk->strbuf );
      PP_Put( wk->pp, ID_PARA_nickname, (u32)(wk->strbuf) );
    }
    NAMEIN_FreeParam( wk->nameinParam );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(namein) );
    wk->nameinParam = NULL;
    (*seq)++;
    break;

  case 3:
    {
      POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );

      if( wk->box_strbuf == NULL ){
        PokeParty_Add( party, wk->pp );
      }else{
        BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );
        BOXDAT_PutPokemon( boxman, PP_GetPPPPointer(wk->pp) );
      }
    }
    (*seq)++;
    break;
  case 4:
    //進化チェック
    if( wk->shinka_poke_bit )
    {
      u16 after_mons_no = 0;
      int pos;
      SHINKA_COND cond;
      POKEPARTY* party  = GAMEDATA_GetMyPokemon( param->gameData );
      (*seq) = 6;
      OS_TPrintf("進化ポケモンがいる\n");
      while( wk->shinka_poke_bit )
      {
        if( wk->shinka_poke_bit & 1 )
        {
          POKEMON_PARAM* pp = PokeParty_GetMemberPointer( party, wk->shinka_poke_pos );
          after_mons_no = SHINKA_Check( party, pp, SHINKA_TYPE_LEVELUP, 0, &cond, wk->heapID );
          pos = wk->shinka_poke_pos;
        }
        wk->shinka_poke_bit = wk->shinka_poke_bit >> 1;
        wk->shinka_poke_pos++;
        if( after_mons_no )
        {
          GFL_OVERLAY_Load( FS_OVERLAY_ID(shinka_demo) );
          wk->shinka_param = SHINKADEMO_AllocParam( wk->heapID, param->gameData, party, after_mons_no, pos, cond, FALSE );
          GFL_PROC_SysCallProc( NO_OVERLAY_ID, &ShinkaDemoProcData, wk->shinka_param );
          (*seq) = 5;
          break;
        }
      }
    }
    else
    {
      (*seq) = 6;
    }
    break;
  case 5:
    SHINKADEMO_FreeParam( wk->shinka_param );
    GFL_OVERLAY_Unload( FS_OVERLAY_ID(shinka_demo) );
    wk->shinka_param = NULL;
    if( wk->shinka_poke_bit )
    {
      (*seq) = 4;
    }
    else
    {
      (*seq)++;
    }
    break;
  case 6:
  default:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
/**
 * レベルアップしているポケモンを調べて進化チェックするか判断する
 */
//--------------------------------------------------------------------------
static void check_lvup_poke( BTLRET_WORK* wk, BTLRET_PARAM* param )
{
  //勝利、逃げる、ゲット以外は進化チェックしない（逃げる、ゲットは2vs2野生でありうるので）
  if( ( param->btlResult->result != BTL_RESULT_WIN ) &&
      ( param->btlResult->result != BTL_RESULT_RUN ) &&
      ( param->btlResult->result != BTL_RESULT_CAPTURE ) )
  {
    return;
  }

  // 野生＆ゲーム内トレーナー（サブウェイ除く）との対戦以外はチェックしない（レベルアップしないはず）
  if( (param->btlResult->competitor != BTL_COMPETITOR_WILD)
  &&  (param->btlResult->competitor != BTL_COMPETITOR_TRAINER)
  ){
    OS_TPrintf("サブウェイモードなどで進化チェックを行わない\n");
    return;
  }

  {
    POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );
    POKEMON_PARAM*  old_pp;
    POKEMON_PARAM*  new_pp;
    int i;
    int max = PokeParty_GetPokeCount( party );

    for( i = 0 ; i < max ; i++ )
    {
      old_pp = PokeParty_GetMemberPointer( party, i );
      new_pp = PokeParty_GetMemberPointer( param->btlResult->party[ BTL_CLIENT_PLAYER ], i );
      if( PP_Get( old_pp, ID_PARA_level, NULL ) < PP_Get( new_pp, ID_PARA_level, NULL ) )
      {
        wk->shinka_poke_bit |= ( 1 << i );
      }
    }
  }
}

