


//=============================================================================
/**
 * @file	  debug_saveaddr.c
 * @brief	  PDWでアクセスする物のアドレスリスト
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"

#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "net/network_define.h"
#include "savedata/wifilist.h"
#include "msg/msg_d_ohno.h"

#include "savedata/dreamworld_data.h"
#include "savedata/mystatus.h"
#include "../../savedata/mystatus_local.h"
#include "savedata/record.h"


typedef struct _SAVEADDR_WORK SAVEADDR_WORK;

typedef void (StateFunc)(SAVEADDR_WORK* pState);



static void _changeState(SAVEADDR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(SAVEADDR_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



struct _SAVEADDR_WORK {
	GFL_FONT		  *fontHandle;
	GFL_MSGDATA		*mm;
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	HEAPID heapID;
	STRBUF			*strbuf;
	STRBUF			*strbufEx;
  WORDSET			*WordSet;								// メッセージ展開用ワークマネージャー

	SAVE_CONTROL_WORK* pSaveData;
	StateFunc* state;      ///< ハンドルのプログラム状態
	vu32 count;

	void* buffer;
	int bufferSize;
	int hz;
	int req;
	int getdataCount;
	BOOL bEnd;
//	DWCGHTTPPost post;
	u8 mac[6];
//	u8 buffer[0x102];
};



static void _msgPrint(SAVEADDR_WORK* pWork,int msg)
{
	GFL_BMP_Clear( pWork->bmp, 0xff );
	GFL_MSG_GetString(pWork->mm, msg, pWork->strbuf);
	PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
	GFL_BMPWIN_TransVramCharacter( pWork->win );
}

static void _msgPrintNo(SAVEADDR_WORK* pWork,int msg,int no)
{
	GFL_BMP_Clear( pWork->bmp, 0xff );
	GFL_MSG_GetString(pWork->mm, msg, pWork->strbufEx);

	WORDSET_RegisterNumber(pWork->WordSet, 0, no,
												 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr( pWork->WordSet, pWork->strbuf,pWork->strbufEx );


	PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
	GFL_BMPWIN_TransVramCharacter( pWork->win );
}


//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(SAVEADDR_WORK* pWork,StateFunc state)
{
	pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef _NET_DEBUG
static void _changeStateDebug(SAVEADDR_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
	OS_TPrintf("ghttp: %d\n",line);
#endif
	_changeState(pWork, state);
}
#endif





//------------------------------------------------------------------------------
/**
 * @brief   キー入力で動きを変える
 * @retval  none
 */
//------------------------------------------------------------------------------

#include "savedata/situation.h"
#include "savedata/mystery_data.h"
#include "savedata/record.h"
#include "system\pms_data.h"
#include "savedata/worldtrade_data.h"
#include "savedata/sodateya_work.h"
#include "savedata/misc.h"
#include "savedata/misc_local.h"
#include "savedata/intrude_save.h"
#include "savedata/system_data_local.h"
#include "savedata/dreamworld_data_local.h"
#include	"poke_tool/pokeparty.h"
#include "savedata/wifihistory.h"
#include "savedata/wifihistory_local.h"
#include "savedata/regulation.h"
#include "net/dreamworld_netdata.h"



static void _keyWait(SAVEADDR_WORK* pWork)
{
  {
    int i;
    u32 size;
    u8* pAddr;
    u8* topAddr = (u8*)SaveControl_GetSaveWorkAdrs(pWork->pSaveData, &size);

    OS_TPrintf("SAVESIZE ,%x\n", size);

    {
      u8 data[]="abcdefg";
      OS_TPrintf("CRC ,%x %d\n", GFL_STD_CrcCalc(data,7) , sizeof(REGULATION_CARDDATA));
    }
    {
      DREAM_WORLD_SERVER_DOWNLOAD_DATA buff;

      GFL_STD_MemFill(&buff, 0, sizeof(buff));

      // buff に値を設定
      buff.findPokemon = 100;
      buff.findPokemon = 50;
      buff.findPokemonForm = 1;
      
      OS_TPrintf("CRC ,%x %d\n", GFL_STD_CrcCalc(&buff,sizeof(buff)-2) , sizeof(buff));
    }
    

    
/*
    for(i=0;i<26;i++){
      REGULATION* pData = GFL_ARC_UTIL_Load(ARCID_REGULATION, i, FALSE, HEAPID_PROC );

      OS_TPrintf("read %d \n",i);

      OS_TPrintf("LOHI %d %d\n",Regulation_GetParam(pData, REGULATION_NUM_LO), Regulation_GetParam(pData, REGULATION_NUM_HI));
      GFL_HEAP_FreeMemory(pData);
    }
*/
    {//ミステリー関連 
      MYSTERY_DATA *pMystery = SaveData_GetMysteryData(pWork->pSaveData); 
      GIFT_PACK_DATA  *pGift  = MYSTERYDATA_GetCardData( pMystery, 0 );
      pAddr = (u8*)pGift;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MYSTERY", (u32)pAddr-(u32)topAddr, sizeof(GIFT_PACK_DATA));
      OS_TPrintf( "download %d  pack%d present%d\n", sizeof(DOWNLOAD_GIFT_DATA), sizeof(GIFT_PACK_DATA), sizeof(GIFT_PRESENT) );
    }

    
    {//DreamWorld関連
      DREAMWORLD_SAVEDATA* pDW = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);
      pAddr = (u8*)DREAMWORLD_SV_GetSleepPokemon(pDW);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","SLEEP_POKEMON", (u32)pAddr-(u32)topAddr, POKETOOL_GetWorkSize());
      pAddr = (u8*)&pDW->pokemonIn;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","ISIN_SLEEP_POKEMON", (u32)pAddr-(u32)topAddr, sizeof(u8));
      
    }
    {//MISC
      MISC* pMisc = SaveData_GetMisc(pWork->pSaveData);
      pAddr = (u8*)&pMisc->gold;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_GOLD", (u32)pAddr-(u32)topAddr, sizeof(pMisc->gold));
    }


    {//Myステータス
      MYSTATUS* pMy = SaveData_GetMyStatus(pWork->pSaveData);
      pAddr = (u8*)&pMy->name;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PLAYER_NAME", (u32)pAddr-(u32)topAddr, sizeof(pMy->name));
      pAddr = (u8*)&pMy->id;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "PLAYER_ID",(u32)pAddr-(u32)topAddr,sizeof(pMy->id));
      pAddr = (u8*)&pMy->sex;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n", "PLAYER_SEX",((u32)pAddr-(u32)topAddr), sizeof(pMy->sex));
      pAddr = (u8*)&pMy->nation;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MY_NATION", (u32)pAddr-(u32)topAddr, sizeof(pMy->nation));
      pAddr = (u8*)&pMy->area;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","MY_AREA", (u32)pAddr-(u32)topAddr, sizeof(pMy->area));
    }

    
    { //ジオネット
      WIFI_HISTORY* pHis = SaveData_GetWifiHistory(pWork->pSaveData);

    }

    

    {//レコード
      long* rec = (long*)SaveData_GetRecord(pWork->pSaveData);

      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECORDTOP_ADDR", (u32)&rec[0]-(u32)topAddr, 4);

      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECID_CAPTURE_POKE", (u32)&rec[RECID_CAPTURE_POKE]-(u32)topAddr, 4);
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","RECID_FISHING_SUCCESS", (u32)&rec[RECID_FISHING_SUCCESS]-(u32)topAddr, 4);
    }

    {//システム
      SYSTEMDATA* pSys = SaveData_GetSystemData(pWork->pSaveData);

      pAddr = (u8*)&pSys->profileId;
      OS_TPrintf("\"%s\",\"0x%x\",\"%d\"\n","PROFILE_ID", (u32)pAddr-(u32)topAddr, sizeof(pSys->profileId));
    }

    {
      POKEPARTY* Party = SaveData_GetTemotiPokemon(pWork->pSaveData);
      for(i = 0 ; i < 6 ; i++){
        pAddr = (u8*)PokeParty_GetMemberPointer( Party, i );
        OS_TPrintf("\"%s%d\",\"0x%x\",\"%d\"\n","PARTY_POKEMON",i, (u32)pAddr-(u32)topAddr, POKETOOL_GetWorkSize());
      }
    }

    {
      BOX_MANAGER* pBox = BOX_DAT_InitManager( GFL_HEAPID_APP , pWork->pSaveData);
      int j,i;
      for(i=0;i<BOX_MAX_TRAY;i++){
        for(j=0;j<BOX_MAX_POS;j++){
          pAddr = (u8*)BOXDAT_GetPokeDataAddress(pBox, i, j);
          OS_TPrintf("\"%s%d\",\"0x%x\",\"%d\"\n","BOX_POKEMON",i*BOX_MAX_POS+j, (u32)pAddr-(u32)topAddr, POKETOOL_GetPPPWorkSize());
        }
      }
    }
  }

  {
    int size,sizetotal=0;
    size = Situation_GetWorkSize();
    OS_TPrintf("%d Situation_GetWorkSize\n",size);
    sizetotal+=size;

    size = MYSTERYDATA_GetWorkSize();
    OS_TPrintf("%d MYSTERYDATA_GetWorkSize\n",size);
    OS_TPrintf("%d MYSTERYDATA  GIFT_PRESENT_POKEMON\n",sizeof(GIFT_PRESENT_POKEMON));
    sizetotal+=size;

    size = RECORD_GetWorkSize();
    OS_TPrintf("%d RECORD_GetWorkSize\n",size);
    sizetotal+=size;

    size = PMSW_GetSaveDataSize();
    OS_TPrintf("%d PMSW_GetSaveDataSize\n",size);
    sizetotal+=size;

    size = WorldTradeData_GetWorkSize();
    OS_TPrintf("%d WorldTradeData_GetWorkSize\n",size);
    sizetotal+=size;

    size = SODATEYA_WORK_GetWorkSize();
    OS_TPrintf("%d SODATEYA_WORK_GetWorkSize\n",size);
    sizetotal+=size;

    size = MISC_GetWorkSize();
    OS_TPrintf("%d MISC_GetWorkSize\n",size);
    sizetotal+=size;

    size = IntrudeSave_GetWorkSize();
    OS_TPrintf("%d IntrudeSave_GetWorkSize\n",size);
    sizetotal+=size;

    size = DREAMWORLD_SV_GetWorkSize();
    OS_TPrintf("%d DREAMWORLD_SV_GetWorkSize\n",size);
    sizetotal+=size;

    OS_TPrintf("%d total\n",sizetotal);
  }

  _CHANGE_STATE(NULL);

}




static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};


static GFL_PROC_RESULT DebugSaveAddrProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );//テスト

	{
		SAVEADDR_WORK* pWork = GFL_PROC_AllocWork( proc, sizeof(SAVEADDR_WORK), HEAPID_PROC );

		pWork->pSaveData = SaveControl_GetPointer();  //デバッグ
		pWork->heapID = HEAPID_PROC;

		GFL_DISP_SetBank( &vramBank );

		//バックグラウンドの色を入れておく
		GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);
	
		// 各種効果レジスタ初期化
		G2_BlendNone();

		// BGsystem初期化
		GFL_BG_Init( pWork->heapID );
		GFL_BMPWIN_Init( pWork->heapID );
		GFL_FONTSYS_Init();

			//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
		GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		pWork->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 12, 24, 16, 0, GFL_BMP_CHRAREA_GET_F );
		pWork->bmp = GFL_BMPWIN_GetBmp( pWork->win);
		GFL_BMP_Clear( pWork->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( pWork->win );
		GFL_BMPWIN_TransVramCharacter( pWork->win );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		pWork->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
																			GFL_FONT_LOADTYPE_FILE, FALSE, pWork->heapID );

		pWork->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_ohno_dat, pWork->heapID );
		pWork->strbuf = GFL_STR_CreateBuffer(64, pWork->heapID);
		pWork->strbufEx = GFL_STR_CreateBuffer(64, pWork->heapID);
		pWork->WordSet    = WORDSET_Create( pWork->heapID );

	//フォントパレット転送
		GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
																	0, 0x20, HEAPID_PROC);

		
		_CHANGE_STATE(_keyWait);
	}
  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT DebugSaveAddrProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SAVEADDR_WORK* pWork = mywk;
	StateFunc* state = pWork->state;

	if( state ){
		state( pWork );
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT DebugSaveAddrProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	SAVEADDR_WORK* pWork = mywk;

	GFL_STR_DeleteBuffer(pWork->strbuf);
	GFL_STR_DeleteBuffer(pWork->strbufEx);
	GFL_MSG_Delete(pWork->mm);
	GFL_FONT_Delete(pWork->fontHandle);

	WORDSET_Delete( pWork->WordSet );

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_PROC);  //テスト
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA DebugSaveAddrProcData = {
  DebugSaveAddrProc_Init,
  DebugSaveAddrProc_Main,
  DebugSaveAddrProc_End,
};

