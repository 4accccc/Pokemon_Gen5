//======================================================================
/**
 * @file  select_moji_mode.c
 * @brief ゲーム開始時の漢字・ひらがなの選択
 * @author  ariizumi
 * @data  09/05/11
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"

#include "arc_def.h"
#include "print/printsys.h"
#include "poke_tool/poke_tool.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_select_moji_mode.h"
#include "sound/pm_sndsys.h"

#include "select_moji_mode.h"
#include "test/ariizumi/ari_debug.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define

//項目数
#define SEL_MODE_ITEM_NUM (2)

//BGPlane
#define SEL_MODE_BG_BACK (GFL_BG_FRAME3_M)
#define SEL_MODE_BG_ITEM (GFL_BG_FRAME1_M)
#define SEL_MODE_BG_STR  (GFL_BG_FRAME1_S)

//BGキャラアドレス
#define SEL_MODE_FRAMECHR1  (1)
#define SEL_MODE_FRAMECHR2  (SEL_MODE_FRAMECHR1 + TALK_WIN_CGX_SIZ)
#define SEL_MODE_FRAMECHR_TALK  (1)

//パレット
#define SEL_MODE_PLT_FONT   (0)
#define SEL_MODE_PLT_SEL    (3)
#define SEL_MODE_PLT_NOSEL  (4)
#define SEL_MODE_PLT_S_FONT    (0)
#define SEL_MODE_PLT_S_TALKWIN (1)

//フォントオフセット
#define SEL_MODE_FONT_TOP  (2)
#define SEL_MODE_FONT_LEFT (2)

//選択肢位置・サイズ
#define SEL_MODE_ITEM_TOP     (12)
#define SEL_MODE_ITEM_LEFT    (6)
#define SEL_MODE_ITEM_HEIGHT  (2)
#define SEL_MODE_ITEM_WIDTH   (20)

//会話ウィンドウ
#define SEL_MODE_TALK_TOP     (19)
#define SEL_MODE_TALK_LEFT    (1)
#define SEL_MODE_TALK_HEIGHT  (4)
#define SEL_MODE_TALK_WIDTH   (29)

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  SMS_KANJI,
  SMS_SEX,
  SMS_COMM,
  SMS_WAIT_FADEOUT,
}SELECT_MODE_STATE;

typedef enum
{
  SMUR_CONTINUE,
  SMUR_TRUE,
  SMUR_FALSE,
}SELECT_MODE_UI_RETURN;

//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  u8 selectItem;
  SELECT_MODE_STATE state;
  GFL_FONT   *fontHandle;
  GFL_BMPWIN *strWin;
  GFL_BMPWIN *itemWin[SEL_MODE_ITEM_NUM];
  
  u16 anmCnt;
  u16 transBuf;
}SEL_MODE_WORK;


//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

//Procデータ
static GFL_PROC_RESULT SEL_MODE_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT SEL_MODE_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT SEL_MODE_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

static void SEL_MODE_InitGraphic( SEL_MODE_WORK *work );
static void SEL_MODE_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );
static void SEL_MODE_InitItem( SEL_MODE_WORK *work );
static void SEL_MODE_ExitItem( SEL_MODE_WORK *work );

static const SELECT_MODE_UI_RETURN SEL_MODE_UpdateUI( SEL_MODE_WORK *work );

static void SEL_MODE_DrawWinFrame( SEL_MODE_WORK *work );
static void SEL_MODE_UpdatePalletAnime( SEL_MODE_WORK *work );

const GFL_PROC_DATA SelectModeProcData = {
  SEL_MODE_ProcInit,
  SEL_MODE_ProcMain,
  SEL_MODE_ProcEnd,
};

//--------------------------------------------------------------
//  
//--------------------------------------------------------------

static GFL_PROC_RESULT SEL_MODE_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SELECT_MODE_INIT_WORK *initWork = pwk;
  SEL_MODE_WORK *work;
    //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_SEL_MODE, 0x80000 );
  work = GFL_PROC_AllocWork( proc, sizeof(SEL_MODE_WORK), HEAPID_SEL_MODE );

  if( initWork->type == SMT_START_GAME )
  {
    work->state = SMS_KANJI;
  }
  else
  {
    work->state = SMS_COMM;
  }
  work->anmCnt = 0;

  SEL_MODE_InitGraphic( work );
  SEL_MODE_InitItem( work );
  
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT SEL_MODE_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  u8 i;
  SEL_MODE_WORK *work = (SEL_MODE_WORK*)mywk;
  SELECT_MODE_INIT_WORK *initWork = pwk;

  SEL_MODE_ExitItem( work );
  GFL_FONT_Delete( work->fontHandle );
  GFL_BMPWIN_Exit();
  
  GFL_BG_FreeBGControl(SEL_MODE_BG_BACK);
  GFL_BG_FreeBGControl(SEL_MODE_BG_ITEM);
  GFL_BG_FreeBGControl(SEL_MODE_BG_STR);
  GFL_BG_Exit();

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap( HEAPID_SEL_MODE );
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT SEL_MODE_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  SEL_MODE_WORK *work = (SEL_MODE_WORK*)mywk;
  SELECT_MODE_INIT_WORK *initWork = pwk;
  
  switch( work->state )
  {
  case SMS_KANJI:
    {
      const SELECT_MODE_UI_RETURN ret = SEL_MODE_UpdateUI( work );
      if( ret != SMUR_CONTINUE )
      {
        if( ret == SMUR_TRUE )
        {
          CONFIG_SetMojiMode(initWork->configSave,MOJIMODE_HIRAGANA );
        }
        else
        {
          CONFIG_SetMojiMode(initWork->configSave,MOJIMODE_KANJI );
        }
        SEL_MODE_ExitItem( work );
        work->state = SMS_SEX;
        SEL_MODE_InitItem( work );
      }
    }
    break;
  case SMS_SEX:
    {
      const SELECT_MODE_UI_RETURN ret = SEL_MODE_UpdateUI( work );
      if( ret != SMUR_CONTINUE )
      {
        if( ret == SMUR_TRUE )
        {
          MyStatus_SetMySex( initWork->mystatus , PTL_SEX_MALE );
        }
        else
        {
          MyStatus_SetMySex( initWork->mystatus , PTL_SEX_FEMALE );
        }
        SEL_MODE_ExitItem( work );
        work->state = SMS_COMM;
        SEL_MODE_InitItem( work );
      }
    }
    break;
  case SMS_COMM:
    {
      const SELECT_MODE_UI_RETURN ret = SEL_MODE_UpdateUI( work );
      if( ret != SMUR_CONTINUE )
      {
        if( ret == SMUR_TRUE )
        {
					CONFIG_SetNetworkSearchMode(initWork->configSave, NETWORK_SEARCH_ON );
        }
        else
        {
					CONFIG_SetNetworkSearchMode(initWork->configSave, NETWORK_SEARCH_OFF );
        }
        WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                        WIPE_FADE_BLACK , 12 , WIPE_DEF_SYNC , HEAPID_SEL_MODE );
        
        work->state = SMS_WAIT_FADEOUT;
      }
    }
    break;
  case SMS_WAIT_FADEOUT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return GFL_PROC_RES_FINISH;
    }

    break;
  }
  
  SEL_MODE_UpdatePalletAnime( work );
  
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------
//  描画系初期化
//--------------------------------------------------------------------------
static void SEL_MODE_InitGraphic( SEL_MODE_WORK *work )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_128_D,      // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K,   // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K,   // サブOBJマッピングモード
  };  

  static const GFL_BG_SYS_HEADER sysHeader = {
    GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
  };
  
  static const GFL_BG_BGCNT_HEADER bgCont_Item = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };

  static const GFL_BG_BGCNT_HEADER bgCont_BackGround = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
  };

  static const GFL_BG_BGCNT_HEADER bgCont_Str = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };


  GX_SetMasterBrightness(-16);  
  GXS_SetMasterBrightness(-16);
  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetBank( &vramBank );
  GFL_BG_Init( HEAPID_SEL_MODE );
  GFL_BG_SetBGMode( &sysHeader ); 
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

  SEL_MODE_InitBgFunc( &bgCont_BackGround , SEL_MODE_BG_BACK );
  SEL_MODE_InitBgFunc( &bgCont_Item , SEL_MODE_BG_ITEM );
  SEL_MODE_InitBgFunc( &bgCont_Str , GFL_BG_FRAME1_S );

  GFL_BMPWIN_Init( HEAPID_SEL_MODE );
  
  //フォント用パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , SEL_MODE_PLT_FONT * 32, 16*2, HEAPID_SEL_MODE );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , SEL_MODE_PLT_S_FONT * 32, 16*2, HEAPID_SEL_MODE );
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_SEL_MODE );

  //WinFrame用グラフィック設定
  BmpWinFrame_GraphicSet( SEL_MODE_BG_ITEM , SEL_MODE_FRAMECHR1 , SEL_MODE_PLT_SEL  , 1, HEAPID_SEL_MODE);
  BmpWinFrame_GraphicSet( SEL_MODE_BG_ITEM , SEL_MODE_FRAMECHR2 , SEL_MODE_PLT_NOSEL , 0, HEAPID_SEL_MODE);
  TalkWinFrame_GraphicSet( SEL_MODE_BG_STR , SEL_MODE_FRAMECHR_TALK , SEL_MODE_PLT_S_TALKWIN , 0 , HEAPID_SEL_MODE);

  //背景色
  *((u16 *)HW_BG_PLTT) = 0x7d8c;//RGB(12, 12, 31);
  *((u16 *)(HW_BG_PLTT+0x400)) = 0x7d8c;//RGB(12, 12, 31);

  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT , 16 , 0 , ARI_FADE_SPD );
  
}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void SEL_MODE_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  選択肢を作る
//--------------------------------------------------------------------------
static void SEL_MODE_InitItem( SEL_MODE_WORK *work )
{
  u8 i;
  
  GFL_MSGDATA *msghandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE ,
                                NARC_message_select_moji_mode_dat , HEAPID_SEL_MODE );
  
  work->selectItem = 0;

  //タイトルの表示
  {
    STRBUF *str;
    work->strWin = GFL_BMPWIN_Create( SEL_MODE_BG_STR , 
                          SEL_MODE_TALK_LEFT , SEL_MODE_TALK_TOP , 
                          SEL_MODE_TALK_WIDTH, SEL_MODE_TALK_HEIGHT , 
                          SEL_MODE_PLT_S_FONT , GFL_BMP_CHRAREA_GET_B );
    GFL_BMPWIN_MakeScreen( work->strWin );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->strWin ), 0xf );

    str = GFL_MSG_CreateString( msghandle , QUESTION_STR1 + work->state );

    TalkWinFrame_Write( work->strWin , WINDOW_TRANS_ON ,
                  SEL_MODE_FRAMECHR_TALK ,SEL_MODE_PLT_S_TALKWIN );

    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->strWin),
            SEL_MODE_FONT_TOP,
            SEL_MODE_FONT_LEFT,
            str,
            work->fontHandle);

    GFL_STR_DeleteBuffer( str );

    GFL_BMPWIN_TransVramCharacter( work->strWin );
  }

  for( i=0 ; i<SEL_MODE_ITEM_NUM ; i++ )
  {
    STRBUF *str;
    work->itemWin[i] = GFL_BMPWIN_Create( SEL_MODE_BG_ITEM , 
                          SEL_MODE_ITEM_LEFT , SEL_MODE_ITEM_TOP+i*(SEL_MODE_ITEM_HEIGHT+2) , 
                          SEL_MODE_ITEM_WIDTH, SEL_MODE_ITEM_HEIGHT , 
                          SEL_MODE_PLT_FONT , GFL_BMP_CHRAREA_GET_B );
    GFL_BMPWIN_MakeScreen( work->itemWin[i] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->itemWin[i] ), 0xf );
    
    str = GFL_MSG_CreateString( msghandle , ITEM1_STR1 + i + work->state*2 );

    PRINTSYS_Print( GFL_BMPWIN_GetBmp(work->itemWin[i]),
            SEL_MODE_FONT_TOP,
            SEL_MODE_FONT_LEFT,
            str,
            work->fontHandle);
    
    GFL_STR_DeleteBuffer( str );
    
    GFL_BMPWIN_TransVramCharacter( work->itemWin[i] );
  }
  
  GFL_MSG_Delete( msghandle );
  
  SEL_MODE_DrawWinFrame( work );
}

static void SEL_MODE_ExitItem( SEL_MODE_WORK *work )
{
  u8 i;
  GFL_BMPWIN_Delete( work->strWin );
  for( i=0 ; i<SEL_MODE_ITEM_NUM ; i++ )
  {
    GFL_BMPWIN_Delete( work->itemWin[i] );
  }
}

//--------------------------------------------------------------------------
//  選択肢の枠を描画
//--------------------------------------------------------------------------
static void SEL_MODE_DrawWinFrame( SEL_MODE_WORK *work )
{
  u8 i;

  for( i=0 ; i<SEL_MODE_ITEM_NUM ; i++ )
  {
    if( i == work->selectItem )
    {
      BmpWinFrame_Write(work->itemWin[i], WINDOW_TRANS_ON, 
              SEL_MODE_FRAMECHR1, SEL_MODE_PLT_SEL);
    }
    else
    {
      BmpWinFrame_Write(work->itemWin[i], WINDOW_TRANS_ON, 
              SEL_MODE_FRAMECHR2, SEL_MODE_PLT_NOSEL);
    }
  }
}

//--------------------------------------------------------------------------
//  入力の更新
//--------------------------------------------------------------------------
static const SELECT_MODE_UI_RETURN  SEL_MODE_UpdateUI( SEL_MODE_WORK *work )
{
  //キー更新
  if( (GFL_UI_KEY_GetTrg() & PAD_KEY_UP) ||
      (GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN) )
  {
    if( work->selectItem == 0 )
    {
      work->selectItem = 1;
    }
    else
    {
      work->selectItem = 0;
    }
    SEL_MODE_DrawWinFrame( work );
    PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
  }
  
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
  {
    PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
    if( work->selectItem == 0 )
    {
      return SMUR_TRUE;
    }
    else
    {
      return SMUR_FALSE;
    }
  }
  
  //TP更新
  {
    const GFL_UI_TP_HITTBL hitTbl[SEL_MODE_ITEM_NUM+1] =
    {
      {
        SEL_MODE_ITEM_TOP*8 ,
        (SEL_MODE_ITEM_TOP+SEL_MODE_ITEM_HEIGHT)*8 ,
        SEL_MODE_ITEM_LEFT*8 ,
        (SEL_MODE_ITEM_LEFT+SEL_MODE_ITEM_WIDTH)*8 ,
      },
      {
        (SEL_MODE_ITEM_TOP+SEL_MODE_ITEM_HEIGHT+2)*8 ,
        (SEL_MODE_ITEM_TOP+SEL_MODE_ITEM_HEIGHT*2+2)*8 ,
        SEL_MODE_ITEM_LEFT*8 ,
        (SEL_MODE_ITEM_LEFT+SEL_MODE_ITEM_WIDTH)*8 ,
      },
      { GFL_UI_TP_HIT_END,0,0,0 }
    };
    
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    if( ret != GFL_UI_TP_HIT_NONE )
    {
      PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
      if( ret == 0 )
      {
        return SMUR_TRUE;
      }
      else
      {
        return SMUR_FALSE;
      }
    }
  }
  return SMUR_CONTINUE;
}


//--------------------------------------------------------------
//	パレットアニメーションの更新
//--------------------------------------------------------------
//プレートのアニメ。sin使うので0～0xFFFFのループ
#define SEL_MODE_ANIME_VALUE (0x400)
#define SEL_MODE_ANIME_S_R (25)
#define SEL_MODE_ANIME_S_G (30)
#define SEL_MODE_ANIME_S_B (29)
#define SEL_MODE_ANIME_E_R ( 5)
#define SEL_MODE_ANIME_E_G (15)
#define SEL_MODE_ANIME_E_B (21)
//プレートのアニメする色
#define SEL_MODE_ANIME_COL (0x6)

static void SEL_MODE_UpdatePalletAnime( SEL_MODE_WORK *work )
{
  //プレートアニメ
  if( work->anmCnt + SEL_MODE_ANIME_VALUE >= 0x10000 )
  {
    work->anmCnt = work->anmCnt+SEL_MODE_ANIME_VALUE-0x10000;
  }
  else
  {
    work->anmCnt += SEL_MODE_ANIME_VALUE;
  }
  {
    //1～0に変換
    const fx16 cos = (FX_CosIdx(work->anmCnt)+FX16_ONE)/2;
    const u8 r = SEL_MODE_ANIME_S_R + (((SEL_MODE_ANIME_E_R-SEL_MODE_ANIME_S_R)*cos)>>FX16_SHIFT);
    const u8 g = SEL_MODE_ANIME_S_G + (((SEL_MODE_ANIME_E_G-SEL_MODE_ANIME_S_G)*cos)>>FX16_SHIFT);
    const u8 b = SEL_MODE_ANIME_S_B + (((SEL_MODE_ANIME_E_B-SEL_MODE_ANIME_S_B)*cos)>>FX16_SHIFT);
    
    work->transBuf = GX_RGB(r, g, b);
    
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        SEL_MODE_PLT_SEL * 32 + SEL_MODE_ANIME_COL*2 ,
                                        &work->transBuf , 2 );
  }
}
