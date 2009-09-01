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

#include "trade.naix"

#include "ircpokemontrade.h"
#include "ircpokemontrade_local.h"

#include "system/bmp_winframe.h"

#include "ircbattle.naix"
#include "trade.naix"


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
//	GFL_ARCHDL_UTIL_TransVramScreenCharOfs(p_handle,
	//																			 NARC_ircbattle_DsTradeList_Sub3_NSCR,
		//																		 GFL_BG_FRAME2_M, 0,
			//																	 GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
				//																 pWork->heapID);




	GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_trade_wb_trade_bg_NCLR,
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


