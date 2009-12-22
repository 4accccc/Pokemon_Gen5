//======================================================================
/**
 * @file  startmenu.c
 * @brief 最初から・続きからを行うトップメニュー
 * @author  ariizumi
 * @data  09/01/07
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/bmp_winframe.h"
#include "arc_def.h"
#include "font/font.naix"

#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "message.naix"
#include "startmenu.naix"
#include "msg/msg_startmenu.h"
#include "sound/pm_sndsys.h"

#include "net/dwc_raputil.h"
#include "savedata/save_control.h"
#include "savedata/mystatus.h"
#include "app/name_input.h"
#include "title/startmenu.h"
#include "title/title.h"
#include "title/game_start.h"
#include "battle_championship/battle_championship.h"
#include "net_app/irc_battle.h"
#include "test/ariizumi/ari_debug.h"
#include "net_app/mystery.h"
#include "msg/msg_wifi_system.h"

//======================================================================
//  define
//======================================================================
#define BG_PLANE_MENU (GFL_BG_FRAME1_M)
#define BG_PLANE_BACK_GROUND (GFL_BG_FRAME3_M)

//BGキャラアドレス
#define START_MENU_FRAMECHR1  (1)
#define START_MENU_FRAMECHR2  (START_MENU_FRAMECHR1 + TALK_WIN_CGX_SIZ)

//パレット
#define START_MENU_PLT_FONT   (0)
#define START_MENU_PLT_SEL    (3)
#define START_MENU_PLT_NOSEL  (4)

//フォントオフセット
#define START_MENU_FONT_TOP  (2)
#define START_MENU_FONT_LEFT (2)

//カーソルの位置
#define START_MENU_CUR_POS_X (232)
#define START_MENU_CUR_UP_POS_Y (24)
#define START_MENU_CUR_DOWN_POS_Y (192-24)

//メニュー項目の左右サイズ(キャラ単位)
#define START_MENU_ITEM_LEFT (3)
#define START_MENU_ITEM_WIDTH (22)

// カーソルパレットアニメの初期カラー
#define START_MENU_ANIME_E_R ( 5)
#define START_MENU_ANIME_E_G (15)
#define START_MENU_ANIME_E_B (21)
//======================================================================
//  enum
//======================================================================
typedef enum 
{
  SMS_FADE_IN,
  SMS_SELECT,
  SMS_FADE_OUT,

  SMS_MAX,
}START_MENU_STATE;

//スタートメニューの項目
enum START_MENU_ITEM
{
  SMI_CONTINUE,   //続きから
  SMI_NEWGAME,    //最初から
  SMI_MYSTERY_GIFT, //不思議な贈り物
  SMI_WIFI_SETTING, //Wifi設定
  SMI_PDW_ACCOUNT,  //PDWアカウント設定
  SMI_GBS_CONNECT,  //GlobalBattleStation接続
  
  SMI_MAX,
  
  SMI_RETURN_TITLE = 0xFF,  //Bで戻る
};

//矢印アニメの種類
enum
{
  CUR_ANM_UP_ON,
  CUR_ANM_DOWN_ON,
  CUR_ANM_UP_OFF,
  CUR_ANM_DOWN_OFF,
};
//======================================================================
//  typedef struct
//======================================================================

//項目情報
typedef struct
{
  BOOL enable;
  u8  top;
  GFL_BMPWIN *win;
}START_MENU_ITEM_WORK;

//ワーク
typedef struct
{
  HEAPID heapId;
  START_MENU_STATE state;
    
  u8    selectItem;  //選択中項目
  u8    selectButtom;  //選択中位置(下端
  int   dispPos;   //表示中位置(dot単位
  int   targetPos;   //表示目標位置(dot単位
  u8    length;    //メニューの長さ(キャラ単位
  
  MYSTATUS  *mystatus;
  
  GFL_FONT *fontHandle;
  GFL_MSGDATA *msgMng; //メニュー作成のところでだけ有効
  GFL_MSGDATA *pMsgWiFiData;  //
  GFL_TCB   *vblankFuncTcb;

  GFL_CLUNIT  *cellUnit;
  GFL_CLWK  *cellCursor[2];
  u32     pltIdx;
  u32     ncgIdx;
  u32     anmIdx;

  START_MENU_ITEM_WORK itemWork[SMI_MAX];

  u16 anmCnt;
  u16 transBuf;
  
  u16 anime_r;
  u16 anime_g;
  u16 anime_b;
  
}START_MENU_WORK;

//項目設定情報
//項目を開くか？
typedef BOOL (*START_MENU_ITEM_CheckFunc)(START_MENU_WORK *work);
//決定時動作
typedef BOOL (*START_MENU_ITEM_SelectFunc)(START_MENU_WORK *work);
//項目内描画
typedef void (*START_MENU_ITEM_DrawFunc)(START_MENU_WORK *work,GFL_BMPWIN *win, const u8 idx );
typedef struct
{
  u8  height;  //高さ(キャラ単位
  //各種動作
  START_MENU_ITEM_CheckFunc checkFunc;
  START_MENU_ITEM_SelectFunc  selectFunc;
  START_MENU_ITEM_DrawFunc  drawFunc;
}START_MENU_ITEM_SETTING;


//======================================================================
//  proto
//======================================================================

static void START_MENU_VBlankFunc(GFL_TCB *tcb,void *work);

static void START_MENU_InitGraphic( START_MENU_WORK *work );
static void START_MENU_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane );

static void START_MENU_CreateMenuItem( START_MENU_WORK *work );

static void START_MENU_UpdatePalletAnime( START_MENU_WORK *work );

//メニュー項目用機能関数
static BOOL START_MENU_ITEM_TempCheck( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_TempCheckFalse( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_CheckContinue( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_CommonSelect( START_MENU_WORK *work );
static BOOL START_MENU_ITEM_WifiUtil( START_MENU_WORK *work );
static void START_MENU_ITEM_CommonDraw( START_MENU_WORK *work , GFL_BMPWIN *win, const u8 idx );
static void START_MENU_ITEM_ContinueDraw( START_MENU_WORK *work , GFL_BMPWIN *win, const u8 idx );

static void START_MENU_ITEM_MsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId );
static void START_MENU_ITEM_WordMsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId , WORDSET *word);

static void START_MENU_UpdateControl( START_MENU_WORK *work );
static void START_MENU_UpdatePad( START_MENU_WORK *work );
static void START_MENU_UpdateTp( START_MENU_WORK *work );

static BOOL START_MENU_MoveSelectItem( START_MENU_WORK *work , const BOOL isDown );
static void START_MENU_CheckButtonAnime( START_MENU_WORK *work );
static void START_MENU_ChangeActiveItem( START_MENU_WORK *work , const u8 newItem , const u8 oldItem );

//Procデータ
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );

const GFL_PROC_DATA StartMenuProcData = {
  START_MENU_ProcInit,
  START_MENU_ProcMain,
  START_MENU_ProcEnd,
};

//メニュー項目設定データ配列
static const START_MENU_ITEM_SETTING ItemSettingData[SMI_MAX] =
{
  { 10 , START_MENU_ITEM_CheckContinue , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_ContinueDraw },
  {  4 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
  {  4 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
  {  4 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_WifiUtil , START_MENU_ITEM_CommonDraw },
  {  4 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
  {  4 , START_MENU_ITEM_TempCheck , START_MENU_ITEM_CommonSelect , START_MENU_ITEM_CommonDraw },
};

FS_EXTERN_OVERLAY(battle_championship);


//--------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  u8 i;
  
  START_MENU_WORK *work;
    //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_STARTMENU, 0x80000 );
  work = GFL_PROC_AllocWork( proc, sizeof(START_MENU_WORK), HEAPID_STARTMENU );
  GFL_STD_MemClear(work, sizeof(START_MENU_WORK));

  work->heapId = HEAPID_STARTMENU;
  for( i=0;i<SMI_MAX;i++ )
  {
    work->itemWork[i].enable = FALSE;
  }
  work->mystatus = SaveData_GetMyStatus( SaveControl_GetPointer() );
  work->state = SMS_FADE_IN;
  work->anmCnt = 0;

  START_MENU_InitGraphic( work );

  //フォント用パレット
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , START_MENU_PLT_FONT * 32, 16*2, work->heapId );
  work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );

  //WinFrame用グラフィック設定
  BmpWinFrame_GraphicSet( BG_PLANE_MENU , START_MENU_FRAMECHR1 , START_MENU_PLT_SEL   , 0, work->heapId);
  BmpWinFrame_GraphicSet( BG_PLANE_MENU , START_MENU_FRAMECHR2 , START_MENU_PLT_NOSEL , 1, work->heapId);
  //背景色
  *((u16 *)HW_BG_PLTT) = 0x7d8c;//RGB(12, 12, 31);
  
  START_MENU_CreateMenuItem( work );
  
  work->vblankFuncTcb = GFUser_VIntr_CreateTCB( START_MENU_VBlankFunc , (void*)work , 0 );

  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 16 , 0 , ARI_FADE_SPD );

  work->anime_r = START_MENU_ANIME_E_R;
  work->anime_g = START_MENU_ANIME_E_G;
  work->anime_b = START_MENU_ANIME_E_B;


//  work->pMsgWiFiData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_system_dat, work->heapID );


  
  return GFL_PROC_RES_FINISH;
}

FS_EXTERN_OVERLAY(pdw_acc);
   
extern const GFL_PROC_DATA PDW_ACC_MainProcData;

//--------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  START_MENU_WORK *work = mywk;

  //選択による分岐処理
  {
    switch( work->selectItem )
    {
    case SMI_CONTINUE:    //続きから
      //Procの変更を中でやってる
      GameStart_Continue();
      break;
    case SMI_NEWGAME:   //最初から
      //Procの変更を中でやってる
      GameStart_Beginning();
      break;
      
    case SMI_MYSTERY_GIFT:  //不思議な贈り物
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(mystery), &MysteryGiftProcData, NULL);
      break;

    case SMI_WIFI_SETTING: //WIFI設定
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(wifi_util), &WifiUtilProcData, NULL);
      break;

    case SMI_PDW_ACCOUNT: //ポケモンドリームワールド設定
      GFL_PROC_SysSetNextProc(NO_OVERLAY_ID, &PDW_ACC_MainProcData, NULL);
        break;
      
    case SMI_GBS_CONNECT: //バトル大会メニュー
      // @todo irc_battleではなく大会メニューへつなぐ
      //GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(battle_championship), &BATTLE_CHAMPIONSHIP_ProcData, NULL);
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(battle_championship), &IRC_BATTLE_ProcData, NULL);
      break;
                
    case SMI_RETURN_TITLE:
      GFL_PROC_SysSetNextProc(FS_OVERLAY_ID(title), &TitleProcData, NULL);
      break;
    default:
      //Procの変更を中でやってる
      GameStart_Beginning();
      break;
    }
  }

  //開放処理
  {
    u8 i;
    GFL_TCB_DeleteTask( work->vblankFuncTcb );

    GFL_CLACT_WK_Remove( work->cellCursor[0] );
    GFL_CLACT_WK_Remove( work->cellCursor[1] );
    
    GFL_CLGRP_PLTT_Release( work->pltIdx );
    GFL_CLGRP_CGR_Release( work->ncgIdx );
    GFL_CLGRP_CELLANIM_Release( work->anmIdx );
    
    GFL_CLACT_UNIT_Delete( work->cellUnit );
    GFL_CLACT_SYS_Delete();

    for( i=0;i<SMI_MAX;i++ )
    {
      if( work->itemWork[i].enable == TRUE )
      {
        GFL_BMPWIN_Delete( work->itemWork[i].win );
      }
    }

    GFL_FONT_Delete( work->fontHandle );
    GFL_BMPWIN_Exit();

    GFL_BG_FreeBGControl(BG_PLANE_MENU);
    GFL_BG_FreeBGControl(BG_PLANE_BACK_GROUND);
    GFL_BG_Exit();
    
    GFL_PROC_FreeWork( proc );

    GFL_HEAP_DeleteHeap( HEAPID_STARTMENU );
  }

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
static GFL_PROC_RESULT START_MENU_ProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  START_MENU_WORK *work = mywk;
  
  switch( work->state )
  {
  case SMS_FADE_IN:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      work->state = SMS_SELECT;
    }
    break;
    
  case SMS_SELECT:
    START_MENU_UpdateControl( work );
    break;
    
  case SMS_FADE_OUT:
    if( GFL_FADE_CheckFade() == FALSE )
    {
      return GFL_PROC_RES_FINISH;
    }
    break;
  }

  
  //スクロール処理(現状特にオーバー処理など入れてないので8の約数で
  if( work->targetPos > work->dispPos )
  {
    work->dispPos += 4;
    GFL_BG_SetScroll( BG_PLANE_MENU , GFL_BG_SCROLL_Y_SET , work->dispPos );
  }
  else
  if( work->targetPos < work->dispPos )
  {
    work->dispPos -= 4;
    GFL_BG_SetScroll( BG_PLANE_MENU , GFL_BG_SCROLL_Y_SET , work->dispPos );
  }

  //OBJの更新
  GFL_CLACT_SYS_Main();

  START_MENU_UpdatePalletAnime( work );

  return GFL_PROC_RES_CONTINUE;
}

//------------------------------------------------------------------
/**
 * @brief VBLank Function
 * @param NONE
 * @return  NONE
 */
//------------------------------------------------------------------
static void START_MENU_VBlankFunc(GFL_TCB *tcb,void *work)
{
  GFL_CLACT_SYS_VBlankFunc();
}

//--------------------------------------------------------------------------
//  グラフィック周り初期化
//--------------------------------------------------------------------------
static void START_MENU_InitGraphic( START_MENU_WORK *work )
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
  
  static const GFL_BG_BGCNT_HEADER bgCont_Menu = {
  0, 0, 0x1000, 0,
  GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };

  static const GFL_BG_BGCNT_HEADER bgCont_BackGround = {
  0, 0, 0x800, 0,
  GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
  GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
  GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
  };
  
  ARCHANDLE *p_handle = GFL_ARC_OpenDataHandle( ARCID_STARTMENU , work->heapId );

  GX_SetMasterBrightness(-16);  
  GXS_SetMasterBrightness(-16);
  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);
  GX_SetDispSelect(GX_DISP_SELECT_MAIN_SUB);

  GFL_DISP_SetBank( &vramBank );
  GFL_BG_Init( work->heapId );
  GFL_BG_SetBGMode( &sysHeader ); 

  START_MENU_InitBgFunc( &bgCont_Menu , BG_PLANE_MENU );
  START_MENU_InitBgFunc( &bgCont_BackGround , BG_PLANE_BACK_GROUND );
  
  GFL_BMPWIN_Init( work->heapId );
  
  //OBJ系
  {
    GFL_CLWK_DATA cellInitData;

    GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
    cellSysInitData.oamst_main = 0x10;  //デバッグメータの分
    cellSysInitData.oamnum_main = 128-0x10;
    GFL_CLACT_SYS_Create( &cellSysInitData , &vramBank ,work->heapId );
    work->cellUnit  = GFL_CLACT_UNIT_Create( 2 , 0, work->heapId );
    GFL_CLACT_UNIT_SetDefaultRend( work->cellUnit );

    //各種素材の読み込み
    work->pltIdx = GFL_CLGRP_PLTT_Register( p_handle , NARC_startmenu_title_cursor_NCLR , CLSYS_DRAW_MAIN , 0 , work->heapId  );
    work->ncgIdx = GFL_CLGRP_CGR_Register( p_handle , NARC_startmenu_title_cursor_NCGR , FALSE , CLSYS_DRAW_MAIN , work->heapId  );
    work->anmIdx = GFL_CLGRP_CELLANIM_Register( p_handle , NARC_startmenu_title_cursor_NCER , NARC_startmenu_title_cursor_NANR, work->heapId  );

    //セルの生成

    cellInitData.pos_x = 232;
    cellInitData.pos_y =  24;
    cellInitData.anmseq = CUR_ANM_UP_ON;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    work->cellCursor[0] = GFL_CLACT_WK_Create( work->cellUnit ,work->ncgIdx,work->pltIdx,work->anmIdx,
                  &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
    GFL_CLACT_WK_SetAutoAnmSpeed( work->cellCursor[0], FX32_ONE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->cellCursor[0], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->cellCursor[0], TRUE );
    
    //↓矢印
    cellInitData.pos_y =  192-24;
    cellInitData.anmseq = CUR_ANM_UP_OFF;
    work->cellCursor[1] = GFL_CLACT_WK_Create( work->cellUnit ,work->ncgIdx,work->pltIdx,work->anmIdx,
                  &cellInitData ,CLSYS_DEFREND_MAIN , work->heapId );
    GFL_CLACT_WK_SetAutoAnmSpeed( work->cellCursor[1], FX32_ONE );
    GFL_CLACT_WK_SetAutoAnmFlag( work->cellCursor[1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->cellCursor[1], TRUE );
        
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
  }

  GFL_ARC_CloseDataHandle( p_handle );
}

//--------------------------------------------------------------------------
//  Bg初期化 機能部
//--------------------------------------------------------------------------
static void START_MENU_InitBgFunc( const GFL_BG_BGCNT_HEADER *bgCont , u8 bgPlane )
{
  GFL_BG_SetBGControl( bgPlane, bgCont, GFL_BG_MODE_TEXT );
  GFL_BG_SetVisible( bgPlane, VISIBLE_ON );
  GFL_BG_ClearFrame( bgPlane );
  GFL_BG_LoadScreenReq( bgPlane );
}

//--------------------------------------------------------------------------
//  メニューの作成
//--------------------------------------------------------------------------
static void START_MENU_CreateMenuItem( START_MENU_WORK *work )
{
  u8  i;
  u8  bgTopPos = 1; //アイテムの表示位置(キャラ単位

  work->msgMng = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_startmenu_dat, work->heapId);
  work->selectItem = 0xFF; //最初にアクティブなのは一番上にするための判定用
  for( i=0;i<SMI_MAX;i++ )
  {
    if( ItemSettingData[i].checkFunc(work) == TRUE )
    {
      work->itemWork[i].enable = TRUE;
      work->itemWork[i].win = GFL_BMPWIN_Create( 
            BG_PLANE_MENU , 
            START_MENU_ITEM_LEFT , bgTopPos ,
            START_MENU_ITEM_WIDTH , ItemSettingData[i].height ,
            START_MENU_PLT_FONT , 
            GFL_BMP_CHRAREA_GET_B );
      GFL_BMPWIN_MakeScreen( work->itemWork[i].win );
      GFL_BMPWIN_TransVramCharacter( work->itemWork[i].win );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->itemWork[i].win), 0xf );

      ItemSettingData[i].drawFunc(work,work->itemWork[i].win,i);
      
      GFL_BMPWIN_TransVramCharacter( work->itemWork[i].win );
      BmpWinFrame_Write(work->itemWork[i].win, WINDOW_TRANS_ON, 
              START_MENU_FRAMECHR1, START_MENU_PLT_NOSEL);
      
      bgTopPos += ItemSettingData[i].height + 2;
      if( work->selectItem == 0xFF )
      {
        //一番上の項目
        work->selectItem = i;
      }
    }
  }
  GFL_MSG_Delete( work->msgMng );
  work->msgMng = NULL;
  
  GF_ASSERT( work->selectItem != 0xFF );
  work->selectButtom = ItemSettingData[work->selectItem].height + 2;
  work->length = bgTopPos-1; //次のメニューの枠の分も足しているので1引く
  START_MENU_ChangeActiveItem( work , work->selectItem , 0xFF );
  
  //最初の下カーソルのアニメチェック
  if( work->length > 24 )
  {
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[0], CUR_ANM_UP_OFF );
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[1], CUR_ANM_DOWN_ON );
  }
  else
  {
    //スクロールできないなら消す
    GFL_CLACT_WK_SetDrawEnable( work->cellCursor[0], FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->cellCursor[1], FALSE );
  }

}

//--------------------------------------------------------------------------
//  選択メニューの切り替え
//--------------------------------------------------------------------------
static void START_MENU_ChangeActiveItem( START_MENU_WORK *work , const u8 newItem , const u8 oldItem )
{
  if( newItem != 0xFF )
  {
    BmpWinFrame_Write(work->itemWork[newItem].win, WINDOW_TRANS_ON, 
            START_MENU_FRAMECHR2, START_MENU_PLT_SEL);
  }
  if( oldItem != 0xFF )
  {
    BmpWinFrame_Write(work->itemWork[oldItem].win, WINDOW_TRANS_ON, 
            START_MENU_FRAMECHR1, START_MENU_PLT_NOSEL);
  }
}

// カーソルが一番白くなる時
#define CURSOR_MAX_WHITE_POINT  (0xc000)

//--------------------------------------------------------------
//  パレットアニメーションの更新
//--------------------------------------------------------------
//プレートのアニメ。sin使うので0～0xFFFFのループ
#define START_MENU_ANIME_VALUE (0x400)
#define START_MENU_ANIME_S_R (25)
#define START_MENU_ANIME_S_G (30)
#define START_MENU_ANIME_S_B (29)
//プレートのアニメする色
#define START_MENU_ANIME_COL (0x6)

static void START_MENU_UpdatePalletAnime( START_MENU_WORK *work )
{
  //プレートアニメ
  if( work->anmCnt + START_MENU_ANIME_VALUE >= 0x10000 )
  {
    work->anmCnt = work->anmCnt+START_MENU_ANIME_VALUE-0x10000;
  }
  else
  {
    work->anmCnt += START_MENU_ANIME_VALUE;
  }

  // カーソル一番白くなる時にカラーを変更する
  if(work->anmCnt==CURSOR_MAX_WHITE_POINT){
    work->anime_r = GFL_STD_MtRand(14)+10;
    work->anime_g = GFL_STD_MtRand(24);
    work->anime_b = 24-work->anime_g;
  }

  {
    //1～0に変換
    const fx16 sin = (FX_SinIdx(work->anmCnt)+FX16_ONE)/2;
    const u8 r = START_MENU_ANIME_S_R + (((work->anime_r-START_MENU_ANIME_S_R)*sin)>>FX16_SHIFT);
    const u8 g = START_MENU_ANIME_S_G + (((work->anime_g-START_MENU_ANIME_S_G)*sin)>>FX16_SHIFT);
    const u8 b = START_MENU_ANIME_S_B + (((work->anime_b-START_MENU_ANIME_S_B)*sin)>>FX16_SHIFT);
    
    work->transBuf = GX_RGB(r, g, b);
    
    NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_2D_BG_PLTT_MAIN ,
                                        START_MENU_PLT_SEL * 32 + START_MENU_ANIME_COL*2 ,
                                        &work->transBuf , 2 );
  }
}


#pragma mark MenuSelectFunc

//--------------------------------------------------------------------------
//  メニューを有効にするか？
//  @return TRUE=有効
//--------------------------------------------------------------------------
static BOOL START_MENU_ITEM_TempCheck( START_MENU_WORK *work )
{
  return TRUE;
}
static BOOL START_MENU_ITEM_TempCheckFalse( START_MENU_WORK *work )
{
  return FALSE;
}
static BOOL START_MENU_ITEM_CheckContinue( START_MENU_WORK *work )
{
  return SaveData_GetExistFlag( SaveControl_GetPointer() );
}

//--------------------------------------------------------------------------
//  決定時の動作
//  @return TRUE=スタートメニュの終了処理へ
//--------------------------------------------------------------------------
static BOOL START_MENU_ITEM_CommonSelect( START_MENU_WORK *work )
{
  //今のところ選択された
  return TRUE;
}
#if 0

//------------------------------------------------------------------------------
/**
 * @brief   説明ウインドウ表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _infoMessageDisp(GAMESYNC_MENU* pWork)
{
  GFL_BMPWIN* pwin;

  
  if(pWork->infoDispWin==NULL){
    pWork->infoDispWin = GFL_BMPWIN_Create(
      GFL_BG_FRAME1_S ,
      1 , 3, 30 ,4 ,
      _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  }
  pwin = pWork->infoDispWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);
  GFL_FONTSYS_SetColor(1, 2, 15);

  pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
}
#endif
//--------------------------------------------------------------------------
//  決定時の動作  WIFI設定
//  @return TRUE=スタートメニュの終了処理へ
//--------------------------------------------------------------------------
static BOOL START_MENU_ITEM_WifiUtil( START_MENU_WORK *work )
{
  if( !OS_IsRunOnTwl() ){//DSIは呼ぶことが出来ない
    return TRUE;
  }
  else{


    
    //GFL_MSG_GetString( pWork->pMsgWiFiData, dwc_message_0017, pWork->pStrBuf );

    
//    GF_ASSERT(0);
  }
}



//メニュー内描画共通(idxでmsgIdを読み分ける
static void START_MENU_ITEM_CommonDraw( START_MENU_WORK *work , GFL_BMPWIN *win , const u8 idx )
{
  static const msgIdArr[SMI_MAX] =
  {
    START_MENU_STR_ITEM_01_01,  //実際はこない(ダミー
    START_MENU_STR_ITEM_02,
    START_MENU_STR_ITEM_03,
    START_MENU_STR_ITEM_04,
    START_MENU_STR_ITEM_06,
    START_MENU_STR_ITEM_05,
  };
  
  START_MENU_ITEM_MsgDrawFunc( work , win , 0,0, msgIdArr[idx] );
}

//続きからメニュー内描画
static void START_MENU_ITEM_ContinueDraw( START_MENU_WORK *work , GFL_BMPWIN *win , const u8 idx )
{
  u8 lCol,sCol,bCol;
  WORDSET *word;
  STRBUF *nameStr;
  const STRCODE *myname = MyStatus_GetMyName( work->mystatus );

  word = WORDSET_Create( work->heapId );
  //Font色を戻すため取っておく
  GFL_FONTSYS_GetColor( &lCol,&sCol,&bCol );

  START_MENU_ITEM_MsgDrawFunc( work , win ,  0, 0, START_MENU_STR_ITEM_01_01 );

  GFL_FONTSYS_SetColor( 5,sCol,bCol );
  START_MENU_ITEM_MsgDrawFunc( work , win , 32,16, START_MENU_STR_ITEM_01_02 );
  START_MENU_ITEM_MsgDrawFunc( work , win , 32,32, START_MENU_STR_ITEM_01_03 );
  START_MENU_ITEM_MsgDrawFunc( work , win , 32,48, START_MENU_STR_ITEM_01_04 );
  START_MENU_ITEM_MsgDrawFunc( work , win , 32,64, START_MENU_STR_ITEM_01_05 );

  //名前  
  nameStr =   GFL_STR_CreateBuffer( 16, work->heapId );
  if( MyStatus_CheckNameClear( work->mystatus ) == FALSE )
  {
    GFL_STR_SetStringCode( nameStr , myname );
  }
  else
  {
    //TODO 念のため名前が入ってないときに落ちないようにしておく
    u16 tempName[7] = { L'N',L'o',L'N',L'a',L'm',L'e',0xFFFF };
    GFL_STR_SetStringCode( nameStr , tempName );
  }
  WORDSET_RegisterWord( word, 0, nameStr, 0, TRUE , 0 );
  START_MENU_ITEM_WordMsgDrawFunc( work , win ,160,16, START_MENU_STR_ITEM_01_06 , word);

  //プレイ時間
  WORDSET_RegisterNumber( word, 0, 9, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
  WORDSET_RegisterNumber( word, 1, 2, 2, STR_NUM_DISP_ZERO , STR_NUM_CODE_HANKAKU );
  START_MENU_ITEM_WordMsgDrawFunc( work , win ,160,32, START_MENU_STR_ITEM_01_07 , word);
  //ポケモン図鑑
  WORDSET_RegisterNumber( word, 0, 55, 3, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
  START_MENU_ITEM_WordMsgDrawFunc( work , win ,160,48, START_MENU_STR_ITEM_01_08 , word);
  //バッジ
  WORDSET_RegisterNumber( word, 0, MyStatus_GetBadgeCount(work->mystatus), 1, STR_NUM_DISP_SPACE, STR_NUM_CODE_HANKAKU );
  START_MENU_ITEM_WordMsgDrawFunc( work , win ,160,64, START_MENU_STR_ITEM_01_09 , word);

  //Font色を戻す
  GFL_FONTSYS_SetColor( lCol,sCol,bCol );
  
  GFL_STR_DeleteBuffer( nameStr );
  WORDSET_Delete( word );
}

static void START_MENU_ITEM_MsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId )
{
  STRBUF  *str = GFL_STR_CreateBuffer( 96 , work->heapId );
  GFL_MSG_GetString( work->msgMng , msgId , str );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(win),
          posX+START_MENU_FONT_LEFT,
          posY+START_MENU_FONT_TOP,
          str,
          work->fontHandle);
  
  GFL_STR_DeleteBuffer( str );
}

static void START_MENU_ITEM_WordMsgDrawFunc( START_MENU_WORK *work , GFL_BMPWIN *win , u8 posX , u8 posY , u16 msgId , WORDSET *word)
{
  u8 strLen;
  STRBUF  *str = GFL_STR_CreateBuffer( 96 , work->heapId );
  STRBUF  *baseStr = GFL_STR_CreateBuffer( 96 , work->heapId );
  GFL_MSG_GetString( work->msgMng , msgId , baseStr );
  WORDSET_ExpandStr( word , str , baseStr );

  strLen = PRINTSYS_GetStrWidth( str , work->fontHandle , 0 );
  
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(win),
          posX+START_MENU_FONT_LEFT - strLen,
          posY+START_MENU_FONT_TOP,
          str,
          work->fontHandle);
  
  GFL_STR_DeleteBuffer( baseStr );
  GFL_STR_DeleteBuffer( str );
  
}

#pragma mark ControlFunc

//--------------------------------------------------------------------------
//  操作系更新
//--------------------------------------------------------------------------
static void START_MENU_UpdateControl( START_MENU_WORK *work )
{
  START_MENU_UpdatePad(work);
//  START_MENU_UpdateTp(work);
}

//--------------------------------------------------------------------------
//  キー操作
//--------------------------------------------------------------------------
static void START_MENU_UpdatePad( START_MENU_WORK *work )
{
  const int keyTrg = GFL_UI_KEY_GetTrg();
  if( keyTrg & PAD_KEY_UP )
  {
    START_MENU_MoveSelectItem( work , FALSE );
  }
  else
  if( keyTrg & PAD_KEY_DOWN )
  {
    START_MENU_MoveSelectItem( work , TRUE );
  }
  else
  if( keyTrg & PAD_BUTTON_A )
  {
    const BOOL ret = ItemSettingData[work->selectItem].selectFunc( work );
    if( ret == TRUE )
    {
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 0 , 16 , ARI_FADE_SPD );
      work->state = SMS_FADE_OUT;
      PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
    }
  }
  else
  if( keyTrg & PAD_BUTTON_B )
  {
    GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 0 , 16 , ARI_FADE_SPD );
    work->selectItem = SMI_RETURN_TITLE;
    work->state = SMS_FADE_OUT;
    PMSND_PlaySystemSE( SEQ_SE_CANCEL1 );
  }
}

//--------------------------------------------------------------------------
//  TP操作
//--------------------------------------------------------------------------
static void START_MENU_UpdateTp( START_MENU_WORK *work )
{
  //上下ボタン
  if( GFL_UI_TP_GetTrg() == TRUE )
  {
    GFL_UI_TP_HITTBL hitTbl[3] =
    {
      { 
        START_MENU_CUR_UP_POS_Y - 16 , START_MENU_CUR_UP_POS_Y+16,
        START_MENU_CUR_POS_X - 16 , START_MENU_CUR_POS_X + 16 
      },
      { 
        START_MENU_CUR_DOWN_POS_Y-16 , START_MENU_CUR_DOWN_POS_Y+16,
        START_MENU_CUR_POS_X - 16 , START_MENU_CUR_POS_X + 16 
      },
      { 
        GFL_UI_TP_HIT_END , 0,0,0 
      },
    };
    
    const int ret = GFL_UI_TP_HitTrg( hitTbl );
    if( ret == 0 && 
        GFL_CLACT_WK_GetAnmSeq( work->cellCursor[0] ) == CUR_ANM_UP_ON )
    {
      work->targetPos -= 192;
      if( work->targetPos < 0 )
      {
        work->targetPos = 0;
      }
      START_MENU_CheckButtonAnime( work );
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
    }
    else
    if( ret == 1 && 
        GFL_CLACT_WK_GetAnmSeq( work->cellCursor[1] ) == CUR_ANM_DOWN_ON )
    {
      work->targetPos += 192;
      if( work->targetPos > work->length*8 -192 )
      {
        work->targetPos = work->length*8 -192;
      }
      START_MENU_CheckButtonAnime( work );
      PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
    }

    if( ret == GFL_UI_TP_HIT_NONE )
    {
      //ボタンとの判定
      u32 tpx,tpy;
      GFL_UI_TP_GetPointTrg( &tpx,&tpy );
      if( tpx >= START_MENU_ITEM_LEFT*8 &&
          tpx < (START_MENU_ITEM_LEFT+START_MENU_ITEM_WIDTH)*8 )
        { 
        u8 i =0;
        u8 touchLine = tpy/8;
        int nowLine = -(work->targetPos/8);
        for( i = 0; i < SMI_MAX ; i++ )
        {
          if( work->itemWork[i].enable == TRUE )
          {
            //枠の上の分で+1
            if( nowLine+1 <= touchLine &&
                nowLine+ItemSettingData[i].height+1 > touchLine )
            {
              const BOOL ret = ItemSettingData[i].selectFunc( work );
              if( ret == TRUE )
              {
                GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN , 0 , 16 , ARI_FADE_SPD );
                work->state = SMS_FADE_OUT;
                work->selectItem = i;
                PMSND_PlaySystemSE( SEQ_SE_DECIDE1 );
              }
              break;
            }
            nowLine += ItemSettingData[i].height + 2;//枠で+2
          }
        }
      }
    }
  }
}


//--------------------------------------------------------------------------
//  項目移動
//  @return 移動できたか？
//  @value  BOOL isDown　FALSE:↑移動 TRUE:↓移動
//--------------------------------------------------------------------------
static BOOL START_MENU_MoveSelectItem( START_MENU_WORK *work , const BOOL isDown )
{
  const int moveValue = (isDown==TRUE ? 1 : -1 );
  const int endValue = (isDown==TRUE ? SMI_MAX : -1 );
  int i;
  BOOL isShow;
  //開始位置は１つ分動かしておく
  for( i = work->selectItem + moveValue; i != endValue ; i += moveValue )
  {
    if( work->itemWork[i].enable == TRUE )
    {
      break;
    }
  }
  if( i == endValue )
  {
    //動けなかった
    return FALSE;
  }
  
  START_MENU_ChangeActiveItem( work , i , work->selectItem );
  if( isDown == TRUE )
  {
    work->selectButtom += ItemSettingData[i].height + 2;  //枠で+2
  }
  else
  {
    work->selectButtom -= ItemSettingData[work->selectItem].height + 2;  //枠で+2
  }
  
  work->selectItem = i;
  
  //表示位置計算
  if( work->selectButtom*8 > work->targetPos + 192 )
  {
    //下のチェック
    work->targetPos = work->selectButtom*8 -192;
  }
  else  //メニューの頭の位置なので計算が長い・・・
  if( ( work->selectButtom - ItemSettingData[work->selectItem].height - 2 )*8 < work->targetPos )
  {
    //上のチェック
    work->targetPos = ( work->selectButtom - ItemSettingData[work->selectItem].height - 2 )*8;
  }
  PMSND_PlaySystemSE( SEQ_SE_SELECT1 );
  START_MENU_CheckButtonAnime( work );
  return TRUE;
}

static void START_MENU_CheckButtonAnime( START_MENU_WORK *work )
{

  //カーソル表示のチェック
  //上
  if( work->targetPos > 0 )
  {
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[0], CUR_ANM_UP_ON );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[0], CUR_ANM_UP_OFF );
  }

  //下
  if( work->targetPos + 192 < work->length*8 )
  {
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[1], CUR_ANM_DOWN_ON );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( work->cellCursor[1], CUR_ANM_DOWN_OFF );
  }
  
}

