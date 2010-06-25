//=============================================================================================
/**
 * @file  btlv_scd.c
 * @brief ポケモンWB バトル 描画 下画面制御モジュール
 * @author  taya
 *
 * @date  2008.11.18  作成
 */
//=============================================================================================
#include <ui.h>
#include "print/printsys.h"

#include "arc_def.h"
#include "font/font.naix"
#include "sound/pm_sndsys.h"
#include "poke_tool/monsno_def.h"

#include "msg/msg_btl_ui.h"

#include "battle/btl_common.h"
#include "battle/btl_util.h"
#include "battle/btl_string.h"
#include "battle/btl_action.h"

#include "btlv_core.h"
#include "btlv_effect.h"
#include "btlv_mcss.h"
#include "btlv_scd.h"

#include "btlv_input.h"

#include "debug/debug_hudson.h"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  TEST_SELWIN_COL1 = 0x04,
  TEST_SELWIN_COL2 = 0x06,
  TEST_SELWIN_COL3 = 0x07,
  TEST_SELWIN_COL4 = 0x0A,

  POKEWIN_WIDTH  = 15*8,
  POKEWIN_HEIGHT = 4*8,

  POKEWIN_1_X = 0,
  POKEWIN_1_Y = 8,
  POKEWIN_2_X = 256-POKEWIN_WIDTH,
  POKEWIN_2_Y = POKEWIN_1_Y,

  POKEWIN_3_X = 0,
  POKEWIN_3_Y = POKEWIN_1_Y+POKEWIN_HEIGHT+8,
  POKEWIN_4_X = 256-POKEWIN_WIDTH,
  POKEWIN_4_Y = POKEWIN_3_Y,

  POKEWIN_5_X = 0,
  POKEWIN_5_Y = POKEWIN_3_Y+POKEWIN_HEIGHT+8,
  POKEWIN_6_X = 256-POKEWIN_WIDTH,
  POKEWIN_6_Y = POKEWIN_5_Y,

  POKEWIN_BACK_X = POKEWIN_6_X + 64,
  POKEWIN_BACK_Y = POKEWIN_6_Y + POKEWIN_HEIGHT + 24,

  SUBPROC_STACK_DEPTH = 4,
};

//--------------------------------------------------------------
/**
 *  ワザ対象の選択／確認画面用パラメータ
 */
//--------------------------------------------------------------
enum {
  STW_SELECT_POKE,      ///< ポケモン選択
  STW_CONFIRM_POKE,     ///< 対象ポケモン確認のみ
  STW_CONFIRM_FIELD,    ///< 対象がポケモン以外（フィールド）
};
typedef u8 StwConfirmType;

typedef struct {

  u8 selectablePokeCount;       ///< 選択できるポケモンの候補数（0なら選択不可->確認モード）
  u8 confirmCount;              ///< 確認モード対象の数
  StwConfirmType  confirmType;  ///< 確認モード対象タイプ（0:ポケモン, 1:場）

  u8 pos[ BTL_POS_MAX ];    ///< 選択・確認するポケモンの位置

}SEL_TARGET_WORK;

//--------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------
struct _BTLV_SCD {

  GFL_BMPWIN*     win;
  GFL_BMP_DATA*   bmp;
  GFL_FONT*       font;
  STRBUF*         strbuf;
  const BTL_CLIENT* clientWork;

  PRINT_QUE*      printQue;
  PRINT_UTIL      printUtil;

  BTL_PROC      subProcStack[ SUBPROC_STACK_DEPTH ];
  u32           subProcStackPtr;
  int           sub_seq;
  BPFunc        wazaSelFunc;

  const BTL_POKEPARAM*  bpp;
  BTL_ACTION_PARAM*     destActionParam;

  const BTL_POKESELECT_PARAM*   pokesel_param;
  BTL_POKESELECT_RESULT*        pokesel_result;
  BTLV_ROTATION_WAZASEL_PARAM*  rotationSelParam;

  SEL_TARGET_WORK   selTargetWork;
  u8                selTargetDone;
  u8                selTargetType;
  u8                fActionPrevButton;
  s8                fadeValue;
  s8                fadeValueEnd;
  s8                fadeStep;
  u8                cursor_flag;    //下画面カーソルフラグ
  u8                bagMode;
  u8                shooterEnergy;
  u8                forceQuitSeq    : 7;
  u8                fForceQuitDoing : 1;

  u8                yesno_seq;
  int               yesno_result;

  BtlAction  selActionResult;
  const BTLV_CORE* vcore;
  const BTL_MAIN_MODULE* mainModule;
  const BTL_POKE_CONTAINER* pokeCon;
  const BTL_PARTY*          playerParty;
  HEAPID  heapID;

  BTLV_INPUT_WORK     *biw;
};


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void setupMainWin( BTLV_SCD* wk );
static void spstack_init( BTLV_SCD* wk );
static void spstack_push( BTLV_SCD* wk, BPFunc initFunc, BPFunc loopFunc );
static BOOL spstack_call( BTLV_SCD* wk );
static BOOL selectAction_init( int* seq, void* wk_adrs );
static BOOL selectActionRoot_loop( int* seq, void* wk_adrs );
static BOOL selectDemoRoot_loop( int* seq, void* wk_adrs );
static BOOL selectWaza_init( int* seq, void* wk_adrs );
static BOOL selectWaza_loop( int* seq, void* wk_adrs );
static BOOL selectRotationWaza_init( int* seq, void* wk_adrs );
static BOOL selectRotationWaza_loop( int* seq, void* wk_adrs );
static void stw_init( SEL_TARGET_WORK* stw );
static void stw_convert_pos_to_index( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, u8 num );
static void stw_setSelectablePoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmField( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_addFarPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, WazaID waza, BtlPokePos myPos );
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, BtlPokePos* targetPos );
static inline u8 stwdraw_vpos_to_tblidx( u8 vpos );
static void stwdraw_button( const u8* pos, u8 count, u8 format, BTLV_SCD* wk );
static void stw_draw( const SEL_TARGET_WORK* stw, BTLV_SCD* work );
static BOOL stw_draw_wait( BTLV_SCD* wk );
static BOOL selectTarget_init( int* seq, void* wk_adrs );
static BOOL selectTarget_loop( int* seq, void* wk_adrs );
static void seltgt_init_setup_work( SEL_TARGET_WORK* stw, BTLV_SCD* wk );

#ifdef DEBUG_ONLY_FOR_hudson

typedef struct {
  int TrunCnt;
} HUDSON_WORK;

static HUDSON_WORK wkHudson = {0};

// A連打
static BOOL UIFuncButtonA( GFL_UI_DEBUG_OVERWRITE* p_data, GFL_UI_DEBUG_OVERWRITE* p_data30 )
{
  static const GFL_UI_DEBUG_OVERWRITE w = {
    PAD_BUTTON_A,
    PAD_BUTTON_A,
  };

  *p_data = w;
  *p_data30 = w;

  return TRUE;
}
#endif // DEBUG_ONLY_FOR_hudson


BTLV_SCD*  BTLV_SCD_Create( const BTLV_CORE* vcore, const BTL_MAIN_MODULE* mainModule,
        const BTL_POKE_CONTAINER* pokeCon, GFL_TCBLSYS* tcbl, GFL_FONT* font, const BTL_CLIENT* client, BtlBagMode bagMode, HEAPID heapID )
{
  BTLV_SCD* wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(BTLV_SCD) );

  wk->vcore = vcore;
  wk->mainModule = mainModule;
  wk->pokeCon = pokeCon;
  wk->heapID = heapID;
  wk->font = font;
  wk->clientWork = client;
  wk->playerParty = BTL_POKECON_GetPartyDataConst( pokeCon, BTL_CLIENT_GetClientID(client) );
  wk->strbuf = GFL_STR_CreateBuffer( 128, heapID );
  wk->pokesel_param = NULL;
  wk->bagMode = bagMode;
  wk->shooterEnergy = 0;
  wk->forceQuitSeq = 0;
  wk->fForceQuitDoing = FALSE;

  wk->cursor_flag = 0;

  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

  spstack_init( wk );

#ifdef DEBUG_ONLY_FOR_hudson
  // Aボタン連打
  if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) ||
      HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) )
  {
    GFL_UI_DEBUG_OVERWRITE_SetCallBack( UIFuncButtonA );
  }
#endif // DEBUG_ONLY_FOR_hudson

  return wk;
}

void BTLV_SCD_Init( BTLV_SCD* wk )
{
  wk->biw = BTLV_INPUT_Init( BTL_MAIN_GetGameData( wk->mainModule ),
                             BTL_MAIN_GetRule( wk->mainModule ),
                             BTL_MAIN_GetCompetitor( wk->mainModule ),
                             BTLV_EFFECT_GetPfd(), wk->font, &wk->cursor_flag, wk->bagMode, wk->heapID );

  ///<obj
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  setupMainWin( wk );
}

//=============================================================================================
/**
 * フェードアウト開始
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_SCD_FadeOut( BTLV_SCD* wk, BOOL start )
{
  wk->fadeValue = 0;
  wk->fadeValueEnd = -16;
  wk->fadeStep = -1;
  BTLV_INPUT_SetFadeOut( wk->biw, start );
}
//=============================================================================================
/**
 * フェードイン開始
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_SCD_FadeIn( BTLV_SCD* wk )
{
  wk->fadeValue = -16;
  wk->fadeValueEnd = 0;
  wk->fadeStep = 1;
  BTLV_INPUT_SetFadeIn( wk->biw );
}
//=============================================================================================
/**
 * フェード（イン／アウト）進行
 *
 * @param   wk
 *
 * @retval  BOOL    TRUEで終了
 */
//=============================================================================================
BOOL BTLV_SCD_FadeFwd( BTLV_SCD* wk )
{
  return !BTLV_INPUT_CheckFadeExecute( wk->biw );
}
void BTLV_SCD_Cleanup( BTLV_SCD* wk )
{
  //GFL_BMPWIN_Delete( wk->win );
}
void BTLV_SCD_Setup( BTLV_SCD* wk )
{
  setupMainWin( wk );
}

static void setupMainWin( BTLV_SCD* wk )
{
#if 0
  wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME2_S, 0, 2, 32, 22, 0x0e, GFL_BMP_CHRAREA_GET_F );
  wk->bmp = GFL_BMPWIN_GetBmp( wk->win );
  GFL_BMP_Clear( wk->bmp, 0x00 );
  PRINT_UTIL_Setup( &wk->printUtil, wk->win );
  GFL_BMPWIN_MakeScreen( wk->win );
  GFL_BMPWIN_TransVramCharacter( wk->win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
#endif
}

void BTLV_SCD_RestartUI_IfNotStandBy( BTLV_SCD* wk )
{
  if( BTLV_INPUT_GetScrType(wk->biw) != BTLV_INPUT_SCRTYPE_STANDBY )
  {
    BTLV_SCD_RestartUI( wk );
  }
}
/**
 *  UI再起動
 */
void BTLV_SCD_RestartUI( BTLV_SCD* wk )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
  BTLV_EFFECT_SetGaugeYure( BTLV_EFFECT_GAUGE_YURE_STOP );
  BTLV_EFFECT_SetCameraWorkStop();
}
/**
 *  UI再起動の完了待ち
 */
BOOL BTLV_SCD_WaitRestartUI( BTLV_SCD* wk )
{
  return !BTLV_INPUT_CheckExecute( wk->biw );
}

void BTLV_SCD_Delete( BTLV_SCD* wk )
{
  //GFL_BMPWIN_Delete( wk->win );

#ifdef DEBUG_ONLY_FOR_hudson
  if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) ||
      HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) )
  {
    GFL_UI_DEBUG_OVERWRITE_SetCallBack( NULL );
  }
#endif // DEBUG_ONLY_FOR_hudson

  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_STR_DeleteBuffer( wk->strbuf );
  BTLV_INPUT_Exit( wk->biw );
  GFL_HEAP_FreeMemory( wk );
}

//-------------------------------------------------
// サブプロセススタック管理

static void spstack_init( BTLV_SCD* wk )
{
  int i;
  for(i=0; i<SUBPROC_STACK_DEPTH; i++)
  {
    BTL_UTIL_ClearProc( &wk->subProcStack[i] );
  }
  wk->subProcStackPtr = 0;
}

static void spstack_push( BTLV_SCD* wk, BPFunc initFunc, BPFunc loopFunc )
{
  GF_ASSERT(wk->subProcStackPtr < SUBPROC_STACK_DEPTH);

  {
    BTL_PROC* proc = &wk->subProcStack[wk->subProcStackPtr++];
    BTL_UTIL_SetupProc( proc, wk, initFunc, loopFunc );
  }
}
static BOOL spstack_call( BTLV_SCD* wk )
{
  if( wk->subProcStackPtr > 0 )
  {
    BTL_PROC* proc = &wk->subProcStack[ wk->subProcStackPtr - 1 ];

    if( BTL_UTIL_CallProc(proc) )
    {
      BTL_UTIL_ClearProc( proc );
      wk->subProcStackPtr--;
    }
    return FALSE;
  }
  else
  {
    return TRUE;
  }
}
//=============================================================================================
//=============================================================================================

//=============================================================================================
/**
 * アクション選択処理 開始
 *
 * @param   wk
 * @param   bpp
 * @param   fPrevButton
 * @param   dest
 */
//=============================================================================================
void BTLV_SCD_StartActionSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BOOL fPrevButton, u8 shooterEnergy, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;
  wk->selActionResult = BTL_ACTION_NULL;
  wk->fActionPrevButton = fPrevButton;
  wk->shooterEnergy = shooterEnergy;

  spstack_push( wk, selectAction_init, selectActionRoot_loop );
}
void BTLV_SCD_StartActionSelectDemoCapture( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BOOL fPrevButton, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;
  wk->selActionResult = BTL_ACTION_NULL;
  wk->fActionPrevButton = fPrevButton;

  spstack_push( wk, selectAction_init, selectDemoRoot_loop );
}
//=============================================================================================
/**
 * アクション選択処理 終了待ち
 *
 * @param   wk
 *
 * @retval  BtlAction
 */
//=============================================================================================
BtlAction BTLV_SCD_WaitActionSelect( BTLV_SCD* wk )
{
  if( spstack_call( wk ) ){
    BTL_Printf("選択されたアクション=%d\n", wk->selActionResult);
    return wk->selActionResult;
  }else{
    return BTL_ACTION_NULL;
  }
}

//=============================================================================================
/**
 * ワザ選択処理 開始
 *
 * @param   wk
 * @param   bpp
 * @param   dest
 */
//=============================================================================================
void BTLV_SCD_StartWazaSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;
  wk->selActionResult = BTL_ACTION_NULL;

  wk->wazaSelFunc = selectWaza_loop;

  spstack_push( wk, selectWaza_init, selectWaza_loop );
}
//=============================================================================================
/**
 * ワザ選択処理（ローテーションバトル用）開始
 *
 * @param   wk
 * @param   selParam
 * @param   actionParam
 */
//=============================================================================================
void BTLV_SCD_StartRotationWazaSelect( BTLV_SCD* wk, BTLV_ROTATION_WAZASEL_PARAM* selParam, BTL_ACTION_PARAM* actionParam )
{
  wk->rotationSelParam = selParam;
  wk->destActionParam = actionParam;
  wk->selActionResult = BTL_ACTION_NULL;

  wk->wazaSelFunc = selectRotationWaza_loop;

  BTL_ACTION_SetNULL( wk->destActionParam );
  BTL_ACTION_SetNULL( &selParam->actRotation );
  BTL_ACTION_SetNULL( &selParam->actWaza );

  spstack_push( wk, selectRotationWaza_init, selectRotationWaza_loop );
}
//=============================================================================================
/**
 * ワザ選択処理 終了待ち（通常／ローテーション共通）
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCD_WaitWazaSelect( BTLV_SCD* wk )
{
  return spstack_call( wk );
}
//=============================================================================================
/**
 * ワザ選択画面そのまま処理再開（通常／ローテーション共通）
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_SCD_RestartWazaSelect( BTLV_SCD* wk )
{
  spstack_push( wk, NULL, wk->wazaSelFunc );
}


void BTLV_SCD_StartWazaSelectDemoCapture( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;
  wk->selActionResult = BTL_ACTION_NULL;

  spstack_push( wk, selectWaza_init, selectDemoRoot_loop );
}

//=============================================================================================
/**
 * ワザ対象選択（確認）処理 開始
 *
 * @param   wk
 * @param   bpp
 * @param   dest
 */
//=============================================================================================
void BTLV_SCD_StartTargetSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;

  spstack_push( wk, selectTarget_init, selectTarget_loop );
}
//=============================================================================================
/**
 * ワザ対象選択（確認）処理 終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_SCD_WaitTargetSelect( BTLV_SCD* wk )
{
  return spstack_call( wk );
}

//=============================================================================================
/**
 * コマンド選択の制限時間による強制終了（アクション・ワザ・対象共通）
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_SCD_ForceQuitSelect( BTLV_SCD* wk )
{
  spstack_init( wk );
}

#include "data\command_sel.cdat"

static BOOL selectAction_init( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  BTLV_INPUT_COMMAND_PARAM  bicp;
  const BTL_PARTY* party[2] = {NULL};
  const BTL_POKEPARAM* bpp;
  const POKEMON_PARAM* pp;
  u16 members, hp, i, j;
  u16 party_members[2] = {0};
  int party_idx;
  int party_ofs;
  u8 clientID_1, clientID_2;
  BtlRule btl_rule;

  MI_CpuClear16( &bicp, sizeof( BTLV_INPUT_COMMAND_PARAM ) );

#ifdef DEBUG_ONLY_FOR_hudson
  // ターン数をカウント
  if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) || HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) ||
      HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) )
  {
    wkHudson.TrunCnt++;
    OS_TPrintf("TrunCnt=%d\n",wkHudson.TrunCnt);
  }
#endif // DEBUG_ONLY_FOR_hudson

  bicp.bagMode = wk->bagMode;
  bicp.shooterEnergy = wk->shooterEnergy;
  bicp.pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                       BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID( wk->bpp ) ) );

  BTLV_EFFECT_SetGaugeYure( bicp.pos );

  btl_rule = BTL_MAIN_GetRule( wk->mainModule );

  for( j = 0 ; j < 2 ; j++ )
  {
    //2client用パーティ格納ワーククリア
    party[0] = NULL;
    party[1] = NULL;

    if( j == 0 )
    {
      clientID_1 = BTL_MAIN_GetPlayerClientID( wk->mainModule );
      clientID_2 = BTL_MAIN_GetPlayerFriendCleintID( wk->mainModule );
      if( (btl_rule == BTL_RULE_DOUBLE) && (clientID_2 != BTL_CLIENTID_NULL) )
      {
        // 自分と仲間の手持ち
        if( clientID_1 < clientID_2 ){
          party[0] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_1 );
          party_members[0] = BTL_PARTY_GetMemberCount( party[0] );

          party[1] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_2 );
          party_members[1] = BTL_PARTY_GetMemberCount( party[1] );
        }else{
          party[0] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_2 );
          party_members[0] = BTL_PARTY_GetMemberCount( party[0] );

          party[1] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_1 );
          party_members[1] = BTL_PARTY_GetMemberCount( party[1] );
        }
      }
      else
      {
        //自分の手持ちのみ
        party[0] = wk->playerParty;
        party_members[0] = BTL_PARTY_GetMemberCount( party[0] );
      }


      switch( btl_rule ){
      case BTL_RULE_SINGLE:    ///< シングル
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_AA ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 0 ] = BPP_GetMonsNo( bpp );
          //bicp.form_no[ 0 ] = BPP_GetValue( bpp, BPP_FORM );
          //見た目にかかわるパラメータはPOKEMON_PARAM依存なので、ppから取得
          bicp.form_no[ 0 ] = PP_Get( BPP_GetViewSrcData( bpp ), ID_PARA_form_no, NULL );
          bicp.sex[ 0 ]     = BPP_GetValue( bpp, BPP_SEX );
        }
        break;
      case BTL_RULE_TRIPLE:    ///< トリプル
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_E ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 2 ] = BPP_GetMonsNo( bpp );
          //bicp.form_no[ 2 ] = BPP_GetValue( bpp, BPP_FORM );
          //見た目にかかわるパラメータはPOKEMON_PARAM依存なので、ppから取得
          bicp.form_no[ 2 ] = PP_Get( BPP_GetViewSrcData( bpp ), ID_PARA_form_no, NULL );
          bicp.sex[ 2 ]     = BPP_GetValue( bpp, BPP_SEX );
        }
      /* fallthru */
      case BTL_RULE_DOUBLE:    ///< ダブル
      case BTL_RULE_ROTATION:   ///< ローテーション @todo これでいいのか？
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_A ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 0 ] = BPP_GetMonsNo( bpp );
          //bicp.form_no[ 0 ] = BPP_GetValue( bpp, BPP_FORM );
          //見た目にかかわるパラメータはPOKEMON_PARAM依存なので、ppから取得
          bicp.form_no[ 0 ] = PP_Get( BPP_GetViewSrcData( bpp ), ID_PARA_form_no, NULL );
          bicp.sex[ 0 ]     = BPP_GetValue( bpp, BPP_SEX );
        }
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_C ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 1 ] = BPP_GetMonsNo( bpp );
          //bicp.form_no[ 1 ] = BPP_GetValue( bpp, BPP_FORM );
          //見た目にかかわるパラメータはPOKEMON_PARAM依存なので、ppから取得
          bicp.form_no[ 1 ] = PP_Get( BPP_GetViewSrcData( bpp ), ID_PARA_form_no, NULL );
          bicp.sex[ 1 ]     = BPP_GetValue( bpp, BPP_SEX );
        }
        //bicp.pos = ( bicp.pos - BTLV_MCSS_POS_A ) / 2;
        break;

      default:
        GF_ASSERT_MSG( 0, "知らない戦闘ルールです\n" );
        break;
      }
    }
    else
    {
      if( BTL_MAIN_GetCompetitor( wk->mainModule ) == BTL_COMPETITOR_WILD )
      {
        break;
      }

      clientID_1 = BTL_MAIN_GetEnemyClientID ( wk->mainModule, 0 );
      clientID_2 = BTL_MAIN_GetEnemyClientID ( wk->mainModule, 1 );

      bicp.trainer_flag = TRUE;
      //相手の手持ち
      party[0] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_1 );
      party_members[0] = BTL_PARTY_GetMemberCount( party[0] );

      // タッグバトル 2人目の相手情報も取得
      if(btl_rule == BTL_RULE_DOUBLE ){
        if( clientID_1 != clientID_2 ){
          party[1] = BTL_POKECON_GetPartyDataConst( wk->pokeCon, clientID_2 );
          party_members[1] = BTL_PARTY_GetMemberCount( party[1] );
        }
      }
    }

    if( party[1] == NULL ){
      // クライアント１人
      members = party_members[0];
    }else{
      //タッグ、マルチ　3以下のみ参照
      if( party_members[0] > (TEMOTI_POKEMAX/2) ){ party_members[0] = (TEMOTI_POKEMAX/2); }
      if( party_members[1] > (TEMOTI_POKEMAX/2) ){ party_members[1] = (TEMOTI_POKEMAX/2); }
      members = party_members[0] + party_members[1];
    }

    for(i=0; i<members; ++i)
    {
      if( i<party_members[0] ){
        party_idx = 0;
        party_ofs = i;
      }else{
        party_idx = 1;
        party_ofs = i-party_members[0];
      }

      bpp = BTL_PARTY_GetMemberDataConst( party[party_idx], party_ofs );

      if( BPP_IsDead( bpp ) == FALSE )
      {
        if( BPP_GetPokeSick( bpp ) != POKESICK_NULL )
        {
          bicp.bidp[ j ][ (party_idx*3)+party_ofs ].status = BTLV_INPUT_STATUS_NG;
        }
        else if( BPP_IsFightEnable( bpp ) == FALSE )
        {
          bicp.bidp[ j ][ (party_idx*3)+party_ofs ].status = BTLV_INPUT_STATUS_NONE;
        }
        else
        {
          bicp.bidp[ j ][ (party_idx*3)+party_ofs ].status = BTLV_INPUT_STATUS_ALIVE;
        }
      }
      else
      {
        bicp.bidp[ j ][ (party_idx*3)+party_ofs ].status = BTLV_INPUT_STATUS_DEAD;
      }
    }
  }

  bicp.center_button_type = wk->fActionPrevButton;
  bicp.weather = BTL_CLIENT_GetWeather( wk->clientWork );

  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_COMMAND, &bicp );

  return TRUE;
}

static BOOL selectActionRoot_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  int hit;

  switch( *seq ){
  case 0:
    hit = BTLV_INPUT_CheckInput( wk->biw, &BattleMenuTouchData, BattleMenuKeyData );
#ifdef DEBUG_ONLY_FOR_hudson
    //下画面変形中は入力を無視
    if( BTLV_INPUT_CheckExecute( wk->biw ) == FALSE )
    {
      // 一定ターンで逃げる
      if( ( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) && wkHudson.TrunCnt >= 2 ) ||
          ( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) && wkHudson.TrunCnt >= 6*5+1 ) ||
          ( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_POKE ) && wkHudson.TrunCnt >= 0 ) ||
          ( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA_CAM ) && wkHudson.TrunCnt >= 4 ) )
      {
        if( BTL_MAIN_GetRule( wk->mainModule ) == BTL_RULE_SINGLE )
        {
          BTL_POKEPARAM* bpp;
          bpp = (BTL_POKEPARAM*)BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_AA ) );
          BPP_CureWazaSick( bpp, WAZASICK_FREEFALL );
        }
        hit = 3;
      }
    }
#endif // DEBUG_ONLY_FOR_hudson
    if( hit != GFL_UI_TP_HIT_NONE )
    {
      static const u8 action[] = {
        BTL_ACTION_FIGHT,   BTL_ACTION_ITEM,
        BTL_ACTION_CHANGE,  BTL_ACTION_ESCAPE,
      };

      wk->selActionResult = action[ hit ];

      if( wk->selActionResult == BTL_ACTION_ESCAPE )
      {
        BTL_ACTION_SetEscapeParam( wk->destActionParam );
        BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
        BTLV_EFFECT_SetGaugeYure( BTLV_EFFECT_GAUGE_YURE_STOP );
        BTLV_EFFECT_SetCameraWorkStop();
        (*seq)++;
      }
      else{
        return TRUE;
      }
    }
    break;

  case 1:
    if( !BTLV_INPUT_CheckExecute(wk->biw) ){
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL selectDemoRoot_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  BOOL ret = FALSE;

  if( ( ret = BTLV_INPUT_CheckInputDemo( wk->biw ) ) == TRUE )
  {
    wk->selActionResult = BTL_ACTION_FIGHT;
  }

  return ret;
}


static BOOL selectWaza_init( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  BTLV_INPUT_WAZA_PARAM biwp;
  u16 wazaCnt, wazaID, i;
  u8 PP, PPMax;

  wazaCnt = BPP_WAZA_GetCount( wk->bpp );
  biwp.henshin_flag = BPP_HENSIN_Check( wk->bpp );
  BTL_Printf("ワザ数:%d, pokeID=%d\n", wazaCnt, BPP_GetID(wk->bpp));
  for(i=0; i<wazaCnt; i++)
  {
    wazaID = BPP_WAZA_GetParticular( wk->bpp, i, &PP, &PPMax );
    BTL_Printf(" waza(%d) = %d\n", i, wazaID);
    biwp.wazano[ i ] = wazaID;
    biwp.pp[ i ]     = PP;
    biwp.ppmax[ i ]  = PPMax;
  }
  for( ; i<PTL_WAZA_MAX; i++){
    biwp.wazano[ i ]  = 0;
    biwp.pp[ i ]      = 0;
    biwp.ppmax[ i ]   = 0;
  }

  biwp.pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                       BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID( wk->bpp ) ) );

  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_WAZA, &biwp );

  return TRUE;
}
static BOOL selectWaza_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  int hit;
  int pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                       BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID( wk->bpp ) ) );
  const BTLV_INPUT_HITTBL* SkillMenuTouchData = ( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) &&
                                                  ( pos != BTLV_MCSS_POS_C ) ) ?
                                                  &SkillMenuTouchData3vs3 :
                                                  &SkillMenuTouchDataNormal;
  const BTLV_INPUT_KEYTBL* SkillMenuKeyData = ( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) &&
                                                ( pos != BTLV_MCSS_POS_C ) ) ?
                                                SkillMenuKeyData3vs3 :
                                                SkillMenuKeyDataNormal;

//  hit = GFL_UI_TP_HitTrg( SkillMenuTouchData );
  hit = BTLV_INPUT_CheckInput( wk->biw, SkillMenuTouchData, SkillMenuKeyData );

  if( hit != GFL_UI_TP_HIT_NONE )
  {
    BOOL fWazaInfoMode = BTLV_INPUT_CheckWazaInfoModeMask( &hit );
    //キャンセルが押された
    if( hit == PTL_WAZA_MAX )
    {
      BTL_ACTION_SetNULL( wk->destActionParam );
      return TRUE;
    }
    else if( hit < BPP_WAZA_GetCount( wk->bpp ) )
    {
      WazaID waza;

      waza = BPP_WAZA_GetID( wk->bpp, hit );
      BTL_ACTION_SetFightParam( wk->destActionParam, waza, BTL_POS_NULL );
      if( fWazaInfoMode ){
        BTL_ACTION_FightParamToWazaInfoMode( wk->destActionParam );
      }

      return TRUE;
    }
    else if( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) && ( hit == 5 ) )
    {
      BTL_ACTION_SetMoveParam( wk->destActionParam );
      return TRUE;
    }
  }

  return FALSE;
}
//--------------------------------------------------------------------------------------
// ローテーションワザ選択
//--------------------------------------------------------------------------------------

static BOOL selectRotationWaza_init( int* seq, void* wk_adrs )
{
  #ifdef ROTATION_NEW_SYSTEM
  BTLV_SCD* wk = wk_adrs;
  BTLV_INPUT_ROTATE_PARAM birp;
  int i, j;

  for(i=0; i<BTL_ROTATE_NUM; ++i)
  {
    birp.bpp[ i ] = wk->rotationSelParam->poke[i].bpp;
    for(j=0; j<PTL_WAZA_MAX; ++j)
    {
      birp.waza_exist[ i ][ j ] = wk->rotationSelParam->poke[ i ].fWazaUsable[ j ];
    }
  }

  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_ROTATE, &birp );
  #endif

  return TRUE;
}

static BOOL selectRotationWaza_loop( int* seq, void* wk_adrs )
{
  #ifdef ROTATION_NEW_SYSTEM

  BTLV_SCD* wk = wk_adrs;
  BtlRotateDir dir;
  int  wazaIdx;

  if( BTLV_INPUT_CheckInputRotate(wk->biw, &dir, &wazaIdx) )
  {
    BOOL fWazaInfoMode = BTLV_INPUT_CheckWazaInfoModeMask( &wazaIdx );
    if( wazaIdx != PTL_WAZA_MAX )
    {
      WazaID waza;

      if( dir == BTL_ROTATEDIR_NONE ){
        dir = BTL_ROTATEDIR_STAY;
      }

      if( wazaIdx != BTLV_INPUT_WARUAGAKI_BUTTON )
      {
        int pokeIdx = BTL_MAINUTIL_GetRotateInPosIdx( dir );
        waza = BPP_WAZA_GetID( wk->rotationSelParam->poke[ pokeIdx ].bpp, wazaIdx );
      }
      else{
        waza = WAZANO_WARUAGAKI;
      }

      BTL_ACTION_SetRotation( &wk->rotationSelParam->actRotation, dir );
      BTL_ACTION_SetFightParam( &wk->rotationSelParam->actWaza, waza, BTL_POS_NULL );
      BTL_ACTION_SetFightParam( wk->destActionParam, waza, BTL_POS_NULL );
      if( fWazaInfoMode ){
        BTL_ACTION_FightParamToWazaInfoMode( wk->destActionParam );
      }
    }
    //一旦、くりだすことの出来ない技を選択したのち戻るボタンを押しても、
    //繰り出すことが出来ないメッセージが出てしまう不具合の対処
    //wk->destActionParam->gen.cmdがBTL_ACTION_NULLじゃないとアラートメッセージになるので、
    //初期化しておく
    else if( wazaIdx == BTLV_INPUT_CANCEL_BUTTON )
    {
      BTL_ACTION_SetNULL( wk->destActionParam );
    }
    return TRUE;
  }
  return FALSE;
  #else
  return TRUE;
  #endif
}


//--------------------------------------------------------------------------------------
// ターゲット選択画面描画処理
//--------------------------------------------------------------------------------------
enum {
// 2vs2
//    D B
//
// A C
  STW_BTN_WIDTH = 14,
  STW_BTN_HEIGHT = 10,

  STW_BTNPOS_D_X = 1,
  STW_BTNPOS_D_Y = 1,
  STW_BTNPOS_B_X = STW_BTNPOS_D_X + STW_BTN_WIDTH + 1,
  STW_BTNPOS_B_Y = STW_BTNPOS_D_Y,
  STW_BTNPOS_A_X = STW_BTNPOS_D_X,
  STW_BTNPOS_A_Y = STW_BTNPOS_D_Y + STW_BTN_HEIGHT + 1,
  STW_BTNPOS_C_X = STW_BTNPOS_B_X,
  STW_BTNPOS_C_Y = STW_BTNPOS_A_Y,

};

#include "data\target_sel.cdat"

/*
typedef struct {

  u8 selectablePokeCount;   ///< 選択できるポケモンの候補数（0なら選択不可->確認モード）
  u8 confirmCount;          ///< 確認モード対象の数
  u8 confirmType;           ///< 確認モード対象タイプ（0:ポケモン, 1:場）

  u8 pos[ BTL_POS_MAX ];    ///< 選択・確認するポケモンの位置

}SEL_TARGET_WORK;
*/
// 構造体初期化
static void stw_init( SEL_TARGET_WORK* stw )
{
  GFL_STD_MemClear( stw, sizeof( *stw ) );
}
// pos(戦闘位置）配列を、ポケモン選択インターフェイス用のインデックスに変換
static void stw_convert_pos_to_index( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, u8 num )
{
  u8 i;

  for(i=0; i<num; ++i)
  {
    stw->pos[i] = BTL_MAIN_BtlPosToViewPos( mainModule, stw->pos[i] );
  }
}
// 選択可能ポケモンをセット
static void stw_setSelectablePoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
  u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
  stw->selectablePokeCount = num;
  TAYA_Printf("選択可能ポケ数=%d\n", stw->selectablePokeCount);
}
// 確認のみ（ポケモン）をセット
static void stw_setConfirmPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
  u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
  stw->confirmCount = num;
  stw->confirmType = STW_CONFIRM_POKE;
}
// 確認のみ（場）をセット
static void stw_setConfirmField( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos )
{
  u8 num = BTL_MAIN_ExpandBtlPos( mainModule, exPos, stw->pos );
  stw->confirmCount = num;
  stw->confirmType = STW_CONFIRM_FIELD;
}
// 遠隔ヒットワザなら、自分から見て遠隔地に居るポケモンを追加する
static void stw_addFarPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, WazaID waza, BtlPokePos myPos )
{
  if( (BTL_MAIN_GetRule(mainModule) == BTL_RULE_TRIPLE)
  &&  (WAZADATA_GetFlag(waza, WAZAFLAG_TripleFar))
  ){
    BtlPokePos  farPos;
    if( BTL_MAINUTIL_GetTripleFarPos(myPos, &farPos) )
    {
      stw->pos[ stw->selectablePokeCount++ ] = farPos;
      stw->pos[ stw->selectablePokeCount++ ] = BTL_MAINUTIL_GetFacedPokePos( BTL_RULE_TRIPLE, farPos );
    }
  }
}
//
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, BtlPokePos* targetPos )
{
  GF_ASSERT_MSG(hitPos<NELEMS(STW_HitTblIndex), "hitPos=%d", hitPos);

  if( stw->selectablePokeCount )
  {
    u8 hitVpos = STW_HitTblIndex[ hitPos ];
    u8 i;

    *targetPos = BTL_MAIN_ViewPosToBtlPos( mainModule, hitVpos );

    BTL_N_Printf( DBGSTR_SCD_TargetHitPosConvert, hitPos, hitVpos, *targetPos );
    for(i=0; i<stw->selectablePokeCount; ++i)
    {
      if( *targetPos == stw->pos[i] )
      {
        BTL_N_Printf( DBGSTR_SCD_TargetHitPosFix, *targetPos );
        return TRUE;
      }
    }
    return FALSE;
  }
  else
  {
    // 確認のみモードの場合、どこをタッチしてもOKにしておく
    *targetPos = BTL_POS_NULL;
    return TRUE;
  }
}
//
static inline u8 stwdraw_vpos_to_tblidx( u8 vpos )
{
  u8 i;
  for(i=0; i<NELEMS(STW_HitTblIndex); ++i)
  {
    if( STW_HitTblIndex[i] == vpos )
    {
      return i;
    }
  }
  GF_ASSERT(0);
  return 0;
}

//----------------------------------------------------------------------------------
/**
 *
 *
 * @param   pos
 * @param   count
 * @param   format
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void stwdraw_button( const u8* pos, u8 count, u8 format, BTLV_SCD* wk )
{
  const BTL_POKEPARAM*    bpp;
  const POKEMON_PARAM*    pp;
  BTLV_INPUT_SCENE_PARAM  bisp;
  u8 vpos;
  WazaID waza = wk->destActionParam->fight.waza;

  MI_CpuClear16( &bisp, sizeof(bisp) );

  bisp.pos         = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                               BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(wk->bpp) )
                                             );


  bisp.waza_target = WAZADATA_GetParam( waza, WAZAPARAM_TARGET );
  if(waza == WAZANO_NOROI){// BTS:2957 のろい対処
    bisp.waza_target = BTL_CALC_GetNoroiTargetType( wk->bpp );
  }

  if( ( bisp.waza_target == WAZA_TARGET_OTHER_SELECT ) && ( WAZADATA_GetFlag( waza, WAZAFLAG_TripleFar ) ) )
  {
    bisp.waza_target = WAZA_TARGET_MAX;
  }

  BTL_Printf(" **** triple select **** \n");
  while( count-- )
  {
    bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, *pos );
    pp  = BPP_GetViewSrcData( bpp );
    vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, *pos );
    BTL_Printf(" count[%d]: pos=%d -> vpos=%d, pokeID=%d\n", count, *pos, vpos, BPP_GetID(bpp));
    vpos -= BTLV_MCSS_POS_A;
#if 0
  BTLV_MCSS_POS_A = 2
  BTLV_MCSS_POS_B,
  BTLV_MCSS_POS_C,
  BTLV_MCSS_POS_D,
  BTLV_MCSS_POS_E,
  BTLV_MCSS_POS_F,

  BTL_POS_1ST_0=0,    ///< スタンドアローンならプレイヤー側、通信対戦ならサーバ側
  BTL_POS_2ND_0,
  BTL_POS_1ST_1,
  BTL_POS_2ND_1,
  BTL_POS_1ST_2,
  BTL_POS_2ND_2,
#endif
    bisp.bidp[ vpos ].hp = BPP_GetValue( bpp, BPP_HP );
    bisp.bidp[ vpos ].exist = 1;
    if( bisp.bidp[ vpos ].hp )
    {
      int mons_no = BPP_GetMonsNo( bpp );
      bisp.bidp[ vpos ].pp = pp;
      bisp.bidp[ vpos ].hpmax = BPP_GetValue( bpp, BPP_MAX_HP );
      if( ( mons_no == MONSNO_NIDORAN_M ) || ( mons_no == MONSNO_NIDORAN_F ) )
      {
        bisp.bidp[ vpos ].sex = PTL_SEX_UNKNOWN;
      }
      else
      {
        //見た目にかかわるパラメータはPOKEMON_PARAM依存なので、ppから取得
        bisp.bidp[ vpos ].sex = PP_GetSex( BPP_GetViewSrcData( bpp ) );
      }

      if( BPP_GetPokeSick( bpp ) )
      {
        bisp.bidp[ vpos ].status = BTLV_INPUT_STATUS_NG;
      }
      else
      {
        bisp.bidp[ vpos ].status = BTLV_INPUT_STATUS_ALIVE;
      }
    }
    else
    {
      bisp.bidp[ vpos ].status = BTLV_INPUT_STATUS_DEAD;
    }
    pos++;
  }

  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_DIR, &bisp );
}
static void stw_draw( const SEL_TARGET_WORK* stw, BTLV_SCD* work )
{
  // ポケモン選択モード
  if( stw->selectablePokeCount )
  {
    stwdraw_button( stw->pos, stw->selectablePokeCount, 0, work );
  }
  // 確認のみ
  else
  {
    GF_ASSERT(stw->confirmCount);
    if( stw->confirmType == STW_CONFIRM_POKE )
    {
      stwdraw_button( stw->pos, stw->confirmCount, 1, work );
    }
    else
    {
      stwdraw_button( stw->pos, stw->confirmCount, 2, work );
    }
  }
}
static BOOL stw_draw_wait( BTLV_SCD* wk )
{
  PRINTSYS_QUE_Main( wk->printQue );
  return PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );
}


//=============================================================================================
//  ワザターゲット選択（確認）処理
//=============================================================================================

BOOL BTLV_SCD_IsSelTargetDone( BTLV_SCD* wk )
{
  return wk->selTargetDone;
}

static BOOL selectTarget_init( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;

  seltgt_init_setup_work( &wk->selTargetWork, wk );

  // トリプルはターゲット選択画面を新たに作る必要ある
  if( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE )
  {
    BtlPokePos  targetPos = BTL_POS_NULL;
    if( wk->selTargetWork.selectablePokeCount )
    {
      BtlPokePos  basePos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(wk->bpp) );
      const BTL_TRIPLE_ATTACK_AREA* area = BTL_MAINUTIL_GetTripleAttackArea( basePos );
      targetPos = area->enemyPos[ 0 ];
    }
    BTL_ACTION_SetFightParam( wk->destActionParam, wk->destActionParam->fight.waza, targetPos );
    wk->selTargetDone = TRUE;
  }

  stw_draw( &wk->selTargetWork, wk );
  wk->selTargetDone = FALSE;
  wk->selTargetType = WAZADATA_GetParam( wk->destActionParam->fight.waza, WAZAPARAM_TARGET );
  if(wk->destActionParam->fight.waza == WAZANO_NOROI){  // BTS:2957 のろい対処
    wk->selTargetType = BTL_CALC_GetNoroiTargetType( wk->bpp );
  }
  return TRUE;
}
static BOOL selectTarget_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;

  switch( *seq ){
  case 0:
    {
      const BTLV_INPUT_HITTBL *touch_data;
      const BTLV_INPUT_KEYTBL *key_data;
      int hit;
      u8  touch_max;
      u8  pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                          BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(wk->bpp) ) );
      u8  target = wk->selTargetType;

      GF_ASSERT( target < 14 );

      pos = ( pos - BTLV_MCSS_POS_A ) / 2;

      if( BTL_MAIN_GetRule( wk->mainModule ) != BTL_RULE_TRIPLE ){
        GF_ASSERT_MSG( ( ( pos > -1 ) && ( pos < 2 ) ), "pos:%d\n", pos );
        touch_data = PokeSeleMenuTouch4Data[ pos ][ target ];
        key_data = PokeSeleMenuKey4Data[ pos ][ target ];
        touch_max = 4;
      }else{
        GF_ASSERT_MSG( ( ( pos > -1 ) && ( pos < 3 ) ), "pos:%d\n", pos );
        //遠隔技のRANGEはWAZA_TARGET_MAXと同値にしている
        if( ( target == WAZA_TARGET_OTHER_SELECT ) &&
            ( WAZADATA_GetFlag( wk->destActionParam->fight.waza, WAZAFLAG_TripleFar ) ) )
        {
          target = WAZA_TARGET_MAX;
        }
        touch_data = PokeSeleMenuTouch6Data[ pos ][ target ];
        key_data = PokeSeleMenuKey6Data[ pos ][ target ];
        touch_max = 6;
      }

      hit = BTLV_INPUT_CheckInput( wk->biw, touch_data, key_data );

#ifdef DEBUG_ONLY_FOR_hudson
      //攻撃対象をスイッチ
      if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA2 ) && wkHudson.TrunCnt < 6*5+1 )
      {
        static const u8 tbl[6][5] =
        {
          { 5, 3, 2, 3, 5 },
          { 5, 3, 1, 4, 0 },
          { 3, 1, 2, 1, 3 },
          { 4, 2, 3, 2, 4 },
          { 0, 2, 4, 5, 1 },
          { 0, 2, 3, 2, 0 },
        };
        int id = wkHudson.TrunCnt-1;

        hit = tbl[ id%6 ][ id/6 ];
        OS_TPrintf("TAGET [%d]体目の[%d]番目 hit=%d\n",id%6, id/6, hit);
      }
#endif // DEBUG_ONLY_FOR_hudson

      if( hit != GFL_UI_TP_HIT_NONE )
      {
        BTL_Printf("hitBtn = %d\n", hit );
        if( hit != touch_max )
        {
          BtlPokePos targetPos;

          if( stw_is_enable_hitpos( &wk->selTargetWork, hit, wk->mainModule, &targetPos ) )
          {
            BTL_N_Printf( DBGSTR_SCD_DecideTargetPos, hit, wk->selTargetWork.pos[hit], targetPos);
            BTL_ACTION_SetFightParam( wk->destActionParam, wk->destActionParam->fight.waza, targetPos );
            wk->selTargetDone = TRUE;
            return TRUE;
          }
        }
        else
        {
          wk->selTargetDone = FALSE;
          return TRUE;
        }
      }
    }
    break;
  }
  return FALSE;
}

static void seltgt_init_setup_work( SEL_TARGET_WORK* stw, BTLV_SCD* wk )
{
  WazaID waza = wk->destActionParam->fight.waza;
  WazaTarget target = WAZADATA_GetParam( waza, WAZAPARAM_TARGET );
  BtlPokePos  basePos = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID(wk->bpp) );

  stw_init( stw );

  BTL_Printf("stw basePoke=%d, basePos=%d\n", BPP_GetID(wk->bpp), basePos);

  // 「のろい」は自分がゴーストタイプを持っているかどうかで対象範囲が異なる
  if( waza == WAZANO_NOROI ){
    target = BTL_CALC_GetNoroiTargetType( wk->bpp );
  }

  switch( target ){
  case WAZA_TARGET_OTHER_SELECT:        ///< 自分以外の１体（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_OTHERS, basePos) );
    stw_addFarPoke( stw, wk->mainModule, waza, basePos );
    break;
  case WAZA_TARGET_ENEMY_SELECT:        ///< 敵１体（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ENEMY, basePos) );
    stw_addFarPoke( stw, wk->mainModule, waza, basePos );
    break;
  case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_MYTEAM, basePos) );
    break;
  case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_FRIENDS, basePos) );
    break;

  case WAZA_TARGET_ENEMY_ALL:           ///< 敵側２体
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ENEMY, basePos) );
    break;
  case WAZA_TARGET_OTHER_ALL:           ///< 自分以外全部
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_OTHERS, basePos) );
    break;
  case WAZA_TARGET_USER:                ///< 自分１体のみ
  case WAZA_TARGET_ENEMY_RANDOM:        ///< 敵ランダム
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_DEFAULT, basePos) );
    break;

  case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣地
  case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
    //stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_MYTEAM, basePos) );
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_FULL_FRIENDS, basePos) );
    break;

  case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣地
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_FULL_ENEMY, basePos) );
    break;

  case WAZA_TARGET_FIELD:               ///< 場に効く（天候系など）
  case WAZA_TARGET_ALL:
    //stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ALL, basePos) );
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_FULL_ALL, basePos) );
    break;

  case WAZA_TARGET_UNKNOWN:             ///< 特殊系（ゆびをふる等）
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_DEFAULT, basePos) );
    break;

  default:
    GF_ASSERT_MSG(0, "illegal target(%d)", target);
    break;
  }
}
//=============================================================================================
//  ローテーション選択
//=============================================================================================
/**
 *  ローテーション選択 開始
 */
void BTLV_SCD_SelectRotate_Start( BTLV_SCD* wk, BTLV_INPUT_ROTATE_PARAM* param )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_ROTATE, param );
}
/**
 *  ローテーション選択 終了待ち
 */
BOOL BTLV_SCD_SelectRotate_Wait( BTLV_SCD* wk, BtlRotateDir* result )
{
  int input = BTLV_INPUT_CheckInput( wk->biw, NULL, NULL );
  if( input != GFL_UI_TP_HIT_NONE )
  {
    *result = input;
    return TRUE;
  }
  return FALSE;
}
/**
 *  ローテーション選択 コマンド選択時間制限による強制終了
 */
void BTLV_SCD_SelectRotate_ForceQuit( BTLV_SCD* wk )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
  BTLV_EFFECT_SetGaugeYure( BTLV_EFFECT_GAUGE_YURE_STOP );
  BTLV_EFFECT_SetCameraWorkStop();
}

//=============================================================================================
//  はい／いいえ選択
//=============================================================================================
#include "data\yesno_sel.cdat"

void BTLV_SCD_SelectYesNo_Start( BTLV_SCD* wk, BTLV_INPUT_YESNO_PARAM* param )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_YES_NO, param );
  wk->yesno_seq = 0;
}

BOOL BTLV_SCD_SelectYesNo_Wait( BTLV_SCD* wk, BtlYesNo* result )
{
  switch( wk->yesno_seq ){
  case 0:
    {
      int input = BTLV_INPUT_CheckInput( wk->biw, &YesNoTouchData, YesNoKeyData );
#ifdef DEBUG_ONLY_FOR_hudson
      // 自爆対策 次のポケモンを出さない
      if( HUDSON_IsTestCode( HUDSON_TESTCODE_ALL_WAZA ) )
      {
        input = 1;
      }
#endif // DEBUG_ONLY_FOR_hudson
      if( input != GFL_UI_TP_HIT_NONE )
      {
        wk->yesno_result = input;
        BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
        wk->yesno_seq++;
      }
    }
    break;

  case 1:
    if( !BTLV_INPUT_CheckExecute(wk->biw) ){
      *result = wk->yesno_result;
      return TRUE;
    }
    break;
  }
  return FALSE;
}




//=============================================================================================
//  カーソルフラグポインタ取得
//=============================================================================================
u8* BTLV_SCD_GetCursorFlagPtr( BTLV_SCD* wk )
{
  return &wk->cursor_flag;
}

//=============================================================================================
//  時間制限によるアクション選択の強制終了対応
//=============================================================================================
void   BTLV_SCD_ForceQuitInput_Notify( BTLV_SCD* wk )
{
  if( wk->fForceQuitDoing == FALSE ){
    wk->forceQuitSeq = 0;
    wk->fForceQuitDoing = TRUE;
  }
}


BOOL BTLV_SCD_ForceQuitInput_Wait( BTLV_SCD* wk )
{
  switch( wk->forceQuitSeq ){
  case 0:
    if( !BTLV_INPUT_CheckExecute( wk->biw ) ){
      wk->forceQuitSeq++;
    }
    break;
  case 1:
    BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
    BTLV_EFFECT_SetGaugeYure( BTLV_EFFECT_GAUGE_YURE_STOP );
    BTLV_EFFECT_SetCameraWorkStop();
    wk->forceQuitSeq++;
    break;
  case 2:
    if( !BTLV_INPUT_CheckExecute( wk->biw ) ){
      wk->forceQuitSeq++;
    }
    break;
  case 3:
    wk->fForceQuitDoing = FALSE;
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
//  録画再生モード
//=============================================================================================

void BTLV_SCD_SetupRecPlayerMode( BTLV_SCD* wk, const BTLV_INPUT_BATTLE_RECORDER_PARAM* param )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_BATTLE_RECORDER, (void*)param );
}

int BTLV_SCD_CheckRecPlayerInput( BTLV_SCD* wk )
{
  return BTLV_INPUT_CheckInput( wk->biw, &BattleRecorderTouchData, BattleRecorderKeyData );
}

