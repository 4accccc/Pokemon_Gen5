//============================================================================================
/**
 * @file  d_taya.c
 * @brief デバッグ用関数
 * @author  taya
 * @date  2008.08.01
 */
//============================================================================================

// lib includes -----------------------
#include <gflib.h>
#include <assert_default.h>
#include <procsys.h>
#include <tcbl.h>
#include <msgdata.h>
#include <net.h>


// global includes --------------------
#include "system\main.h"
#include "print\printsys.h"
#include "print\gf_font.h"
#include "poke_tool\pokeparty.h"
#include "poke_tool\poke_tool.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazadata.h"
#include "item\itemsym.h"
#include "net\network_define.h"
#include "battle\battle.h"
#include "sound/pm_sndsys.h"
#include "gamesystem\game_data.h"
#include "gamesystem\btl_setup.h"

// local includes ---------------------
#include "msg\msg_d_taya.h"

// archive includes -------------------
#include "arc_def.h"
#include "message.naix"
#include "test_graphic/d_taya.naix"
#include "font/font.naix"

/*--------------------------------------------------------------------------*/
/* Consts                                                                   */
/*--------------------------------------------------------------------------*/
enum {
  GENERIC_WORK_SIZE = 1024,

  HEAPID_CORE = HEAPID_TAYA_DEBUG,
  HEAPID_TEMP = HEAPID_TAYA_DEBUG_SUB,
};

#define HEAPID_CORE_LOW (GFL_HEAP_LOWID( HEAPID_CORE))

/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BOOL (*pSubProc)( GFL_PROC*, int*, void*, void* );

typedef struct _V_MENU_CTRL   V_MENU_CTRL;

struct _V_MENU_CTRL {
  s16   selPos;
  s16   writePos;
  s16   maxLines;
  s16   maxElems;
};

typedef struct {

  u16   yofs;
  u8    strNum;
  u8    kanjiMode;

}TEST_PACKET;

typedef struct {

  u16   seq;
  HEAPID  heapID;
  u16   strNum;
  u16   kanjiMode;
  u32   yofs;

  GFL_BMPWIN*     win;
  GFL_BMP_DATA*   bmp;
  GFL_MSGDATA*    mm;
  STRBUF*         strbuf;
  GFL_TCBLSYS*    tcbl;

  GFL_FONT*         fontHandle;
  GFL_FONT*         numFont;
  PRINT_STREAM*     printStream;
  PRINT_QUE*        printQue;
  PRINT_UTIL        printUtil[1];
  GFL_PROCSYS*      localProc;

  pSubProc    subProc;
  int         subSeq;
  u16         subArg;
  u8          tmpModuleExistFlag;

  V_MENU_CTRL   menuCtrl;
  GAMEDATA*     gameData;

  void*     arignDmy;
  u8        genericWork[ GENERIC_WORK_SIZE ];

//  PRINT_STREAM_HANDLE psHandle;

  GFLNetInitializeStruct  netInitWork;
  int                     netTestSeq;
  GFL_NETHANDLE*          netHandle;
  BOOL                    ImParent;
  POKEPARTY*              partyPlayer;
  POKEPARTY*              partyEnemy;

  TEST_PACKET     packet;
  POKEMON_PARAM*  testPoke;
  u8              testPokeEditFlag;

}MAIN_WORK;


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static GFL_PROC_RESULT DebugTayaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static GFL_PROC_RESULT DebugTayaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void initGraphicSystems( MAIN_WORK* wk );
static void quitGraphicSystems( MAIN_WORK* wk );
static void startView( MAIN_WORK* wk );
static void createTemporaryModules( MAIN_WORK* wk );
static void deleteTemporaryModules( MAIN_WORK* wk );
static void changeScene_start( MAIN_WORK* wk );
static void changeScene_recover( MAIN_WORK* wk );
static GFL_PROC_RESULT DebugTayaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk );
static void VMENU_Init( V_MENU_CTRL* ctrl, u8 maxLines, u8 maxElems );
static BOOL VMENU_Ctrl( V_MENU_CTRL* ctrl );
static u8 VMENU_GetSelPos( const V_MENU_CTRL* ctrl );
static u8 VMENU_GetWritePos( const V_MENU_CTRL* ctrl );
static void print_menu( MAIN_WORK* wk, const V_MENU_CTRL* menuCtrl );
static void* getGenericWork( MAIN_WORK* mainWork, u32 size );
static BOOL SUBPROC_PrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void testPacketFunc( const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle );
static void* testBeaconGetFunc( void* pWork );
static int testBeaconGetSizeFunc( void* pWork );
static BOOL testBeaconCompFunc( GameServiceID myNo, GameServiceID beaconNo );
static void testCallBack(void* pWork);
static void autoConnectCallBack( void* pWork );
static BOOL SUBPROC_MakePokeTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static BOOL SUBPROC_GoBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void* btlBeaconGetFunc( void* pWork );
static int btlBeaconGetSizeFunc( void* pWork );
static BOOL btlBeaconCompFunc( GameServiceID myNo, GameServiceID beaconNo );
static void btlAutoConnectCallback( void* pWork );
static BOOL SUBPROC_CommBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static BOOL SUBPROC_MultiBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void setup_party( HEAPID heapID, POKEPARTY* party, ... );
static BOOL SUBPROC_KanjiMode( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static BOOL SUBPROC_NetPrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk );



/*--------------------------------------------------------------------------*/
/* Menu Table                                                               */
/*--------------------------------------------------------------------------*/

static const struct {
  u32       strID;
  pSubProc  subProc;
  u16       subArg;
  u16       quickKey;
}MainMenuTbl[] = {
  { DEBUG_TAYA_MENU1,   SUBPROC_GoBattle,       0, 0 },
  { DEBUG_TAYA_MENU2,   SUBPROC_MakePokeTest,   0, 0 },
  { DEBUG_TAYA_MENU3,   SUBPROC_CommBattle,     0, 0 },
  { DEBUG_TAYA_MENU4,   SUBPROC_MultiBattle,    1, PAD_BUTTON_X },
  { DEBUG_TAYA_MENU5,   SUBPROC_MultiBattle,    0, PAD_BUTTON_Y },
  { DEBUG_TAYA_MENU6,   SUBPROC_KanjiMode,      0, 0 },
  { DEBUG_TAYA_MENU8,   SUBPROC_NetPrintTest,   0, 0 },
  { DEBUG_TAYA_MENU9,   SUBPROC_PrintTest,      0, 0 },
};

enum {
  MAINMENU_DISP_MAX = 12,
  MAINMENU_PRINT_OX = 0,
  MAINMENU_PRINT_OY = 0,
};


//--------------------------------------------------------------------------
/**
 * PROC Init
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  MAIN_WORK* wk;

  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CORE,   0x12000 );
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TEMP,   0xb0000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(MAIN_WORK), HEAPID_CORE );
  wk->heapID = HEAPID_TEMP;
  wk->tmpModuleExistFlag = FALSE;
  wk->localProc = GFL_PROC_LOCAL_boot( wk->heapID );

  initGraphicSystems( wk );
  createTemporaryModules( wk );

  startView( wk );

  VMENU_Init( &wk->menuCtrl, MAINMENU_DISP_MAX, NELEMS(MainMenuTbl) );
  wk->seq = 0;
  wk->subArg = 0;
  wk->subProc = NULL;
  wk->testPoke = PP_Create( MONSNO_POTTYAMA, 20, 3594, HEAPID_CORE );
  wk->testPokeEditFlag = FALSE;
//  wk->gameData = GAMEDATA_Create( HEAPID_CORE );

  return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  MAIN_WORK* wk = mywk;

  GFL_PROC_LOCAL_Exit( wk->localProc );
  GFL_HEAP_FreeMemory( wk->testPoke );
  GFL_HEAP_DeleteHeap( HEAPID_TEMP );
  GFL_HEAP_DeleteHeap( HEAPID_CORE );
  return GFL_PROC_RES_FINISH;
}



static void initGraphicSystems( MAIN_WORK* wk )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_0_F,     // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_0123_E,     // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_128K,// メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
  };

  GFL_DISP_SetBank( &vramBank );

  // 各種効果レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  GFL_BG_Init( wk->heapID );
  GFL_BMPWIN_Init( wk->heapID );
  GFL_FONTSYS_Init();

  //ＢＧモード設定
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &sysHeader );
  }

  // 個別フレーム設定
  {
    static const GFL_BG_BGCNT_HEADER bgcntText = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };
    static const GFL_BG_BGCNT_HEADER bgcntText2 = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x4000, GX_BG_CHARBASE_0x18000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

    GFL_BG_SetBGControl( GFL_BG_FRAME1_M,   &bgcntText2,   GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

    GFL_BG_SetVisible( GFL_BG_FRAME0_S,   VISIBLE_ON );
    GFL_BG_SetVisible( GFL_BG_FRAME1_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME2_S,   VISIBLE_OFF );
    GFL_BG_SetVisible( GFL_BG_FRAME3_S,   VISIBLE_OFF );

//    GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->heapID );
//    void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
    GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );
    GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

    GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->heapID );
    GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0xff, 1, 0 );
    GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
  }
}

static void quitGraphicSystems( MAIN_WORK* wk )
{
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_M );
  GFL_BG_FreeBGControl( GFL_BG_FRAME0_S );

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

static void startView( MAIN_WORK* wk )
{
  GFL_BMP_Clear( wk->bmp, 0xff );
  GFL_BMPWIN_MakeScreen( wk->win );

  GFL_BMPWIN_TransVramCharacter( wk->win );
  GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
}

static void createTemporaryModules( MAIN_WORK* wk )
{
  wk->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_S, 0, 0, 32, 24, 0, GFL_BMP_CHRAREA_GET_F );
  wk->bmp = GFL_BMPWIN_GetBmp(wk->win);

  #if 0
  {
    void* mmdat = GFL_ARC_LoadDataAlloc( ARCID_MESSAGE, NARC_message_d_taya_dat, wk->heapID );
    wk->mm = GFL_MSG_Construct( mmdat, wk->heapID );
  }
  #else
  wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_taya_dat, wk->heapID );
  #endif
  wk->strbuf = GFL_STR_CreateBuffer( 1024, wk->heapID );
  wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

  GFL_HEAP_CheckHeapSafe( wk->heapID );
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT,
    NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  wk->numFont = GFL_FONT_Create( ARCID_FONT,
    NARC_font_num_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

  wk->printQue = PRINTSYS_QUE_Create( wk->heapID );
  PRINT_UTIL_Setup( wk->printUtil, wk->win );

  wk->tmpModuleExistFlag = TRUE;
}

static void deleteTemporaryModules( MAIN_WORK* wk )
{
  PRINTSYS_QUE_Delete( wk->printQue );
  GFL_FONT_Delete( wk->numFont );
  GFL_FONT_Delete( wk->fontHandle );


  GFL_TCBL_Exit( wk->tcbl );
  GFL_STR_DeleteBuffer( wk->strbuf );
  GFL_MSG_Delete( wk->mm );

  GFL_BMPWIN_Delete( wk->win );

  wk->tmpModuleExistFlag = FALSE;
}



static void changeScene_start( MAIN_WORK* wk )
{
  deleteTemporaryModules( wk );
  quitGraphicSystems( wk );
  GFL_HEAP_DeleteHeap( HEAPID_TEMP );
}
static void changeScene_recover( MAIN_WORK* wk )
{
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TEMP,  0xb0000 );
  initGraphicSystems( wk );
  createTemporaryModules( wk );
  startView( wk );
}


//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugTayaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  MAIN_WORK* wk = mywk;

  if( wk->subProc != NULL )
  {
    if( wk->subProc( proc, &(wk->subSeq), pwk, mywk ) )
    {
      wk->subProc = NULL;
      *seq = 0;
    }
    return GFL_PROC_RES_CONTINUE;
  }


  PRINTSYS_QUE_Main( wk->printQue );
  if( !PRINT_UTIL_Trans( wk->printUtil, wk->printQue ) )
  {
    return FALSE;
  }

  switch( *seq ){
  case 0:
    print_menu( wk, &wk->menuCtrl );
    (*seq)++;
    break;

  case 1:
    if( VMENU_Ctrl(&wk->menuCtrl) )
    {
      print_menu( wk, &wk->menuCtrl );
      break;
    }

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
    {
      GFL_MSGSYS_SetLangID( !GFL_MSGSYS_GetLangID() );
      print_menu( wk, &wk->menuCtrl );
      break;
    }

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      u16 p = VMENU_GetSelPos( &wk->menuCtrl );
      wk->subProc = MainMenuTbl[p].subProc;
      wk->subArg = MainMenuTbl[p].subArg;
      wk->subSeq = 0;
      (*seq)++;
    }
    else
    {
      u16 key = GFL_UI_KEY_GetTrg();

      if( key & PAD_BUTTON_B ){ key = PAD_BUTTON_B; }
      else if( key & PAD_BUTTON_X ){ key = PAD_BUTTON_X; }
      else if( key & PAD_BUTTON_Y ){ key = PAD_BUTTON_Y; }
      else{ key = 0; }

      if( key )
      {
        int i;
        for(i=0; i<NELEMS(MainMenuTbl); ++i)
        {
          if( MainMenuTbl[i].quickKey == key )
          {
            wk->subProc = MainMenuTbl[i].subProc;
            wk->subArg = MainMenuTbl[i].subArg;
            wk->subSeq = 0;
            (*seq)++;
            break;
          }
        }
      }
    }
    break;

  case 2:
    if( wk->subProc( proc, &(wk->subSeq), pwk, mywk ) )
    {
      wk->subProc = NULL;
      *seq = 0;
    }
    break;
  }

  return GFL_PROC_RES_CONTINUE;
}

//-------------------------------------------------------------------------------------------------------
// メニューコントローラ
//-------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------
/**
 * 縦メニューコントローラ初期化
 *
 * @param   ctrl      コントローラ
 * @param   maxLines    表示可能な行数
 * @param   maxElems    選択可能な項目数
 *
 */
//--------------------------------------------------------------------------
static void VMENU_Init( V_MENU_CTRL* ctrl, u8 maxLines, u8 maxElems )
{
  ctrl->selPos = 0;
  ctrl->writePos = 0;
  ctrl->maxElems = maxElems;
  ctrl->maxLines = maxLines;
}
//--------------------------------------------------------------------------
/**
 * 縦メニューコントローラ　キーチェック
 *
 * @param   ctrl    コントローラ
 *
 * @retval  BOOL    選択項目の変更があればTRUE
 */
//--------------------------------------------------------------------------
static BOOL VMENU_Ctrl( V_MENU_CTRL* ctrl )
{
  u16 key = GFL_UI_KEY_GetRepeat();

  do {

    if( key & PAD_KEY_DOWN )
    {
      if( ctrl->selPos < (ctrl->maxElems-1) )
      {
        if( ++(ctrl->selPos) >= (ctrl->writePos + ctrl->maxLines) )
        {
          ctrl->writePos++;
        }
        break;
      }
    }

    if( key & PAD_KEY_UP )
    {
      if( ctrl->selPos )
      {
        if( --(ctrl->selPos) < ctrl->writePos )
        {
          --(ctrl->writePos);
        }
        break;
      }
    }

    return FALSE;

  }while(0);

  return TRUE;
}

static u8 VMENU_GetSelPos( const V_MENU_CTRL* ctrl )
{
  return ctrl->selPos;
}
static u8 VMENU_GetWritePos( const V_MENU_CTRL* ctrl )
{
  return ctrl->writePos;
}


//--------------------------------------------------------------------------
/**
 * メニュー項目描画
 *
 * @param   wk
 *
 */
//--------------------------------------------------------------------------
static void print_menu( MAIN_WORK* wk, const V_MENU_CTRL* menuCtrl )
{
  u16 selPos, writePos;
  u16 ypos;

  u8 fontCol;

  selPos = VMENU_GetSelPos( menuCtrl );
  writePos = VMENU_GetWritePos( menuCtrl );

  GFL_BMP_Clear( wk->bmp, 0xff );

  ypos = MAINMENU_PRINT_OY;

  while( writePos < NELEMS(MainMenuTbl) )
  {
    GFL_MSG_GetString( wk->mm, MainMenuTbl[writePos].strID, wk->strbuf );

    if( writePos == selPos )
    {
      fontCol = 0x03;
    }
    else
    {
      switch( MainMenuTbl[writePos].quickKey ){
      case PAD_BUTTON_X: fontCol = 0x05; break;
      case PAD_BUTTON_Y: fontCol = 0x09; break;
      case PAD_BUTTON_B: fontCol = 0x0b; break;
      default: fontCol = 0x01; break;
      }
    }

    GFL_FONTSYS_SetColor( fontCol, 2, 0x0f );

    {
//      GFL_FONT* font = (writePos==0)? wk->numFont : wk->fontHandle;
      GFL_FONT* font = wk->fontHandle;

      PRINTSYS_Print( wk->bmp, MAINMENU_PRINT_OX, ypos, wk->strbuf, font );
      GFL_FONTSYS_SetDefaultColor();

      ypos += (GFL_FONT_GetLineHeight(font)+2);
    }
    writePos++;
  }
  GFL_FONTSYS_SetDefaultColor();

  GFL_BMPWIN_TransVramCharacter( wk->win );
}

//------------------------------------------------------------------------------------------------------
// メインワーク関数
//------------------------------------------------------------------------------------------------------
static void* getGenericWork( MAIN_WORK* mainWork, u32 size )
{
  GF_ASSERT(size<GENERIC_WORK_SIZE);
//  GFL_STD_MemClear( mainWork->genericWork, size );
  {
    u32 adrs = (u32)(&mainWork->genericWork[0]);
    GF_ASSERT(adrs%4==0);
  }
  return mainWork->genericWork;
}

//------------------------------------------------------------------------------------------------------
// スタンドアロン状態での漢字PrintTest
//------------------------------------------------------------------------------------------------------
static BOOL SUBPROC_PrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  static const u16 strID[] = {
    DEBUG_TAYA_STR01,
    DEBUG_TAYA_STR02,
    DEBUG_TAYA_STR03,
    DEBUG_TAYA_STR04,
    DEBUG_TAYA_STR05,
    DEBUG_TAYA_STR06,
    DEBUG_TAYA_STR07,
    DEBUG_TAYA_STR08,
    DEBUG_TAYA_STR09,
  };


  enum {
    RAND_MAX_NUM = 100,
    RAND_TEST_UNIT = 10000,
    RAND_TEST_COUNT = RAND_MAX_NUM * RAND_TEST_UNIT,
    RAND_TEST_DOWN_VAL = RAND_MAX_NUM/10,
    RAND_TEST_UP_VAL = RAND_MAX_NUM - RAND_TEST_DOWN_VAL,
  };

  MAIN_WORK* wk = mywk;

  static const u8 letterCol[3] = {
    0x05, 0x09, 0x0d
  };

  switch( *seq ){
  case 0:
    {
      int i;
      PRINTSYS_LSB  color;

      GFL_BMP_Clear( wk->bmp, 0x0f );

      for(i=0; i<NELEMS(strID); ++i)
      {
        GFL_MSG_GetString( wk->mm, strID[i], wk->strbuf );
        color = PRINTSYS_LSB_Make( letterCol[i%3], 0x0f, 0x0f );
        PRINTSYS_PrintQueColor( wk->printQue, wk->bmp, 4, 4+i*12+((i/8)*16), wk->strbuf, wk->fontHandle, color );
      }
      (*seq)++;
    }
    break;
  case 1:
    if( PRINTSYS_QUE_Main( wk->printQue ) )
    {
      TAYA_Printf("QUE処理しおわったからVRAM転送します\n");
      GFL_BMPWIN_TransVramCharacter( wk->win );
      (*seq)++;
    }
    break;
  }
  return FALSE;
}


//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA   DebugTayaMainProcData = {
  DebugTayaMainProcInit,
  DebugTayaMainProcMain,
  DebugTayaMainProcEnd,
};


//-----------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------

enum {
  TEST_GGID       = 0x3594,
  TEST_COMM_MEMBER_MAX  = 2,
  TEST_MULTI_MEMBER_MAX = 4,
  TEST_COMM_SEND_SIZE_MAX = 100,
  TEST_COMM_BCON_MAX    = 1,

  TEST_MULTI_BCON_MAX = 4,

  TEST_TIMINGID_INIT    = 11,
  TEST_TIMINGID_PRINT,

};

static const NetRecvFuncTable  testPacketTbl[] = {
//    { testPacketFunc, GFL_NET_COMMAND_SIZE(sizeof(TEST_PACKET)), NULL },
    { testPacketFunc, NULL },
};

typedef struct{
    int gameNo;   ///< ゲーム種類
}TEST_BCON;

static TEST_BCON testBcon = { WB_NET_SERVICEID_DEBUG_BATTLE };



static const GFLNetInitializeStruct testNetInitParam = {
  testPacketTbl,        // 受信関数テーブル
  NELEMS(testPacketTbl),    // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL,           // ユーザー同士が交換するデータのポインタ取得関数
  NULL,           // ユーザー同士が交換するデータのサイズ取得関数
  testBeaconGetFunc,      // ビーコンデータ取得関数
  testBeaconGetSizeFunc,    // ビーコンデータサイズ取得関数
  testBeaconCompFunc,     // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,      // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,           // 通信切断時に呼ばれる関数
  NULL,           // オート接続で親になった場合
#if GFL_NET_WIFI
  NULL, ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL, ///< wifiフレンドリスト削除コールバック
  NULL, ///< DWC形式の友達リスト
  NULL, ///< DWCのユーザデータ（自分のデータ）
  0,    ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
  TEST_GGID,          // ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,       //元になるheapid
  HEAPID_NETWORK,       //通信用にcreateされるHEAPID
  HEAPID_WIFI,        //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,       //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,       // 通信アイコンXY位置
  GFL_WICON_POSY,
  TEST_COMM_MEMBER_MAX,   // 最大接続人数
  TEST_COMM_SEND_SIZE_MAX,  // １フレームあたりの最大送信バイト数
  TEST_COMM_BCON_MAX,     // 最大ビーコン収集数
  TRUE,           // CRC計算
  FALSE,            // MP通信＝親子型通信モードかどうか
  FALSE,            // wifi通信を行うかどうか
  TRUE,           // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_SERVICEID_DEBUG_BATTLE,//GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
  0,//dummy
};

static void testPacketFunc( const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle )
{
  MAIN_WORK* wk = pWork;
  const TEST_PACKET* packet = pData;

  wk->netTestSeq = 1;
  wk->packet = *packet;

  /*
  SAMPLE_WORK* psw = sampleWork;

  if(GetSampleNetID() != netID){
      GFL_STD_MemCopy(pData, &psw->recvWork, sizeof(VecFx32));
  }
  */
}

///< ビーコンデータ取得関数
static void* testBeaconGetFunc( void* pWork )
{
  return &testBcon;
}
///< ビーコンデータサイズ取得関数
static int testBeaconGetSizeFunc( void* pWork )
{
  return sizeof(testBcon);
}

///< ビーコンデータ比較関数
static BOOL testBeaconCompFunc( GameServiceID myNo, GameServiceID beaconNo )
{
    if(myNo != beaconNo ){
        return FALSE;
    }
    return TRUE;
}


static void testCallBack(void* pWork)
{
}

static void autoConnectCallBack( void* pWork )
{
  MAIN_WORK* wk = pWork;
  wk->netTestSeq = 1;
}
//------------------------------------------------------------------------------------------------------
// LOCAL PROC 終了待ち
//------------------------------------------------------------------------------------------------------
static BOOL WaitLocalProcDone( GFL_PROCSYS* localProc )
{
  if( GFL_PROC_LOCAL_Main(localProc) != GFL_PROC_MAIN_VALID ){
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------------------------------------------
// ポケモン作成画面へ
//------------------------------------------------------------------------------------------------------
#include "debug\debug_makepoke.h"
#include "poke_tool\monsno_def.h"

static BOOL SUBPROC_MakePokeTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
FS_EXTERN_OVERLAY(debug_makepoke);

  MAIN_WORK* wk = mywk;

  switch( *seq ){
  case 0:
    changeScene_start( wk );
    (*seq)++;
    break;
  case 1:
    {
      PROCPARAM_DEBUG_MAKEPOKE* para = getGenericWork( wk, sizeof(PROCPARAM_DEBUG_MAKEPOKE) );
      para->dst = wk->testPoke;
      GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(debug_makepoke), &ProcData_DebugMakePoke, para );
      (*seq)++;
    }
    break;
  case 2:
    if( WaitLocalProcDone(wk->localProc) ){
      wk->testPokeEditFlag = TRUE;
      changeScene_recover( wk );
    }
    return TRUE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------------------------------
// バトル画面へ
//------------------------------------------------------------------------------------------------------

FS_EXTERN_OVERLAY(battle);

#include "battle\battle.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazano_def.h"
#include "test\performance.h"
#include "savedata\config.h"

static void set_test_playername( MYSTATUS* status )
{
  static const STRCODE name[] = { 0x307e, 0x3055, 0x304a, 0xffff };
  MyStatus_SetMyName( status, name );
}

//----------------------------------
// スタンドアロン
//----------------------------------
static BOOL SUBPROC_GoBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAIN_WORK* wk = mywk;

  switch( *seq ){
  case 0:

    GFL_STD_MtRandInit(0);
    {
      // メッセージ速度を最速に
      CONFIG* cfg = SaveData_GetConfig( SaveControl_GetPointer() );
      CONFIG_SetMsgSpeed( cfg, MSGSPEED_FAST );
    }

    changeScene_start( wk );
    (*seq)++;
    break;
  case 1:
    {
      BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );

      wk->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE_LOW );  ///< プレイヤーのパーティ
      wk->partyEnemy  = PokeParty_AllocPartyWork( HEAPID_CORE_LOW );  ///< 対戦相手パーティ

      #ifdef DEBUG_ONLY_FOR_taya
        setup_party( HEAPID_CORE, wk->partyPlayer, MONSNO_GYARADOSU,   MONSNO_PIKATYUU, 0 );
//        , MONSNO_GURAADON, MONSNO_KAIOOGA,
//                  MONSNO_RAITYUU, MONSNO_KEKKINGU, 0 );
        setup_party( HEAPID_CORE, wk->partyEnemy, MONSNO_MANYUURA,  MONSNO_AABOKKU, 0 );//, MONSNO_YADOKINGU, MONSNO_REKKUUZA, 0 );
        {
          POKEMON_PARAM* pp = PokeParty_GetMemberPointer( wk->partyPlayer, 0 );
          PP_Put( pp, ID_PARA_exp, 150 );
          PP_Put( pp, ID_PARA_item, ITEM_TABENOKOSI );
          PP_SetWazaPos( pp, WAZANO_NAKIGOE, 1 );
          PP_Renew( pp );

          pp = PokeParty_GetMemberPointer( wk->partyEnemy, 0 );
          PP_SetWazaPos( pp, WAZANO_KINOKONOHOUSI, 0 );
          PP_SetWazaPos( pp, WAZANO_DOKUNOKONA, 1 );
          PP_SetWazaPos( pp, 0, 2 );
          PP_SetWazaPos( pp, 0, 3 );
        }
      #else
        setup_party( HEAPID_CORE, wk->partyPlayer, MONSNO_ARUSEUSU+2, MONSNO_ARUSEUSU+1, MONSNO_PERIPPAA, 0 );
        setup_party( HEAPID_CORE, wk->partyEnemy, MONSNO_ARUSEUSU+1, MONSNO_ARUSEUSU+2, MONSNO_IWAAKU,   0 );
      #endif

      {
        u16 key = GFL_UI_KEY_GetCont();
        BTL_FIELD_SITUATION sit;

        BTL_FIELD_SITUATION_Init(&sit);

        if( key & PAD_BUTTON_L ){
          sit.bgType = BATTLE_BG_TYPE_ROOM;
          sit.bgType = BATTLE_BG_ATTR_NORMAL_GROUND;
          BTL_SETUP_Single_Trainer( para, wk->gameData, &sit, 2, HEAPID_CORE );
          BATTLE_PARAM_SetPokeParty( para, wk->partyEnemy, BTL_CLIENT_ENEMY1 );
        }else if( key & PAD_BUTTON_R ){
          sit.bgType = BATTLE_BG_TYPE_ROOM;
          sit.bgAttr = BATTLE_BG_ATTR_NORMAL_GROUND;
          BTL_SETUP_Double_Trainer( para, wk->gameData, &sit, 2, HEAPID_CORE );
          BATTLE_PARAM_SetPokeParty( para, wk->partyEnemy, BTL_CLIENT_ENEMY1 );
        }else{
//          BTL_SETUP_Triple_Trainer( para, wk->gameData, wk->partyEnemy, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE, 2 );
//          setup_party( HEAPID_CORE, wk->partyEnemy, MONSNO_MANYUURA, 0 );
//          BTL_SETUP_Single_Wild( para, wk->gameData, wk->partyEnemy, BTL_LANDFORM_ROOM, BTL_WEATHER_NONE );
          sit.bgType = BATTLE_BG_TYPE_GRASS;
          sit.bgAttr = BATTLE_BG_ATTR_GRASS;
          BTL_SETUP_Wild( para, wk->gameData, wk->partyEnemy, &sit, BTL_RULE_SINGLE, HEAPID_CORE );
        }

        MYITEM_AddItem( para->itemData, ITEM_MASUTAABOORU, 1, HEAPID_CORE );
        MYITEM_AddItem( para->itemData, ITEM_HAIPAABOORU, 4, HEAPID_CORE );
        MYITEM_AddItem( para->itemData, ITEM_KIZUGUSURI, 4, HEAPID_CORE );

        set_test_playername( (MYSTATUS*)(para->playerStatus[BTL_CLIENT_PLAYER]) );
      }

      if( wk->testPokeEditFlag )
      {
        PokeParty_SetMemberData( wk->partyPlayer, 0, wk->testPoke );
      }
      BATTLE_PARAM_SetPokeParty( para, wk->partyPlayer, BTL_CLIENT_PLAYER );

      GFL_HEAP_FreeMemory( wk->partyEnemy );
      GFL_HEAP_FreeMemory( wk->partyPlayer );

      PMSND_PlayBGM( para->musicDefault );
      GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(battle), &BtlProcData, para );

      (*seq)++;
    }
    break;
  case 2:
    if( WaitLocalProcDone(wk->localProc) )
    {
      BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );
      GFL_HEAP_FreeMemory( para->party[BTL_CLIENT_PLAYER] );
      GFL_HEAP_FreeMemory( para->party[BTL_CLIENT_ENEMY1] );
      {
        BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );
        switch( para->result ){
        case BTL_RESULT_WIN:   TAYA_Printf("勝ったよ\n"); break;
        case BTL_RESULT_LOSE:  TAYA_Printf("負けたよ\n"); break;
        case BTL_RESULT_RUN:   TAYA_Printf("逃げたよ\n"); break;
        default:
          TAYA_Printf("不明な終了コード %d\n", para->result);
        }
      }
    }
    (*seq)++;
    break;
  case 3:
    changeScene_recover( wk );
    return TRUE;
  }

  return FALSE;

}

// バトル用受信関数テーブル
extern const NetRecvFuncTable BtlRecvFuncTable[];

typedef struct{
    int gameNo;   ///< ゲーム種類
}BTL_BCON;

static BTL_BCON btlBcon = { WB_NET_BATTLE_SERVICEID };
static BTL_BCON MultiBcon = { WB_NET_BATTLE_SERVICEID };

///< ビーコンデータ取得関数
static void* btlBeaconGetFunc( void* pWork )
{
  return &btlBcon;
}
///< ビーコンデータサイズ取得関数
static int btlBeaconGetSizeFunc( void* pWork )
{
  return sizeof(btlBcon);
}

///< ビーコンデータ比較関数
static BOOL btlBeaconCompFunc( GameServiceID myNo, GameServiceID beaconNo )
{
    if( myNo != beaconNo ){
        return FALSE;
    }
    return TRUE;
}

static const GFLNetInitializeStruct btlNetInitParam = {
  BtlRecvFuncTable,     // 受信関数テーブル
  BTL_NETFUNCTBL_ELEMS, // 受信テーブル要素数
  NULL,                 // ハードで接続した時に呼ばれる
  NULL,                 // ネゴシエーション完了時にコール
  NULL,                 // ユーザー同士が交換するデータのポインタ取得関数
  NULL,                 // ユーザー同士が交換するデータのサイズ取得関数
  btlBeaconGetFunc,     // ビーコンデータ取得関数
  btlBeaconGetSizeFunc, // ビーコンデータサイズ取得関数
  btlBeaconCompFunc,    // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,                 // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,      // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,                 // 通信切断時に呼ばれる関数
  NULL,                 // オート接続で親になった場合
#if GFL_NET_WIFI
  NULL, ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL, ///< wifiフレンドリスト削除コールバック
  NULL, ///< DWC形式の友達リスト
  NULL, ///< DWCのユーザデータ（自分のデータ）
  0,    ///< DWCへのHEAPサイズ
  TRUE, ///< デバック用サーバにつなぐかどうか
#endif
  TEST_GGID,            // ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,       //元になるheapid
  HEAPID_NETWORK,       //通信用にcreateされるHEAPID
  HEAPID_WIFI,          //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,       //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,       // 通信アイコンXY位置
  GFL_WICON_POSY,
  TEST_COMM_MEMBER_MAX,   // 最大接続人数
  TEST_COMM_SEND_SIZE_MAX,// 最大送信バイト数
  TEST_COMM_BCON_MAX,     // 最大ビーコン収集数
  TRUE,                   // CRC計算
  FALSE,                  // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,  /// 使用する通信を指定
  TRUE,                   // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_BATTLE_SERVICEID,//GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
  0,//dummy
};

static const GFLNetInitializeStruct btlMultiNetInitParam = {
  BtlRecvFuncTable,     ///< 受信関数テーブル
  BTL_NETFUNCTBL_ELEMS, ///< 受信テーブル要素数
  NULL,                 ///< ハードで接続した時に呼ばれる
  NULL,                 ///< ネゴシエーション完了時にコール
  NULL,                 // ユーザー同士が交換するデータのポインタ取得関数
  NULL,                 // ユーザー同士が交換するデータのサイズ取得関数
  btlBeaconGetFunc,     // ビーコンデータ取得関数
  btlBeaconGetSizeFunc, // ビーコンデータサイズ取得関数
  btlBeaconCompFunc,    // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,                 // 普通のエラーが起こった場合 通信終了
  FatalError_Disp,      // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,           // 通信切断時に呼ばれる関数
  NULL,           // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif
  TEST_GGID,            // ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,       //元になるheapid
  HEAPID_NETWORK,       //通信用にcreateされるHEAPID
  HEAPID_WIFI,          //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,       //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,       // 通信アイコンXY位置
  GFL_WICON_POSY,
  TEST_MULTI_MEMBER_MAX,    // 最大接続人数
  TEST_COMM_SEND_SIZE_MAX,  // 最大送信バイト数
  TEST_MULTI_BCON_MAX,      // 最大ビーコン収集数
  TRUE,                     // CRC計算
  FALSE,                    // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIRELESS,    // 使用する通信を指定
  TRUE,                     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_BATTLE_SERVICEID,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
  0,//dummy
};

static void btlAutoConnectCallback( void* pWork )
{
  MAIN_WORK* wk = pWork;

  wk->netTestSeq = 1;
  TAYA_Printf("GFL_NET AutoConnCallBack\n");
}


//----------------------------------
// 通信（通常）
//----------------------------------
static BOOL SUBPROC_CommBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAIN_WORK* wk = mywk;

  switch( *seq ){
  case 0:
    changeScene_start( wk );
    (*seq)++;
    break;
  case 1:
    GFL_NET_Init( &btlNetInitParam, testCallBack, (void*)wk );
    (*seq)++;
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      TAYA_Printf("GFL_NET Initi wait ...\n");
    }
    if( GFL_NET_IsInit() )
    {
      GFL_NET_ChangeoverConnect( btlAutoConnectCallback ); // 自動接続
      (*seq)++;
    }
    break;
  case 3:
    if( wk->netTestSeq )
    {
      GFL_NET_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), 0 );
      (*seq)++;
    }
    break;
  case 4:
    if( GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), 0) )
    {
      BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );
      GFL_NETHANDLE* netHandle = GFL_NET_HANDLE_GetCurrentHandle();


      wk->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE_LOW );
      if( GFL_NET_GetNetID(netHandle) == 0 ){
//        setup_party( HEAPID_CORE, wk->partyPlayer, MONSNO_PORIGON, MONSNO_PIKATYUU, MONSNO_RIZAADON, 0 );
        setup_party( HEAPID_CORE, wk->partyPlayer, MONSNO_NOZUPASU,   MONSNO_PIKATYUU, MONSNO_GURAADON, MONSNO_KAIOOGA,
                  MONSNO_RAITYUU, MONSNO_KEKKINGU, 0 );
      }else{
        setup_party( HEAPID_CORE, wk->partyPlayer, MONSNO_YADOKINGU, MONSNO_METAGUROSU, MONSNO_SUTAAMII,
                  MONSNO_ENEKO, MONSNO_NYAASU, 0 );
      }

      BTL_SETUP_Single_Comm( para, wk->gameData, netHandle,  BTL_COMM_DS, HEAPID_CORE );

      BATTLE_PARAM_SetPokeParty( para, wk->partyPlayer, BTL_CLIENT_PLAYER );
      GFL_HEAP_FreeMemory( wk->partyPlayer );

      set_test_playername( (MYSTATUS*)(para->playerStatus[BTL_CLIENT_PLAYER]) );

      DEBUG_PerformanceSetActive( FALSE );
      GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(battle), &BtlProcData, para );
      (*seq)++;
    }
    break;
  case 5:
    if( WaitLocalProcDone(wk->localProc) ){
      BATTLE_PARAM_Release( (BATTLE_SETUP_PARAM*)getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) ) );
      changeScene_recover( wk );
      return TRUE;
    }
  }

  return FALSE;
}
//----------------------------------
// 通信（マルチ）
//----------------------------------
static BOOL SUBPROC_MultiBattle( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  enum {
    MULTI_TIMING_NUMBER = 24,
  };
  MAIN_WORK* wk = mywk;

  switch( *seq ){
  case 0:
    changeScene_start( wk );
    (*seq)++;
    break;
  case 1:
    GFL_NET_Init( &btlMultiNetInitParam, testCallBack, (void*)wk );
    (*seq)++;
    break;
  case 2:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
    {
      TAYA_Printf("GFL_NET Initi wait ...\n");
    }
    if( GFL_NET_IsInit() )
    {
      (*seq)++;
    }
    break;
  case 3:
    // subArg!=0 -> 親機に
    if( wk->subArg )
    {
      TAYA_Printf("[D_TAYA] マルチ親機になります\n");
      GFL_NET_InitServer();
      (*seq)++;
    }
    // 子機ならビーコンをひろいまくる
    else
    {
      GFL_NET_StartBeaconScan();
      (*seq)++;
    }
    break;
  case 4:
    if( wk->subArg )
    {
      (*seq)++;
    }
    else
    {
      int i;
      BTL_BCON* bcon;

      TAYA_Printf("[D_TAYA] 子機なのでビーコンひろいます\n");
      for(i=0; i<TEST_MULTI_BCON_MAX; ++i)
      {
        bcon = GFL_NET_GetBeaconData(i);
        if( bcon )
        {
          if( bcon->gameNo == MultiBcon.gameNo )
          {
            break;
          }
        }
      }
      if( i != TEST_MULTI_BCON_MAX )
      {
        u8* macAdrs = GFL_NET_GetBeaconMacAddress( i );
        if( macAdrs != NULL )
        {
          TAYA_Printf("[D_TAYA] 子機がマルチ親機を見つけた! macAdrs=");
          {
            u8 x;
            for(x=0; x<8; ++x)
            {
              TAYA_Printf("%02x.", macAdrs[0]);
            }
            TAYA_Printf("\n");
          }
          GFL_NET_ConnectToParent( macAdrs );
          (*seq)++;
        }
      }
    }
    break;

  case 5:
    if( GFL_NET_HANDLE_RequestNegotiation() )
    {
      TAYA_Printf("[D_TAYA] ネゴシエーション成功\n");
    }
    if( GFL_NET_GetConnectNum() == TEST_MULTI_MEMBER_MAX )
    {
      TAYA_Printf("[D_TAYA] %d台そろったので次へ \n", TEST_MULTI_MEMBER_MAX);
      (*seq)++;
    }
    break;

  // ４台つながったらシンクロ開始
  case 6:
    if( GFL_NET_GetConnectNum() == TEST_MULTI_MEMBER_MAX )
    {
      GFL_NET_TimingSyncStart( GFL_NET_HANDLE_GetCurrentHandle(), MULTI_TIMING_NUMBER );
      TAYA_Printf("[D_TAYA] マルチシンクロ開始します ... \n");
      (*seq)++;
    }
    else
    {
      if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_L )
      {
        u8 n = GFL_NET_GetConnectNum();
        NetID netID = GFL_NET_GetNetID( GFL_NET_HANDLE_GetCurrentHandle() );
        TAYA_Printf("SubArg=%d, conNum=%d, myNetID=%d : ", wk->subArg, n, netID);
        for(netID=0; netID<TEST_MULTI_MEMBER_MAX; ++netID)
        {
          if( GFL_NET_IsConnectMember(netID) )
          {
            TAYA_Printf("%d,", netID);
          }
        }
        TAYA_Printf("\n");
      }
    }
    break;

  case 7:
    if( GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(), MULTI_TIMING_NUMBER) )
    {
      BATTLE_SETUP_PARAM* para = getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) );
      GFL_NETHANDLE* netHandle = GFL_NET_HANDLE_GetCurrentHandle();
      TAYA_Printf("[DTAYA] Multi Timing Sync Finish! NetHandle=%p\n", netHandle );

      BTL_SETUP_Multi_Comm( para, wk->gameData,
        netHandle, BTL_COMM_DS, 0, HEAPID_CORE );
/*
      para->rule = BTL_RULE_DOUBLE;
      para->competitor = BTL_COMPETITOR_COMM;

      para->netHandle = GFL_NET_HANDLE_GetCurrentHandle();
      para->netID = GFL_NET_GetNetID( para->netHandle );
      para->commMode = BTL_COMM_DS;
      para->multiMode = 1;

      para->partyPlayer = PokeParty_AllocPartyWork( HEAPID_CORE );  ///< プレイヤーのパーティ
      para->partyEnemy1 = NULL;   ///< 1vs1時の敵AI, 2vs2時の１番目敵AI用
      para->partyPartner = NULL;  ///< 2vs2時の味方AI（不要ならnull）
      para->partyEnemy2 = NULL;   ///< 2vs2時の２番目敵AI用（不要ならnull）
*/
      switch( GFL_NET_GetNetID(netHandle) ){
      case 0:
        setup_party( HEAPID_CORE, para->party[BTL_CLIENT_PLAYER], MONSNO_GYARADOSU, MONSNO_PIKATYUU, MONSNO_RIZAADON, 0 );
        break;
      case 1:
        setup_party( HEAPID_CORE, para->party[BTL_CLIENT_PLAYER], MONSNO_YADOKINGU, MONSNO_METAGUROSU, MONSNO_SUTAAMII, 0 );
        break;
      case 2:
        setup_party( HEAPID_CORE, para->party[BTL_CLIENT_PLAYER], MONSNO_BAKUUDA, MONSNO_DONKARASU, MONSNO_SANDAASU, 0 );
        break;
      case 3:
        setup_party( HEAPID_CORE, para->party[BTL_CLIENT_PLAYER], MONSNO_HERAKUROSU, MONSNO_GENGAA, MONSNO_EAAMUDO, 0 );
        break;
      }

      DEBUG_PerformanceSetActive( FALSE );
      GFL_PROC_LOCAL_CallProc( wk->localProc, FS_OVERLAY_ID(battle), &BtlProcData, para );
      (*seq)++;
    }
    break;

  case 8:
    if( WaitLocalProcDone(wk->localProc) ){
      BATTLE_PARAM_Release( (BATTLE_SETUP_PARAM*)getGenericWork( wk, sizeof(BATTLE_SETUP_PARAM) ) );
      changeScene_recover( wk );
      return TRUE;
    }
  }

  return FALSE;
}


static void setup_party( HEAPID heapID, POKEPARTY* party, ... )
{
  va_list  list;
  int monsno;
  POKEMON_PARAM* pp;

  PokeParty_InitWork( party );

  va_start( list, party );
  while( 1 )
  {
    monsno = va_arg( list, int );
    if( monsno )
    {
      TAYA_Printf("Create MonsNo=%d\n", monsno);
      pp = PP_Create( monsno, 50, 3594, GFL_HEAP_LOWID(heapID) );
      if( monsno == MONSNO_PORIGON )
      {
        PP_SetWazaPush( pp, WAZANO_TEKUSUTYAA );
        PP_SetWazaPush( pp, WAZANO_NAMINORI );
        PP_SetWazaPush( pp, WAZANO_KAMINARI );
        PP_SetWazaPush( pp, WAZANO_HUNKA );
      }
      PokeParty_Add( party, pp );
      GFL_HEAP_FreeMemory( pp );
    }
    else
    {
      break;
    }
  }
  va_end( list );
}

//------------------------------------------------------------------------------------------------------
// 漢字モード切り替え
//------------------------------------------------------------------------------------------------------
static BOOL SUBPROC_KanjiMode( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAIN_WORK* wk = mywk;

  GFL_MSGSYS_SetLangID( !GFL_MSGSYS_GetLangID() );
  print_menu( wk, &wk->menuCtrl );
  return TRUE;
}

//------------------------------------------------------------------------------------------------------
// 通信状態での漢字PrintTest
//------------------------------------------------------------------------------------------------------
static BOOL SUBPROC_NetPrintTest( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  static const u16 strID[] = {
    DEBUG_TAYA_STR01,
    DEBUG_TAYA_STR02,
    DEBUG_TAYA_STR03,
    DEBUG_TAYA_STR04,
    DEBUG_TAYA_STR05,
    DEBUG_TAYA_STR06,
    DEBUG_TAYA_STR07,
    DEBUG_TAYA_STR08,
    DEBUG_TAYA_STR09,
  };

  MAIN_WORK* wk = mywk;

  GFL_TCBL_Main( wk->tcbl );

  if( !PRINT_UTIL_Trans(wk->printUtil, wk->printQue) )
  {
    return FALSE;
  }

  switch( *seq ){
  case 0:
    GFL_MSG_GetString( wk->mm, DEBUG_TAYA_WAIT, wk->strbuf );
    GFL_BMP_Clear( wk->bmp, 0xff );
    PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 0, wk->strbuf, wk->fontHandle );
    wk->netInitWork = testNetInitParam;
    wk->netTestSeq = 0;
    wk->kanjiMode = 0;
    wk->packet.kanjiMode = 0;
    GFL_NET_Init(&(wk->netInitWork), testCallBack, (void*)wk);
    (*seq)++;
    break;

  case 1:
    if( GFL_NET_IsInit() )
    {
      GFL_NET_ChangeoverConnect( autoConnectCallBack ); // 自動接続
      (*seq)++;
    }
    break;

  case 2:
    if( wk->netTestSeq )
    {
      wk->netTestSeq = 0;
      wk->netHandle = GFL_NET_HANDLE_GetCurrentHandle();
      GFL_NET_TimingSyncStart( wk->netHandle, TEST_TIMINGID_INIT );
      (*seq)++;
    }
    break;

  case 3:
    if( GFL_NET_IsTimingSync(wk->netHandle, TEST_TIMINGID_INIT) )
    {
      wk->ImParent = GFL_NET_IsParentMachine();
      (*seq)++;
    }
    break;

  case 4:
    GFL_BMP_Clear( wk->bmp, 0xff );
    GFL_MSG_GetString(  wk->mm, (wk->ImParent)? DEBUG_TAYA_STR_PARENT : DEBUG_TAYA_STR_CHILD, wk->strbuf );
    PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 0, wk->strbuf, wk->fontHandle );
//    GFL_BMPWIN_TransVramCharacter( wk->win );
    wk->packet.strNum = 0;
    wk->packet.yofs = 30;
    (*seq)++;
    break;

  case 5:
    if( wk->ImParent )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        GFL_NET_SendData( wk->netHandle, GFL_NET_CMD_DEBUG_BATTLE, sizeof(TEST_PACKET), &(wk->packet) );
        (*seq)++;
        break;
      }
      else
      {
        GFL_BMP_Fill( wk->bmp, 0, 0, 256, 16, 0xff );
        PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 0, wk->strbuf, wk->fontHandle );
        PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 16, wk->strbuf, wk->fontHandle );
      }
      #if 0
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
      {
        wk->netTestSeq = 0;
        GFL_NET_Exit( autoConnectCallBack );
        (*seq) = 100;
        break;
      }
      #endif
    }
    else
    {
      (*seq)++;
    }
    break;

  case 6:
    if( wk->netTestSeq )
    {
      wk->netTestSeq = 0;
      GFL_NET_TimingSyncStart( wk->netHandle, TEST_TIMINGID_PRINT );
      (*seq)++;
    }
    else
    {
      GFL_BMP_Fill( wk->bmp, 0, 0, 256, 16, 0xff );
      PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 0, wk->strbuf, wk->fontHandle );
      PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 16, wk->strbuf, wk->fontHandle );
    }
    break;

  case 7:
    if( GFL_NET_IsTimingSync(wk->netHandle, TEST_TIMINGID_PRINT) )
    {
      GFL_MSG_GetString( wk->mm, strID[wk->packet.strNum], wk->strbuf );
      wk->printStream = PRINTSYS_PrintStream( wk->win, 0, wk->packet.yofs,
              wk->strbuf, wk->fontHandle, 0, wk->tcbl, 0, wk->heapID, 0xff );
      (*seq)++;
    }
    break;

  case 8:
    if( PRINTSYS_PrintStreamGetState(wk->printStream) == PRINTSTREAM_STATE_DONE )
    {
      PRINTSYS_PrintStreamDelete( wk->printStream );
      (*seq)++;
    }
    break;

  case 9:
    if( wk->ImParent )
    {
      if( ++(wk->packet.strNum) <= NELEMS(strID) )
      {
        wk->packet.yofs += 16;
        if( strID[wk->packet.strNum] == DEBUG_TAYA_STR09 )
        {
          wk->packet.yofs += 16;
        }
      }
      else
      {
        wk->packet.kanjiMode = !(wk->packet.kanjiMode);
      }
      GFL_NET_SendData( wk->netHandle, GFL_NET_CMD_DEBUG_BATTLE,sizeof(TEST_PACKET), &(wk->packet) );
    }
    (*seq)++;
    break;

  case 10:
    if( wk->netTestSeq )
    {
      wk->netTestSeq = 0;

      if( wk->kanjiMode != wk->packet.kanjiMode )
      {
        wk->kanjiMode = wk->packet.kanjiMode;
        GFL_MSGSYS_SetLangID( wk->kanjiMode );

        GFL_BMP_Clear( wk->bmp, 0xff );
        GFL_BMPWIN_TransVramCharacter( wk->win );

        (*seq)++;
      }
      else
      {
        (*seq) = 5;
      }
    }
    break;

  case 11:
    GFL_MSG_GetString( wk->mm, DEBUG_TAYA_STR10, wk->strbuf );
    PRINT_UTIL_Print( wk->printUtil, wk->printQue, 0, 0, wk->strbuf, wk->fontHandle );
    (*seq)++;
    break;

  case 12:
    if( wk->ImParent )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        GFL_NET_SendData( wk->netHandle, GFL_NET_CMD_DEBUG_BATTLE, sizeof(TEST_PACKET), &(wk->packet) );
        (*seq)++;
      }
      break;
    }
    (*seq)++;
    break;

  case 13:
    if( wk->netTestSeq )
    {
      wk->netTestSeq = 0;
      GFL_BMP_Clear( wk->bmp, 0xff );
      (*seq)=4;
    }
    break;

  case 100:
    if( wk->netTestSeq )
    {
      wk->netTestSeq = 0;
      (*seq)++;
    }
    break;

  case 101:
    return TRUE;

  }

    return FALSE;
}

