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

#include "infowin/infowin.h"
#include "system/main.h"
#include "system/wipe.h"
#include "gamesystem/msgspeed.h" // MSGSPEED_GetWait

#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"

#include "../../field/event_ircbattle.h"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK

#include "gtsnego_local.h"
#include "msg/msg_gtsnego.h"
#include "gtsnego.naix"



struct _GTSNEGO_DISP_WORK {
	u32 subchar;
  u32 mainchar;
  HEAPID heapID;

};

static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

  //        GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_32K,

};

static GFL_BG_SYS_HEADER BGsys_data = {
  GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

static void dispInit(GTSNEGO_DISP_WORK* pWork);
static void settingSubBgControl(GTSNEGO_DISP_WORK* pWork);


GTSNEGO_DISP_WORK* GTSNEGO_DISP_Init(HEAPID id)
{
  GTSNEGO_DISP_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(GTSNEGO_DISP_WORK));
  pWork->heapID = id;


  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GXS_DispOn();
  GX_DispOn();

  GFL_BG_Init(pWork->heapID);

  GFL_DISP_SetBank( &_defVBTbl );
  GFL_BG_SetBGMode( &BGsys_data );
  settingSubBgControl(pWork);
  dispInit(pWork);
  
  return pWork;
}

void GTSNEGO_DISP_Main(GTSNEGO_DISP_WORK* pWork)
{
}

void GTSNEGO_DISP_End(GTSNEGO_DISP_WORK* pWork)
{
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_Exit();
  GFL_HEAP_FreeMemory(pWork);

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
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );
    GFL_BG_SetPriority( GFL_BG_FRAME0_M, 0 );
		GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );

  }

  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
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
    int frame = GFL_BG_FRAME3_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}

static void dispInit(GTSNEGO_DISP_WORK* pWork)
{
	{
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GTSNEGO, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gtsnego_connect_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gtsnego_connect_sub_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gtsnego_connect_sub_NSCR,
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

    GFL_ARC_CloseDataHandle(p_handle);
	}

}
