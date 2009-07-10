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
#include "print/wordset.h"
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
#include "net_app/connect_anm.h"
#include "../event_ircbattle.h"
#include "net/dwc_rapfriend.h"
#include "savedata/wifilist.h"
#include "savedata/box_savedata.h"  //デバッグアイテム生成用

#include "system/mcss.h"
#include "system/mcss_tool.h"

#define _TIMING_ENDNO (12)
#define BOX_MONS_NUM (30)
#define _BRIGHTNESS_SYNC (2)  // フェードのＳＹＮＣは要調整
#define _OBJPLT_POKEICON  (6)  //3本
#define PLIST_RENDER_MAIN (1)
#define PSTATUS_MCSS_POS_X1 (FX32_CONST(( 50 )/16.0f))
#define PSTATUS_MCSS_POS_X2 (FX32_CONST(( 190 )/16.0f))
#define PSTATUS_MCSS_POS_Y (FX32_CONST((192.0f-( 140 ))/16.0f))


typedef enum
{
  PLT_POKEICON,
  ANM_POKEICON,
  CEL_RESOURCE_MAX,
} CEL_RESOURCE;



typedef enum {
	_NETCMD_SELECT_POKEMON = GFL_NET_CMD_IRCBATTLE,
	_NETCMD_CHANGE_POKEMON,
} _BATTLEIRC_SENDCMD;

typedef void (StateFunc)(IRC_POKEMON_TRADE* pState);


typedef struct
{
	u32 pltIdx;
	u32 ncgIdx[BOX_MONS_NUM];
	u32 anmIdx;
}TRADE_CELL_RES;

typedef struct
{
	int		heapID_;
	u8		anmCnt_;
	BOOL	isInit_;
	BOOL	isInitBox_;	//一回BOXの表示をしているか？
	GFL_CLWK	*cellBox_[BOX_MONS_NUM];
	BOOL		isUseBoxData_[BOX_MONS_NUM];
	TRADE_CELL_RES	resCell_;

} TRADE_DISP_SYS;



struct _IRC_POKEMON_TRADE {
	POKEMON_PASO_PARAM* sendPoke;
	POKEMON_PASO_PARAM* recvPoke[2];
	EVENT_IRCBATTLE_WORK* pParentWork;
	StateFunc* state;      ///< ハンドルのプログラム状態
	int selectType;   // 接続タイプ
	HEAPID heapID;
	//    GFL_BMPWIN* buttonWin[_WINDOW_MAXNUM]; /// ウインドウ管理
	//    GFL_MSGDATA *pMsgData;  //
	//    WORDSET *pWordSet;								// メッセージ展開用ワークマネージャー
	//    GFL_FONT* pFontHandle;
	//    STRBUF* pStrBuf;
	//    BMPWINFRAME_AREAMANAGER_POS aPos;
  //3D
	BOX_DATA* pBox;
  GFL_G3D_CAMERA    *camera;
  GFL_BBD_SYS       *bbdSys;
  MCSS_SYS_WORK *mcssSys;
  MCSS_WORK     *pokeMcss[2];

	GFL_ARCUTIL_TRANSINFO subchar;
	u32 cellRes[CEL_RESOURCE_MAX];

	GAMESYS_WORK* pGameSys;

//	TRADE_DISP_SYS TradeDispWork;
	GFL_CLUNIT	*cellUnit;
	GFL_TCB *g3dVintr; //3D用vIntrTaskハンドル

	u32 pokeIconNcgRes[BOX_MONS_NUM];
	GFL_CLWK* pokeIcon[BOX_MONS_NUM];
	
	int windowNum;
	BOOL IsIrc;
	u32 connect_bit;
	BOOL connect_ok;
	BOOL receive_ok;
	u32 receive_result_param;
	u32 receive_first_param;

	u32 x;
	u32 y;
	BOOL bUpVec;
	int catchIndex;   //つかんでるポケモン
	int selectIndex;  //候補のポケモンIndex
	int selectBoxno;  //候補のポケモンBox

	BOOL bParent;
};



static void _changeState(IRC_POKEMON_TRADE* pWork,StateFunc* state);
static void _changeStateDebug(IRC_POKEMON_TRADE* pWork,StateFunc* state, int line);
static void _recvSelectPokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void _recvMystatus(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static u8* _setChangePokemonBuffer(int netID, void* pWork, int size);



#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



///通信コマンドテーブル
static const NetRecvFuncTable _PacketTbl[] = {
	{_recvSelectPokemon,   _setChangePokemonBuffer},    ///_NETCMD_SELECT_POKEMON
	{_recvChangePokemon,   NULL},    ///_NETCMD_SELECT_POKEMON

};


//--------------------------------------------------------------
//	ポケモンMCSS作成
//--------------------------------------------------------------
static void PSTATUS_SUB_PokeCreateMcss( IRC_POKEMON_TRADE *pWork ,int no, POKEMON_PASO_PARAM *ppp )
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



//------------------------------------------------------------------------------
/**
 * @brief   下画面ポケモン表示
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

#if 1

static void _InitBoxIcon( BOX_DATA* boxData , u8 trayNo ,IRC_POKEMON_TRADE* pWork )
{
	//パレット・セルファイルを共通で用意し使う
	//キャラクタファイルのみ読み替え
	u8 i;
	u8 loadNum;

	ARCHANDLE *arcHandle = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );


	loadNum = 0;
	for( i=0;i<BOX_MONS_NUM;i++ )
	{
		int	fileNo,monsno,formno,bEgg;
		POKEMON_PASO_PARAM* ppp = BOXDAT_GetPokeDataAddress(boxData,trayNo,i);
		monsno = PPP_Get(ppp,ID_PARA_monsno,NULL);

		if( monsno != 0 )	//ポケモンがいるかのチェック
		{
			GFL_CLWK_DATA cellInitData;
			u8 pltNum;
			static const u8	iconSize = 24;
			static const u8 iconTop = 72;
			static const u8 iconLeft = 72;

			pWork->pokeIconNcgRes[i] =
				GFL_CLGRP_CGR_Register( arcHandle , POKEICON_GetCgxArcIndex(ppp) , FALSE , CLSYS_DRAW_SUB , pWork->heapID );

			pltNum = POKEICON_GetPalNumGetByPPP( ppp );
			cellInitData.pos_x = (i%6) * iconSize + iconLeft;
			cellInitData.pos_y = (i/6) * iconSize + iconTop;
			cellInitData.anmseq = POKEICON_ANM_HPMAX;
			cellInitData.softpri = 0;
			cellInitData.bgpri = 0;
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
	GFL_ARC_CloseDataHandle( arcHandle );
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

//	POKEMON_PASO_PARAM* ppp = BOXDAT_GetPokeDataAddress(pBox,0,0);
	PSTATUS_SUB_PokeDeleteMcss(pWork, 1);
	PSTATUS_SUB_PokeCreateMcss(pWork, 1, pWork->recvPoke[netID] );

}
 

static void _recvChangePokemon(const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
	IRC_POKEMON_TRADE *pWork = pWk;

	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	if(netID == GFL_NET_SystemGetCurrentID()){
		return;	//自分のデータは無視
	}

	//交換する

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

//----------------------------------------------------------------------------
/**
 *	@brief	MyStatusを送る
 *	@param	IRC_POKEMON_TRADE		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendMystatus(IRC_POKEMON_TRADE* pWork)
{
	DWCFriendData friendData;

//	GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(),_NETCMD_MYSTATUS,
	//								 MyStatus_GetWorkSize(),
		//							 GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(IrcBattle_GetGAMESYS_WORK(pWork->pParentWork))));

	_CHANGE_STATE(pWork,_sendTiming);
}

//----------------------------------------------------------------------------
/**
 *	@brief	FriendDataを送る
 *	@param	IRC_POKEMON_TRADE		ワーク
 */
//-----------------------------------------------------------------------------

static void _sendFriendCode(IRC_POKEMON_TRADE* pWork)
{
	DWCFriendData friendData;



	_CHANGE_STATE(pWork,_sendMystatus);

}

static void _noneState(IRC_POKEMON_TRADE* pWork)
{
	//なにもしない
}



static void _touchState(IRC_POKEMON_TRADE* pWork)
{
	u32 x,y;


	if(GFL_UI_TP_GetPointCont(&x,&y)){   //ベクトルを監視
		pWork->bUpVec = FALSE;
		if(pWork->y > y){
			pWork->bUpVec = TRUE;
		}
		pWork->x = x;
		pWork->y = y;
	}
	

	if(GFL_UI_TP_GetPointTrg(&x,&y)==TRUE){

		if((x >=  72) && (186 > x)){
			if((y >=  56) && (160 > y)){
				x = (x - 72) / 24;
				y = (y - 56) / 24;
				pWork->catchIndex = x + y * 6;

				
			}
		}
	}
	if(GFL_UI_TP_GetCont()==FALSE){
		if((pWork->catchIndex!=-1) && pWork->bUpVec){

			pWork->selectIndex = pWork->catchIndex;
			pWork->selectBoxno = 0;
			{
				POKEMON_PASO_PARAM* ppp = BOXDAT_GetPokeDataAddress(pWork->pBox,0,0);
				PSTATUS_SUB_PokeDeleteMcss(pWork,0);
				PSTATUS_SUB_PokeCreateMcss(pWork,0,ppp);
			}
		}
		pWork->catchIndex = -1;
	}


	if(pWork->catchIndex != -1){
		GFL_CLACTPOS pos;
		if(GFL_UI_TP_GetPointCont(&x,&y)){
			pos.x = x;
			pos.y = y;
			GFL_CLACT_WK_SetPos( pWork->pokeIcon[pWork->catchIndex], &pos, CLSYS_DRAW_SUB);
		}
	}


	
}




static void _testState(IRC_POKEMON_TRADE* pWork)
{
	//BOX_DATA* pBox = SaveData_GetBoxData(GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pWork->pGameSys)));

	{
		POKEMON_PASO_PARAM* ppp = BOXDAT_GetPokeDataAddress(pWork->pBox,0,0);
		PSTATUS_SUB_PokeCreateMcss(pWork,0,ppp);
	}
	{
		POKEMON_PASO_PARAM* ppp = BOXDAT_GetPokeDataAddress(pWork->pBox,0,1);
		PSTATUS_SUB_PokeCreateMcss(pWork,1,ppp);
	}
	_CHANGE_STATE(pWork,_noneState);

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

  //ポケアイコン用リソース
  //キャラクタは各プレートで
#if 1
	{
    ARCHANDLE *arcHandlePoke = GFL_ARC_OpenDataHandle( ARCID_POKEICON , pWork->heapID );
    pWork->cellRes[PLT_POKEICON] = GFL_CLGRP_PLTT_RegisterComp( arcHandlePoke , 
          POKEICON_GetPalArcIndex() , CLSYS_DRAW_SUB , 
          _OBJPLT_POKEICON*32 , pWork->heapID  );
    pWork->cellRes[ANM_POKEICON] = GFL_CLGRP_CELLANIM_Register( arcHandlePoke , 
          POKEICON_GetCellArcIndex() , POKEICON_GetAnmArcIndex(), pWork->heapID  );
    
    GFL_ARC_CloseDataHandle(arcHandlePoke);
  }
#endif

	GFL_NET_ReloadIcon();

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
			GX_VRAM_TEX_0_C,				// テクスチャイメージスロット
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
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
			2, 0, 0, FALSE
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
			0, 0, 0, FALSE
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



	//セル系システムの作成
	pWork->cellUnit = GFL_CLACT_UNIT_Create( 100 , 0 , pWork->heapID );

	
	_InitBoxIcon(pWork->pBox, 0, pWork);

#if 1
  pWork->mcssSys = MCSS_Init( 2 , pWork->heapID );
  MCSS_SetTextureTransAdrs( pWork->mcssSys , 0 );
  MCSS_SetOrthoMode( pWork->mcssSys );
#endif
	
	GFL_BG_SetVisible( GFL_BG_FRAME0_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME1_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
	GFL_BG_SetVisible( GFL_BG_FRAME0_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME1_S, VISIBLE_OFF );
	GFL_BG_SetVisible( GFL_BG_FRAME2_S, VISIBLE_ON );

	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, _BRIGHTNESS_SYNC);


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
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_IRCBATTLE, 0x20000 );

	{
		IRC_POKEMON_TRADE *pWork = GFL_PROC_AllocWork( proc, sizeof( IRC_POKEMON_TRADE ), HEAPID_IRCBATTLE );
		GFL_STD_MemClear(pWork, sizeof(IRC_POKEMON_TRADE));
		pWork->heapID = HEAPID_IRCBATTLE;
		pWork->selectType =  EVENT_IrcBattleGetType((EVENT_IRCBATTLE_WORK*) pwk);
		pWork->pParentWork = pwk;
		pWork->pGameSys = IrcBattle_GetGAMESYS_WORK(pWork->pParentWork);
		// 通信テーブル追加
		GFL_NET_AddCommandTable(GFL_NET_CMD_IRCBATTLE,_PacketTbl,NELEMS(_PacketTbl), pWork);
		_CHANGE_STATE( pWork, _dispInit);
		pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

		pWork->pBox = SaveData_GetBoxData(GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pWork->pGameSys)));


		
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
	IRC_POKEMON_TRADE* pWork = mywk;
	EVENT_IRCBATTLE_WORK* pParentWork = pwk;

	GFL_TCB_DeleteTask( pWork->g3dVintr );
  GFL_BG_Exit();
  GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_IRCBATTLE);

	EVENT_IrcBattle_SetEnd(pParentWork);
  MCSS_Exit( pWork->mcssSys );


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


