//============================================================================================
/**
 * @file      event_pdc_return.c
 * @brief     ポケモンドリームキャッチからフィールドへの復帰管理
 * @author    Koji Kawada
 * @date      2010.03.23
 */
//============================================================================================

// lib includes -----------------------
#include <gflib.h>
#include <procsys.h>

// global includes --------------------
#include "system\main.h"
#include "savedata\box_savedata.h"
#include "app\name_input.h"
#include "app/zukan_toroku.h"
#include "poke_tool/poke_memo.h"
#include "field/zonedata.h"  //ZONEDATA_GetPlaceNameID
#include "gamesystem/game_data.h"
#include "savedata/record.h"

#include "arc_def.h"
#include "message.naix"
#include "msg/msg_zkn.h"

#include "../../../resource/fldmapdata/flagwork/flag_define.h"

// local includes --------------------
#include "event_pdc_return.h"


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
  ZUKAN_TOROKU_PARAM zukan_toroku_param;
  STRBUF*            box_strbuf;  // NULLでないときポケモン捕獲でボックス転送になる
  u32                box_tray;

  HEAPID  heapID;

  // ローカルPROCシステム
  GFL_PROCSYS*  local_procsys;
}PDCRET_WORK;

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT PdcRet_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PdcRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT PdcRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

/*--------------------------------------------------------------------------*/
/* Proc Table                                                               */
/*--------------------------------------------------------------------------*/
const GFL_PROC_DATA   PDCRET_ProcData = {
  PdcRet_ProcInit,
  PdcRet_ProcMain,
  PdcRet_ProcQuit,
};


//--------------------------------------------------------------------------
/**
 * @brief PDCRET_PARAM生成
 *
 * @param[in] gameData  GAMEDATA
 * @param[in] result    PDC_RESULT
 * @param[in] pp        POKEMON_PARAM
 * @param[in] heapID    ヒープID
 *
 * @retval  PDCRET_PARAM
 */
//--------------------------------------------------------------------------
PDCRET_PARAM*  PDCRET_AllocParam( GAMEDATA* gameData, PDC_RESULT result, POKEMON_PARAM* pp,
                                  HEAPID heapID )
{
  PDCRET_PARAM* param = GFL_HEAP_AllocMemory( heapID, sizeof( PDCRET_PARAM ) );
  param->gameData = gameData;
  param->result   = result;
  param->pp       = pp;
  return param;
}
//--------------------------------------------------------------------------
/**
 * @brief PDCRET_PARAM破棄
 *
 * @param[in,out] param  PDCRET_PARAM
 *
 * @retval  
 */
//--------------------------------------------------------------------------
void  PDCRET_FreeParam( PDCRET_PARAM* param )
{
  GFL_HEAP_FreeMemory( param );
}


//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PdcRet_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDCRET_WORK* wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PDCRET_SYS, 0x1000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(PDCRET_WORK), HEAPID_PDCRET_SYS );

  wk->strbuf = GFL_STR_CreateBuffer( STRBUF_SIZE, HEAPID_PDCRET_SYS );
  wk->nameinParam = NULL;
  wk->box_strbuf = NULL;
  wk->box_tray = 0;
  wk->heapID = HEAPID_PDCRET_SYS;

  // ローカルPROCシステムを作成
  wk->local_procsys = GFL_PROC_LOCAL_boot( wk->heapID );

  return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PdcRet_ProcQuit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDCRET_WORK* wk = mywk;

  // ローカルPROCシステムを破棄
  GFL_PROC_LOCAL_Exit( wk->local_procsys ); 

  if( wk->box_strbuf )
  {
    GFL_STR_DeleteBuffer( wk->box_strbuf );
  }

  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_PDCRET_SYS );
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PdcRet_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  PDCRET_WORK*  wk = mywk;
  PDCRET_PARAM* param = pwk;

  // ローカルPROCの更新処理
  GFL_PROC_MAIN_STATUS  local_proc_status   =  GFL_PROC_LOCAL_Main( wk->local_procsys );
  if( local_proc_status == GFL_PROC_MAIN_VALID ) return GFL_PROC_RES_CONTINUE;

  switch( *seq ){
  case 0:
    {
      POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );
      MYSTATUS*  myStatus = GAMEDATA_GetMyStatus( param->gameData );
      ZUKAN_SAVEDATA* zukan_savedata = GAMEDATA_GetZukanSave( param->gameData );
      BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );

      // 捕獲した
      if( param->result == PDC_RESULT_CAPTURE )
      {
        BOOL zenkoku_flag = FALSE;  // 全国図鑑のときTRUE
        BOOL get_first = FALSE;  // 初捕獲のときTRUE

        {//捕獲回数インクリメント
          RECORD *record = GAMEDATA_GetRecordPtr(param->gameData);
          RECORD_Inc(record,RECID_CAPTURE_POKE);
          RECORD_Inc(record,RECID_DAYCNT_CAPTURE);
        }

        // 親名セットしてるけど、本来はエンカウント前にフィールド側で設定すべき？
        MyStatus_CopyNameString( myStatus, wk->strbuf );
        PP_Put( param->pp, ID_PARA_oyaname, (u32)(wk->strbuf) );

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
          GF_ASSERT_MSG( empty, "PdcRet_ProcMain : ボックスに空きがありませんでした。\n" );
          wk->box_tray = (u32)tray_num;

          // STRBUFを作成する
          msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_zkn_dat, HEAPID_PDCRET_SYS );
          flag_pcname = EVENTWORK_CheckEventFlag( GAMEDATA_GetEventWork(param->gameData), SYS_FLAG_PCNAME );
          wk->box_strbuf = GFL_MSG_CreateString( msgdata, (flag_pcname)?(ZKN_POKEGET_BOX_02):(ZKN_POKEGET_BOX_01) );
          GFL_MSG_Delete( msgdata );
        }

        // トレーナーメモ
        {
          PLAYER_WORK* player_wk = GAMEDATA_GetMyPlayerWork( param->gameData );
          POKE_MEMO_SetTrainerMemoPP(
              param->pp,
              POKE_MEMO_SET_CAPTURE,
              myStatus,
              ZONEDATA_GetPlaceNameID( PLAYERWORK_getZoneID( player_wk ) ),
              wk->heapID );
        }

        // 全国図鑑か否か
        //zenkoku_flag = ZUKANSAVE_GetZukanMode( zukan_savedata );  // TRUEのとき全国図鑑として扱っている
        zenkoku_flag = ZUKANSAVE_GetZenkokuZukanFlag( zukan_savedata );  // TRUEのとき全国図鑑を持っている

        // 図鑑登録画面 or ニックネーム命名確認画面 へ
        get_first = !ZUKANSAVE_GetPokeGetFlag( zukan_savedata, (u16)( PP_Get(param->pp, ID_PARA_monsno, NULL) ) );
        // 図鑑登録（捕まえた）
        {
          ZUKANSAVE_SetPokeSee( zukan_savedata, param->pp );  // 見た  // 図鑑フラグをセットする
          ZUKANSAVE_SetPokeGet( zukan_savedata, param->pp );  // 捕まえた  // 図鑑フラグをセットする
        }

        // ZUKAN_TOROKU_PARAMの設定
        {
          wk->zukan_toroku_param.pp               = param->pp;
          wk->zukan_toroku_param.b_zenkoku_flag   = zenkoku_flag;
          wk->zukan_toroku_param.box_strbuf       = wk->box_strbuf;
          wk->zukan_toroku_param.box_manager      = boxman;
          wk->zukan_toroku_param.box_tray         = wk->box_tray;
          wk->zukan_toroku_param.gamedata         = param->gameData;
        }
        if( get_first )
        {
          wk->zukan_toroku_param.launch           = ZUKAN_TOROKU_LAUNCH_TOROKU;
        }
        else
        {
          wk->zukan_toroku_param.launch           = ZUKAN_TOROKU_LAUNCH_NICKNAME;
        }
        // ローカルPROC呼び出し
        GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(zukan_toroku), &ZUKAN_TOROKU_ProcData, &(wk->zukan_toroku_param) );
        (*seq)++;
      }else{
        // 逃げた
        if( param->result == PDC_RESULT_ESCAPE )
        {
          // 図鑑登録（逃げたので見ただけ）
          {
            ZUKANSAVE_SetPokeSee( zukan_savedata, param->pp );  // 見た  // 図鑑フラグをセットする
          }
        }

        (*seq) = 4;
      }
    }
    break;

  case 1:
    {
      // ローカルPROCが終了するのを待つ  // このMainの最初でGFL_PROC_MAIN_VALIDならreturnしているので、ここでは判定しなくてもよいが念のため
      if( local_proc_status != GFL_PROC_MAIN_VALID )
      {
        BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );

        BOOL nickname = FALSE;
        if( wk->zukan_toroku_param.result == ZUKAN_TOROKU_RESULT_NICKNAME )
        {
          nickname = TRUE;
        }

        if( nickname )
        {
          MISC *misc = SaveData_GetMisc( GAMEDATA_GetSaveControlWork(param->gameData) );
          // 名前入力画面へ
          wk->nameinParam = NAMEIN_AllocParamPokemonCapture( wk->heapID, param->pp, NAMEIN_POKEMON_LENGTH, NULL,
                                                           wk->box_strbuf, boxman, wk->box_tray,  misc );

          // ローカルPROC呼び出し
          GFL_PROC_LOCAL_CallProc( wk->local_procsys, FS_OVERLAY_ID(namein), &NameInputProcData, wk->nameinParam );
          (*seq)++;
        }
        else
        {
          (*seq) = 3;
        }
      }
      else
      {
        return GFL_PROC_RES_CONTINUE;
      }
    }
    break;

  case 2:
    // ローカルPROCが終了するのを待つ  // このMainの最初でGFL_PROC_MAIN_VALIDならreturnしているので、ここでは判定しなくてもよいが念のため
    if( local_proc_status != GFL_PROC_MAIN_VALID )
    {
      if( !NAMEIN_IsCancel(wk->nameinParam) ){
        NAMEIN_CopyStr( wk->nameinParam, wk->strbuf );
        PP_Put( param->pp, ID_PARA_nickname, (u32)(wk->strbuf) );

        // ポケモンにニックネームを付けた回数
        {
          RECORD* rec = GAMEDATA_GetRecordPtr(param->gameData); 
          RECORD_Inc(rec, RECID_NICKNAME);
        }
      }
      NAMEIN_FreeParam( wk->nameinParam );
      wk->nameinParam = NULL;
      (*seq)++;
    }
    else
    {
      return GFL_PROC_RES_CONTINUE;
    }
    break;

  case 3:
    {
      POKEPARTY* party    = GAMEDATA_GetMyPokemon( param->gameData );

      if( wk->box_strbuf == NULL ){
        PokeParty_Add( party, param->pp );
      }else{
        BOX_MANAGER* boxman = GAMEDATA_GetBoxManager( param->gameData );
        BOXDAT_PutPokemon( boxman, PP_GetPPPPointer(param->pp) );
      }
    }
    (*seq)++;
    break;
  case 4:
  default:
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

