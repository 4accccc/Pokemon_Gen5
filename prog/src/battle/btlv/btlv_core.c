//=============================================================================================
/**
 * @file  btlv_core.c
 * @brief ポケモンWB バトル描画メインモジュール
 * @author  taya
 *
 * @date  2008.10.02  作成
 */
//=============================================================================================
#include <tcbl.h>

#include "print/gf_font.h"
#include "waza_tool/wazano_def.h"   //soga
#include "arc_def.h"
#include "sound/pm_sndsys.h"
#include "font/font.naix"


#include "battle/btl_common.h"
#include "battle/btl_main.h"
#include "battle/btl_action.h"
#include "battle/btl_calc.h"
#include "battle/btl_util.h"
#include "../app/b_bag.h"
#include "../app/b_plist.h"

#include "btlv_scu.h"
#include "btlv_scd.h"
#include "btlv_effect.h"
#include "btlv_input.h"

#include "btlv_core.h"


//バトル描画とバトルバッグとバトルポケモンリストをオーバーレイ化
//FS_EXTERN_OVERLAY(battle_view);
FS_EXTERN_OVERLAY(battle_b_app);
FS_EXTERN_OVERLAY(battle_bag);
FS_EXTERN_OVERLAY(battle_plist);
FS_EXTERN_OVERLAY(vram_h);

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  STR_BUFFER_SIZE = 384,
  GENERIC_WORK_SIZE = 128,
};

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*pCmdProc)( BTLV_CORE*, int*, void* );

/*--------------------------------------------------------------------------*/
/* Structures                                                               */
/*--------------------------------------------------------------------------*/
struct _BTLV_CORE {

  BTL_MAIN_MODULE*          mainModule;
  const BTL_CLIENT*         myClient;
  const BTL_POKE_CONTAINER* pokeCon;
  u8                        myClientID;

  BtlvCmd   processingCmd;
  pCmdProc  mainProc;
  int       mainSeq;
  BTL_PROC  subProc;
  u8        genericWork[ GENERIC_WORK_SIZE ];

  STRBUF*           strBuf;
  GFL_FONT*         largeFontHandle;
  GFL_FONT*         smallFontHandle;
  BTL_ACTION_PARAM* actionParam;
  const BTL_POKEPARAM*  procPokeParam;
  u32                   procPokeID;
  BtlAction             playerAction;
  BBAG_DATA             bagData;
  BPLIST_DATA           plistData;
  BTL_POKESELECT_RESULT* pokeselResult;
  u8                    selectItemSeq;
  u8                    fActionPrevButton;


  GFL_TCBLSYS*  tcbl;
  BTLV_SCU*     scrnU;
  BTLV_SCD*     scrnD;

  HEAPID  heapID;
};

/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static void* getGenericWork( BTLV_CORE* core, u32 size );
static BOOL CmdProc_Setup( BTLV_CORE* core, int* seq, void* workBuffer );
static BOOL CmdProc_SetupDemo( BTLV_CORE* core, int* seq, void* workBuffer );
static BOOL CmdProc_SelectAction( BTLV_CORE* core, int* seq, void* workBufer );
static BOOL CmdProc_SelectWaza( BTLV_CORE* core, int* seq, void* workBufer );
static BOOL CmdProc_SelectTarget( BTLV_CORE* core, int* seq, void* workBufer );
static void mainproc_setup( BTLV_CORE* core, pCmdProc proc );
static BOOL mainproc_call( BTLV_CORE* core );
static BOOL subprocDamageEffect( int* seq, void* wk_adrs );
static BOOL subprocMemberIn( int* seq, void* wk_adrs );
static void setup_core( BTLV_CORE* wk, HEAPID heapID );
static void cleanup_core( BTLV_CORE* wk );
static BOOL subprocMoveMember( int* seq, void* wk_adrs );
static BOOL subprocRotateMember( int* seq, void* wk_adrs );

static  const BTL_POKEPARAM*  get_btl_pokeparam( BTLV_CORE* core, BtlvMcssPos pos );

//=============================================================================================
/**
 * 描画メインモジュールの生成
 *
 * @param   mainModule    システムメインモジュールのハンドラ
 * @param   heapID      生成先ヒープID
 *
 * @retval  BTLV_CORE*    生成された描画メインモジュールのハンドラ
 */
//=============================================================================================
BTLV_CORE*  BTLV_Create( BTL_MAIN_MODULE* mainModule, const BTL_CLIENT* client, const BTL_POKE_CONTAINER* pokeCon, HEAPID heapID )
{
  BTLV_CORE* core = GFL_HEAP_AllocMemory( heapID, sizeof(BTLV_CORE) );

  core->mainModule = mainModule;
  core->myClient = client;
  core->pokeCon = pokeCon;
  core->myClientID = BTL_CLIENT_GetClientID( client );
  core->processingCmd = BTLV_CMD_NULL;
  core->heapID = heapID;
  core->strBuf = GFL_STR_CreateBuffer( STR_BUFFER_SIZE, heapID );
  core->largeFontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
            GFL_FONT_LOADTYPE_FILE, FALSE, heapID );
  core->smallFontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_small_batt_gftr,
            GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

//  BTL_Printf("フォント高さ=%d, %d\n",
//    GFL_FONT_GetLineHeight(core->largeFontHandle), GFL_FONT_GetLineHeight(core->smallFontHandle));

  core->tcbl = GFL_TCBL_Init( heapID, heapID, 64, 128 );

  core->scrnU = BTLV_SCU_Create( core, core->mainModule, pokeCon, core->tcbl,
          core->largeFontHandle, core->smallFontHandle, core->myClientID, heapID );

  core->scrnD = BTLV_SCD_Create( core, core->mainModule, pokeCon, core->tcbl,
          core->largeFontHandle, core->myClient, heapID );

  core->mainProc = NULL;
  core->mainSeq = 0;
  core->selectItemSeq = 0;

  BTL_STR_InitSystem( mainModule, client, pokeCon, heapID );

  //描画系オーバーレイロード
//  GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_view ) );

  return core;
}

//=============================================================================================
/**
 * 描画メインモジュールの破棄
 *
 * @param   core    描画メインモジュール
 *
 */
//=============================================================================================
void BTLV_Delete( BTLV_CORE* core )
{
  BTL_STR_QuitSystem();

  BTLV_SCD_Delete( core->scrnD );
  BTLV_SCU_Delete( core->scrnU );
  cleanup_core( core );

  GFL_TCBL_Exit( core->tcbl );
  GFL_STR_DeleteBuffer( core->strBuf );
  GFL_FONT_Delete( core->largeFontHandle );
  GFL_HEAP_FreeMemory( core );

  //描画系オーバーレイアンロード
//  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_view ) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( vram_h ) );
}

//=============================================================================================
/**
 * 描画メインループ
 *
 * @param   core
 *
 */
//=============================================================================================
void BTLV_CORE_Main( BTLV_CORE* core )
{
  GFL_TCBL_Main( core->tcbl );
  BTLV_EFFECT_Main();   //soga
}


//=============================================================================================
/**
  * 描画コマンド発行
 *
 * @param   core    描画メインモジュールのハンドラ
 * @param   client    関連クライアントモジュールハンドラ
 * @param   cmd     描画コマンド
 *
 */
//=============================================================================================
void BTLV_StartCommand( BTLV_CORE* core, BtlvCmd cmd )
{
  GF_ASSERT( core->processingCmd == BTLV_CMD_NULL );

  {
    static const struct {
      BtlvCmd   cmd;
      pCmdProc  proc;
    }procTbl[] = {
      { BTLV_CMD_SETUP,           CmdProc_Setup },
      { BTLV_CMD_SETUP_DEMO,      CmdProc_SetupDemo },
    };

    int i;

    for(i=0; i<NELEMS(procTbl); i++)
    {
      if( procTbl[i].cmd == cmd )
      {
        core->processingCmd = cmd;
        core->mainProc = procTbl[i].proc;
        core->mainSeq = 0;
        return;
      }
    }

    GF_ASSERT(0);
  }
}

//=============================================================================================
/**
 * BTLV_StartCommand で受け付けた描画コマンドの終了を待つ
 *
 * @param   core      描画メインモジュールのハンドラ
 *
 * @retval  BOOL    終了していたらTRUE
 */
//=============================================================================================
BOOL BTLV_WaitCommand( BTLV_CORE* core )
{
  if( core->processingCmd != BTLV_CMD_NULL )
  {
    if( core->mainProc(core, &core->mainSeq, core->genericWork) )
    {
      core->processingCmd = BTLV_CMD_NULL;
      core->mainProc = NULL;
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}


static void* getGenericWork( BTLV_CORE* core, u32 size )
{
  GF_ASSERT(size<sizeof(core->genericWork));

  return core->genericWork;
}


//----------------------------------------------------------------------------------
/**
 * バトル画面構築用VRAM設定～双方のポケモン登場まで
 *
 * @param   core
 * @param   seq
 * @param   workBuffer
 *
 * @retval  BOOL        終了時TRUE
 */
//----------------------------------------------------------------------------------
static BOOL CmdProc_Setup( BTLV_CORE* core, int* seq, void* workBuffer )
{
  switch( *seq ){
  case 0:
    setup_core( core, core->heapID );
    BTLV_EFFECT_Init( BTL_MAIN_GetRule( core->mainModule ), BTL_MAIN_GetFieldSituation( core->mainModule ),
                      core->smallFontHandle, core->heapID );
    BTLV_SCU_Setup( core->scrnU );
    BTLV_SCD_Init( core->scrnD );
    (*seq)++;
    break;

  case 1:
    BTLV_SCU_StartBtlIn( core->scrnU );
    (*seq)++;
    break;

  case 2:
    if( BTLV_SCU_WaitBtlIn( core->scrnU ) )
    {
      return TRUE;
    }
    break;
  }

  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * 捕獲デモ
 */
//----------------------------------------------------------------------------------
#include "message.naix"
#include "msg/msg_capturedemo.h"
typedef struct
{ 
  GFL_MSGDATA*  msg;
  int           wait;
}CAPTURE_DEMO_WORK;

enum
{ 
  CAPTURE_DEMO_MSG_WAIT = 60,
};

//----------------------------------------------------------------------------------
/**
 * バトル画面構築用VRAM設定～捕獲デモ～
 *
 * @param   core
 * @param   seq
 * @param   workBuffer
 *
 * @retval  BOOL        終了時TRUE
 */
//----------------------------------------------------------------------------------
static BOOL CmdProc_SetupDemo( BTLV_CORE* core, int* seq, void* workBuffer )
{
  CAPTURE_DEMO_WORK*  cdw = ( CAPTURE_DEMO_WORK* )workBuffer; 
  switch( *seq ){
  case 0:
    setup_core( core, core->heapID );
    BTLV_EFFECT_Init( BTL_MAIN_GetRule( core->mainModule ), BTL_MAIN_GetFieldSituation( core->mainModule ),
                      core->smallFontHandle, core->heapID );
    BTLV_SCU_Setup( core->scrnU );
    BTLV_SCD_Init( core->scrnD );
    cdw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_capturedemo_dat, core->heapID );
    cdw->wait = 0;
    (*seq)++;
    break;

  case 1:
    BTLV_SCU_StartBtlIn( core->scrnU );
    (*seq)++;
    break;

  case 2:
    if( BTLV_SCU_WaitBtlIn( core->scrnU ) )
    {
      BTLV_STRPARAM sp;
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_AA );
      // 「○○はどうする？」表示
      BTLV_STRPARAM_Setup( &sp, BTL_STRTYPE_STD, BTL_STRID_STD_SelectAction );
      BTLV_STRPARAM_AddArg( &sp, BPP_GetID( pp ) );
      BTLV_STRPARAM_SetWait( &sp, 0 );
      BTLV_StartMsg( core, &sp );
      (*seq)++;
    }
    break;
  case 3:
    if( BTLV_WaitMsg( core ) )
    {
      core->procPokeParam = get_btl_pokeparam( core, BTLV_MCSS_POS_AA );
      core->procPokeID = BPP_GetID( core->procPokeParam );
      core->actionParam = NULL;
      core->playerAction = BTL_ACTION_NULL;
      core->fActionPrevButton = FALSE;
      BTLV_SCD_StartActionSelectDemoCapture( core->scrnD, core->procPokeParam, core->fActionPrevButton, core->actionParam );
      (*seq)++;
    }
    break;
  case 4:
    //下画面演出終了待ち
    if( BTLV_SCD_WaitActionSelect( core->scrnD ) != BTL_ACTION_NULL )
    { 
      BTLV_SCD_StartWazaSelectDemoCapture( core->scrnD, core->procPokeParam, core->actionParam );
      (*seq)++;
    }
    break;
  case 5:
    //下画面演出終了待ち
    if( BTLV_SCD_WaitActionSelect( core->scrnD ) != BTL_ACTION_NULL )
    {
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_AA );
      BTLV_StartMsgWaza( core, BPP_GetID( pp ), WAZANO_HATAKU );
      BTLV_SCD_RestartUI( core->scrnD );
      (*seq)++;
    }
    break;
  case 6:
    if( BTLV_WaitMsg( core ) )
    {
      BTLV_SCU_StartWazaEffect( core->scrnU, BTLV_MCSS_POS_AA, BTLV_MCSS_POS_BB, WAZANO_HATAKU, BTLV_WAZAEFF_TURN_DEFAULT, 0 );
      (*seq)++;
    }
    break;
  case 7:
    if( BTLV_SCU_WaitWazaEffect( core->scrnU ) )
    { 
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_BB );
      int value = BPP_GetValue( pp, BPP_HP ) - 5;

      BTLV_EFFECT_CalcGaugeHP( BTLV_MCSS_POS_BB, value );
      BTLV_EFFECT_Damage( BTLV_MCSS_POS_BB, WAZANO_HATAKU );
      PMSND_PlaySE( SEQ_SE_KOUKA_M );
      (*seq)++;
    }
    break;
  case 8:
    if( BTLV_SCU_WaitWazaDamageAct( core->scrnU ) )
    { 
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_BB );
      BTLV_StartMsgWaza( core, BPP_GetID( pp ), WAZANO_NIRAMITUKERU );
      (*seq)++;
    }
    break;
  case 9:
    if( BTLV_WaitMsg( core ) )
    { 
      BTLV_SCU_StartWazaEffect( core->scrnU, BTLV_MCSS_POS_BB, BTLV_MCSS_POS_AA, WAZANO_NIRAMITUKERU,
                                BTLV_WAZAEFF_TURN_DEFAULT, 0 );
      (*seq)++;
    }
    break;
  case 10:
    if( BTLV_SCU_WaitWazaEffect( core->scrnU ) )
    { 
      BTLV_StartRankDownEffect( core, BTLV_MCSS_POS_AA );
      (*seq)++;
    }
    break;
  case 11:
    if( BTLV_WaitRankEffect( core, BTLV_MCSS_POS_AA ) )
    {
      BTLV_STRPARAM sp;
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_AA );
      BTLV_STRPARAM_Setup( &sp, BTL_STRTYPE_SET, BTL_STRID_SET_Rankdown_DEF );
      BTLV_STRPARAM_AddArg( &sp, BPP_GetID( pp ) );
      BTLV_STRPARAM_SetWait( &sp, 0 );
      BTLV_StartMsg( core, &sp );
      (*seq)++;
    }
    break;
  case 12:
    if( BTLV_WaitMsg( core ) )
    { 
      //たいりょくがへったでしょメッセージ
      GFL_MSG_GetString( cdw->msg, msg_cappture_demo_01, core->strBuf );
      BTLV_SCU_StartMsg( core->scrnU, core->strBuf, 0 );
      cdw->wait = CAPTURE_DEMO_MSG_WAIT;
      (*seq)++;
    }
    break;
  case 13:
    if( BTLV_WaitMsg( core ) )
    { 
      if( --cdw->wait == 0 )
      { 
        BTLV_SCD_StartActionSelectDemoCapture( core->scrnD, core->procPokeParam, core->fActionPrevButton, core->actionParam );
        (*seq)++;
      }
    }
    break;
  case 14:
    //下画面演出終了待ち
    if( BTLV_SCD_WaitActionSelect( core->scrnD ) != BTL_ACTION_NULL )
    { 
      BTLV_ITEMSELECT_Start( core, BBAG_MODE_GETDEMO, 0, 0 );
      (*seq)++;
    }
    break;
  case 15:
    if( BTLV_ITEMSELECT_Wait( core ) )
    { 
      //ボールなげメッセージ
      GFL_MSG_GetString( cdw->msg, msg_cappture_demo_02, core->strBuf );
      BTLV_SCU_StartMsg( core->scrnU, core->strBuf, 0 );
      (*seq)++;
    }
    break;
  case 16:
    if( BTLV_WaitMsg( core ) )
    { 
      BTLV_EFFECT_BallThrow( BTLV_MCSS_POS_BB, ITEM_MONSUTAABOORU, 3, TRUE );
      (*seq)++;
    }
    break;
  case 17:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      BTLV_STRPARAM sp;
      const BTL_POKEPARAM*  pp  = get_btl_pokeparam( core, BTLV_MCSS_POS_BB );
      BTLV_STRPARAM_Setup( &sp, BTL_STRTYPE_STD, BTL_STRID_STD_BallThrowS );
      BTLV_STRPARAM_AddArg( &sp, BPP_GetID( pp ) );
      BTLV_StartMsg( core, &sp );
      (*seq)++;
    }
    break;
  case 18:
    if( BTLV_IsJustDoneMsg( core ) )
    {
      PMSND_PlayBGM( SEQ_ME_POKEGET );
    }
    if( BTLV_WaitMsg( core ) )
    {
      (*seq)++;
    }
    break;
  case 19:
    if( !PMSND_CheckPlayBGM() )
    { 
      GFL_MSG_Delete( cdw->msg );
      (*seq)++;
    }
    break;
  case 20:
    return TRUE;
    break;
  }

  return FALSE;
}

static  const BTL_POKEPARAM*  get_btl_pokeparam( BTLV_CORE* core, BtlvMcssPos pos )
{ 
  BtlPokePos  pokePos = BTL_MAIN_ViewPosToBtlPos( core->mainModule, pos );
  return  BTL_POKECON_GetFrontPokeDataConst( core->pokeCon, pokePos );
}

//-------------------------------------------

static void setup_core( BTLV_CORE* wk, HEAPID heapID )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_D,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,           // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_01_AB,          // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
  };

  // BGsystem初期化
  GFL_BG_Init( heapID );
  GFL_BMPWIN_Init( heapID );
  GFL_FONTSYS_Init();

//  GFL_BG_DebugPrintCtrl( GFL_BG_SUB_DISP, TRUE );

  // VRAMバンク設定
  GFL_DISP_SetBank( &vramBank );

  // 各種効果レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  //3D関連初期化 soga
  {
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, heapID, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    G3X_AlphaBlend( TRUE );
//    G3X_AlphaTest( TRUE, 31 );    //alpha0でワイヤーフレームにならないようにする
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( FALSE );
    G3X_SetFog( FALSE, 0, 0, 0 );
    G2_SetBG0Priority( 1 );
  }
  //ウインドマスク設定（画面両端のエッジマーキングのゴミを消す）soga
  {
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 |
                 GX_WND_PLANEMASK_BG1 |
                 GX_WND_PLANEMASK_BG2 |
                 GX_WND_PLANEMASK_BG3 |
                 GX_WND_PLANEMASK_OBJ,
                 TRUE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, TRUE );
    G2_SetWnd0Position( 1, 0, 255, 192 );
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
    G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1,
                      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 |
                      GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
                      31, 3 );
  }

  {
    static  const GFL_CLSYS_INIT clsysinit = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
//      48,48,48,48,
      56,48,48,48,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &clsysinit, &vramBank, heapID );
  }

//  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );


  GX_SetBankForLCDC( GX_VRAM_LCDC_H );
  GFL_OVERLAY_Load( FS_OVERLAY_ID( vram_h ) );
}
static void cleanup_core( BTLV_CORE* wk )
{
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  //エフェクト削除 soga
  BTLV_EFFECT_Exit();

  //セルアクター削除
  GFL_CLACT_SYS_Delete();

  //3D関連削除 soga
  GFL_G3D_Exit();
}

static BOOL CmdProc_SelectAction( BTLV_CORE* core, int* seq, void* workBufer )
{
  switch( *seq ){
  case 0:
    BTLV_SCD_StartActionSelect( core->scrnD, core->procPokeParam,
        core->fActionPrevButton, core->actionParam );
    (*seq)++;
  case 1:
    core->playerAction = BTLV_SCD_WaitActionSelect( core->scrnD );
    if( core->playerAction != BTL_ACTION_NULL ){
      return TRUE;
    }
  }
  return FALSE;
}

static BOOL CmdProc_SelectWaza( BTLV_CORE* core, int* seq, void* workBufer )
{
  switch( *seq ){
  case 0:
    BTLV_SCD_StartWazaSelect( core->scrnD, core->procPokeParam, core->actionParam );
    (*seq)++;
    break;
  case 1:
    if( BTLV_SCD_WaitWazaSelect( core->scrnD ) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//--------------------------------------------------------------------------
/**
 * ワザ対象選択
 *
 * @param   core
 * @param   seq
 * @param   workBufer
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------
static BOOL CmdProc_SelectTarget( BTLV_CORE* core, int* seq, void* workBufer )
{
  switch( *seq ){
  case 0:
    BTLV_SCD_StartTargetSelect( core->scrnD, core->procPokeParam, core->actionParam );
    (*seq)++;
    break;
  case 1:
    if( BTLV_SCD_WaitTargetSelect(core->scrnD) )
    {
      return TRUE;
    }
    break;
  }
  return FALSE;
}

//--------------------------------------------------------------------------
/**
 * メインプロセス設定
 *
 * @param   core
 * @param   proc
 *
 */
//--------------------------------------------------------------------------
static void mainproc_setup( BTLV_CORE* core, pCmdProc proc )
{
  GF_ASSERT(core);
  GF_ASSERT_MSG( core->mainProc == NULL, "mainProc=%p\n", core->mainProc );

  core->mainProc = proc;
  core->mainSeq = 0;
}
//--------------------------------------------------------------------------
/**
 * メインプロセスコール
 *
 * @param   core
 *
 * @retval  BOOL
 */
//--------------------------------------------------------------------------
static BOOL mainproc_call( BTLV_CORE* core )
{
  if( core->mainProc != NULL )
  {
    if( core->mainProc(core, &core->mainSeq, core->genericWork) )
    {
      core->mainProc = NULL;
      return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}


//=============================================================================================
/**
 * アクション選択開始
 *
 * @param   core
 * @param   bpp           対象ポケモンデータ
 * @param   fPrevButton   TRUEなら「もどる」表示／FALSEなら「にげる」表示
 * @param   dest          [out] 行動結果パラメータ
 */
//=============================================================================================
void BTLV_UI_SelectAction_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BOOL fPrevButton, BTL_ACTION_PARAM* dest )
{
  core->procPokeParam = bpp;
  core->procPokeID = BPP_GetID( bpp );
  core->actionParam = dest;
  core->playerAction = BTL_ACTION_NULL;
  core->fActionPrevButton = fPrevButton;
  mainproc_setup( core, CmdProc_SelectAction );
}
//=============================================================================================
/**
 * アクション選択終了待ち
 *
 * @param   core
 *
 * @retval  BtlAction   選択アクション（選択されていなければ BTL_ACTION_NULL ）
 */
//=============================================================================================
BtlAction BTLV_UI_SelectAction_Wait( BTLV_CORE* core )
{
  if( mainproc_call( core ) ){
    return core->playerAction;
  }
  return BTL_ACTION_NULL;
}
//=============================================================================================
/**
 * ワザ選択開始
 *
 * @param   core
 * @param   bpp
 * @param   dest
 */
//=============================================================================================
void BTLV_UI_SelectWaza_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
  core->procPokeParam = bpp;
  core->procPokeID = BPP_GetID( bpp );
  BTL_Printf("ワザ選択開始：対象ポケID=%d\n", core->procPokeID);
  core->actionParam = dest;
  mainproc_setup( core, CmdProc_SelectWaza );
}
//=============================================================================================
/**
 * ワザ選択終了待ち
 *
 * @param   core
 *
 * @retval  BOOL    終了していたらTRUE
 */
//=============================================================================================
BOOL BTLV_UI_SelectWaza_Wait( BTLV_CORE* core )
{
  return mainproc_call( core );
}
//=============================================================================================
/**
 * ワザ対象選択開始
 *
 * @param   core
 * @param   bpp
 * @param   dest
 */
//=============================================================================================
void BTLV_UI_SelectTarget_Start( BTLV_CORE* core, const BTL_POKEPARAM* bpp, BTL_ACTION_PARAM* dest )
{
  core->procPokeParam = bpp;
  core->procPokeID = BPP_GetID( bpp );
  core->actionParam = dest;
  mainproc_setup( core, CmdProc_SelectTarget );
}
//=============================================================================================
/**
 * ワザ選択終了待ち
 *
 * @param   core
 *
 * @retval  BOOL    終了していたらTRUE
 */
//=============================================================================================
BtlvResult BTLV_UI_SelectTarget_Wait( BTLV_CORE* core )
{
  if( mainproc_call(core) ){
    if( BTLV_SCD_IsSelTargetDone(core->scrnD) ){
      return BTLV_RESULT_DONE;
    }
    return BTLV_RESULT_CANCEL;
  }
  return BTLV_RESULT_NONE;
}


void BTLV_UI_Restart( BTLV_CORE* core )
{
  BTLV_SCD_RestartUI( core->scrnD );
}


void BTLV_StartPokeSelect( BTLV_CORE* wk, const BTL_POKESELECT_PARAM* param, int outMemberIndex, BOOL fCantEsc, BTL_POKESELECT_RESULT* result )
{
  wk->plistData.pp = BTL_MAIN_GetPlayerPokeParty( wk->mainModule );
  wk->plistData.multi_pp = BTL_MAIN_GetMultiPlayerPokeParty( wk->mainModule );
  wk->plistData.multiMode = ( wk->plistData.multi_pp != NULL );
  wk->plistData.multiPos = BTL_MAIN_GetPlayerMultiPos( wk->mainModule );
  wk->plistData.font = wk->largeFontHandle;
  wk->plistData.heap = wk->heapID;
  wk->plistData.mode = param->bplMode;
  wk->plistData.end_flg = FALSE;
  if( outMemberIndex >= 0 ){
    wk->plistData.sel_poke = outMemberIndex;
  }else{
    wk->plistData.sel_poke = 0;
  }
  wk->plistData.chg_waza = 0;
  wk->plistData.rule = BTL_MAIN_GetRule( wk->mainModule );
  wk->plistData.cursor_flg = BTLV_SCD_GetCursorFlagPtr( wk->scrnD );
  wk->plistData.tcb_sys = BTLV_EFFECT_GetTCBSYS();
  wk->plistData.pfd = BTLV_EFFECT_GetPfd();
  wk->plistData.chg_waza = fCantEsc;  // 逃げ・交換禁止フラグ

  // 既に選択されているポケモンの位置情報を初期化
  {
    u32 i, selectedPokeCnt = BTL_POKESELECT_RESULT_GetCount( result );

    BTL_N_Printf( DBGSTR_VCORE_PokeListStart, wk->plistData.mode, selectedPokeCnt);
    for(i=0; i<selectedPokeCnt; ++i)
    {
      if( i >= NELEMS(wk->plistData.change_sel) ){ break; }
      wk->plistData.change_sel[i] = BTL_POKESELECT_RESULT_Get( result, i );
    }
    for( ; i<NELEMS(wk->plistData.change_sel); ++i){
      wk->plistData.change_sel[i] = BPL_CHANGE_SEL_NONE;
    }
  }

  result->fCancel = TRUE;
  wk->pokeselResult = result;
  wk->selectItemSeq = 0;
}

BOOL BTLV_WaitPokeSelect( BTLV_CORE* wk )
{
  switch( wk->selectItemSeq ){
  case 0:
    BTLV_SCD_FadeOut( wk->scrnD );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_b_app ) );
    wk->selectItemSeq++;
    break;
  case 1:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      BTLV_SCD_Cleanup( wk->scrnD );
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_plist ) );
      BattlePokeList_TaskAdd( &wk->plistData );
      wk->selectItemSeq++;
    }
    break;
  case 2:
    if( wk->plistData.end_flg )
    {
      u32 i;

      BTL_N_Printf( DBGSTR_VCORE_SelPokeEnd );

      for(i=0; i<NELEMS(wk->plistData.sel_pos); ++i)
      {
        if( wk->plistData.sel_pos[i] != BPL_SELPOS_NONE ){
          BTL_POKESELECT_RESULT_Push( wk->pokeselResult, wk->plistData.sel_pos[i] );
          {
            u8 storeCnt = BTL_POKESELECT_RESULT_GetCount( wk->pokeselResult );
            BTL_N_Printf( DBGSTR_VCORE_SelPokeEnd_Sel, i, storeCnt);
            wk->pokeselResult->fCancel = FALSE;
          }
        }
        else{
          BTL_N_Printf( DBGSTR_VCORE_SelPokeEnd_Unsel, i );
        }
      }

      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );
      BTLV_SCD_FadeIn( wk->scrnD );
      BTLV_SCD_Setup( wk->scrnD );
      wk->selectItemSeq++;
    }
    break;
  case 3:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      wk->selectItemSeq++;
    }
    break;
  default:
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
    wk->selectItemSeq = 0;
    return TRUE;
  }
  return FALSE;
}
//=============================================================================================
/**
 * アイテム選択開始
 *
 * @param   wk
 * @param   bagMode   バッグモード（通常／シューター）
 * @param   energy
 */
//=============================================================================================
void BTLV_ITEMSELECT_Start( BTLV_CORE* wk, u8 bagMode, u8 energy, u8 reserved_energy )
{
  if( wk->selectItemSeq == 0 )
  {
    wk->bagData.myitem = BTL_MAIN_GetItemDataPtr( wk->mainModule );
    wk->bagData.bagcursor = BTL_MAIN_GetBagCursorData( wk->mainModule );
    wk->bagData.mode = bagMode;
    wk->bagData.font = wk->largeFontHandle;
    wk->bagData.heap = wk->heapID;
    wk->bagData.energy = energy;
    wk->bagData.reserved_energy = reserved_energy;
    wk->bagData.end_flg = FALSE;
    wk->bagData.ret_item = ITEM_DUMMY_DATA;
    wk->bagData.cursor_flg = BTLV_SCD_GetCursorFlagPtr( wk->scrnD );

    wk->plistData.pp = BTL_MAIN_GetPlayerPokeParty( wk->mainModule );
    wk->plistData.multi_pp = BTL_MAIN_GetMultiPlayerPokeParty( wk->mainModule );
    wk->plistData.multiMode = ( wk->plistData.multi_pp != NULL );
    wk->plistData.multiPos = BTL_MAIN_GetPlayerMultiPos( wk->mainModule );
    wk->plistData.font = wk->largeFontHandle;
    wk->plistData.heap = wk->heapID;
    wk->plistData.mode = BPL_MODE_ITEMUSE;
    wk->plistData.end_flg = FALSE;
    wk->plistData.sel_poke = 0;
    wk->plistData.chg_waza = 0;
    wk->plistData.cursor_flg = BTLV_SCD_GetCursorFlagPtr( wk->scrnD );
    wk->plistData.tcb_sys = BTLV_EFFECT_GetTCBSYS();
    wk->plistData.pfd = BTLV_EFFECT_GetPfd();

    wk->selectItemSeq = 1;
  }
}
//=============================================================================================
/**
 * アイテム選択終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL    終了したらTRUE
 */
//=============================================================================================
BOOL BTLV_ITEMSELECT_Wait( BTLV_CORE* wk )
{
  switch( wk->selectItemSeq ){
  case 1:
    BTLV_SCD_FadeOut( wk->scrnD );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_b_app ) );
    wk->selectItemSeq++;
    break;

  case 2:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      BTLV_SCD_Cleanup( wk->scrnD );
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_bag ) );
      BattleBag_TaskAdd( &wk->bagData );
      wk->selectItemSeq++;
    }
    break;

  case 3:
    if( wk->bagData.end_flg ){
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_bag ) );
      wk->selectItemSeq++;
    }
    break;

 case 4:
    if( (wk->bagData.ret_item != ITEM_DUMMY_DATA)
    &&  (wk->bagData.ret_page != BBAG_POKE_BALL)
    ){
      wk->plistData.item = wk->bagData.ret_item;
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_plist ) );
      BattlePokeList_TaskAdd( &wk->plistData );
      BTL_Printf("アイテム選択:%d\n", wk->plistData.item);
      wk->selectItemSeq++;
    }else{
      BTLV_SCD_FadeIn( wk->scrnD );
      BTLV_SCD_Setup( wk->scrnD );
      wk->selectItemSeq = 6;
    }
    break;

  case 5:
    if( wk->plistData.end_flg ){
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );
      BTLV_SCD_FadeIn( wk->scrnD );
      BTLV_SCD_Setup( wk->scrnD );
      wk->selectItemSeq++;
    }
    break;

  case 6:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
      wk->selectItemSeq = 0;
      return TRUE;
    }
  }
  return FALSE;
}
//=============================================================================================
/**
 * アイテム選択結果取得（使用アイテムID）
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
u16 BTLV_ITEMSELECT_GetItemID( BTLV_CORE* wk )
{
  if( wk->bagData.end_flg ){
    return wk->bagData.ret_item;
  }
  return ITEM_DUMMY_DATA;
}
//=============================================================================================
/**
 * アイテム選択結果取得（使用アイテムのコスト：シューター時）
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTLV_ITEMSELECT_GetCost( BTLV_CORE* wk )
{
  if( wk->bagData.end_flg ){
    return wk->bagData.ret_cost;
  }
  return 0;
}
//=============================================================================================
/**
 * アイテム選択結果取得（対象インデックス）
 *
 * @param   wk
 *
 * @retval  u8
 */
//=============================================================================================
u8 BTLV_ITEMSELECT_GetTargetIdx( BTLV_CORE* wk )
{
  BTL_Printf(" Item Target Index = %d\n",wk->plistData.sel_poke );
  return wk->plistData.sel_poke;
}
//=============================================================================================
/**
 * アイテム使用時、カーソルデータに反映する
 *
 * @param   wk
 */
//=============================================================================================
void BTLV_ITEMSELECT_ReflectUsedItem( BTLV_CORE* wk )
{
  if( (wk->bagData.ret_item != ITEM_DUMMY_DATA)
  &&  (wk->plistData.sel_poke != BPL_SEL_EXIT)
  &&  (wk->bagData.mode != BBAG_MODE_SHOOTER)
  ){
    MYITEM_BattleBagCursorSet( wk->bagData.bagcursor, wk->bagData.item_pos, wk->bagData.item_scr );
    MYITEM_BattleBagLastItemSet( wk->bagData.bagcursor, wk->bagData.ret_item, wk->bagData.ret_page );
  }
}

//--------------------------------------

//=============================================================================================
/**
 * ワザエフェクト開始
 *
 * @param   core
 * @param   dst
 *
 */
//=============================================================================================
void BTLV_ACT_WazaEffect_Start( BTLV_CORE* wk, BtlPokePos atPokePos, BtlPokePos defPokePos, WazaID waza,
  BtlvWazaEffect_TurnType turnType, u8 continueCount )
{
  BtlvMcssPos  at_pos, def_pos;

  at_pos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, atPokePos );
  def_pos = (defPokePos != BTL_POS_NULL)?
      BTL_MAIN_BtlPosToViewPos( wk->mainModule, defPokePos ) : BTLV_MCSS_POS_ERROR;


  BTLV_SCU_StartWazaEffect( wk->scrnU, at_pos, def_pos, waza, turnType, continueCount );
}
//=============================================================================================
/**
 * ワザエフェクト終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_ACT_WazaEffect_Wait( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitWazaEffect( wk->scrnU );
}

//--------------------------------------

typedef struct {
  WazaID      wazaID;
  BtlTypeAff  affinity;
  u16     damage;
  u16     timer;
  u8      defPokePos;
}WAZA_DMG_ACT_WORK;

//=============================================================================================
/**
 * 単体ワザダメージエフェクト開始
 *
 * @param   wk
 * @param   wazaID
 * @param   defPokePos
 * @param   damage
 * @param   aff
 */
//=============================================================================================
void BTLV_ACT_DamageEffectSingle_Start( BTLV_CORE* wk, WazaID wazaID, BtlPokePos defPokePos, u16 damage, BtlTypeAff aff )
{
  WAZA_DMG_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG_ACT_WORK));

  subwk->affinity = aff;
  subwk->damage = damage;
  subwk->defPokePos = defPokePos;
  subwk->timer = 0;
  subwk->wazaID = wazaID;

  BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocDamageEffect );
}
//=============================================================================================
/**
 * 単体ワザダメージエフェクト終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_ACT_DamageEffectSingle_Wait( BTLV_CORE* wk )
{
  return BTL_UTIL_CallProc( &wk->subProc );
}

static BOOL subprocDamageEffect( int* seq, void* wk_adrs )
{
  BTLV_CORE* wk = wk_adrs;
  WAZA_DMG_ACT_WORK* subwk = getGenericWork(wk, sizeof(WAZA_DMG_ACT_WORK));

  switch( *seq ){
  case 0:
    BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->defPokePos, subwk->wazaID );
    if( subwk->affinity < BTL_TYPEAFF_100 )
    {
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffBad, 0 );
      BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );

      PMSND_PlaySE( SEQ_SE_KOUKA_L );
    }
    else if ( subwk->affinity > BTL_TYPEAFF_100 )
    {
      BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffGood, 0 );
      BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
      PMSND_PlaySE( SEQ_SE_KOUKA_H );
    }
    else{
      PMSND_PlaySE( SEQ_SE_KOUKA_M );
    }
    (*seq)++;
    break;

  case 1:
    if( BTLV_SCU_WaitWazaDamageAct(wk->scrnU)
    &&  BTLV_SCU_WaitMsg(wk->scrnU)
    ){
      return TRUE;
    }
    break;

  }
  return FALSE;
}
//--------------------------------------
typedef struct {
  u8  pokePos[ BTL_POS_MAX ];
  u8  pokeCnt;
  u16 seq;
  BtlTypeAffAbout  affAbout;
  WazaID           wazaID;
}DMG_PLURAL_ACT_WORK;


void BTLV_ACT_DamageEffectPlural_Start( BTLV_CORE* wk, u32 pokeCnt, BtlTypeAffAbout affAbout, const u8* pokeID, WazaID wazaID )
{
  DMG_PLURAL_ACT_WORK* subwk = getGenericWork(wk, sizeof(DMG_PLURAL_ACT_WORK));

  subwk->affAbout = affAbout;
  subwk->pokeCnt = pokeCnt;
  subwk->wazaID = wazaID;
  subwk->seq = 0;

  BTL_Printf("複数体ダメージ処理 (%d体) \n", pokeCnt);

  {
    u32 i;
    for(i=0; i<pokeCnt; ++i)
    {
      BTL_Printf("  対象ポケID=%d\n", pokeID[i]);
      subwk->pokePos[i] = BTL_MAIN_PokeIDtoPokePos( wk->mainModule, wk->pokeCon, pokeID[i] );
    }
  }
}

BOOL BTLV_ACT_DamageEffectPlural_Wait( BTLV_CORE* wk )
{
  DMG_PLURAL_ACT_WORK* subwk = getGenericWork(wk, sizeof(DMG_PLURAL_ACT_WORK));

  switch( subwk->seq ){
  case 0:
    {
      u32 i;
      for(i=0; i<subwk->pokeCnt; ++i){
        BTLV_SCU_StartWazaDamageAct( wk->scrnU, subwk->pokePos[i], subwk->wazaID );
      }
      switch( subwk->affAbout ){
      case BTL_TYPEAFF_ABOUT_ADVANTAGE:
        BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffGood, 0 );
        BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
        PMSND_PlaySE( SEQ_SE_KOUKA_H );
        break;
      case BTL_TYPEAFF_ABOUT_DISADVANTAGE:
        BTL_STR_MakeStringStd( wk->strBuf, BTL_STRID_STD_AffBad, 0 );
        BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
        PMSND_PlaySE( SEQ_SE_KOUKA_L );
        break;
      default:
        PMSND_PlaySE( SEQ_SE_KOUKA_M );
      }
      subwk->seq++;
    }
    break;
  case 1:
    if( BTLV_SCU_WaitWazaDamageAct(wk->scrnU)
    &&  BTLV_SCU_WaitMsg(wk->scrnU)
    ){
      return TRUE;
    }
    break;
  }
  return FALSE;

}

//=============================================================================================
/**
 * シンプルHPエフェクト開始
 * ※HPメーターのみ変動。状態異常や天候ダメージに使う。変動後の値は、直接ポケモンデータを参照している。
 *
 * @param   wk
 * @param   pokePos
 *
 */
//=============================================================================================
void BTLV_ACT_SimpleHPEffect_Start( BTLV_CORE* wk, BtlPokePos pokePos )
{
  BTLV_SCU_StartHPGauge( wk->scrnU, pokePos );
}
BOOL BTLV_ACT_SimpleHPEffect_Wait( BTLV_CORE* wk )
{
  if( BTLV_SCU_WaitHPGauge(wk->scrnU) )
  {
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * 位置指定汎用エフェクト開始
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTLV_AddEffectByPos( BTLV_CORE* wk, BtlvMcssPos vpos, u16 effectNo )
{
  BTLV_EFFECT_AddByPos( vpos, effectNo );
}
BOOL BTLV_WaitEffectByPos( BTLV_CORE* wk, BtlvMcssPos vpos )
{
  if( !BTLV_EFFECT_CheckExecute() ){
    return TRUE;
  }
  return FALSE;
}

//=============================================================================================
/**
 * 開始＆終端位置指定汎用エフェクト開始
 *
 * @param   wk
 * @param   vpos_from
 * @param   vpos_to
 */
//=============================================================================================
void BTLV_AddEffectByDir( BTLV_CORE* wk, BtlvMcssPos vpos_from, BtlvMcssPos vpos_to, u16 effectNo )
{
  BTLV_EFFECT_AddByDir( vpos_from, vpos_to, effectNo );
}
BOOL BTLV_WaitEffectByDir( BTLV_CORE* wk )
{
  if( !BTLV_EFFECT_CheckExecute() ){
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * ポケモンひんしアクション開始
 *
 * @param   wk
 * @param   pos   ひんしになったポケモンの位置ID
 *
 */
//=============================================================================================
void BTLV_StartDeadAct( BTLV_CORE* wk, BtlPokePos pos )
{
  BTLV_SCU_StartDeadAct( wk->scrnU, pos );
}
BOOL BTLV_WaitDeadAct( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitDeadAct( wk->scrnU );
}
//=============================================================================================
/**
 * ポケモン生き返りアクション開始
 *
 * @param   wk
 * @param   pos   ひんしになったポケモンの位置ID
 *
 */
//=============================================================================================
void BTLV_StartReliveAct( BTLV_CORE* wk, BtlPokePos pos )
{
  BTLV_SCU_StartReliveAct( wk->scrnU, pos );
}
BOOL BTLV_WaitReliveAct( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitReliveAct( wk->scrnU );
}




//=============================================================================================
/**
 * ポケモン退出アクション開始
 *
 * @param   wk
 * @param   clientID
 * @param   memberIdx
 *
 */
//=============================================================================================
void BTLV_ACT_MemberOut_Start( BTLV_CORE* wk, BtlvMcssPos vpos )
{
  BTLV_SCU_StartMemberOutAct( wk->scrnU, vpos );
}
BOOL BTLV_ACT_MemberOut_Wait( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitMemberOutAct( wk->scrnU );
}


//------------------------------------------
typedef struct {
  BtlPokePos  pokePos;
  u8 clientID;
  u8 memberIdx;
  u8 pokeID;
}MEMBER_IN_WORK;

//=============================================================================================
/**
 * ポケモン入れ替えアクション開始
 *
 * @param   wk
 * @param   clientID
 * @param   memberIdx
 *
 */
//=============================================================================================
void BTLV_StartMemberChangeAct( BTLV_CORE* wk, BtlPokePos pos, u8 clientID, u8 memberIdx )
{
  MEMBER_IN_WORK* subwk = getGenericWork( wk, sizeof(MEMBER_IN_WORK) );
  const BTL_POKEPARAM* bpp = BTL_POKECON_GetFrontPokeDataConst( wk->pokeCon, pos );

  subwk->clientID = clientID;
  subwk->memberIdx = memberIdx;
  subwk->pokePos = pos;
  subwk->pokeID = BPP_GetID( bpp );
  BTL_Printf("メンバー入れ替え : pos=%d, Idx=%d, pokeID=%d\n", pos, memberIdx, subwk->pokeID );

  BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocMemberIn );
}

BOOL BTLV_WaitMemberChangeAct( BTLV_CORE* wk )
{
  return BTL_UTIL_CallProc( &wk->subProc );
}

static BOOL subprocMemberIn( int* seq, void* wk_adrs )
{
  BTLV_CORE* wk = wk_adrs;
  MEMBER_IN_WORK* subwk = getGenericWork( wk, sizeof(MEMBER_IN_WORK) );

  switch( *seq ){
  case 0:
    BTLV_SCU_StartPokeIn( wk->scrnU, subwk->pokePos, subwk->clientID, subwk->memberIdx );
    (*seq)++;
    break;
  case 1:
    if( BTLV_SCU_WaitPokeIn(wk->scrnU) )
    {
      return TRUE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 *
 *
 * @param   param
 * @param   dst
 */
//----------------------------------------------------------------------------------
static void StrParamToString( const BTLV_STRPARAM* param, STRBUF* dst )
{
  switch( param->strType ){
  case BTL_STRTYPE_STD:
    BTL_STR_MakeStringStdWithArgArray( dst, param->strID, param->args );
    break;
  case BTL_STRTYPE_SET:
    BTL_STR_MakeStringSet( dst, param->strID, param->args );
    break;
  case BTL_STRTYPE_UI:
    BTL_STR_GetUIString( dst, param->strID );
    break;
  case BTL_STRTYPE_WAZAOBOE:
    BTL_STR_MakeStringWazaOboeWithArgArray( dst, param->strID, param->args );
    break;
  case BTL_STRTYPE_YESNO:
    BTL_STR_MakeStringYesNoWithArgArray( dst, param->strID, param->args );
    break;
  case BTL_STRTYPE_WAZA:
    BTL_STR_MakeStringWaza( dst, param->args[0], param->args[1] );
  default:
    GF_ASSERT(0);
    return;
  }
}


//=============================================================================================
/**
 * メッセージ表示開始
 *
 * @param   wk
 * @param   param
 */
//=============================================================================================
void BTLV_StartMsg( BTLV_CORE* wk, const BTLV_STRPARAM* param )
{
  StrParamToString( param, wk->strBuf );
  BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, param->wait );
}
//=============================================================================================
/**

 *
 * @param   wk
 * @param   strID
 *
 */
//=============================================================================================
void BTLV_StartMsgStd( BTLV_CORE* wk, u16 strID, const int* args )
{
  BTL_STR_MakeStringStdWithArgArray( wk->strBuf, strID, args );
  BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
}

//=============================================================================================
/**
 * メッセージ表示開始
 *
 * @param   wk
 * @param   strID
 *
 */
//=============================================================================================
void BTLV_StartMsgSet( BTLV_CORE* wk, u16 strID, const int* args )
{
  BTL_STR_MakeStringSet( wk->strBuf, strID, args );
  BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
}

void BTLV_StartMsgWaza( BTLV_CORE* wk, u8 pokeID, u16 waza )
{
  BTL_STR_MakeStringWaza( wk->strBuf, pokeID, waza );
  BTLV_SCU_StartMsg( wk->scrnU, wk->strBuf, BTLV_MSGWAIT_STD );
}

BOOL BTLV_WaitMsg( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitMsg( wk->scrnU );
}
BOOL BTLV_IsJustDoneMsg( BTLV_CORE* wk )
{
  return BTLV_SCU_IsJustDoneMsg( wk->scrnU );
}



//=============================================================================================
/**
 * とくせいウィンドウ表示開始
 *
 * @param   wk
 * @param   pos
 * @param   fFlash  効果発動を示すフラッシュ動作を入れるか？
 */
//=============================================================================================
void BTLV_TokWin_DispStart( BTLV_CORE* wk, BtlPokePos pos, BOOL fFlash )
{
  BTLV_SCU_TokWin_DispStart( wk->scrnU, pos, fFlash );
}
BOOL BTLV_TokWin_DispWait( BTLV_CORE* wk, BtlPokePos pos )
{
  return BTLV_SCU_TokWin_DispWait( wk->scrnU, pos );
}
void BTLV_QuitTokWin( BTLV_CORE* wk, BtlPokePos pos )
{
  BTLV_SCU_TokWin_HideStart( wk->scrnU, pos );
}
BOOL BTLV_QuitTokWinWait( BTLV_CORE* wk, BtlPokePos pos )
{
  return BTLV_SCU_TokWin_HideWait( wk->scrnU, pos );
}

//=============================================================================================
/**
 * とくせいウィンドウの内容更新（開始）
 *
 * @param   wk
 * @param   pos
 *
 */
//=============================================================================================
void BTLV_TokWin_Renew_Start( BTLV_CORE* wk, BtlPokePos pos )
{
  BTLV_SCU_TokWin_Renew_Start( wk->scrnU, pos );
}
//=============================================================================================
/**
 * とくせいウィンドウの内容更新（終了待ち）
 *
 * @param   wk
 * @param   pos
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_TokWin_Renew_Wait( BTLV_CORE* wk, BtlPokePos pos )
{
  return BTLV_SCU_TokWin_Renew_Wait( wk->scrnU, pos );
}



void BTLV_StartRankDownEffect( BTLV_CORE* wk, u8 vpos )
{
  BTL_Printf("ランクダウンエフェクト開始 : %d\n", vpos);
  BTLV_EFFECT_AddByPos( vpos, BTLEFF_STATUS_DOWN );
}
void BTLV_StartRankUpEffect( BTLV_CORE* wk, u8 vpos )
{
  BTLV_EFFECT_AddByPos( vpos, BTLEFF_STATUS_UP );
}
BOOL BTLV_WaitRankEffect( BTLV_CORE* wk, u8 vpos )
{
  if( !BTLV_EFFECT_CheckExecute() ){
    BTL_Printf("エフェクト終了した ... 一応vpos=%d\n", vpos);
    return TRUE;
  }
  return FALSE;
}


//=============================================================================================
/**
 * 「つうしんたいきちゅう…」メッセージを表示
 *
 * @param   wk
 *
 */
//=============================================================================================
void BTLV_StartCommWait( BTLV_CORE* wk )
{
  BTLV_SCU_StartCommWaitInfo( wk->scrnU );
}
BOOL BTLV_WaitCommWait( BTLV_CORE* wk )
{
  return BTLV_SCU_WaitCommWaitInfo( wk->scrnU );
}
void BTLV_ResetCommWaitInfo( BTLV_CORE* wk )
{
  BTLV_SCU_ClearCommWaitInfo( wk->scrnU );
}


//=============================================================================================
/**
 * きのみ食べアクション開始
 *
 * @param   wk
 * @param   pos   きのみを食べるポケモンの位置
 */
//=============================================================================================
void BTLV_KinomiAct_Start( BTLV_CORE* wk, BtlPokePos pos )
{
  BtlvMcssPos vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
  BTLV_SCU_KinomiAct_Start( wk->scrnU, vpos );
}
//=============================================================================================
/**
 * きのみ食べアクション終了待ち
 *
 * @param   wk
 * @param   pos   きのみを食べるポケモンの位置
 */
//=============================================================================================
BOOL BTLV_KinomiAct_Wait( BTLV_CORE* wk, BtlPokePos pos )
{
  BtlvMcssPos vpos = BTL_MAIN_BtlPosToViewPos( wk->mainModule, pos );
  return BTLV_SCU_KinomiAct_Wait( wk->scrnU, vpos );
}
//=============================================================================================
/**
 * イリュージョン解除アクション 開始
 *
 * @param   wk
 * @param   pos
 */
//=============================================================================================
void BTLV_FakeDisable_Start( BTLV_CORE* wk, BtlPokePos pos )
{
  BTLV_SCU_FakeDisable_Start( wk->scrnU, pos );
}
//=============================================================================================
/**
 * イリュージョン解除アクション 終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_FakeDisable_Wait( BTLV_CORE* wk )
{
  return BTLV_SCU_FakeDisable_Wait( wk->scrnU );
}

/*--------------------------------------------------------------------------------------------------*/
/* トリプルバトル ムーブアクション関連                                                              */
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
  u8 clientID;
  u8 vpos1;
  u8 vpos2;
  u8 posIdx1;
  u8 posIdx2;
  BtlPokePos  pos1;
  BtlPokePos  pos2;
}MOVE_MEMBER_WORK;

//=============================================================================================
/**
 * ムーブアクション開始
 *
 * @param   wk
 * @param   vpos1
 * @param   vpos2
 */
//=============================================================================================
void BTLV_ACT_MoveMember_Start( BTLV_CORE* wk, u8 clientID, u8 vpos1, u8 vpos2, u8 posIdx1, u8 posIdx2 )
{
  MOVE_MEMBER_WORK* subwk = getGenericWork( wk, sizeof(MOVE_MEMBER_WORK) );

  subwk->clientID = clientID;
  subwk->vpos1 = vpos1;
  subwk->vpos2 = vpos2;
  subwk->posIdx1 = posIdx1;
  subwk->posIdx2 = posIdx2;
  subwk->pos1 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx1 );
  subwk->pos2 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, posIdx2 );

  BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocMoveMember );
}
//=============================================================================================
/**
 * ムーブアクション終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_ACT_MoveMember_Wait( BTLV_CORE* wk )
{
  return BTL_UTIL_CallProc( &wk->subProc );
}
static BOOL subprocMoveMember( int* seq, void* wk_adrs )
{
  BTLV_CORE* wk = wk_adrs;
  MOVE_MEMBER_WORK* subwk = getGenericWork( wk, sizeof(MOVE_MEMBER_WORK) );

  // @todo 今は単に該当個所の絵を消して、新しく出してるだけで、
  // 演出的な動作がまったく入っていない。いずれちゃんと作る。
  switch( *seq ){
  case 0:
    if( BTLV_EFFECT_CheckExist(subwk->vpos1) ){
      BTLV_EFFECT_DelPokemon( subwk->vpos1 );
    }
    if( BTLV_EFFECT_CheckExist(subwk->vpos2) ){
      BTLV_EFFECT_DelPokemon( subwk->vpos2 );
    }
    (*seq)++;
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      const BTL_PARTY* party = BTL_POKECON_GetPartyDataConst( wk->pokeCon, subwk->clientID );
      const BTL_POKEPARAM* bpp;
      bpp = BTL_PARTY_GetMemberDataConst( party, subwk->posIdx1 );
      if( !BPP_IsDead(bpp) ){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(bpp), subwk->vpos1 );
      }
      bpp = BTL_PARTY_GetMemberDataConst( party, subwk->posIdx2 );
      if( !BPP_IsDead(bpp) ){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(bpp), subwk->vpos2 );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() ){
      BTLV_SCU_MoveGauge_Start( wk->scrnU, subwk->pos1, subwk->pos2 );
      (*seq)++;
    }
    break;

  case 3:
    if( BTLV_SCU_MoveGauge_Wait(wk->scrnU) ){
      return TRUE;
    }
    break;
  }
  return FALSE;
}


/*--------------------------------------------------------------------------------------------------*/
/* ローテーションバトル関連                                                                         */
/*--------------------------------------------------------------------------------------------------*/
typedef struct {
  u8 clientID;
  u8 dir;
  u8 vpos1;
  u8 vpos2;
  BtlPokePos  pos1;
  BtlPokePos  pos2;
}ROTATE_MEMBER_WORK;


//#define TMP_SELROT_SKIP

//=============================================================================================
/**
 * ユーザ入力：ローテーション選択開始
 *
 * @param   wk
 * @param   prevDir   前ターンに選択されたローテーション方向
 */
//=============================================================================================
void BTLV_UI_SelectRotation_Start( BTLV_CORE* wk, BtlRotateDir prevDir )
{
  BTLV_INPUT_ROTATE_PARAM* rotateParam = getGenericWork( wk, sizeof(BTLV_INPUT_ROTATE_PARAM) );
  const BTL_PARTY* party = BTL_POKECON_GetPartyDataConst( wk->pokeCon, wk->myClientID );
  const BTL_POKEPARAM* bpp;
  u32 i;

  for(i=0; i<BTL_ROTATE_NUM; ++i){
    bpp = BTL_PARTY_GetMemberDataConst( party, i );
    rotateParam->pp[i] = BPP_GetSrcData( bpp );
  }
  rotateParam->before_select_dir = prevDir;

#ifndef TMP_SELROT_SKIP
  BTLV_SCD_SelectRotate_Start( wk->scrnD, rotateParam );
#endif
}
//=============================================================================================
/**
 * ユーザ入力：ローテーション選択終了待ち
 *
 * @param   wk
 * @param   result    [out] 今回選択されたローテーション方向
 *
 * @retval  BOOL      終了時TRUE
 */
//=============================================================================================
BOOL BTLV_UI_SelectRotation_Wait( BTLV_CORE* wk, BtlRotateDir* result )
{
  #ifndef TMP_SELROT_SKIP
  if( BTLV_SCD_SelectRotate_Wait(wk->scrnD, result) ){
    return TRUE;
  }
  return FALSE;
  #else
  BTLV_INPUT_ROTATE_PARAM* rotateParam = getGenericWork( wk, sizeof(BTLV_INPUT_ROTATE_PARAM) );
  u8 rnd = BTL_CALC_GetRand( 100 );
  switch( rotateParam->before_select_dir ){
  case BTL_ROTATEDIR_STAY:
    *result = (rnd < 50)? BTL_ROTATEDIR_L : BTL_ROTATEDIR_R;
    break;
  case BTL_ROTATEDIR_L:
    *result = (rnd < 50)? BTL_ROTATEDIR_STAY : BTL_ROTATEDIR_R;
    break;
  case BTL_ROTATEDIR_R:
    *result = (rnd < 50)? BTL_ROTATEDIR_STAY : BTL_ROTATEDIR_L;
    break;
  default:
    *result = BTL_ROTATEDIR_STAY;
    break;
  }
  return TRUE;
  #endif
}

//=============================================================================================
/**
 * ローテーションアクション開始
 *
 * @param   wk
 * @param   clientID    対象クライアントID
 * @param   dir         ローテーション方向
 */
//=============================================================================================
void BTLV_RotationMember_Start( BTLV_CORE* wk, u8 clientID, BtlRotateDir dir )
{
  if( dir != BTL_ROTATEDIR_STAY )
  {
    ROTATE_MEMBER_WORK* subwk = getGenericWork( wk, sizeof(ROTATE_MEMBER_WORK) );

    subwk->clientID = clientID;
    subwk->dir = dir;

    subwk->pos1 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, 0 );
    subwk->pos2 = BTL_MAIN_GetClientPokePos( wk->mainModule, clientID, 1 );
    subwk->vpos1 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos1 );
    subwk->vpos2 = BTL_MAIN_BtlPosToViewPos( wk->mainModule, subwk->pos2 );

    BTL_UTIL_SetupProc( &wk->subProc, wk, NULL, subprocRotateMember );
  }

}
//=============================================================================================
/**
 * ローテーションアクション終了待ち
 *
 * @param   wk
 *
 * @retval  BOOL
 */
//=============================================================================================
BOOL BTLV_RotationMember_Wait( BTLV_CORE* wk )
{
  return BTL_UTIL_CallProc( &wk->subProc );
}
static BOOL subprocRotateMember( int* seq, void* wk_adrs )
{
  // @todo 今は単に該当個所の絵を消して、新しく出してるだけで、
  // 演出的な動作がまったく入っていない。いずれちゃんと作る。
  BTLV_CORE* wk = wk_adrs;
  ROTATE_MEMBER_WORK* subwk = getGenericWork( wk, sizeof(ROTATE_MEMBER_WORK) );

  switch( *seq ){
  case 0:
    if( BTLV_EFFECT_CheckExist(subwk->vpos1) ){
      BTLV_EFFECT_DelPokemon( subwk->vpos1 );
    }
    if( BTLV_EFFECT_CheckExist(subwk->vpos2) ){
      BTLV_EFFECT_DelPokemon( subwk->vpos2 );
    }
    (*seq)++;
    break;

  case 1:
    if( !BTLV_EFFECT_CheckExecute() )
    {
      const BTL_PARTY* party = BTL_POKECON_GetPartyDataConst( wk->pokeCon, subwk->clientID );
      const BTL_POKEPARAM* bpp;
      bpp = BTL_PARTY_GetMemberDataConst( party, 0 );
      if( !BPP_IsDead(bpp) ){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(bpp), subwk->vpos1 );
      }
      bpp = BTL_PARTY_GetMemberDataConst( party, 1 );
      if( !BPP_IsDead(bpp) ){
        BTLV_EFFECT_SetPokemon( BPP_GetSrcData(bpp), subwk->vpos2 );
      }
      (*seq)++;
    }
    break;

  case 2:
    if( !BTLV_EFFECT_CheckExecute() ){
      BTLV_SCU_UpdateGauge_Start( wk->scrnU, subwk->pos1 );
      BTLV_SCU_UpdateGauge_Start( wk->scrnU, subwk->pos2 );
      (*seq)++;
    }
    break;

  case 3:
    {
      BOOL fEnd = TRUE;
      if( BTLV_SCU_UpdateGauge_Wait(wk->scrnU, subwk->pos1) == FALSE ){
        fEnd = FALSE;
      }
      if( BTLV_SCU_UpdateGauge_Wait(wk->scrnU, subwk->pos2) == FALSE ){
        fEnd = FALSE;
      }
      if( fEnd ){
        return TRUE;
      }
    }
    break;
  }
  return FALSE;
}


//=============================================================================================
/**
 * はい／いいえ選択
 *
 * @param   wk
 * @param   yes_msg はいのボタンに出すメッセージ
 * @param   no_msg  いいえのボタンに出すメッセージ
 */
//=============================================================================================
void BTLV_YESNO_Start( BTLV_CORE* wk, BTLV_STRPARAM* yes_msg, BTLV_STRPARAM* no_msg )
{
  BTLV_INPUT_YESNO_PARAM* yesnoParam = getGenericWork( wk, sizeof( BTLV_INPUT_YESNO_PARAM ) );

  yesnoParam->yes_msg = GFL_STR_CreateBuffer( BTL_YESNO_MSG_LENGTH, GFL_HEAP_LOWID(wk->heapID) );
  yesnoParam->no_msg = GFL_STR_CreateBuffer( BTL_YESNO_MSG_LENGTH, GFL_HEAP_LOWID(wk->heapID) );

  StrParamToString( yes_msg, yesnoParam->yes_msg );
  StrParamToString( no_msg,  yesnoParam->no_msg );
//  BTL_STR_MakeStringYesNoWithArgArray( yesnoParam->yes_msg, yes_msg->strID, yes_msg->args );
//  BTL_STR_MakeStringYesNoWithArgArray( yesnoParam->no_msg, no_msg->strID, no_msg->args );

  BTLV_SCD_SelectYesNo_Start( wk->scrnD, yesnoParam );

  GFL_HEAP_FreeMemory( yesnoParam->yes_msg );
  GFL_HEAP_FreeMemory( yesnoParam->no_msg );
}

BOOL BTLV_YESNO_Wait( BTLV_CORE* wk, BtlYesNo* result )
{
  return BTLV_SCD_SelectYesNo_Wait( wk->scrnD, result );
}

//=============================================================================================
/**
 * 技忘れ
 *
 * @param   wk
 * @param   pos   技を覚えるポケモンの位置
 * @param   waza  新しく覚える技ナンバー
 */
//=============================================================================================
void BTLV_WAZAWASURE_Start( BTLV_CORE* wk, u8 pos, WazaID waza )
{
  wk->plistData.pp = BTL_MAIN_GetPlayerPokeParty( wk->mainModule );
  wk->plistData.multi_pp = BTL_MAIN_GetMultiPlayerPokeParty( wk->mainModule );
  wk->plistData.multiMode = ( wk->plistData.multi_pp != NULL );
  wk->plistData.multiPos = BTL_MAIN_GetPlayerMultiPos( wk->mainModule );
  wk->plistData.font = wk->largeFontHandle;
  wk->plistData.heap = wk->heapID;
  wk->plistData.mode = BPL_MODE_WAZASET;
  wk->plistData.end_flg = FALSE;
  wk->plistData.sel_poke = pos;
  wk->plistData.chg_waza = waza;
  wk->plistData.rule = BTL_MAIN_GetRule( wk->mainModule );
  wk->plistData.cursor_flg = BTLV_SCD_GetCursorFlagPtr( wk->scrnD );
  wk->plistData.tcb_sys = BTLV_EFFECT_GetTCBSYS();
  wk->plistData.pfd = BTLV_EFFECT_GetPfd();

  wk->selectItemSeq = 0;
}

BOOL BTLV_WAZAWASURE_Wait( BTLV_CORE* wk, u8* result )
{
  switch( wk->selectItemSeq ){
  case 0:
    BTLV_SCD_FadeOut( wk->scrnD );
    GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_b_app ) );
    wk->selectItemSeq++;
    break;
  case 1:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      BTLV_SCD_Cleanup( wk->scrnD );
      GFL_OVERLAY_Load( FS_OVERLAY_ID( battle_plist ) );
      BattlePokeList_TaskAdd( &wk->plistData );
      wk->selectItemSeq++;
    }
    break;
  case 2:
    if( wk->plistData.end_flg )
    {
      GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_plist ) );
      BTLV_SCD_FadeIn( wk->scrnD );
      BTLV_SCD_Setup( wk->scrnD );
      wk->selectItemSeq++;
    }
    break;
  case 3:
    if( BTLV_SCD_FadeFwd(wk->scrnD) ){
      wk->selectItemSeq++;
    }
    break;
  default:
    *result = wk->plistData.sel_wp;
    GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_b_app ) );
    wk->selectItemSeq = 0;
    return TRUE;
  }
  return FALSE;
}

/*--------------------------------------------------------------------------------------------------*/
/* 下請けから呼び出される関数群                                                                     */
/*--------------------------------------------------------------------------------------------------*/

u8 BTLV_CORE_GetPlayerClientID( const BTLV_CORE* core )
{
  return core->myClientID;
}
