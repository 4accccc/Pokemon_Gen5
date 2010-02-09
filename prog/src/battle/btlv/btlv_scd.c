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

  BTL_PROC      subProc;
  BTL_PROC      subProcStack[ SUBPROC_STACK_DEPTH ];
  u32           subProcStackPtr;
  int           sub_seq;

  const BTL_POKEPARAM*  bpp;
  BTL_ACTION_PARAM*     destActionParam;

  const BTL_POKESELECT_PARAM*   pokesel_param;
  BTL_POKESELECT_RESULT*        pokesel_result;

  SEL_TARGET_WORK   selTargetWork;
  u8                selTargetDone;
  u8                selTargetType;
  u8                fActionPrevButton;
  s8                fadeValue;
  s8                fadeValueEnd;
  s8                fadeStep;
  u8                cursor_flag;    //下画面カーソルフラグ

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
static void stw_init( SEL_TARGET_WORK* stw );
static void stw_convert_pos_to_index( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, u8 num );
static void stw_setSelectablePoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmPoke( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static void stw_setConfirmField( SEL_TARGET_WORK* stw, const BTL_MAIN_MODULE* mainModule, BtlExPos exPos );
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, BtlPokePos* targetPos );
static inline u8 stwdraw_vpos_to_tblidx( u8 vpos );
static void stwdraw_button( const u8* pos, u8 count, u8 format, BTLV_SCD* wk );
static void stw_draw( const SEL_TARGET_WORK* stw, BTLV_SCD* work );
static BOOL stw_draw_wait( BTLV_SCD* wk );
static BOOL selectTarget_init( int* seq, void* wk_adrs );
static BOOL selectTarget_loop( int* seq, void* wk_adrs );
static void seltgt_init_setup_work( SEL_TARGET_WORK* stw, BTLV_SCD* wk );
static BOOL selectPokemon_init( int* seq, void* wk_adrs );
static BOOL selectPokemon_loop( int* seq, void* wk_adrs );
static void printCommWait( BTLV_SCD* wk );
static  inline  void  SePlayDecide( BTLV_SCD* wk );
static  inline  void  SePlayCancel( BTLV_SCD* wk );

BTLV_SCD*  BTLV_SCD_Create( const BTLV_CORE* vcore, const BTL_MAIN_MODULE* mainModule,
        const BTL_POKE_CONTAINER* pokeCon, GFL_TCBLSYS* tcbl, GFL_FONT* font, const BTL_CLIENT* client, HEAPID heapID )
{
  BTLV_SCD* wk = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_SCD) );

  wk->vcore = vcore;
  wk->mainModule = mainModule;
  wk->pokeCon = pokeCon;
  wk->heapID = heapID;
  wk->font = font;
  wk->clientWork = client;
  wk->playerParty = BTL_POKECON_GetPartyDataConst( pokeCon, BTL_CLIENT_GetClientID(client) );
  wk->strbuf = GFL_STR_CreateBuffer( 128, heapID );
  wk->pokesel_param = NULL;

  wk->cursor_flag = 0;

  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

  spstack_init( wk );

  return wk;
}

void BTLV_SCD_Init( BTLV_SCD* wk )
{
  wk->biw = BTLV_INPUT_Init( BTL_MAIN_GetRule(wk->mainModule), BTL_MAIN_GetCompetitor(wk->mainModule), wk->clientWork, wk->font, &wk->cursor_flag, wk->heapID );

  ///<obj
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );

  //GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );
//  PaletteWorkSet_Arc( BTLV_EFFECT_GetPfd(), ARCID_FONT, NARC_font_default_nclr, wk->heapID, FADE_SUB_BG, 0x10, 0xe0 );

  setupMainWin( wk );
}

//=============================================================================================
/**
 * フェードアウト開始
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_SCD_FadeOut( BTLV_SCD* wk )
{
  wk->fadeValue = 0;
  wk->fadeValueEnd = -16;
  wk->fadeStep = -1;
  BTLV_INPUT_SetFadeOut( wk->biw );
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

void BTLV_SCD_RestartUI( BTLV_SCD* wk )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
}

void BTLV_SCD_Delete( BTLV_SCD* wk )
{
  //GFL_BMPWIN_Delete( wk->win );

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
void BTLV_SCD_StartActionSelect( BTLV_SCD* wk, const BTL_POKEPARAM* bpp, BOOL fPrevButton, BTL_ACTION_PARAM* dest )
{
  wk->bpp = bpp;
  wk->destActionParam = dest;
  wk->selActionResult = BTL_ACTION_NULL;
  wk->fActionPrevButton = fPrevButton;

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

  spstack_push( wk, selectWaza_init, selectWaza_loop );
}
//=============================================================================================
/**
 * ワザ選択処理 終了待ち
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
#include "msg/msg_d_soga.h"
#include "message.naix"

static BOOL selectAction_init( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  BTLV_INPUT_COMMAND_PARAM  bicp;
  const BTL_PARTY* party;
  const BTL_POKEPARAM* bpp;
  const POKEMON_PARAM* pp;
  u16 members, hp, i, j;

  MI_CpuClear16( &bicp, sizeof( BTLV_INPUT_COMMAND_PARAM ) );

  bicp.pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule,
                                       BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, BPP_GetID( wk->bpp ) ) );

  for( j = 0 ; j < 2 ; j++ )
  {
    if( j == 0 )
    {
      //自分の手持ち
      party = wk->playerParty;
      members = BTL_PARTY_GetMemberCount( party );

      switch( BTL_MAIN_GetRule( wk->mainModule ) ){
      case BTL_RULE_SINGLE:    ///< シングル
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_AA ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 0 ] = BPP_GetMonsNo( bpp );
          bicp.form_no[ 0 ] = BPP_GetValue( bpp, BPP_FORM );
        }
        break;
      case BTL_RULE_TRIPLE:    ///< トリプル
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_E ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 2 ] = BPP_GetMonsNo( bpp );
          bicp.form_no[ 2 ] = BPP_GetValue( bpp, BPP_FORM );
        }
      /* fallthru */
      case BTL_RULE_DOUBLE:    ///< ダブル
      case BTL_RULE_ROTATION:   ///< ローテーション @todo これでいいのか？
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_A ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 0 ] = BPP_GetMonsNo( bpp );
          bicp.form_no[ 0 ] = BPP_GetValue( bpp, BPP_FORM );
        }
        bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, BTL_MAIN_ViewPosToBtlPos( wk->mainModule, BTLV_MCSS_POS_C ) );
        if( BPP_IsDead( bpp ) == FALSE )
        {
          bicp.mons_no[ 1 ] = BPP_GetMonsNo( bpp );
          bicp.form_no[ 1 ] = BPP_GetValue( bpp, BPP_FORM );
        }
        bicp.pos = ( bicp.pos - BTLV_MCSS_POS_A ) / 2;
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
      bicp.trainer_flag = TRUE;
      //相手の手持ち
      party = BTL_POKECON_GetPartyDataConst( wk->pokeCon,
                                             BTL_MAIN_GetOpponentClientID( wk->mainModule,
                                             BTL_MAIN_GetPlayerClientID( wk->mainModule ), 0 ) );
      members = BTL_PARTY_GetMemberCount( party );
    }

    for(i=0; i<members; ++i)
    {
      bpp = BTL_PARTY_GetMemberDataConst( party, i );
      pp  = BPP_GetSrcData( bpp );
      hp = PP_Get( pp, ID_PARA_hp, NULL );

      if( hp )
      {
        if( PP_Get( pp, ID_PARA_condition, NULL ) ){
          bicp.bidp[ j ][ i ].status = BTLV_INPUT_STATUS_NG;
        }
        else{
          bicp.bidp[ j ][ i ].status = BTLV_INPUT_STATUS_ALIVE;
        }
      }
      else{
        bicp.bidp[ j ][ i ].status = BTLV_INPUT_STATUS_DEAD;
      }
    }
  }

  bicp.center_button_type = wk->fActionPrevButton;

  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_COMMAND, &bicp );

  return TRUE;
}

static BOOL selectActionRoot_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  int hit;

  //カメラワークエフェクト
  if( !BTLV_EFFECT_CheckExecute() ){
    BTLV_EFFECT_Add( BTLEFF_CAMERA_WORK );
  }

//  hit = GFL_UI_TP_HitTrg( BattleMenuTouchData );
  hit = BTLV_INPUT_CheckInput( wk->biw, BattleMenuTouchData, BattleMenuKeyData );
  if( hit != GFL_UI_TP_HIT_NONE )
  {
    static const u8 action[] = {
      BTL_ACTION_FIGHT,   BTL_ACTION_ITEM,
      BTL_ACTION_CHANGE,  BTL_ACTION_ESCAPE,
    };

    wk->selActionResult = action[ hit ];

    switch( wk->selActionResult ){
    // @@@ 現状アイテム選択はクライアント側で状況に応じて勝手に行っている
    case BTL_ACTION_ITEM:
      break;

    case BTL_ACTION_ESCAPE:
      BTL_ACTION_SetEscapeParam( wk->destActionParam );
      BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
      break;
    }

    SePlayDecide( wk );
    return TRUE;
  }

  return FALSE;
}

static BOOL selectDemoRoot_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;
  BOOL ret = FALSE;

  //カメラワークエフェクト（捕獲デモはしなくていいよね？）
  //if( !BTLV_EFFECT_CheckExecute() ){
  //  BTLV_EFFECT_Add( BTLEFF_CAMERA_WORK );
  //}

  if( ( ret = BTLV_INPUT_CheckInputDemo( wk->biw ) ) == TRUE )
  {
    wk->selActionResult = BTL_ACTION_FIGHT;
    SePlayDecide( wk );
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
  const GFL_UI_TP_HITTBL* SkillMenuTouchData = ( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) &&
                                                 ( pos != BTLV_MCSS_POS_C ) ) ?
                                                SkillMenuTouchData3vs3 :
                                                SkillMenuTouchDataNormal;
  const BTLV_INPUT_KEYTBL* SkillMenuKeyData = ( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) &&
                                                ( pos != BTLV_MCSS_POS_C ) ) ?
                                                SkillMenuKeyData3vs3 :
                                                SkillMenuKeyDataNormal;
  //カメラワークエフェクト
  if( !BTLV_EFFECT_CheckExecute() ){
    BTLV_EFFECT_Add( BTLEFF_CAMERA_WORK );
  }

//  hit = GFL_UI_TP_HitTrg( SkillMenuTouchData );
  hit = BTLV_INPUT_CheckInput( wk->biw, SkillMenuTouchData, SkillMenuKeyData );
  if( hit != GFL_UI_TP_HIT_NONE )
  {
    //キャンセルが押された
    if( hit == 4 )
    {
      SePlayCancel( wk );
      BTL_ACTION_SetNULL( wk->destActionParam );
      return TRUE;
    }
    else if( hit < BPP_WAZA_GetCount( wk->bpp ) )
    {
      WazaID waza;

      waza = BPP_WAZA_GetID( wk->bpp, hit );
      BTL_ACTION_SetFightParam( wk->destActionParam, waza, BTL_POS_NULL );

      SePlayDecide( wk );
      return TRUE;
    }
    else if( ( BTL_MAIN_GetRule(wk->mainModule) == BTL_RULE_TRIPLE ) && ( hit == 5 ) )
    {
      BTL_ACTION_SetMoveParam( wk->destActionParam );
      SePlayDecide( wk );
      return TRUE;
    }
  }

  return FALSE;
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
//
static BOOL stw_is_enable_hitpos( SEL_TARGET_WORK* stw, int hitPos, const BTL_MAIN_MODULE* mainModule, BtlPokePos* targetPos )
{
  GF_ASSERT_MSG(hitPos<NELEMS(STW_HitTblIndex), "hitPos=%d", hitPos);

  if( stw->selectablePokeCount )
  {
    u8 hitVpos = STW_HitTblIndex[ hitPos ];
    u8 i;

    *targetPos = BTL_MAIN_ViewPosToBtlPos( mainModule, hitVpos );

    BTL_Printf( "hitPos=%d, vpos=%d, btlPos=%d\n", hitPos, hitVpos, *targetPos );
    for(i=0; i<stw->selectablePokeCount; ++i)
    {
      if( *targetPos == stw->pos[i] )
      {
        BTL_Printf( " btlPos[%d]  Selectable !\n", *targetPos );
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
  //@todo ただしく遠隔フラグが立っていないので、今回は見送る
#if 0
  if( bisp.waza_target == WAZA_TARGET_OTHER_SELECT )
  {
    if( WAZADATA_GetFlag( waza, WAZAFLAG_TripleFar ) )
    {
      bisp.waza_target = WAZA_TARGET_LONG_SELECT;
    }
  }
#endif

  BTL_Printf(" **** triple select **** \n");
  while( count-- )
  {
    bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, *pos );
    pp  = BPP_GetSrcData( bpp );
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
        bisp.bidp[ vpos ].sex = BPP_GetValue( bpp, BPP_SEX );
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

  // @@@ トリプルはターゲット選択画面を新たに作る必要あるので、仮処理。
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
  return TRUE;
}
static BOOL selectTarget_loop( int* seq, void* wk_adrs )
{
  BTLV_SCD* wk = wk_adrs;

  switch( *seq ){
  case 0:
    {
      const GFL_UI_TP_HITTBL *touch_data;
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
        touch_data = PokeSeleMenuTouch4Data;
        key_data = PokeSeleMenuKey4Data[ pos ][ target ];
        touch_max = 4;
      }else{
        GF_ASSERT_MSG( ( ( pos > -1 ) && ( pos < 3 ) ), "pos:%d\n", pos );
        touch_data = PokeSeleMenuTouch6Data;
        key_data = PokeSeleMenuKey6Data[ pos ][ target ];
        touch_max = 6;
      }

      hit = BTLV_INPUT_CheckInput( wk->biw, touch_data, key_data );
      if( hit != GFL_UI_TP_HIT_NONE )
      {
        BTL_Printf("hitBtn = %d\n", hit );
        if( hit != touch_max )
        {
          BtlPokePos targetPos;

          SePlayDecide( wk );
          if( stw_is_enable_hitpos( &wk->selTargetWork, hit, wk->mainModule, &targetPos ) )
          {
            BTL_Printf("ターゲット決定 ... hitBtn=%d, hitPos=%d, targetPos=%d\n", hit, wk->selTargetWork.pos[hit], targetPos);
            BTL_ACTION_SetFightParam( wk->destActionParam, wk->destActionParam->fight.waza, targetPos );
            wk->selTargetDone = TRUE;
            return TRUE;
          }
        }
        else
        {
          SePlayCancel( wk );
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
    break;
  case WAZA_TARGET_ENEMY_SELECT:        ///< 敵１体（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ENEMY, basePos) );
    break;
  case WAZA_TARGET_FRIEND_USER_SELECT:  ///< 自分を含む味方ポケ（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_MYTEAM, basePos) );
    break;
  case WAZA_TARGET_FRIEND_SELECT:       ///< 自分以外の味方ポケ（選択）
    stw_setSelectablePoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_FRIENDS, basePos) );
    break;

  case WAZA_TARGET_ENEMY_ALL:           ///< 敵側２体
  case WAZA_TARGET_ENEMY_RANDOM:        ///< 敵ランダム
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ENEMY, basePos) );
    break;
  case WAZA_TARGET_OTHER_ALL:           ///< 自分以外全部
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_OTHERS, basePos) );
    break;
  case WAZA_TARGET_USER:                ///< 自分１体のみ
    stw_setConfirmPoke( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_DEFAULT, basePos) );
    break;

  case WAZA_TARGET_SIDE_FRIEND:         ///< 自分側陣地
  case WAZA_TARGET_FRIEND_ALL:          ///< 味方側全ポケ
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_MYTEAM, basePos) );
    break;

  case WAZA_TARGET_SIDE_ENEMY:          ///< 敵側陣地
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ENEMY, basePos) );
    break;

  case WAZA_TARGET_FIELD:               ///< 場に効く（天候系など）
  case WAZA_TARGET_ALL:
    stw_setConfirmField( stw, wk->mainModule, EXPOS_MAKE(BTL_EXPOS_AREA_ALL, basePos) );
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
}

//=============================================================================================
//  はい／いいえ選択
//=============================================================================================
#include "data\yesno_sel.cdat"

void BTLV_SCD_SelectYesNo_Start( BTLV_SCD* wk, BTLV_INPUT_YESNO_PARAM* param )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_YES_NO, param );
}

BOOL BTLV_SCD_SelectYesNo_Wait( BTLV_SCD* wk, BtlYesNo* result )
{
  int input = BTLV_INPUT_CheckInput( wk->biw, YesNoTouchData, YesNoKeyData );
  if( input != GFL_UI_TP_HIT_NONE )
  {
    SePlayDecide( wk );
    *result = input;
    BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
    return TRUE;
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
//  決定音再生
//=============================================================================================
static  inline  void  SePlayDecide( BTLV_SCD* wk )
{
  if( BTL_MAIN_GetCompetitor( wk->mainModule ) != BTL_COMPETITOR_COMM )
  {
    PMSND_PlaySE( SEQ_SE_DECIDE2 );
  }
}

//=============================================================================================
//  キャンセル音再生
//=============================================================================================
static  inline  void  SePlayCancel( BTLV_SCD* wk )
{
  if( BTL_MAIN_GetCompetitor( wk->mainModule ) != BTL_COMPETITOR_COMM )
  {
    PMSND_PlaySE( SEQ_SE_CANCEL2 );
  }
}

//=============================================================================================
//  時間制限によるアクション選択の強制終了対応
//=============================================================================================
void   BTLV_SCD_ForceQuitInput_Notify( BTLV_SCD* wk )
{
  BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
}


BOOL BTLV_SCD_ForceQuitInput_Wait( BTLV_SCD* wk )
{
  return TRUE;
}

