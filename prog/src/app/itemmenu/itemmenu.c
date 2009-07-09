//============================================================================================
/**
 * @file	  itemmenu.h
 * @brief	  アイテムメニュー
 * @author	k.ohno
 * @date	  2009.06.30
 */
//============================================================================================


#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "item/item.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_init.h"
#include "gamesystem/game_event.h"
#include "gamesystem/game_data.h"
#include "system/bmp_winframe.h"

#include "message.naix"
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

//------------------------------------------------------------------
//------------------------------------------------------------------
#include "system/main.h"			//GFL_HEAPID_APP参照

//============================================================================================
//============================================================================================

#define DEBUG_ITEMDISP_FRAME (GFL_BG_FRAME1_S)


#define _DISP_INITX (1)
#define _DISP_INITY (1)
#define _DISP_SIZEX (30)
#define _DISP_SIZEY (20)
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (11)  // メッセージフォント
#define	FBMP_COL_WHITE		(15)
#define WINCLR_COL(col)	(((col)<<4)|(col))


#define _ITEMUSE_DISP_INITX (17)
#define _ITEMUSE_DISP_INITY (10)
#define _ITEMUSE_DISP_SIZEX (8)
#define _ITEMUSE_DISP_SIZEY (10)



#define FLD_SUBSCR_FADE_DIV (1)
#define FLD_SUBSCR_FADE_SYNC (1)

//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(FIELD_ITEMMENU_WORK* pWork,StateFunc* state);
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc* state, int line);
static void _itemNumSelectMenu(FIELD_ITEMMENU_WORK* wk);
static void _lineCallback(BMPMENULIST_WORK * mwk,u32 param,u8 y);
static void _windowRewrite(FIELD_ITEMMENU_WORK* wk);
static void _itemUseWindowCreate(FIELD_ITEMMENU_WORK* wk);
static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* wk);
static void _itemUseMenu(FIELD_ITEMMENU_WORK* wk);


#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(FIELD_ITEMMENU_WORK* wk,StateFunc state)
{
	wk->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(FIELD_ITEMMENU_WORK* wk,StateFunc state, int line)
{
	NET_PRINT("EvDebugItem: %d\n",line);
	_changeState(wk, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   絵の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _createSubBg(void)
{

	// 背景面
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
		int frame = GFL_BG_FRAME3_S;
		GFL_BG_BGCNT_HEADER TextBgCntDat = {
			0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000, 0x8000,GX_BG_EXTPLTT_01,
			0, 0, 0, FALSE
			};
		GFL_BG_SetBGControl(
			frame, &TextBgCntDat, GFL_BG_MODE_TEXT );
		GFL_BG_SetVisible( frame, VISIBLE_ON );
		GFL_BG_LoadScreenReq( frame );
	}
}


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


static void _graphicInit(FIELD_ITEMMENU_WORK* wk)
{
	G2_BlendNone();

	GFL_BG_Init(wk->heapID);
	GFL_BMPWIN_Init(wk->heapID);
	GFL_FONTSYS_Init();
	GFL_DISP_SetBank( &_defVBTbl );

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
	wk->SysMsgQue = PRINTSYS_QUE_Create( wk->heapID );


	wk->bgchar = BmpWinFrame_GraphicSetAreaMan(DEBUG_ITEMDISP_FRAME,
																						 _BUTTON_WIN_PAL, MENU_TYPE_SYSTEM, wk->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, wk->heapID);
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
																0x20*_BUTTON_MSG_PAL, 0x20, wk->heapID);
}



#define	FBMP_COL_NULL		(0)
#define	FBMP_COL_BLACK		(1)
#define	FBMP_COL_BLK_SDW	(2)
#define	FBMP_COL_RED		(3)
#define	FBMP_COL_RED_SDW	(4)
#define	FBMP_COL_GREEN		(5)
#define	FBMP_COL_GRN_SDW	(6)
#define	FBMP_COL_BLUE		(7)
#define	FBMP_COL_BLU_SDW	(8)
#define	FBMP_COL_PINK		(9)
#define	FBMP_COL_PNK_SDW	(10)
#define	FBMP_COL_WHITE		(15)
#define _BMPMENULIST_FONTSIZE   (16)




///選択メニューのリスト
static BMPMENULIST_HEADER _itemMenuListHeader = {
	NULL,			// 表示文字データポインタ
	NULL,					// カーソル移動ごとのコールバック関数
	NULL,					// 一列表示ごとのコールバック関数
	NULL,                      // GF_BGL_BMPWIN* win
	9,// リスト項目数
	9,// 表示最大項目数
	0,						// ラベル表示Ｘ座標
	16,						// 項目表示Ｘ座標
	0,						// カーソル表示Ｘ座標
	0,						// 表示Ｙ座標
	FBMP_COL_BLACK,			// 文字色
	FBMP_COL_WHITE,			// 背景色
	FBMP_COL_BLK_SDW,		// 文字影色
	0,						// 文字間隔Ｘ
	18,						// 文字間隔Ｙ
	BMPMENULIST_NO_SKIP,		// ページスキップタイプ
	0,//FONT_SYSTEM,				// 文字指定
	0,						// ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
	NULL,                   // work
	_BMPMENULIST_FONTSIZE,			//文字サイズX(ドット
	_BMPMENULIST_FONTSIZE,			//文字サイズY(ドット
	NULL,		//表示に使用するメッセージバッフ
	NULL,		//表示に使用するプリントユーティ
	NULL,		//表示に使用するプリントキュー
	NULL,		//表示に使用するフォントハンドル
};

static void _windowCreate(FIELD_ITEMMENU_WORK* wk)
{

	wk->win = GFL_BMPWIN_Create(
		DEBUG_ITEMDISP_FRAME,
		_DISP_INITX, _DISP_INITY,
		_DISP_SIZEX, _DISP_SIZEY,
		_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

	//  GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );
	GFL_BMPWIN_MakeScreen( wk->win );
	BmpWinFrame_Write( wk->win, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar), _BUTTON_WIN_PAL );
	_windowRewrite(wk);
}


static void _windowRewrite(FIELD_ITEMMENU_WORK* wk)
{
	BMPMENULIST_HEADER list_h = _itemMenuListHeader;
	int length,i;

	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->win), WINCLR_COL(FBMP_COL_WHITE) );

	length = MYITEM_GetItemPocketNumber( wk->pMyItem, wk->pocketno);

	if(wk->submenulist){
		BmpMenuWork_ListDelete(wk->submenulist);
	}
	wk->submenulist = BmpMenuWork_ListCreate(  length, wk->heapID );
	GF_ASSERT(wk->submenulist);
	
	for(i=0; i< length ; i++){
		ITEM_ST * item;
		item = MYITEM_PosItemGet( wk->pMyItem, wk->pocketno, i );
		if((item==NULL) || (item->id==ITEM_DUMMY_DATA)){
			break;
		}
		//OS_TPrintf("item no %d num %d\n",item->id,item->no);

		GFL_MSG_GetString(  wk->MsgManager, MSG_ITEMDEBUG_STR36, wk->pStrBuf );

		WORDSET_RegisterNumber(wk->WordSet, 0, item->no,
													 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
		WORDSET_RegisterItemName(wk->WordSet, 1, item->id);
		WORDSET_ExpandStr( wk->WordSet, wk->pExpStrBuf, wk->pStrBuf  );

		BmpMenuWork_ListAddString( wk->submenulist, wk->pExpStrBuf, 0,wk->heapID );
	}


	list_h.list = wk->submenulist;
	list_h.work = wk;
	list_h.win = wk->win;
	list_h.count = length;

	list_h.print_que = wk->SysMsgQue;
	PRINT_UTIL_Setup( &wk->SysMsgPrintUtil , wk->win );
	list_h.print_util = &wk->SysMsgPrintUtil;
	list_h.font_handle = wk->fontHandle;

	if(wk->sublw){
		BmpMenuList_Exit(wk->sublw,NULL,NULL);
	}
	wk->sublw = BmpMenuList_Set(&list_h, 0, 0, wk->heapID);
	BmpMenuList_SetCursorBmp(wk->sublw, wk->heapID);

}


//------------------------------------------------------------------------------
/**
 * @brief   キーの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemKindSelectMenu(FIELD_ITEMMENU_WORK* wk)
{
	u32	ret;

	ret = BmpMenuList_Main(wk->sublw);


	if( PRINT_UTIL_Trans( &wk->SysMsgPrintUtil, wk->SysMsgQue ) == TRUE){
		GFL_BMPWIN_TransVramCharacter( wk->win );
	}

	{
		int oldpoket = wk->pocketno;
		if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_R){
			wk->pocketno++;
		}
		if(GFL_UI_KEY_GetTrg()== PAD_BUTTON_L){
			wk->pocketno--;
		}
		if(wk->pocketno >= BAG_POKE_MAX){
			wk->pocketno = 0;
		}
		if(wk->pocketno < 0){
			wk->pocketno = BAG_POKE_MAX-1;
		}
		if(oldpoket != wk->pocketno){
			_windowRewrite(wk);
		}
	}


	switch(ret){
	case BMPMENULIST_NULL:
		return;
	case BMPMENULIST_CANCEL:
		BmpMenuList_Exit(wk->sublw, NULL, NULL);
		_CHANGE_STATE(wk, NULL);
		break;
	default:
		{
			ITEM_ST* item = MYITEM_PosItemGet(wk->pMyItem, wk->pocketno, ret);
			wk->ret_item = item->id;
		
			_itemUseWindowCreate(wk);
			BmpMenuList_Rewrite(wk->lwItemUse);
			GFL_BMPWIN_TransVramCharacter(wk->itemUseWin);
			
			_CHANGE_STATE(wk, _itemUseMenu);
		}
		OS_TPrintf("ret %d \n",ret);
		break;
	}
}

#if 0
//--------------------------------------------------------------------------------------------
/**
 * アイテム使用エラーメッセージセット
 *
 * @param	myst	プレーヤーデータ
 * @param	buf		メッセージ展開場所
 * @param	item	アイテム番号
 * @param	err		エラーID
 * @param	heap	ヒープID
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
static void BAG_ItemUseErrorMsgSet( MYSTATUS * myst, STRBUF * buf, u16 item, u32 err, FIELD_ITEMMENU_WORK * wk )
{
	WORDSET * wset;
	STRBUF * str;

	switch( err ){
	case ITEMCHECK_ERR_CYCLE_OFF:		// 自転車を降りれない
		GFL_MSG_GetString( wk->MsgManager, msg_bag_058, buf );
		break;
	case ITEMCHECK_ERR_COMPANION:		// 使用不可・連れ歩き
		GFL_MSG_GetString( wk->MsgManager, msg_bag_err_pair, buf );
		break;
	case ITEMCHECK_ERR_DISGUISE:		// 使用不可・ロケット団変装中
		GFL_MSG_GetString( wk->MsgManager, msg_bag_err_disguise, buf );
		break;
	default:							// 使用不可・博士の言葉
		wset = WORDSET_Create( wk->heapID );
		str  = GFL_MSG_CreateString( wk->MsgManager, msg_bag_059 );
		WORDSET_RegisterPlayerName( wset, 0, myst );
		WORDSET_ExpandStr( wset, buf, str );
		GFL_STR_DeleteBuffer( str );
		WORDSET_Delete( wset );
	}
}


//--------------------------------------------------------------------------------------------
/**
 * 会話（風）メッセージ表示
 *
 * @param	wk		バッグ画面ワーク
 * @param	type	BAG_TALK_FULL=画面一杯, BAG_TALK_MINI=画面2/3
 *
 * @return	メッセージID
 */
//--------------------------------------------------------------------------------------------
u8 Bag_TalkMsgPrint( BAG_WORK * wk, int type )
{
	u8	idx;
	GF_BGL_BMPWIN *win;
	if(type==BAG_TALK_FULL){
		win = &wk->win[WIN_TALK];
	}else{
		// 呼び出せない時がある
		GF_ASSERT( wk->sub_win[ADD_WIN_MINITALK].ini!=NULL );
		win = &wk->sub_win[ADD_WIN_MINITALK];
	}

	GF_BGL_BmpWinDataFill( win, 15 );
	BmpTalkWinWrite( win, WINDOW_TRANS_OFF, TALK_SUB_WIN_CGX_NUM, TALKWIN_PAL );
	GF_BGL_BmpWinOnVReq( win );

	MsgPrintSkipFlagSet( MSG_SKIP_ON );
	MsgPrintAutoFlagSet( MSG_AUTO_OFF );

	idx = GF_STR_PrintSimple(	win, FONT_TALK,wk->expb, 0, 0,
								CONFIG_GetMsgPrintSpeed( wk->cfg ), BAG_TalkMsgCallBack );

	return idx;
}

//--------------------------------------------------------------------------------------------
/**
 * メニュー：つかう
 *
 * @param	wk		バッグ画面のワーク
 *
 * @return	移行するメインシーケンス
 */
//--------------------------------------------------------------------------------------------
static int Bag_MenuUse( FIELD_ITEMMENU_WORK * wk )
{
	ITEMCHECK_FUNC	check;
	s32	id;

	// どうぐ使用処理取得
	id    = ITEM_GetParam( wk->ret_item, ITEM_PRM_FIELD, wk->heapID );
	// 使うチェックをする関数
	check = (ITEMCHECK_FUNC)ITEMUSE_GetFunc( ITEMUSE_PRM_CHECKFUNC, id );

	// どうぐ使用チェック
	if( check != NULL ){
		u32	ret = check( &wk->icwk );
		if( ret != ITEMCHECK_TRUE ){
			BAG_ItemUseErrorMsgSet( wk->mystatus, wk->expb, wk->ret_item, ret, wk );
			wk->midx = Bag_TalkMsgPrint( wk, BAG_TALK_FULL );
			return SEQ_ITEM_ERR_WAIT;
		}
	}

	return BAG_ItemUse( wk );
}
#endif


//--------------------------------------------------------------------------------------------
/**
 * Useメニュー作成
 * @param	wk		バッグ画面のワーク
 * @return	none
 */
//--------------------------------------------------------------------------------------------
#if 1

int (*MenuFuncTbl[])( FIELD_ITEMMENU_WORK *wk)={
	//Bag_MenuUse,		// つかう
	//Bag_MenuUse,		// おりる
	//Bag_MenuUse,		// みる
	//Bag_MenuUse,		// うめる
	//Bag_MenuUse,		// ひらく
	//Bag_MenuSub,		// すてる
	//Bag_MenuCnvSet,		// とうろく
	//Bag_MenuCnvDel,		// かいじょ
	//Bag_MenuSet,		// もたせる
	//Bag_MenuTag,		// タグをみる
	//Bag_MenuKettei,		// けってい
	NULL,				// やめる
	//Bag_MenuItemMove,	// いどう
	NULL,				// うる
	//Bag_MenuPlanterUse,	// つかう（木の実プランター用）
	//Bag_MenuUse,		// とめる
};

static BOOL BAGMAIN_NPlanterUseCheck( u16 pocket, u16 item )
{
	if( pocket == BAG_POKE_NUTS ||
		( item >= COMPOST_START_ITEMNUM && item < (COMPOST_START_ITEMNUM+ITEM_COMPOST_MAX) ) ){
		return TRUE;
	}
	return FALSE;
}

static void ItemMenuMake( FIELD_ITEMMENU_WORK * wk, u8* tbl )
{
	int	 i;
	void * itemdata;
	u8	pocket;

	itemdata = ITEM_GetItemArcData( wk->ret_item, ITEM_GET_DATA, wk->heapID );
	pocket   = wk->pocketno;

	for(i=0;i<BAG_MENUTBL_MAX;i++){
		wk->menu_func[i] = NULL;
	}

	// フィールド
	if( wk->mode == BAG_MODE_FIELD ){
		// コロシアム・ユニオンルームでは「みる」のみ
		if( wk->mode == BAG_MODE_COLOSSEUM || wk->mode == BAG_MODE_UNION )
		{
			tbl[BAG_MENU_USE] = BAG_MENU_MIRU;
		}
		else
		{
			// つかう
			// おりる
			// みる
			// ひらく
			// うめる
			// とめる
			if( ITEM_GetBufParam( itemdata,  ITEM_PRM_FIELD ) != 0 )
			{
				if( wk->ret_item == ITEM_ZITENSYA && wk->cycle_flg == 1 )
				{
					tbl[BAG_MENU_USE] = BAG_MENU_ORIRU;
				}
				else if( wk->pocketno == BAG_POKE_SEAL )
				{
					tbl[BAG_MENU_USE] = BAG_MENU_MIRU;
				}
				else if( wk->ret_item == ITEM_POFINKEESU )
				{
					tbl[BAG_MENU_USE] = BAG_MENU_HIRAKU;
				}
				//			else if( wk->ret_item == ITEM_gbPUREIYAA && Snd_GameBoyFlagCheck() == 1 )
				//		{
				//		tbl[BAG_MENU_USE] = BAG_MENU_TOMERU;
				//}
				else
				{
					tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU;
				}
			}
		}
		// もたせる
		// すてる
		if( ITEM_GetBufParam( itemdata, ITEM_PRM_EVENT ) == 0 ){
			if( ITEM_CheckPokeAdd( wk->ret_item ) == TRUE ){
				tbl[BAG_MENU_GIVE] = BAG_MENU_MOTASERU;
			}
			if( pocket != BAG_POKE_WAZA ){
				// 技マシンじゃない場合は捨てるを登録
				tbl[BAG_MENU_SUB] = BAG_MENU_SUTERU;
			}
		}
		// とうろく
		// かいじょ
		if( ITEM_GetBufParam( itemdata, ITEM_PRM_CNV ) != 0 ){
			if( MYITEM_CnvButtonItemGet( wk->pMyItem ) == wk->ret_item ){
				tbl[BAG_MENU_SUB] = BAG_MENU_KAIZYO;
			}else{
				tbl[BAG_MENU_SUB] = BAG_MENU_TOUROKU;
			}
		}
	}
	// 木の実プランター
	else if( wk->mode == BAG_MODE_N_PLANTER ){
		if( BAGMAIN_NPlanterUseCheck( pocket, wk->ret_item ) == TRUE ){
			tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU_NP;
		}
	}

	// わざマシンときのみでなければどうぐ移動メニューを登録
	if( wk->mode != BAG_MODE_N_PLANTER ){
		if(pocket!=BAG_POKE_WAZA && pocket!=BAG_POKE_NUTS){
			tbl[BAG_MENU_ITEMMOVE] = BAG_MENU_IDOU;
		}
	}
	// やめる
	tbl[BAG_MENU_CANCEL] = BAG_MENU_YAMERU;

	OS_Printf("pocket=%d\n",   pocket);
	OS_Printf("tbl[BAG_MENU_USE]=%d\n",   tbl[BAG_MENU_USE]);
	OS_Printf("tbl[BAG_MENU_GIVE]=%d\n",  tbl[BAG_MENU_GIVE]);
	OS_Printf("tbl[BAG_MENU_SUB]=%d\n",   tbl[BAG_MENU_SUB]);
	OS_Printf("tbl[BAG_MENU_MOVE]=%d\n",   tbl[BAG_MENU_ITEMMOVE]);
//	OS_Printf("tbl[BAG_MENU_CANCEL]=%d\n",tbl[BAG_MENU_CANCEL]);

	for(i=0;i<BAG_MENUTBL_MAX;i++){
		if(tbl[i]!=255){
			wk->menu_func[i] = MenuFuncTbl[tbl[i]];
		}
		OS_Printf("menu_func[%d]=%08x\n", i,wk->menu_func[i]);
	}

	// メニュー作成
//	Bag_ItemMenuSet( wk, tbl, 5 );
	// 選択メニューの初期化
//	SelectMenuInit(wk, tbl);

	GFL_HEAP_FreeMemory( itemdata );
}

#endif

static void _itemUseWindowCreate(FIELD_ITEMMENU_WORK* wk)
{

	wk->itemUseWin = GFL_BMPWIN_Create(
		DEBUG_ITEMDISP_FRAME,
		_ITEMUSE_DISP_INITX, _ITEMUSE_DISP_INITY,
		_ITEMUSE_DISP_SIZEX, _ITEMUSE_DISP_SIZEY,
		_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

	GFL_BMPWIN_MakeScreen( wk->itemUseWin );
	BmpWinFrame_Write( wk->itemUseWin, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar), _BUTTON_WIN_PAL );
	_itemUseWindowRewrite(wk);
}


static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* wk)
{
	BMPMENULIST_HEADER list_h = _itemMenuListHeader;
	int length,i;

	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(wk->itemUseWin), WINCLR_COL(FBMP_COL_WHITE) );

	length = BAG_MENUTBL_MAX;

	if(wk->itemUseMenuList){
		BmpMenuWork_ListDelete(wk->itemUseMenuList);
	}
	wk->itemUseMenuList = BmpMenuWork_ListCreate(  BAG_MENUTBL_MAX, wk->heapID );
	GF_ASSERT(wk->itemUseMenuList);

	{
		u8	tbl[BAG_MENUTBL_MAX]={255, 255, 255, 255, 255};
		int strtbl[]={msg_bag_001,msg_bag_007,msg_bag_017,mes_bag_104,mes_bag_105,
			msg_bag_002,msg_bag_003,msg_bag_019,msg_bag_004,msg_bag_005,msg_bag_006,
			msg_bag_009, msg_bag_094, mes_shop_103,msg_bag_001,msg_bag_menu_ex_01};

		
		ItemMenuMake(wk, tbl);
		
		for(i=0; i< BAG_MENUTBL_MAX ; i++){
			if(tbl[i]==255){
				continue;
			}
			OS_Printf("tblno %d %d\n",tbl[i],strtbl[tbl[i]] );
			
			GFL_MSG_GetString(  wk->MsgManager, strtbl[tbl[i]], wk->pStrBuf );
			BmpMenuWork_ListAddString( wk->itemUseMenuList, wk->pStrBuf, tbl[i], wk->heapID );
		}
	}


	list_h.list = wk->itemUseMenuList;
	list_h.work = wk;
	list_h.win = wk->itemUseWin;
	list_h.count = length;

	list_h.print_que = wk->SysMsgQue;
	PRINT_UTIL_Setup( &wk->SysMsgPrintUtil , wk->itemUseWin );
	list_h.print_util = &wk->SysMsgPrintUtil;
	list_h.font_handle = wk->fontHandle;

	if(wk->lwItemUse){
		BmpMenuList_Exit(wk->lwItemUse,NULL,NULL);
	}
	wk->lwItemUse = BmpMenuList_Set(&list_h, 0, 0, wk->heapID);
	BmpMenuList_SetCursorBmp(wk->lwItemUse, wk->heapID);

}




//------------------------------------------------------------------------------
/**
 * @brief   アイテムをどう使うかのメニュー
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemUseMenu(FIELD_ITEMMENU_WORK* wk)
{
	int ret;
	ret = BmpMenuList_Main(wk->lwItemUse);
	switch(ret){
	case BMPMENULIST_NULL:
		return;
	case BMPMENULIST_CANCEL:
		BmpMenuList_Exit(wk->lwItemUse, NULL, NULL);
//		GFL_BMPWIN_ClearScreen(wk->itemUseWin);
//		BmpWinFrame_Clear(wk->itemUseWin, WINDOW_TRANS_OFF);
		GFL_BMPWIN_MakeScreen( wk->win );
		GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
		GFL_BMPWIN_Delete(wk->itemUseWin);
		BmpMenuWork_ListDelete(wk->itemUseMenuList);

		wk->lwItemUse=NULL;
		wk->itemUseWin=NULL;
		wk->itemUseMenuList=NULL;
	
		_CHANGE_STATE(wk, _itemKindSelectMenu);
		break;
	default:
		break;
	}
}


//------------------------------------------------------------------------------
/**
 * @brief   プロセス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FIELD_ITEMMENU_WORK* wk = pwk;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ITEMMENU, 0x28000 );
	wk->heapID = HEAPID_ITEMMENU;

	_graphicInit(wk);

	wk->pMyItem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(wk->gsys));
	wk->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
																	 NARC_message_bag_dat, wk->heapID );
	wk->pStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
	wk->pExpStrBuf = GFL_STR_CreateBuffer(100,wk->heapID);
	wk->WordSet    = WORDSET_Create( wk->heapID );
	wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr ,
																		GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapID );

	GFL_UI_KEY_SetRepeatSpeed(1,6);
	_windowCreate(wk);

	BmpMenuList_Rewrite(wk->sublw);
	GFL_BMPWIN_TransVramCharacter(wk->win);

	//_windowRewrite(wk,0);

	WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
									WIPE_FADE_BLACK , FLD_SUBSCR_FADE_DIV , FLD_SUBSCR_FADE_SYNC , wk->heapID );

	_CHANGE_STATE(wk, _itemKindSelectMenu);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FIELD_ITEMMENU_WORK* wk = pwk;
	StateFunc* state = wk->state;

	if(state == NULL){
		return GFL_PROC_RES_FINISH;
	}
	if( WIPE_SYS_EndCheck() != TRUE ){
		return GFL_PROC_RES_CONTINUE;
	}
	state(wk);
	return GFL_PROC_RES_CONTINUE;

}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスEnd
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	FIELD_ITEMMENU_WORK* wk = pwk;

	GFL_BG_FreeCharacterArea(DEBUG_ITEMDISP_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(wk->bgchar),
													 GFL_ARCUTIL_TRANSINFO_GetSize(wk->bgchar));
	GFL_BMPWIN_ClearScreen(wk->win);
	BmpWinFrame_Clear(wk->win, WINDOW_TRANS_OFF);
	GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
	GFL_BMPWIN_Delete(wk->win);
	GFL_MSG_Delete(wk->MsgManager);
	GFL_STR_DeleteBuffer(wk->pStrBuf);
	GFL_STR_DeleteBuffer(wk->pExpStrBuf);
	WORDSET_Delete(wk->WordSet);
	BmpMenuWork_ListDelete(wk->submenulist);
	GFL_FONT_Delete(wk->fontHandle);
	PRINTSYS_QUE_Clear(wk->SysMsgQue);
	PRINTSYS_QUE_Delete(wk->SysMsgQue);

	GFL_BMPWIN_Exit();
	GFL_BG_Exit();


	GFL_HEAP_DeleteHeap(  HEAPID_ITEMMENU );

	return GFL_PROC_RES_FINISH;

}



// プロセス定義データ
const GFL_PROC_DATA ItemMenuProcData = {
	FieldItemMenuProc_Init,
	FieldItemMenuProc_Main,
	FieldItemMenuProc_End,
};
