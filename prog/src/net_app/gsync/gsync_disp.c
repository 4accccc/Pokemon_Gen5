//============================================================================================
/**
 * @file    gsync_disp.c
 * @bfief   GSYNC画像表示関連
 * @author  k.ohno
 * @date    2009.11.14
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>
#include "calctool.h"

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
#include "pokeicon/pokeicon.h"
#include "system/laster.h"

#include "net/dreamworld_netdata.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "ui/touchbar.h"

#include "../../field/event_ircbattle.h"
#include "app/app_taskmenu.h"  //APP_TASKMENU_INITWORK

#include "gsync_local.h"
#include "msg/msg_gtsnego.h"
#include "gsync.naix"
#include "box_gra.naix"
#include "item_icon.naix"

#include "gsync_poke.cdat"
#include "gsync_obj_NANR_LBLDEFS.h"


#define _OBJPLT_POKEICON_OFFSET (0)  //ポケモンパレット
#define _OBJPLT_GSYNC_OFFSET (3)     //ゲームシンク素材
#define _OBJPLT_HAND_OFFSET (7)    //手
#define _OBJPLT_ITEM_OFFSET (8)    //アイテム

FS_EXTERN_OVERLAY(ui_common);

typedef enum
{
  PLT_GSYNC,
  PLT_HANDOBJ,
  PLT_POKEICON,
  _CLACT_PLT,
  _CLACT_PLT1,
  ITEM_CLACT_PLT,
  PLT_RESOURCE_MAX,
  CHAR_GSYNC = PLT_RESOURCE_MAX,
  CHAR_HANDOBJ,
  CHAR_SELECT_POKE,
  CHAR_DOWN_POKE,
  _CLACT_CHR,
  _CLACT_CHR1,
  ITEM_CLACT_CHR,
  CHAR_RESOURCE_MAX,
  ANM_GSYNC = CHAR_RESOURCE_MAX,
  ANM_HANDOBJ,
  ANM_POKEICON,
  _CLACT_ANM,
  _CLACT_ANM1,
  _CLACT_ANM_ITEM,
  ITEM_CLACT_ANM,
  ANM_RESOURCE_MAX,
  CEL_RESOURCE_MAX,
} _CELL_RESOURCE_TYPE;


typedef enum
{
  CELL_CUR_BED,
  CELL_CUR_BED_ANI,
  CELL_CUR_KEMURI_R,
  CELL_CUR_KEMURI_L,
  CELL_CUR_RUG_ANI1 ,
  CELL_CUR_RUG_ANI2,
  CELL_CUR_RUG_ANI3,
  CELL_CUR_RUG_ANI4,
  CELL_CUR_ZZZ_ANI ,
  CELL_CUR_YUME_1  ,
  CELL_CUR_MU,
  CELL_CUR_KAGE_MU,
  CELL_CUR_KAGE_BED,
  CELL_CUR_POKE_PLAYER,
  CELL_CUR_HAND,
  CELL_CUR_HUTON,

  _CELL_DISP_NUM,
} _CELL_WK_ENUM;

typedef enum  //お互いのセルの表示順序
{
  _ZZZ_CELL_PRI,
  _SMOKE_CELL_PRI,
  _FUTON_CELL_PRI,
  _POKEMON_CELL_PRI,
  _BED_CELL_PRI,
  _SHADOW_CELL_PRI,

} _CELL_PRI_ENUM;

typedef struct {
  u8 aPal[2*16];
  u8 aCgx[4*8*4*4];
  int palno;
} ICON_RES_PARAM;


typedef struct {
  GFL_CLWK* pIcon;
  u16 startx;
  u16 starty;
  u16 sinCount1;
  u16 sinCount2;
  u16 downCount1;
  u16 on;
  u16 no;
  u16 form;
  u16 sex;
  u16 end;
} ICON_MOVE_PARAM;

//#define _CELL_DISP_NUM (12)

struct _GSYNC_DISP_WORK {
  u32 subchar;
  u32 mainchar;
  TOUCHBAR_WORK* pTouchWork;
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
  GFL_TCB *p_hblank; //HBlankハンドル
  NNSG2dPaletteData *palData;
  void* pPalOrg;

  u32 cellRes[CEL_RESOURCE_MAX];
  GFL_CLWK* curIcon[_CELL_DISP_NUM];
  GFL_CLWK* downloadPoke;
  GFL_CLWK* downloadItem;

  ICON_RES_PARAM aIconRes[DREAM_WORLD_DATA_MAX_ITEMBOX+DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX];  //アイテム+ポケモン
  ICON_MOVE_PARAM aIconParam[DREAM_WORLD_DATA_MAX_ITEMBOX+DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX];  //アイテム+ポケモン

  HEAPID heapID;

  s16 scroll_index;
  s16 scroll[192];

  GFL_CLACTPOS PokePos;
  GFL_CLACTPOS BedPos;
  BOOL bBedSyncPoke;
  fx32 blendCount;
  int blendStart;
  int performCnt;
  int GetIconApper;
  int starCount;
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


typedef struct {
  int x;
  int y;
  int pri;
} _GSYNCOBJPOS_STRUCT;




static _GSYNCOBJPOS_STRUCT gsyncObjPosTable[]={
  {128,150,_BED_CELL_PRI},   //NANR_gsync_obj_bed    0 //
  {128,150,_BED_CELL_PRI},   //NANR_gsync_obj_bed_ani    1 //
  {132,155,_SMOKE_CELL_PRI},   //NANR_gsync_obj_kemuri_r    2 //
  {122,155,_SMOKE_CELL_PRI},   //NANR_gsync_obj_kemuri_l    3 //
  {128,145,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani1    4 //
  {128,145,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani2    5 //
  {128,145,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani3    5 //
  {128,145,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani4    5 //
  {128,150,_ZZZ_CELL_PRI},   //NANR_gsync_obj_zzz_ani    6 //
  {100,125,_ZZZ_CELL_PRI},   //NANR_gsync_obj_yume_1    6 //
  {128,160,_SMOKE_CELL_PRI},   //NANR_gsync_obj_musha    7 //
  {128,160,_SHADOW_CELL_PRI},   //NANR_gsync_obj_musha_shadow    7 //
  {128,160,_SHADOW_CELL_PRI},   //NANR_gsync_obj_bed_shadow    7 //
};






static GFL_BG_SYS_HEADER BGsys_data = {
  //    GX_DISPMODE_GRAPHICS,GX_BGMODE_3,GX_BGMODE_0,GX_BG0_AS_2D,
  GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

static void dispInit(GSYNC_DISP_WORK* pWork);
static void settingSubBgControl(GSYNC_DISP_WORK* pWork);
static void	_VBlank( GFL_TCB *tcb, void *work );
static void _SetHand(GSYNC_DISP_WORK* pWork,int x,int y);
static void _SetBed(GSYNC_DISP_WORK* pWork,int no);

static void _HandRelease(GSYNC_DISP_WORK* pWork);
static void _CreatePokeIconResource(GSYNC_DISP_WORK* pWork);
static void _blendSmoke(GSYNC_DISP_WORK* pWork);
static void _pokeResChange(GSYNC_DISP_WORK* pWork,int no,  GFL_CLWK* pIcon);
static void _itemResChange(GSYNC_DISP_WORK* pWork,int no,  GFL_CLWK* pIcon);
static void _DownloadPokemonIconCreate(GSYNC_DISP_WORK* pWork);
static void _DownloadItemiconCreate(GSYNC_DISP_WORK* pWork);



GSYNC_DISP_WORK* GSYNC_DISP_Init(HEAPID id)
{
  GSYNC_DISP_WORK* pWork = GFL_HEAP_AllocClearMemory(id, sizeof(GSYNC_DISP_WORK));
  pWork->heapID = id;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(ui_common));

  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);

  GFL_DISP_SetDispSelect(GFL_DISP_3D_TO_MAIN);
  GXS_DispOn();
  GX_DispOn();

  GFL_BG_Init(pWork->heapID);
  GFL_CLACT_SYS_Create(	&GFL_CLSYSINIT_DEF_CONSCREEN, &_defVBTbl, pWork->heapID );
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 40 , 0 , pWork->heapID );

  GFL_DISP_SetBank( &_defVBTbl );
  GFL_BG_SetBGMode( &BGsys_data );
  settingSubBgControl(pWork);
  dispInit(pWork);
  _DownloadPokemonIconCreate(pWork);
  _DownloadItemiconCreate(pWork);

  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  pWork->performCnt=200;

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );

  return pWork;
}

void GSYNC_DISP_Main(GSYNC_DISP_WORK* pWork)
{
  pWork->scroll_index++;
  if(pWork->scroll_index>=192){
    pWork->scroll_index=0;
  }
  GFL_CLACT_SYS_Main();
  _blendSmoke(pWork);
  GSYNC_DISP_PokemonMove(pWork);

}

void GSYNC_DISP_End(GSYNC_DISP_WORK* pWork)
{
  int i;
  _HandRelease(pWork);

  GFL_HEAP_FreeMemory(pWork->pPalOrg);

  GFL_CLACT_WK_Remove( pWork->downloadPoke );
  GFL_CLACT_WK_Remove( pWork->downloadItem );
  
  for(i=0;i<PLT_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_PLTT_Release(pWork->cellRes[i] );
    }
  }
  for(;i<CHAR_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_CGR_Release(pWork->cellRes[i] );
    }
  }
  for(;i<ANM_RESOURCE_MAX;i++){
    if(pWork->cellRes[i] ){
      GFL_CLGRP_CELLANIM_Release(pWork->cellRes[i] );
    }
  }

  if(pWork->p_hblank){
    GFL_TCB_DeleteTask( pWork->p_hblank );
  }
  GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  GFL_CLACT_SYS_Delete();

  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_M, 1, 0);
  //  GFL_BG_FillCharacterRelease( GFL_BG_FRAME1_S, 1, 0);
  GFL_BG_FillCharacterRelease( GFL_BG_FRAME2_S, 1, 0);
  GFL_BG_FreeBGControl(GFL_BG_FRAME1_S);
  GFL_BG_FreeBGControl(GFL_BG_FRAME0_S);
  GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
  GFL_BG_Exit();
  GFL_HEAP_FreeMemory(pWork);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(ui_common));

}


static void settingSubBgControl(GSYNC_DISP_WORK* pWork)
{

  // 背景面
  {
    int frame = GFL_BG_FRAME0_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME1_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };

    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME3_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
      };

    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }

  {
    int frame = GFL_BG_FRAME0_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
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
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };

    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x18000, 0x8000,GX_BG_EXTPLTT_01,
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
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
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
  GSYNC_DISP_WORK *pWork=work;

  GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank
  pWork->starCount++;
  if((pWork->starCount % 10)==1){
    int x = (pWork->starCount / 10)+2;
    u16* pal = (u16*)pWork->palData->pRawData;
    GX_LoadBGPltt(&pal[x*16], 2*32, 32);
    GXS_LoadBGPltt(&pal[x*16], 2*32, 32);
  }
  if(pWork->starCount>=80){
    pWork->starCount=0;
  }

}

static void dispInit(GSYNC_DISP_WORK* pWork)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GSYNC, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gsync_gsync_bg_NCLR,
                                      PALTYPE_SUB_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gsync_gsync_bg_NCGR,
                                                                  GFL_BG_FRAME0_S, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_downner_bg_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_upper_bg2_NSCR,
                                              GFL_BG_FRAME3_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);


    pWork->pPalOrg = GFL_ARCHDL_UTIL_LoadPalette(p_handle, NARC_gsync_gsync_bg_NCLR, &pWork->palData, pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_gsync_gsync_bg_NCLR,
                                      PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);
    // サブ画面BG0キャラ転送
    pWork->mainchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_gsync_gsync_bg_NCGR,
                                                                   GFL_BG_FRAME0_M, 0, 0, pWork->heapID);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_upper_bg_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_gsync_upper_bg2_NSCR,
                                              GFL_BG_FRAME1_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
                                              pWork->heapID);

    pWork->cellRes[CHAR_GSYNC] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gsync_gsync_obj_NCGR ,
                              FALSE , CLSYS_DRAW_MAX , pWork->heapID );
    pWork->cellRes[PLT_GSYNC] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gsync_gsync_obj_NCLR , CLSYS_DRAW_MAX, _OBJPLT_GSYNC_OFFSET*0x20, 0, 4, pWork->heapID  );
    pWork->cellRes[ANM_GSYNC] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gsync_gsync_obj_NCER, NARC_gsync_gsync_obj_NANR , pWork->heapID  );


    pWork->cellRes[CHAR_HANDOBJ] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gsync_hand_obj_NCGR ,
                              FALSE , CLSYS_DRAW_MAX , pWork->heapID );
    pWork->cellRes[PLT_HANDOBJ] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gsync_hand_obj2_NCLR , CLSYS_DRAW_MAX ,  _OBJPLT_HAND_OFFSET*0x20, 0, 0, pWork->heapID  );
    pWork->cellRes[ANM_HANDOBJ] =
      GFL_CLGRP_CELLANIM_Register(
        p_handle , NARC_gsync_hand_obj_NCER, NARC_gsync_hand_obj_NANR , pWork->heapID  );


    _CreatePokeIconResource(pWork);

    GFL_ARC_CloseDataHandle(p_handle);
  }

}


void GSYNC_DISP_HandInit(GSYNC_DISP_WORK* pWork)
{
  _SetHand(pWork,_POKEMON_CELLX, _POKEMON_CELLY);
}


void GSYNC_DISP_ObjInit(GSYNC_DISP_WORK* pWork,int no)
{
  _SetBed(pWork, no);
}


void GSYNC_DISP_ObjChange(GSYNC_DISP_WORK* pWork,int no,int no2)
{
  GFL_CLACT_WK_SetAnmSeq( pWork->curIcon[no] , no2 );
}

void GSYNC_DISP_SetCallback(GSYNC_DISP_WORK* pWork,int no,const GFL_CLWK_ANM_CALLBACK* cp_data)
{
  GFL_CLACT_WK_StartAnmCallBack( pWork->curIcon[no] , cp_data );
}

void GSYNC_DISP_ResetCallback(GSYNC_DISP_WORK* pWork,int no)
{
  GFL_CLACT_WK_StopAnmCallBack( pWork->curIcon[no] );
}



void GSYNC_DISP_ObjEnd(GSYNC_DISP_WORK* pWork,int no)
{
  if(pWork->curIcon[no]){
    GFL_CLACT_WK_Remove( pWork->curIcon[no] );
    pWork->curIcon[no]=NULL;
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBARWORKをえる
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


TOUCHBAR_WORK* GSYNC_DISP_GetTouchWork(GSYNC_DISP_WORK* pWork)
{
  return pWork->pTouchWork;
}


//----------------------------------------------------------------------------
/**
 *	@brief	→設定
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void _SetHand(GSYNC_DISP_WORK* pWork,int x,int y)
{
  int i=0;




  if(pWork->curIcon[CELL_CUR_HAND]==NULL){
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = x;
    cellInitData.pos_y = y;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 1;
    pWork->curIcon[CELL_CUR_HAND] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                         pWork->cellRes[CHAR_HANDOBJ],
                                                         pWork->cellRes[PLT_HANDOBJ],
                                                         pWork->cellRes[ANM_HANDOBJ],
                                                         &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_HAND] , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_HAND], TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	→設定
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void _SetBed(GSYNC_DISP_WORK* pWork,int no)
{
  if(pWork->curIcon[no]){
    GFL_CLACT_WK_Remove(pWork->curIcon[no]);

  }
  {
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = gsyncObjPosTable[no].x;
    cellInitData.pos_y = gsyncObjPosTable[no].y;
    cellInitData.anmseq = no;
    cellInitData.softpri = gsyncObjPosTable[no].pri;
    cellInitData.bgpri = 1;
    pWork->curIcon[no] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->cellRes[CHAR_GSYNC],
                                              pWork->cellRes[PLT_GSYNC],
                                              pWork->cellRes[ANM_GSYNC],
                                              &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
    GFL_CLACT_WK_SetAnmFrame(pWork->curIcon[no],0);
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[no] , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[no], TRUE );
    GFL_CLACT_WK_SetAffineParam( pWork->curIcon[no], CLSYS_AFFINETYPE_DOUBLE );

  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ベッドとポケモンの同期処理開始
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


void GSYNC_DISP_BedSyncPokemonStart(GSYNC_DISP_WORK* pWork)
{
  GFL_CLACT_WK_SetAnmSeq( pWork->curIcon[CELL_CUR_POKE_PLAYER] , 2 );
}

//----------------------------------------------------------------------------
/**
 *	@brief	→開放
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _HandRelease(GSYNC_DISP_WORK* pWork)
{
  int i=0;

  for(i = 0 ; i < _CELL_DISP_NUM ;i++){
    if(pWork->curIcon[i]!=NULL){
      GFL_CLACT_WK_Remove(pWork->curIcon[i]);
      pWork->curIcon[i]=NULL;
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモンアイコンリソースの作成
 * @param   POKEMON_TRADE_WORK
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _CreatePokeIconResource(GSYNC_DISP_WORK* pWork)
{
  {
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
    pWork->cellRes[PLT_POKEICON] =
      GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke ,
                                   POKEICON_GetPalArcIndex() , CLSYS_DRAW_MAX ,
                                   _OBJPLT_POKEICON_OFFSET , pWork->heapID  );
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
  {
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_GSYNC , pWork->heapID );
    pWork->cellRes[ANM_POKEICON] =
      GFL_CLGRP_CELLANIM_Register( arcHandlePoke ,
                                   NARC_gsync_poke_icon_128k_NCER, NARC_gsync_poke_icon_128k_NANR, pWork->heapID  );
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンアイコンCreate
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


void GSYNC_DISP_PokemonIconCreate(GSYNC_DISP_WORK* pWork, POKEMON_PASO_PARAM* ppp, int draw)
{

  int x[] = { _POKEMON_CELLX, _POKEMON_CELLX-4};
  int y[] = { _POKEMON_CELLY,_POKEMON_CELLY+4};
  
  {
    GFL_CLWK_DATA cellInitData;
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );

    pWork->cellRes[CHAR_SELECT_POKE] =
      GFL_CLGRP_CGR_Register( arcHandlePoke , POKEICON_GetCgxArcIndex(ppp) ,
                              FALSE , CLSYS_DRAW_MAX , pWork->heapID );

    if(draw== CLSYS_DRAW_MAIN){
      cellInitData.pos_x = x[1];
      cellInitData.pos_y = y[1];
    }
    else{
      cellInitData.pos_x = x[0];
      cellInitData.pos_y = y[0];
    }
    cellInitData.anmseq = 0;
    cellInitData.softpri = _POKEMON_CELL_PRI;
    cellInitData.bgpri = 1;
    pWork->curIcon[CELL_CUR_POKE_PLAYER] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                                pWork->cellRes[CHAR_SELECT_POKE],
                                                                pWork->cellRes[PLT_POKEICON],
                                                                pWork->cellRes[ANM_POKEICON],
                                                                &cellInitData ,draw , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_POKE_PLAYER] , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[CELL_CUR_POKE_PLAYER], TRUE );
    GFL_CLACT_WK_SetPlttOffs( pWork->curIcon[CELL_CUR_POKE_PLAYER] , POKEICON_GetPalNumGetByPPP( ppp ) , CLWK_PLTTOFFS_MODE_PLTT_TOP );

    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
}


void GSYNC_DISP_PokemonIconMove(GSYNC_DISP_WORK* pWork)
{
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_POKE_PLAYER] , TRUE );
}

void GSYNC_DISP_PokemonIconJump(GSYNC_DISP_WORK* pWork)
{
  GFL_CLACT_WK_SetAnmSeq( pWork->curIcon[CELL_CUR_POKE_PLAYER] , 1 );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[CELL_CUR_POKE_PLAYER] , TRUE );
}





static u16 paletteTbl[]={0x5ab6,0x5ef7,0x6318,0x6739,0x6b5a,0x6f7b,0x739c,0x77bd,0x7bde,0x7fff };

#define PALETTEPOS (32*0xA+2)

static void dreamSmoke_HBlank( GFL_TCB* p_tcb, void* p_work )
{
  GSYNC_DISP_WORK* pWork = p_work;
  int v_c,v_c2;

  v_c2 = GX_GetVCount();
  v_c = v_c2;

  v_c += (pWork->scroll_index + 1);
  v_c %= 192;

  if( GX_IsHBlank() ){
    G2_SetBG3Offset( pWork->scroll[v_c], 0 );
#if 1
    if(pWork->performCnt >= 192){
      GX_LoadBGPltt(&paletteTbl[0], PALETTEPOS, 2);
    }
    else if(pWork->performCnt == 0){
      GX_LoadBGPltt(&paletteTbl[9], PALETTEPOS, 2);
    }
    else if(200 < v_c2){
      GX_LoadBGPltt(&paletteTbl[0], PALETTEPOS, 2);
    }
    else if(pWork->performCnt > v_c2){
      GX_LoadBGPltt(&paletteTbl[0], PALETTEPOS, 2);
    }
    else if(((v_c2 - pWork->performCnt) < elementof(paletteTbl))  && ((v_c2 - pWork->performCnt) >= 0)){
      GX_LoadBGPltt(&paletteTbl[v_c2 - pWork->performCnt], PALETTEPOS, 2);
    }
    else{
      GX_LoadBGPltt(&paletteTbl[9], PALETTEPOS, 2);
    }
#endif
  }



}




void GSYNC_DISP_DreamSmokeBgStart(GSYNC_DISP_WORK* pWork)
{

  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GSYNC, pWork->heapID );

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(
    p_handle, NARC_gsync_downner_bg2_NSCR,
    GFL_BG_FRAME3_M, 0,
    GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainchar), 0, 0,
    pWork->heapID);

  GFL_ARC_CloseDataHandle(p_handle);


  pWork->p_hblank = GFUser_HIntr_CreateTCB( dreamSmoke_HBlank, pWork, 0 );
  LASTER_ScrollMakeSinTbl( pWork->scroll, 192, GFL_CALC_GET_ROTA_NUM(8), 1.5*FX32_ONE );


}



void GSYNC_DISP_BlendSmokeStart(GSYNC_DISP_WORK* pWork,BOOL bFadein)
{
  if(bFadein){
    pWork->blendCount = 0;
    pWork->blendStart = 1;
  }
  else{
    pWork->blendCount =16*FX32_ONE;
    pWork->blendStart =-1;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	→開放
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _blendSmoke(GSYNC_DISP_WORK* pWork)
{
  int i=0;

  if(pWork->blendStart==0){
    return;
  }
  pWork->blendCount +=  _SMOKE_FADE_TIME * pWork->blendStart;
  i = FX_Whole(pWork->blendCount);

  GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );
  if(i > 10){
    //    G2_BlendNone();
    pWork->blendStart = 0;
    return;
  }
  else if(i < 0){
    GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
    //  G2_BlendNone();
    pWork->blendStart = 0;
    return;
  }

  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG3,
                   GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|
                   GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
                   i,8);


}


void GSYNC_DISP_SetPerfomance(GSYNC_DISP_WORK* pWork,int percent)
{
  float num = percent;
  int cnt;

  num = num * 1.2;
  if(percent > 100){
    cnt = 0;
  }
  else{
    cnt = 120 - num;
  }
  if(pWork->performCnt > cnt){
    pWork->performCnt = cnt;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	ポケモンアイコンCreate
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void GSYNC_DISP_FriendPokeIconCreate(GSYNC_DISP_WORK* pWork, u32 index)
{
  {
    _pokeResChange(pWork,index,pWork->downloadPoke);

    {
      GFL_CLWK* pIcon = pWork->downloadPoke;
      GFL_CLACTPOS pos={128,0};
      GFL_CLSCALE scale={FX32_ONE,FX32_ONE};
      
      GFL_CLACT_WK_SetPos( pIcon ,&pos, CLSYS_DEFREND_MAIN);
      GFL_CLACT_WK_SetScale( pIcon ,&scale);
      GFL_CLACT_WK_SetAutoAnmFlag( pIcon , FALSE );
      GFL_CLACT_WK_SetDrawEnable(pIcon, FALSE );
      GFL_CLACT_WK_SetObjMode( pIcon, GX_OAM_MODE_XLU );
      GFL_CLACT_WK_SetAffineParam( pIcon, CLSYS_AFFINETYPE_DOUBLE );
      pWork->aIconParam[index].pIcon=pIcon;
    }
  }
//  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG3,
  //                 GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|
    //               GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
      //             8,8);

}


//-----------------------------------------------------------------------------
/**
 *  @brief  アイコン 消去
 */
//-----------------------------------------------------------------------------



static void GSYNC_DISP_IconOff(ICON_MOVE_PARAM* pIconMove)
{
  if(pIconMove->pIcon){
    GFL_CLACT_WK_SetDrawEnable(pIconMove->pIcon, FALSE );
  }
  pIconMove->pIcon=NULL;
//  pIconMove->end=TRUE;    //ここでは、まだエンドにしない　100708 del Saito  BTS7584
}




//-----------------------------------------------------------------------------
/**
 *  @brief  アイテムアイコン表示
 */
//-----------------------------------------------------------------------------

static void GSYNC_DISP_ItemiconCreate(GSYNC_DISP_WORK* pWork, int index)
{

  {
    _itemResChange(pWork,index,pWork->downloadItem);

    {
      GFL_CLWK* pIcon = pWork->downloadItem;
      GFL_CLACTPOS pos={156,28};
      GFL_CLSCALE scale={FX32_ONE,FX32_ONE};
      
      GFL_CLACT_WK_SetPos( pIcon ,&pos, CLSYS_DEFREND_MAIN);
      GFL_CLACT_WK_SetScale( pIcon ,&scale);
      GFL_CLACT_WK_SetAutoAnmFlag( pIcon , FALSE );
      GFL_CLACT_WK_SetDrawEnable(pIcon, FALSE );
      GFL_CLACT_WK_SetObjMode( pIcon, GX_OAM_MODE_XLU );
      GFL_CLACT_WK_SetAffineParam( pIcon, CLSYS_AFFINETYPE_DOUBLE );
      pWork->aIconParam[index].pIcon=pIcon;
    }
  }


//  G2_SetBlendAlpha(GX_BLEND_PLANEMASK_BG3,
  //                 GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|
    //               GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ|GX_BLEND_PLANEMASK_BD,
      //             8,8);
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アイテムアイコン 消去
 */
//-----------------------------------------------------------------------------





static void _PokemonMove(ICON_MOVE_PARAM* pIcon)
{

  if(pIcon){
    GFL_CLACTPOS pos;
    GFL_CLSCALE scale;

    fx16 af = FX_SinIdx(pIcon->sinCount1)*4;
    fx16 bf = FX_SinIdx(pIcon->sinCount2)*3;

    af = af >> FX16_SHIFT;
    bf = bf >> FX16_SHIFT;

    if(pIcon->pIcon){
      GFL_CLACT_WK_GetPos(pIcon->pIcon, &pos, CLSYS_DEFREND_MAIN);
      pos.x = pIcon->startx + af + bf;
      pos.y = pIcon->starty + af + bf + pIcon->downCount1>>2;

      if(pIcon->downCount1 % 8==0){
        if(pIcon->startx > 128){
          pIcon->startx = pIcon->startx - 1;
        }
        else{
          pIcon->startx = pIcon->startx + 1;
        }
      }
      GFL_CLACT_WK_SetPos(pIcon->pIcon, &pos, CLSYS_DEFREND_MAIN);
      GFL_CLACT_WK_SetDrawEnable(pIcon->pIcon,TRUE);
      pIcon->downCount1++;
      if(pos.y > 25){
        GFL_CLACT_WK_GetScale(pIcon->pIcon , &scale );
        scale.x -= 1<<4;
        scale.y -= 1<<4;
        if((scale.x < 0) || (scale.y < 0)){
          GSYNC_DISP_IconOff(pIcon);
        }
        else{
          GFL_CLACT_WK_SetScale(pIcon->pIcon ,&scale);
        }
      }
      if(pos.y > 50){
        GSYNC_DISP_IconOff(pIcon);
      }
    }
#if 0        //100708 mod Saito  BTS7584
    pIcon->sinCount1 += 200 + GFUser_GetPublicRand(12);
    pIcon->sinCount2 -= 666 + GFUser_GetPublicRand(12);
#else
    else    //アイコンアクターがＮＵＬＬになって初めてエンドにする
    {
      pIcon->end = TRUE;
    }
    
    if ( !pIcon->end )
    {
      pIcon->sinCount1 += 200 + GFUser_GetPublicRand(12);
      pIcon->sinCount2 -= 666 + GFUser_GetPublicRand(12);
    }
#endif    
  }
}


void GSYNC_DISP_PokemonMove(GSYNC_DISP_WORK* pWork)
{
  int i,k,cnt=0;
  int rand;
  BOOL flg = FALSE;



  for(i = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
    if(pWork->aIconParam[i].on && !pWork->aIconParam[i].end){
      _PokemonMove( &pWork->aIconParam[i] );
      flg = TRUE;
    }
  }
  if(flg==FALSE){
    for(i = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX;i++){
      rand = GFUser_GetPublicRand(DREAM_WORLD_DATA_MAX_ITEMBOX);
      if((pWork->aIconParam[rand].on==FALSE) && (pWork->aIconParam[rand].no != 0) ){

        OS_TPrintf("アイテム絵の切り替え %d\n",rand);

        pWork->aIconParam[rand].on = TRUE;
        pWork->aIconParam[rand].startx = 64 + GFUser_GetPublicRand(128);
        pWork->aIconParam[rand].starty = 0;
        GSYNC_DISP_ItemiconCreate(pWork, rand);
        break;
      }
    }
  }

  for(i = 0; i < DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX;i++){
    k = i + DREAM_WORLD_DATA_MAX_ITEMBOX;
    if(pWork->aIconParam[k].on && !pWork->aIconParam[k].end){
      _PokemonMove( &pWork->aIconParam[k] );
      flg = TRUE;
    }
  }
  if(flg==FALSE){
    for(i = 0; i < DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX;i++){
      rand = GFUser_GetPublicRand(DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX) + DREAM_WORLD_DATA_MAX_ITEMBOX;
      if((pWork->aIconParam[rand].on==FALSE) && (pWork->aIconParam[rand].no != 0) ){

        OS_TPrintf("ぽけ絵の切り替え %d\n",rand);

        pWork->aIconParam[rand].on = TRUE;
        pWork->aIconParam[rand].startx = 64 + GFUser_GetPublicRand(128);
        pWork->aIconParam[rand].starty = 0;
        GSYNC_DISP_FriendPokeIconCreate(pWork,rand);
        break;
      }
    }
  }
}

void GSYNC_DISP_MoveIconPokeAdd(GSYNC_DISP_WORK* pWork,int index, int no, int form, int sex)
{
  if((no!=0) && (no <= MONSNO_ARUSEUSU)){  // データ読み込み・破棄
    pWork->aIconParam[index].no = no;
    pWork->aIconParam[index].form = form;
    if(sex > 2){
      sex=0;
    }
    pWork->aIconParam[index].sex = sex;
  }
}

void GSYNC_DISP_MoveIconItemAdd(GSYNC_DISP_WORK* pWork,int index, int no)
{
  pWork->aIconParam[index].no = no;
}



void GSYNC_DISP_DownloadFileRead(GSYNC_DISP_WORK* pWork)
{
  NNSG2dCharacterData* pCharData;
  NNSG2dPaletteData* pPalData;
  void* arcData;
  {
    int i;
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_ITEMICON, pWork->heapID );

#if 1

    for(i = 0; i < DREAM_WORLD_DATA_MAX_ITEMBOX ; i++){
      if(pWork->aIconParam[i].no != 0){
        int itemid = pWork->aIconParam[i].no;
//        pWork->aIconRes[i].pPal = GFL_ARC_LoadDataAllocByHandle( p_handle ,ITEM_GetIndex(itemid,ITEM_GET_ICON_PAL), pWork->heapID );

     //   OS_TPrintf("%d %d %d -<>\n",itemid,ITEM_GetIndex(itemid,ITEM_GET_ICON_PAL),ITEM_GetIndex(itemid,ITEM_GET_ICON_CGX));
        
        arcData = GFL_ARC_LoadDataAllocByHandle( p_handle ,ITEM_GetIndex(itemid,ITEM_GET_ICON_PAL), pWork->heapID );

        NNS_G2dGetUnpackedPaletteData(arcData, &pPalData);

        GFL_STD_MemCopy(pPalData->pRawData, pWork->aIconRes[i].aPal, 16*2);
        GFL_HEAP_FreeMemory(arcData);

        arcData = GFL_ARC_LoadDataAllocByHandle( p_handle ,ITEM_GetIndex(itemid,ITEM_GET_ICON_CGX) ,pWork->heapID );
        NNS_G2dGetUnpackedBGCharacterData( arcData, &pCharData );
        GFL_STD_MemCopy(pCharData->pRawData, pWork->aIconRes[i].aCgx, 4*8*4*4);
        GFL_HEAP_FreeMemory(arcData);

        OS_TPrintf("アイテム登録 %d\n",i);

      }
    }
#endif
    GFL_ARC_CloseDataHandle( p_handle );
    
  }
  {
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
    int i;


    
    for(i = 0; i < DREAM_WORLD_SERVER_DOWNLOADPOKE_MAX ; i++){
      int j=i+DREAM_WORLD_DATA_MAX_ITEMBOX;
      int mons_no = pWork->aIconParam[j].no;
      if(mons_no!=0){
        int form_no = pWork->aIconParam[j].form;
        int sex = pWork->aIconParam[j].sex;
        u8 form =  POKETOOL_CheckPokeFormNo(  mons_no,  form_no );
        u8 pal = POKEICON_GetPalNum( mons_no,form,sex, FALSE );
        u16 charno = POKEICON_GetCgxArcIndexByMonsNumber(mons_no,form,sex, FALSE);
        arcData = GFL_ARC_LoadDataAllocByHandle( arcHandlePoke ,charno,pWork->heapID );
        NNS_G2dGetUnpackedBGCharacterData( arcData, &pCharData );
        GFL_STD_MemCopy(pCharData->pRawData,pWork->aIconRes[j].aCgx, 4*8*4*4);
        GFL_HEAP_FreeMemory(arcData);
        pWork->aIconRes[j].palno = pal;

        OS_TPrintf("ぽけ登録 %d\n",j);


      }
    }
    GFL_ARC_CloseDataHandle(arcHandlePoke);


  }

}

//リソース変更
static void _pokeResChange(GSYNC_DISP_WORK* pWork,int no,  GFL_CLWK* pIcon)
{
	void *obj_vram = G2_GetOBJCharPtr();
  NNSG2dImageProxy aproxy;

   GFL_CLACT_WK_GetImgProxy(pIcon, &aproxy );

  {
    GFL_STD_MemCopy(pWork->aIconRes[no].aCgx ,
                    (char*)((u32)obj_vram) + aproxy.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_2DMAIN],
                    4*8*4*4);
  }
  NET_PRINT("%d \n",pWork->aIconRes[no].palno);
  GFL_CLACT_WK_SetPlttOffs( pIcon , pWork->aIconRes[no].palno , CLWK_PLTTOFFS_MODE_PLTT_TOP );
}


//リソース変更+パレット変更
static void _itemResChange(GSYNC_DISP_WORK* pWork,int no,  GFL_CLWK* pIcon)
{
  NNSG2dImageProxy aproxy;
	void *obj_vram = G2_GetOBJCharPtr();

  GFL_CLACT_WK_GetImgProxy(pIcon, &aproxy );
  {
    GFL_STD_MemCopy(pWork->aIconRes[no].aCgx ,
                    (char*)((u32)obj_vram) + aproxy.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_2DMAIN],
                    4*8*4*4);
  }
  DC_FlushRange(pWork->aIconRes[no].aPal, 16*2);
  GX_LoadOBJPltt(pWork->aIconRes[no].aPal, _OBJPLT_ITEM_OFFSET*16*2 ,16*2);
}




//----------------------------------------------------------------------------
/**
 *	@brief	ダウンロード時に使うポケモンアイコンCreate 仮のもの
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------


static void _DownloadPokemonIconCreate(GSYNC_DISP_WORK* pWork)
{
  {
    GFL_CLWK_DATA cellInitData;
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
    u16 charno = POKEICON_GetCgxArcIndexByMonsNumber(1,0,0, FALSE);

    pWork->cellRes[CHAR_DOWN_POKE] =
      GFL_CLGRP_CGR_Register( arcHandlePoke , charno,
                              FALSE , CLSYS_DRAW_MAIN , pWork->heapID );

    cellInitData.pos_x = _POKEMON_CELLX;
    cellInitData.pos_y = _POKEMON_CELLY;
    cellInitData.anmseq = 0;
    cellInitData.softpri = _POKEMON_CELL_PRI;
    cellInitData.bgpri = 2;
    pWork->downloadPoke = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                          pWork->cellRes[CHAR_DOWN_POKE],
                                                          pWork->cellRes[PLT_POKEICON],
                                                          pWork->cellRes[ANM_POKEICON],
                                                          &cellInitData ,CLSYS_DRAW_MAIN , pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->downloadPoke , FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->downloadPoke, FALSE );
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
}



//-----------------------------------------------------------------------------
/**
 *  @brief  ダウンロード時に使うアイテムアイコン作成
 */
//-----------------------------------------------------------------------------
static void _DownloadItemiconCreate(GSYNC_DISP_WORK* pWork)
{
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_ITEMICON, pWork->heapID );
    int itemid=1;

    pWork->cellRes[ITEM_CLACT_PLT] = GFL_CLGRP_PLTT_RegisterEx( p_handle ,
                                                                 ITEM_GetIndex(itemid,ITEM_GET_ICON_PAL) ,
                                                                 CLSYS_DRAW_MAIN ,
                                                                 (_OBJPLT_ITEM_OFFSET)*0x20, 0, 1 , pWork->heapID );
    pWork->cellRes[ITEM_CLACT_CHR] = GFL_CLGRP_CGR_Register( p_handle ,
                                                              ITEM_GetIndex(itemid,ITEM_GET_ICON_CGX) ,
                                                              FALSE , CLSYS_DRAW_MAIN , pWork->heapID );
    pWork->cellRes[ITEM_CLACT_ANM] = GFL_CLGRP_CELLANIM_Register( p_handle ,
                                                                   NARC_item_icon_itemicon_NCER ,
                                                                   NARC_item_icon_itemicon_NANR ,
                                                                   pWork->heapID );

    GFL_ARC_CloseDataHandle( p_handle );
  }

  {
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = 156;
    cellInitData.pos_y = 28;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 2;
    pWork->downloadItem = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->cellRes[ITEM_CLACT_CHR],
                                               pWork->cellRes[ITEM_CLACT_PLT],
                                               pWork->cellRes[ITEM_CLACT_ANM],
                                               &cellInitData ,
                                               CLSYS_DRAW_MAIN ,
                                               pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->downloadItem, FALSE );
    GFL_CLACT_WK_SetDrawEnable( pWork->downloadItem, FALSE );
  }
}

