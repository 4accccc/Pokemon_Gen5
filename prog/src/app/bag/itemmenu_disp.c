//============================================================================================
/**
 * @file	  itemmenu_disp.c
 * @brief	  アイテムメニューの描画周り
 * @author	k.ohno
 * @date	  2009.07.19
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "app/app_menu_common.h"
#include "app_menu_common.naix"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"

#include "message.naix"
#include "bag.naix"
#include "item_icon.naix"
#include "msg/msg_d_field.h"
#include "msg/msg_bag.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "field/fieldmap.h"
#include "font/font.naix" //NARC_font_large_nftr
#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "itemmenu.h"
#include "itemmenu_local.h"
#include "app/itemuse.h"
#include "savedata/mystatus.h"
#include "itemmenu_local.h"

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "system/main.h"			//GFL_HEAPID_APP参照



#define ITEMREPORT_FRAME (GFL_BG_FRAME2_S)
#define _UP_ITEMNAME_INITX (9)
#define _UP_ITEMNAME_INITY (6)
#define _UP_ITEMNAME_SIZEX (12)
#define _UP_ITEMNAME_SIZEY (3)


#define _UP_ITEMNUM_INITX (23)
#define _UP_ITEMNUM_INITY (6)
#define _UP_ITEMNUM_SIZEX (6)
#define _UP_ITEMNUM_SIZEY (3)

#define _UP_ITEMREPORT_INITX (2)
#define _UP_ITEMREPORT_INITY (12)
#define _UP_ITEMREPORT_SIZEX (26)
#define _UP_ITEMREPORT_SIZEY (9)

#define _ITEMICON_SCR_X (14)
#define _ITEMICON_SCR_Y (8)


#define ITEM_LIST_NUM (8)


typedef enum{
  _CLACT_PLT,
  _CLACT_CHR,
  _CLACT_ANM,
} _BAG_CLACT_TYPE;


static void _itemiconAnim(FIELD_ITEMMENU_WORK* pWork,int itemid);


//------------------------------------------------------------------------------
/**
 * @brief   絵の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

void _createSubBg(void)
{
  GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000,
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
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000,
      0x8000,
      GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME1_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
    GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );

  }
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000,
      0x8000,
      GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME2_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
    GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );

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
    int frame = GFL_BG_FRAME2_S;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x10000, 0x8000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl(
      frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_LoadScreenReq( frame );
  }
}

#define FIELD_CLSYS_RESOUCE_MAX		(100)


//--------------------------------------------------------------
///	セルアクター　初期化データ
//--------------------------------------------------------------
static const GFL_CLSYS_INIT fldmapdata_CLSYS_Init =
{
  0, 0,
  0, 512,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL,
  GFL_CLSYS_OAMMAN_INTERVAL, 128-GFL_CLSYS_OAMMAN_INTERVAL, //通信アイコン部分
  0,
  FIELD_CLSYS_RESOUCE_MAX,
  FIELD_CLSYS_RESOUCE_MAX,
  FIELD_CLSYS_RESOUCE_MAX,
  FIELD_CLSYS_RESOUCE_MAX,
  16, 16,
};

static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
  GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
  GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
  GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
  GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,			// テクスチャパレットスロット
  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_32K,
};

//------------------------------------------------------------------------------
/**
 * @brief   絵の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

void _graphicInit(FIELD_ITEMMENU_WORK* pWork)
{
  G2_BlendNone();

  GFL_BG_Init(pWork->heapID);
  GFL_BMPWIN_Init(pWork->heapID);
  GFL_FONTSYS_Init();
  GFL_DISP_SetBank( &_defVBTbl );
  GFL_CLACT_SYS_Create(	&fldmapdata_CLSYS_Init, &_defVBTbl, pWork->heapID );

  {
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }
  GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);
  _createSubBg();


  GFL_FONTSYS_SetDefaultColor();
  pWork->SysMsgQue = PRINTSYS_QUE_Create( pWork->heapID );


  //	pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(DEBUG_ITEMDISP_FRAME,
  //																						 _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);


  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_BAG, pWork->heapID );

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_bag_bag_basebg_u_NCLR,
                                      PALTYPE_SUB_BG, 0, 4*0x20,  pWork->heapID);
    pWork->subbg = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_bag_bag_basebg_u_NCGR,
                                                                GFL_BG_FRAME0_S, 0, 0, pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( p_handle, NARC_bag_bag_basebg_u_NSCR,
                                            GFL_BG_FRAME0_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subbg), 0, 0, pWork->heapID);


    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_bag_bag_basebg_d_NCLR,
                                      PALTYPE_MAIN_BG, 0, 4*0x20,  pWork->heapID);
    pWork->mainbg = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_bag_bag_basebg_d_NCGR,
                                                                 GFL_BG_FRAME0_M, 0, 0, pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( p_handle, NARC_bag_bag_basebg_d_NSCR,
                                            GFL_BG_FRAME0_M, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainbg), 0, 0, pWork->heapID);

    GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_bag_bag_win01_u_NCLR,
                                      PALTYPE_SUB_BG, 4*0x20, 4*0x20,  pWork->heapID);
    pWork->subbg2 = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_bag_bag_win01_u_NCGR,
                                                                 GFL_BG_FRAME1_S, 0, 0, pWork->heapID);
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( p_handle, NARC_bag_bag_win01_u_NSCR,
                                            GFL_BG_FRAME1_S, 0,
                                            GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subbg2), 0, 0, pWork->heapID);

    GFL_ARC_CloseDataHandle(p_handle);
  }

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

  pWork->cellUnit = GFL_CLACT_UNIT_Create( _CELLUNIT_NUM , 0 , pWork->heapID );

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
  GFL_DISP_GX_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

  {
    ARCHANDLE *archandle = GFL_ARC_OpenDataHandle( ARCID_APP_MENU_COMMON , pWork->heapID );
    //下画面バー
    GFL_ARCHDL_UTIL_TransVramPalette( archandle , NARC_app_menu_common_menu_bar_NCLR , 
                                      PALTYPE_MAIN_BG , 8*32 , 32 , pWork->heapID );
    pWork->barbg = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( archandle , NARC_app_menu_common_menu_bar_NCGR ,
                                                                GFL_BG_FRAME2_M , 0 , 0, pWork->heapID );
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( archandle , NARC_app_menu_common_menu_bar_NSCR , 
                                            GFL_BG_FRAME2_M , 0 ,
                                            0x8000+GFL_ARCUTIL_TRANSINFO_GetPos(pWork->barbg),0, 0, pWork->heapID );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
    
    GFL_ARC_CloseDataHandle(archandle);
  }
}

void ITEMDISP_graphicDelete(FIELD_ITEMMENU_WORK* pWork)
{

  GFL_BG_FreeCharacterArea(GFL_BG_FRAME0_S,
                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subbg),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subbg));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,
                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subbg2),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subbg2));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME0_M,
                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->mainbg),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->mainbg));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_M,
                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->barbg),
													 GFL_ARCUTIL_TRANSINFO_GetSize(pWork->barbg));

}

//------------------------------------------------------------------------------
/**
 * @brief   上画面のメッセージを再描画
 * @retval  none
 */
//------------------------------------------------------------------------------

void ITEMDISP_upMessageRewrite(FIELD_ITEMMENU_WORK* pWork)
{
  ITEM_ST * item = MYITEM_PosItemGet( pWork->pMyItem, pWork->pocketno, pWork->curpos );
  if((item==NULL) || (item->id==ITEM_DUMMY_DATA)){
    return;
  }


  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->winItemName), 0 );
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->winItemNum), 0 );
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->winItemReport), 0 );
  
  GFL_FONTSYS_SetDefaultColor();

  GFL_MSG_GetString(  pWork->MsgManager, MSG_ITEM_STR001, pWork->pStrBuf );
  WORDSET_RegisterItemName(pWork->WordSet, 0, item->id);
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->winItemName), 0, 0, pWork->pExpStrBuf, pWork->fontHandle);


  GFL_MSG_GetString(  pWork->MsgManager, MSG_ITEM_STR002, pWork->pStrBuf );

  WORDSET_RegisterNumber(pWork->WordSet, 0, item->no,
                         3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->winItemNum), 0, 0, pWork->pExpStrBuf, pWork->fontHandle);


  GFL_MSG_GetString(  pWork->MsgManagerItemInfo, item->id, pWork->pStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->winItemReport), 0, 0, pWork->pStrBuf, pWork->fontHandle);

  GFL_BMPWIN_TransVramCharacter(pWork->winItemName);
  GFL_BMPWIN_TransVramCharacter(pWork->winItemNum);
  GFL_BMPWIN_TransVramCharacter(pWork->winItemReport);
  GFL_BG_LoadScreenV_Req(ITEMREPORT_FRAME);

  _itemiconAnim(pWork, item->id);

}


//------------------------------------------------------------------------------
/**
 * @brief   上画面のメッセージの開放
 * @retval  none
 */
//------------------------------------------------------------------------------

void ITEMDISP_upMessageDelete(FIELD_ITEMMENU_WORK* pWork)
{
  int i;
  
  GFL_BMPWIN_ClearScreen(pWork->winItemName);
  GFL_BMPWIN_ClearScreen(pWork->winItemNum);
  GFL_BMPWIN_ClearScreen(pWork->winItemReport);
  GFL_BG_LoadScreenV_Req(ITEMREPORT_FRAME);
  GFL_BMPWIN_Delete(pWork->winItemName);
  GFL_BMPWIN_Delete(pWork->winItemReport);
  GFL_BMPWIN_Delete(pWork->winItemNum);

  GFL_CLACT_WK_Remove( pWork->cellicon );
  GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR] );
  GFL_CLGRP_PLTT_Release( pWork->objRes[_CLACT_PLT] );
  GFL_CLGRP_CELLANIM_Release(pWork->objRes[_CLACT_ANM]);

  for( i=0 ; i < ITEM_LIST_NUM ; i++ ){
    GFL_BMP_Delete(pWork->listBmp[i]);
  }

  GFL_CLACT_UNIT_Delete(pWork->cellUnit);
  
  GFL_CLACT_SYS_Delete();

}

//------------------------------------------------------------------------------
/**
 * @brief   上画面のメッセージの初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

void ITEMDISP_upMessageCreate(FIELD_ITEMMENU_WORK* pWork)
{

  pWork->winItemName = GFL_BMPWIN_Create(
    ITEMREPORT_FRAME,
    _UP_ITEMNAME_INITX, _UP_ITEMNAME_INITY,
    _UP_ITEMNAME_SIZEX, _UP_ITEMNAME_SIZEY,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

  pWork->winItemNum = GFL_BMPWIN_Create(
    ITEMREPORT_FRAME,
    _UP_ITEMNUM_INITX, _UP_ITEMNUM_INITY,
    _UP_ITEMNUM_SIZEX, _UP_ITEMNUM_SIZEY,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

  pWork->winItemReport = GFL_BMPWIN_Create(
    ITEMREPORT_FRAME,
    _UP_ITEMREPORT_INITX, _UP_ITEMREPORT_INITY,
    _UP_ITEMREPORT_SIZEX, _UP_ITEMREPORT_SIZEY,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );


  GFL_BMPWIN_MakeScreen( pWork->winItemName );
  GFL_BMPWIN_MakeScreen( pWork->winItemNum );
  GFL_BMPWIN_MakeScreen( pWork->winItemReport );
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_ITEMICON, pWork->heapID );

    pWork->objRes[_CLACT_ANM] = GFL_CLGRP_CELLANIM_Register( p_handle ,
                                                             NARC_item_icon_itemicon_NCER ,
                                                             NARC_item_icon_itemicon_NANR ,
                                                             pWork->heapID );
    GFL_ARC_CloseDataHandle( p_handle );
  }

  {
    ITEMDISP_upMessageRewrite(pWork);
  }

}

static void _itemiconAnim(FIELD_ITEMMENU_WORK* pWork,int itemid)
{
  if(pWork->cellicon!=NULL){
    GFL_CLACT_WK_Remove( pWork->cellicon );
    GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR] );
    GFL_CLGRP_PLTT_Release( pWork->objRes[_CLACT_PLT] );
  }

  
  {
    ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_ITEMICON, pWork->heapID );

    pWork->objRes[_CLACT_PLT] = GFL_CLGRP_PLTT_Register( p_handle ,
                                                         ITEM_GetIndex(itemid,ITEM_GET_ICON_PAL) ,
                                                         CLSYS_DRAW_SUB , 0 , pWork->heapID );
    pWork->objRes[_CLACT_CHR] = GFL_CLGRP_CGR_Register( p_handle ,
                                                        ITEM_GetIndex(itemid,ITEM_GET_ICON_CGX) ,
                                                        FALSE , CLSYS_DRAW_SUB , pWork->heapID );


    GFL_ARC_CloseDataHandle( p_handle );
  }

  {
    GFL_CLWK_DATA cellInitData;


    OS_TPrintf("アイコン絵交換\n");

    cellInitData.pos_x = _ITEMICON_SCR_X * 8+16;
    cellInitData.pos_y = _ITEMICON_SCR_Y * 8+16;
    cellInitData.anmseq = 0;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->cellicon = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                           pWork->objRes[_CLACT_CHR],
                                           pWork->objRes[_CLACT_PLT],
                                           pWork->objRes[_CLACT_ANM],
                                           &cellInitData ,
                                           CLSYS_DEFREND_SUB ,
                                           pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellicon, TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellicon, TRUE );
  }
}


void _dispMain(FIELD_ITEMMENU_WORK* pWork)
{
  GFL_CLACT_SYS_Main(); // CLSYSメイン
}





#if 1

void ITEMDISP_CellResourceCreate( FIELD_ITEMMENU_WORK* pWork )
{
  int i;
  ARCHANDLE *archandle = GFL_ARC_OpenDataHandle( ARCID_BAG , pWork->heapID );

  pWork->cellRes[_PLT_CUR] = GFL_CLGRP_PLTT_RegisterEx( archandle ,
        NARC_bag_bag_win03_d_NCLR , CLSYS_DRAW_MAIN ,
        0 , 0 , 0 , pWork->heapID  );

  pWork->cellRes[_NCG_CUR] = GFL_CLGRP_CGR_Register(
    archandle , NARC_bag_bag_win03_d_NCGR ,
    FALSE , CLSYS_DRAW_MAIN , pWork->heapID  );

  pWork->cellRes[_ANM_CUR] = GFL_CLGRP_CELLANIM_Register(
    archandle , NARC_bag_bag_win03_d_NCER,NARC_bag_bag_win03_d_NANR ,
     pWork->heapID  );


  for( i=0 ; i < ITEM_LIST_NUM ; i++ )
  {
    pWork->listRes[i] = GFL_CLGRP_CGR_Register(
      archandle , NARC_bag_bag_win01_d_NCGR ,
      FALSE , CLSYS_DRAW_MAIN , pWork->heapID  );
    pWork->listBmp[i] = GFL_BMP_Create( 12, 2, GFL_BMP_16_COLOR, pWork->heapID );

  }

  pWork->cellRes[_ANM_LIST] = GFL_CLGRP_CELLANIM_Register(
    archandle , NARC_bag_bag_win01_d_NCER, NARC_bag_bag_win01_d_NANR ,
     pWork->heapID  );



  
  GFL_ARC_CloseDataHandle(archandle);
}


void ITEMDISP_CellCreate( FIELD_ITEMMENU_WORK* pWork )
{
  //選択カーソル
  {
    GFL_CLWK_DATA cellInitData;
    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.softpri = 10;
    cellInitData.bgpri = 2;
    cellInitData.anmseq = 0;
    
    pWork->clwkCur = GFL_CLACT_WK_Create(
      pWork->cellUnit ,
      pWork->cellRes[_NCG_CUR], pWork->cellRes[_PLT_CUR],  pWork->cellRes[_ANM_CUR],
      &cellInitData ,CLSYS_DEFREND_MAIN , pWork->heapID );

    GFL_CLACT_WK_SetDrawEnable( pWork->clwkCur , TRUE );
  }  
  
    //セルの作成
  {
    int i;
    for( i=0 ; i < ITEM_LIST_NUM ; i++ )
    {
      GFL_CLWK_DATA cellInitData;
      cellInitData.pos_x = 17*8;
      cellInitData.pos_y = 3*8*i + 24;
      cellInitData.softpri = 10;
      cellInitData.bgpri = 2;
      cellInitData.anmseq = 0;
    
      pWork->listCell[i] = GFL_CLACT_WK_Create(
        pWork->cellUnit ,
        pWork->listRes[ i ],pWork->cellRes[_PLT_CUR],  pWork->cellRes[_ANM_LIST],
        &cellInitData ,CLSYS_DEFREND_MAIN , pWork->heapID );

      GFL_CLACT_WK_SetDrawEnable( pWork->listCell[i] , TRUE );

    }
  }
}


void ITEMDISP_CellMessagePrint( FIELD_ITEMMENU_WORK* pWork )
{
  int i;

	for(i = 0; i< ITEM_LIST_NUM ; i++){
		ITEM_ST * item;

    item = MYITEM_PosItemGet( pWork->pMyItem, pWork->pocketno,  pWork->curpos + i );
		if((item==NULL) || (item->id==ITEM_DUMMY_DATA)){
			break;
		}
    GFL_BMP_Clear(pWork->listBmp[i],3);
    GFL_FONTSYS_SetColor( 0xf, 0xe, 3 );
    GFL_MSG_GetString(  pWork->MsgManager, MSG_ITEM_STR001, pWork->pStrBuf );
    WORDSET_RegisterItemName(pWork->WordSet, 0, i);
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    PRINTSYS_Print( pWork->listBmp[i], 0, 0, pWork->pExpStrBuf, pWork->fontHandle);
	}
}

void ITEMDISP_CellVramTrans( FIELD_ITEMMENU_WORK* pWork )
{
  int i;

	for(i = 0; i< ITEM_LIST_NUM ; i++){
    {
      u32 dest_adrs = GFL_CLGRP_CGR_GetAddr( pWork->listRes[i], CLSYS_DRAW_MAIN);
      u8* charbuff = GFL_BMP_GetCharacterAdrs(pWork->listBmp[i]);
      u32 size = GFL_BMP_GetBmpDataSize(pWork->listBmp[i]);
      DC_FlushRange(charbuff, size);
      
      dest_adrs += (8)*32;
      GX_LoadOBJ(&charbuff[8*32], dest_adrs, (32*4));
      dest_adrs += (4)*32;
      GX_LoadOBJ(&charbuff[(20*32)], dest_adrs, (32*4));

      dest_adrs += (12)*32;
      GX_LoadOBJ(&charbuff[4*32], dest_adrs, (32*4));
      dest_adrs += (4)*32;
      GX_LoadOBJ(&charbuff[(16*32)], dest_adrs, (32*4));

      dest_adrs += (12)*32;
      GX_LoadOBJ(&charbuff[0*32], dest_adrs, (32*4));
      dest_adrs += (4)*32;
      GX_LoadOBJ(&charbuff[(12*32)], dest_adrs, (32*4));
      
    }
	}
}

#endif
