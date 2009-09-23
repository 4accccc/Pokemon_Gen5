//=============================================================================
/**
 * @file	  ircpoketrade_gra.c
 * @bfief	  ポケモン交換グラフィック部分
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/08/28
 */
//=============================================================================


#include <gflib.h>
#include "arc_def.h"
#include "poke_tool/monsno_def.h"

#include "trade.naix"

#include "ircpokemontrade.h"
#include "ircpokemontrade_local.h"
#include "msg/msg_poke_trade.h"
#include "gamesystem/msgspeed.h"  //MSGSPEED_GetWait
#include "font/font.naix"    // NARC_font_default_nclr

#include "system/bmp_winframe.h"
#include "savedata/box_savedata.h"
#include "pokeicon/pokeicon.h"

#include "ircbattle.naix"
#include "trade.naix"
#include "tradedemo.naix"


#define _SUBLIST_NORMAL_PAL   (9)   //サブメニューの通常パレット


static void IRC_POKETRADE_TrayInit(IRC_POKEMON_TRADE* pWork,int subchar);
static void IRC_POKETRADE_TrayExit(IRC_POKEMON_TRADE* pWork);
static int _DotToLine(int pos);
static void PokeIconCgxLoad(IRC_POKEMON_TRADE* pWork );


static int _Line2RingLineIconGet(IRC_POKEMON_TRADE* pWork,int line);
static void _Line2RingLineSet(IRC_POKEMON_TRADE* pWork,int add);
static void _iconAllDrawDisable(IRC_POKEMON_TRADE* pWork);



static int LINE2TRAY(int lineno)
{
  if(lineno > HAND_HORIZONTAL_NUM){
    return (lineno - HAND_HORIZONTAL_NUM) / BOX_HORIZONTAL_NUM;
  }
  return -1;
}
    
static int LINE2POKEINDEX(int lineno,int verticalindex)
{
  if(lineno > HAND_HORIZONTAL_NUM){
    return ((lineno - HAND_HORIZONTAL_NUM) % BOX_HORIZONTAL_NUM) + (BOX_HORIZONTAL_NUM * verticalindex);
  }
  return lineno*3+verticalindex;
}

//------------------------------------------------------------------
/**
 * $brief   初期化時のグラフィック初期化
 * @param   IRC_POKEMON_TRADE   ワークポインタ
 * @retval  none
 */
//------------------------------------------------------------------

void IRC_POKETRADE_GraphicInit(IRC_POKEMON_TRADE* pWork)
{
  
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
	MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );
	u32 sex = MyStatus_GetMySex(pMy);



	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_trade_wb_trade_bg_NCLR,
																		PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);


	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_trade_wb_trade_bg01_NCGR,
																																GFL_BG_FRAME2_M, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_trade_bg01_NSCR,
																				 GFL_BG_FRAME1_M, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_trade_bg01_back_NSCR,
																				 GFL_BG_FRAME2_M, 0,
                                         GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                         pWork->heapID);




	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_trade_wb_trade_bg_NCLR,
																		PALTYPE_SUB_BG, 0, 0,  pWork->heapID);


	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_trade_wb_trade_bg02_NCGR,
																																GFL_BG_FRAME2_S, 0, 0, pWork->heapID);
	pWork->subchar1 = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_trade_wb_trade_bg02_NCGR,
																																GFL_BG_FRAME1_S, 0, 0, pWork->heapID);

//	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
//																				 NARC_trade_wb_trade_bg02_NSCR,
//																				 GFL_BG_FRAME2_S, 0,
//																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
//																				 pWork->heapID);





	GFL_ARC_CloseDataHandle( p_handle );


  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME3_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);
  
  IRC_POKETRADE_TrayInit(pWork,pWork->subchar);

  PokeIconCgxLoad( pWork );


#if 0

	//読み込み
	{	
    ARCHANDLE	*p_handle	= GFL_ARC_OpenDataHandle( ARCID_POKETRADEDEMO, pWork->heapID );

    //モデル読み込み
    {	
      BOOL			result;
      
      GFL_G3D_RES	*p_tex	= NULL;
      GFL_G3D_RES	*p_mdl	= NULL;
      GFL_G3D_RND* pRender = NULL;

//      pWork->pG3dRes	= GFL_G3D_CreateResourceHandle( p_handle, NARC_tradedemo_trade_nsbmd );
      pWork->pG3dRes	= GFL_G3D_CreateResourceHandle( p_handle, NARC_tradedemo_trailer_nsbmd );
      GF_ASSERT( pWork->pG3dRes != NULL );

      if( GFL_G3D_CheckResourceType( pWork->pG3dRes, GFL_G3D_RES_CHKTYPE_TEX ) ){
        p_tex	= pWork->pG3dRes;
        result	= GFL_G3D_TransVramTexture( p_tex );
        OS_TPrintf("転送した\n");
        GF_ASSERT( result );
      }


      pRender = GFL_G3D_RENDER_Create( pWork->pG3dRes, 0, NULL );

      pWork->pG3dObj = GFL_G3D_OBJECT_Create(pRender, NULL, 0 );
      
    }

	//座標
    {	
      MTX_Identity33( &pWork->status.rotate );
      VEC_Set( &pWork->status.scale, FX32_ONE, FX32_ONE, FX32_ONE );
    }

    GFL_ARC_CloseDataHandle( p_handle );
  }


  {
    static const VecFx32 sc_CAMERA_PER_POS		= { 0,0,FX32_CONST( 160 ) };
    static const VecFx32 sc_CAMERA_PER_UP			= { 0,FX32_ONE,0 };
    static const VecFx32 sc_CAMERA_PER_TARGET	= { 0,0,FX32_CONST( 0 ) };

    enum
    {	
      CAMERA_PER_FOVY		=	(40),
      CAMERA_PER_ASPECT =	(FX32_ONE * 4 / 3),
      CAMERA_PER_NEAR		=	(FX32_ONE * 1),
      CAMERA_PER_FER		=	(FX32_ONE * 6000),
      CAMERA_PER_SCALEW	=(0),
    };

    
	//射影
    pWork->p_camera =
      GFL_G3D_CAMERA_CreatePerspective( CAMERA_PER_FOVY, CAMERA_PER_ASPECT,
                                        CAMERA_PER_NEAR, CAMERA_PER_FER, CAMERA_PER_SCALEW, 
                                        &sc_CAMERA_PER_POS, &sc_CAMERA_PER_UP, &sc_CAMERA_PER_TARGET, pWork->heapID );
  }
#endif

  // カメラ初期設定
#if 0
  {
#define cameraPerspway  ( 0x0b60 )
#define cameraAspect    ( FX32_ONE * 4 / 3 )
#define cameraNear	    ( 1 << FX32_SHIFT )
#define cameraFar       ( 1024 << FX32_SHIFT )

    
    GFL_G3D_PROJECTION proj = { GFL_G3D_PRJPERS, 0, 0, cameraAspect, 0, cameraNear, cameraFar, 0 }; 
    proj.param1 = FX_SinIdx( cameraPerspway ); 
    proj.param2 = FX_CosIdx( cameraPerspway ); 
    GFL_G3D_SetSystemProjection( &proj );	
  }
#endif
  
}


void IRC_POKETRADE_G3dDraw(IRC_POKEMON_TRADE* pWork)
{
//		G3X_EdgeMarking( FALSE );
//  GFL_G3D_CAMERA_Switching( pWork->p_camera );  
//  GFL_G3D_DRAW_DrawObject( pWork->pG3dObj, &pWork->status );
}



void IRC_POKETRADE_GraphicExit(IRC_POKEMON_TRADE* pWork)
{
  IRC_POKETRADE_TrayExit(pWork);
}


void IRC_POKETRADE_SubStatusInit(IRC_POKEMON_TRADE* pWork,int pokeposx)
{

	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_POKETRADE, pWork->heapID );
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_trade_wb_trade_stbg01_NSCR,
																				 GFL_BG_FRAME1_S, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar1), 0, 0,
																				 pWork->heapID);

  if(pokeposx < 128){
    GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, 128 );
  }
	G2S_SetBlendAlpha( GFL_BG_FRAME2_S, GFL_BG_FRAME1_S , 3, 16 );
  //G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_OBJ , -8 );
  
	GFL_ARC_CloseDataHandle( p_handle );

}


void IRC_POKETRADE_SubStatusEnd(IRC_POKEMON_TRADE* pWork)
{
	GFL_BG_SetScroll( GFL_BG_FRAME1_S, GFL_BG_SCROLL_X_SET, 0 );
  G2S_BlendNone();
  GFL_BG_ClearScreen(GFL_BG_FRAME1_S);  //自分ステータス

}


void IRC_POKETRADE_AppMenuOpen(IRC_POKEMON_TRADE* pWork, int *menustr,int num)
{
  int i;
  APP_TASKMENU_INITWORK appinit;

  appinit.heapId = pWork->heapID;
  appinit.itemNum =  num;
  appinit.itemWork =  &pWork->appitem[0];
  appinit.bgFrame =  GFL_BG_FRAME3_S;
  appinit.palNo = _SUBLIST_NORMAL_PAL;

  appinit.posType = ATPT_RIGHT_DOWN;
  appinit.charPosX = 32;
  appinit.charPosY = 24;

  appinit.msgHandle = pWork->pMsgData;
  appinit.fontHandle =  pWork->pFontHandle;
  appinit.printQue =  pWork->SysMsgQue;

  for(i=0;i<num;i++){
    pWork->appitem[i].str = GFL_STR_CreateBuffer(100, pWork->heapID);
    GFL_MSG_GetString(pWork->pMsgData, menustr[i], pWork->appitem[i].str);
    pWork->appitem[i].msgColor = PRINTSYS_LSB_Make( 0xe,0xf,0);
  }
  pWork->pAppTask = APP_TASKMENU_OpenMenu(&appinit);
  for(i=0;i<num;i++){
    GFL_STR_DeleteBuffer(pWork->appitem[i].str);
  }
  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , -8 );

}

void IRC_POKETRADE_MessageOpen(IRC_POKEMON_TRADE* pWork, int msgno)
{


  GFL_MSG_GetString( pWork->pMsgData, msgno, pWork->pMessageStrBufEx );

	WORDSET_ExpandStr( pWork->pWordSet, pWork->pMessageStrBufEx, pWork->pMessageStrBuf  );

}

void IRC_POKETRADE_MessageWindowOpen(IRC_POKEMON_TRADE* pWork, int msgno)
{
  GFL_BMPWIN* pwin;

  IRC_POKETRADE_MessageWindowClose(pWork);

  //IRC_POKETRADE_MessageOpen(pWork,msgno);


  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
  pWork->mesWin = GFL_BMPWIN_Create(GFL_BG_FRAME3_S , 1 , 1, 30 ,4 ,  _BUTTON_MSG_PAL , GFL_BMP_CHRAREA_GET_B );
  
  pwin = pWork->mesWin;

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pwin), 15);


 pWork->pStream = PRINTSYS_PrintStream(pwin ,0,0, pWork->pMessageStrBuf, pWork->pFontHandle,
                                        MSGSPEED_GetWait(), pWork->pMsgTcblSys, 2, pWork->heapID, 15);

  
  BmpWinFrame_Write( pwin, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_BMPWIN_TransVramCharacter(pwin);
  GFL_BMPWIN_MakeScreen(pwin);
  GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);

}


void IRC_POKETRADE_MessageWindowClear(IRC_POKEMON_TRADE* pWork)
{

	if(pWork->mesWin){
		GFL_BMPWIN_ClearScreen(pWork->mesWin);
		GFL_BG_LoadScreenV_Req(GFL_BG_FRAME3_S);
		BmpWinFrame_Clear(pWork->mesWin, WINDOW_TRANS_OFF);
		GFL_BMPWIN_Delete(pWork->mesWin);
		pWork->mesWin=NULL;
	}
}



void IRC_POKETRADE_MessageWindowClose(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->mesWin){
		GFL_BMPWIN_Delete(pWork->mesWin);
    pWork->mesWin=NULL;
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   メッセージの終了待ち
 * @retval  none
 */
//------------------------------------------------------------------------------

BOOL IRC_POKETRADE_MessageEndCheck(IRC_POKEMON_TRADE* pWork)
{
  if(pWork->pStream){
    int state = PRINTSYS_PrintStreamGetState( pWork->pStream );
    switch(state){
    case PRINTSTREAM_STATE_DONE:
      PRINTSYS_PrintStreamDelete( pWork->pStream );
      pWork->pStream = NULL;
      break;
    case PRINTSTREAM_STATE_PAUSE:
      if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE){
        PRINTSYS_PrintStreamReleasePause( pWork->pStream );
      }
      break;
    default:
      break;
    }
    return FALSE;  //まだ終わってない
  }
  return TRUE;// 終わっている
}

//------------------------------------------------------------------------------
/**
 * @brief   トレイの表示
 * @retval  none
 */
//------------------------------------------------------------------------------

#define _TEMOTITRAY_SCR_MAX (12)
#define _BOXTRAY_SCR_MAX (20)
#define _SRCMAX ((BOX_MAX_TRAY*_BOXTRAY_SCR_MAX)+_TEMOTITRAY_SCR_MAX)

#define _TEMOTITRAY_MAX (8*_TEMOTITRAY_SCR_MAX)
#define _BOXTRAY_MAX (8*_BOXTRAY_SCR_MAX)

#define _DOTMAX ((BOX_MAX_TRAY*_BOXTRAY_MAX)+_TEMOTITRAY_MAX)


#define _LINEMAX (BOX_MAX_TRAY * 6 + 2)

//------------------------------------------------------------------------------
/**
 * @brief   スクリーン座標からスクリーンデータを取り出す
 * @retval  none
 */
//------------------------------------------------------------------------------

static u16 _GetScr(int x , int y, IRC_POKEMON_TRADE* pWork)
{
  int x2=x;

  if(x >= _SRCMAX){
    x2 = x - _SRCMAX;
  }
  if(x2 < _TEMOTITRAY_SCR_MAX){
    return pWork->scrTemoti[ 18+(y * 32) + x2 ];
  }
  x2 = x2 - _TEMOTITRAY_SCR_MAX;
  x2 = x2 % _BOXTRAY_SCR_MAX;
  return pWork->scrTray[ 18+(y * 32) + x2 ];
}


void IRC_POKETRADE_TrayDisp(IRC_POKEMON_TRADE* pWork)
{
  int bgscr = pWork->BoxScrollNum / 8;  //マスの単位は画面スクロールを使う
  int frame = GFL_BG_FRAME2_S;
  int x,y;

  for(y = 0; y < 24 ; y++){
    for(x = 0; x < (_BOXTRAY_SCR_MAX*2+_TEMOTITRAY_SCR_MAX) ; x++){
      u16 scr = _GetScr( bgscr + x , y ,pWork);
//      GFL_BG_WriteScreen(frame, &scr, x, y, 1, 1 );
      GFL_BG_ScrSetDirect(frame, x, y, scr);
    }
  }

  pWork->bgscroll = pWork->BoxScrollNum % 8;  //マスの単位は画面スクロールを使う
  pWork->bgscrollRenew = TRUE;

  
  GFL_BG_LoadScreenV_Req(frame);

}


static void IRC_POKETRADE_TrayInit(IRC_POKEMON_TRADE* pWork,int subchar)
{
  // スクリーンを読み込んでおく
  pWork->scrTray = GFL_ARC_LoadDataAlloc( ARCID_POKETRADE, NARC_trade_wb_trade_bg02_NSCR, pWork->heapID);
  pWork->scrTemoti = GFL_ARC_LoadDataAlloc( ARCID_POKETRADE, NARC_trade_wb_trade_bg03_NSCR, pWork->heapID);


  {
    int i;
    for(i=0;i<(32*24);i++){
      pWork->scrTray[18+i] += GFL_ARCUTIL_TRANSINFO_GetPos(subchar);
      pWork->scrTemoti[18+i] += GFL_ARCUTIL_TRANSINFO_GetPos(subchar);
    }
  }

  pWork->BoxScrollNum = _DOTMAX - 80;
  IRC_POKETRADE_TrayDisp(pWork);
}

static void IRC_POKETRADE_TrayExit(IRC_POKEMON_TRADE* pWork)
{
  GFL_HEAP_FreeMemory( pWork->scrTray);
  GFL_HEAP_FreeMemory( pWork->scrTemoti);
}





//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン表示
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

//	u32 pokeIconNcgRes[BOX_HORIZONTAL_NUM * BOX_MAX_TRAY + HAND_HORIZONTAL_NUM][BOX_VERTICAL_NUM];
//	GFL_CLWK* pokeIcon[BOX_HORIZONTAL_NUM * BOX_MAX_TRAY + HAND_HORIZONTAL_NUM][BOX_VERTICAL_NUM];


//------------------------------------------------------------------------------
/**
 * @brief   必要なくなったラインのリソース開放
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _deletePokeIconResource(IRC_POKEMON_TRADE* pWork, int line)
{
  int i;
  
	for( i = 0 ; i < BOX_VERTICAL_NUM ; i++ ){
		if(pWork->pokeIconNcgRes[line][i]){
			GFL_CLGRP_CGR_Release(pWork->pokeIconNcgRes[line][i]);
			pWork->pokeIconNcgRes[line][i] = NULL;
		}
		if(pWork->pokeIcon[line][i]){
			GFL_CLACT_WK_Remove(pWork->pokeIcon[line][i]);
			pWork->pokeIcon[line][i]=NULL;
		}
	}
}

void IRC_POKETRADE_AllDeletePokeIconResource(IRC_POKEMON_TRADE* pWork)
{
  int i;
  
  for(i = 0 ; i < _LINEMAX ; i++){
    _deletePokeIconResource(pWork,i);
  }
}



static const POKEMON_PASO_PARAM* _getPokeDataAddress(BOX_DATA* boxData , int lineno, int verticalindex , IRC_POKEMON_TRADE* pWork)
{

  if(lineno > HAND_HORIZONTAL_NUM){
    int tray = LINE2TRAY(lineno);
    int index = LINE2POKEINDEX(lineno, verticalindex);
    
    return BOXDAT_GetPokeDataAddress(boxData,tray,index);
  }
	{
		POKEPARTY* party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(pWork->pGameSys));

    if(verticalindex <3){
      int index = lineno * 3 + verticalindex;
      if(index < PokeParty_GetPokeCount(party)){
        const POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , index );
        return PP_GetPPPPointerConst( pp );
      }
		}
	}
	return NULL;

}


static void _calcPokeIconPos(int line,int index, GFL_CLACTPOS* pos)
{
	static const u8	iconSize = 24;
	static const u8 iconTop = 72;
	static const u8 iconLeft = 24;

	pos->x = line * iconSize + iconLeft;
	pos->y = index * iconSize + iconTop;
}


//ラインにあわせたポケモン表示
static void _createPokeIconResource(IRC_POKEMON_TRADE* pWork,BOX_DATA* boxData ,int line)
{
  int i,k;
	void *obj_vram = G2S_GetOBJCharPtr();
	ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );

  k =  _Line2RingLineIconGet(pWork, line);

	for( i = 0 ; i < BOX_VERTICAL_NUM ; i++ )
	{
    {
      int	fileNo,monsno,formno,bEgg;
      const POKEMON_PASO_PARAM* ppp = _getPokeDataAddress(boxData, line, i, pWork);
      pWork->pokeIconLine[k][i] = line;
      if(!ppp){
        continue;
      }
      monsno = PPP_Get(ppp,ID_PARA_monsno,NULL);
      if( monsno == 0 ){	//ポケモンがいるかのチェック
        continue;
      }
      else if(pWork->pokeIconNo[k][i] == monsno){
        GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[k][i], TRUE );
      }
      else if(pWork->pokeIconNo[k][i] != monsno){
        GFL_CLWK_DATA cellInitData;
        u8 pltNum;
        GFL_CLACTPOS pos;
        NNSG2dImageProxy aproxy;
          
 //         pWork->pokeIconNcgRes[line][i] =
   //         GFL_CLGRP_CGR_Register( arcHandle , POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_SUB , pWork->heapID );

   //     OS_TPrintf("変更 k%d i%d %d %d\n",k,i,pWork->pokeIconNo[k][i],monsno);
        pWork->pokeIconNo[k][i] = monsno;
        
        pltNum = POKEICON_GetPalNumGetByPPP( ppp );
        _calcPokeIconPos(line, i, &pos);

        GFL_CLACT_WK_GetImgProxy( pWork->pokeIcon[k][i], &aproxy );
        
        GFL_STD_MemCopy(&pWork->pCharMem[4*8*4*4*monsno] , (char*)((u32)obj_vram) + aproxy.vramLocation.baseAddrOfVram[NNS_G2D_VRAM_TYPE_2DSUB], 4*8*4*4);
        
        
        GFL_CLACT_WK_SetPlttOffs( pWork->pokeIcon[k][i] , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
        GFL_CLACT_WK_SetAutoAnmFlag( pWork->pokeIcon[k][i] , FALSE );
        GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[k][i], TRUE );
      }
    }
  }
  GFL_ARC_CloseDataHandle( arcHandle );
}


//------------------------------------------------------------------------------
/**
 * @brief   BoxScrollNumの値からLineの基本値を得る
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _boxScrollNum2Line(IRC_POKEMON_TRADE* pWork)
{
  int line,i;
  
  if(pWork->BoxScrollNum < (_TEMOTITRAY_MAX/2)){
    line = 0;
  }
  else if(pWork->BoxScrollNum < _TEMOTITRAY_MAX){
    line = 1;
  }
  else{
    i = (pWork->BoxScrollNum - _TEMOTITRAY_MAX) / _BOXTRAY_MAX; //BOX数
    line = i * 6 + 2;
    line += ((pWork->BoxScrollNum - _TEMOTITRAY_MAX) - (i*_BOXTRAY_MAX)) / 27 ;
    //OS_TPrintf("LINE %d %d\n", pWork->BoxScrollNum, line);
  }
  return line;
}





void IRC_POKETRADE_InitBoxIcon( BOX_DATA* boxData ,IRC_POKEMON_TRADE* pWork )
{
  int i,line = _boxScrollNum2Line(pWork);

  if( pWork->oldLine != line ){
    pWork->oldLine = line;

    _Line2RingLineSet(pWork,line);
    _iconAllDrawDisable(pWork);  // アイコン表示を一旦消す
  
    for(i=0;i < _LING_LINENO_MAX;i++){
      //OS_TPrintf("create %d\n", line);
      _createPokeIconResource(pWork, boxData,line );  //アイコン表示
      line++;
      if(line >= _LINEMAX){
        line=0;
      }
    }
  }
  IRC_POKETRADE_PokeIcomPosSet(pWork);
  
}



void IRC_POKETRADE_PokeIcomPosSet(IRC_POKEMON_TRADE* pWork)
{
  int line, i, no, m;
  int x,y;
  //int bgscr = pWork->BoxScrollNum;

  for(m = 0; m < _LING_LINENO_MAX; m++){
    line = pWork->oldLine + m;
    
    no = _Line2RingLineIconGet(pWork, line);

    for(i=0;i<BOX_VERTICAL_NUM;i++){
      GFL_CLACTPOS apos;
      y = 32+i*24;
      if(line == 0){
        x = 16;
      }
      else if(line == 1){
        x = 56;
      }
      else{
        x = ((line - 2) / 6) * _BOXTRAY_MAX;
        x += ((line - 2) % 6) * 24 + 16 + 4;
        x += _TEMOTITRAY_MAX;
      }
      x = x - pWork->BoxScrollNum;
      apos.x = x;
      apos.y = y;
      
      GFL_CLACT_WK_SetPos(pWork->pokeIcon[no][i], &apos, CLSYS_DRAW_SUB);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ドットの位置
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------
#if 1

static int _DotToLine(int pos)
{


  return 0;
}




//--------------------------------------------------------------------------------------------
/**
 * ポケモンアイコンキャラデータをメモリに展開
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	ppp		POKEMON_PASO_PARAM
 * @param	chr		NNSG2dCharacterData
 *
 * @return	buf
 */
//--------------------------------------------------------------------------------------------
static void  PokeIconCgxLoad(IRC_POKEMON_TRADE* pWork )
{
  int i;
  void* pMem;
	u32	arcIndex;
  ARCHANDLE * pokeicon_ah = GFL_ARC_OpenDataHandle( ARCID_POKEICON, pWork->heapID );
  NNSG2dCharacterData* pCharData;


  GF_ASSERT(MONSNO_MAX < 650);
  pWork->pCharMem = GFL_HEAP_AllocMemory(pWork->heapID, 4*8*4*4*650 );
  
  for(i=0;i<MONSNO_MAX; i++){ //@@OO フォルム違いを持ってくる必要あり
  
    arcIndex = POKEICON_GetCgxArcIndexByMonsNumber( i, 0, 0 );
    pMem = GFL_ARCHDL_UTIL_LoadBGCharacter(pokeicon_ah, arcIndex, FALSE, &pCharData, pWork->heapID);

    GFL_STD_MemCopy(pCharData->pRawData,&pWork->pCharMem[4*8*4*4*i] , 4*8*4*4);

    GFL_HEAP_FreeMemory(pMem);
    
  }

  GFL_ARC_CloseDataHandle(pokeicon_ah);
}

//--------------------------------------------------------------------------------------------
/**
 * リングバッファのライン番号を返す
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	ppp		POKEMON_PASO_PARAM
 * @param	chr		NNSG2dCharacterData
 *
 * @return	buf
 */
//--------------------------------------------------------------------------------------------

static int _Line2RingLineIconGet(IRC_POKEMON_TRADE* pWork,int line)
{
  int ret;

//  ret = pWork->ringLineNo + line;
  ret = line % _LING_LINENO_MAX;
  return ret;
}

//--------------------------------------------------------------------------------------------
/**
 * リングバッファを進める＆もどす
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	ppp		POKEMON_PASO_PARAM
 * @param	chr		NNSG2dCharacterData
 *
 * @return	buf
 */
//--------------------------------------------------------------------------------------------

static void _Line2RingLineSet(IRC_POKEMON_TRADE* pWork,int add)
{
  int ret;

  pWork->ringLineNo = add;
}

//--------------------------------------------------------------------------------------------
/**
 * アイコン表示を全部禁止
 *
 * @param	appwk	ボックス画面アプリワーク
 * @param	ppp		POKEMON_PASO_PARAM
 * @param	chr		NNSG2dCharacterData
 *
 * @return	buf
 */
//--------------------------------------------------------------------------------------------

static void _iconAllDrawDisable(IRC_POKEMON_TRADE* pWork)
{
  int line,i;
  
  for(line =0 ;line < _LING_LINENO_MAX; line++)
  {
    for(i = 0;i < BOX_VERTICAL_NUM; i++)
    {
      GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[line][i], FALSE );
    }
  }
}




GFL_CLWK* IRC_POKETRADE_GetCLACT( IRC_POKEMON_TRADE* pWork , int x, int y, int* trayno, int* pokeindex)
{
  GFL_CLACTPOS apos;
  int line,i,line2;

  for(line =0 ;line < _LING_LINENO_MAX; line++)
  {
    for(i = 0;i < BOX_VERTICAL_NUM; i++)
    {
      GFL_CLACT_WK_GetPos( pWork->pokeIcon[line][i], &apos ,CLSYS_DRAW_SUB );
      if((apos.x <= x) && (x < (apos.x+24))){
        if((apos.y <= y) && (y < (apos.y+24))){

          if(trayno!=NULL){//位置情報を得たい場合に計算

            line2 = pWork->pokeIconLine[line][i];
            *trayno = LINE2TRAY(line2);
            *pokeindex = LINE2POKEINDEX(line2, i);
          }
          return pWork->pokeIcon[line][i];
          //          line += BoxScrollNum(pWork);
        }
      }
    }
  }
  return NULL;
}


#endif



