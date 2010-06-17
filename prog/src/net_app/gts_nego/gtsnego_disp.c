//============================================================================================
/**
 * @file    gtsnego_disp.c
 * @bfief   GTSネゴシエーション画像表示関連
 * @author  k.ohno
 * @date    2009.11.14
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>

#include "arc_def.h"
#include "net_app/gsync.h"

#include "debug/debugwin_sys.h"

#include "infowin/infowin.h"

#include "system/main.h"
#include "system/wipe.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "system/blink_palanm.h"

#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait
#include "system/palanm.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "msg/msg_gtsnego.h"

#include "sound/pm_sndsys.h"
#include "ui/touchbar.h"

#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK

#include "../../field/event_ircbattle.h"

#include "gtsnego_local.h"
#include "gtsnego.naix"
#include "wifi_unionobj_plt.cdat"
#include "wifi_unionobj.naix"

FS_EXTERN_OVERLAY(ui_common);


//-------------------------------------
///	トレーナーOBJ
//=====================================


// ユニオンキャラクター
#define WF_2DC_ARC_UNICHARIDX	( NARC_wifi_unionobj_normal00_NCGR )	// キャラクタグラフィックの開始 boy1 から normal00に置き換えました nagihashi
#define WF_2DC_ARC_GETUNINCL	(NARC_wifi_unionobj_wifi_union_obj_NCLR)	// パレット取得
#define WF_2DC_ARC_GETUNINCG(x)	(WF_2DC_ARC_UNICHARIDX + x)		// キャラクタ取得
#define WF_2DC_UNIPLTT_NUM		( 8 )	// ユニオンキャラクタが使用するパレット本数
#define WF_2DC_ARC_UNIANMIDX	( NARC_wifi_unionobj_normal00_NCER )	// ユニオンアニメデータ
#define WF_2DC_ARC_GETUNICEL(x)	(WF_2DC_ARC_UNIANMIDX+((x)*2))	// セルdataidゲット
#define WF_2DC_ARC_GETUNIANM(x)	(WF_2DC_ARC_GETUNICEL(x)+1)		// アニメdataidゲット


#define UNION_TRAINER_NUM  (16)


#define SEARCH_ANIM_START  (11)
#define SEARCH_ANIM_END  (26)


typedef enum
{
  PLT_NEGOOBJ,
  PLT_NEGOOBJ_MAIN,
  PLT_UNION,  
  PLT_MENUBAR,
  PLT_RESOURCE_MAX,
  CHAR_NEGOOBJ = PLT_RESOURCE_MAX,
  CHAR_NEGOOBJ_MAIN,
  CHAR_UNION,
  CHAR_UNION1,
  CHAR_UNION2,
  CHAR_UNION3,
  CHAR_UNION4,
  CHAR_UNION5,
  CHAR_UNION6,
  CHAR_UNION7,
  CHAR_UNION8,
  CHAR_UNION9,
  CHAR_UNIONa,
  CHAR_UNIONb,
  CHAR_UNIONc,
  CHAR_UNIONd,
  CHAR_UNIONe,
  CHAR_UNIONf,
  CHAR_MENUBAR,
  CHAR_RESOURCE_MAX,
  ANM_NEGOOBJ = CHAR_RESOURCE_MAX,
  ANM_NEGOOBJ_MAIN,
  ANM_UNION,
  ANM_UNION1,
  ANM_UNION2,
  ANM_UNION3,
  ANM_UNION4,
  ANM_UNION5,
  ANM_UNION6,
  ANM_UNION7,
  ANM_UNION8,
  ANM_UNION9,
  ANM_UNIONa,
  ANM_UNIONb,
  ANM_UNIONc,
  ANM_UNIONd,
  ANM_UNIONe,
  ANM_UNIONf,
  ANM_MENUBAR,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;



#define _PALMODE_LEVEL (0)
#define _PALMODE_FRIEND (1)


typedef enum
{
  _SCROLLBAR_OAM_CHIP,
  _SCROLLBAR_OAM_BAR,
  _SCROLLBAR_OAM_TITLEP,
  _SCROLLBAR_OAM_NUM
} _SCROLLBAR_OAM_TYPE;

#define _CELL_DISP_NUM (12)

struct _GTSNEGO_DISP_WORK {
	u32 subchar;
  u32 mainchar;
  u32 backchar;
  TOUCHBAR_WORK* pTouchWork;
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル

  u32 cellRes[CEL_RESOURCE_MAX];

  GFL_CLWK* curIcon[_CELL_DISP_NUM];
  HEAPID heapID;
  BLINKPALANM_WORK* pBlink;
  BLINKPALANM_WORK* pBlinkUnder;
  BOOL bgscrollRenew;
  int bgscroll;
  int bgscrollType;
  int bgscrollPos;
//  int listmax;
  int SearchScroll;
  BOOL bSearchScroll;
  GAMEDATA* pGameData;

  NNSG2dScreenData* pBGPanelScr;
  void* pBGPanelScrAddr;
  void* pVramOBJ;
  void* pVramBG;
  
  GFL_CLWK* SearchBackOAM;
  GFL_CLWK* SearchPeopleOAM;
  GFL_CLWK* crossIcon;
  GFL_CLWK* menubarObj;
  GFL_CLWK* pTrOAM;
  GFL_CLWK* scrollbarOAM[_SCROLLBAR_OAM_NUM];
  GFL_CLWK* unionOAM[GTSNEGO_WINDOW_MAXNUM];
};


typedef struct{
  u8 no;
  u8 x;
  u8 y;
  u8 flash;
} CROSSCUR_STRUCT;




static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

  //        GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_16_I,	// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K,

};

//--------------------------------------------------------------
///	セルアクター　初期化データ
//--------------------------------------------------------------
static const GFL_CLSYS_INIT _CLSYS_Init =
{
  0, 0,
  0, 512,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL, //通信アイコン部分
  0,
  100,
  100,
  100,
  100,
  16, 16,
};


static GFL_BG_SYS_HEADER BGsys_data = {
  GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

static void dispInit(GTSNEGO_DISP_WORK* pWork);
static void settingSubBgControl(GTSNEGO_DISP_WORK* pWork);
static void _TOUCHBAR_Init(GTSNEGO_DISP_WORK* pWork, int palPos);
static void	_VBlank( GFL_TCB *tcb, void *work );
static void _SetArrow(GTSNEGO_DISP_WORK* pWork,int x,int y,BOOL bRight);
static void _ArrowRelease(GTSNEGO_DISP_WORK* pWork);
static void _DebugDataCreate(GTSNEGO_DISP_WORK* pWork);
static void _CreateCrossIcon(GTSNEGO_DISP_WORK* pWork);
static void _CreateMenuBarObj(GTSNEGO_DISP_WORK* pWork);
static void _RemoveMenuBarObj(GTSNEGO_DISP_WORK* pWork);

static void _PanelScrollMain(GTSNEGO_DISP_WORK* pWork);
static void _CreateScrollBarObj(GTSNEGO_DISP_WORK* pWork, BOOL bCursor);
static void _DeleteScrollBarObj(GTSNEGO_DISP_WORK* pWork);
static void GTSNEGO_DISP_UnionWkScroll(GTSNEGO_DISP_WORK* pWork,int workIndex, int move);
static void _paletteModeChange(GTSNEGO_DISP_WORK* pWork, int mode);
static void _BGPanelFree(GTSNEGO_DISP_WORK* pWork);



GTSNEGO_DISP_WORK* GTSNEGO_DISP_Init(HEAPID id, GAMEDATA* pGameData)
{
  GTSNEGO_DISP_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(GTSNEGO_DISP_WORK));
  pWork->heapID = id;
  pWork->pGameData = pGameData;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GXS_DispOn();
  GX_DispOn();

  GFL_BG_Init(pWork->heapID);
  GFL_CLACT_SYS_Create(	&_CLSYS_Init, &_defVBTbl, pWork->heapID );
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 64 , 0 , pWork->heapID );

  GFL_DISP_SetBank( &_defVBTbl );
  GFL_BG_SetBGMode( &BGsys_data );
  settingSubBgControl(pWork);
  dispInit(pWork);
  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );
  pWork->pVramOBJ = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ
  pWork->pVramBG = GFL_HEAP_AllocMemory(pWork->heapID, 16*2*16);  //バックアップ

  _CreateCrossIcon(pWork);
  
  _TOUCHBAR_Init(pWork, _TOUCHBAR_PAL1);

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  
  return pWork;
}

void GTSNEGO_DISP_Main(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLACT_SYS_Main();
  if(pWork->pBlink){
    BLINKPALANM_Main(pWork->pBlink);
  }
  if(pWork->pBlinkUnder){
    BLINKPALANM_Main(pWork->pBlinkUnder);
  }
  GTSNEGO_DISP_SearchPeopleDispMain(pWork);

}

void GTSNEGO_DISP_End(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  GFL_CLACT_WK_Remove(pWork->crossIcon);

  GTSNEGO_DISP_DeleteTouchWork(pWork);

  _RemoveMenuBarObj(pWork);
  _BGPanelFree(pWork);

  GFL_HEAP_FreeMemory(pWork->pVramOBJ);
  GFL_HEAP_FreeMemory(pWork->pVramBG);

  _ArrowRelease(pWork);
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_NEGOOBJ] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_NEGOOBJ_MAIN] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_NEGOOBJ] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_NEGOOBJ_MAIN] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_NEGOOBJ] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_NEGOOBJ_MAIN] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_MENUBAR] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_MENUBAR] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_MENUBAR] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_UNION] );

  for(i=0;i<UNION_TRAINER_NUM;i++){
    GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_UNION+i] );
    GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_UNION+i] );
  }

  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  BLINKPALANM_Exit(pWork->pBlink);
  BLINKPALANM_Exit(pWork->pBlinkUnder);

  GFL_BG_FreeCharacterArea(GFL_BG_FRAME3_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->backchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->backchar));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME0_S,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME0_M,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->mainchar));
  
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_M, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME2_S, 1, 0);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );

  for(i=GFL_BG_FRAME0_M;i<=GFL_BG_FRAME3_S;i++){
    GFL_BG_FreeBGControl(i);
  }
	GFL_BG_Exit();
  GFL_HEAP_FreeMemory(pWork);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
  

}

GFL_CLUNIT* GTSNEGO_DISP_GetCellUtil(GTSNEGO_DISP_WORK* pWork)
{
  return pWork->cellUnit;
}


static void settingSubBgControl(GTSNEGO_DISP_WORK* pWork)
{

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000,  0x8000, GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
  }
  {
    int frame = GFL_BG_FRAME1_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME2_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }

  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME1_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_256x512, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x8000,GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}


#if 1


static void _PaletteFadeSingle(GTSNEGO_DISP_WORK* pWork, int type, int palettenum)
{
  u32 addr;
  PALETTE_FADE_PTR pP = PaletteFadeInit(pWork->heapID);
  PaletteFadeWorkAllocSet(pP, type, 16 * 32, pWork->heapID);
  PaletteWorkSet_VramCopy( pP, type, 0, palettenum*32);
  SoftFadePfd(pP, type, 0, 16 * palettenum, 6, 0);
  addr = (u32)PaletteWorkTransWorkGet( pP, type );

  switch(type){
  case FADE_SUB_OBJ:
    GXS_LoadOBJPltt((void*)addr, 0, palettenum * 32);
    break;
  case FADE_SUB_BG:
    GXS_LoadBGPltt((void*)addr, 0, palettenum * 32);
    break;
  }
  PaletteFadeWorkAllocFree(pP,type);
  PaletteFadeFree(pP);
}

static void _PaletteFadeSingle2(GTSNEGO_DISP_WORK* pWork, int type, int* start, int* palettenum, int num)
{
  u32 addr;
  int i;
  PALETTE_FADE_PTR pP = PaletteFadeInit(pWork->heapID);
  PaletteFadeWorkAllocSet(pP, type, 16 * 32, pWork->heapID);
  PaletteWorkSet_VramCopy( pP, type, 0, 16*32);
  SoftFadePfd(pP, type,  0, 16 * 16, 6, 0);
  addr = (u32)PaletteWorkTransWorkGet( pP, type );

  for(i=0;i<num;i++){
    switch(type){
    case FADE_SUB_OBJ:
      DC_FlushRange( (void*)(addr+32*start[i]), palettenum[i] * 32 );
      GXS_LoadOBJPltt((void*)(addr+32*start[i]), start[i]*16*2, palettenum[i] * 32);
      break;
    case FADE_SUB_BG:
      DC_FlushRange( (void*)(addr+32*start[i]), palettenum[i] * 32 );
      GXS_LoadBGPltt((void*)(addr+32*start[i]),  start[i]*16*2, palettenum[i] * 32);
      break;
    }
  }
  PaletteFadeWorkAllocFree(pP,type);
  PaletteFadeFree(pP);
}



void GTSNEGO_DISP_PaletteFade(GTSNEGO_DISP_WORK* pWork,BOOL bFade, int palette)
{
  u32 addr;

  if(bFade){
    {
      GFL_STD_MemCopy((void*)HW_DB_OBJ_PLTT, pWork->pVramOBJ, 16*2*16);
      GFL_STD_MemCopy((void*)HW_DB_BG_PLTT, pWork->pVramBG, 16*2*16);

      switch(palette){
      case _PALETTEFADE_PATTERN3:
        {
          int stb[]={ 0};
          int paln[]={16};
          _PaletteFadeSingle2( pWork,  FADE_SUB_OBJ, stb, paln, elementof(stb));
        }
        break;
      case _PALETTEFADE_PATTERN1:
        {
          int stb[]={ 1};
          int paln[]={15};
          _PaletteFadeSingle2( pWork,  FADE_SUB_OBJ, stb, paln, 1);
        }
        break;
      case _PALETTEFADE_PATTERN2:
        {
          int stb[]={ 0,11};
          int paln[]={10,5};
          _PaletteFadeSingle2( pWork,  FADE_SUB_OBJ, stb, paln, 2);
        }
        break;
      }

      switch(palette){
      case _PALETTEFADE_PATTERN3:
      case _PALETTEFADE_PATTERN1:
        {
          int stb[]={  0,11};
          int paln[]={ 9, 4};
          _PaletteFadeSingle2( pWork,  FADE_SUB_BG, stb, paln, 2);
        }
        break;
      case _PALETTEFADE_PATTERN2:
        {
          int stb[]={  0,11};
          int paln[]={ 9, 5};
          _PaletteFadeSingle2( pWork,  FADE_SUB_BG, stb, paln, 2);
        }
        break;
      }
    }
  }
  else{
    DC_FlushRange( (void*)pWork->pVramOBJ, 16 * 32 );
    GXS_LoadOBJPltt((void*)pWork->pVramOBJ, 0, 16 * 32);
    DC_FlushRange( (void*)pWork->pVramBG, 16 * 32 );
    GXS_LoadBGPltt((void*)pWork->pVramBG, 0, 16 * 32);
  }

}
#endif


//--------------------------------------------------------------
/**
 * G3D VBlank処理
 * @param TCB GFL_TCB
 * @param work tcb work
 * @retval nothing
 */
//--------------------------------------------------------------
static void	_VBlank( GFL_TCB *tcb, void *work )
{
  GTSNEGO_DISP_WORK *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank

  if(pWork->bgscrollRenew){
    GFL_BG_SetScroll(GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, (pWork->bgscroll + SCROLL_HEIGHT_DEFAULT));
    GFL_BG_SetScroll(GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, (pWork->bgscroll + SCROLL_HEIGHT_DEFAULT));
    pWork->bgscrollRenew = FALSE;
  }

}

static void dispInit(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  ARCHANDLE* p_handle;
	{
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gtsnego_nego_under_bg_NCLR,
                                      PALTYPE_SUB_BG, 0, 6*0x20,  pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gtsnego_nego_back_NCLR,
                                      PALTYPE_SUB_BG, 0, 2*0x20,  pWork->heapID);

    pWork->backchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gtsnego_nego_back_NCGR,
                                                                  GFL_BG_FRAME3_S, 0, 0, pWork->heapID);
    // サブ画面BG3スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_under_NSCR,
                                              GFL_BG_FRAME3_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->backchar), 0, 0,
                                              pWork->heapID);

    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gtsnego_nego_under_bg_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg1_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);




    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gtsnego_nego_back_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    // MAIN画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gtsnego_nego_back_NCGR,
                                                                  GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // MAIN画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);


    pWork->cellRes[CHAR_MENUBAR] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gtsnego_menu_bar_NCGR ,
                              FALSE , CLSYS_DRAW_SUB , pWork->heapID );
    pWork->cellRes[PLT_MENUBAR] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_menu_bar_NCLR , CLSYS_DRAW_SUB ,    0x20*_OBJPAL_MENUBAR_POS, 0, _OBJPAL_MENUBAR_NUM, pWork->heapID  );
    pWork->cellRes[ANM_MENUBAR] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gtsnego_menubar_NCER, NARC_gtsnego_menubar_NANR , pWork->heapID  );

    
    
    pWork->cellRes[CHAR_NEGOOBJ] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gtsnego_nego_obj_NCGR ,
                              FALSE , CLSYS_DRAW_SUB , pWork->heapID );


    pWork->cellRes[PLT_NEGOOBJ] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_nego_obj_NCLR , CLSYS_DRAW_SUB , 0x20*_OBJPAL_NEGOOBJ_POS, 0, _OBJPAL_NEGOOBJ_NUM, pWork->heapID  );


    pWork->cellRes[ANM_NEGOOBJ] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gtsnego_nego_obj_NCER, NARC_gtsnego_nego_obj_NANR , pWork->heapID  );



    pWork->cellRes[CHAR_NEGOOBJ_MAIN] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gtsnego_nego_obj_NCGR ,
                              FALSE , CLSYS_DRAW_MAIN , pWork->heapID );


    pWork->cellRes[PLT_NEGOOBJ_MAIN] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_nego_obj_NCLR , CLSYS_DRAW_MAIN , 0x20*_OBJPAL_NEGOOBJ_POS_MAIN, 0, _OBJPAL_NEGOOBJ_NUM_MAIN, pWork->heapID  );


    pWork->cellRes[ANM_NEGOOBJ_MAIN] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gtsnego_nego_obj_NCER, NARC_gtsnego_nego_obj_NANR , pWork->heapID  );



    pWork->pBlink = BLINKPALANM_Create(0,16,GFL_BG_FRAME0_M,pWork->heapID);
    BLINKPALANM_SetPalBufferArcHDL(pWork->pBlink,p_handle,NARC_gtsnego_nego_back_NCLR,0,16 );

    pWork->pBlinkUnder = BLINKPALANM_Create(0,16,GFL_BG_FRAME3_S,pWork->heapID);
    BLINKPALANM_SetPalBufferArcHDL(pWork->pBlinkUnder,p_handle,NARC_gtsnego_nego_back_NCLR,0,16 );
    
    
    GFL_ARC_CloseDataHandle(p_handle);

    
	}

  {

    p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, pWork->heapID );
    for(i=0;i<UNION_TRAINER_NUM;i++){
      pWork->cellRes[CHAR_UNION+i] =
        GFL_CLGRP_CGR_Register( p_handle , WF_2DC_ARC_GETUNINCG(i) ,
                                FALSE , CLSYS_DRAW_SUB , pWork->heapID );
      pWork->cellRes[ANM_UNION+i] =
        GFL_CLGRP_CELLANIM_Register(
          p_handle , WF_2DC_ARC_GETUNICEL(i), WF_2DC_ARC_GETUNIANM(i) , pWork->heapID  );
    }
    pWork->cellRes[PLT_UNION] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,WF_2DC_ARC_GETUNINCL , CLSYS_DRAW_SUB ,    0x20*_OBJPAL_UNION_POS, 0, _OBJPAL_UNION_NUM, pWork->heapID  );
    GFL_ARC_CloseDataHandle(p_handle);
  }

  G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG2,
                    GX_BLEND_PLANEMASK_BG3|GX_BLEND_PLANEMASK_OBJ,15,9);

}



//--------------------------------------------------------------
/**
 * パレットが足りないので ともだちとジャンルとでパレット読み替え
 */
//--------------------------------------------------------------


static void _paletteModeChange(GTSNEGO_DISP_WORK* pWork, int mode)
{
  /*
  ARCHANDLE* p_handle;
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

  if(mode==_PALMODE_LEVEL){  //ジャンル選択時
    GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_NEGOOBJ] );
    pWork->cellRes[PLT_NEGOOBJ] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_nego_obj_NCLR , CLSYS_DRAW_SUB , 0x20*_OBJPAL_NEGOOBJ_POS, 0, _OBJPAL_NEGOOBJ_NUM, pWork->heapID  );
  }
  else{  //待ち合わせ時
    GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_MENUBAR] );
    pWork->cellRes[PLT_MENUBAR] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_menu_bar_NCLR , CLSYS_DRAW_SUB ,    0x20*_OBJPAL_MENUBAR_POS, 0, _OBJPAL_MENUBAR_NUM, pWork->heapID  );
  }
  GFL_ARC_CloseDataHandle(p_handle);
*/
}




void GTSNEGO_DISP_LevelInputInit(GTSNEGO_DISP_WORK* pWork)
{

  _paletteModeChange(pWork, _PALMODE_LEVEL);

  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    //キャラパレット転送済み
    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg2_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg3_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 2*0x20*3, 0,
                                              pWork->heapID);


    GFL_ARC_CloseDataHandle(p_handle);
	}

  _SetArrow(pWork,_ARROW_LEVEL_XU * 8, _ARROW_LEVEL_YU * 8,FALSE);
  _SetArrow(pWork,_ARROW_LEVEL_XD * 8, _ARROW_LEVEL_YD * 8,TRUE);

  _SetArrow(pWork,_ARROW_MY_XU * 8, _ARROW_MY_YU * 8,FALSE);
  _SetArrow(pWork,_ARROW_MY_XD * 8, _ARROW_MY_YD * 8,TRUE);

  _SetArrow(pWork,_ARROW_FRIEND_XU * 8, _ARROW_FRIEND_YU * 8,FALSE);
  _SetArrow(pWork,_ARROW_FRIEND_XD * 8, _ARROW_FRIEND_YD * 8,TRUE);

}

void GTSNEGO_DISP_LevelInputFree(GTSNEGO_DISP_WORK* pWork)
{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );
  _ArrowRelease(pWork);
  GFL_BG_FillScreen( GFL_BG_FRAME2_S,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg1_NSCR,
                                            GFL_BG_FRAME0_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                            pWork->heapID);


  
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
  GFL_ARC_CloseDataHandle(p_handle);
}


//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _TOUCHBAR_Init(GTSNEGO_DISP_WORK* pWork,int palPos)
{
  //アイコンの設定
  //数分作る
  TOUCHBAR_ITEM_ICON touchbar_icon_tbl[]	=
  {
    {
      TOUCHBAR_ICON_RETURN,
      {	TOUCHBAR_ICON_X_07, TOUCHBAR_ICON_Y },
    },
  };
  TOUCHBAR_SETUP	touchbar_setup;

  GFL_STD_MemClear( &touchbar_setup, sizeof(TOUCHBAR_SETUP) );

  touchbar_setup.p_item		= touchbar_icon_tbl;				//上の窓情報
  touchbar_setup.item_num	= NELEMS(touchbar_icon_tbl);//いくつ窓があるか
  touchbar_setup.p_unit		= pWork->cellUnit;										//OBJ読み込みのためのCLUNIT
  touchbar_setup.is_notload_bg = FALSE;
  touchbar_setup.bar_frm	= GFL_BG_FRAME3_S;						//BG読み込みのためのBG面上下画面判定にも必要
  touchbar_setup.bg_plt		= _TOUCHBAR_PAL;			//BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt	= palPos;			//OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード
  pWork->pTouchWork = TOUCHBAR_Init(&touchbar_setup, pWork->heapID);
}

TOUCHBAR_WORK* GTSNEGO_DISP_GetTouchWork(GTSNEGO_DISP_WORK* pWork)
{
  return pWork->pTouchWork;
}

void GTSNEGO_DISP_DeleteTouchWork(GTSNEGO_DISP_WORK* pWork)
{
  if(pWork->pTouchWork){
    TOUCHBAR_Exit(pWork->pTouchWork);
    pWork->pTouchWork=NULL;
  }
}


static void _CreateCrossIcon(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLWK_DATA cellInitData;

  cellInitData.pos_x = 0;
  cellInitData.pos_y = 0;
  cellInitData.anmseq = 0;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 2;
  pWork->crossIcon = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                          pWork->cellRes[CHAR_NEGOOBJ],
                                          pWork->cellRes[PLT_NEGOOBJ],
                                          pWork->cellRes[ANM_NEGOOBJ],
                                          &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->crossIcon , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->crossIcon, FALSE );
  GFL_CLACT_WK_SetObjMode( pWork->crossIcon, GX_OAM_MODE_XLU);
}


#define _FLASH_ANIM_NO (29)

static CROSSCUR_STRUCT crosspos[]=
{
  5, 128, 64,   29,
  5, 128, 64+6*8,  29,
  6, 128+8*5, 8*7,   30,
  7, 128+8*5, 8*12,  31,
  7, 128+8*5, 8*17,   31,
  0,0,0, 0,
  8, 128, 8*6,     32,
  8, 128, 8*6*2,    32,
  8, 128, 8*6*3,    32,
  0,0,0, 0,
};


void GTSNEGO_DISP_CrossIconDisp(GTSNEGO_DISP_WORK* pWork,APP_TASKMENU_WIN_WORK* pAppWin , CROSSCUR_TYPE type)
{
  switch(type){
  case _CROSSCUR_TYPE_NONE:
    GFL_CLACT_WK_SetDrawEnable( pWork->crossIcon, FALSE );
    break;
  case _CROSSCUR_TYPE_ANY4:
    GFL_CLACT_WK_SetDrawEnable( pWork->crossIcon, FALSE );
    if(pAppWin){
      APP_TASKMENU_WIN_SetActive( pAppWin, TRUE );
    }
    break;
  case _CROSSCUR_TYPE_ANY1:
  case _CROSSCUR_TYPE_ANY2:
  case _CROSSCUR_TYPE_ANY3:
  case _CROSSCUR_TYPE_FRIEND1:
  case _CROSSCUR_TYPE_FRIEND2:
  case _CROSSCUR_TYPE_FRIEND3:
    if(pAppWin){
      APP_TASKMENU_WIN_SetActive( pAppWin, FALSE );
    }
    //break;
  default:
    {
      GFL_CLACTPOS pos;
      pos.x = crosspos[type].x;
      pos.y = crosspos[type].y;
      GFL_CLACT_WK_SetPos(pWork->crossIcon, &pos, CLSYS_DRAW_SUB);
      GFL_CLACT_WK_SetAnmSeq(pWork->crossIcon, crosspos[type].no);
      GFL_CLACT_WK_SetDrawEnable( pWork->crossIcon, TRUE );
    }
    break;
  }
}

BOOL GTSNEGO_DISP_CrossIconFlash(GTSNEGO_DISP_WORK* pWork, CROSSCUR_TYPE type)
{
  if(pWork->crossIcon==NULL){
    return FALSE;
  }
//  if(GFL_CLACT_WK_GetAnmSeq(pWork->crossIcon) >= _FLASH_ANIM_NO){
  //  return FALSE;
//  }
  GFL_CLACT_WK_SetAnmSeq(pWork->crossIcon, crosspos[type].flash);
  return TRUE;
}

BOOL GTSNEGO_DISP_CrossIconFlashEnd(GTSNEGO_DISP_WORK* pWork)
{
  if(pWork->crossIcon==NULL){
    return FALSE;
  }
  if(GFL_CLACT_WK_GetAnmSeq(pWork->crossIcon) >= _FLASH_ANIM_NO){
    return !GFL_CLACT_WK_CheckAnmActive(pWork->crossIcon);
  }
  return FALSE;
}


static void _SetArrow(GTSNEGO_DISP_WORK* pWork,int x,int y,BOOL bRight)
{
  int i=0;
   
  for(i = 0 ; i < _CELL_DISP_NUM ;i++){

    if(pWork->curIcon[i]==NULL){
      GFL_CLWK_DATA cellInitData;

      cellInitData.pos_x = x;
      cellInitData.pos_y = y;
      cellInitData.anmseq = bRight ? 2 :  0;
      cellInitData.softpri = 0;
      cellInitData.bgpri = 1;
      pWork->curIcon[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                                pWork->cellRes[CHAR_NEGOOBJ],
                                                                pWork->cellRes[PLT_NEGOOBJ],
                                                                pWork->cellRes[ANM_NEGOOBJ],
                                                                &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[i] , TRUE );
      GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], TRUE );
      break;
    }
  }
}

static void _ArrowRelease(GTSNEGO_DISP_WORK* pWork)
{
  int i=0;
   
  for(i = 0 ; i < _CELL_DISP_NUM ;i++){
    if(pWork->curIcon[i]!=NULL){
      GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      pWork->curIcon[i]=NULL;
    }
  }
}

void GTSNEGO_DISP_ArrowAnim(GTSNEGO_DISP_WORK* pWork,int i)
{
  int a = GFL_CLACT_WK_GetAnmSeq( pWork->curIcon[i]);
  if(a<2){
    a=1;
  }
  else{
    a=3;
  }
  GFL_CLACT_WK_SetAnmSeq( pWork->curIcon[i] , a);
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[i] , TRUE );

}


//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_FriendSelectInit(GTSNEGO_DISP_WORK* pWork, BOOL bCursor)
{

  GTSNEGO_DISP_DeleteTouchWork(pWork);
  _TOUCHBAR_Init(pWork, _TOUCHBAR_PAL2);

  _paletteModeChange(pWork, _PALMODE_FRIEND);
	{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );
    pWork->pBGPanelScrAddr = GFL_ARCHDL_UTIL_LoadScreen(p_handle, NARC_gtsnego_nego_wait_under_bg2_NSCR,
                                              FALSE, &pWork->pBGPanelScr, pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramPaletteEx(p_handle, NARC_gtsnego_menu_bar_NCLR,
                                       PALTYPE_SUB_OBJ, 0, 0x20*_OBJPAL_MENUBAR_POS,
                                       0x20*_OBJPAL_MENUBAR_NUM, pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);

    p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramPaletteEx(p_handle, WF_2DC_ARC_GETUNINCL,
                                       PALTYPE_SUB_OBJ, 0, 0x20*_OBJPAL_UNION_POS,
                                       0x20*_OBJPAL_UNION_NUM, pWork->heapID);
    GFL_ARC_CloseDataHandle(p_handle);
  }


  _CreateMenuBarObj(pWork);
  _CreateScrollBarObj( pWork, bCursor);
  pWork->bgscrollRenew=TRUE;
}

static void _BGPanelFree(GTSNEGO_DISP_WORK* pWork)
{
  if(pWork->pBGPanelScrAddr){
    GFL_HEAP_FreeMemory(pWork->pBGPanelScrAddr);
    pWork->pBGPanelScrAddr=NULL;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択パネルを書く
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void GTSNEGO_DISP_FriendSelectPlateWrite(GTSNEGO_DISP_WORK* pWork, int index)
{
  u16* scrBuff = (u16*)pWork->pBGPanelScr->rawData;
  int add = GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar);
  int j=0,x,y;

  for(y = 0; y < 6; y++){
    for(x = 0;x<32;x++){
      GFL_BG_ScrSetDirect(GFL_BG_FRAME0_S, x, y + index*6, scrBuff[j]+add);
      j++;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択パネルを消す
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void GTSNEGO_DISP_FriendSelectPlateClean(GTSNEGO_DISP_WORK* pWork, int index)
{
  u16* scrBuff = (u16*)pWork->pBGPanelScr->rawData;
  int add = GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar);
  int j=0,x,y;

  for(y = 0; y < 6; y++){
    for(x = 0;x<32;x++){
      GFL_BG_ScrSetDirect(GFL_BG_FRAME0_S, x, y + index*6, scrBuff[31]+add);
      j++;
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択パネルを MYSTATUSをみて書き直す
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_FriendSelectPlateView(GTSNEGO_DISP_WORK* pWork, GAMEDATA* pGameData, int topindex)
{
  int i;
  
  for(i=0; i < SCROLL_PANEL_NUM;i++){
    MYSTATUS* pMy = GTSNEGO_GetMyStatus(pGameData, topindex+i);
    if(pMy){
      GTSNEGO_DISP_FriendSelectPlateWrite(pWork, i);
    }
    else{
      GTSNEGO_DISP_FriendSelectPlateClean(pWork, i);
    }
  }
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
}

//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択開放
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_FriendSelectFree(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

  for(i=0;i<SCROLL_PANEL_NUM;i++){
    if(pWork->unionOAM[i]){
      GFL_CLACT_WK_Remove(pWork->unionOAM[i]);
      pWork->unionOAM[i]=NULL;
    }
  }
  
  
  GTSNEGO_DISP_DeleteTouchWork(pWork);
  _TOUCHBAR_Init(pWork, _TOUCHBAR_PAL1);
  _RemoveMenuBarObj(pWork);
  GFL_ARC_CloseDataHandle(p_handle);
}

void GTSNEGO_DISP_FriendSelectFree2(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

  _BGPanelFree(pWork);


  _DeleteScrollBarObj(pWork);
  
  GFL_BG_FillScreen( GFL_BG_FRAME2_S,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_S );
  GFL_ARCHDL_UTIL_TransVramPaletteEx(p_handle, NARC_gtsnego_nego_obj_NCLR,
                                     PALTYPE_SUB_OBJ, 0, 0x20*_OBJPAL_NEGOOBJ_POS,
                                     0x20*_OBJPAL_NEGOOBJ_NUM, pWork->heapID);
  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg1_NSCR,
                                            GFL_BG_FRAME0_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                            pWork->heapID);

  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_S );
  GFL_ARC_CloseDataHandle(p_handle);

}



//----------------------------------------------------------------------------
/**
 *	@brief	下キーが押された時の処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

int GTSNEGO_DISP_FriendListDownChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur)
{
  int ChangeCode = 0;

  if((pCur->curpos < 2) && ( pCur->curpos < (pCur->listmax-1) )){
    pCur->curpos++;
    ChangeCode = 1;
  }
  else if((pCur->curpos==2) && ((pCur->oamlistpos+3) < pCur->listmax)){
    //カーソルはそのままでリストが移動
    pCur->oamlistpos++;
    ChangeCode = 2;
  }
  return ChangeCode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	上キーが押された時の処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

int GTSNEGO_DISP_FriendListUpChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur)
{
  int ChangeCode = 0;

  if(pCur->curpos > 0){
    pCur->curpos--;
    ChangeCode = 1;
  }
  else if((pCur->curpos==0) && (pCur->oamlistpos!=0)){
    //カーソルはそのままでリストが移動
    pCur->oamlistpos--;
    ChangeCode = 2;
  }

  return ChangeCode;
}

//----------------------------------------------------------------------------
/**
 *	@brief	カーソル位置をセット
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_FriendListSetCurPos(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur, const u32 curpos)
{
  NOZOMU_Printf("カーソル位置のセット %d\n",pCur->listmax);
  if (curpos < pCur->listmax)
  {
    pCur->curpos = curpos;
  }

  if (pCur->curpos > 2) pCur->curpos = 2;
  NOZOMU_Printf("CurPos = %d\n",pCur->curpos);

}

static void _CreateMenuBarObj(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLWK_DATA cellInitData;

  cellInitData.pos_x = 128;
  cellInitData.pos_y = 192+8;
  cellInitData.anmseq = 0;
  cellInitData.softpri = 1;
  cellInitData.bgpri = 1;
  pWork->menubarObj = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                          pWork->cellRes[CHAR_MENUBAR],
                                          pWork->cellRes[PLT_MENUBAR],
                                          pWork->cellRes[ANM_MENUBAR],
                                          &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->menubarObj , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->menubarObj, TRUE );
}

static void _RemoveMenuBarObj(GTSNEGO_DISP_WORK* pWork)
{
  if(pWork->menubarObj){
    GFL_CLACT_WK_Remove(pWork->menubarObj);
  }
  pWork->menubarObj=NULL;
}




static void _CreateScrollBarObj(GTSNEGO_DISP_WORK* pWork, BOOL bCursor)
{
  GFL_CLWK_DATA cellInitData;
  int i;
  u16 buffx[]={250,  250,  128};
  u16 buffy[]={192/2,192/2, 16};
  u16 oamno[]={4,10, 9};
  u16 bgpri[]={1, 1, 1};


  for(i=0;i< _SCROLLBAR_OAM_NUM;i++){
    if(!bCursor && i == 0){
    }
    else{
      cellInitData.pos_x = buffx[i];
      cellInitData.pos_y = buffy[i];
      cellInitData.anmseq = oamno[i];
      cellInitData.softpri = i;
      cellInitData.bgpri = bgpri[i];
      pWork->scrollbarOAM[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                    pWork->cellRes[CHAR_NEGOOBJ],
                                                    pWork->cellRes[PLT_NEGOOBJ],
                                                    pWork->cellRes[ANM_NEGOOBJ],
                                                    &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->scrollbarOAM[i] , TRUE );
      GFL_CLACT_WK_SetDrawEnable( pWork->scrollbarOAM[i], TRUE );
    }
  }

}


static void _DeleteScrollBarObj(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  for(i=0;i< _SCROLLBAR_OAM_NUM;i++){
    if(pWork->scrollbarOAM[i]){
      GFL_CLACT_WK_Remove(pWork->scrollbarOAM[i]);
      pWork->scrollbarOAM[i]=NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクロールのチップのアニメONOFF
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_SetAnmScrollBarObj(GTSNEGO_DISP_WORK* pWork, BOOL bFlg)
{
  if(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP]){
    GFL_CLACT_WK_ResetAnm(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP]);
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP] , bFlg );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	下に１つスクロールする処理開始
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


void GTSNEGO_DISP_PanelScrollStart(GTSNEGO_DISP_WORK* pWork,int scrollType)
{
  pWork->bgscrollType = scrollType;
  if(pWork->bgscrollType == PANEL_DOWNSCROLL_){
    pWork->bgscrollPos = pWork->bgscroll+48;
  }
  else if(pWork->bgscrollType == PANEL_UPSCROLL_){
    pWork->bgscrollPos = pWork->bgscroll-48;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	キャンセル処理
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


void GTSNEGO_DISP_PanelScrollCancel(GTSNEGO_DISP_WORK* pWork)
{
  pWork->bgscrollType = PANEL_NONESCROLL_;
}

//----------------------------------------------------------------------------
/**
 *	@brief	下に１つスクロールする処理開始
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


int GTSNEGO_DISP_PanelScrollMain(GTSNEGO_DISP_WORK* pWork,int* EndTrg)
{
  u8 ret = PANEL_NONESCROLL_;
  int i;
  
  if(pWork->bgscrollType == PANEL_DOWNSCROLL_){
    pWork->bgscroll+=8;
    if(pWork->bgscrollPos==pWork->bgscroll){
      *EndTrg = pWork->bgscrollType;
      pWork->bgscrollType = PANEL_NONESCROLL_;
      pWork->bgscroll-=SCROLL_HEIGHT_SINGLE;
    }
    for(i=0;i<GTSNEGO_WINDOW_MAXNUM;i++){
      GTSNEGO_DISP_UnionWkScroll(pWork,i,-8);
    }
    pWork->bgscrollRenew=TRUE;
  }
  if(pWork->bgscrollType == PANEL_UPSCROLL_){
    pWork->bgscroll-=8;
    if(pWork->bgscrollPos==pWork->bgscroll){
      *EndTrg = pWork->bgscrollType;
      pWork->bgscrollType = PANEL_NONESCROLL_;
      pWork->bgscroll+=SCROLL_HEIGHT_SINGLE;
    }
    for(i=0;i<GTSNEGO_WINDOW_MAXNUM;i++){
      GTSNEGO_DISP_UnionWkScroll(pWork,i,8);
    }
    pWork->bgscrollRenew=TRUE;
  }
  return pWork->bgscrollType;
}


void GTSNEGO_DISP_PanelScrollAdjust(GTSNEGO_DISP_WORK* pWork,BOOL bDown)
{
  int i,k=1;

  if(bDown){
    k=-1;
  }
  
  for(i=0;i<GTSNEGO_WINDOW_MAXNUM;i++){
    GTSNEGO_DISP_UnionWkScroll(pWork,i,48*k);
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロールリセット
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_ScrollReset(GTSNEGO_DISP_WORK* pWork)
{
  GFL_BG_SetScroll(GFL_BG_FRAME0_S, GFL_BG_SCROLL_Y_SET, 0);
  GFL_BG_SetScroll(GFL_BG_FRAME2_S, GFL_BG_SCROLL_Y_SET, 0);
}

//----------------------------------------------------------------------------
/**
 *	@brief	アイコンWorkCreate
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void GTSNEGO_DISP_UnionWkCreate(GTSNEGO_DISP_WORK* pWork,int workIndex,int unionNo)
{
  GFL_CLWK_DATA cellInitData;
  int i = workIndex;

  GF_ASSERT(workIndex<GTSNEGO_WINDOW_MAXNUM);
  GF_ASSERT(unionNo<UNION_TRAINER_NUM);

  if(pWork->unionOAM[i]){
    GFL_CLACT_WK_Remove(pWork->unionOAM[i]);
  }

  
  cellInitData.pos_x = 0;
  cellInitData.pos_y = 0;
  cellInitData.anmseq = 1;
  cellInitData.softpri = 10;
  cellInitData.bgpri = 2;
  pWork->unionOAM[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                pWork->cellRes[CHAR_UNION+unionNo],
                                                pWork->cellRes[PLT_UNION],
                                                pWork->cellRes[ANM_UNION+unionNo],
                                                &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->unionOAM[i] , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->unionOAM[i], FALSE );
}






//----------------------------------------------------------------------------
/**
 *	@brief	アイコン表示
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void GTSNEGO_DISP_UnionWkDisp(GTSNEGO_DISP_WORK* pWork,int workIndex,int x,int y)
{
  GFL_CLACTPOS pos;

  GF_ASSERT(workIndex<GTSNEGO_WINDOW_MAXNUM);
  if(workIndex<GTSNEGO_WINDOW_MAXNUM){
    pos.x = x;
    pos.y = y;
    GFL_CLACT_WK_SetPos(pWork->unionOAM[workIndex], &pos, CLSYS_DRAW_SUB);
    GFL_CLACT_WK_SetDrawEnable( pWork->unionOAM[workIndex], TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	アイコン表示
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_UnionListDisp(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy,int workIndex)
{
  int i=workIndex;

  if(pMy){
    GTSNEGO_DISP_UnionWkCreate(pWork, i, MyStatus_GetTrainerView(pMy));
    GTSNEGO_DISP_UnionWkDisp(pWork, i , 8*3, -8*6+i*8*6 );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	アイコン移動
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void GTSNEGO_DISP_UnionWkScroll(GTSNEGO_DISP_WORK* pWork,int workIndex, int move)
{
  int i=workIndex;
  GFL_CLACTPOS pos;

  if(pWork->unionOAM[workIndex]){
    GFL_CLACT_WK_GetPos(pWork->unionOAM[workIndex], &pos, CLSYS_DRAW_SUB);
    pos.y += move;
    GFL_CLACT_WK_SetPos(pWork->unionOAM[workIndex], &pos, CLSYS_DRAW_SUB);
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	UNIONアイコンが上にスクロール(下にキーが入った)ときの処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_UnionListDown(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy)
{
  int i,endmark = SCROLL_PANEL_NUM-1;

  if(pMy){
    if(pWork->unionOAM[0]){
      GFL_CLACT_WK_Remove(pWork->unionOAM[0]);
    }
    for(i = 0 ; i < endmark ; i++){  //場所スライド
      pWork->unionOAM[i] =pWork->unionOAM[i+1];
    }
    pWork->unionOAM[endmark]=NULL;
    GTSNEGO_DISP_UnionListDisp(pWork, pMy, endmark );

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	UNIONアイコンが下にスクロール(上にキーが入った)ときの処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_UnionListUp(GTSNEGO_DISP_WORK* pWork,MYSTATUS* pMy)
{
  int i,endmark = SCROLL_PANEL_NUM-1;

  if(pMy){
    if(pWork->unionOAM[endmark]){
      GFL_CLACT_WK_Remove(pWork->unionOAM[endmark]);
    }
    for(i = endmark ; i > 0; i--){  //場所スライド
      OS_TPrintf("Uplist %d\n",i);
      pWork->unionOAM[i] = pWork->unionOAM[i-1];
    }
    pWork->unionOAM[0] = NULL;
    GTSNEGO_DISP_UnionListDisp(pWork, pMy, 0 );
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	UNIONアイコン全部更新
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_UnionListRenew(GTSNEGO_DISP_WORK* pWork,GAMEDATA* pGameData, int no)
{
  MYSTATUS* pMyStatus;
  int i;

  for( i =0;i < SCROLL_PANEL_NUM;i++){
    int x = i+no;
    if(x >= 0){
      pMyStatus = GTSNEGO_GetMyStatus(pGameData, x);
      if(pMyStatus){
        GTSNEGO_DISP_UnionListDisp(pWork, pMyStatus, i);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクロールカーソルの位置
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static int _getScrollCurPos(GTSNEGO_DISP_WORK* pWork,int pos,int max)
{
  int ret = 0;
  
  if(pos == 0){
    ret = 0;
  }
  else if(pos == (max-1)){
    ret = SCROLLBAR_LENGTH;
  }
  else{
    ret = (SCROLLBAR_LENGTH * pos) / (max-1);
    NET_PRINT("%d  %d %d\n",ret,pos,max);
  }
  return (ret );//+ _OFFSET_SCROLL);
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロールカーソルの位置
 *	@param	
 *	@return	index
 */
//-----------------------------------------------------------------------------

static int _touchToCurcorPos(GTSNEGO_DISP_WORK* pWork,int y,int max)
{
  float yf = y - SCROLLBAR_TOP;// - _OFFSET_SCROLL;
  float length = SCROLLBAR_LENGTH;
  float ansf;

  if(max <= 3){
    return 0;
  }

  length = length / (max-2);
  ansf = yf / length;

  if(ansf >= max-2 ){
    ansf = max-3;
  }

  return (int)ansf;
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロールバーのチップの位置変更
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_ScrollChipDisp(GTSNEGO_DISP_WORK* pWork,int index,int max)
{
  GFL_CLACTPOS pos;

  if(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP]){
    GFL_CLACT_WK_GetPos(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP], &pos, CLSYS_DRAW_SUB);
    pos.y = _getScrollCurPos(pWork,index, max)+SCROLLBAR_TOP;
    GFL_CLACT_WK_SetPos(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP], &pos, CLSYS_DRAW_SUB);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	スクロールバーのチップの位置変更
 *	@param	GTSNEGO_DISP_WORK
 *	@return	カーソル位置
 */
//-----------------------------------------------------------------------------

int GTSNEGO_DISP_ScrollChipDispMouse(GTSNEGO_DISP_WORK* pWork,int y,int max)
{
  int ret = 0;
  GFL_CLACTPOS pos;

  if(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP]){
    GFL_CLACT_WK_GetPos(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP], &pos, CLSYS_DRAW_SUB);
    pos.y = y;
    GFL_CLACT_WK_SetPos(pWork->scrollbarOAM[_SCROLLBAR_OAM_CHIP], &pos, CLSYS_DRAW_SUB);
    ret = _touchToCurcorPos( pWork, y, max);
  }
  return ret;
}




static void _modeFlashCallback(u32 param, fx32 currentFrame )
{
  GTSNEGO_DISP_WORK* pWork = (GTSNEGO_DISP_WORK*)param;
  {
    u16 no = GFL_CLACT_WK_GetAnmSeq(pWork->SearchPeopleOAM);
    no++;
    if(no > SEARCH_ANIM_END){
      no = SEARCH_ANIM_START;
    }
    GFL_CLACT_WK_SetAnmSeq(pWork->SearchPeopleOAM, no);
  }
}



static void _SetSearchPeople(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLWK_DATA cellInitData;

  cellInitData.pos_x = 128;
  cellInitData.pos_y = 64-8-4;
  cellInitData.anmseq =  27;
  cellInitData.softpri = 2;
  cellInitData.bgpri = 1;
  pWork->SearchBackOAM = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->cellRes[CHAR_NEGOOBJ_MAIN],
                                              pWork->cellRes[PLT_NEGOOBJ_MAIN],
                                              pWork->cellRes[ANM_NEGOOBJ_MAIN],
                                              &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->SearchBackOAM , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->SearchBackOAM, TRUE );

  cellInitData.anmseq =  11;
  cellInitData.softpri = 1;
  pWork->SearchPeopleOAM = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->cellRes[CHAR_NEGOOBJ_MAIN],
                                              pWork->cellRes[PLT_NEGOOBJ_MAIN],
                                              pWork->cellRes[ANM_NEGOOBJ_MAIN],
                                              &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->SearchPeopleOAM , TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->SearchPeopleOAM, TRUE );

  {
    GFL_CLWK_ANM_CALLBACK cbwk;

    cbwk.callback_type = CLWK_ANM_CALLBACK_TYPE_LAST_FRM ;  // CLWK_ANM_CALLBACK_TYPE
    cbwk.param = (u32)pWork;          // コールバックワーク
    cbwk.p_func = _modeFlashCallback; // コールバック関数
    GFL_CLACT_WK_StartAnmCallBack( pWork->SearchPeopleOAM, &cbwk );
  }
  
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
}



static void _RemoveSearchPeople(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLACT_WK_SetDrawEnable( pWork->SearchBackOAM, FALSE );
  GFL_CLACT_WK_Remove(pWork->SearchBackOAM);
  GFL_CLACT_WK_SetDrawEnable( pWork->SearchPeopleOAM, FALSE );
  GFL_CLACT_WK_Remove(pWork->SearchPeopleOAM);
}



//----------------------------------------------------------------------------
/**
 *	@brief	サーチ完了時に画面に変更
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_SearchEndPeopleDispSet(GTSNEGO_DISP_WORK* pWork,int index)
{
  int i;
  ARCHANDLE* p_handle;
  int unionno[]=
  {0,6,8,12,14,3,4,10,
   11,1,13,5,7,9,15,2 };
//たんパンこぞう
//エリートトレーナー♂
//ポケモンレンジャー♂
//ポケモンブリーダー♂
//けんきゅういん♂
//やまおとこ
//スキンヘッズ
//ようちえん♂
//ミニスカート
//エリートトレーナー♀
//ポケモンレンジャー♀
//ポケモンブリーダー♀
//けんきゅういん♀
//パラソルおねえさん
//ナース
//ようちえん♀

//  NET_PRINT("ユニオン %d >> %d\n",index,unionno[index] );
	{
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_search3_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);

    GFL_CLACT_WK_SetAutoAnmFlag( pWork->SearchPeopleOAM , FALSE );
    GFL_CLACT_WK_SetAnmSeq(pWork->SearchPeopleOAM, 11 + unionno[index]);
    GFL_CLACT_WK_SetAnmIndex(pWork->SearchPeopleOAM,4);
    GFL_CLACT_WK_SetAnmSeq(pWork->SearchBackOAM, 28);

    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
    pWork->SearchScroll=0;
    pWork->bSearchScroll=FALSE;
    
	}
}




//----------------------------------------------------------------------------
/**
 *	@brief	サーチ中画面に変更
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_SearchPeopleDispSet(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  ARCHANDLE* p_handle;
	{
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_search2_NSCR,
                                              GFL_BG_FRAME2_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_search_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_search4_NSCR,
                                              GFL_BG_FRAME3_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);

    _SetSearchPeople( pWork);
    
    
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME2_M );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
  GFL_BG_LoadScreenV_Req( GFL_BG_FRAME3_M );
    pWork->SearchScroll=0;
    pWork->bSearchScroll=TRUE;
    
	}
}


//----------------------------------------------------------------------------
/**
 *	@brief	サーチ中画面に変更
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_SearchPeopleDispMain(GTSNEGO_DISP_WORK* pWork)
{
  if(pWork->bSearchScroll){
    GFL_BG_SetScroll(GFL_BG_FRAME2_M, GFL_BG_SCROLL_Y_SET, pWork->SearchScroll );
    pWork->SearchScroll++;
  }
}


void GTSNEGO_DISP_ResetDispSet(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  ARCHANDLE* p_handle;
	{
    for(i=0;i<SCROLL_PANEL_NUM;i++){
      if(pWork->unionOAM[i]){
        GFL_CLACT_WK_SetDrawEnable(pWork->unionOAM[i],FALSE);
        GFL_CLACT_WK_Remove(pWork->unionOAM[i]);
        pWork->unionOAM[i]=NULL;
      }
    }
    _RemoveMenuBarObj(pWork);
    _BGPanelFree(pWork);
    _RemoveSearchPeople(pWork);
    _DeleteScrollBarObj(pWork);
    _ArrowRelease(pWork);
    //-----------------------再転送
    p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_under_bg1_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);
    
    GFL_ARCHDL_UTIL_TransVramPaletteEx(p_handle, NARC_gtsnego_nego_obj_NCLR,
                                       PALTYPE_SUB_OBJ, 0, 0x20*_OBJPAL_NEGOOBJ_POS,
                                       0x20*_OBJPAL_NEGOOBJ_NUM, pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);
    //---------------------

    GTSNEGO_DISP_DeleteTouchWork(pWork);
    _TOUCHBAR_Init(pWork, _TOUCHBAR_PAL1);
    
    GFL_BG_SetVisible( GFL_BG_FRAME2_M,FALSE );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );
    GFL_BG_SetVisible( GFL_BG_FRAME3_M,FALSE );
    pWork->SearchScroll=0;
    pWork->bSearchScroll=FALSE;
    
	}
}

