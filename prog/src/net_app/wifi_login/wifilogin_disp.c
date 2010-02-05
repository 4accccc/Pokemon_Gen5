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

#include "wifilogin_local.h"
//#include "msg/msg_gtsnego.h"
#include "wifi_login.naix"

#include "gsync.naix"
#include "../gsync/gsync_obj_NANR_LBLDEFS.h"

typedef enum
{
  PLT_SMOKE,
  PLT_RESOURCE_MAX,
  CHAR_SMOKE = PLT_RESOURCE_MAX,
  CHAR_RESOURCE_MAX,
  ANM_SMOKE = CHAR_RESOURCE_MAX,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;

typedef enum
{
  CELL_SMOKE1,
  CELL_SMOKE2,
  CELL_SMOKE3,
  CELL_SMOKE4,
  CELL_SMOKE5,
  _CELL_DISP_NUM,
} _CELL_WK_ENUM;


#define MOVESMOKE_RANDX (2)
#define MOVESMOKE_RANDY (3)

#define MOVESMOKE_SLEEPX (3)
#define MOVESMOKE_SLEEPY (2)

#define MOVESMOKE_CREATE (50)


typedef struct{
  int rightleft;
} MOVESMOKE;




struct _WIFILOGIN_DISP_WORK {
	u32 subchar;
  u32 mainchar;
  WIFILOGIN_BG  bg;
  HEAPID heapID;

  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
  u32 cellRes[CEL_RESOURCE_MAX];
  GFL_CLWK* curIcon[_CELL_DISP_NUM];
  MOVESMOKE moveSmoke[_CELL_DISP_NUM];
  int smokeTimer;
};

static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_128K,

};

static GFL_BG_SYS_HEADER BGsys_data = {
  GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

static void dispInit(WIFILOGIN_DISP_WORK* pWork);
static void dispInitDream(WIFILOGIN_DISP_WORK* pWork);
static void dispOamInit(WIFILOGIN_DISP_WORK* pWork);
static void dispOamMain(WIFILOGIN_DISP_WORK* pWork);
static void dispOamEnd(WIFILOGIN_DISP_WORK* pWork);
static void	_VBlank( GFL_TCB *tcb, void *work );
static void _moveSmoke(WIFILOGIN_DISP_WORK* pWork);

static void settingSubBgControl(WIFILOGIN_DISP_WORK* pWork,int frame);


WIFILOGIN_DISP_WORK* WIFILOGIN_DISP_Init(HEAPID id, WIFILOGIN_BG bg, WIFILOGIN_DISPLAY display )
{
  WIFILOGIN_DISP_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(WIFILOGIN_DISP_WORK));
  pWork->heapID = id;
  pWork->bg = bg;


  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GXS_DispOn();
  GX_DispOn();

  GFL_BG_Init(pWork->heapID);

  GFL_DISP_SetBank( &_defVBTbl );
  GFL_BG_SetBGMode( &BGsys_data );
  settingSubBgControl(pWork,GFL_BG_FRAME0_M);
  settingSubBgControl(pWork,GFL_BG_FRAME0_S);
  switch( pWork->bg ){
  case WIFILOGIN_BG_DREAM_WORLD:
    dispOamInit(pWork);
    dispInitDream(pWork);
    break;
  case WIFILOGIN_BG_NORMAL:
    dispInit(pWork);
    break;
  }

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_OBJ );

  return pWork;
}

void WIFILOGIN_DISP_Main(WIFILOGIN_DISP_WORK* pWork)
{
  if(pWork->bg == WIFILOGIN_BG_DREAM_WORLD){
    dispOamMain(pWork);
  }
}

void WIFILOGIN_DISP_End(WIFILOGIN_DISP_WORK* pWork)
{
  if(pWork->bg == WIFILOGIN_BG_DREAM_WORLD){
    dispOamEnd(pWork);
  }


  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME2_S, 1, 0);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_Exit();
  GFL_HEAP_FreeMemory(pWork);

}


static void settingSubBgControl(WIFILOGIN_DISP_WORK* pWork,int frameno)
{
  /*
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
  */


  
  {
    int frame = frameno;
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
    int frame = frameno+1;
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
    int frame = frameno+2;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x10000, 0x8000,GX_BG_EXTPLTT_01,
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
    int frame = frameno+3;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x18000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}

static void dispInit(WIFILOGIN_DISP_WORK* pWork)
{
  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFI_LOGIN, pWork->heapID );
  
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifi_login_conect_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifi_login_conect_sub_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifi_login_conect_sub_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);


    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifi_login_conect_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifi_login_conect_NCGR,
                                                                  GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifi_login_conect_01_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);
}

static void dispInitDream(WIFILOGIN_DISP_WORK* pWork)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GSYNC, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gsync_connect_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gsync_connect_sub_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_connect_sub_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);


    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gsync_connect_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gsync_connect_NCGR,
                                                                  GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_connect_01_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);


    pWork->cellRes[CHAR_SMOKE] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gsync_gsync_obj_NCGR ,
                              FALSE , CLSYS_DRAW_MAIN , pWork->heapID );
    pWork->cellRes[PLT_SMOKE] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gsync_gsync_obj_NCLR , CLSYS_DRAW_MAIN ,    0, 0, 3, pWork->heapID  );
    pWork->cellRes[ANM_SMOKE] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gsync_gsync_obj_NCER, NARC_gsync_gsync_obj_NANR , pWork->heapID  );

    GFL_ARC_CloseDataHandle(p_handle);

  }

}


static void dispOamInit(WIFILOGIN_DISP_WORK* pWork)
{

  GFL_CLACT_SYS_Create(	&GFL_CLSYSINIT_DEF_DIVSCREEN, &_defVBTbl, pWork->heapID );
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 40 , 0 , pWork->heapID );
  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  
}

static void dispOamMain(WIFILOGIN_DISP_WORK* pWork)
{
  _moveSmoke(pWork);
  GFL_CLACT_SYS_Main();
}


static void dispOamEnd(WIFILOGIN_DISP_WORK* pWork)
{
  int i=0;
   
  for(i = 0 ; i < _CELL_DISP_NUM ;i++){
    if(pWork->curIcon[i]!=NULL){
      GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      pWork->curIcon[i]=NULL;
    }
  }
  GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_SMOKE] );
  GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_SMOKE] );
  GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_SMOKE] );
  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();
}



//----------------------------------------------------------------------------
/**
 *	@brief	煙をセット
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void _SetSmoke(WIFILOGIN_DISP_WORK* pWork,int x,int y)
{
  int i=0;


  
  for(i = 0 ; i < _CELL_DISP_NUM ;i++){

    if(pWork->curIcon[i]==NULL){
      GFL_CLWK_DATA cellInitData;

      cellInitData.pos_x = x;
      cellInitData.pos_y = y;
      cellInitData.anmseq = NANR_gsync_obj_yume_1;
      cellInitData.softpri = 0;
      cellInitData.bgpri = 0;
      pWork->curIcon[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[CHAR_SMOKE],
                                               pWork->cellRes[PLT_SMOKE],
                                               pWork->cellRes[ANM_SMOKE],
                                               &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[i] , TRUE );
      GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], TRUE );
      if(50 > GFUser_GetPublicRand(100)){
        pWork->moveSmoke[i].rightleft = -1;
      }
      else{
        pWork->moveSmoke[i].rightleft = 1;
      }
      break;
    }
  }
}


static void _moveSmoke(WIFILOGIN_DISP_WORK* pWork)
{
  int i;


  if(pWork->smokeTimer==0){
    return;
  }
  pWork->smokeTimer++;

  if((pWork->smokeTimer % MOVESMOKE_CREATE) == 12){
    if(30 > GFUser_GetPublicRand(100)){
      _SetSmoke(pWork,128,190);
    }
  }

  for(i = 0 ; i < _CELL_DISP_NUM ;i++){
    if(pWork->curIcon[i]!=NULL){
      GFL_CLACTPOS pos;
      GFL_CLACT_WK_GetPos( pWork->curIcon[i], &pos, CLSYS_DEFREND_MAIN );
      if( pos.y < -40){
        GFL_CLACT_WK_Remove(pWork->curIcon[i]);
        pWork->curIcon[i]=NULL;
      }
      else{
        if( (pWork->smokeTimer % MOVESMOKE_SLEEPY)==0 ){
          pos.y -= GFUser_GetPublicRand(MOVESMOKE_RANDY);
        }
        if( (pWork->smokeTimer % MOVESMOKE_SLEEPX)==0 ){
          pos.x += GFUser_GetPublicRand(MOVESMOKE_RANDX) * pWork->moveSmoke[i].rightleft ;
        }
        GFL_CLACT_WK_SetPos( pWork->curIcon[i], &pos, CLSYS_DEFREND_MAIN );
      }
    }
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
  WIFILOGIN_DISP_WORK *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank

}

void WIFILOGIN_DISP_StartSmoke(WIFILOGIN_DISP_WORK* pWork)
{
  pWork->smokeTimer=1;
}

