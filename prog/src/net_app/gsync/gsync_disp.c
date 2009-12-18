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

#include "gsync_poke.cdat"
#include "gsync_obj_NANR_LBLDEFS.h"


#define _OBJPLT_POKEICON_OFFSET (0)  //ポケモンパレット
#define _OBJPLT_GSYNC_OFFSET (3)     //ゲームシンク素材
#define _OBJPLT_HAND_OFFSET (6)    //手

FS_EXTERN_OVERLAY(ui_common);

typedef enum
{
  PLT_GSYNC,
  PLT_HANDOBJ,
  PLT_POKEICON,
  PLT_RESOURCE_MAX,
  CHAR_GSYNC = PLT_RESOURCE_MAX,
  CHAR_HANDOBJ,
  CHAR_SELECT_POKE,
  CHAR_RESOURCE_MAX,
  ANM_GSYNC = CHAR_RESOURCE_MAX,
  ANM_HANDOBJ,
  ANM_POKEICON,
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

} _CELL_PRI_ENUM;




//#define _CELL_DISP_NUM (12)

struct _GSYNC_DISP_WORK {
	u32 subchar;
  u32 mainchar;
  TOUCHBAR_WORK* pTouchWork;
  GFL_CLUNIT	*cellUnit;
  GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル
  GFL_TCB *p_hblank; //HBlankハンドル

  u32 cellRes[CEL_RESOURCE_MAX];
  GFL_CLWK* curIcon[_CELL_DISP_NUM];
  HEAPID heapID;

  s16 scroll_index;
  s16 scroll[192];

  fx32 blendCount;
  int blendStart;
  int performCnt;
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
  {124,160,_BED_CELL_PRI},   //NANR_gsync_obj_bed    0 // 
  {124,160,_BED_CELL_PRI},   //NANR_gsync_obj_bed_ani    1 // 
  {132,155,_SMOKE_CELL_PRI},   //NANR_gsync_obj_kemuri_r    2 //
  {122,155,_SMOKE_CELL_PRI},   //NANR_gsync_obj_kemuri_l    3 //
  {124,125,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani1    4 //
  {124,125,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani2    5 //
  {124,125,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani3    5 //
  {124,125,_FUTON_CELL_PRI},   //NANR_gsync_obj_rug_ani4    5 //
  {100,125,_ZZZ_CELL_PRI},   //NANR_gsync_obj_zzz_ani    6 // 
  {128,160,_SMOKE_CELL_PRI},   //NANR_gsync_obj_yume_1    7 // 
};






static GFL_BG_SYS_HEADER BGsys_data = {
//    GX_DISPMODE_GRAPHICS,GX_BGMODE_3,GX_BGMODE_0,GX_BG0_AS_2D,
  GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
};

static void dispInit(GSYNC_DISP_WORK* pWork);
static void settingSubBgControl(GSYNC_DISP_WORK* pWork);
static void _TOUCHBAR_Init(GSYNC_DISP_WORK* pWork);
static void	_VBlank( GFL_TCB *tcb, void *work );
static void _SetHand(GSYNC_DISP_WORK* pWork,int x,int y);
static void _SetBed(GSYNC_DISP_WORK* pWork,int no);

static void _HandRelease(GSYNC_DISP_WORK* pWork);
static void _CreatePokeIconResource(GSYNC_DISP_WORK* pWork);
static void _blendSmoke(GSYNC_DISP_WORK* pWork);



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

  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  pWork->performCnt=200;
  
  //_TOUCHBAR_Init(pWork);

  GFL_DISP_GXS_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  GFL_DISP_GX_SetVisibleControlDirect( GX_PLANEMASK_BG0|GX_PLANEMASK_BG3|GX_PLANEMASK_OBJ );
  
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
}

void GSYNC_DISP_End(GSYNC_DISP_WORK* pWork)
{
  int i;
  _HandRelease(pWork);

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
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x8000, GX_BG_EXTPLTT_01,
      3, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( frame );
    GFL_BG_LoadScreenReq( frame );
		GFL_BG_SetVisible( frame, VISIBLE_ON );
  }
  {
    int frame = GFL_BG_FRAME3_M;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };

    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 64, 24, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
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
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
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
    int frame = GFL_BG_FRAME3_S;
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

    pWork->cellRes[CHAR_GSYNC] =
      GFL_CLGRP_CGR_Register( p_handle , NARC_gsync_gsync_obj_NCGR ,
                              FALSE , CLSYS_DRAW_MAX , pWork->heapID );
    pWork->cellRes[PLT_GSYNC] =
      GFL_CLGRP_PLTT_RegisterEx(
        p_handle ,NARC_gsync_gsync_obj_NCLR , CLSYS_DRAW_MAX, _OBJPLT_GSYNC_OFFSET*0x20, 0, 3, pWork->heapID  );
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


    _SetBed(pWork, NANR_gsync_obj_bed);
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

void GSYNC_DISP_ObjEnd(GSYNC_DISP_WORK* pWork,int no)
{
  GFL_CLACT_WK_Remove( pWork->curIcon[no] );
  pWork->curIcon[no]=NULL;
}



//----------------------------------------------------------------------------
/**
 *	@brief	TOUCHBAR初期化
 *	@param	POKEMON_TRADE_WORK
 *	@return	none
 */
//-----------------------------------------------------------------------------

static void _TOUCHBAR_Init(GSYNC_DISP_WORK* pWork)
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
  touchbar_setup.bar_frm	= GFL_BG_FRAME1_S;						//BG読み込みのためのBG面上下画面判定にも必要
  touchbar_setup.bg_plt		= _TOUCHBAR_PAL;			//BGﾊﾟﾚｯﾄ
  touchbar_setup.obj_plt	= _TOUCHBAR_PAL;			//OBJﾊﾟﾚｯﾄ
  touchbar_setup.mapping	= APP_COMMON_MAPPING_128K;	//マッピングモード
  pWork->pTouchWork = TOUCHBAR_Init(&touchbar_setup, pWork->heapID);
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
  GF_ASSERT(pWork->curIcon[no]==NULL);
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
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[no] , TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[no], TRUE );
    GFL_CLACT_WK_SetAffineParam( pWork->curIcon[no], CLSYS_AFFINETYPE_DOUBLE );

  }
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


  {
    GFL_CLWK_DATA cellInitData;
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );

    pWork->cellRes[CHAR_SELECT_POKE] =
      GFL_CLGRP_CGR_Register( arcHandlePoke , POKEICON_GetCgxArcIndex(ppp) ,
                              FALSE , CLSYS_DRAW_MAX , pWork->heapID );

    cellInitData.pos_x = _POKEMON_CELLX;
    cellInitData.pos_y = _POKEMON_CELLY;
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
      GX_LoadBGPltt(&paletteTbl[0], 62, 2);
    }
    else if(pWork->performCnt == 0){
      GX_LoadBGPltt(&paletteTbl[9], 62, 2);
    }
    else if(200 < v_c2){
      GX_LoadBGPltt(&paletteTbl[0], 62, 2);
    }
    else if(pWork->performCnt > v_c2){
      GX_LoadBGPltt(&paletteTbl[0], 62, 2);
    }
    else if(((v_c2 - pWork->performCnt) < elementof(paletteTbl))  && ((v_c2 - pWork->performCnt) >= 0)){
      GX_LoadBGPltt(&paletteTbl[v_c2 - pWork->performCnt], 62, 2);
    }
    else{
      GX_LoadBGPltt(&paletteTbl[9], 62, 2);
    }
#endif
	}



}




void GSYNC_DISP_DreamSmokeBgStart(GSYNC_DISP_WORK* pWork)
{

  ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GSYNC, pWork->heapID );

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BD , 0, 16);
  
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
    G2_BlendNone();
    pWork->blendStart = 0;
    return;
  }
  else if(i < 0){
    GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_OFF );
    G2_BlendNone();
    pWork->blendStart = 0;
    return;
  }

  G2_SetBlendAlpha( GX_BLEND_PLANEMASK_BG3, GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BD , i, 16);



}


void GSYNC_DISP_SetPerfomance(GSYNC_DISP_WORK* pWork,int percent)
{
  if(percent>100){
    pWork->performCnt = 0;
  }
  else{
    pWork->performCnt = 200- percent * 2;
  }
}

