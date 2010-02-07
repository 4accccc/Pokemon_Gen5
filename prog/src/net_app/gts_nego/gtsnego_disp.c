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


FS_EXTERN_OVERLAY(ui_common);

typedef enum
{
  PLT_NEGOOBJ,  
  PLT_MENUBAR,  
  PLT_RESOURCE_MAX,
  CHAR_NEGOOBJ = PLT_RESOURCE_MAX,
  CHAR_MENUBAR,
  CHAR_RESOURCE_MAX,
  ANM_NEGOOBJ = CHAR_RESOURCE_MAX,
  ANM_MENUBAR,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;


typedef enum
{
_SCROLLBAR_OAM_TITLEP,
_SCROLLBAR_OAM_BAR,
_SCROLLBAR_OAM_CHIP,
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
//  int curpos;
 // int oamlistpos;
  GAMEDATA* pGameData;

  GFL_CLWK* crossIcon;
  GFL_CLWK* menubarObj;
  GFL_CLWK* scrollbarOAM[_SCROLLBAR_OAM_NUM];
};


typedef struct{
  u16 no;
  u8 x;
  u8 y;
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
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

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
static void _TOUCHBAR_Init(GTSNEGO_DISP_WORK* pWork);
static void	_VBlank( GFL_TCB *tcb, void *work );
static void _SetArrow(GTSNEGO_DISP_WORK* pWork,int x,int y,BOOL bRight);
static void _ArrowRelease(GTSNEGO_DISP_WORK* pWork);
static void _FriendListPlateDisp(GTSNEGO_DISP_WORK* pWork, GTSNEGO_MESSAGE_WORK* pMessageWork);
static void _DebugDataCreate(GTSNEGO_DISP_WORK* pWork);
static void _CreateCrossIcon(GTSNEGO_DISP_WORK* pWork);
static void _CreateMenuBarObj(GTSNEGO_DISP_WORK* pWork);

static void _PanelScrollMain(GTSNEGO_DISP_WORK* pWork);
static void _CreateScrollBarObj(GTSNEGO_DISP_WORK* pWork);
static void _DeleteScrollBarObj(GTSNEGO_DISP_WORK* pWork);




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
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 40 , 0 , pWork->heapID );

  GFL_DISP_SetBank( &_defVBTbl );
  GFL_BG_SetBGMode( &BGsys_data );
  settingSubBgControl(pWork);
  dispInit(pWork);
  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  _CreateCrossIcon(pWork);
  
  _TOUCHBAR_Init(pWork);

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

#if DEBUG_ONLY_FOR_ohno
  _DebugDataCreate(pWork);
#endif
  _CreateMenuBarObj(pWork);
  
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
}

void GTSNEGO_DISP_End(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLACT_WK_Remove(pWork->crossIcon);
  GFL_CLACT_WK_Remove(pWork->menubarObj);

  
  _ArrowRelease(pWork);
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_NEGOOBJ] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_NEGOOBJ] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_NEGOOBJ] );
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_MENUBAR] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_MENUBAR] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_MENUBAR] );
  

  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
  BLINKPALANM_Exit(pWork->pBlink);
  BLINKPALANM_Exit(pWork->pBlinkUnder);

  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_M, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME2_S, 1, 0);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_Exit();
  GFL_HEAP_FreeMemory(pWork);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));
  

}


static void settingSubBgControl(GTSNEGO_DISP_WORK* pWork)
{

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
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
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
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
	{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

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
    // サブ画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gtsnego_nego_back_NCGR,
                                                                  GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_back_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);


    pWork->cellRes[CHAR_NEGOOBJ] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gtsnego_nego_obj_NCGR ,
                              FALSE , CLSYS_DRAW_SUB , pWork->heapID );
    pWork->cellRes[PLT_NEGOOBJ] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_nego_obj_NCLR , CLSYS_DRAW_SUB ,    0, 0, 6, pWork->heapID  );
    pWork->cellRes[ANM_NEGOOBJ] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gtsnego_nego_obj_NCER, NARC_gtsnego_nego_obj_NANR , pWork->heapID  );


    pWork->cellRes[CHAR_MENUBAR] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gtsnego_menu_bar_NCGR ,
                              FALSE , CLSYS_DRAW_SUB , pWork->heapID );
    pWork->cellRes[PLT_MENUBAR] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gtsnego_menu_bar_NCLR , CLSYS_DRAW_SUB ,    0x20*6, 0, 1, pWork->heapID  );
    pWork->cellRes[ANM_MENUBAR] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gtsnego_menubar_NCER, NARC_gtsnego_menubar_NANR , pWork->heapID  );
    
    pWork->pBlink = BLINKPALANM_Create(0,16,GFL_BG_FRAME0_M,pWork->heapID);
    BLINKPALANM_SetPalBufferArcHDL(pWork->pBlink,p_handle,NARC_gtsnego_nego_back_NCLR,0,16 );

    pWork->pBlinkUnder = BLINKPALANM_Create(0,16,GFL_BG_FRAME3_S,pWork->heapID);
    BLINKPALANM_SetPalBufferArcHDL(pWork->pBlinkUnder,p_handle,NARC_gtsnego_nego_back_NCLR,0,16 );
    
    
    GFL_ARC_CloseDataHandle(p_handle);

    
	}
  G2S_SetBlendAlpha(GX_BLEND_PLANEMASK_BG0,GX_BLEND_PLANEMASK_BG3,15,9);

}


void GTSNEGO_DISP_LevelInputInit(GTSNEGO_DISP_WORK* pWork)
{
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

static void _TOUCHBAR_Init(GTSNEGO_DISP_WORK* pWork)
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
  touchbar_setup.obj_plt	= _TOUCHBAR_PAL;			//OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード
  pWork->pTouchWork = TOUCHBAR_Init(&touchbar_setup, pWork->heapID);
}

TOUCHBAR_WORK* GTSNEGO_DISP_GetTouchWork(GTSNEGO_DISP_WORK* pWork)
{
  return pWork->pTouchWork;
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
}


static CROSSCUR_STRUCT crosspos[]=
{
  5, 128, 64,
  5, 128, 64+6*8,
  6, 128+8*5, 8*7,
  7, 128+8*5, 8*12,
  7, 128+8*5, 8*17,
  0,0,0,
  8, 128, 8*6,
  8, 128, 8*6*2,
  8, 128, 8*6*3,
  0,0,0,
};


void GTSNEGO_DISP_CrossIconDisp(GTSNEGO_DISP_WORK* pWork,APP_TASKMENU_WIN_WORK* pAppWin , CROSSCUR_TYPE type)
{
  switch(type){
  case _CROSSCUR_TYPE_NONE:
    GFL_CLACT_WK_SetDrawEnable( pWork->crossIcon, FALSE );
    break;
  case _CROSSCUR_TYPE_ANY4:
  case _CROSSCUR_TYPE_FRIEND4:
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


//----------------------------------------------------------------------------
/**
 *	@brief	フレンド選択初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

void GTSNEGO_DISP_FriendSelectInit(GTSNEGO_DISP_WORK* pWork, GTSNEGO_MESSAGE_WORK* pMessageWork)
{

	{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );


    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_wait_under_bg2_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);

//    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_nego_wait_under_bg_NSCR,
  //                                            GFL_BG_FRAME0_S, 0,
    //                                          GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
      //                                        pWork->heapID);
    
    GFL_ARC_CloseDataHandle(p_handle);
	}
 // pWork->bgscroll = 0;
  pWork->bgscrollRenew=TRUE;

//  _FriendListPlateDisp(pWork, pMessageWork);
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
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

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
 *	@brief	下キーが押された時の処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

BOOL GTSNEGO_DISP_FriendListDownChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur)
{
  BOOL bChange = FALSE;

  if(pCur->curpos < 2){
    pCur->curpos++;
  }
  else if((pCur->curpos==2) && ((pCur->oamlistpos+3) < pCur->listmax)){
    //カーソルはそのままでリストが移動
    pCur->oamlistpos++;
    bChange = TRUE;
  }
  return bChange;
}

//----------------------------------------------------------------------------
/**
 *	@brief	上キーが押された時の処理
 *	@param	GTSNEGO_DISP_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

BOOL GTSNEGO_DISP_FriendListUpChk(GTSNEGO_DISP_WORK* pWork, SCROLLPANELCURSOR* pCur)
{
  BOOL bChange = FALSE;

  if(pCur->curpos > 0){
    pCur->curpos--;
  }
  else if((pCur->curpos==0) && (pCur->oamlistpos!=-1)){
    //カーソルはそのままでリストが移動
    pCur->oamlistpos--;
    bChange = TRUE;
  }

  return bChange;
}


//----------------------------------------------------------------------------
/**
 *	@brief	フレンドダミーデータ作成
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------
#if DEBUG_ONLY_FOR_ohno

static void _DebugDataCreate(GTSNEGO_DISP_WORK* pWork)
{
  int i;

  for(i = 0; i < WIFI_NEGOTIATION_DATAMAX;i++){
    MYSTATUS* pMyStatus = MyStatus_AllocWork(pWork->heapID);
    MyStatus_SetProfileID(pMyStatus,12+i);
    MyStatus_SetID(pMyStatus,12+i);
    MyStatus_SetMyNationArea(pMyStatus,1+i,1);
    WIFI_NEGOTIATION_SV_SetFriend(GAMEDATA_GetWifiNegotiation(pWork->pGameData),pMyStatus);
    GFL_HEAP_FreeMemory(pMyStatus);
  }
}


#endif


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



static void _CreateScrollBarObj(GTSNEGO_DISP_WORK* pWork)
{
  GFL_CLWK_DATA cellInitData;
  int i;
  u16 buffx[]={250,250,128};
  u16 buffy[]={192/2,192/2,16};
  u16 oamno[]={4,9,10};


  for(i=0;i< _SCROLLBAR_OAM_NUM;i++){
    cellInitData.pos_x = buffx[i];
    cellInitData.pos_y = buffy[i];
    cellInitData.anmseq = oamno[i];
    cellInitData.softpri = i;
    cellInitData.bgpri = 0;
    pWork->scrollbarOAM[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                  pWork->cellRes[CHAR_NEGOOBJ],
                                                  pWork->cellRes[PLT_NEGOOBJ],
                                                  pWork->cellRes[ANM_NEGOOBJ],
                                                  &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->scrollbarOAM[i] , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->scrollbarOAM[i], TRUE );
  }

}


static void _DeleteScrollBarObj(GTSNEGO_DISP_WORK* pWork)
{
  int i;
  for(i=0;i< _SCROLLBAR_OAM_NUM;i++){
    GFL_CLACT_WK_Remove(pWork->scrollbarOAM[i]);
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
 *	@brief	下に１つスクロールする処理開始
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


int GTSNEGO_DISP_PanelScrollMain(GTSNEGO_DISP_WORK* pWork)
{
  u8 ret = PANEL_NONESCROLL_;
  
  if(pWork->bgscrollType == PANEL_DOWNSCROLL_){
    pWork->bgscroll+=8;
    if(pWork->bgscrollPos==pWork->bgscroll){
      ret = pWork->bgscrollType;
      pWork->bgscrollType = PANEL_NONESCROLL_;
      pWork->bgscroll-=SCROLL_HEIGHT_SINGLE;
    }
    pWork->bgscrollRenew=TRUE;
  }
  if(pWork->bgscrollType == PANEL_UPSCROLL_){
    pWork->bgscroll-=8;
    if(pWork->bgscrollPos==pWork->bgscroll){
      ret = pWork->bgscrollType;
      pWork->bgscrollType = PANEL_NONESCROLL_;
      pWork->bgscroll+=SCROLL_HEIGHT_SINGLE;
    }
    pWork->bgscrollRenew=TRUE;
  }
  return ret;
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
