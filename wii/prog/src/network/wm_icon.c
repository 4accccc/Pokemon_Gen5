//=============================================================================
/**
 * @file	wm_icon.c
 * @brief	通信中に表示されるアイコン
 * @author	gamefreak
 * @date    2007.3.1
 */
//=============================================================================

#include "gflib.h"
#include "net.h"
#include "wm_icon.h"
#include "device/net_whpipe.h"

// アニメパターンの数
#define WM_ICON_ANIME_MAX	 ( 4 )

#define WM_ICON_MAIN_OAM_ADRS	(GXOamAttr*)(HW_OAM)
#define WM_ICON_SUB_OAM_ADRS	(GXOamAttr*)(HW_OAM_END)

// メイン画面・サブ画面の切り替えに合わせる動作モード
enum {
	MAIN_SUB_MODE_DEFAULT,	///< 何もしない（メインに出続ける）
	MAIN_SUB_MODE_TOP,		///< 上画面に出続ける
	MAIN_SUB_MODE_BOTTOM,	///< 下画面に出続ける
};

//----------------------------------------------
/**
 *	管理用構造体定義
 */
//----------------------------------------------
typedef struct _VINTR_WIRELESS_ICON		VINTR_WIRELESS_ICON;

struct _VINTR_WIRELESS_ICON {
	int		anime_seq;
	int     anime;
	u16     x,y;
    u8      bWifi;
    u8      main_sub_mode;
    u8      sub_wrote_flag;
	const 	VOamAnm *table;
	GXOamAttr*  prevOamPtr;
};


//アイコンの位置
static VINTR_WIRELESS_ICON *VintrWirelessIconPtr = NULL;


//==============================================================
// Prototype
//==============================================================
static void WirelessIconAnimeFunc( void *work );
static void transIconData(int vramType,BOOL bWifi, int heapID);
static int get_target_vram( VINTR_WIRELESS_ICON* vwi );
static int calc_anime_index( int targetVram, int anime_ptn );


//==============================================================================
/**
 * Vblank通信アイコンアニメ開始関数
 *
 * @param   objVRAM		
 * @param   HeapId		ヒープ
 *
 * @retval  VINTR_WIRELESS_ICON *		
 */
//==============================================================================
//==============================================================================
/**
 * $brief   Vblank通信アイコンアニメ開始関数
 *
 * @param   objVRAM		未使用（消します）
 * @param   HeapId		
 * @param   x			表示位置X
 * @param   y			表示位置Y
 * @param   bWifi		DS通信アイコンか？wifi通信アイコンか？
 *
 * @retval  VINTR_WIRELESS_ICON *		
 */
//==============================================================================
static VINTR_WIRELESS_ICON *AddWirelessIconOAM(u32 objVRAM, u32 HeapId, int x, int y, BOOL bWifi,VINTR_WIRELESS_ICON* pVwi)
{
	VINTR_WIRELESS_ICON *vwi;

//    GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
    transIconData(NNS_G2D_VRAM_TYPE_2DMAIN, bWifi, HeapId);
    
    vwi = pVwi;
    if(vwi==NULL){
        // タスク登録・ワーク初期化
        vwi        = (VINTR_WIRELESS_ICON*)GFL_HEAP_AllocMemoryLo(HeapId, sizeof(VINTR_WIRELESS_ICON));
    }

	vwi->x     = x;
	vwi->y     = y;
	vwi->anime_seq = 0;
    vwi->anime     = 3;		// 0:3本 1:2本 2:1本 0:切断 (の通信アイコン）
    vwi->bWifi     = bWifi;
    vwi->sub_wrote_flag = FALSE;
    vwi->main_sub_mode = MAIN_SUB_MODE_DEFAULT;
    vwi->prevOamPtr = (GXOamAttr *)HW_OAM;

	return vwi;
}


//------------------------------------------------------------------
/**
 * 通信アイコンTCBメイン【タスク】
 *
 * @param   tcb		
 * @param   work	
 *
 * @retval  none	
 */
//------------------------------------------------------------------
static void WirelessIconAnimeFunc( void *work )
{
	VINTR_WIRELESS_ICON *vwi = (VINTR_WIRELESS_ICON *)work;

	int targetVram, anime_index;
	GXOamAttr* pOam;

	targetVram = get_target_vram( vwi );
	anime_index = calc_anime_index( targetVram, vwi->anime );
	pOam = ( targetVram == NNS_G2D_VRAM_TYPE_2DMAIN )? WM_ICON_MAIN_OAM_ADRS : WM_ICON_SUB_OAM_ADRS;

	G2_SetOBJAttr(
		pOam,
    	vwi->x,
    	vwi->y,
    	0,
    	GX_OAM_MODE_NORMAL,
    	0,
    	GX_OAM_EFFECT_NONE,
    	GX_OAM_SHAPE_16x16,
	    GX_OAM_COLORMODE_16,
	    anime_index,
	    WM_ICON_PAL_POS,
	    0
	);

	if( pOam != vwi->prevOamPtr )
	{
		G2_SetOBJAttr(
			vwi->prevOamPtr,
	    	0,
	    	0,
	    	0,
	    	GX_OAM_MODE_NORMAL,
	    	0,
	    	GX_OAM_EFFECT_NODISPLAY,
	    	GX_OAM_SHAPE_16x16,
		    GX_OAM_COLORMODE_16,
		    0,
		    0,
		    0
		);
		vwi->prevOamPtr = pOam;
	}

}
//------------------------------------------------------------------
/**
 * 現在のターゲットVRAMを取得
 *
 * @param   vwi		
 *
 * @retval  int 	NNS_G2D_VRAM_TYPE_2DMAIN or NNS_G2D_VRAM_TYPE_2DSUB
 */
//------------------------------------------------------------------
static int get_target_vram( VINTR_WIRELESS_ICON* vwi )
{
	switch( vwi->main_sub_mode ){
	case MAIN_SUB_MODE_TOP:
		return (GX_GetDispSelect() == GX_DISP_SELECT_MAIN_SUB )? NNS_G2D_VRAM_TYPE_2DMAIN : NNS_G2D_VRAM_TYPE_2DSUB;

	case MAIN_SUB_MODE_BOTTOM:
		return (GX_GetDispSelect() == GX_DISP_SELECT_MAIN_SUB )? NNS_G2D_VRAM_TYPE_2DSUB : NNS_G2D_VRAM_TYPE_2DMAIN;

	default:
		return NNS_G2D_VRAM_TYPE_2DMAIN;
	}
}

//------------------------------------------------------------------
/**
 * アニメインデックス計算
 *
 * @param   targetVram		
 * @param   anime_ptn		
 *
 * @retval  int 			アニメインデックス
 */
//------------------------------------------------------------------
static int calc_anime_index( int targetVram, int anime_ptn )
{
	int vramMode, bank, mapping;

	if( targetVram == NNS_G2D_VRAM_TYPE_2DMAIN )
	{
		bank = GX_GetBankForOBJ();
		mapping = GX_GetOBJVRamModeChar();
	}
	else
	{
		bank = GX_GetBankForSubOBJ();
		mapping = GXS_GetOBJVRamModeChar();
	}

#if 1
    //return 4*anime_ptn;
    switch(mapping){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		return 4*anime_ptn;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		return 2*anime_ptn;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		return anime_ptn;
	case GX_OBJVRAMMODE_CHAR_1D_256K:
	default:
		GF_ASSERT(0 && "非対応のOBJマッピングモード");
		return anime_ptn;
	}
#else
    switch( bank ){
      case GX_VRAM_OBJ_16_F:  	//OBJに16KBytes確保します。VRAM-Fを割り当てます。
      case GX_VRAM_OBJ_16_G: 	//OBJに16KBytes確保します。VRAM-Gを割り当てます。
        return 512-16+4*anime_ptn;
      case GX_VRAM_OBJ_32_FG: 	//OBJに32KBytes確保します。VRAM-F,Gを割り当てます。
        return 1024-16+4*anime_ptn;
      case GX_VRAM_OBJ_64_E: 	//OBJに64KBytes確保します。VRAM-Eを割り当てます。
		return 1024-16+4*anime_ptn;
      case GX_VRAM_OBJ_80_EF: 	//OBJに80KBytes確保します。VRAM-E,Fを割り当てます。
      case GX_VRAM_OBJ_80_EG: 	//OBJに80KBytes確保します。VRAM-E,Gを割り当てます。
        return 640-4+1*anime_ptn;
      case GX_VRAM_OBJ_96_EFG: 	//OBJに96KBytes確保します。VRAM-E,F,Gを割り当てます。
      case GX_VRAM_OBJ_128_A: 	//OBJに128KBytes確保します。VRAM-Aを割り当てます。
      case GX_VRAM_OBJ_128_B: 	//OBJに128KBytes確保します。VRAM-Bを割り当てます。
        return 1024-4+1*anime_ptn;
      case GX_VRAM_OBJ_256_AB: 	//OBJに256KBytes確保します。VRAM-A,Bを割り当てます。
      default:
		return 1024-8+2*anime_ptn;
    }
#endif
}
//------------------------------------------------------------------
/**
 * 後が残らないように、アイコンが消えた状態のOAMを直書きしておく
 *
 * @param   oam		書き込み先アドレス
 *
 */
//------------------------------------------------------------------
static inline void clear_oam( GXOamAttr* oam )
{
	G2_SetOBJAttr(
		oam,
		0,
		0,
		0,
		GX_OAM_MODE_NORMAL,
		0,
		GX_OAM_EFFECT_NODISPLAY,
		GX_OAM_SHAPE_16x16,
		GX_OAM_COLORMODE_16,
		0,
		0,
		0
	);
}



//==============================================================================
/**
 * 通信アイコンアニメチェンジ
 *
 * @param   vwi		
 * @param   anime		
 *
 * @retval  none		
 */
//==============================================================================
static void WirelessIconAnimeChange(VINTR_WIRELESS_ICON *vwi, int anime)
{
    // アニメチェンジ
	if(anime<WM_ICON_ANIME_MAX)
	{
		vwi->anime = anime;
	}
}

//==============================================================================
/**
 * Vblank通信アイコン表示処理終了
 *
 * @param   vwi		
 *
 * @retval  none		
 */
//==============================================================================
static void WirelessIconEnd(VINTR_WIRELESS_ICON *vwi)
{
	clear_oam( WM_ICON_MAIN_OAM_ADRS );
	if( vwi->sub_wrote_flag )
	{
		clear_oam( WM_ICON_SUB_OAM_ADRS );
	}
	// ワーク解放
	GFL_HEAP_FreeMemory( vwi );
}


//==============================================================================
/**
 * 通信アイコンの表示LCDを上画面 or 下画面に固定する
 *
 * @param   vwi			アイコンワークポインタ
 * @param   bTop		TRUEだと上画面に固定／FALSEだと下画面に固定
 * @param   heapID		テンポラリ使用ヒープID
 *
 */
//==============================================================================
static void WirelessIconHoldLCD( VINTR_WIRELESS_ICON *vwi, BOOL bTop, HEAPID heapID )
{
    transIconData(NNS_G2D_VRAM_TYPE_2DSUB, vwi->bWifi, heapID);

    vwi->main_sub_mode = (bTop)? MAIN_SUB_MODE_TOP : MAIN_SUB_MODE_BOTTOM;
	vwi->sub_wrote_flag = TRUE;
}


//==============================================================================
/**
 * 通信アイコン位置の表示LCD固定をやめてデフォルトの挙動に戻す
 *
 * @param   vwi			アイコンワークポインタ
 *
 */
//==============================================================================
static void WirelessIconDefaultLCD( VINTR_WIRELESS_ICON* vwi )
{
	if( vwi->main_sub_mode != MAIN_SUB_MODE_DEFAULT )
	{
		vwi->main_sub_mode = MAIN_SUB_MODE_DEFAULT;
		vwi->sub_wrote_flag = FALSE;
		clear_oam( WM_ICON_SUB_OAM_ADRS );
	}
}

//==============================================================================
/**
 * @brief   vramTypeにあった キャラクターオフセットを返す
 * @retval   none
 */
//==============================================================================

static int _getCharOffset(int vramType)
{
    int vramMode,objBank,offset;
    
    // VRAM設定に合わせて転送位置を決定
    if( vramType == NNS_G2D_VRAM_TYPE_2DMAIN ){
        objBank = GX_GetBankForOBJ();
    }
    else{
        objBank = GX_GetBankForSubOBJ();
    }
#if 1
    return 0;
#else
	switch( objBank ){
      case GX_VRAM_OBJ_16_F:  	//OBJに16KBytes確保します。VRAM-Fを割り当てます。
      case GX_VRAM_OBJ_16_G: 	//OBJに16KBytes確保します。VRAM-Gを割り当てます。
        offset = WM_ICON_CHAR_OFFSET16;
        break;
      case GX_VRAM_OBJ_32_FG: 	//OBJに32KBytes確保します。VRAM-F,Gを割り当てます。
        offset = WM_ICON_CHAR_OFFSET32;
        break;
      case GX_VRAM_OBJ_64_E: 	//OBJに64KBytes確保します。VRAM-Eを割り当てます。
        offset = WM_ICON_CHAR_OFFSET64;
        break;
      case GX_VRAM_OBJ_80_EF: 	//OBJに80KBytes確保します。VRAM-E,Fを割り当てます。
      case GX_VRAM_OBJ_80_EG: 	//OBJに80KBytes確保します。VRAM-E,Gを割り当てます。
        offset = WM_ICON_CHAR_OFFSET80;
        break;
      case GX_VRAM_OBJ_96_EFG: 	//OBJに96KBytes確保します。VRAM-E,F,Gを割り当てます。
      case GX_VRAM_OBJ_128_A: 	//OBJに128KBytes確保します。VRAM-Aを割り当てます。
      case GX_VRAM_OBJ_128_B: 	//OBJに128KBytes確保します。VRAM-Bを割り当てます。
        offset = WM_ICON_CHAR_OFFSET128;
        break;
      case GX_VRAM_OBJ_256_AB: 	//OBJに256KBytes確保します。VRAM-A,Bを割り当てます。
      default:
        offset = WM_ICON_CHAR_OFFSET64;
    }
    return offset;
#endif
}

//==============================================================================
/**
 * @brief    パレットとキャラをVRAMへ転送する
 * @retval   none
 */
//==============================================================================

static void transIconData(int vramType,BOOL bWifi, int heapID)
{
    int aNoBuff[3];
    int palType,charNo,charType,vramMode;

    NET_ICONDATA_GetNoBuff(aNoBuff);
    if( vramType == NNS_G2D_VRAM_TYPE_2DMAIN ){
        palType = PALTYPE_MAIN_OBJ;
        charType = OBJTYPE_MAIN;
    }
    else{
        palType = PALTYPE_SUB_OBJ;
        charType = OBJTYPE_SUB;
    }
    OS_TPrintf("ICONPAL %d %d %d \n",NET_ICONDATA_GetTableID(), aNoBuff[GFL_NET_ICON_WMNCLR],palType);
    GFL_ARC_UTIL_TransVramPalette(NET_ICONDATA_GetTableID(), aNoBuff[GFL_NET_ICON_WMNCLR],
                                  palType, WM_ICON_PAL_OFFSET, WM_ICON_PAL_SIZE, heapID);

    if(bWifi){
        charNo = GFL_NET_ICON_WIFINCGR;
    }
    else{
        charNo = GFL_NET_ICON_WMNCGR;
    }
    GFL_ARC_UTIL_TransVramObjCharacter(NET_ICONDATA_GetTableID(),aNoBuff[charNo],charType,
                                       _getCharOffset(vramType),
                                       0,FALSE,heapID);
}

//==============================================================================
/**
 * $brief   超絶らくちん通信アイコン
 *
 * @param   mode		
 *
 * @retval  none		
 */
//==============================================================================
void GFL_NET_WirelessIconEasy(BOOL bWifi, HEAPID heapID)
{
    GFL_NET_WirelessIconEasyXY(GFL_WICON_POSX,GFL_WICON_POSY, bWifi, heapID);
}

//==============================================================================
/**
 * $brief   超絶らくちん通信アイコン 引数あり版
 *
 * @param   mode		
 *
 * @retval  none		
 */
//==============================================================================
void GFL_NET_WirelessIconEasyXY(int x,int y, BOOL bWifi,HEAPID heapID)
{
    if(!GFL_NET_IsIchneumon()){  // イクニューモンが無い場合通信してない
        return ;
    }
    GFL_NET_WirelessIconEasyEnd();
    VintrWirelessIconPtr = AddWirelessIconOAM(0,heapID, x, y, bWifi, VintrWirelessIconPtr);
}


//==============================================================================
/**
 * $brief   超絶らくちん通信アイコン削除
 *
 * @param   none		
 *
 * @retval  none		
 */
//==============================================================================
void GFL_NET_WirelessIconEasyEnd(void)
{
    if(VintrWirelessIconPtr){
        WirelessIconEnd(VintrWirelessIconPtr);
        VintrWirelessIconPtr = NULL;
    }
}

//==============================================================================
/**
 * $brief   通信レベルをセットする
 * @param   level
 * @retval  none
 */
//==============================================================================
void GFL_NET_WirelessIconEasy_SetLevel(int level)
{
    if(VintrWirelessIconPtr){
        WirelessIconAnimeChange(VintrWirelessIconPtr, level);
    }
}

//------------------------------------------------------------------
/**
 * 超絶らくちん通信アイコン：表示LCDを上画面or下画面に固定する
 *
 * @param   bTop		TRUEだと上画面／FALSEだと下画面
 * @param   heapID		テンポラリ使用ヒープID
 *
 */
//------------------------------------------------------------------
void GFL_NET_WirelessIconEasy_HoldLCD( BOOL bTop, HEAPID heapID )
{
    if(VintrWirelessIconPtr)
    {
		WirelessIconHoldLCD(VintrWirelessIconPtr, bTop, heapID );
	}
}

//------------------------------------------------------------------
/**
 * 超絶らくちん通信アイコン：表示LCDの固定を止め、デフォルトの挙動に戻す
 */
//------------------------------------------------------------------
void GFL_NET_WirelessIconEasy_DefaultLCD( void )
{
    if(VintrWirelessIconPtr){
		WirelessIconDefaultLCD( VintrWirelessIconPtr );
	}
}


//==============================================================================
/**
 * $brief   通信アイコン実行関数
 * @retval  none
 */
//==============================================================================
void GFL_NET_WirelessIconEasyFunc(void)
{
 
    if(VintrWirelessIconPtr){
        WirelessIconAnimeFunc( VintrWirelessIconPtr );
    }
}

