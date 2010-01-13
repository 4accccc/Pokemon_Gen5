//============================================================================================
/**
 * @file  d_btltest.c
 * @brief デバッグ用関数
 * @author  soga
 * @date  2008.09.09
 */
//============================================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include <nnsys/mcs.h>

#include "system/main.h"
#include "arc_def.h"

#include "system\gfl_use.h"
#include "battle/btlv/btlv_effect.h"
#include "pokegra/pokegra_wb.naix"
#include "battle/battgra_wb.naix"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "test/performance.h"

#include "waza_tool/wazano_def.h"

#include "print/printsys.h"
#include "font/font.naix"
#include "message.naix"

#include "battle/btlv/btlv_input.h"
#include "battle/btlv/btlv_b_gauge.h"

#include "tr_tool/tr_tool.h"

#include "battle/battle.h"

#define BTLV_MCSS_1vs1    //1vs1描画

#define PAD_BUTTON_EXIT ( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START )

#define  NIN_SPL_MAX  (3)

#define CAMERA_SPEED    ( FX32_ONE * 2 )
#define MOVE_SPEED      ( FX32_ONE >> 2 )
#define ROTATE_SPEED    ( 65536 / 128 )
#define PHI_MAX       ( ( 65536 / 360 ) * 89 )
#define DEFAULT_PHI     ( 65536 / 12 )
#define DEFAULT_THETA   ( 65536 / 4 )
#define DEFAULT_SCALE   ( FX32_ONE * 16  )
#define SCALE_SPEED     ( FX32_ONE )
#define SCALE_MAX     ( FX32_ONE * 200 )

#define SWITCH_TIME     ( 60 * 5 )

#define G2D_BACKGROUND_COL  (0x0000)
#define G2D_FONT_COL    (0x7fff)
#define G2D_FONTSELECT_COL  (0x001f)

enum{
  BMPWIN_MONSNO = 0,
  BMPWIN_PROJECTION,
  BMPWIN_SCALE_E,
  BMPWIN_SCALE_M,

  BMPWIN_MAX
};

typedef struct
{
  int                 seq_no;
  BTLV_CAMERA_WORK    *bcw;
//  BTLV_EFFECT_WORK  *bew;
  HEAPID              heapID;
  int                 visible_flag;
  int                 timer_flag;
  int                 timer;

  POKEMON_PARAM       *pp;
  int                 mons_no;
  GFL_BMPWIN          *bmpwin[ BMPWIN_MAX ];
  int                 position;
  BTLV_MCSS_WORK      *bmw;
  int                 key_repeat_speed;
  int                 key_repeat_wait;
  int                 ortho_mode;
  BTLV_INPUT_WORK*    biw;
  GFL_MSGDATA         *msg;
  GFL_FONT            *font;
  GFL_FONT            *small_font;
  fx32                scale;
  fx32                scale_value;
  BTLV_BALL_GAUGE_WORK* bbgw[ 2 ];
  u8                  cursor_flag;
}SOGA_WORK;

static  void  MoveCamera( SOGA_WORK *wk );

static  void  set_pokemon( SOGA_WORK *wk );
static  void  del_pokemon( SOGA_WORK *wk );

static  void  d_btl_v_blank( GFL_TCB *tcb, void *work );

static  const int pokemon_pos_table[][2]={
  { BTLV_MCSS_POS_AA, BTLV_MCSS_POS_BB },
  { BTLV_MCSS_POS_A, BTLV_MCSS_POS_B },
  { BTLV_MCSS_POS_C, BTLV_MCSS_POS_D }
};

//FS_EXTERN_OVERLAY(battle_view);
FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugBattleTestProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SOGA_WORK* wk;
  static const GFL_DISP_VRAM dispvramBank = {
    GX_VRAM_BG_128_D,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,      // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_64_E,       // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,     // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_01_AB,        // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_01_FG,      // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_64K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };

//  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle_view));
  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle));

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
  wk = GFL_PROC_AllocWork( proc, sizeof( SOGA_WORK ), HEAPID_SOGABE_DEBUG );
  MI_CpuClearFast( wk, sizeof( SOGA_WORK ) );
  wk->heapID = HEAPID_SOGABE_DEBUG;

  GFL_DISP_SetBank( &dispvramBank );

  //VRAMクリア
  MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
  MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

  GX_SetBankForLCDC( GX_VRAM_LCDC_D );

  G2_BlendNone();
  GFL_BG_Init( wk->heapID );
  GFL_BMPWIN_Init( wk->heapID );

  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
//      GX_DISPMODE_VRAM_C, GX_BGMODE_4, GX_BGMODE_3, GX_BG0_AS_3D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  {
    ///< main
    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

    ///< sub
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_ON );

    ///<obj
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
  }
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  //3D関連初期化
  {
    int i;
    u32 fog_table[8];

    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, wk->heapID, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3X_AlphaBlend( TRUE );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( TRUE );
    /*
    G3X_SetFog( TRUE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x0020, 0 );
    G3X_SetFogColor( GX_RGB( 31, 31, 31 ), 0 );

        for ( i=0; i<8; i++ ){
            fog_table[i] = (u32)(((i*16)<<0) | ((i*16+4)<<8) | ((i*16+8)<<16) | ((i*16+12)<<24));
        }
    G3X_SetFogTable( &fog_table[0] );
    */
    GFL_BG_SetBGControl3D( 1 );
  }

  // フォント作成を前に  taya
  wk->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );
  wk->small_font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_batt_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  wk->seq_no = 0;
  {
    BTL_FIELD_SITUATION bfs = { 
      0, 0, 0, 0, 0
    };

    GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &dispvramBank, wk->heapID );

    BTLV_EFFECT_Init( BTL_RULE_SINGLE, &bfs, wk->small_font, wk->heapID );
    wk->bcw = BTLV_EFFECT_GetCameraWork();
  }

  set_pokemon( wk );

  //2D画面初期化
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat[] = {
      ///<FRAME1_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
      ///<FRAME2_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
      ///<FRAME3_M
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
    };
    GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME2_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M );
  }

  //ウインドマスク設定（画面両端のエッジマーキングのゴミを消す）
  {
    G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_BG0 |
                 GX_WND_PLANEMASK_BG1 |
                 GX_WND_PLANEMASK_BG2 |
                 GX_WND_PLANEMASK_BG3 |
                 GX_WND_PLANEMASK_OBJ,
                 FALSE );
    G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, FALSE );
    G2_SetWnd0Position( 1, 1, 255, 191 );
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
  }

  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x0000 );

  wk->biw = BTLV_INPUT_Init( BTLV_INPUT_TYPE_SINGLE, NULL, wk->font, &wk->cursor_flag, wk->heapID );

  //フェードイン
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 2 );

  GFL_UI_KEY_GetRepeatSpeed( &wk->key_repeat_speed, &wk->key_repeat_wait );
  GFL_UI_KEY_SetRepeatSpeed( wk->key_repeat_speed / 4, wk->key_repeat_wait );

  wk->scale = FX32_ONE * 3;
  wk->scale_value = -0x100;

  wk->bbgw[ 0 ] = NULL;
  wk->bbgw[ 1 ] = NULL;

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugBattleTestProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int pad = GFL_UI_KEY_GetCont();
  int trg = GFL_UI_KEY_GetTrg();
  int rep = GFL_UI_KEY_GetRepeat();
  int tp = GFL_UI_TP_GetTrg();
  SOGA_WORK* wk = mywk;

  //画面切り替え実験
  if( wk->timer_flag ){
    wk->timer++;
  }
  if(wk->timer > SWITCH_TIME ){
    wk->timer=0;
    wk->visible_flag ^= 1;
    if( wk->visible_flag ){
      GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_ON );
    }
    else{
      GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    }
  }

  if( trg & PAD_BUTTON_START ){
    VecFx32 pos,target;
    BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
//    BTLV_CAMERA_MoveCameraPosition( wk->bcw, &pos, &target );
    BTLV_CAMERA_MoveCameraInterpolation( wk->bcw, &pos, &target, 20, 0, 20 );
  }

  if( trg & PAD_BUTTON_A )
  {
    STRBUF*  str = GFL_STR_CreateBuffer( 100, wk->heapID );
    TT_TrainerMessageGet( 1, 0, str, wk->heapID );
    GFL_STR_DeleteBuffer( str );
  }

  MoveCamera( wk );

#if 0
  if( trg & PAD_BUTTON_SELECT )
  {
    BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_STANDBY, NULL );
  }
  if( trg & PAD_BUTTON_A )
  {
    BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_COMMAND, NULL );
  }
  if( trg & PAD_BUTTON_B )
  {
    BTLV_INPUT_CreateScreen( wk->biw, BTLV_INPUT_SCRTYPE_WAZA, NULL );
  }
#endif

#if 0
  if( trg & PAD_BUTTON_A )
  {
    BTLV_EFFECT_CalcGauge( BTLV_MCSS_POS_AA, -8 );
  }
  if( trg & PAD_BUTTON_B )
  {
    BTLV_EFFECT_CalcGauge( BTLV_MCSS_POS_AA, 12 );
  }
  if( trg & PAD_BUTTON_X )
  {
    if( wk->bbgw[ 0 ] )
    {
      BTLV_BALL_GAUGE_Delete( wk->bbgw[ 0 ] );
      wk->bbgw[ 0 ] = NULL;
    }
    if( wk->bbgw[ 1 ] )
    {
      BTLV_BALL_GAUGE_Delete( wk->bbgw[ 1 ] );
      wk->bbgw[ 1 ] = NULL;
    }
  }
  if( trg & PAD_BUTTON_Y )
  {
    BTLV_BALL_GAUGE_PARAM param;
    int i;

    if( wk->bbgw[ 0 ] )
    {
      BTLV_BALL_GAUGE_Delete( wk->bbgw[ 0 ] );
      wk->bbgw[ 0 ] = NULL;
    }
    if( wk->bbgw[ 1 ] )
    {
      BTLV_BALL_GAUGE_Delete( wk->bbgw[ 1 ] );
      wk->bbgw[ 1 ] = NULL;
    }

    param.type = BTLV_BALL_GAUGE_TYPE_ENEMY;
    for( i = 0 ; i < TEMOTI_POKEMAX ; i++ )
    {
      param.status[ i ] = BTLV_BALL_GAUGE_STATUS_ALIVE;
    }
    wk->bbgw[ 0 ]  = BTLV_BALL_GAUGE_Create( &param, wk->heapID );

    param.type = BTLV_BALL_GAUGE_TYPE_MINE;

    wk->bbgw[ 1 ]  = BTLV_BALL_GAUGE_Create( &param, wk->heapID );
  }
#endif
  BTLV_EFFECT_Main();
  BTLV_INPUT_Main( wk->biw );

#if 0
  {
    MtxFx22 mtx;

    MTX_Scale22( &mtx, wk->scale, wk->scale );
    GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_X_SET, 128, &mtx, 256, 256 );
    GFL_BG_SetAffineScroll( GFL_BG_FRAME3_S, GFL_BG_SCROLL_Y_SET, 128 + 40, &mtx, 256, 256 );

    wk->scale += wk->scale_value;

    if( ( wk->scale == FX32_ONE ) || ( wk->scale == FX32_ONE * 3 ) )
    {
      wk->scale_value *= -1;
    }
  }
#endif

  if( pad == PAD_BUTTON_EXIT ){
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, 2 );
    return GFL_PROC_RES_FINISH;
  }
  else{
    return GFL_PROC_RES_CONTINUE;
  }
}
//--------------------------------------------------------------------------
/**
 * PROC Exit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugBattleTestProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SOGA_WORK* wk = mywk;

  if( GFL_FADE_CheckFade() == TRUE ){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_UI_KEY_SetRepeatSpeed( wk->key_repeat_speed, wk->key_repeat_wait );

  BTLV_EFFECT_Exit();

  GFL_CLACT_SYS_Delete();

  GFL_G3D_Exit();

  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_SOGABE_DEBUG );

//  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_view ) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA   DebugBattleTestProcData = {
  DebugBattleTestProcInit,
  DebugBattleTestProcMain,
  DebugBattleTestProcExit,
};

//======================================================================
//  カメラの制御
//======================================================================
static  void  MoveCamera( SOGA_WORK *wk )
{
  int pad = GFL_UI_KEY_GetCont();
  VecFx32 pos,ofsx,ofsz,camPos, camTarget;
  fx32  xofs=0,yofs=0,zofs=0;

  if( pad & PAD_BUTTON_L ){
    if( pad & PAD_KEY_LEFT ){
      BTLV_CAMERA_MoveCameraAngle( wk->bcw, 0, -ROTATE_SPEED );
    }
    if( pad & PAD_KEY_RIGHT ){
      BTLV_CAMERA_MoveCameraAngle( wk->bcw, 0,  ROTATE_SPEED );
    }
    if( pad & PAD_KEY_UP ){
      BTLV_CAMERA_MoveCameraAngle( wk->bcw, -ROTATE_SPEED, 0 );
    }
    if( pad & PAD_KEY_DOWN ){
      BTLV_CAMERA_MoveCameraAngle( wk->bcw,  ROTATE_SPEED, 0 );
    }
  }
  else{
    BTLV_CAMERA_GetCameraPosition( wk->bcw, &camPos, &camTarget );

    if( pad & PAD_KEY_LEFT ){
      xofs = -MOVE_SPEED;
    }
    if( pad & PAD_KEY_RIGHT ){
      xofs = MOVE_SPEED;
    }

    if( pad & PAD_BUTTON_R ){
      if( pad & PAD_KEY_UP ){
        zofs = MOVE_SPEED;
      }
      if( pad & PAD_KEY_DOWN ){
        zofs = -MOVE_SPEED;
      }
    }
    else{
      if( pad & PAD_KEY_UP ){
        yofs = MOVE_SPEED;
      }
      if( pad & PAD_KEY_DOWN ){
        yofs = -MOVE_SPEED;
      }
    }
    pos.x = camPos.x - camTarget.x;
    pos.y = 0;
    pos.z = camPos.z - camTarget.z;
    VEC_Normalize( &pos, &pos );

    ofsx.x = FX_MUL( pos.z, xofs );
    ofsx.y = 0;
    ofsx.z = -FX_MUL( pos.x, xofs );

    ofsz.x = -FX_MUL( pos.x, zofs );
    ofsz.y = yofs;
    ofsz.z = -FX_MUL( pos.z, zofs );

    camPos.x += ofsx.x + ofsz.x;
    camPos.y += ofsx.y + ofsz.y;
    camPos.z += ofsx.z + ofsz.z;
    camTarget.x += ofsx.x + ofsz.x;
    camTarget.y += ofsx.y + ofsz.y;
    camTarget.z += ofsx.z + ofsz.z;

    BTLV_CAMERA_MoveCameraPosition( wk->bcw, &camPos, &camTarget );
#if 0
    OS_TPrintf("pos_x:%08x pos_y:%08x pos_z:%08x tar_x:%08x tar_y:%08x tar_z:%08x\n",
      camPos.x,
      camPos.y,
      camPos.z,
      camTarget.x,
      camTarget.y,
      camTarget.z );
#endif
  }

}

static  void  set_pokemon( SOGA_WORK *wk )
{
  //POKEMON_PARAM生成
  POKEMON_PARAM *pp = GFL_HEAP_AllocMemory( wk->heapID, POKETOOL_GetWorkSize() );

  if( wk->timer_flag ){
    PP_SetupEx( pp, wk->mons_no, 0, 0, 0, 255 );
    PP_Put( pp, ID_PARA_id_no, 0x10 );
    BTLV_MCSS_Add( wk->bmw, pp, pokemon_pos_table[ wk->position ][ 0 ] );
    BTLV_MCSS_Add( wk->bmw, pp, pokemon_pos_table[ wk->position ][ 1 ] );
  }
  else{
#ifdef BTLV_MCSS_1vs1
//1vs1
    PP_SetupEx( pp, MONSNO_WARUBIRU, 0, 0, 0, 255 );
    PP_Put( pp, ID_PARA_id_no, 0x10 );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_AA );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_BB );
//    BTLV_EFFECT_SetGauge( pp, BTLV_MCSS_POS_AA );
//    BTLV_EFFECT_SetGauge( pp, BTLV_MCSS_POS_BB );
#else
//2vs2
    PP_SetupEx( pp, MONSNO_AUSU + 1, 0, 0, 0, 255 );
    PP_Put( pp, ID_PARA_id_no, 0x10 );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_A );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_B );
    PP_Put( pp, ID_PARA_monsno, MONSNO_AUSU + 2 );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_C );
    BTLV_EFFECT_SetPokemon( pp, BTLV_MCSS_POS_D );
#endif
  }
  GFL_HEAP_FreeMemory( pp );
}

static  void  del_pokemon( SOGA_WORK *wk )
{
  BTLV_MCSS_Del( wk->bmw, pokemon_pos_table[ wk->position ][ 0 ] );
  BTLV_MCSS_Del( wk->bmw, pokemon_pos_table[ wk->position ][ 1 ] );
}

#if 0
//======================================================================
//  カメラの初期化
//======================================================================
static  void  InitCamera( CAMERA_WORK *cw, HEAPID heapID )
{
//  VecFx32 cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 16.5f ), FX_F32_TO_FX32( 38.0f ) };
  VecFx32 cam_pos = { FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( 7.8f ), FX_F32_TO_FX32( 21.0f ) };

  cw->target.x = FX_F32_TO_FX32( 0.0f );
  cw->target.y = FX_F32_TO_FX32( 2.6f );
  cw->target.z = FX_F32_TO_FX32( 0.0f );
  cw->camUp.x = 0;
  cw->camUp.y = FX32_ONE;
  cw->camUp.z = 0;

  cw->phi   = FX_Atan2Idx( cam_pos.y, cam_pos.z );
  cw->theta = FX_Atan2Idx( cam_pos.y, cam_pos.x );
  cw->scale = VEC_Mag( &cam_pos );

  CalcCamera( cw );

  if( cw->camera == NULL ){
    cw->camera = GFL_G3D_CAMERA_Create( GFL_G3D_PRJPERS,
                      FX32_SIN13,
                      FX32_COS13,
//                      FX32_ONE * 4 / 3,
                      FX_F32_TO_FX32( 1.33f ),
                      NULL,
                      FX32_ONE,
                      FX32_ONE * 1000,
                      NULL,
                      &cw->camPos,
                      &cw->camUp,
                      &cw->target,
                      heapID );
  }
}

//======================================================================
//  カメラの制御
//======================================================================
static  void  MoveCamera( SOGA_WORK *wk )
{
  CAMERA_WORK *cw = &wk->cw;
  int pad = GFL_UI_KEY_GetCont();
  VecFx32 pos,ofsx,ofsz;
  fx32  xofs=0,yofs=0,zofs=0;

  if( pad & PAD_BUTTON_Y ){
    if( pad & PAD_KEY_LEFT ){
      cw->theta -= ROTATE_SPEED;
    }
    if( pad & PAD_KEY_RIGHT ){
      cw->theta += ROTATE_SPEED;
    }
    if( pad & PAD_KEY_UP ){
      cw->phi -= ROTATE_SPEED;
      if( cw->phi <= -PHI_MAX ){
        cw->phi = -PHI_MAX;
      }
    }
    if( pad & PAD_KEY_DOWN ){
      cw->phi += ROTATE_SPEED;
      if( cw->phi >= PHI_MAX ){
        cw->phi = PHI_MAX;
      }
    }
  }
  else{
    if( pad & PAD_KEY_LEFT ){
      xofs = -MOVE_SPEED;
    }
    if( pad & PAD_KEY_RIGHT ){
      xofs = MOVE_SPEED;
    }
    if( pad & PAD_KEY_UP ){
      yofs = MOVE_SPEED;
    }
    if( pad & PAD_KEY_DOWN ){
      yofs = -MOVE_SPEED;
    }
    if( pad & PAD_BUTTON_X ){
      zofs = MOVE_SPEED;
    }
    if( pad & PAD_BUTTON_B ){
      zofs = -MOVE_SPEED;
    }
    if( wk->mw.pos_x ){
      xofs = MOVE_SPEED * wk->mw.pos_x;
      wk->mw.pos_x = 0;
    }
    if( wk->mw.pos_y ){
      yofs = MOVE_SPEED * wk->mw.pos_y;
      wk->mw.pos_y = 0;
    }
    if( wk->mw.distance ){
      zofs = MOVE_SPEED * (wk->mw.distance / 50);
      wk->mw.distance = 0;
    }
    pos.x = cw->camPos.x - cw->target.x;
    pos.y = 0;
    pos.z = cw->camPos.z - cw->target.z;
    VEC_Normalize( &pos, &pos );

    ofsx.x = FX_MUL( pos.z, xofs );
    ofsx.y = 0;
    ofsx.z = -FX_MUL( pos.x, xofs );

    ofsz.x = -FX_MUL( pos.x, zofs );
    ofsz.y = yofs;
    ofsz.z = -FX_MUL( pos.z, zofs );

    cw->target.x += ofsx.x + ofsz.x;
    cw->target.y += ofsx.y + ofsz.y;
    cw->target.z += ofsx.z + ofsz.z;
  }
  if( pad & PAD_BUTTON_L ){
    if( cw->scale > 0 ){
      cw->scale -= SCALE_SPEED;
    }
  }
  if( pad & PAD_BUTTON_R ){
    if( cw->scale < SCALE_MAX ){
      cw->scale += SCALE_SPEED;
    }
  }
  if( wk->mw.theta ){
    cw->theta -= wk->mw.theta * 32 ;
    wk->mw.theta = 0;
  }

  if( wk->mw.phi ){
    cw->phi -= wk->mw.phi * 32 ;
    if( cw->phi <= -PHI_MAX ){
      cw->phi = -PHI_MAX;
    }
    if( cw->phi >= PHI_MAX ){
      cw->phi = PHI_MAX;
    }
    wk->mw.phi = 0;
  }

  CalcCamera( cw );
  GFL_G3D_CAMERA_SetPos( cw->camera, &cw->camPos );
  GFL_G3D_CAMERA_SetTarget( cw->camera, &cw->target );
  GFL_G3D_CAMERA_Switching( cw->camera );
}

//======================================================================
//  カメラ位置の計算
//======================================================================
static  void  CalcCamera( CAMERA_WORK *cw )
{
  MtxFx43 scale,trans;
  VecFx32 pos;

  //phiとthetaとscaleからcamPosを計算
  pos.x = FX_MUL( FX_CosIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
  pos.y = FX_SinIdx( cw->phi & 0xffff );
  pos.z = FX_MUL( FX_SinIdx( cw->theta & 0xffff ), FX_CosIdx( cw->phi & 0xffff ) );
  MTX_Scale43( &scale, cw->scale, cw->scale, cw->scale );
  MTX_MultVec43( &pos, &scale, &pos );
  MTX_Identity43( &trans );
//  MTX_TransApply43( &trans, &trans, cw->target.x, cw->target.y, cw->target.z );
  MTX_MultVec43( &pos, &trans, &cw->camPos );
}
#endif

