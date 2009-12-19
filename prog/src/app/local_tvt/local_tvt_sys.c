//======================================================================
/**
 * @file	local_tvt_sys.c
 * @brief	非通信テレビトランシーバー メイン
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/bmp_winframe.h"
#include "gamesystem/msgspeed.h"

#include "arc_def.h"
#include "local_tvt.naix"
#include "message.naix"
#include "font/font.naix"

#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define LTVT_MSG_MSGWIN_CGX (1)


const u8 LOCAL_TVT_CHARA_BASE_POS[LOCAL_TVT_MEMBER_MAX][2] = {{0,0},{16,0},{0,12},{16,12}};

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  LTMP_UP,
  LTMP_DOWN,
}LOCAL_TVT_MSG_POS;

typedef enum
{
  LTS_FADEIN,
  LTS_FADEIN_WAIT,
  LTS_FADEOUT,
  LTS_FADEOUT_WAIT,

  LTS_MAIN,
}LOCAL_TVT_STATE;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static const GFL_DISP_VRAM vramBank = {
  GX_VRAM_BG_256_AB,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_23_G,     // メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_16_F ,       // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_NONE,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,        // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_32K,
  GX_OBJVRAMMODE_CHAR_1D_32K
};

static void LOCAL_TVT_Init( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_Term( LOCAL_TVT_WORK *work );
static const BOOL LOCAL_TVT_Main( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_VBlankFunc(GFL_TCB *tcb, void *wk );

static void LOCAL_TVT_InitGraphic( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_TermGraphic( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode );
static void LOCAL_TVT_LoadResource( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_DispRecIcon( LOCAL_TVT_WORK *work , const u8 idx );
static void LOCAL_TVT_HideRecIcon( LOCAL_TVT_WORK *work );

static void LOCAL_TVT_MSG_InitMessage( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_MSG_TermMessage( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_MSG_UpdateMessage( LOCAL_TVT_WORK *work );
static const BOOL LOCAL_TVT_MSG_IsFinishMsg( LOCAL_TVT_WORK *work );
static void LOCAL_TVT_MSG_OpenWindow( LOCAL_TVT_WORK *work , LOCAL_TVT_MSG_POS pos );
static void LOCAL_TVT_MSG_DispMessage( LOCAL_TVT_WORK *work , const u16 msgId );

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static void LOCAL_TVT_Init( LOCAL_TVT_WORK *work )
{
  u8 i;
  LOCAL_TVT_InitGraphic( work );
  
  work->archandle = GFL_ARC_OpenDataHandle( ARCID_LOCAL_TVT , work->heapId );
  LOCAL_TVT_LoadResource( work );
  LOCAL_TVT_MSG_InitMessage( work );

  for( i=0 ; i<work->mode ; i++ )
  {
    work->charaWork[i] = LOCAL_TVT_CHARA_Init( work , i );
  }

  work->vBlankTcb = GFUser_VIntr_CreateTCB( LOCAL_TVT_VBlankFunc , work , 8 );
  

  work->transCnt = 0;
  work->bufNo = 1;
  work->transState = LTTS_CHARA1;
  work->transChara = NULL;
  
  work->state = LTS_FADEIN;

  //---仮処理---
  work->scriptIdx = 0;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static void LOCAL_TVT_Term( LOCAL_TVT_WORK *work )
{
  u8 i;

  GFL_TCB_DeleteTask( work->vBlankTcb );

  LOCAL_TVT_MSG_TermMessage( work );

  GFL_CLACT_WK_Remove( work->clwkRecIcon );
  GFL_CLGRP_PLTT_Release( work->cellRes[LTCR_PLT] );
  GFL_CLGRP_CGR_Release( work->cellRes[LTCR_NCG] );
  GFL_CLGRP_CELLANIM_Release( work->cellRes[LTCR_ANM] );
  GFL_CLACT_UNIT_Delete( work->cellUnit );


  for( i=0 ; i<work->mode ; i++ )
  {
    LOCAL_TVT_CHARA_Term( work , work->charaWork[i] );
  }
  
  GFL_ARC_CloseDataHandle( work->archandle );
  
  LOCAL_TVT_TermGraphic( work );
}

//--------------------------------------------------------------
//  更新
//--------------------------------------------------------------
static const BOOL LOCAL_TVT_Main( LOCAL_TVT_WORK *work )
{
  u8 i;
  
  switch( work->state )
  {
  case LTS_FADEIN:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = LTS_FADEIN_WAIT;
    break;
  case LTS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      work->state = LTS_MAIN;
    }
    break;
  case LTS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , work->heapId );
    work->state = LTS_FADEOUT_WAIT;
    break;
  case LTS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return TRUE;
    }
    break;

  case LTS_MAIN:
    //---仮処理---
    {
      if( LOCAL_TVT_MSG_IsFinishMsg(work) == TRUE )
      {
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ||
            work->scriptIdx == 0 )
        {
          if( work->scriptIdx < 5 )
          {
            LOCAL_TVT_DispRecIcon( work , work->scriptIdx%2 );
            LOCAL_TVT_MSG_OpenWindow( work , LTMP_DOWN );
            LOCAL_TVT_MSG_DispMessage( work , work->scriptIdx );
          }
          else
          {
            LOCAL_TVT_HideRecIcon( work );
            work->state = LTS_FADEOUT;
          }
          work->scriptIdx++;
        }
      }
    }
    //---仮処理---
    break;
  }
  
  
  //フリップタイミング
  if( work->transCnt < LTVT_TRANS_COUNT_MAX )
  {
    work->transCnt++;
  }
  //else
  {
    if( work->transChara == NULL &&
        work->transState == LTTS_TRANS_WAIT )
    {
      GXBGCharBase charaBaseAdr;
      GXBGCharBase bgBaseAdr;
      if( work->bufNo == 0 )
      {
        charaBaseAdr = GX_BG_CHARBASE_0x10000;
        bgBaseAdr = GX_BG_CHARBASE_0x28000;
      }
      else
      {
        charaBaseAdr = GX_BG_CHARBASE_0x1c000;
        bgBaseAdr = GX_BG_CHARBASE_0x34000;
      }
      G2_SetBG2Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x6800 ,
                                    charaBaseAdr );
      G2_SetBG3Control256x16Affine( GX_BG_SCRSIZE_256x16PLTT_256x256 , 
                                    GX_BG_AREAOVER_REPEAT ,
                                    GX_BG_SCRBASE_0x7000 ,
                                    bgBaseAdr );
      work->bufNo = !work->bufNo;
      work->transState = LTTS_CHARA1;
      work->transCnt = 0;
    }
  }
  
  //絵の転送
  if( work->transChara != NULL )
  {
    if( LOCAL_TVT_CHARA_isFinishTrans( work , work->transChara ) == TRUE )
    {
      work->transChara = NULL;
    }
  }
  
  if( work->transChara == NULL )
  {
    switch( work->transState )
    {
    case LTTS_CHARA1:
    case LTTS_CHARA2:
    case LTTS_CHARA3:
    case LTTS_CHARA4:
      {
        const u8 charaNo = work->transState - LTTS_CHARA1;
        LOCAL_TVT_CHARA_LoadChara( work , work->charaWork[charaNo] , work->bufNo );
        work->transChara = work->charaWork[charaNo];
        if( work->mode == LTM_2_MEMBER &&
            work->transState == LTTS_CHARA2 )
        {
          work->transState = LTTS_TRANS_WAIT;
        }
        else
        {
          work->transState++;
        }
      }
      break;
    }
  }

  for( i=0 ; i<work->mode ; i++ )
  {
    LOCAL_TVT_CHARA_Main( work , work->charaWork[i] );
  }
  
  LOCAL_TVT_MSG_UpdateMessage( work );
  
  //OBJの更新
  GFL_CLACT_SYS_Main();

  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START &&
      GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
//	VBlankTcb
//--------------------------------------------------------------
static void LOCAL_TVT_VBlankFunc(GFL_TCB *tcb, void *wk )
{
  //OBJの更新
  GFL_CLACT_SYS_VBlankFunc();
}
//--------------------------------------------------------------
//  グラフィック系初期化
//--------------------------------------------------------------
static void LOCAL_TVT_InitGraphic( LOCAL_TVT_WORK *work )
{
  GFL_DISP_GX_InitVisibleControl();
  GFL_DISP_GXS_InitVisibleControl();
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
  WIPE_SetBrightness(WIPE_DISP_MAIN,WIPE_FADE_BLACK);
  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
  WIPE_ResetWndMask(WIPE_DISP_MAIN);
  WIPE_ResetWndMask(WIPE_DISP_SUB);
  
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);
  GFL_DISP_SetBank( &vramBank );

  //BG系の初期化
  GFL_BG_Init( work->heapId );
  GFL_BMPWIN_Init( work->heapId );

  //Vram割り当ての設定
  {
    static const GFL_BG_SYS_HEADER sys_data = {
        GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    // BG0 MAIN (メッセージ
    static const GFL_BG_BGCNT_HEADER header_main0 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000,0x6000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG1 MAIN (名前
    static const GFL_BG_BGCNT_HEADER header_main1 = {
      0, 0, 0x800, 0, // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x08000,0x8000,
      GX_BG_EXTPLTT_01, 1, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG2 MAIN (キャラ
    static const GFL_BG_BGCNT_HEADER header_main2 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x10000,0x0C000,
      GX_BG_EXTPLTT_23, 2, 1, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    // BG3 MAIN (背景
    static const GFL_BG_BGCNT_HEADER header_main3 = {
      0, 0, 0x800, 0,  // scrX, scrY, scrbufSize, scrbufofs,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x28000,0x0C000,
      GX_BG_EXTPLTT_23, 3, 0, 0, FALSE  // pal, pri, areaover, dmy, mosaic
    };
    GFL_BG_SetBGMode( &sys_data );

    LOCAL_TVT_SetupBgFunc( &header_main0 , LTVT_FRAME_MESSAGE , GFL_BG_MODE_TEXT );
    LOCAL_TVT_SetupBgFunc( &header_main1 , LTVT_FRAME_NAME    , GFL_BG_MODE_TEXT );
    LOCAL_TVT_SetupBgFunc( &header_main2 , LTVT_FRAME_CHARA   , GFL_BG_MODE_256X16 );
    LOCAL_TVT_SetupBgFunc( &header_main3 , LTVT_FRAME_BG      , GFL_BG_MODE_256X16 );
    
  }
  //OBJ系の初期化
  {
    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
    //GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );

    work->cellUnit  = GFL_CLACT_UNIT_Create( 4 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );
  }
}

static void LOCAL_TVT_TermGraphic( LOCAL_TVT_WORK *work )
{
  GFL_CLACT_SYS_Delete();
  GFL_BG_FreeBGControl( LTVT_FRAME_BG );
  GFL_BG_FreeBGControl( LTVT_FRAME_CHARA );
  GFL_BG_FreeBGControl( LTVT_FRAME_NAME );
  GFL_BG_FreeBGControl( LTVT_FRAME_MESSAGE );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}


//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void LOCAL_TVT_SetupBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane , u8 mode )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, mode );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}


//--------------------------------------------------------------
//  リソース読み込み
//--------------------------------------------------------------
static void LOCAL_TVT_LoadResource( LOCAL_TVT_WORK *work )
{
  u32 datId;
  if( work->mode == LTM_2_MEMBER )
  {
    datId = NARC_local_tvt_local_tvt_scr_2_NSCR;
  }
  else
  {
    datId = NARC_local_tvt_local_tvt_scr_4_NSCR;
  }
  
  GFL_ARCHDL_UTIL_TransVramScreen( work->archandle , 
                                   datId ,
                                   LTVT_FRAME_CHARA ,
                                   0,0,FALSE,work->heapId );
  
  GFL_ARCHDL_UTIL_TransVramScreen( work->archandle , 
                                   datId ,
                                   LTVT_FRAME_BG ,
                                   0,0,FALSE,work->heapId );
  //セル
  {
    work->cellRes[LTCR_PLT] = GFL_CLGRP_PLTT_RegisterEx( work->archandle , 
          NARC_local_tvt_local_tvt_obj_NCLR , CLSYS_DRAW_MAIN , 
          LTVT_PLT_OBJ_MAIN*32 , 0 , 1 , work->heapId  );
    work->cellRes[LTCR_NCG] = GFL_CLGRP_CGR_Register( work->archandle , 
          NARC_local_tvt_local_tvt_obj_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    work->cellRes[LTCR_ANM] = GFL_CLGRP_CELLANIM_Register( work->archandle , 
          NARC_local_tvt_local_tvt_obj_NCER , NARC_local_tvt_local_tvt_obj_NANR, work->heapId  );
  }
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 1;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = 0;

    work->clwkRecIcon = GFL_CLACT_WK_Create( work->cellUnit ,
              work->cellRes[LTCR_NCG],
              work->cellRes[LTCR_PLT],
              work->cellRes[LTCR_ANM],
              &cellInitData ,CLSYS_DRAW_MAIN , work->heapId );
    GFL_CLACT_WK_SetDrawEnable( work->clwkRecIcon , FALSE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->clwkRecIcon , TRUE );
  }
}

static void LOCAL_TVT_DispRecIcon( LOCAL_TVT_WORK *work , const u8 idx )
{
  const u8 nameLen = LOCAL_TVT_CHARA_GetNameLen( work->charaWork[idx] );
  GFL_CLACTPOS pos;
  pos.x = LOCAL_TVT_CHARA_BASE_POS[idx][0]*8 + 64 + nameLen/2 + 16;
  pos.y = LOCAL_TVT_CHARA_BASE_POS[idx][1]*8 + 16;
  GFL_CLACT_WK_SetPos( work->clwkRecIcon , &pos , CLSYS_DRAW_MAIN );
  GFL_CLACT_WK_SetDrawEnable( work->clwkRecIcon , TRUE );
}
static void LOCAL_TVT_HideRecIcon( LOCAL_TVT_WORK *work )
{
  GFL_CLACT_WK_SetDrawEnable( work->clwkRecIcon , FALSE );
}


#pragma mark [>message func
static void LOCAL_TVT_MSG_InitMessage( LOCAL_TVT_WORK *work )
{
  //フォント読み込み
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
  
  //メッセージ
  work->msgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tvt_event_system_dat , work->heapId );
  work->talkMsgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_tvt_event_01_dat , work->heapId );
  
  BmpWinFrame_GraphicSet( LTVT_FRAME_MESSAGE , LTVT_MSG_MSGWIN_CGX , LTVT_PLT_MAIN_WINFRAME , 0 , work->heapId );

  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , LTVT_PLT_MAIN_FONT*0x20, 16*2, work->heapId );
  GFL_FONTSYS_SetDefaultColor();
  
  work->tcblSys = GFL_TCBL_Init( work->heapId , work->heapId , 3 , 0x100 );
  work->printHandle = NULL;
  work->msgWin = NULL;
  work->msgStr = NULL;
  work->wordSet = NULL;
  work->printQue = PRINTSYS_QUE_Create( work->heapId );

  GFL_FONTSYS_SetColor( 1,2,0xf );
  
}

static void LOCAL_TVT_MSG_TermMessage( LOCAL_TVT_WORK *work )
{
  PRINTSYS_QUE_Delete( work->printQue );
  if( work->printHandle != NULL )
  {
    PRINTSYS_PrintStreamDelete( work->printHandle );
  }
  if( work->msgStr != NULL )
  {
    GFL_STR_DeleteBuffer( work->msgStr );
  }
  if( work->wordSet != NULL )
  {
    WORDSET_Delete( work->wordSet );
  }
  GFL_MSG_Delete( work->talkMsgHandle );
  GFL_MSG_Delete( work->msgHandle );
  if( work->msgWin != NULL )
  {
    GFL_BMPWIN_Delete( work->msgWin );
  }
  GFL_FONT_Delete( work->fontHandle );
  GFL_TCBL_Exit( work->tcblSys );

  GFL_FONTSYS_SetDefaultColor();
}

static void LOCAL_TVT_MSG_UpdateMessage( LOCAL_TVT_WORK *work )
{
  GFL_TCBL_Main( work->tcblSys );
  if( work->printHandle != NULL  )
  {
    if( PRINTSYS_PrintStreamGetState( work->printHandle ) == PRINTSTREAM_STATE_DONE )
    {
      PRINTSYS_PrintStreamDelete( work->printHandle );
      work->printHandle = NULL;
      GFL_STR_DeleteBuffer( work->msgStr );
      work->msgStr = NULL;
    }
    else
    if( PRINTSYS_PrintStreamGetState( work->printHandle ) == PRINTSTREAM_STATE_PAUSE )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ||
          GFL_UI_TP_GetTrg() == TRUE )
      {
        PRINTSYS_PrintStreamReleasePause( work->printHandle );
      }
    }
  }
  PRINTSYS_QUE_Main( work->printQue );
}

static const BOOL LOCAL_TVT_MSG_IsFinishMsg( LOCAL_TVT_WORK *work )
{
  if( work->printHandle == NULL )
  {
    return TRUE;
  }
  return FALSE;
}

static void LOCAL_TVT_MSG_OpenWindow( LOCAL_TVT_WORK *work , LOCAL_TVT_MSG_POS pos )
{
  if( work->msgWin != NULL )
  {
    GFL_BMPWIN_Delete( work->msgWin );
    work->msgWin = NULL;
  }
  
  if( pos == LTMP_UP )
  {
    work->msgWin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
                                      1 , 1 , 30 , 4 , 
                                      LTVT_PLT_MAIN_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
  }
  else
  if( pos == LTMP_DOWN )
  {
    work->msgWin = GFL_BMPWIN_Create( LTVT_FRAME_MESSAGE , 
                                      1 , 19 , 30 , 4 , 
                                      LTVT_PLT_MAIN_FONT ,
                                      GFL_BMP_CHRAREA_GET_B );
  }
  
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0x0F );
  GFL_BMPWIN_TransVramCharacter( work->msgWin );
  GFL_BMPWIN_MakeScreen( work->msgWin );
  GFL_BG_LoadScreenReq( LTVT_FRAME_MESSAGE );
}

//--------------------------------------------------------------------------
//  メッセージ表示
//--------------------------------------------------------------------------
static void LOCAL_TVT_MSG_DispMessage( LOCAL_TVT_WORK *work , const u16 msgId )
{
  if( work->printHandle != NULL )
  {
    OS_TPrintf( "Message is not finish!!\n" );
    PRINTSYS_PrintStreamDelete( work->printHandle );
    work->printHandle = NULL;
  }

  {
    if( work->msgStr != NULL )
    {
      GFL_STR_DeleteBuffer( work->msgStr );
      work->msgStr = NULL;
    }
    
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->msgWin ) , 0xf );
    work->msgStr = GFL_MSG_CreateString( work->talkMsgHandle , msgId );
    
    if( work->wordSet != NULL )
    {
      STRBUF *msgWorkStr = GFL_STR_CreateBuffer( 128 , work->heapId );
      WORDSET_ExpandStr( work->wordSet , msgWorkStr , work->msgStr );
      GFL_STR_DeleteBuffer( work->msgStr );
      work->msgStr = msgWorkStr;
    }
    
    work->printHandle = PRINTSYS_PrintStream( work->msgWin , 0,0, work->msgStr ,work->fontHandle ,
                        MSGSPEED_GetWait() , work->tcblSys , 2 , work->heapId , 0x0f );
  }
  BmpWinFrame_Write( work->msgWin , WINDOW_TRANS_ON_V , LTVT_MSG_MSGWIN_CGX , LTVT_PLT_MAIN_WINFRAME );
}



#pragma mark [>proc func
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

GFL_PROC_DATA LocalTvt_ProcData =
{
  LOCAL_TVT_ProcInit,
  LOCAL_TVT_ProcMain,
  LOCAL_TVT_ProcTerm
};

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcInit( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_INIT_WORK *initWork;
  LOCAL_TVT_WORK *work;
  
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_LOCAL_TVT, 0x40000 );
  work = GFL_PROC_AllocWork( proc, sizeof(LOCAL_TVT_WORK), HEAPID_LOCAL_TVT );
  if( pwk == NULL )
  {
    initWork = GFL_HEAP_AllocMemory( HEAPID_LOCAL_TVT , sizeof( LOCAL_TVT_INIT_WORK ));
    initWork->gameData = GAMEDATA_Create( GFL_HEAPID_APP );
  }
  else
  {
    initWork = pwk;
  }

  work->charaType[0] = LTCT_DOCTOR_D;
  work->charaType[1] = LTCT_SUPPORT;
  work->charaType[2] = LTCT_RIVAL;
  work->charaType[3] = LTCT_PLAYER_M;
  work->bgType[0] = 1;
  work->bgType[1] = 0;
  work->bgType[2] = 0;
  work->bgType[3] = 0;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    work->mode = LTM_2_MEMBER;
  }
  else
  {
    work->mode = LTM_4_MEMBER;
  }

  work->heapId = HEAPID_LOCAL_TVT;
  work->initWork = initWork;
  
  LOCAL_TVT_Init( work );
  
  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  開放
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcTerm( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  
  LOCAL_TVT_Term( work );

  if( pwk == NULL )
  {
    GAMEDATA_Delete( work->initWork->gameData );
    GFL_HEAP_FreeMemory( work->initWork );
  }
  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_LOCAL_TVT );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//  ループ
//--------------------------------------------------------------
static GFL_PROC_RESULT LOCAL_TVT_ProcMain( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
  LOCAL_TVT_WORK *work = mywk;
  const BOOL ret = LOCAL_TVT_Main( work );
  
  if( ret == TRUE )
  {
    return GFL_PROC_RES_FINISH;
  }
  return GFL_PROC_RES_CONTINUE;
}
