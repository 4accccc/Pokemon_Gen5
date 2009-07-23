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
#include "bag.naix"
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

#define _1CHAR (8)


//ならびは savedata/myitem_savedata.hに準拠
static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右
	{	7*_1CHAR,  10*_1CHAR,  3*_1CHAR, 6*_1CHAR },  //どうぐ
	{	0x0e*_1CHAR,  0x11*_1CHAR,  2*_1CHAR, 5*_1CHAR },  //回復
	{	0x10*_1CHAR,  0x13*_1CHAR,  6*_1CHAR, 8*_1CHAR },  //技マシン
	{	0x0e*_1CHAR,  0x11*_1CHAR,  10*_1CHAR, 13*_1CHAR },  //木の実
	{	7*_1CHAR,  10*_1CHAR,  9*_1CHAR, 12*_1CHAR },  //大切な物
	{GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------


static void _changeState(FIELD_ITEMMENU_WORK* pWork,StateFunc* state);
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc* state, int line);
static void _itemNumSelectMenu(FIELD_ITEMMENU_WORK* pWork);
static void _lineCallback(BMPMENULIST_WORK * mpWork,u32 param,u8 y);
static void _windowRewrite(FIELD_ITEMMENU_WORK* pWork);
static void _itemUseWindowCreate(FIELD_ITEMMENU_WORK* pWork);
static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork);
static void _itemUseMenu(FIELD_ITEMMENU_WORK* pWork);


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

static void _changeState(FIELD_ITEMMENU_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc state, int line)
{
	NET_PRINT("EvDebugItem: %d\n",line);
	_changeState(pWork, state);
}
#endif


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

static void _windowCreate(FIELD_ITEMMENU_WORK* pWork)
{
	_windowRewrite(pWork);

}


static void _windowRewrite(FIELD_ITEMMENU_WORK* pWork)
{

  ITEMDISP_upMessageRewrite(pWork);  
  ITEMDISP_CellMessagePrint(pWork);
  pWork->bChange = TRUE;


}

//------------------------------------------------------------------------------
/**
 * @brief   ポケットに応じたカーソル位置を覚えておく 新しいポケットでのカーソル位置を引き出す
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _pocketCursorChange(FIELD_ITEMMENU_WORK* pWork,int oldpocket, int newpocket)
{
  s16 cur,scr;
  
  MYITEM_FieldBagPocketSet(pWork->pBagCursor, oldpocket);
  MYITEM_FieldBagCursorSet(pWork->pBagCursor, oldpocket, pWork->curpos, pWork->oamlistpos+1 );
  MYITEM_FieldBagCursorGet(pWork->pBagCursor, newpocket, &cur, &scr );
  pWork->curpos = cur;
  pWork->oamlistpos = scr - 1;
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテムインデックスをカーソルとリストの位置から計算
 * @retval  none
 */
//------------------------------------------------------------------------------

int ITEMMENU_GetItemIndex(FIELD_ITEMMENU_WORK* pWork)
{
  return pWork->curpos + pWork->oamlistpos + 1;
}

//------------------------------------------------------------------------------
/**
 * @brief   キーがした押されたときの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _posplus(FIELD_ITEMMENU_WORK* pWork, int length)
{
  BOOL bChange = FALSE;
  
  if((pWork->curpos==4) && ((pWork->oamlistpos+7) < length)){
    //カーソルはそのままでリストが移動
    pWork->oamlistpos++;
    bChange = TRUE;
  }
  else if((pWork->curpos==4) && ((pWork->curpos+1) < length)){
    //リストの終端まで来たのでカーソルが移動
    pWork->curpos++;
    bChange = TRUE;
  }
  else if((pWork->curpos!=5) && ((pWork->curpos+1) < length)){
    pWork->curpos++;
    bChange = TRUE;
  }
  return bChange;
}


//------------------------------------------------------------------------------
/**
 * @brief   スクロールの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _itemScrollCheck(FIELD_ITEMMENU_WORK* pWork)
{
  u32 x,y,i;
  int ymax = _SCROLL_BOTTOM_Y - _SCROLL_TOP_Y;

  if(GFL_UI_TP_GetPointCont(&x, &y) == TRUE){
    if((y <= _SCROLL_TOP_Y)  || (y >= _SCROLL_BOTTOM_Y)){
      return FALSE;
    }
    if((x >= (32*8)) || (x <= (30*8)) ){
      return FALSE;
    }
    {
      int length = MYITEM_GetItemPocketNumber( pWork->pMyItem, pWork->pocketno);
      int num = (length * (y-_SCROLL_TOP_Y)) / ymax;

      pWork->curpos = 0;
      pWork->oamlistpos = -1;
      for(i = 0 ; i < num ; i++){
        _posplus(pWork, length);
      }
    }
    return TRUE;
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   キーの動きの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _keyMoveCheck(FIELD_ITEMMENU_WORK* pWork)
{
  BOOL bChange = FALSE;
  {
    int pos = pWork->curpos;
    int length = MYITEM_GetItemPocketNumber( pWork->pMyItem, pWork->pocketno);
    
		if(GFL_UI_KEY_GetRepeat()== PAD_KEY_DOWN){
      bChange = _posplus(pWork, length);
		}
		if(GFL_UI_KEY_GetRepeat()== PAD_KEY_UP){
      if((pWork->curpos==1) && (pWork->oamlistpos!=-1)){
        //カーソルはそのままでリストが移動
        pWork->oamlistpos--;
        bChange = TRUE;
      }
      else if((pWork->curpos==1)){
        //リストの終端まで来たのでカーソルが移動
        pWork->curpos--;
        bChange = TRUE;
      }
      else if(pWork->curpos != 0){
        pWork->curpos--;
        bChange = TRUE;
      }
		}
  }
  return bChange;
}


//------------------------------------------------------------------------------
/**
 * @brief   キーの処理
 * @retval  none
 */
//------------------------------------------------------------------------------


//カーソルの位置
//OAMLIST の 先頭位置 -1は表示しない



static void _itemKindSelectMenu(FIELD_ITEMMENU_WORK* pWork)
{
	u32	ret=0;
  BOOL bChange=FALSE;

	if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_B){
    _CHANGE_STATE(pWork,NULL);
    return;
  }

  if( _itemScrollCheck(pWork) ){
    ITEMDISP_scrollCursorMove(pWork);
    bChange = TRUE;
  }
  else if(_keyMoveCheck(pWork)){
    bChange = TRUE;
  }
  {
		int oldpocket = pWork->pocketno;
		if(GFL_UI_KEY_GetTrg() == PAD_KEY_RIGHT){
			pWork->pocketno++;
		}
		if(GFL_UI_KEY_GetTrg() == PAD_KEY_LEFT){
			pWork->pocketno--;
		}
		if(pWork->pocketno >= BAG_POKE_MAX){
			pWork->pocketno = 0;
		}
		if(pWork->pocketno < 0){
			pWork->pocketno = BAG_POKE_MAX-1;
		}
		if(oldpocket != pWork->pocketno){
      _pocketCursorChange(pWork, oldpocket, pWork->pocketno);
      bChange = TRUE;
		}
	}

  if(bChange){
    _windowRewrite(pWork);
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
static void BAG_ItemUseErrorMsgSet( MYSTATUS * myst, STRBUF * buf, u16 item, u32 err, FIELD_ITEMMENU_WORK * pWork )
{
	WORDSET * wset;
	STRBUF * str;

	switch( err ){
	case ITEMCHECK_ERR_CYCLE_OFF:		// 自転車を降りれない
		GFL_MSG_GetString( pWork->MsgManager, msg_bag_058, buf );
		break;
	case ITEMCHECK_ERR_COMPANION:		// 使用不可・連れ歩き
		GFL_MSG_GetString( pWork->MsgManager, msg_bag_err_pair, buf );
		break;
	case ITEMCHECK_ERR_DISGUISE:		// 使用不可・ロケット団変装中
		GFL_MSG_GetString( pWork->MsgManager, msg_bag_err_disguise, buf );
		break;
	default:							// 使用不可・博士の言葉
		wset = WORDSET_Create( pWork->heapID );
		str  = GFL_MSG_CreateString( pWork->MsgManager, msg_bag_059 );
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
 * @param	pWork		バッグ画面ワーク
 * @param	type	BAG_TALK_FULL=画面一杯, BAG_TALK_MINI=画面2/3
 *
 * @return	メッセージID
 */
//--------------------------------------------------------------------------------------------
u8 Bag_TalkMsgPrint( BAG_WORK * pWork, int type )
{
	u8	idx;
	GF_BGL_BMPWIN *win;
	if(type==BAG_TALK_FULL){
		win = &pWork->win[WIN_TALK];
	}else{
		// 呼び出せない時がある
		GF_ASSERT( pWork->sub_win[ADD_WIN_MINITALK].ini!=NULL );
		win = &pWork->sub_win[ADD_WIN_MINITALK];
	}

	GF_BGL_BmpWinDataFill( win, 15 );
	BmpTalkWinWrite( win, WINDOW_TRANS_OFF, TALK_SUB_WIN_CGX_NUM, TALKWIN_PAL );
	GF_BGL_BmpWinOnVReq( win );

	MsgPrintSkipFlagSet( MSG_SKIP_ON );
	MsgPrintAutoFlagSet( MSG_AUTO_OFF );

	idx = GF_STR_PrintSimple(	win, FONT_TALK,pWork->expb, 0, 0,
								CONFIG_GetMsgPrintSpeed( pWork->cfg ), BAG_TalkMsgCallBack );

	return idx;
}

//--------------------------------------------------------------------------------------------
/**
 * メニュー：つかう
 *
 * @param	pWork		バッグ画面のワーク
 *
 * @return	移行するメインシーケンス
 */
//--------------------------------------------------------------------------------------------
static int Bag_MenuUse( FIELD_ITEMMENU_WORK * pWork )
{
	ITEMCHECK_FUNC	check;
	s32	id;

	// どうぐ使用処理取得
	id    = ITEM_GetParam( pWork->ret_item, ITEM_PRM_FIELD, pWork->heapID );
	// 使うチェックをする関数
	check = (ITEMCHECK_FUNC)ITEMUSE_GetFunc( ITEMUSE_PRM_CHECKFUNC, id );

	// どうぐ使用チェック
	if( check != NULL ){
		u32	ret = check( &pWork->icpWork );
		if( ret != ITEMCHECK_TRUE ){
			BAG_ItemUseErrorMsgSet( pWork->mystatus, pWork->expb, pWork->ret_item, ret, pWork );
			pWork->midx = Bag_TalkMsgPrint( pWork, BAG_TALK_FULL );
			return SEQ_ITEM_ERR_WAIT;
		}
	}

	return BAG_ItemUse( pWork );
}
#endif



//--------------------------------------------------------------------------------------------
/**
 * Useメニュー作成
 * @param	pWork		バッグ画面のワーク
 * @return	none
 */
//--------------------------------------------------------------------------------------------
#if 1

int (*MenuFuncTbl[])( FIELD_ITEMMENU_WORK *pWork)={
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

static void ItemMenuMake( FIELD_ITEMMENU_WORK * pWork, u8* tbl )
{
	int	 i;
	void * itemdata;
	u8	pocket;

	itemdata = ITEM_GetItemArcData( pWork->ret_item, ITEM_GET_DATA, pWork->heapID );
	pocket   = pWork->pocketno;

	for(i=0;i<BAG_MENUTBL_MAX;i++){
		pWork->menu_func[i] = NULL;
	}

	// フィールド
	if( pWork->mode == BAG_MODE_FIELD ){
		// コロシアム・ユニオンルームでは「みる」のみ
		if( pWork->mode == BAG_MODE_COLOSSEUM || pWork->mode == BAG_MODE_UNION )
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
				if( pWork->ret_item == ITEM_ZITENSYA && pWork->cycle_flg == 1 )
				{
					tbl[BAG_MENU_USE] = BAG_MENU_ORIRU;
				}
				else if( pWork->ret_item == ITEM_POFINKEESU )
				{
					tbl[BAG_MENU_USE] = BAG_MENU_HIRAKU;
				}
				//			else if( pWork->ret_item == ITEM_gbPUREIYAA && Snd_GameBoyFlagCheck() == 1 )
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
			if( ITEM_CheckPokeAdd( pWork->ret_item ) == TRUE ){
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
			if( MYITEM_CnvButtonItemGet( pWork->pMyItem ) == pWork->ret_item ){
				tbl[BAG_MENU_SUB] = BAG_MENU_KAIZYO;
			}else{
				tbl[BAG_MENU_SUB] = BAG_MENU_TOUROKU;
			}
		}
	}
	// 木の実プランター
	else if( pWork->mode == BAG_MODE_N_PLANTER ){
		if( BAGMAIN_NPlanterUseCheck( pocket, pWork->ret_item ) == TRUE ){
			tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU_NP;
		}
	}

	// わざマシンときのみでなければどうぐ移動メニューを登録
	if( pWork->mode != BAG_MODE_N_PLANTER ){
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
			pWork->menu_func[i] = MenuFuncTbl[tbl[i]];
		}
		OS_Printf("menu_func[%d]=%08x\n", i,pWork->menu_func[i]);
	}

	// メニュー作成
//	Bag_ItemMenuSet( pWork, tbl, 5 );
	// 選択メニューの初期化
//	SelectMenuInit(pWork, tbl);

	GFL_HEAP_FreeMemory( itemdata );
}

#endif

static void _itemUseWindowCreate(FIELD_ITEMMENU_WORK* pWork)
{

	pWork->itemUseWin = GFL_BMPWIN_Create(
		DEBUG_ITEMDISP_FRAME,
		_ITEMUSE_DISP_INITX, _ITEMUSE_DISP_INITY,
		_ITEMUSE_DISP_SIZEX, _ITEMUSE_DISP_SIZEY,
		_BUTTON_MSG_PAL, GFL_BMP_CHRAREA_GET_B );

	GFL_BMPWIN_MakeScreen( pWork->itemUseWin );
	BmpWinFrame_Write( pWork->itemUseWin, WINDOW_TRANS_ON, GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar), _BUTTON_WIN_PAL );
	_itemUseWindowRewrite(pWork);
}


static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork)
{
	BMPMENULIST_HEADER list_h = _itemMenuListHeader;
	int length,i;

	GFL_BMP_Clear(GFL_BMPWIN_GetBmp(pWork->itemUseWin), WINCLR_COL(FBMP_COL_WHITE) );

	length = BAG_MENUTBL_MAX;

	if(pWork->itemUseMenuList){
		BmpMenuWork_ListDelete(pWork->itemUseMenuList);
	}
	pWork->itemUseMenuList = BmpMenuWork_ListCreate(  BAG_MENUTBL_MAX, pWork->heapID );
	GF_ASSERT(pWork->itemUseMenuList);

	{
		u8	tbl[BAG_MENUTBL_MAX]={255, 255, 255, 255, 255};
		int strtbl[]={msg_bag_001,msg_bag_007,msg_bag_017,mes_bag_104,mes_bag_105,
			msg_bag_002,msg_bag_003,msg_bag_019,msg_bag_004,msg_bag_005,msg_bag_006,
			msg_bag_009, msg_bag_094, mes_shop_103,msg_bag_001,msg_bag_menu_ex_01};

		
		ItemMenuMake(pWork, tbl);
		
		for(i=0; i< BAG_MENUTBL_MAX ; i++){
			if(tbl[i]==255){
				continue;
			}
			OS_Printf("tblno %d %d\n",tbl[i],strtbl[tbl[i]] );
			
			GFL_MSG_GetString(  pWork->MsgManager, strtbl[tbl[i]], pWork->pStrBuf );
			BmpMenuWork_ListAddString( pWork->itemUseMenuList, pWork->pStrBuf, tbl[i], pWork->heapID );
		}
	}


	list_h.list = pWork->itemUseMenuList;
	list_h.work = pWork;
	list_h.win = pWork->itemUseWin;
	list_h.count = length;

	list_h.print_que = pWork->SysMsgQue;
	PRINT_UTIL_Setup( &pWork->SysMsgPrintUtil , pWork->itemUseWin );
	list_h.print_util = &pWork->SysMsgPrintUtil;
	list_h.font_handle = pWork->fontHandle;

	if(pWork->lwItemUse){
		BmpMenuList_Exit(pWork->lwItemUse,NULL,NULL);
	}
	pWork->lwItemUse = BmpMenuList_Set(&list_h, 0, 0, pWork->heapID);
	BmpMenuList_SetCursorBmp(pWork->lwItemUse, pWork->heapID);

}




//------------------------------------------------------------------------------
/**
 * @brief   アイテムをどう使うかのメニュー
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemUseMenu(FIELD_ITEMMENU_WORK* pWork)
{
	int ret;
	ret = BmpMenuList_Main(pWork->lwItemUse);
	switch(ret){
	case BMPMENULIST_NULL:
		return;
	case BMPMENULIST_CANCEL:
		BmpMenuList_Exit(pWork->lwItemUse, NULL, NULL);
//		GFL_BMPWIN_ClearScreen(pWork->itemUseWin);
//		BmpWinFrame_Clear(pWork->itemUseWin, WINDOW_TRANS_OFF);
		GFL_BG_LoadScreenV_Req(DEBUG_ITEMDISP_FRAME);
		GFL_BMPWIN_Delete(pWork->itemUseWin);
		BmpMenuWork_ListDelete(pWork->itemUseMenuList);

		pWork->lwItemUse=NULL;
		pWork->itemUseWin=NULL;
		pWork->itemUseMenuList=NULL;
	
		_CHANGE_STATE(pWork, _itemKindSelectMenu);
		break;
	default:
		break;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  FIELD_ITEMMENU_WORK* pWork = p_work;

  _pocketCursorChange(pWork, pWork->pocketno, bttnid);
  pWork->pocketno = bttnid;

  _windowRewrite(pWork);

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
  FIELD_ITEMMENU_WORK* pWork = work;
  
	GFL_CLACT_SYS_VBlankFunc();	//セルアクターVBlank
  if(pWork->bChange){
    ITEMDISP_CellVramTrans(pWork);
    pWork->bChange = FALSE;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   プロセス初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Init( GFL_PROC * proc, int * seq, void * ppWork, void * mypWork )
{
	FIELD_ITEMMENU_WORK* pWork = ppWork;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ITEMMENU, 0x28000 );
	pWork->heapID = HEAPID_ITEMMENU;
  pWork->pBagCursor = GAMEDATA_GetBagCursor(GAMESYSTEM_GetGameData(pWork->gsys));

  pWork->pocketno = MYITEM_FieldBagPocketGet(pWork->pBagCursor);
  {
    s16 cur,scr;
    MYITEM_FieldBagCursorGet(pWork->pBagCursor, pWork->pocketno, &cur, &scr );
    pWork->curpos = cur;
    pWork->oamlistpos = scr - 1;
  }
	_graphicInit(pWork);

	pWork->pMyItem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(pWork->gsys));
	pWork->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
																	 NARC_message_bag_dat, pWork->heapID );

	pWork->MsgManagerItemInfo = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
																							NARC_message_iteminfo_dat, pWork->heapID );

	pWork->pStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
	pWork->pExpStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
	pWork->WordSet    = WORDSET_Create( pWork->heapID );
	pWork->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr ,
																		GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );

	GFL_UI_KEY_SetRepeatSpeed(1, 6);
	ITEMDISP_upMessageCreate(pWork);
  ITEMDISP_CellResourceCreate(pWork);
	_windowCreate(pWork);
  ITEMDISP_CellCreate(pWork);

	pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );

	WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
									WIPE_FADE_BLACK , FLD_SUBSCR_FADE_DIV , FLD_SUBSCR_FADE_SYNC , pWork->heapID );

	pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  
	_CHANGE_STATE(pWork, _itemKindSelectMenu);
	return GFL_PROC_RES_FINISH;
}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスMain
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_Main( GFL_PROC * proc, int * seq, void * ppWork, void * mypWork )
{
	FIELD_ITEMMENU_WORK* pWork = ppWork;
	StateFunc* state = pWork->state;

	GFL_BMN_Main( pWork->pButton );
	if(state == NULL){
		return GFL_PROC_RES_FINISH;
	}
	if( WIPE_SYS_EndCheck() != TRUE ){
		return GFL_PROC_RES_CONTINUE;
	}
	state(pWork);
	_dispMain(pWork);
	
	return GFL_PROC_RES_CONTINUE;

}

//------------------------------------------------------------------------------
/**
 * @brief   プロセスEnd
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT FieldItemMenuProc_End( GFL_PROC * proc, int * seq, void * ppWork, void * mypWork )
{
	FIELD_ITEMMENU_WORK* pWork = ppWork;

  if(pWork->bgchar!=0){
    GFL_BG_FreeCharacterArea(DEBUG_ITEMDISP_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                             GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));
  }
	GFL_TCB_DeleteTask( pWork->g3dVintr );

  MYITEM_FieldBagPocketSet(pWork->pBagCursor, pWork->pocketno);
  MYITEM_FieldBagCursorSet(pWork->pBagCursor, pWork->pocketno, pWork->curpos, pWork->oamlistpos+1 );

  ITEMDISP_upMessageDelete(pWork);
  ITEMDISP_graphicDelete(pWork);
  
	GFL_MSG_Delete(pWork->MsgManager);
	GFL_MSG_Delete(pWork->MsgManagerItemInfo);
	GFL_STR_DeleteBuffer(pWork->pStrBuf);
	GFL_STR_DeleteBuffer(pWork->pExpStrBuf);
	WORDSET_Delete(pWork->WordSet);
//  if(pWork->submenulist){
//    BmpMenuWork_ListDelete(pWork->submenulist);
//  }
	GFL_FONT_Delete(pWork->fontHandle);
	PRINTSYS_QUE_Clear(pWork->SysMsgQue);
	PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  GFL_BMN_Delete(pWork->pButton);

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
