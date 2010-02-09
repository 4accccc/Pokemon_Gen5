//============================================================================================
/**
 * @file  trainer_viewer.c
 * @brief MCSを利用してトレーナーを見るビューワー
 * @author  soga
 * @date  2009.04.01
 */
//============================================================================================

#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>

#include "system/main.h"
#include "arc_def.h"

#include "battle/btlv/btlv_effect.h"

#include "tr_tool/tr_tool.h"
#include "tr_tool/trtype_def.h"

#include "debug/gf_mcs.h"

#include "print/printsys.h"
#include "print/wordset.h"
#include "msg/msg_d_soga.h"
#include "font/font.naix"
#include "message.naix"


//============================================================================================
//  定数定義
//============================================================================================

#define PAD_BUTTON_EXIT ( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START )
#define PAD_BUTTON_RESET_VALUE ( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_SELECT )

#define CAMERA_SPEED    ( FX32_ONE * 2 )
#define MOVE_SPEED      ( FX32_ONE >> 2 )
#define ROTATE_SPEED    ( 65536 / 128 )
#define PHI_MAX       ( ( 65536 / 360 ) * 89 )
#define DEFAULT_PHI     ( 65536 / 12 )
#define DEFAULT_THETA   ( 65536 / 4 )
#define DEFAULT_SCALE   ( FX32_ONE * 16  )
#define SCALE_SPEED     ( FX32_ONE )
#define SCALE_MAX     ( FX32_ONE * 200 )

#define HEAD_SIZE     ( 4 )

#define MCS_READ_CH     ( 0 )
#define MCS_WRITE_CH    ( 0 )

#define TR_INFO_X     ( 74 )
#define TR_INFO_Y     ( 96 - 6 )

#define NEW_POKEMON_START ( 500 )
#define NEW_POKEMON_END ( NEW_POKEMON_START + NEW_POKEMON_COUNT - 1 )

enum{
  BACK_COL = 0,
  LETTER_COL_NORMAL,
  LETTER_COL_SELECT,
  LETTER_COL_CURSOR,
  SHADOW_COL,
  BGCOL_AA,
  BGCOL_BB,
  BGCOL_A,
  BGCOL_B,
  BGCOL_C,
  BGCOL_D,
  BGCOL_E,
  BGCOL_F,
};

#define G2D_BACKGROUND_COL  ( GX_RGB(  0,  0,  0 ) )
#define G2D_FONT_COL        ( GX_RGB( 31, 31, 31 ) )
#define G2D_FONT_SELECT_COL ( GX_RGB(  0, 31,  0 ) )
#define G2D_FONT_CURSOR_COL ( GX_RGB( 31,  0,  0 ) )
#define G2D_SHADOW_COL      ( GX_RGB( 15, 15, 15 ) )
#define G2D_AA_BGCOL_ON     ( GX_RGB( 31,  0,  0 ) )
#define G2D_BB_BGCOL_ON     ( GX_RGB(  0, 31,  0 ) )
#define G2D_A_BGCOL_ON      ( GX_RGB(  0,  0, 31 ) )
#define G2D_B_BGCOL_ON      ( GX_RGB( 31, 31,  0 ) )
#define G2D_C_BGCOL_ON      ( GX_RGB( 31,  0, 31 ) )
#define G2D_D_BGCOL_ON      ( GX_RGB(  0, 31, 31 ) )
#define G2D_AA_BGCOL_OFF    ( GX_RGB( 15,  0,  0 ) )
#define G2D_BB_BGCOL_OFF    ( GX_RGB(  0, 15,  0 ) )
#define G2D_A_BGCOL_OFF     ( GX_RGB(  0,  0, 15 ) )
#define G2D_B_BGCOL_OFF     ( GX_RGB( 15, 15,  0 ) )
#define G2D_C_BGCOL_OFF     ( GX_RGB( 15,  0, 15 ) )
#define G2D_D_BGCOL_OFF     ( GX_RGB(  0, 15, 15 ) )

typedef struct{
  u8 posx;
  u8 posy;
  u8 sizx;
  u8 sizy;

  u8 palnum;
  u8 msgx;
  u8 msgy;
  u8 frame;
}BMP_CREATE_TABLE;

enum{
  SEND_NCBR = 0,
  SEND_NCBR_OK,
  SEND_NCLR,
  SEND_NCLR_OK,
  SEND_NCER,
  SEND_NCER_OK,
  SEND_NMCR,
  SEND_NMCR_OK,
  SEND_NANR,
  SEND_NANR_OK,
  SEND_NMAR,
  SEND_NMAR_OK,
  SEND_NCEC,
  SEND_NCEC_OK,
  SEND_POS,
  SEND_POS_OK,
  SEND_CLEAR,
  SEND_CLEAR_OK,
  SEND_IDLE,
};

enum{
  RESOURCE_NCBR,
  RESOURCE_NCLR,
  RESOURCE_NCER,
  RESOURCE_NMCR,
  RESOURCE_NANR,
  RESOURCE_NMAR,
  RESOURCE_NCEC,
  RESOURCE_MAX,
};

enum{
  SEQ_IDLE = 0,
  SEQ_LOAD_POS_DATA,
  SEQ_LOAD_RESOURCE_DATA,
  SEQ_CLEAR,
};

enum{
  BMPWIN_BB,
  BMPWIN_B,
  BMPWIN_D,

  BMPWIN_MAX
};

enum{
  VALUE_X = 0,
  VALUE_Y,
  VALUE_Z,
  VALUE_SCALE_PERS,
  VALUE_SCALE_ORTH,

  VALUE_MAX,
};

//============================================================================================
//  構造体定義
//============================================================================================

typedef struct
{
  HEAPID                heapID;
  GFL_MSGDATA*          msg;
  GFL_FONT*             font;
  GFL_FONT*             small_font;

  int                   seq_no;
  int                   mcs_enable;
  int                   trtype_no[ BMPWIN_MAX ];
  GFL_BMPWIN*           bmpwin[ BMPWIN_MAX ];
  GFL_BMPWIN*           bmpwin2;
  int                   key_repeat_speed;
  int                   key_repeat_wait;
  int                   read_position;
  int                   read_resource;
  int                   add_trainer_req;
  int                   edit_mode;
  int                   edit_pos;

  void*                 resource_data[ RESOURCE_MAX ][ BMPWIN_MAX ];
}TRAINER_VIEWER_WORK;

//============================================================================================
//  プロトタイプ宣言
//============================================================================================

static  void  TrainerViewerSequence( TRAINER_VIEWER_WORK *tvw );
static  BOOL  TrainerViewerSubSequence( TRAINER_VIEWER_WORK *tvw );
static  void  TrainerViewerRead( TRAINER_VIEWER_WORK *tvw );
static  void  TrainerViewerPositionLoad( TRAINER_VIEWER_WORK *tvw );
static  void  TrainerViewerResourceLoad( TRAINER_VIEWER_WORK *tvw );
static  void  TrainerViewerAddTrainer( TRAINER_VIEWER_WORK *tvw );

static  void  TextPrint( TRAINER_VIEWER_WORK *tvw, int num, int bmpwin_num );
static  void  TrainerViewerDrawInfo( TRAINER_VIEWER_WORK *tvw, int num );

static  void  MoveCamera( TRAINER_VIEWER_WORK *tvw );

static  void  set_trainer( TRAINER_VIEWER_WORK *tvw, int num );

//============================================================================================
//  データ
//============================================================================================

static  const u32 btlv_mcss_pos_msg[BMPWIN_MAX]={
  PVMSG_POS_BB,
  PVMSG_POS_B,
  PVMSG_POS_D,
};

static  const BtlvMcssPos tr_pos[BMPWIN_MAX]={
  BTLV_MCSS_POS_TR_BB,
  BTLV_MCSS_POS_TR_B,
  BTLV_MCSS_POS_TR_D,
};

static const BMP_CREATE_TABLE bmp_create_table[] = {
  //BB
  { 10,  0, 10, 24, BGCOL_BB, 16, 96, GFL_BG_FRAME1_S },
  //B
  { 20,  0, 12, 24, BGCOL_B,  16, 96, GFL_BG_FRAME1_S },
  //D
  {  0,  0, 10, 24, BGCOL_D,  16, 96, GFL_BG_FRAME1_S },
};

static const GFL_UI_TP_HITTBL TP_HitTbl[] = {
  {   0, 191,  80, 159 },   //BB
  {   0, 191, 160, 255 },   //B
  {   0, 191,   0,  79 },   //D
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};

FS_EXTERN_OVERLAY(battle_view);
FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TrainerViewerProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  TRAINER_VIEWER_WORK *tvw;

  static const GFL_DISP_VRAM dispvramBank = {
    GX_VRAM_BG_128_D,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
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

  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle_view));
  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle));

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
  tvw = GFL_PROC_AllocWork( proc, sizeof( TRAINER_VIEWER_WORK ), HEAPID_SOGABE_DEBUG );
  MI_CpuClearFast( tvw, sizeof( TRAINER_VIEWER_WORK ) );
  tvw->heapID = HEAPID_SOGABE_DEBUG;

  GFL_DISP_SetBank( &dispvramBank );

  //VRAMクリア
  MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
  MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

  G2_BlendNone();
  GFL_BG_Init( tvw->heapID );
  GFL_BMPWIN_Init( tvw->heapID );

  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_3D,
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
    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

  }
  GX_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  //3D関連初期化
  {
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, tvw->heapID, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_MANUAL, GX_BUFFERMODE_Z );
    G3X_AlphaBlend( TRUE );
    G3X_AlphaTest( FALSE, 31 );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( TRUE );
    GFL_BG_SetBGControl3D( 1 );
  }

  // フォント作成
  tvw->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, tvw->heapID );
  tvw->small_font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, tvw->heapID );

  //戦闘エフェクト初期化
  {
    BTL_FIELD_SITUATION bfs = { 
      0, 0, 0, 0, 0
    };

    GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &dispvramBank, tvw->heapID );
    BTLV_EFFECT_Init( BTL_RULE_SINGLE, &bfs, tvw->small_font, tvw->heapID );
  }

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
      ///<FRAME1_S
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
      ///<FRAME2_S
      {
        0, 0, 0x0800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
        GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,
        GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
      },
    };
    GFL_BG_SetBGControl(GFL_BG_FRAME1_M, &TextBgCntDat[0], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME1_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME2_M, &TextBgCntDat[1], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME2_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME3_M, &TextBgCntDat[2], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M );
    GFL_BG_SetBGControl(GFL_BG_FRAME1_S, &TextBgCntDat[3], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME1_S );
    GFL_BG_SetBGControl(GFL_BG_FRAME2_S, &TextBgCntDat[4], GFL_BG_MODE_TEXT );
    GFL_BG_ClearScreen(GFL_BG_FRAME2_S );
  }

  {
    int i;

    //フォントパレット作成＆転送
    {
      static  u16 plt[16] = {
        G2D_BACKGROUND_COL, G2D_FONT_COL, G2D_FONT_SELECT_COL, G2D_FONT_CURSOR_COL, G2D_SHADOW_COL,
        G2D_AA_BGCOL_ON, G2D_BB_BGCOL_ON, G2D_A_BGCOL_ON, G2D_B_BGCOL_ON, G2D_C_BGCOL_ON, G2D_D_BGCOL_ON,
        G2D_AA_BGCOL_OFF, G2D_BB_BGCOL_OFF, G2D_A_BGCOL_OFF, G2D_B_BGCOL_OFF, G2D_C_BGCOL_OFF };
        PaletteWorkSet( BTLV_EFFECT_GetPfd(), &plt, FADE_SUB_BG, 0, 16 * 2 );
    }

    //メッセージ系初期化
    GFL_FONTSYS_Init();
    tvw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_soga_dat, tvw->heapID );

    for( i = 0 ; i < BMPWIN_MAX ; i++ ){
      tvw->bmpwin[ i ] = GFL_BMPWIN_Create(
                         bmp_create_table[ i ].frame,
                         bmp_create_table[ i ].posx,
                         bmp_create_table[ i ].posy,
                         bmp_create_table[ i ].sizx,
                         bmp_create_table[ i ].sizy,
                         0,
                         GFL_BG_CHRAREA_GET_B );
      TextPrint( tvw, i, i );
    }
    tvw->bmpwin2 = GFL_BMPWIN_Create( GFL_BG_FRAME2_S, 0, 0, 32, 24, 0, GFL_BG_CHRAREA_GET_B );
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

  //フェードイン
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, 2 );

  GFL_UI_KEY_GetRepeatSpeed( &tvw->key_repeat_speed, &tvw->key_repeat_wait );
  GFL_UI_KEY_SetRepeatSpeed( tvw->key_repeat_speed / 4, tvw->key_repeat_wait );

  {
    VecFx32 pos = { 0x000004cd, 0x000047bd, 0x00003015 };
    VecFx32 tar = { 0xfffff4cd, 0x00003800, 0xffff9000 };

    BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &pos, &tar );
  }

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT TrainerViewerProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int pad = GFL_UI_KEY_GetCont();
  int trg = GFL_UI_KEY_GetTrg();
  int rep = GFL_UI_KEY_GetRepeat();
  TRAINER_VIEWER_WORK *tvw = mywk;
  int i, j;

  if( tvw->mcs_enable ){
    MCS_Main();
  }
  else
  {
    if( TrainerViewerSubSequence( tvw ) == TRUE )
    {
      BTLV_EFFECT_Main();
      return GFL_PROC_RES_CONTINUE;
    }
  }

  if( trg & PAD_BUTTON_START )
  {
    if( tvw->mcs_enable )
    {
      tvw->seq_no = SEQ_IDLE;
      MCS_Exit();
      tvw->mcs_enable = 0;
      for( i = 0 ; i < BMPWIN_MAX ; i++ )
      {
        for( j = 0 ; j < RESOURCE_MAX ; j++ )
        {
          if( tvw->resource_data[ j ][ i ] )
          {
            if( j == RESOURCE_NCBR )
            {
              BTLV_EFFECT_DelTrainer( tr_pos[ i ] );
            }
            tvw->resource_data[ j ][ i ] = NULL;
          }
        }
      }
    }
    else
    {
      if( MCS_Init( tvw->heapID ) == FALSE )
      {
        tvw->mcs_enable = 1;
      }
    }
  }

  if( (trg & PAD_BUTTON_DEBUG ) )
  {
    int mcss_pos;
    VecFx32 pos,target;
    VecFx32 scale_value[] = {
      { FX32_ONE * 16 * 2, FX32_ONE * 16 * 2, 0 },
      { FX32_ONE * 16, FX32_ONE * 16, 0 }
    };

    BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
    BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &pos, &target, 20, 0, 20 );

  }

  {
    int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
    if( hit != GFL_UI_TP_HIT_NONE ){
      if( BTLV_EFFECT_CheckExist( tr_pos[ hit ] ) == TRUE ){
        BTLV_MCSS_SetVanishFlag( BTLV_EFFECT_GetMcssWork(), hit, BTLV_MCSS_VANISH_FLIP );
        TextPrint( tvw, hit, hit );
      }
    }
  }

  TrainerViewerSequence( tvw );

  //MoveCamera( tvw );
  BTLV_EFFECT_Main();

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
static GFL_PROC_RESULT TrainerViewerProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  TRAINER_VIEWER_WORK *tvw = mywk;
  int i;

  if( GFL_FADE_CheckFade() == TRUE ){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_UI_KEY_SetRepeatSpeed( tvw->key_repeat_speed, tvw->key_repeat_wait );

  BTLV_EFFECT_Exit();

  GFL_CLACT_SYS_Delete();

  GFL_G3D_Exit();

  GFL_MSG_Delete( tvw->msg );
  GFL_FONT_Delete( tvw->font );
  GFL_FONT_Delete( tvw->small_font );

  for( i = 0 ; i < BMPWIN_MAX ; i++ ){
    GFL_BMPWIN_Delete( tvw->bmpwin[ i ] );
  }

  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_SOGABE_DEBUG );

  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle_view ) );
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( battle ) );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA   TrainerViewerProcData = {
  TrainerViewerProcInit,
  TrainerViewerProcMain,
  TrainerViewerProcExit,
};

//======================================================================
//  メインシーケンス
//======================================================================
static  void  TrainerViewerSequence( TRAINER_VIEWER_WORK *tvw )
{
  switch( tvw->seq_no ){
  default:
  case SEQ_IDLE:
    TrainerViewerAddTrainer( tvw );
    TrainerViewerRead( tvw );
    break;
  case SEQ_LOAD_POS_DATA:
    TrainerViewerPositionLoad( tvw );
    break;
  case SEQ_LOAD_RESOURCE_DATA:
    TrainerViewerResourceLoad( tvw );
    break;
  case SEQ_CLEAR:
    {
      int head = SEND_IDLE;
      int i;
      int res;

      MCS_Write( MCS_WRITE_CH, &head, 4 );
      tvw->seq_no = SEQ_IDLE;

      for( i = 0 ; i < BMPWIN_MAX ; i++ ){
        if( BTLV_EFFECT_CheckExist( tr_pos[ i ] ) == TRUE ){
          BTLV_EFFECT_DelTrainer( tr_pos[ i ] );
          for( res = RESOURCE_NCBR ; res < RESOURCE_MAX ; res++ ){
            //リソース自体の解放は、MCSS_Delの中でやっているので、NULL初期化だけ行う
            tvw->resource_data[ res ][ i ] = NULL;
          }
        }
        TextPrint( tvw, i, i );
      }
    }
    break;
  }
}

//======================================================================
//  ロムに入っているトレーナーを閲覧するモード
//======================================================================
static  BOOL  TrainerViewerSubSequence( TRAINER_VIEWER_WORK *tvw )
{
  BOOL  ret = FALSE;
  if( tvw->edit_mode )
  {
    int rep = GFL_UI_KEY_GetRepeat();
    int cont = GFL_UI_KEY_GetCont();
    int trg = GFL_UI_KEY_GetTrg();

    if( rep & PAD_KEY_UP )
    {
      tvw->trtype_no[ tvw->edit_pos ]++;
    }
    if( rep & PAD_KEY_DOWN )
    {
      tvw->trtype_no[ tvw->edit_pos ]--;
    }
    if( rep & PAD_KEY_LEFT )
    {
      tvw->trtype_no[ tvw->edit_pos ] -= 10;
    }
    if( rep & PAD_KEY_RIGHT )
    {
      tvw->trtype_no[ tvw->edit_pos ] += 10;
    }
    if( rep & PAD_BUTTON_L )
    {
      tvw->trtype_no[ tvw->edit_pos ] -= 100;
    }
    if( rep & PAD_BUTTON_R )
    {
      tvw->trtype_no[ tvw->edit_pos ] += 100;
    }
    while( tvw->trtype_no[ tvw->edit_pos ] < 0 )
    {
      tvw->trtype_no[ tvw->edit_pos ] += TRTYPE_MAX;
    }
    while( tvw->trtype_no[ tvw->edit_pos ] >= TRTYPE_MAX )
    {
      tvw->trtype_no[ tvw->edit_pos ] -= TRTYPE_MAX;
    }
    if( rep != 0 )
    {
      if( BTLV_EFFECT_CheckExist( tr_pos[ tvw->edit_pos ] ) == TRUE ){
          BTLV_EFFECT_DelTrainer( tr_pos[ tvw->edit_pos ] );
      }
      set_trainer( tvw, tvw->edit_pos );
      ret = TRUE;
    }
  }
  else
  {
    int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
    if( hit != GFL_UI_TP_HIT_NONE ){
      tvw->edit_mode = 1;
      tvw->edit_pos = hit;
      set_trainer( tvw, tvw->edit_pos );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_ON );
      ret = TRUE;
    }
  }

  if( ret == TRUE )
  {
    TrainerViewerDrawInfo( tvw, tvw->edit_pos );
  }

  return ret;
}

//======================================================================
//  MCSからの読み込みチェック
//======================================================================
static  void  TrainerViewerRead( TRAINER_VIEWER_WORK *tvw )
{
  u32 size;
  u32 head;

  if( tvw->mcs_enable == 0 ){
    return;
  }

  size = MCS_CheckRead();

  if( size ){
    MCS_Read( &head, HEAD_SIZE );
    if( head == SEND_CLEAR ){
      tvw->seq_no = SEQ_CLEAR;
    }
    else if( head == SEND_POS ){
      tvw->seq_no = SEQ_LOAD_POS_DATA;
    }
    else{
      tvw->seq_no = SEQ_LOAD_RESOURCE_DATA;
    }
    if( head == SEND_NCEC ){
      tvw->add_trainer_req = 1;
    }
    tvw->read_resource = head / 2;
    head++;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
  }
}

//======================================================================
//  MCSからのポジションデータ読み込みチェック
//======================================================================
static  void  TrainerViewerPositionLoad( TRAINER_VIEWER_WORK *tvw )
{
  u32 size;
  u32 head;
  int res;

  size = MCS_CheckRead();

  if( size ){
    MCS_Read( &tvw->read_position, size );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    tvw->seq_no = SEQ_IDLE;
    if( BTLV_EFFECT_CheckExist( tr_pos[ tvw->read_position ] ) == TRUE ){
      BTLV_EFFECT_DelTrainer( tr_pos[ tvw->read_position ] );
      for( res = RESOURCE_NCBR ; res < RESOURCE_MAX ; res++ ){
        //リソース自体の解放は、MCSS_Delの中でやっているので、NULL初期化だけ行う
        tvw->resource_data[ res ][ tr_pos[ tvw->read_position ] ] = NULL;
      }
    }
  }
}

//======================================================================
//  MCSからのシーケンス読み込みチェック
//======================================================================
static  void  TrainerViewerResourceLoad( TRAINER_VIEWER_WORK *tvw )
{
  u32 size;
  u32 head;

  size = MCS_CheckRead();

  if( size ){
    tvw->resource_data[ tvw->read_resource ][ tvw->read_position ] = GFL_HEAP_AllocClearMemory( tvw->heapID, size );
    MCS_Read( tvw->resource_data[ tvw->read_resource ][ tvw->read_position ], size );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    tvw->seq_no = SEQ_IDLE;
  }
}

//======================================================================
//  テキスト表示
//======================================================================
static  void  TextPrint( TRAINER_VIEWER_WORK *tvw, int num, int bmpwin_num )
{
  int flag = 0;
  STRBUF  *strbuf;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( tvw->bmpwin[ bmpwin_num ] ), bmp_create_table[ bmpwin_num ].palnum );

  GFL_FONTSYS_SetColor( BACK_COL, SHADOW_COL, bmp_create_table[ bmpwin_num ].palnum );

  strbuf = GFL_MSG_CreateString( tvw->msg,  btlv_mcss_pos_msg[ num ] );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( tvw->bmpwin[ bmpwin_num ] ),
          bmp_create_table[ bmpwin_num ].msgx,
          bmp_create_table[ bmpwin_num ].msgy,
          strbuf, tvw->font );
  GFL_HEAP_FreeMemory( strbuf );

  GFL_BMPWIN_TransVramCharacter( tvw->bmpwin[ bmpwin_num ] );
  GFL_BMPWIN_MakeScreen( tvw->bmpwin[ bmpwin_num ] );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//======================================================================
//  テキスト表示
//======================================================================
static  void  TrainerViewerDrawInfo( TRAINER_VIEWER_WORK *tvw, int num )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( tvw->bmpwin2 ), 0 );

  {
    STRBUF  *str_src;
    STRBUF  *str_dst = GFL_STR_CreateBuffer( 100, tvw->heapID );
    WORDSET*  tr_info = WORDSET_Create( tvw->heapID );
    GFL_MSGDATA*  man;

    GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, BACK_COL );

    WORDSET_RegisterNumber( tr_info, 0, tvw->trtype_no[ num ], 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    man = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_trtype_dat, tvw->heapID );
    GFL_MSG_GetString( man, tvw->trtype_no[ num ], str_dst );
    WORDSET_RegisterWord( tr_info, 1, str_dst, NULL, FALSE, NULL );
    GFL_MSG_Delete( man );

    str_src = GFL_MSG_CreateString( tvw->msg,  TVMSG_TRTYPE );
    WORDSET_ExpandStr( tr_info, str_dst, str_src );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( tvw->bmpwin2 ), TR_INFO_X, TR_INFO_Y, str_dst, tvw->font );
    GFL_HEAP_FreeMemory( str_src );
    GFL_HEAP_FreeMemory( str_dst );
    WORDSET_Delete( tr_info );
  }

  GFL_BMPWIN_TransVramCharacter( tvw->bmpwin2 );
  GFL_BMPWIN_MakeScreen( tvw->bmpwin2 );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
}

//======================================================================
//  カメラの制御
//======================================================================
static  void  MoveCamera( TRAINER_VIEWER_WORK *tvw )
{
  int pad = GFL_UI_KEY_GetCont();
  VecFx32 pos,ofsx,ofsz,camPos, camTarget;
  fx32  xofs=0,yofs=0,zofs=0;

  if( pad & PAD_BUTTON_L ){
    if( pad & PAD_KEY_LEFT ){
      BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), 0, -ROTATE_SPEED );
    }
    if( pad & PAD_KEY_RIGHT ){
      BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), 0,  ROTATE_SPEED );
    }
    if( pad & PAD_KEY_UP ){
      BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(), -ROTATE_SPEED, 0 );
    }
    if( pad & PAD_KEY_DOWN ){
      BTLV_CAMERA_MoveCameraAngle( BTLV_EFFECT_GetCameraWork(),  ROTATE_SPEED, 0 );
    }
  }
  else{
    BTLV_CAMERA_GetCameraPosition( BTLV_EFFECT_GetCameraWork(), &camPos, &camTarget );

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

    BTLV_CAMERA_MoveCameraPosition( BTLV_EFFECT_GetCameraWork(), &camPos, &camTarget );
  }

}

static  void  TrainerViewerAddTrainer( TRAINER_VIEWER_WORK *tvw )
{
  MCSS_ADD_DEBUG_WORK madw;

  if( tvw->add_trainer_req ){
    tvw->add_trainer_req = 0;
    madw.ncbr = tvw->resource_data[ RESOURCE_NCBR ][ tvw->read_position ];
    madw.nclr = tvw->resource_data[ RESOURCE_NCLR ][ tvw->read_position ];
    madw.ncer = tvw->resource_data[ RESOURCE_NCER ][ tvw->read_position ];
    madw.nmcr = tvw->resource_data[ RESOURCE_NMCR ][ tvw->read_position ];
    madw.nanr = tvw->resource_data[ RESOURCE_NANR ][ tvw->read_position ];
    madw.nmar = tvw->resource_data[ RESOURCE_NMAR ][ tvw->read_position ];
    madw.ncec = tvw->resource_data[ RESOURCE_NCEC ][ tvw->read_position ];

    BTLV_EFFECT_SetPokemonDebug( &madw, tr_pos[ tvw->read_position ] );
  }
}

static  void  set_trainer( TRAINER_VIEWER_WORK *tvw, int num )
{
  BTLV_EFFECT_SetTrainer( tvw->trtype_no[ num ], tr_pos[ num ], 0, 0, 0 );
}

