//=============================================================================
/**
 * @file	  ircpokemontrade.c
 * @bfief	  ポケモン交換して通信を終了する
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date	  09/07/09
 */
//=============================================================================

#include <gflib.h>
#include "arc_def.h"
#include "net/network_define.h"

#include "ircpokemontrade.h"
#include "system/main.h"

#include "message.naix"
#include "print/printsys.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "pokeicon/pokeicon.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"

#include "msg/msg_ircbattle.h"
#include "ircbattle.naix"
#include "trade.naix"
#include "net_app/connect_anm.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"

#include "system/touch_subwindow.h"

#include "poke_tool/poke_tool_def.h"
#include "box_m_obj_NANR_LBLDEFS.h"
#include "p_st_obj_d_NANR_LBLDEFS.h"

#include "ircpokemontrade_local.h"

///通信コマンド
typedef enum {
	_NETCMD_SELECT_POKEMON = GFL_NET_CMD_IRCTRADE,
	_NETCMD_CHANGE_POKEMON,
	_NETCMD_CHANGE_YESNO,
	_NETCMD_CHANGE_CANCEL,
	_NETCMD_END_REQ,
	_NETCMD_END,
} _BATTLEIRC_SENDCMD;


static void _changeState(IRC_POKEMON_TRADE* pWork,StateFunc* state);
static void _changeStateDebug(IRC_POKEMON_TRADE* pWork,StateFunc* state, int line);
static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangeYesNo(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _recvEndReq(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);

static void _touchState(IRC_POKEMON_TRADE* pWork);
static u8* _setChangePokemonBuffer(int netID, void* pWork, int size);
static void _changeWaitState(IRC_POKEMON_TRADE* pWork);
static void _changeYesNoWaitState(IRC_POKEMON_TRADE* pWork);
static void _endWaitState(IRC_POKEMON_TRADE* pWork);



#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
	{_recvSelectPokemon,   _setChangePokemonBuffer},    ///_NETCMD_SELECT_POKEMON
	{_recvChangePokemon,   NULL},    ///_NETCMD_CHANGE_POKEMON
	{_recvChangeYesNo,   NULL},    ///_NETCMD_CHANGE_YESNO
	{_recvChangeCancel,   NULL},    ///_NETCMD_SELECT_POKEMON
	{_recvEndReq, NULL},
	{_recvEnd, NULL},

};




static void _messageDelete(IRC_POKEMON_TRADE *pWork)
{
	if(pWork->MessageWin){
		GFL_BMPWIN_ClearScreen(pWork->MessageWin);
		GFL_BG_LoadScreenV_Req(GFL_BG_FRAME1_S);
		BmpWinFrame_Clear(pWork->MessageWin, WINDOW_TRANS_OFF);
		GFL_BMPWIN_Delete(pWork->MessageWin);
		pWork->MessageWin=NULL;
	}

}


//------------------------------------------------------------------
/**
 * $brief   YESNOウインドウシステムスタート
 *
 * @param   bgl		
 * @param   touch_sub_window_sys		
 *
 * @retval  static		
 */
//------------------------------------------------------------------
static void TouchYesNoStart( TOUCH_SW_SYS* touch_sub_window_sys )
{
	TOUCH_SW_PARAM param;

	// YES NO ウィンドウボタンの表示
	param.bg_frame	= GFL_BG_FRAME2_S;
	param.char_offs	= YESNO_CHARA_OFFSET+YESNO_CHARA_W*YESNO_CHARA_H;
	param.pltt_offs = 8;
	param.x			= 25;
	param.y			= 6;

	param.key_pos = 0;
	param.type = TOUCH_SW_TYPE_S;

	TOUCH_SW_Init( touch_sub_window_sys, &param );
	GFL_BG_LoadScreenV_Req( GFL_BG_FRAME0_M );

}

//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeCreateMcss( IRC_POKEMON_TRADE *pWork ,int no, const POKEMON_PASO_PARAM *ppp )
{
	MCSS_ADD_WORK addWork;
	VecFx32 scale = {FX32_ONE*16,FX32_ONE*16,FX32_ONE};
	int xpos[] = { PSTATUS_MCSS_POS_X1 , PSTATUS_MCSS_POS_X2};

	GF_ASSERT( pWork->pokeMcss[no] == NULL );

	MCSS_TOOL_MakeMAWPPP( ppp , &addWork , MCSS_DIR_FRONT );
	pWork->pokeMcss[no] = MCSS_Add( pWork->mcssSys , xpos[no] , PSTATUS_MCSS_POS_Y ,0 , &addWork );
	MCSS_SetScale( pWork->pokeMcss[no] , &scale );
}

//--------------------------------------------------------------
//	ポケモンMCSS削除
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeDeleteMcss( IRC_POKEMON_TRADE *pWork,int no  )
{
	if( pWork->pokeMcss[no] == NULL ){
		return;
	}

	MCSS_SetVanishFlag( pWork->pokeMcss[no] );
	MCSS_Del(pWork->mcssSys,pWork->pokeMcss[no]);
	pWork->pokeMcss[no] = NULL;
}

//--------------------------------------------------------------
//	ポケモン状態文章セット
//--------------------------------------------------------------

static void _setPokemonStatusMessage(IRC_POKEMON_TRADE *pWork, int side,const POKEMON_PASO_PARAM* ppp)
{


  int i=0,lv;
  int frame = GFL_BG_FRAME3_M;
	int sidew = side*4;

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

	if(!pWork->StatusWin[sidew]){
		int sidex[] = {5, 21};
		pWork->StatusWin[sidew] = GFL_BMPWIN_Create(frame,	sidex[side], 1, 9, 2,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
		pWork->StatusWin[sidew+1] = GFL_BMPWIN_Create(frame,	sidex[side], 3, 9, 2,	_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
	}

  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->StatusWin[sidew]), 0);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->StatusWin[sidew+1]), 0);
  GFL_BMPWIN_MakeScreen(pWork->StatusWin[sidew]);
  GFL_BMPWIN_MakeScreen(pWork->StatusWin[sidew+1]);

	PPP_Get(ppp, ID_PARA_nickname, pWork->pStrBuf);

	GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->StatusWin[sidew]), 0, 1, pWork->pStrBuf, pWork->pFontHandle);


	lv=	PPP_Get(ppp, ID_PARA_level, NULL);

	GFL_MSG_GetString(  pWork->pMsgData, IRCBTL_STR_21, pWork->pExStrBuf );
	WORDSET_RegisterNumber(pWork->pWordSet, 0, lv,
												 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);

	WORDSET_ExpandStr( pWork->pWordSet, pWork->pStrBuf, pWork->pExStrBuf  );
	
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->StatusWin[sidew+1]), 0, 1, pWork->pStrBuf, pWork->pFontHandle);
	


  GFL_BMPWIN_TransVramCharacter(pWork->StatusWin[sidew]);
  GFL_BMPWIN_TransVramCharacter(pWork->StatusWin[sidew+1]);

	GFL_BG_LoadScreenV_Req( frame );
	//OS_TPrintf("ポケモン文字表示\n");

}


//--------------------------------------------------------------
//	ポケモン表示セット
//--------------------------------------------------------------
static void _Pokemonset(IRC_POKEMON_TRADE *pWork, int side, const POKEMON_PASO_PARAM* ppp )
{
	PSTATUS_SUB_PokeDeleteMcss(pWork, side);
	PSTATUS_SUB_PokeCreateMcss(pWork, side, ppp );

	pWork->bPokemonSet[side]=TRUE;

	_setPokemonStatusMessage(pWork,side ,ppp);
}



static void _msgWindowCreate(IRC_POKEMON_TRADE* pWork,int strno)
{
  int i=0;
  int frame = GFL_BG_FRAME1_S;

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);

	if(pWork->MessageWin){
		return;
	}
	GFL_FONTSYS_SetDefaultColor();

  pWork->MessageWin = GFL_BMPWIN_Create(
    frame,
    ((0x20-_BUTTON_WIN_WIDTH)/2), (0x18-(2+_BUTTON_WIN_HEIGHT)), _BUTTON_WIN_WIDTH,_BUTTON_WIN_HEIGHT,
    _BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_F);
  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->MessageWin), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_MakeScreen(pWork->MessageWin);
  GFL_BMPWIN_TransVramCharacter(pWork->MessageWin);
  BmpWinFrame_Write( pWork->MessageWin, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );

  GFL_MSG_GetString(  pWork->pMsgData, strno, pWork->pStrBuf );

  PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->MessageWin), 4, 4, pWork->pStrBuf, pWork->pFontHandle);
  GFL_BMPWIN_TransVramCharacter(pWork->MessageWin);
	GFL_BG_LoadScreenV_Req( frame );
	//OS_TPrintf("メッセージ\n");

}




//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン位置を計算
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

#if 1

static void _getPokeIconPos(int index, GFL_CLACTPOS* pos)
{
	static const u8	iconSize = 24;
	static const u8 iconTop = 72-24;
	static const u8 iconLeft = 72-48;

	pos->x = (index % 6) * iconSize + iconLeft;
	pos->y = (index / 6) * iconSize + iconTop;
}


static const POKEMON_PASO_PARAM* _getPokeDataAddress(BOX_DATA* boxData , int trayNo, int index,IRC_POKEMON_TRADE* pWork)
{
	if(trayNo!=BOX_MAX_TRAY){
		return BOXDAT_GetPokeDataAddress(boxData,trayNo,index);
  }
	{
		POKEPARTY* party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(pWork->pGameSys));

		if(index < PokeParty_GetPokeCount(party)){
			const POKEMON_PARAM *pp = PokeParty_GetMemberPointer( party , index );
			return PP_GetPPPPointerConst( pp );
		}
	}
	return NULL;

}



//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン表示
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _InitBoxIcon( BOX_DATA* boxData , u8 trayNo ,IRC_POKEMON_TRADE* pWork )
{
	//パレット・セルファイルを共通で用意し使う
	//キャラクタファイルのみ読み替え
	u8 i;
	u8 loadNum;
	ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );


	for( i=0;i<BOX_MONS_NUM;i++ ){
		if(pWork->pokeIconNcgRes[i]){
			GFL_CLGRP_CGR_Release(pWork->pokeIconNcgRes[i]);
			pWork->pokeIconNcgRes[i] = NULL;
		}
		if(pWork->pokeIcon[i]){
			GFL_CLACT_WK_Remove(pWork->pokeIcon[i]);
			pWork->pokeIcon[i]=NULL;
		}
	}


	


	loadNum = 0;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		int	fileNo,monsno,formno,bEgg;
		const POKEMON_PASO_PARAM* ppp = _getPokeDataAddress(boxData,trayNo,i, pWork);
		if(ppp){
			monsno = PPP_Get(ppp,ID_PARA_monsno,NULL);

			if( monsno != 0 )	//ポケモンがいるかのチェック
			{
				GFL_CLWK_DATA cellInitData;
				u8 pltNum;
				GFL_CLACTPOS pos;
				
				pWork->pokeIconNcgRes[i] =
					GFL_CLGRP_CGR_Register( arcHandle , POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_SUB , pWork->heapID );



				
				pltNum = POKEICON_GetPalNumGetByPPP( ppp );
				_getPokeIconPos(i, &pos);
				
				cellInitData.pos_x = pos.x;
				cellInitData.pos_y = pos.y;
				cellInitData.anmseq = POKEICON_ANM_HPMAX;
				cellInitData.softpri = 0;
				cellInitData.bgpri = 1;
				pWork->pokeIcon[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
																									pWork->pokeIconNcgRes[i],
																									pWork->cellRes[PLT_POKEICON],
																									pWork->cellRes[ANM_POKEICON],
																									&cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
				GFL_CLACT_WK_SetPlttOffs( pWork->pokeIcon[i] , pltNum , CLWK_PLTTOFFS_MODE_PLTT_TOP );
				GFL_CLACT_WK_SetAutoAnmFlag( pWork->pokeIcon[i] , TRUE );
				GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[i], TRUE );
			}
		}
	}
	GFL_ARC_CloseDataHandle( arcHandle );


	{//ボックス名表示
		if(trayNo == BOX_MAX_TRAY){
			GFL_MSG_GetString(  pWork->pMsgData, IRCBTL_STR_19, pWork->pStrBuf );
		}
		else{
			BOXDAT_GetBoxName(boxData, trayNo, pWork->pStrBuf);
		}
		GFL_FONTSYS_SetColor( 0xf, 0xe, 0 );
		GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 0 );
		GFL_BMPWIN_MakeScreen(pWork->MyInfoWin);
		PRINTSYS_Print( GFL_BMPWIN_GetBmp(pWork->MyInfoWin), 1, 0, pWork->pStrBuf, pWork->pFontHandle);
		GFL_BMPWIN_TransVramCharacter(pWork->MyInfoWin);
		GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_S );

	}

}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(IRC_POKEMON_TRADE* pWork,StateFunc state)
{
	pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(IRC_POKEMON_TRADE* pWork,StateFunc state, int line)
{
	NET_PRINT("ircmatch: %d\n",line);
	_changeState(pWork, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンの受信バッファを返す
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8* _setChangePokemonBuffer(int netID, void* pWk, int size)
{
	IRC_POKEMON_TRADE *pWork = pWk;
	if((netID >= 0) && (netID < 2)){
		return (u8*)pWork->recvPoke[netID];
	}
	return NULL;
}



static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if(netID == GFL_NET_SystemGetCurrentID()){
		return;	//自分のデータは無視
	}

	_Pokemonset(pWork, 1, pWork->recvPoke[netID]);

}


//
//------------------------------------------------------------------------------
/**
 * @brief  はいいいえウインドウをだす
 * _NETCMD_CHANGE_YESNO
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _recvChangeYesNo(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	//はいいいえウインドウ

	_msgWindowCreate(pWork, IRCBTL_STR_20);

	TouchYesNoStart(pWork->TouchSubWindowSys);
	pWork->bTouchReset=TRUE;
	
	_CHANGE_STATE(pWork,_changeYesNoWaitState);


}

//_NETCMD_CHANGE_CANCEL	

static void _recvChangeCancel(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	//もどす
	if(pWork->bTouchReset){
		TOUCH_SW_Reset( pWork->TouchSubWindowSys );
		pWork->bTouchReset=FALSE;
	}
	_messageDelete(pWork);
	
	_CHANGE_STATE(pWork, _touchState);
	
}

//_NETCMD_END_REQ
static void _recvEndReq(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}

	_msgWindowCreate(pWork, IRCBTL_STR_22);
	TouchYesNoStart(pWork->TouchSubWindowSys);
	pWork->bTouchReset=TRUE;

	_CHANGE_STATE(pWork, _endWaitState);
}

//_NETCMD_END
static void _recvEnd(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}



	_CHANGE_STATE(pWork, NULL);
}


// _NETCMD_CHANGE_POKEMON
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	//OS_TPrintf("こうかん %d\n", netID);
	pWork->bChangeOK[netID] = TRUE;


//	_CHANGE_STATE(pWork, _changeWaitState);
}



//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取ったのでプロセス終了
 *	@param	IRC_POKEMON_TRADE		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTimingCheck(IRC_POKEMON_TRADE* pWork)
{
	if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO)){
		_CHANGE_STATE(pWork,NULL);
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	終了同期を取る
 *	@param	IRC_POKEMON_TRADE		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendTiming(IRC_POKEMON_TRADE* pWork)
{
	GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_ENDNO);
	_CHANGE_STATE(pWork,_sendTimingCheck);
}

static void _noneState(IRC_POKEMON_TRADE* pWork)
{
	
	//なにもしない
}

static void _messageWaitState(IRC_POKEMON_TRADE* pWork)
{
	if(GFL_UI_TP_GetTrg()){
		_messageDelete(pWork);
		_CHANGE_STATE(pWork, _touchState);
	}
}

static void _changeWaitState(IRC_POKEMON_TRADE* pWork)
{
	int i;
	int id = 1-GFL_NET_SystemGetCurrentID();

	for(i=0;i<2;i++){
		if(pWork->bChangeOK[i]==FALSE){
			return;
		}
	}

	//交換する
	// 相手のポケを自分の選んでいた場所に入れる
	if(pWork->selectBoxno == BOX_MAX_TRAY){ //もちものの交換の場合
		//@@OO PPPからPPを作ってくれる物が無いので仮
		POKEPARTY* party = GAMEDATA_GetMyPokemon(GAMESYSTEM_GetGameData(pWork->pGameSys));
		int monsno = PPP_Get(pWork->recvPoke[id],ID_PARA_monsno,NULL);
		int lv = PPP_Get(pWork->recvPoke[id],ID_PARA_level,NULL);
		int id2 = PPP_Get(pWork->recvPoke[id],ID_PARA_id_no,NULL);
		POKEMON_PARAM* pp = PP_Create(monsno,lv ,id2, pWork->heapID);

		{
			POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(monsno, 0, GFL_HEAPID_APP);
			u32 ret = POKE_PERSONAL_GetParam(ppd,POKEPER_ID_sex);

			PP_SetupEx(pp, monsno, lv, 123456,PTL_SETUP_POW_AUTO, ret);

			POKE_PERSONAL_CloseHandle(ppd);
		}

		{
			u16 oyaName[5] = {L'ブ',L'ラ',L'ッ',L'ク',0xFFFF};
			PP_Put( pp , ID_PARA_oyaname_raw , (u32)&oyaName[0] );
			PP_Put( pp , ID_PARA_oyasex , PTL_SEX_MALE );
		}


		
		PokeParty_SetMemberData(party, pWork->selectIndex, pp);
		GFL_HEAP_FreeMemory(pp);
	}
	else{
		//OS_TPrintf("change %d %d \n",pWork->selectBoxno, pWork->selectIndex);
		BOXDAT_PutPokemonPos(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,  pWork->recvPoke[id]);
	}
	pWork->selectBoxno = 0;
	pWork->selectIndex = -1;
	pWork->catchIndex = -1;
	PSTATUS_SUB_PokeDeleteMcss(pWork, 0);
	PSTATUS_SUB_PokeDeleteMcss(pWork, 1);

	_InitBoxIcon(pWork->pBox,pWork->nowBoxno,pWork);  //再描画

	_msgWindowCreate(pWork, IRCBTL_STR_23);

	pWork->bPokemonSet[0]=FALSE;
	pWork->bPokemonSet[1]=FALSE;
  
	_CHANGE_STATE(pWork, _messageWaitState);
}


static void _changeYesNoWaitState(IRC_POKEMON_TRADE* pWork)
{


	int result;
	result = TOUCH_SW_Main( pWork->TouchSubWindowSys );
	switch(result){				//こうかん
	case TOUCH_SW_RET_YES:						//はい
		if(pWork->bTouchReset){
			TOUCH_SW_Reset( pWork->TouchSubWindowSys );
			pWork->bTouchReset=FALSE;
		}
		_messageDelete(pWork);
		GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_POKEMON,0,NULL);
		_CHANGE_STATE(pWork,_changeWaitState);
		break;
	case TOUCH_SW_RET_NO:						//いいえ

		GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_CANCEL,0,NULL);
		
		_CHANGE_STATE(pWork,_noneState);
		break;
	}


}



static void _endWaitState(IRC_POKEMON_TRADE* pWork)
{

	int result;
	result = TOUCH_SW_Main( pWork->TouchSubWindowSys );
	switch(result){				//おわる
	case TOUCH_SW_RET_YES:						//はい

		GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_END,0,NULL);
		
		break;
	case TOUCH_SW_RET_NO:						//いいえ

		GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_CANCEL,0,NULL);
		
		_CHANGE_STATE(pWork,_noneState);
		break;
	}


}



static void _touchState(IRC_POKEMON_TRADE* pWork)
{
	u32 x,y,i;
	GFL_CLACTPOS pos;
	int backBoxNo = pWork->nowBoxno;

	for(i=0;i<2;i++){
		pWork->bChangeOK[i]=FALSE;
	}

	
	if(GFL_UI_TP_GetPointCont(&x,&y)){   //ベクトルを監視
		pWork->bUpVec = FALSE;
		if(pWork->y > y){
			pWork->bUpVec = TRUE;
		}
		pWork->x = x;
		pWork->y = y;
	}


	if(GFL_UI_TP_GetPointTrg(&x, &y)==TRUE){
		if((x >=  CHANGEBUTTON_X) && ((CHANGEBUTTON_X+CHANGEBUTTON_WIDTH) > x)){
			if((y >=  CHANGEBUTTON_Y) && ((CHANGEBUTTON_Y+CHANGEBUTTON_HEIGHT) > y)){   //交換ボタンを押す
				if(pWork->bPokemonSet[0] && pWork->bPokemonSet[1]){  //両方のポケモンがいる場合
					GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_CHANGE_YESNO, 0, NULL);
					_CHANGE_STATE(pWork,_noneState);
					return;
				}
			}
		}
		
		if((x >=  40) && (64 > x)){
			if((y >=  CONTROL_PANEL_Y) && (CONTROL_PANEL_Y+24 > y)){
				pWork->nowBoxno++;
			}
		}
		if((x >=  8) && (32 > x)){
			if((y >=  CONTROL_PANEL_Y) && (CONTROL_PANEL_Y+24 > y)){
				pWork->nowBoxno--;
			}
		}

		if((x >=(232-40)) && ((232-16) > x)){
			if((y >=  CONTROL_PANEL_Y) && (CONTROL_PANEL_Y+24 > y)){
				GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_END_REQ, 0, NULL);
				_CHANGE_STATE(pWork,_noneState);
				return;
			}
		}
		
		if(pWork->nowBoxno < 0){
			pWork->nowBoxno = BOX_MAX_TRAY;
		}
		else if(BOX_MAX_TRAY < pWork->nowBoxno){
			pWork->nowBoxno = 0;
		}
		
		if(backBoxNo !=  pWork->nowBoxno){
			_InitBoxIcon(pWork->pBox,pWork->nowBoxno,pWork);
		}

		if((x >=  8) && ((18*8+8) > x)){
			if((y >=  48) && ((48+8*15) > y)){
				int ans;
				x = (x - 8) / 24;
				y = (y - 40) / 24;
				ans = x + y * 6;
				if(ans >= 0 && ans < 30){
					pWork->catchIndex = ans;
				}
			}
		}
	}
	if(GFL_UI_TP_GetCont()==FALSE){  //ポケモンをつかむ

		if(pWork->nowBoxno == pWork->selectBoxno){
			if(pWork->selectIndex == pWork->catchIndex){
				pWork->catchIndex = -1;  //さっきと同じ物をつかんでいた場合は何もしない
			}
		}
		if(	pWork->pokeIcon[pWork->catchIndex]==NULL ){  //いないものはなにもしない
				pWork->catchIndex = -1;
		}

		
		if((pWork->catchIndex != -1) && !pWork->bUpVec){
			if(pWork->nowBoxno == pWork->selectBoxno){   //つかんでた物を元に戻す
				GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[pWork->catchIndex],TRUE);
				_getPokeIconPos(pWork->catchIndex, &pos);
				GFL_CLACT_WK_SetPos( pWork->pokeIcon[pWork->catchIndex], &pos, CLSYS_DRAW_SUB);
			}
		}
		if((pWork->catchIndex != -1) && pWork->bUpVec){
			if(pWork->selectIndex != -1){
				if(pWork->nowBoxno == pWork->selectBoxno){   //消えてたアイコン復活
					GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[pWork->selectIndex],TRUE);
					_getPokeIconPos(pWork->selectIndex, &pos);
					GFL_CLACT_WK_SetPos( pWork->pokeIcon[pWork->selectIndex], &pos, CLSYS_DRAW_SUB);
				}
			}

			pWork->selectIndex = pWork->catchIndex;
			pWork->selectBoxno = pWork->nowBoxno;
			GFL_CLACT_WK_SetDrawEnable( pWork->pokeIcon[pWork->catchIndex],FALSE);  //選択した物を消す

			{ //選択ポケモン表示
				const POKEMON_PASO_PARAM* ppp = _getPokeDataAddress(pWork->pBox, pWork->selectBoxno, pWork->selectIndex,pWork);

				_Pokemonset(pWork,0,ppp);

				//@@OO POKEMON_PASO_PARAMサイズ取得関数依頼中
				//@@OO 戻り値を見ないとばぐる
				GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_SELECT_POKEMON, sizeof( POKEMON_PASO_PARAM ),ppp);

			}

		}
		pWork->catchIndex = -1;
	}

	if(pWork->catchIndex != -1){
		if(GFL_UI_TP_GetPointCont(&x,&y)){
			pos.x = x;
			pos.y = y;
			if(pWork->pokeIcon[pWork->catchIndex]){
				GFL_CLACT_WK_SetPos( pWork->pokeIcon[pWork->catchIndex], &pos, CLSYS_DRAW_SUB);
			}
		}
	}



}

//----------------------------------------------------------------------------
/**
 *	@brief	ARCから読み込み
 *	@param	IRC_POKEMON_TRADE		ワーク
 */
//-----------------------------------------------------------------------------

static void _subBgMake(IRC_POKEMON_TRADE* pWork)
{
	ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );
	MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );
	u32 sex = MyStatus_GetMySex(pMy);



	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircbattle_DsTradeList_NCLR,
																		PALTYPE_MAIN_BG, 0, 0,  pWork->heapID);


	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_ircbattle_DsTradeList_Sub_NCGR,
																																GFL_BG_FRAME2_M, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_ircbattle_DsTradeList_Sub2_NSCR,
																				 GFL_BG_FRAME1_M, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);
	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_ircbattle_DsTradeList_Sub3_NSCR,
																				 GFL_BG_FRAME2_M, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);




	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_ircbattle_box_wp01_NCLR,
																		PALTYPE_SUB_BG, 0, 0,  pWork->heapID);


	// サブ画面BGキャラ転送
	pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_ircbattle_box_wp01_NCGR,
																																GFL_BG_FRAME2_S, 0, 0, pWork->heapID);

	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
																				 NARC_ircbattle_box_wp01_NSCR,
																				 GFL_BG_FRAME2_S, 0,
																				 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
																				 pWork->heapID);






	GFL_ARC_CloseDataHandle( p_handle );


  pWork->bgchar = BmpWinFrame_GraphicSetAreaMan(GFL_BG_FRAME1_S, _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, pWork->heapID);



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


//------------------------------------------------------------------------------
/**
 * @brief   BG領域設定
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createBg(IRC_POKEMON_TRADE* pWork)
{
	{
		static const GFL_DISP_VRAM vramBank = {
			GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,			// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0_F,			// テクスチャパレットスロット
			GX_OBJVRAMMODE_CHAR_1D_128K,	// メインOBJマッピングモード
			GX_OBJVRAMMODE_CHAR_1D_128K,		// サブOBJマッピングモード
		};
		GFL_DISP_SetBank( &vramBank );


		GFL_CLACT_SYS_Create(	&fldmapdata_CLSYS_Init, &vramBank, pWork->heapID );


	}
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS,GX_BGMODE_0,GX_BGMODE_0,GX_BG0_AS_3D
			};
		GFL_BG_SetBGMode( &sysHeader );
	}
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);

	{
		int frame = GFL_BG_FRAME3_M;
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
			};

		GFL_BG_SetBGControl(
			frame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
		GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
	{
		int frame = GFL_BG_FRAME2_M;
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE
			};

		GFL_BG_SetBGControl(
			frame, &bgcntText, GFL_BG_MODE_TEXT );
		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
		GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
	{
		int frame = GFL_BG_FRAME1_M;
		GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
			};

		GFL_BG_SetBGControl(
			frame, &bgcntText, GFL_BG_MODE_TEXT );
		//		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
		GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}
	{
		int frame = GFL_BG_FRAME0_S;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
			3, 0, 0, FALSE
			};
		GFL_BG_SetBGControl(
			frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
		GFL_BG_FillScreen( frame, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		//		GFL_BG_LoadScreenReq( frame );
		//        GFL_BG_ClearFrame(frame);
	}
	{
		int frame = GFL_BG_FRAME1_S;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x00000, 0x8000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};

		GFL_BG_SetBGControl(
			frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		//		GFL_BG_FillCharacter( frame, 0x00, 1, 0 );
		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
		//        GFL_BG_ClearFrame(frame);
	}
	{
		int frame = GFL_BG_FRAME2_S;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
			2, 0, 0, FALSE
			};

		GFL_BG_SetBGControl(
			frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

		GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( frame );
	}

	GFL_BG_SetBGControl3D( 0 );
	GFL_BG_SetVisible( GFL_BG_FRAME0_M , TRUE );

	//3D系の初期化
	{ //3D系の設定
		static const VecFx32 cam_pos = {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(101.0f)};
		static const VecFx32 cam_target = {FX32_CONST(0.0f),FX32_CONST(0.0f),FX32_CONST(-100.0f)};
		static const VecFx32 cam_up = {0,FX32_ONE,0};
		//エッジマーキングカラー
		static  const GXRgb edge_color_table[8]=
		{ GX_RGB( 0, 0, 0 ), GX_RGB( 0, 0, 0 ), 0, 0, 0, 0, 0, 0 };
		GFL_G3D_Init( GFL_G3D_VMANLNK, GFL_G3D_TEX256K, GFL_G3D_VMANLNK, GFL_G3D_PLT16K,
									0, pWork->heapID, NULL );

		//正射影カメラ
		pWork->camera =  GFL_G3D_CAMERA_Create( GFL_G3D_PRJORTH,
																						FX32_ONE*12.0f,
																						0,
																						0,
																						FX32_ONE*16.0f,
																						(FX32_ONE),
																						(FX32_ONE*200),
																						NULL,
																						&cam_pos,
																						&cam_up,
																						&cam_target,
																						pWork->heapID );

		GFL_G3D_CAMERA_Switching( pWork->camera );
		//エッジマーキングカラーセット
		G3X_SetEdgeColorTable( edge_color_table );
		G3X_EdgeMarking( TRUE );

		GFL_G3D_SetSystemSwapBufferMode( GX_SORTMODE_AUTO , GX_BUFFERMODE_Z );
	}


}



//------------------------------------------------------------------------------
/**
 * @brief   BOXカーソル表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _initBoxCursor(IRC_POKEMON_TRADE* pWork)
{
#if 0
	GFL_CLWK_DATA cellInitData;
	u8 pltNum,i;
	GFL_CLACTPOS pos;

	//素材
	{
		ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );

		pWork->cellRes[CHAR_BOX] =
			GFL_CLGRP_CGR_Register( p_handle , NARC_ircbattle_box_m_obj_NCGR ,
															FALSE , CLSYS_DRAW_SUB , pWork->heapID );
		pWork->cellRes[PLT_BOX] =
			GFL_CLGRP_PLTT_Register( p_handle ,
															 NARC_ircbattle_box_m_obj2_NCLR , CLSYS_DRAW_SUB , _OBJPLT_BOX * 32 , pWork->heapID  );
		pWork->cellRes[ANM_BOX] =
			GFL_CLGRP_CELLANIM_Register( p_handle ,
																	 NARC_ircbattle_box_m_obj_NCER, NARC_ircbattle_box_m_obj_NANR , pWork->heapID  );

		GFL_ARC_CloseDataHandle( p_handle );

	}

	
	//左右カーソル表示
	for(i = 0;i< CUR_NUM;i++){
		int posx[] = {64, 64+136, CHANGEBUTTON_X, YESBUTTON_X,NOBUTTON_X};
		int posy[] = {52, 52,     CHANGEBUTTON_Y, YESBUTTON_Y,NOBUTTON_Y};
		int anmseq[] = {
			NANR_box_m_obj_CellAnime1, NANR_box_m_obj_CellAnime3, NANR_box_m_obj_CellAnime16,
			NANR_box_m_obj_CellAnime17,NANR_box_m_obj_CellAnime18};
		int enable[]={TRUE,TRUE,TRUE,FALSE,FALSE};

		cellInitData.pos_x = posx[i];
		cellInitData.pos_y = posy[i];
		cellInitData.anmseq = anmseq[i];
		cellInitData.bgpri = 1;
		pWork->curIcon[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
																						 pWork->cellRes[CHAR_BOX],
																						 pWork->cellRes[PLT_BOX],
																						 pWork->cellRes[ANM_BOX],
																						 &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
		GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[i] , TRUE );
		GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], enable[i] );
	}
#endif
	GFL_CLWK_DATA cellInitData;
	u8 pltNum,i;
	GFL_CLACTPOS pos;

	//素材
	{
		ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_IRCBATTLE, pWork->heapID );
		pWork->cellRes[CHAR_BOX] =
			GFL_CLGRP_CGR_Register( p_handle , NARC_ircbattle_p_st_obj_d_NCGR ,
															FALSE , CLSYS_DRAW_SUB , pWork->heapID );
		pWork->cellRes[PLT_BOX] =
			GFL_CLGRP_PLTT_Register(
				p_handle ,NARC_ircbattle_p_st_obj_d_NCLR , CLSYS_DRAW_SUB ,
				_OBJPLT_BOX * 32 , pWork->heapID  );
		pWork->cellRes[ANM_BOX] =
			GFL_CLGRP_CELLANIM_Register(
				p_handle , NARC_ircbattle_p_st_obj_d_NCER, NARC_ircbattle_p_st_obj_d_NANR , pWork->heapID  );
		GFL_ARC_CloseDataHandle( p_handle );

	}

	
	//左右カーソル表示
	for(i = 0;i< CUR_NUM;i++){
		int posx[] = { 8,   40, 232-40 , 232-8, 72};
		int posy[] = {CONTROL_PANEL_Y, CONTROL_PANEL_Y, CONTROL_PANEL_Y, CONTROL_PANEL_Y,CONTROL_PANEL_Y};
		int anmseq[] = {
			NANR_p_st_obj_d_CellAnime1, NANR_p_st_obj_d_CellAnime0, NANR_p_st_obj_d_close,
			NANR_p_st_obj_d_return,NANR_p_st_obj_d_CellAnime2};
		int enable[]={TRUE,TRUE,TRUE,TRUE,TRUE};

		cellInitData.pos_x = posx[i];
		cellInitData.pos_y = posy[i];
		cellInitData.anmseq = anmseq[i];
		cellInitData.bgpri = 1;
		pWork->curIcon[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
																						 pWork->cellRes[CHAR_BOX],
																						 pWork->cellRes[PLT_BOX],
																						 pWork->cellRes[ANM_BOX],
																						 &cellInitData ,CLSYS_DRAW_SUB , pWork->heapID );
		GFL_CLACT_WK_SetAutoAnmFlag( pWork->curIcon[i] , TRUE );
		GFL_CLACT_WK_SetDrawEnable( pWork->curIcon[i], enable[i] );
	}

}


//------------------------------------------------------------------------------
/**
 * @brief   画面初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _dispInit(IRC_POKEMON_TRADE* pWork)
{
	//	BOX_DATA* pBox = SaveData_GetBoxData(GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pWork->pGameSys)));

	GFL_BG_Init( pWork->heapID );
	GFL_BMPWIN_Init( pWork->heapID );
	GFL_FONTSYS_Init();

	_createBg(pWork);
	_subBgMake(pWork);

	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

	//文字系初期化
	pWork->pWordSet    = WORDSET_Create( pWork->heapID );
	pWork->pMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_ircbattle_dat, pWork->heapID );
	pWork->pFontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
	pWork->MyInfoWin = GFL_BMPWIN_Create(GFL_BG_FRAME1_S, 0x07, 0x03, 10, 2, _BUTTON_MSG_PAL,  GFL_BMP_CHRAREA_GET_B );
	pWork->pStrBuf = GFL_STR_CreateBuffer( 128, pWork->heapID );
	pWork->pExStrBuf = GFL_STR_CreateBuffer( 128, pWork->heapID );

	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
                                0x20*_BUTTON_MSG_PAL, 0x20, pWork->heapID);
	
	//セル系システムの作成
	pWork->cellUnit = GFL_CLACT_UNIT_Create( 100 , 0 , pWork->heapID );

	_initBoxCursor(pWork);


	//ポケアイコン用リソース
	//キャラクタは各プレートで
#if 1
	{
		ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
		pWork->cellRes[PLT_POKEICON] = GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke ,
																																POKEICON_GetPalArcIndex() , CLSYS_DRAW_SUB ,
																																_OBJPLT_POKEICON*32 , pWork->heapID  );
		pWork->cellRes[ANM_POKEICON] = GFL_CLGRP_CELLANIM_Register( arcHandlePoke ,
																																POKEICON_GetCellSubArcIndex() , POKEICON_GetAnmArcIndex(), pWork->heapID  );

		GFL_ARC_CloseDataHandle(arcHandlePoke);
	}
#endif

	_InitBoxIcon(pWork->pBox, 0, pWork);

#if 1
	pWork->mcssSys = MCSS_Init( 2 , pWork->heapID );
	MCSS_SetTextureTransAdrs( pWork->mcssSys , 0 );
	MCSS_SetOrthoMode( pWork->mcssSys );
#endif

	GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );

	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);

	GFL_NET_ReloadIcon();

	_CHANGE_STATE(pWork, _touchState);
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
	GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank
}



//------------------------------------------------------------------------------
/**
 * @brief   PROCスタート
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	int i;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x20000 );

	{
		IRC_POKEMON_TRADE *pWork = GFL_PROC_AllocWork( proc, sizeof( IRC_POKEMON_TRADE ), HEAPID_IRCBATTLE );
		GFL_STD_MemClear(pWork, sizeof(IRC_POKEMON_TRADE));
		pWork->heapID = HEAPID_IRCBATTLE;
		pWork->selectType =  EVENT_IrcBattleGetType((EVENT_IRCBATTLE_WORK*) pwk);
		pWork->pParentWork = pwk;
		pWork->pGameSys = IrcBattle_GetGAMESYS_WORK(pWork->pParentWork);
		// 通信テーブル追加
		GFL_NET_AddCommandTable(GFL_NET_CMD_IRCTRADE,_PacketTbl,NELEMS(_PacketTbl), pWork);
		_CHANGE_STATE( pWork, _dispInit);
		pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

		pWork->pBox = SaveData_GetBoxData(GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pWork->pGameSys)));

		pWork->catchIndex = -1;
		pWork->selectIndex = -1;


		pWork->recvPoke[0] = GFL_HEAP_AllocClearMemory(pWork->heapID, sizeof(POKEMON_PASO_PARAM));
		pWork->recvPoke[1] = GFL_HEAP_AllocClearMemory(pWork->heapID, sizeof(POKEMON_PASO_PARAM));


		// 下画面ウインドウシステム初期化
		pWork->TouchSubWindowSys = TOUCH_SW_AllocWork( pWork->heapID );

		for(i=0;i<2;i++){
			pWork->bChangeOK[i]=FALSE;
		}
		

	}



	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	IRC_POKEMON_TRADE* pWork = mywk;
	GFL_PROC_RESULT retCode = GFL_PROC_RES_FINISH;

	StateFunc* state = pWork->state;
	if(state != NULL){
		state(pWork);
		retCode = GFL_PROC_RES_CONTINUE;
	}
	GFL_CLACT_SYS_Main(); // CLSYSメイン

	GFL_G3D_DRAW_Start();
	GFL_G3D_DRAW_SetLookAt();
	MCSS_Main( pWork->mcssSys );
	MCSS_Draw( pWork->mcssSys );
	GFL_G3D_DRAW_End();

	//	ConnectBGPalAnm_Main(&pWork->cbp);

	return retCode;
}

//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_PROC_RESULT IrcBattleFriendProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	int i;
	IRC_POKEMON_TRADE* pWork = mywk;
	EVENT_IRCBATTLE_WORK* pParentWork = pwk;

	GFL_NET_DelCommandTable(GFL_NET_CMD_IRCTRADE);
	
	TOUCH_SW_FreeWork( pWork->TouchSubWindowSys );

	if(pWork->MessageWin){
		GFL_BMPWIN_Delete(pWork->MessageWin);
	}
	if(pWork->MyInfoWin){
		GFL_BMPWIN_Delete(pWork->MyInfoWin);
	}
	

	for(i=0;i<8;i++){
		if(pWork->StatusWin[i]){
			GFL_BMPWIN_Delete(pWork->StatusWin[i]);
		}
	}
	if(pWork->pMsgData){
		GFL_MSG_Delete(pWork->pMsgData);
	}
	WORDSET_Delete(pWork->pWordSet);
	GFL_FONT_Delete(	pWork->pFontHandle );
	GFL_STR_DeleteBuffer(pWork->pStrBuf);
	GFL_STR_DeleteBuffer(pWork->pExStrBuf);


	GFL_G3D_CAMERA_Delete(pWork->camera);


	GFL_CLGRP_CGR_Release(pWork->cellRes[CHAR_BOX] );
	GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_BOX]);
	GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_BOX]);
	GFL_CLGRP_PLTT_Release(pWork->cellRes[PLT_POKEICON]);
	GFL_CLGRP_CELLANIM_Release(pWork->cellRes[ANM_POKEICON]);


	for(i=0;i<2;i++){
		if( pWork->pokeMcss[i] ){
			MCSS_Del(pWork->mcssSys,pWork->pokeMcss[i]);
		}
	}
	MCSS_Exit(pWork->mcssSys);
	

	for( i=0;i<BOX_MONS_NUM;i++ ){
		if(pWork->pokeIconNcgRes[i]){
			GFL_CLGRP_CGR_Release(pWork->pokeIconNcgRes[i]);
			pWork->pokeIconNcgRes[i] = NULL;
		}
		if(pWork->pokeIcon[i]){
			GFL_CLACT_WK_Remove(pWork->pokeIcon[i]);
			pWork->pokeIcon[i]=NULL;
		}
	}


	for(i = 0;i< CUR_NUM;i++){
		if(pWork->curIcon[i]){
			GFL_CLACT_WK_Remove(pWork->curIcon[i]);
		}
	}
	GFL_CLACT_UNIT_Delete(pWork->cellUnit);

	//GFL_NET_Exit(NULL);  //@@OO 本来はここで呼ばない


	GFL_CLACT_SYS_Delete();
	GFL_G3D_Exit();
	GFL_HEAP_FreeMemory(pWork->recvPoke[0]);
	GFL_HEAP_FreeMemory(pWork->recvPoke[1]);

	GFL_TCB_DeleteTask( pWork->g3dVintr );
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);

	EVENT_IrcBattle_SetEnd(pParentWork);



	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA IrcPokemonTradeProcData = {
	IrcBattleFriendProcInit,
	IrcBattleFriendProcMain,
	IrcBattleFriendProcEnd,
};


