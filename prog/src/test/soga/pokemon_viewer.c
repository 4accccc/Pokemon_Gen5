//============================================================================================
/**
 * @file  pokemon_viewer.c
 * @brief MCSを利用してポケモンを見るビューワー
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

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

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

#define MONS_INFO_X     ( 74 )
#define MONS_INFO_Y     ( 96 - 6 )

#define PROJ_INFO_X     ( MONS_INFO_X )
#define PROJ_INFO_Y     ( 24 - 8 )

#define VALUE_INFO_X     ( MONS_INFO_X )
#define VALUE_INFO_Y     ( 48 - 8 )
#define VALUE_INFO_Y_OFS ( 24 )

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
  BMPWIN_AA = 0,
  BMPWIN_BB,
  BMPWIN_A,
  BMPWIN_B,
  BMPWIN_C,
  BMPWIN_D,
  BMPWIN_E,
  BMPWIN_F,

  BMPWIN_MAX
};

enum{
  VALUE_X = 0,
  VALUE_Y,
  VALUE_Z,
  VALUE_SCALE_PERS,
  VALUE_SCALE_ORTH,
  VALUE_PERSONAL_RND,

  VALUE_MAX,
};

enum{ 
  MODE_NORMAL = 0,
  MODE_3vs3,
  MODE_ROTATE,

  MODE_MAX,
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
  POKEMON_PARAM*        pp;
  int                   mons_no[ BTLV_MCSS_POS_MAX ];
  int                   form_no[ BTLV_MCSS_POS_MAX ];
  fx32                  value[ MODE_MAX ][ VALUE_MAX ][ BTLV_MCSS_POS_MAX ];
  GFL_BMPWIN*           bmpwin[ BMPWIN_MAX ];
  GFL_BMPWIN*           bmpwin2;
  int                   key_repeat_speed;
  int                   key_repeat_wait;
  int                   read_position;
  int                   read_resource;
  int                   add_pokemon_req;
  int                   mcss_mode;
  int                   edit_mode;
  int                   edit_pos;
  int                   edit_type;
  int                   edit_keta;
  int                   edit_value;
  int                   form_max;
  BTLV_MCSS_PROJECTION  proj;

  void*                 resource_data[ RESOURCE_MAX ][ BTLV_MCSS_POS_MAX ];
}POKEMON_VIEWER_WORK;

//============================================================================================
//  プロトタイプ宣言
//============================================================================================

static  void  PokemonViewerSequence( POKEMON_VIEWER_WORK *pvw );
static  BOOL  PokemonViewerSubSequence( POKEMON_VIEWER_WORK *pvw );
static  void  PokemonViewerRead( POKEMON_VIEWER_WORK *pvw );
static  void  PokemonViewerPositionLoad( POKEMON_VIEWER_WORK *pvw );
static  void  PokemonViewerResourceLoad( POKEMON_VIEWER_WORK *pvw );
static  void  PokemonViewerAddPokemon( POKEMON_VIEWER_WORK *pvw );
static  void  PokemonViewerCameraWork( POKEMON_VIEWER_WORK *pvw );

static  void  TextPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num );
static  void  PokemonViewerDrawInfo( POKEMON_VIEWER_WORK *pvw, BtlvMcssPos pos );
static  void  PokemonViewerPP_Put( POKEMON_VIEWER_WORK *pvw, int pos );

static  void  MoveCamera( POKEMON_VIEWER_WORK *pvw );

static  void  set_pokemon( POKEMON_VIEWER_WORK *pvw, BtlvMcssPos pos );

static  void  get_default_value( POKEMON_VIEWER_WORK *pvw );
static  void  mcss_mode_change( POKEMON_VIEWER_WORK *pvw );

#if 0
static  void  del_pokemon( POKEMON_VIEWER_WORK *pvw );
#endif

//============================================================================================
//  データ
//============================================================================================

static  const int pokemon_pos_table[][2]={
  { BTLV_MCSS_POS_AA, BTLV_MCSS_POS_BB },
  { BTLV_MCSS_POS_A, BTLV_MCSS_POS_B },
  { BTLV_MCSS_POS_C, BTLV_MCSS_POS_D }
};

static  const u32 btlv_mcss_pos_msg[BMPWIN_MAX]={
  PVMSG_POS_AA,
  PVMSG_POS_BB,
  PVMSG_POS_A,
  PVMSG_POS_B,
  PVMSG_POS_C,
  PVMSG_POS_D,
  PVMSG_POS_E,
  PVMSG_POS_F,
};

static const BMP_CREATE_TABLE bmp_create_table[] = {
  //AA
  {  0, 12,  8, 12, BGCOL_AA, 12, 40, GFL_BG_FRAME1_S },
  //BB
  {  0,  0,  8, 12, BGCOL_BB, 12, 40, GFL_BG_FRAME1_S },
  //A
  {  8, 12,  8, 12, BGCOL_A,  16, 40, GFL_BG_FRAME1_S },
  //B
  { 24,  0,  8, 12, BGCOL_B,  16, 40, GFL_BG_FRAME1_S },
  //C
  { 16, 12,  8, 12, BGCOL_C,  16, 40, GFL_BG_FRAME1_S },
  //D
  { 16,  0,  8, 12, BGCOL_D,  16, 40, GFL_BG_FRAME1_S },
  //E
  { 24, 12,  8, 12, BGCOL_E,  16, 40, GFL_BG_FRAME1_S },
  //F
  {  8,  0,  8, 12, BGCOL_F,  16, 40, GFL_BG_FRAME1_S },
};

static const GFL_UI_TP_HITTBL TP_HitTbl[] = {
  {  96, 191,   0,  63 },   //AA
  {   0,  95,   0,  63 },   //BB
  {  96, 191,  64, 127 },   //A
  {   0,  95, 192, 255 },   //B
  {  96, 191, 128, 191 },   //C
  {   0,  95, 128, 191 },   //D
  {  96, 191, 192, 255 },   //E
  {   0,  95,  64, 127 },   //F
  { GFL_UI_TP_HIT_END, 0, 0, 0 },
};

//FS_EXTERN_OVERLAY(battle_view);
FS_EXTERN_OVERLAY(battle);

//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonViewerProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_VIEWER_WORK *pvw;
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

//  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle_view));
  GFL_OVERLAY_Load(FS_OVERLAY_ID(battle));

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SOGABE_DEBUG, 0xc0000 );
  pvw = GFL_PROC_AllocWork( proc, sizeof( POKEMON_VIEWER_WORK ), HEAPID_SOGABE_DEBUG );
  MI_CpuClearFast( pvw, sizeof( POKEMON_VIEWER_WORK ) );
  pvw->heapID = HEAPID_SOGABE_DEBUG;

  GFL_DISP_SetBank( &dispvramBank );

  //VRAMクリア
  MI_CpuClear32((void*)HW_BG_VRAM, HW_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_BG_VRAM, HW_DB_BG_VRAM_SIZE);
  MI_CpuClear32((void*)HW_OBJ_VRAM, HW_OBJ_VRAM_SIZE);
  MI_CpuClear32((void*)HW_DB_OBJ_VRAM, HW_DB_OBJ_VRAM_SIZE);
  MI_CpuFill16((void*)HW_BG_PLTT, 0x0000, HW_BG_PLTT_SIZE);

  G2_BlendNone();
  GFL_BG_Init( pvw->heapID );
  GFL_BMPWIN_Init( pvw->heapID );

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
    GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX128K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K, 0, pvw->heapID, NULL );
    GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO, GX_BUFFERMODE_Z );
    G3X_AlphaBlend( TRUE );
    G3X_AlphaTest( FALSE, 31 );
    G3X_EdgeMarking( TRUE );
    G3X_AntiAlias( TRUE );
    GFL_BG_SetBGControl3D( 1 );
  }

  // フォント
    pvw->font = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, pvw->heapID );
    pvw->small_font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_batt_gftr, GFL_FONT_LOADTYPE_FILE, TRUE, pvw->heapID );

  //戦闘エフェクト初期化
  {
    BTL_FIELD_SITUATION bfs = { 
      0, 0, 0, 0, 0
    };

    GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN, &dispvramBank, pvw->heapID );
    BTLV_EFFECT_Init( BTL_RULE_SINGLE, &bfs, pvw->small_font, pvw->heapID );
  }

//  set_pokemon( pvw );

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
//      GFL_BG_LoadPalette( GFL_BG_FRAME1_S, &plt, 16*2, 0 );
        PaletteWorkSet( BTLV_EFFECT_GetPfd(), &plt, FADE_SUB_BG, 0, 16 * 2 );
    }

    //メッセージ系初期化
    GFL_FONTSYS_Init();
    pvw->msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_soga_dat, pvw->heapID );

    for( i = 0 ; i < BMPWIN_MAX ; i++ ){
      pvw->bmpwin[ i ] = GFL_BMPWIN_Create(
                         bmp_create_table[ i ].frame,
                         bmp_create_table[ i ].posx,
                         bmp_create_table[ i ].posy,
                         bmp_create_table[ i ].sizx,
                         bmp_create_table[ i ].sizy,
                         0,
                         GFL_BG_CHRAREA_GET_B );
      TextPrint( pvw, i, i );
    }

    pvw->bmpwin2 = GFL_BMPWIN_Create( GFL_BG_FRAME2_S, 0, 0, 32, 24, 0, GFL_BG_CHRAREA_GET_B );

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

  GFL_UI_KEY_GetRepeatSpeed( &pvw->key_repeat_speed, &pvw->key_repeat_wait );
  GFL_UI_KEY_SetRepeatSpeed( pvw->key_repeat_speed / 4, pvw->key_repeat_wait );

  //POKEMON_PARAM生成
  pvw->pp = GFL_HEAP_AllocMemory( pvw->heapID, POKETOOL_GetWorkSize() );
  PP_SetupEx( pvw->pp, 0, 0, 0, 0, 255 );

  pvw->mcss_mode = 0;
  get_default_value( pvw );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT PokemonViewerProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  int pad = GFL_UI_KEY_GetCont();
  int trg = GFL_UI_KEY_GetTrg();
  int rep = GFL_UI_KEY_GetRepeat();
  POKEMON_VIEWER_WORK *pvw = mywk;
  int i, j;

  if( pvw->mcs_enable ){
    MCS_Main();
  }
  else
  {
    if( PokemonViewerSubSequence( pvw ) == TRUE )
    {
      BTLV_EFFECT_Main();
      return GFL_PROC_RES_CONTINUE;
    }
  }

  if( ( trg & PAD_BUTTON_START ) && ( pvw->edit_mode == 0 ) )
  {
    if( pvw->mcs_enable )
    {
      pvw->seq_no = SEQ_IDLE;
      MCS_Exit();
      pvw->mcs_enable = 0;
      for( i = 0 ; i < BTLV_MCSS_POS_MAX ; i++ )
      {
        for( j = 0 ; j < RESOURCE_MAX ; j++ )
        {
          if( pvw->resource_data[ j ][ i ] )
          {
            if( j == RESOURCE_NCBR )
            {
              BTLV_EFFECT_DelPokemon( i );
            }
            pvw->resource_data[ j ][ i ] = NULL;
          }
        }
      }
    }
    else
    {
      if( MCS_Init( pvw->heapID ) == FALSE )
      {
        pvw->mcs_enable = 1;
      }
    }
  }
#if 0
  if( (trg & PAD_BUTTON_SELECT ) && ( BTLV_EFFECT_CheckExecute() == FALSE ) )
  {
    PokemonViewerCameraWork( pvw );
  }
#endif
  if( trg & PAD_BUTTON_SELECT )
  {
    mcss_mode_change( pvw );
  }

  if( trg & PAD_BUTTON_L )
  {
    int mcss_mode;
    int mcss_pos;

    for( mcss_mode = 0 ; mcss_mode < MODE_MAX ; mcss_mode++ )
    {
      for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ )
      {
        switch( mcss_mode ){ 
        case MODE_NORMAL:
          OS_TPrintf("2vs2 pos:%d pos_x:%08x pos_y:%08x pos_z:%08x scale:%08x\n", mcss_pos,
              pvw->value[ mcss_mode ][ VALUE_X ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Y ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Z ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_SCALE_PERS ][ mcss_pos ] );
          break;
        case MODE_3vs3:
          OS_TPrintf("3vs3 pos:%d pos_x:%08x pos_y:%08x pos_z:%08x scale:%08x\n", mcss_pos,
              pvw->value[ mcss_mode ][ VALUE_X ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Y ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Z ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_SCALE_PERS ][ mcss_pos ] );
          break;
        case MODE_ROTATE:
          OS_TPrintf("rotate pos:%d pos_x:%08x pos_y:%08x pos_z:%08x scale:%08x\n", mcss_pos,
              pvw->value[ mcss_mode ][ VALUE_X ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Y ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_Z ][ mcss_pos ],
              pvw->value[ mcss_mode ][ VALUE_SCALE_PERS ][ mcss_pos ] );
          break;
        }
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

#if 0
    for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ )
    {
      if( BTLV_EFFECT_CheckExist( mcss_pos ) == TRUE )
      {
        BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(),
                   mcss_pos,
                   EFFTOOL_CALCTYPE_INTERPOLATION,
                   &scale_value[ mcss_pos & 1 ],
                   10, 1, 0 );
      }
    }
#endif
  }

  {
    int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
    if( ( pvw->mcss_mode == 0 ) && ( hit > BMPWIN_D ) )
    {
      hit = GFL_UI_TP_HIT_NONE;
    }
    if( hit != GFL_UI_TP_HIT_NONE ){
      if( BTLV_EFFECT_CheckExist( hit ) == TRUE ){
        BTLV_MCSS_SetVanishFlag( BTLV_EFFECT_GetMcssWork(), hit, BTLV_MCSS_VANISH_FLIP );
        TextPrint( pvw, hit, hit );
      }
    }
  }

  PokemonViewerSequence( pvw );

  MoveCamera( pvw );
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
static GFL_PROC_RESULT PokemonViewerProcExit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  POKEMON_VIEWER_WORK *pvw = mywk;
  int i;

  if( GFL_FADE_CheckFade() == TRUE ){
    return GFL_PROC_RES_CONTINUE;
  }

  GFL_UI_KEY_SetRepeatSpeed( pvw->key_repeat_speed, pvw->key_repeat_wait );

  GFL_HEAP_FreeMemory( pvw->pp );

  BTLV_EFFECT_Exit();

  GFL_CLACT_SYS_Delete();

  GFL_G3D_Exit();

  GFL_MSG_Delete( pvw->msg );
  GFL_FONT_Delete( pvw->font );
  GFL_FONT_Delete( pvw->small_font );

  for( i = 0 ; i < BMPWIN_MAX ; i++ ){
    GFL_BMPWIN_Delete( pvw->bmpwin[ i ] );
  }
  GFL_BMPWIN_Delete( pvw->bmpwin2 );

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
const GFL_PROC_DATA   PokemonViewerProcData = {
  PokemonViewerProcInit,
  PokemonViewerProcMain,
  PokemonViewerProcExit,
};

//======================================================================
//  メインシーケンス
//======================================================================
static  void  PokemonViewerSequence( POKEMON_VIEWER_WORK *pvw )
{
  switch( pvw->seq_no ){
  default:
  case SEQ_IDLE:
    PokemonViewerAddPokemon( pvw );
    PokemonViewerRead( pvw );
    break;
  case SEQ_LOAD_POS_DATA:
    PokemonViewerPositionLoad( pvw );
    break;
  case SEQ_LOAD_RESOURCE_DATA:
    PokemonViewerResourceLoad( pvw );
    break;
  case SEQ_CLEAR:
    {
      int head = SEND_IDLE;
      int mcss_pos;
      int res;

      MCS_Write( MCS_WRITE_CH, &head, 4 );
      pvw->seq_no = SEQ_IDLE;

      for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ ){
        if( BTLV_EFFECT_CheckExist( mcss_pos ) == TRUE ){
          BTLV_EFFECT_DelPokemon( mcss_pos );
          for( res = RESOURCE_NCBR ; res < RESOURCE_MAX ; res++ ){
            //リソース自体の解放は、MCSS_Delの中でやっているので、NULL初期化だけ行う
            pvw->resource_data[ res ][ mcss_pos ] = NULL;
          }
        }
        if( mcss_pos < BMPWIN_MAX ){
          TextPrint( pvw, mcss_pos, mcss_pos );
        }
      }
    }
    break;
  }
}

//======================================================================
//  ロムに入っているポケモンを閲覧するモード
//======================================================================
static  BOOL  PokemonViewerSubSequence( POKEMON_VIEWER_WORK *pvw )
{
  BOOL  ret = FALSE;
  if( pvw->edit_mode )
  {
    int rep = GFL_UI_KEY_GetRepeat();
    int cont = GFL_UI_KEY_GetCont();
    int trg = GFL_UI_KEY_GetTrg();

    if( trg & PAD_BUTTON_DEBUG )
    {
      pvw->edit_type ^= 1;
      pvw->edit_mode = 1;
      pvw->edit_keta = 0;
    }
    if( pvw->edit_type == 0 )
    {
      if( rep & PAD_KEY_UP )
      {
        pvw->mons_no[ pvw->edit_pos ]++;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep & PAD_KEY_DOWN )
      {
        pvw->mons_no[ pvw->edit_pos ]--;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep & PAD_KEY_LEFT )
      {
        pvw->mons_no[ pvw->edit_pos ] -= 10;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep & PAD_KEY_RIGHT )
      {
        pvw->mons_no[ pvw->edit_pos ] += 10;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep & PAD_BUTTON_L )
      {
        pvw->mons_no[ pvw->edit_pos ] -= 100;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep & PAD_BUTTON_R )
      {
        pvw->mons_no[ pvw->edit_pos ] += 100;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( ( rep & PAD_BUTTON_X ) && ( pvw->form_no[ pvw->edit_pos ] < pvw->form_max - 1 ) )
      { 
        pvw->form_no[ pvw->edit_pos ]++;
      }
      if( ( rep & PAD_BUTTON_Y ) && ( pvw->form_no[ pvw->edit_pos ] > 0 ) )
      { 
        pvw->form_no[ pvw->edit_pos ]--;
      }
      if( pvw->mons_no[ pvw->edit_pos ] < 0 )
      {
        pvw->mons_no[ pvw->edit_pos ] += MONSNO_END + 1;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( pvw->mons_no[ pvw->edit_pos ] > MONSNO_END )
      {
        pvw->mons_no[ pvw->edit_pos ] -= MONSNO_END + 1;
        pvw->form_no[ pvw->edit_pos ] = 0;
      }
      if( rep != 0 )
      {
        if( BTLV_EFFECT_CheckExist( pvw->edit_pos ) == TRUE ){
            BTLV_EFFECT_DelPokemon( pvw->edit_pos );
        }
        PokemonViewerPP_Put( pvw, pvw->edit_pos );
        set_pokemon( pvw, pvw->edit_pos );
      }
    }
    else
    {
      int index = pvw->edit_value;

      if( index == VALUE_SCALE_PERS )
      {
        index += pvw->proj;
      }

      if( rep & PAD_KEY_UP )
      {
        if( pvw->edit_mode == 1 )
        {
          if( pvw->edit_value == VALUE_PERSONAL_RND ) 
          { 
            pvw->edit_value = VALUE_SCALE_PERS; 
          }
          else if( pvw->edit_value )
          {
            pvw->edit_value--;
          }
        }
        else
        {
          if( index == VALUE_PERSONAL_RND )
          { 
            pvw->value[ MODE_NORMAL ][ index ][ pvw->edit_pos ] += ( 1 << ( pvw->edit_keta * 4 ) );
          }
          else 
          { 
            pvw->value[ pvw->mcss_mode ][ index ][ pvw->edit_pos ] += ( 1 << ( pvw->edit_keta * 4 ) );
          }
        }
      }
      if( rep & PAD_KEY_DOWN )
      {
        if( pvw->edit_mode == 1 )
        {
          if( pvw->edit_value == VALUE_SCALE_PERS )
          { 
            pvw->edit_value = VALUE_PERSONAL_RND; 
          }
          else if( pvw->edit_value < VALUE_SCALE_PERS )
          {
            pvw->edit_value++;
          }
        }
        else
        {
          if( index == VALUE_PERSONAL_RND )
          { 
            pvw->value[ MODE_NORMAL ][ index ][ pvw->edit_pos ] -= ( 1 << ( pvw->edit_keta * 4 ) );
          }
          else
          { 
            pvw->value[ pvw->mcss_mode ][ index ][ pvw->edit_pos ] -= ( 1 << ( pvw->edit_keta * 4 ) );
          }
        }
      }
      if( ( rep & PAD_KEY_LEFT ) && ( pvw->edit_keta < 7 ) && ( pvw->edit_mode == 2 ) )
      {
        pvw->edit_keta++;
      }
      if( ( rep & PAD_KEY_RIGHT ) && ( pvw->edit_keta > 0 ) && ( pvw->edit_mode == 2 ) )
      {
        pvw->edit_keta--;
      }
      if( ( cont == PAD_BUTTON_RESET_VALUE ) && ( BTLV_EFFECT_CheckExist( pvw->edit_pos ) == TRUE ) )
      {
        VecFx32 vpos;
        VecFx32 scale;

        BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &vpos, pvw->edit_pos );
        pvw->value[ pvw->mcss_mode ][ VALUE_X ][ pvw->edit_pos ] = vpos.x;
        pvw->value[ pvw->mcss_mode ][ VALUE_Y ][ pvw->edit_pos ] = vpos.y;
        pvw->value[ pvw->mcss_mode ][ VALUE_Z ][ pvw->edit_pos ] = vpos.z;
        pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ] = BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos, pvw->proj );
        BTLV_MCSS_SetPosition( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos,
                               pvw->value[ pvw->mcss_mode ][ VALUE_X ][ pvw->edit_pos ],
                               pvw->value[ pvw->mcss_mode ][ VALUE_Y ][ pvw->edit_pos ],
                               pvw->value[ pvw->mcss_mode ][ VALUE_Z ][ pvw->edit_pos ] );
        VEC_Set( &scale,
                 pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                 pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                 FX32_ONE );
        BTLV_MCSS_SetScale( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos, &scale );
      }
      if( ( trg & PAD_BUTTON_Y ) && ( BTLV_EFFECT_CheckExist( pvw->edit_pos ) == TRUE ) )
      {
        VecFx32 scale;
        pvw->proj ^= 1;
        if( pvw->proj )
        {
          BTLV_MCSS_SetOrthoMode( BTLV_EFFECT_GetMcssWork() );
        }
        else
        {
          BTLV_MCSS_ResetOrthoMode( BTLV_EFFECT_GetMcssWork() );
        }
        VEC_Set( &scale,
                 pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                 pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                 FX32_ONE );
        BTLV_MCSS_SetScale( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos, &scale );
      }
      if( rep != 0 )
      {
        if( BTLV_EFFECT_CheckExist( pvw->edit_pos ) == TRUE ){
          VecFx32 scale;
          BTLV_MCSS_SetPosition( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos,
                                 pvw->value[ pvw->mcss_mode ][ VALUE_X ][ pvw->edit_pos ],
                                 pvw->value[ pvw->mcss_mode ][ VALUE_Y ][ pvw->edit_pos ],
                                 pvw->value[ pvw->mcss_mode ][ VALUE_Z ][ pvw->edit_pos ] );
          VEC_Set( &scale,
                   pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                   pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ pvw->edit_pos ],
                   FX32_ONE );
          BTLV_MCSS_SetScale( BTLV_EFFECT_GetMcssWork(), pvw->edit_pos, &scale );
        }
      }
    }
    if( trg & PAD_BUTTON_A )
    {
      pvw->edit_mode ^= 1 ;
      pvw->edit_mode ^= 2 ;
    }
    if( trg & PAD_BUTTON_B )
    {
      pvw->edit_mode = 0;
      GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_ON );
      GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    }
    if( ( trg & PAD_BUTTON_R ) && ( pvw->edit_value == VALUE_PERSONAL_RND ) )
    { 
      BTLV_EFFECT_DelPokemon( pvw->edit_pos );
      PP_SetupEx( pvw->pp, pvw->mons_no[ pvw->edit_pos ], 1, 12345678, 0,
                  pvw->value[ MODE_NORMAL ][ VALUE_PERSONAL_RND ][ pvw->edit_pos ] );
      set_pokemon( pvw, pvw->edit_pos );
    }
    if( cont != 0)
    {
      ret = TRUE;
    }
  }
  else
  {
    int hit = GFL_UI_TP_HitTrg( TP_HitTbl );
    if( ( pvw->mcss_mode == 0 ) && ( hit > BMPWIN_D ) )
    {
      hit = GFL_UI_TP_HIT_NONE;
    }
    if( hit != GFL_UI_TP_HIT_NONE ){
      pvw->edit_mode = 1;
      pvw->edit_pos = hit;
      pvw->edit_type = 0;
      pvw->edit_keta = 0;
      pvw->edit_value = VALUE_X;
      pvw->proj = BTLV_MCSS_PROJ_ORTHO;
      PokemonViewerPP_Put( pvw, hit );
      GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
      GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_ON );
      ret = TRUE;
    }
  }

  if( ret == TRUE )
  {
    pvw->form_max = POKETOOL_GetPersonalParam( pvw->mons_no[ pvw->edit_pos ], 0, POKEPER_ID_form_max );
    PokemonViewerDrawInfo( pvw, pvw->edit_pos );
  }

  return ret;
}

//======================================================================
//  MCSからの読み込みチェック
//======================================================================
static  void  PokemonViewerRead( POKEMON_VIEWER_WORK *pvw )
{
  u32 size;
  u32 head;

  if( pvw->mcs_enable == 0 ){
    return;
  }

  size = MCS_CheckRead();

  if( size ){
    MCS_Read( &head, HEAD_SIZE );
    if( head == SEND_CLEAR ){
      pvw->seq_no = SEQ_CLEAR;
    }
    else if( head == SEND_POS ){
      pvw->seq_no = SEQ_LOAD_POS_DATA;
    }
    else{
      pvw->seq_no = SEQ_LOAD_RESOURCE_DATA;
    }
    if( head == SEND_NCEC ){
      pvw->add_pokemon_req = 1;
    }
    pvw->read_resource = head / 2;
    head++;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
  }
}

//======================================================================
//  MCSからのポジションデータ読み込みチェック
//======================================================================
static  void  PokemonViewerPositionLoad( POKEMON_VIEWER_WORK *pvw )
{
  u32 size;
  u32 head;
  int res;

  if( pvw->mcs_enable == 0 ){
    return;
  }

  size = MCS_CheckRead();

  if( size ){
    MCS_Read( &pvw->read_position, size );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    pvw->seq_no = SEQ_IDLE;
    if( BTLV_EFFECT_CheckExist( pvw->read_position ) == TRUE ){
      BTLV_EFFECT_DelPokemon( pvw->read_position );
      for( res = RESOURCE_NCBR ; res < RESOURCE_MAX ; res++ ){
        //リソース自体の解放は、MCSS_Delの中でやっているので、NULL初期化だけ行う
        pvw->resource_data[ res ][ pvw->read_position ] = NULL;
      }
    }
  }
}

//======================================================================
//  MCSからのシーケンス読み込みチェック
//======================================================================
static  void  PokemonViewerResourceLoad( POKEMON_VIEWER_WORK *pvw )
{
  u32 size;
  u32 head;

  if( pvw->mcs_enable == 0 ){
    return;
  }

  size = MCS_CheckRead();

  if( size ){
    pvw->resource_data[ pvw->read_resource ][ pvw->read_position ] = GFL_HEAP_AllocClearMemory( pvw->heapID, size );
    MCS_Read( pvw->resource_data[ pvw->read_resource ][ pvw->read_position ], size );
    head = SEND_IDLE;
    MCS_Write( MCS_WRITE_CH, &head, 4 );
    pvw->seq_no = SEQ_IDLE;
  }
}

//======================================================================
//  テキスト表示
//======================================================================
static  void  TextPrint( POKEMON_VIEWER_WORK *pvw, int num, int bmpwin_num )
{
  int flag = 0;
  STRBUF  *strbuf;

  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] ), bmp_create_table[ bmpwin_num ].palnum );

  if( ( pvw->mcss_mode == 0 ) && ( num > BMPWIN_D ) )
  {
    GFL_FONTSYS_SetColor( BACK_COL, SHADOW_COL, bmp_create_table[ bmpwin_num ].palnum );
  }
  else
  {
    GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, bmp_create_table[ bmpwin_num ].palnum );
  }

  strbuf = GFL_MSG_CreateString( pvw->msg,  btlv_mcss_pos_msg[ num ] );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( pvw->bmpwin[ bmpwin_num ] ),
          bmp_create_table[ bmpwin_num ].msgx,
          bmp_create_table[ bmpwin_num ].msgy,
          strbuf, pvw->font );
  GFL_HEAP_FreeMemory( strbuf );

  GFL_BMPWIN_TransVramCharacter( pvw->bmpwin[ bmpwin_num ] );
  GFL_BMPWIN_MakeScreen( pvw->bmpwin[ bmpwin_num ] );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME1_S );
}

//======================================================================
//  テキスト表示
//======================================================================
static  void  PokemonViewerDrawInfo( POKEMON_VIEWER_WORK *pvw, BtlvMcssPos pos )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( pvw->bmpwin2 ), 0 );

  if( pvw->edit_type == 0 )
  {
    STRBUF  *str_src;
    STRBUF  *str_dst = GFL_STR_CreateBuffer( 100, pvw->heapID );
    WORDSET*  mons_info = WORDSET_Create( pvw->heapID );

    GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, BACK_COL );

    WORDSET_RegisterNumber( mons_info, 0, pvw->mons_no[ pos ], 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    if( pvw->mons_no[ pos ] <= MONSNO_END )
    {
      WORDSET_RegisterPokeMonsName( mons_info, 1, pvw->pp );
    }
    str_src = GFL_MSG_CreateString( pvw->msg,  PVMSG_MONSNAME );
    WORDSET_ExpandStr( mons_info, str_dst, str_src );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( pvw->bmpwin2 ), MONS_INFO_X, MONS_INFO_Y, str_dst, pvw->font );

    WORDSET_RegisterNumber( mons_info, 0, pvw->form_no[ pos ], 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    { 
      int form_max = ( pvw->form_max == 0 ) ? 0 : pvw->form_max - 1;
      WORDSET_RegisterNumber( mons_info, 1, form_max, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_HANKAKU );
    }
    str_src = GFL_MSG_CreateString( pvw->msg,  PVMSG_FORMNAME );
    WORDSET_ExpandStr( mons_info, str_dst, str_src );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( pvw->bmpwin2 ), MONS_INFO_X, MONS_INFO_Y + 16, str_dst, pvw->font );

    GFL_HEAP_FreeMemory( str_src );
    GFL_HEAP_FreeMemory( str_dst );
    WORDSET_Delete( mons_info );
  }
  else
  {
    STRBUF  *strbuf;
    int     value;
    int     keta;
    u32     num;

    GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, BACK_COL );
    strbuf = GFL_MSG_CreateString( pvw->msg,  PVMSG_PERS + pvw->proj );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( pvw->bmpwin2 ), PROJ_INFO_X, PROJ_INFO_Y, strbuf, pvw->font );

    for( value = VALUE_X ; value < VALUE_MAX ; value++ )
    {
      int index = value;
      int ofsx = 0;
      int ofsy = value;

      if( value == VALUE_SCALE_ORTH ) continue;

      if( value == VALUE_PERSONAL_RND )
      { 
        ofsy--;
      }

      if( value == VALUE_SCALE_PERS )
      {
        index += pvw->proj;
      }
      for( keta = 7 ; keta >= 0 ; keta-- ){
        if( pvw->edit_mode == 1 )
        {
          if( pvw->edit_value == value )
          {
            GFL_FONTSYS_SetColor( LETTER_COL_CURSOR, SHADOW_COL, BACK_COL );
          }
          else
          {
            GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, BACK_COL );
          }
        }
        else
        {
          if( ( keta == pvw->edit_keta ) && ( value == pvw->edit_value ) )
          {
            GFL_FONTSYS_SetColor( LETTER_COL_SELECT, SHADOW_COL, BACK_COL );
          }
          else
          {
            GFL_FONTSYS_SetColor( LETTER_COL_NORMAL, SHADOW_COL, BACK_COL );
          }
        }
        if( index == VALUE_PERSONAL_RND )
        { 
          num = pvw->value[ MODE_NORMAL ][ index ][ pos ] & ( 0x0000000f << ( keta * 4 ) );
        }
        else
        { 
          num = pvw->value[ pvw->mcss_mode ][ index ][ pos ] & ( 0x0000000f << ( keta * 4 ) );
        }
        num = num >> ( keta * 4 );
        strbuf = GFL_MSG_CreateString( pvw->msg,  EVMSG_NUM0 + num );
        PRINTSYS_Print( GFL_BMPWIN_GetBmp( pvw->bmpwin2 ),
                        VALUE_INFO_X + ofsx, VALUE_INFO_Y + VALUE_INFO_Y_OFS * ofsy, strbuf, pvw->font );
        GFL_STR_DeleteBuffer( strbuf );
        ofsx += 8;
      }
    }
  }

  GFL_BMPWIN_TransVramCharacter( pvw->bmpwin2 );
  GFL_BMPWIN_MakeScreen( pvw->bmpwin2 );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME2_S );
}

//======================================================================
//  POKEMON_PARAMセット
//======================================================================
static  void  PokemonViewerPP_Put( POKEMON_VIEWER_WORK *pvw, int pos )
{
  PP_Put( pvw->pp, ID_PARA_monsno, pvw->mons_no[ pos ] );
  PP_Put( pvw->pp, ID_PARA_form_no, pvw->form_no[ pos ] );
}

//======================================================================
//  カメラの制御
//======================================================================
static  void  MoveCamera( POKEMON_VIEWER_WORK *pvw )
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

static  void  PokemonViewerAddPokemon( POKEMON_VIEWER_WORK *pvw )
{
  MCSS_ADD_DEBUG_WORK madw;

  if( pvw->add_pokemon_req ){
    pvw->add_pokemon_req = 0;
    madw.ncbr = pvw->resource_data[ RESOURCE_NCBR ][ pvw->read_position ];
    madw.nclr = pvw->resource_data[ RESOURCE_NCLR ][ pvw->read_position ];
    madw.ncer = pvw->resource_data[ RESOURCE_NCER ][ pvw->read_position ];
    madw.nmcr = pvw->resource_data[ RESOURCE_NMCR ][ pvw->read_position ];
    madw.nanr = pvw->resource_data[ RESOURCE_NANR ][ pvw->read_position ];
    madw.nmar = pvw->resource_data[ RESOURCE_NMAR ][ pvw->read_position ];
    madw.ncec = pvw->resource_data[ RESOURCE_NCEC ][ pvw->read_position ];

    BTLV_EFFECT_SetPokemonDebug( &madw, pvw->read_position );
  }
}

static  void  PokemonViewerCameraWork( POKEMON_VIEWER_WORK *pvw )
{
  int mcss_pos;
  VecFx32 pos,target;
  VecFx32 scale_value[] = {
    { FX32_ONE * 16, FX32_ONE * 16, 0 },
    { FX32_ONE * 16 / 2, FX32_ONE * 16 / 2, 0 },
  };

  BTLV_CAMERA_GetDefaultCameraPosition( &pos, &target );
  target.x += FX32_ONE * 4;
  target.y += FX32_ONE * 1;
  target.z += FX32_ONE * 16;
  pos.x += FX32_ONE * 4;
  pos.y += FX32_ONE * 1;
  pos.z += FX32_ONE * 16;
  BTLV_CAMERA_MoveCameraInterpolation( BTLV_EFFECT_GetCameraWork(), &pos, &target, 32, 0, 32 );

  for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ ){
    if( BTLV_EFFECT_CheckExist( mcss_pos ) == TRUE ){
      BTLV_MCSS_MoveScale( BTLV_EFFECT_GetMcssWork(),
                 mcss_pos,
                 EFFTOOL_CALCTYPE_INTERPOLATION,
                 &scale_value[ mcss_pos & 1 ],
                 16, 1, 0 );
    }
  }
}

static  void  set_pokemon( POKEMON_VIEWER_WORK *pvw, BtlvMcssPos pos )
{
  int mons_no = PP_Get( pvw->pp, ID_PARA_monsno, NULL );

  if( mons_no != 0 )
  {
    BTLV_EFFECT_SetPokemon( pvw->pp, pos );
    BTLV_MCSS_SetPosition( BTLV_EFFECT_GetMcssWork(), pos,
                           pvw->value[ pvw->mcss_mode ][ VALUE_X ][ pos ],
                           pvw->value[ pvw->mcss_mode ][ VALUE_Y ][ pos ],
                           pvw->value[ pvw->mcss_mode ][ VALUE_Z ][ pos ] );
  }
}

static  void  get_default_value( POKEMON_VIEWER_WORK *pvw )
{
  int     mode;
  int     pos;
  int     pos_max;
  VecFx32 vpos;
  int     mode_flag_3vs3[ MODE_MAX ] = { 0, 1, 0 };
  int     mode_flag_rotate[ MODE_MAX ] = { 0, 0, 1 };

  for ( mode = 0 ; mode < MODE_MAX ; mode++ )
  {
    BTLV_MCSS_SetMcss3vs3( BTLV_EFFECT_GetMcssWork(), mode_flag_3vs3[ mode ] );
    BTLV_MCSS_SetMcssRotate( BTLV_EFFECT_GetMcssWork(), mode_flag_rotate[ mode ] );
    pos_max = ( mode == MODE_NORMAL ) ? ( BTLV_MCSS_POS_D + 1 ) : BTLV_MCSS_POS_MAX;
    for ( pos = BTLV_MCSS_POS_AA ; pos < pos_max ; pos++ )
    {
      BTLV_MCSS_GetPokeDefaultPos( BTLV_EFFECT_GetMcssWork(), &vpos, pos );
      pvw->value[ mode ][ VALUE_X ][ pos ] = vpos.x;
      pvw->value[ mode ][ VALUE_Y ][ pos ] = vpos.y;
      pvw->value[ mode ][ VALUE_Z ][ pos ] = vpos.z;
      pvw->value[ mode ][ VALUE_SCALE_PERS ][ pos ] = BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_EFFECT_GetMcssWork(), pos,
                                                                                       BTLV_MCSS_PROJ_PERSPECTIVE );
      pvw->value[ mode ][ VALUE_SCALE_ORTH ][ pos ] = BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_EFFECT_GetMcssWork(), pos,
                                                                                       BTLV_MCSS_PROJ_ORTHO );
    }
  }
  BTLV_MCSS_SetMcss3vs3( BTLV_EFFECT_GetMcssWork(), mode_flag_3vs3[ pvw->mcss_mode ] );
  BTLV_MCSS_SetMcssRotate( BTLV_EFFECT_GetMcssWork(), mode_flag_rotate[ pvw->mcss_mode ] );
}

static  void  mcss_mode_change( POKEMON_VIEWER_WORK *pvw )
{
  int mcss_pos;
  int mode_flag_3vs3[ MODE_MAX ] = { 0, 1, 0 };
  int mode_flag_rotate[ MODE_MAX ] = { 0, 0, 1 };

  pvw->mcss_mode++;
  if( pvw->mcss_mode == MODE_MAX )
  { 
    pvw->mcss_mode = MODE_NORMAL;
  }

  BTLV_MCSS_SetMcss3vs3( BTLV_EFFECT_GetMcssWork(), mode_flag_3vs3[ pvw->mcss_mode ] );
  BTLV_MCSS_SetMcssRotate( BTLV_EFFECT_GetMcssWork(), mode_flag_rotate[ pvw->mcss_mode ] );

  for( mcss_pos = BTLV_MCSS_POS_AA ; mcss_pos < BTLV_MCSS_POS_MAX ; mcss_pos++ )
  {
    TextPrint( pvw, mcss_pos, mcss_pos );
    if( BTLV_EFFECT_CheckExist( mcss_pos ) == TRUE )
    {
      VecFx32 scale;

      BTLV_MCSS_SetPosition( BTLV_EFFECT_GetMcssWork(), mcss_pos,
                             pvw->value[ pvw->mcss_mode ][ VALUE_X ][ mcss_pos ],
                             pvw->value[ pvw->mcss_mode ][ VALUE_Y ][ mcss_pos ],
                             pvw->value[ pvw->mcss_mode ][ VALUE_Z ][ mcss_pos ] );
      VEC_Set( &scale,
               pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ mcss_pos ],
               pvw->value[ pvw->mcss_mode ][ VALUE_SCALE_PERS + pvw->proj ][ mcss_pos ],
               FX32_ONE );
      BTLV_MCSS_SetScale( BTLV_EFFECT_GetMcssWork(), mcss_pos, &scale );
    }
  }

  if( pvw->mcss_mode == 0 )
  {
    if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_E ) == TRUE )
    {
      BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_E );
    }
    if( BTLV_EFFECT_CheckExist( BTLV_MCSS_POS_F ) == TRUE )
    {
      BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_F );
    }
  }
}

#if 0
static  void  del_pokemon( POKEMON_VIEWER_WORK *pvw )
{
  BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_AA );
  BTLV_MCSS_Del( BTLV_EFFECT_GetMcssWork(), BTLV_MCSS_POS_BB );
}
#endif
