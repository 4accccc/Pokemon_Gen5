//============================================================================================
/**
 * @file    itemmenu.c
 * @brief   アイテムメニュー
 * @author  k.ohno
 * @author  genya hosaka (引継)
 * @date    2009.06.30
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
#include "gamesystem/game_beacon.h"
#include "system/bmp_winframe.h"

#include "message.naix"
#include "bag.naix"
#include "msg/msg_bag.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "font/font.naix" //NARC_font_large_gftr
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "ui/touchbar.h"

#include "itemmenu.h"
#include "itemmenu_local.h"

#include "app/itemuse.h"
#include "savedata/mystatus.h"
#include "savedata/encount_sv.h"

#include "system/main.h"      //GFL_HEAPID_APP参照
#include "system/scroll_bar.h"

#include "field/eventwork.h"  // GAMEDATA_GetEventWork
//#include "../../../resource/fldmapdata/flagwork/work_define.h"  // WK_SYS_N01R0502_ITEMUSE

//============================================================================================
//============================================================================================

#define _1CHAR (8)
#define BAG_HEAPSIZE ( 0x34000 ) // 0x28000

//=============================================================================
// 目次
//=============================================================================
//  __TECNIQUE  技マシン シーケンス
//  __TRASH     捨てる シーケンス
//  __SELL      売る シーケンス


//=============================================================================
// データテーブル
//=============================================================================
//ならびは savedata/myitem_savedata.hに準拠
// コールバックは_BttnCallBack
static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右

#include "bag_btnpos.h"  //バッグの絵のタッチエリア

  { 21*_1CHAR,  24*_1CHAR-1,   0*_1CHAR,  3*_1CHAR-1 },  //左
  { 21*_1CHAR,  24*_1CHAR-1,  15*_1CHAR, 18*_1CHAR-1 },  //右

  { 21*_1CHAR,  24*_1CHAR-1,  19*_1CHAR, 22*_1CHAR-1 },  //ソート
  { 21*_1CHAR,  24*_1CHAR-1,  22*_1CHAR, 24*_1CHAR-1 },  //チェックボタン

  { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, _BAR_CELL_CURSOR_EXIT, _BAR_CELL_CURSOR_EXIT+TOUCHBAR_ICON_WIDTH-1 },  //x
  { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, _BAR_CELL_CURSOR_RETURN, _BAR_CELL_CURSOR_RETURN+TOUCHBAR_ICON_WIDTH-1 },  //リターン

  { 1*_1CHAR+4,   4*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア1
  { 4*_1CHAR+4,   7*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア2
  { 7*_1CHAR+4,  11*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア3
  {11*_1CHAR+4,  14*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア4
  {14*_1CHAR+4,  17*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア5
  {17*_1CHAR+4,  20*_1CHAR+3,  18*_1CHAR, 28*_1CHAR-1 },  //アイテム一覧エリア6

  { 1*_1CHAR+4,   4*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア1
  { 4*_1CHAR+4,   7*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア2
  { 7*_1CHAR+4,  11*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア3
  {11*_1CHAR+4,  14*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア4
  {14*_1CHAR+4,  17*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア5
  {17*_1CHAR+4,  20*_1CHAR+3,  15*_1CHAR, 18*_1CHAR-1 },  //だいじなものチェックエリア6

  {GFL_UI_TP_HIT_END,0,0,0},     //終了データ
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(FIELD_ITEMMENU_WORK* pWork,StateFunc state);
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc state, int line);
static void _windowCreate(FIELD_ITEMMENU_WORK* pWork);
static void _windowRewrite(FIELD_ITEMMENU_WORK* pWork);
ITEM_ST* ITEMMENU_GetItem(FIELD_ITEMMENU_WORK* pWork, int no);
static void _ItemChangeSingle(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 );
static void _ItemChange(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 );
static void _pocketCursorChange(FIELD_ITEMMENU_WORK* pWork,int oldpocket, int newpocket);
int ITEMMENU_GetItemIndex(FIELD_ITEMMENU_WORK* pWork);
static BOOL _posplus( FIELD_ITEMMENU_WORK * pWork, int length, BOOL loop );
static BOOL _posminus( FIELD_ITEMMENU_WORK * pWork, int length, BOOL loop );
static BOOL _itemScrollCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _keyChangeItemCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _keyMoveCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _itemMovePositionTouchItem(FIELD_ITEMMENU_WORK* pWork);
static void _itemMovePosition(FIELD_ITEMMENU_WORK* pWork);
static void _itemInnerUse( FIELD_ITEMMENU_WORK* pWork );
static void _itemInnerUseWait( FIELD_ITEMMENU_WORK* pWork );
static void _itemInnerUseError( FIELD_ITEMMENU_WORK* pWork );
static void _itemInnerUseRightStone( FIELD_ITEMMENU_WORK* pWork );
//static void _itemInnerUseRightStoneWait( FIELD_ITEMMENU_WORK* pWork );
static void _itemSelectWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemSelectState(FIELD_ITEMMENU_WORK* pWork);
static void _itemKindSelectMenu(FIELD_ITEMMENU_WORK* pWork);
static void _itemTecniqueUseYesNo(FIELD_ITEMMENU_WORK* pWork);
static void _itemTecniqueUseWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemTecniqueUseInit(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrashEndWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrashYesWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrashYesNoWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrashYesNo(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrashWait(FIELD_ITEMMENU_WORK* pWork);
static void _itemTrash(FIELD_ITEMMENU_WORK* pWork);
static void _itemSellInit( FIELD_ITEMMENU_WORK* pWork );
static void _itemSellInputWait( FIELD_ITEMMENU_WORK* pWork );
static void _itemSellYesnoInit( FIELD_ITEMMENU_WORK* pWork );
static void _itemSellYesnoInput( FIELD_ITEMMENU_WORK* pWork );
static void _itemSellEndMsgWait( FIELD_ITEMMENU_WORK* pWork );
static void _itemSellExit( FIELD_ITEMMENU_WORK* pWork );
static void ITEM_Sub( FIELD_ITEMMENU_WORK* pWork, int sub_num );
static void InputNum_Start( FIELD_ITEMMENU_WORK* pWork, BAG_INPUT_MODE mode );
static void InputNum_Exit( FIELD_ITEMMENU_WORK* pWork );
static BOOL InputNum_Main( FIELD_ITEMMENU_WORK* pWork );
//static void InputNum_ButtonState( FIELD_ITEMMENU_WORK* pWork, BOOL on_off );
static void SORT_Type( FIELD_ITEMMENU_WORK* pWork );
//static void SORT_ABC( FIELD_ITEMMENU_WORK* pWork );
static u16 SORT_GetABCPrio( u16 item_no );
static void SORT_Button( FIELD_ITEMMENU_WORK* pWork );
static void SORT_ModeReset( FIELD_ITEMMENU_WORK* pWork );
//static void SORT_Draw( FIELD_ITEMMENU_WORK* pWork );
static void KTST_SetDraw( FIELD_ITEMMENU_WORK* pWork, BOOL on_off );
static void SHORTCUT_SetEventItem( FIELD_ITEMMENU_WORK* pWork, int pos );
static void SHORTCUT_SetPocket( FIELD_ITEMMENU_WORK* pWork );
static void BTN_DrawCheckBox( FIELD_ITEMMENU_WORK* pWork );
//static void BAG_ItemUseErrorMsgSet( MYSTATUS * myst, STRBUF * buf, u16 item, u32 err, FIELD_ITEMMENU_WORK * pWork );
//u8 Bag_TalkMsgPrint( BAG_WORK * pWork, int type );
//static int Bag_MenuUse( FIELD_ITEMMENU_WORK * pWork );
static void ItemMenuMake( FIELD_ITEMMENU_WORK * pWork, u8* tbl );
static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork);
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work );
static void _VBlank( GFL_TCB *tcb, void *work );
static void _startState(FIELD_ITEMMENU_WORK* pWork);

#ifdef PM_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG

static void ChangeStateItemKindSelectItemMenu( FIELD_ITEMMENU_WORK * wk );
static void ChangeStateItemMovePosition( FIELD_ITEMMENU_WORK * wk );
static void InitBlinkPalAnm( FIELD_ITEMMENU_WORK * wk );
static void ExitBlinkPalAnm( FIELD_ITEMMENU_WORK * wk );
static void _listSelectAnime( FIELD_ITEMMENU_WORK * wk );
static void SetEndButtonAnime( FIELD_ITEMMENU_WORK * wk, u16 type, StateFunc * next );
static void _endButtonAnime( FIELD_ITEMMENU_WORK * wk );
static void SetPageButtonAnime( FIELD_ITEMMENU_WORK * wk, u32 old, u16 type, StateFunc * next );
static void _waitCount( FIELD_ITEMMENU_WORK * wk );
static int CheckNumSelTouch(void);
static void _itemTrashCancel( FIELD_ITEMMENU_WORK * wk );
static void _itemSellInputCancel( FIELD_ITEMMENU_WORK * wk );
static void InitPaletteAnime( FIELD_ITEMMENU_WORK * wk );
static void ExitPaletteAnime( FIELD_ITEMMENU_WORK * wk );
static void _sortMessageSet( FIELD_ITEMMENU_WORK * wk );
static void _sortMessageWait( FIELD_ITEMMENU_WORK * wk );
static BOOL CheckEventItemUse( FIELD_ITEMMENU_WORK * wk, u8 pocket );
static void _itemPocketChange( FIELD_ITEMMENU_WORK * wk );


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
//  GFL_UI_KEY_SetRepeatSpeed(8, 15);  //元に戻す
  GFL_UI_KEY_SetRepeatSpeed( pWork->key_repeat_speed, pWork->key_repeat_wait );
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef PM_DEBUG
static void _changeStateDebug(FIELD_ITEMMENU_WORK* pWork,StateFunc state, int line)
{
  OS_TPrintf("BAG STATE: %d\n",line);
  _changeState(pWork, state);
}
#endif


#define FBMP_COL_NULL   (0)
#define FBMP_COL_BLACK    (1)
#define FBMP_COL_BLK_SDW  (2)
#define FBMP_COL_RED    (3)
#define FBMP_COL_RED_SDW  (4)
#define FBMP_COL_GREEN    (5)
#define FBMP_COL_GRN_SDW  (6)
#define FBMP_COL_BLUE   (7)
#define FBMP_COL_BLU_SDW  (8)
#define FBMP_COL_PINK   (9)
#define FBMP_COL_PNK_SDW  (10)
#define FBMP_COL_WHITE    (15)
#define _BMPMENULIST_FONTSIZE   (16)



/*
///選択メニューのリスト
static BMPMENULIST_HEADER _itemMenuListHeader = {
  NULL,     // 表示文字データポインタ
  NULL,         // カーソル移動ごとのコールバック関数
  NULL,         // 一列表示ごとのコールバック関数
  NULL,                      // GF_BGL_BMPWIN* win
  9,// リスト項目数
  9,// 表示最大項目数
  0,            // ラベル表示Ｘ座標
  16,           // 項目表示Ｘ座標
  0,            // カーソル表示Ｘ座標
  0,            // 表示Ｙ座標
  FBMP_COL_BLACK,     // 文字色
  FBMP_COL_WHITE,     // 背景色
  FBMP_COL_BLK_SDW,   // 文字影色
  0,            // 文字間隔Ｘ
  18,           // 文字間隔Ｙ
  BMPMENULIST_NO_SKIP,    // ページスキップタイプ
  0,//FONT_SYSTEM,        // 文字指定
  0,            // ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
  NULL,                   // work
  _BMPMENULIST_FONTSIZE,      //文字サイズX(ドット
  _BMPMENULIST_FONTSIZE,      //文字サイズY(ドット
  NULL,   //表示に使用するメッセージバッフ
  NULL,   //表示に使用するプリントユーティ
  NULL,   //表示に使用するプリントキュー
  NULL,   //表示に使用するフォントハンドル
};
*/

//-----------------------------------------------------------------------------
/**
 *  @brief  ウィンドウ描画初期化
 *
 *  @param  pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _windowCreate(FIELD_ITEMMENU_WORK* pWork)
{
  pWork->oamlistpos_old = 0xffff;
  _windowRewrite(pWork);
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ウィンドウ再描画
 *
 *  @param  pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _windowRewrite(FIELD_ITEMMENU_WORK* pWork)
{
  // キー操作時のみ、上画面を更新
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY )
  {
    ITEMDISP_upMessageRewrite(pWork);
  }
  else
  {
    ITEMDISP_upMessageClean(pWork);
  }

  ITEMDISP_CellMessagePrint(pWork);
  BTN_DrawCheckBox( pWork );
//  pWork->bChange = TRUE;
  pWork->bCursorChange = 1;
//  pWork->bCellChange = 1;
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケットにあるアイテムを返す
 * @retval  none
 */
//------------------------------------------------------------------------------

ITEM_ST* ITEMMENU_GetItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  ITEM_ST * item;

  if(pWork->moveMode){
    item = &pWork->ScrollItem[no];
  }
  else{
    item = MYITEM_PosItemGet( pWork->pMyItem, pWork->pocketno, no );
  }
  return item;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アイテムの売値を取得
 *
 *  @param  int item_no 売るアイテムNO
 *  @param  input_num   売る個数
 *  @param  heapID      ヒープID
 *
 *  @retval s32 売値
 */
//-----------------------------------------------------------------------------
s32 ITEMMENU_SellPrice( int item_no, int input_num, HEAPID heapID )
{
  s32 val;

  val = ITEM_GetParam( item_no, ITEM_PRM_PRICE, heapID );

  // 売り値は半分
  val /= 2;

  // 個数
  val *= input_num;

  return val;
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
 * @brief   ポケットにあるアイテムの総数を返す
 * @retval  none
 */
//------------------------------------------------------------------------------

int ITEMMENU_GetItemPocketNumber(FIELD_ITEMMENU_WORK* pWork)
{
  int length = 0;

  if(pWork->moveMode){
    length = MYITEM_GetItemThisPocketNumber(pWork->ScrollItem, BAG_MYITEM_MAX);

  }
  else{
    length = MYITEM_GetItemPocketNumber( pWork->pMyItem, pWork->pocketno);
  }
  return length;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム名をレジスト
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *  @param  bufID
 *  @param  itemID 
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
void ITEMMENU_WordsetItemName( FIELD_ITEMMENU_WORK* pWork, u32 bufID, u32 itemID )
{
//    GFL_MSG_GetString( pWork->MsgManagerItemName, itemID, pWork->pItemNameBuf );
//    WORDSET_RegisterWord( pWork->WordSet, bufID, pWork->pItemNameBuf, 0, TRUE, PM_LANG );
  WORDSET_RegisterItemName( pWork->WordSet, bufID, itemID );
}


//-----------------------------------------------------------------------------
/**
 *  @brief  アイテムデータ交換
 *
 *  @param  pWork
 *  @param  no1
 *  @param  no2
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _ItemChangeSingle(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 )
{
  ITEM_ST temp;
  GFL_STD_MemCopy(&pWork->ScrollItem[no1], &temp, sizeof(ITEM_ST));
  GFL_STD_MemCopy(&pWork->ScrollItem[no2],&pWork->ScrollItem[no1], sizeof(ITEM_ST));
  GFL_STD_MemCopy(&temp, &pWork->ScrollItem[no2], sizeof(ITEM_ST));
}


//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム交換処理
 *
 *  @param  pWork
 *  @param  no1
 *  @param  no2
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _ItemChange(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 )
{
  int i;

  if(no1 == no2){
    return;
  }
  else if(no1 < no2){
    for(i = no1; i < no2 ; i++ ){
      _ItemChangeSingle(pWork, i, i+1);
    }
  }
  else{
    for(i = no1; i > no2 ; i-- ){
      _ItemChangeSingle(pWork, i, i-1);
    }
  }
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
  ITEMDISP_ScrollCursorChangePos( pWork );
  ITEMDISP_PocketMessage(pWork, newpocket);
  ITEMDISP_ChangePocketCell( pWork,newpocket );
  ITEMDISP_ItemInfoWindowChange(pWork,newpocket);
}
//-----------------------------------------------------------------------------
/**
 *  @brief  下キーが押された時の処理
 *
 *  @param  pWork
 *  @param  length
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static BOOL _posplus( FIELD_ITEMMENU_WORK * pWork, int length, BOOL loop )
{
  int tmp_cursor, tmp_list;

  // アイテムなし
  if( length == 0 ){ return FALSE; }

  tmp_cursor = pWork->curpos;
  tmp_list   = pWork->oamlistpos;

/*
  if((pWork->curpos==4) && ((pWork->oamlistpos+7) < length)){
    //カーソルはそのままでリストが移動
    pWork->oamlistpos++;
  }
  else if((pWork->curpos==4) && ((pWork->curpos+1) < length)){
    //リストの終端まで来たのでカーソルが移動
    pWork->curpos++;
  }
  else if((pWork->curpos!=5) && ((pWork->curpos+1) < length)){
    //リストの終端まで来たのでカーソルが移動
    pWork->curpos++;
  }else{
    // ループ
    if( loop == TRUE ){
      pWork->oamlistpos = -1;
      pWork->curpos = 0;
    }
  }
*/
  // カーソルはそのままでリストが移動
  if( pWork->curpos == 5 && ( pWork->oamlistpos + 7 ) < length ){
    pWork->oamlistpos++;
  // リストの終端まで来たのでカーソルが移動
  }else if( pWork->curpos != 5 && ( pWork->curpos + 1 ) < length ){
    pWork->curpos++;
  }else{
    // ループ
    if( loop == TRUE ){
      pWork->oamlistpos = -1;
      pWork->curpos = 0;
    }
  }

  if( tmp_cursor != pWork->curpos || tmp_list != pWork->oamlistpos ){
    return TRUE;
  }
  return FALSE;
}


//-----------------------------------------------------------------------------
/**
 *  @brief  上キーが押された時の処理
 *
 *  @param  pWork
 *  @param  length
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static BOOL _posminus( FIELD_ITEMMENU_WORK * pWork, int length, BOOL loop )
{
  int tmp_cursor, tmp_list;

  // アイテムなし
  if( length == 0 ){ return FALSE; }

  tmp_cursor = pWork->curpos;
  tmp_list   = pWork->oamlistpos;

/*
  if((pWork->curpos==1) && (pWork->oamlistpos!=-1)){
    //カーソルはそのままでリストが移動
    pWork->oamlistpos--;
  }
  else if((pWork->curpos==1)){
    //リストの終端まで来たのでカーソルが移動
    pWork->curpos--;
  }
  else if(pWork->curpos != 0){
    pWork->curpos--;
  }else{
    // ループ
    if( loop == TRUE ){
      pWork->oamlistpos = length - 7;
      if( pWork->oamlistpos < -1 ){
        pWork->oamlistpos = -1;
      }
      pWork->curpos = 5;
      if( pWork->curpos >= length ){
        pWork->curpos = length-1;
      }
    }
  }
*/
  // カーソルはそのままでリストが移動
  if( pWork->curpos == 0 && pWork->oamlistpos != -1 ){
    pWork->oamlistpos--;
  // リストの終端まで来たのでカーソルが移動
  }else if( pWork->curpos != 0 ){
    pWork->curpos--;
  }else{
    // ループ
    if( loop == TRUE ){
      pWork->oamlistpos = length - 7;
      if( pWork->oamlistpos < -1 ){
        pWork->oamlistpos = -1;
      }
      pWork->curpos = 5;
      if( pWork->curpos >= length ){
        pWork->curpos = length-1;
      }
    }
  }

  if( tmp_cursor != pWork->curpos || tmp_list != pWork->oamlistpos ){
    return TRUE;
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   スクロールの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _itemScrollCheck(FIELD_ITEMMENU_WORK* pWork)
{
  u32 x, y;
  int length = ITEMMENU_GetItemPocketNumber(pWork);

  if( length >= ITEMMENU_SCROLLBAR_ENABLE_NUM && GFL_UI_TP_GetPointCont(&x, &y) == TRUE ){
    // 範囲判定
    if( (x >= (32*8)) || (x < (28*8)) ){
      return FALSE;
    }
    // 初回チェック
    if( pWork->scrollMode == FALSE ){
      if( GFL_UI_TP_GetTrg() == TRUE ){
//        if( y < _SCROLL_TOP_Y || y > _SCROLL_BOTTOM_Y ){
        if( y < 16 || y >= 176 ){
          return FALSE;
        }
        pWork->scrollMode = TRUE;
      }else{
        return FALSE;
      }
    }

    // Ｙ座標補正
    if( y < _SCROLL_TOP_Y ){
      y = _SCROLL_TOP_Y;
    }else if( y > _SCROLL_BOTTOM_Y ){
      y = _SCROLL_BOTTOM_Y;
    }

    // タッチ状態
    KTST_SetDraw( pWork, FALSE );

    {
      int tmp_scr = pWork->oamlistpos;

      pWork->oamlistpos = SCROLLBAR_GetCount( length+1-6, y, _SCROLL_TOP_Y, _SCROLL_BOTTOM_Y, 0 ) - 1;

      ITEMDISP_ScrollCursorMove( pWork );   // スクロールバーOAM座標を変更

      if( pWork->oamlistpos != tmp_scr ){
        PMSND_PlaySE( SE_BAG_SLIDE );
        return TRUE;
      }
    }
  }

  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   キーの動きの処理(アイテム移動モード)
 * @retval  none
 */
//------------------------------------------------------------------------------

static BOOL _keyChangeItemCheck(FIELD_ITEMMENU_WORK* pWork)
{
  BOOL bChange = FALSE;
  int nowno = ITEMMENU_GetItemIndex(pWork);
  {
    int pos = pWork->curpos;
    int length = ITEMMENU_GetItemPocketNumber( pWork);

    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
      bChange = _posplus( pWork, length, TRUE );
    }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
      bChange = _posminus( pWork, length, TRUE );
    }else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R ){
      u32 i;
      for( i=0; i<6; i++ ){
        if( _posplus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
      if( i != 0 ){ bChange = TRUE; }
    }else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L ){
      u32 i;
      for( i=0; i<6; i++ ){
        if( _posminus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
      if( i != 0 ){ bChange = TRUE; }
    }
  }
  if(bChange){
    int newno = ITEMMENU_GetItemIndex(pWork);
    PMSND_PlaySE( SE_BAG_CURSOR_MOVE );
    _ItemChange(pWork, nowno, newno);
  }
  return bChange;
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
    int length = ITEMMENU_GetItemPocketNumber( pWork);

    if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
      bChange = _posplus( pWork, length, TRUE );
    }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
      bChange = _posminus( pWork, length, TRUE );
    }else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_R ){
      u32 i;
      for( i=0; i<6; i++ ){
        if( _posplus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
      if( i != 0 ){ bChange = TRUE; }
    }else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_L ){
      u32 i;
      for( i=0; i<6; i++ ){
        if( _posminus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
      if( i != 0 ){ bChange = TRUE; }
    }
  }
  return bChange;
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテム移動モード時のアイテム部分のタッチ処理
 * @retval  none
 */
//------------------------------------------------------------------------------
#define ITEM_MOVE_LX    ( 18*8 )      // アイテム移動タッチ有効範囲：左
#define ITEM_MOVE_RX    ( 29*8-1 )    // アイテム移動タッチ有効範囲：右
#define ITEM_MOVE_UY    ( 12 )        // アイテム移動タッチ有効範囲：上
#define ITEM_MOVE_DY    ( 156-1 )     // アイテム移動タッチ有効範囲：下
#define ITEM_MOVE_SY    ( 24 )        // リスト１項目サイズ

static BOOL _itemMovePositionTouchItem(FIELD_ITEMMENU_WORK* pWork)
{
  u32 x, y;

  if( GFL_UI_TP_GetPointCont( &x, &y ) == TRUE ){
    int length;
    int now_pos, new_pos;
    u32 tp;
    u32 cnt;
    u32 i;

    // 範囲チェック
    if( !( x >= ITEM_MOVE_LX && x <= ITEM_MOVE_RX && y >= ITEM_MOVE_UY && y <= ITEM_MOVE_DY ) ){
      return FALSE;
    }

    tp = ( y - ITEM_MOVE_UY ) / ITEM_MOVE_SY;         // タッチ位置

    // 現在のカーソル位置
    if( pWork->curpos == tp ){
      return FALSE;
    }

    length  = ITEMMENU_GetItemPocketNumber( pWork );  // アイテム数
    now_pos = ITEMMENU_GetItemIndex( pWork );         // 現在のカーソル位置のアイテム
    cnt     = GFL_STD_Abs( pWork->curpos - tp );      // カーソル移動カウンタ

    // 下へ
    if( pWork->curpos < tp ){
      for( i=0; i<cnt; i++ ){
        if( _posplus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
    // 上へ
    }else if( pWork->curpos > tp ){
      for( i=0; i<cnt; i++ ){
        if( _posminus( pWork, length, FALSE ) == FALSE ){
          break;
        }
      }
    }
    // 移動できなかった
    if( i == 0 ){ return FALSE; }

    new_pos = ITEMMENU_GetItemIndex( pWork );     // 移動後のカーソル位置のアイテム
    _ItemChange( pWork, now_pos, new_pos );       // アイテム入れ替え

    KTST_SetDraw( pWork, FALSE );

    return TRUE;
  }

  return FALSE;
}

// アイテム移動のキャンセルチェック
static BOOL _itemMoveCancelTouchCheck(void)
{
  u32 x, y;

  if( GFL_UI_TP_GetPointTrg( &x, &y ) == TRUE ){
    if( x >= _BAR_CELL_CURSOR_RETURN && x <= (_BAR_CELL_CURSOR_RETURN+TOUCHBAR_ICON_WIDTH-1) &&
        y >= TOUCHBAR_ICON_Y && y <= (TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1) ){
      return TRUE;
    }
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテム移動モード時のキーの処理
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _itemMoveCancel(FIELD_ITEMMENU_WORK* pWork)
{
  ITEMDISP_ChangeMoveModeButton( pWork, TRUE );
//  _CHANGE_STATE( pWork, _itemKindSelectMenu );
  ChangeStateItemKindSelectItemMenu( pWork );
}

static void _itemMovePosition(FIELD_ITEMMENU_WORK* pWork)
{
  int now_pos, new_pos;

  BLINKPALANM_Main( pWork->blwk );

//  GFL_UI_KEY_SetRepeatSpeed(1, 6);

  if( PRINTSYS_QUE_IsFinished( pWork->SysMsgQue ) == FALSE ){
    return;
  }

  // スクロール中フラグクリア
  if( GFL_UI_TP_GetCont() == FALSE ){
    pWork->scrollMode = FALSE;
    if( pWork->moveDrag == TRUE ){
      PMSND_PlaySE( SE_BAG_DECIDE );
      pWork->moveDrag = FALSE;
      MYITEM_ITEM_STCopy(pWork->pMyItem, pWork->ScrollItem, pWork->pocketno, FALSE);
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur, 1 );
      ITEMDISP_ChangeMoveModeButton( pWork, TRUE );
      pWork->moveMode = FALSE;
      KTST_SetDraw( pWork, FALSE );
//      _CHANGE_STATE( pWork, _itemKindSelectMenu );
      ChangeStateItemKindSelectItemMenu( pWork );
      return;
    }
  }

  // 決定
  if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_DECIDE|PAD_BUTTON_SELECT) ){
    PMSND_PlaySE( SE_BAG_DECIDE );
    MYITEM_ITEM_STCopy(pWork->pMyItem, pWork->ScrollItem, pWork->pocketno, FALSE);
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur, 1 );
    ITEMDISP_ChangeMoveModeButton( pWork, TRUE );
    pWork->moveMode = FALSE;
    KTST_SetDraw( pWork, TRUE );
//    _CHANGE_STATE( pWork, _itemKindSelectMenu );
    ChangeStateItemKindSelectItemMenu( pWork );
    return;
  }
  // キャンセル（キー）
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
    PMSND_PlaySE( SE_BAG_CANCEL );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur, 1 );
    pWork->moveMode = FALSE;
    KTST_SetDraw( pWork, TRUE );
//    _CHANGE_STATE( pWork, _itemMoveCancel );
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite( pWork );
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, _itemMoveCancel );
    return;
  }
  // キャンセル（タッチ）
  if( _itemMoveCancelTouchCheck() == TRUE ){
    PMSND_PlaySE( SE_BAG_CANCEL );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur, 1 );
    pWork->moveMode = FALSE;
    KTST_SetDraw( pWork, FALSE );
//    _CHANGE_STATE( pWork, _itemMoveCancel );
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, _itemMoveCancel );
    return;
  }

  // スクロールバーの操作
  now_pos = ITEMMENU_GetItemIndex( pWork );
  if( _itemScrollCheck(pWork) ){
    new_pos = ITEMMENU_GetItemIndex( pWork );
    _ItemChange( pWork, now_pos, new_pos );
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite( pWork );
  // リストタッチ操作
  }else if( _itemMovePositionTouchItem(pWork) ){
    pWork->moveDrag = TRUE;
    ITEMDISP_ScrollCursorChangePos( pWork );
    PMSND_PlaySE( SE_BAG_SLIDE );
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
  // キー操作
  }else if( _keyChangeItemCheck(pWork) ){ 
    ITEMDISP_ScrollCursorChangePos( pWork );
    PMSND_PlaySE( SE_BAG_SLIDE );
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  スプレー使用チェック
 *
 *  @param  u16 item_id
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL _check_spray( u16 item_id )
{
  return ( item_id == ITEM_GOORUDOSUPUREE ||
      item_id == ITEM_SIRUBAASUPUREE ||
      item_id == ITEM_MUSIYOKESUPUREE  );
}

enum{
  ITEM_USE_NONE=0,          // 通常使用
  INNER_USE_BAG,            // 使うとバッグの中でメッセージがでてバッグ選択に戻る
  INNER_USE_AFTER_OUTBAG,   // 使うとバッグの中でメッセージがでてバッグを終了させる
};

//-----------------------------------------------------------------------------
/**
 *  @brief  バッグ内で使うアイテムか判定
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval TURE:バッグ内で使うアイテム
 */
//-----------------------------------------------------------------------------
static int BAG_IsInnerItem( u16 item_id )
{
  // スプレー
  if(  _check_spray( item_id ) )
  {
    return INNER_USE_BAG;
  } // ダークストーン・ライトストーン処理
  else if( item_id==ITEM_DAAKUSUTOON || item_id==ITEM_RAITOSUTOON){
    return INNER_USE_AFTER_OUTBAG;
  }
  return FALSE;
}



//-----------------------------------------------------------------------------
/**
 *  @brief  バッグ内で使うアイテム処理
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemInnerUse( FIELD_ITEMMENU_WORK* pWork )
{
  // スプレー
  if( _check_spray( pWork->ret_item ) )
  {
    SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( pWork->gamedata );
    ENC_SV_PTR encsv = EncDataSave_GetSaveDataPtr( save );

    // 使用チェック
    if( EncDataSave_CanUseSpray( encsv ) )
    {
      u8 count = ITEM_GetParam( pWork->ret_item, ITEM_PRM_ATTACK, pWork->heapID );

      // スプレー処理
      EncDataSave_SetSprayCnt( encsv, count );

      // アイテムを減らす
      ITEM_Sub( pWork, 1 );
//			pWork->bCursorChange = 1;

      // つかった
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_066, pWork->pStrBuf );
      WORDSET_RegisterPlayerName(pWork->WordSet, 0,  pWork->mystatus );
      ITEMMENU_WordsetItemName( pWork, 1,  pWork->ret_item );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );

      PMSND_PlaySE( SE_BAG_SPRAY );
    }
    else
    {
      // つかえなかった
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_067, pWork->pStrBuf );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
    }

    _CHANGE_STATE(pWork,_itemInnerUseWait);
  }
  else
  {
    GF_ASSERT(0);
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム使用メッセージウェイト
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemInnerUseWait( FIELD_ITEMMENU_WORK* pWork )
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 入力待ち
  if( ( GFL_UI_KEY_GetTrg() & MSG_SKIP_BTN ) || GFL_UI_TP_GetTrg() )
  {
    // 再描画
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
		pWork->bCursorChange = 1;
		ITEMDISP_ScrollCursorChangePos( pWork );

    GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
    ITEMDISP_ChangeActive( pWork, TRUE );
//    _CHANGE_STATE(pWork,_itemKindSelectMenu);
    ChangeStateItemKindSelectItemMenu( pWork );
  }
}


/*
// Nの前で「ほらライトストーンを使ってみせてよ」
#define N01R0502_START     ( 1 )
// ライトストーンを使った
#define N01R0502_CHANGE    ( 2 )
*/
//-----------------------------------------------------------------------------
/**
 *  @brief  バッグ内で使うアイテム処理
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemInnerUseRightStone( FIELD_ITEMMENU_WORK* pWork )
{
  if( pWork->ret_item ==ITEM_DAAKUSUTOON || pWork->ret_item==ITEM_RAITOSUTOON )
  {
/*  シナリオで自動発動するので、確実に使えなくてよい。 2010/05/13 by nakahiro
    SAVE_CONTROL_WORK* save = GAMEDATA_GetSaveControlWork( pWork->gamedata );
    ENC_SV_PTR encsv = EncDataSave_GetSaveDataPtr( save );
    EVENTWORK *evwork = GAMEDATA_GetEventWork(pWork->gamedata);

    // 使用チェック(Nの前でライトストーンを使うシチュエーションか？)
    if( *EVENTWORK_GetEventWorkAdrs(evwork,WK_SYS_N01R0502_ITEMUSE)==N01R0502_START )
    {
      // 使用メッセージ「なにも反応もしめさない」
      if(pWork->ret_item==ITEM_DAAKUSUTOON){
        GFL_MSG_GetString( pWork->MsgManager, msg_bag_n_001w, pWork->pStrBuf );
      }else{
        GFL_MSG_GetString( pWork->MsgManager, msg_bag_n_001b, pWork->pStrBuf );
      }
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );

      PMSND_PlaySE( SE_BAG_RAITOSUTOON );
      _CHANGE_STATE(pWork,_itemInnerUseRightStoneWait);
      
    }
    else
    {
*/
      // つかえなかったので「アララギの　ことば…　つかいどきがある」
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_057, pWork->pStrBuf );
      WORDSET_RegisterPlayerName(pWork->WordSet, 0,  pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
      _CHANGE_STATE(pWork,_itemInnerUseWait);
//    }
  }
  else
  {
    GF_ASSERT(0);
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ライトストーン・ブラックストーン専用メッセージウェイト（と後処理）
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static void _itemInnerUseRightStoneWait( FIELD_ITEMMENU_WORK* pWork )
{
  EVENTWORK *evwork = GAMEDATA_GetEventWork(pWork->gamedata);
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 入力待ち
  if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ) || GFL_UI_TP_GetTrg() )
  {
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);

    // 再描画
    _windowRewrite(pWork);

    pWork->ret_code = BAG_NEXTPROC_EXIT;
    _CHANGE_STATE(pWork,NULL);

    // イベントワークの書き換え
    *EVENTWORK_GetEventWorkAdrs(evwork,WK_SYS_N01R0502_ITEMUSE) = N01R0502_CHANGE;
    
  }
}
*/


//----------------------------------------------------------------------------------
/**
 * @brief 道具が使えなかった際のメッセージ表示（既にExpandStrに準備されているとして）
 *
 * @param   pWork   
 */
//----------------------------------------------------------------------------------
static void _itemInnerUseError( FIELD_ITEMMENU_WORK* pWork )
{
  
  ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
  _CHANGE_STATE(pWork,_itemInnerUseWait);

}

static int _check_Cycle( FIELD_ITEMMENU_WORK *pWork );
static int _check_MailView( FIELD_ITEMMENU_WORK *pWork );
static int _check_BattleRecorder( FIELD_ITEMMENU_WORK *pWork );
static int _check_Turizao( FIELD_ITEMMENU_WORK *pWork );
static int _check_AmaiMitu( FIELD_ITEMMENU_WORK *pWork );
static int _check_TomodatiTechou( FIELD_ITEMMENU_WORK *pWork );
static int _check_TownMap( FIELD_ITEMMENU_WORK *pWork );
static int _check_Cycle( FIELD_ITEMMENU_WORK *pWork );
static int _check_DowsingMachine( FIELD_ITEMMENU_WORK *pWork );
static int _check_AnanukenoHimo( FIELD_ITEMMENU_WORK *pWork );

// アララギ博士のどうぐ使えないよメッセージ
#define MSGID_ITEMUSE_ERROR_ARARAGI   ( msg_bag_059 )
#define MSGID_ITEMUSE_CYCLE_NOSTOP    ( msg_bag_058 )

//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】自転車
 *
 * @param   pWork   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _check_Cycle( FIELD_ITEMMENU_WORK *pWork )
{

  if(BAG_MENU_TSUKAU==pWork->ret_code2){
    if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_CYCLE)){
      pWork->ret_code = BAG_NEXTPROC_RIDECYCLE;  //のる
      _CHANGE_STATE(pWork,NULL);
      return TRUE;
    }else{
      // のれなかった
      GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
      WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
      _CHANGE_STATE(pWork,_itemInnerUseError);
      return FALSE;
    }
  }else if(BAG_MENU_ORIRU==pWork->ret_code2){
    if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_CYCLE)){
      pWork->ret_code = BAG_NEXTPROC_DROPCYCLE;  //おりる
      _CHANGE_STATE(pWork,NULL);
      return TRUE;
    }else{
      // おりれなかった
      GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_CYCLE_NOSTOP, pWork->pStrBuf );
      WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
      _CHANGE_STATE(pWork,_itemInnerUseError);
      return FALSE;
    }
  }
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】タウンマップ
 */
//----------------------------------------------------------------------------------
static int _check_TownMap( FIELD_ITEMMENU_WORK *pWork )
{
  if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_TOWNMAP)){
    pWork->ret_code = BAG_NEXTPROC_TOWNMAP;  //タウンマップ
    _CHANGE_STATE(pWork,NULL);
    return TRUE;
  }else{
    // 使えない
    GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
    WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
    _CHANGE_STATE(pWork,_itemInnerUseError);
    return FALSE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】ともだち手帳
 */
//----------------------------------------------------------------------------------
static int _check_TomodatiTechou( FIELD_ITEMMENU_WORK *pWork )
{
  if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_WIFINOTE)){
    pWork->ret_code = BAG_NEXTPROC_FRIENDNOTE;  //タウンマップ
    _CHANGE_STATE(pWork,NULL);
    return TRUE;
  }else{
    // 使えない
    GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
    WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
    _CHANGE_STATE(pWork,_itemInnerUseError);
    return FALSE;
  }
  return FALSE;
  
}
//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】バトルレコーダー
 */
//----------------------------------------------------------------------------------
static int _check_BattleRecorder( FIELD_ITEMMENU_WORK *pWork )
{
  
  if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_BATTLE_RECORDER)){
    pWork->ret_code = BAG_NEXTPROC_BATTLERECORDER;  //バトルレコーダー
    _CHANGE_STATE(pWork,NULL);
    return TRUE;
  }else{
    // 使えない
    GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
    WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
    _CHANGE_STATE(pWork,_itemInnerUseError);
    return FALSE;
  
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】あまいミツ
 */
//----------------------------------------------------------------------------------
static int _check_AmaiMitu( FIELD_ITEMMENU_WORK *pWork )
{
  if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_AMAIMITU)){
      pWork->ret_code = BAG_NEXTPROC_AMAIMITU;  //あまいミツ
    _CHANGE_STATE(pWork,NULL);
    ITEM_Sub( pWork, 1 );   // アイテムを減らす
    return TRUE;
  }else{
    // 使えない
    GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
    WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
    _CHANGE_STATE(pWork,_itemInnerUseError);
    return FALSE;
  
  }
  return FALSE;
  
}
//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】つりざお
 */
//----------------------------------------------------------------------------------
static int _check_Turizao( FIELD_ITEMMENU_WORK *pWork )
{
  if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_TURIZAO)){
      pWork->ret_code = BAG_NEXTPROC_TURIZAO;  //つりざお
    _CHANGE_STATE(pWork,NULL);
    return TRUE;
  }else{
    // のれなかった
    GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
    WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
    _CHANGE_STATE(pWork,_itemInnerUseError);
    return FALSE;
  }
  return FALSE;
  
}
//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】メール
 */
//----------------------------------------------------------------------------------
static int _check_MailView( FIELD_ITEMMENU_WORK *pWork )
{
  if(pWork->ret_code2==BAG_MENU_MIRU){
    if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_MAIL)){
        pWork->ret_code = BAG_NEXTPROC_MAILVIEW;  //メール閲覧
      _CHANGE_STATE(pWork,NULL);
      return TRUE;
    }else{
      // 使えない
      GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
      WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
      _CHANGE_STATE(pWork,_itemInnerUseError);
      return FALSE;
    
    }
  }
  return FALSE;
  
}


//----------------------------------------------------------------------------------
/**
 * @brief 【道具使用可能チェック・発動】ダウジングマシン
 */
//----------------------------------------------------------------------------------
static int _check_DowsingMachine( FIELD_ITEMMENU_WORK* pWork )
{
  if( BAG_MENU_TSUKAU == pWork->ret_code2 )
  {
    if( ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_DOWSINGMACHINE ) )
    {
      pWork->ret_code = BAG_NEXTPROC_DOWSINGMACHINE;  // バッグもメニューも閉じて、ダウジングマシンを使う
      _CHANGE_STATE( pWork, NULL );
      return TRUE;
    }
    else
    {
      // 使えない
      GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
      WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
      _CHANGE_STATE(pWork,_itemInnerUseError);
      return FALSE;
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief あなぬけのヒモチェック
 *
 * @param   pWork   
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _check_AnanukenoHimo( FIELD_ITEMMENU_WORK *pWork )
{
  if(BAG_MENU_TSUKAU == pWork->ret_code2){
    if( ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_ANANUKENOHIMO ) )
    {
      pWork->ret_code = BAG_NEXTPROC_ANANUKENOHIMO;  // バッグもメニューも閉じて、あなぬけのヒモを使う
      _CHANGE_STATE( pWork, NULL );
      ITEM_Sub( pWork, 1 );   // アイテムを減らす
      return TRUE;
    }
    else
    { // 使えない
      GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_ERROR_ARARAGI, pWork->pStrBuf );
      WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
      _CHANGE_STATE(pWork,_itemInnerUseError);
      return FALSE;
    }
  }
  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------------------------------
typedef struct{
  u16 item;
  u16 num;
  int (*check)(FIELD_ITEMMENU_WORK   *pWork);
} ITEMUSE_FUNC_DATA ;

static const ITEMUSE_FUNC_DATA ItemUseFuncTable[]={
  { // 自転車乗る・降りるチェック
    ITEM_ZITENSYA,  1,
    _check_Cycle,
  },
  { // タウンマップ
    ITEM_TAUNMAPPU,  1,
    _check_TownMap,
  },
  { // ともだち手帳
    ITEM_TOMODATITETYOU,  1,
    _check_TomodatiTechou,
  },
  { // あまいミツ
    ITEM_AMAIMITU,  1,
    _check_AmaiMitu,
  },
  { // つりざお
    ITEM_SUGOITURIZAO,  1,
    _check_Turizao,
  },
  { // バトルレコーダー
    ITEM_BATORUREKOODAA,  1,
    _check_BattleRecorder,
  },
  { // ダウジングマシン
    ITEM_DAUZINGUMASIN,  1,
    _check_DowsingMachine,
  },
  { // あなぬけのヒモ
    ITEM_ANANUKENOHIMO,  1,
    _check_AnanukenoHimo,
  },
};

#define ITEMUSE_MAX (NELEMS(ItemUseFuncTable))

//----------------------------------------------------------------------------------
/**
 * @brief 
 *
 * @param   item    
 *
 * @retval  int   
 */
//----------------------------------------------------------------------------------
static int _hit_item( u16 item )
{
  int i;
  for(i=0;i<ITEMUSE_MAX;i++){
    const ITEMUSE_FUNC_DATA *dat = &ItemUseFuncTable[i];
//    OS_Printf("search=%d, num=%d, item=%d\n", dat->item, dat->num, item);
    if(item>=dat->item && item<(dat->item+dat->num)){
      return i;
    }
  }
  return -1;
}
//------------------------------------------------------------------------------
/**
 * @brief   アイテムを選択してどういう動作を行うかきいているところ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemSelectWait(FIELD_ITEMMENU_WORK* pWork)
{
  BOOL bClear=FALSE;

  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);
    pWork->ret_code2 = pWork->submenuList[selectno];
//    OS_Printf("ret_code2 %d %d\n", pWork->ret_code2,selectno);

    switch(pWork->ret_code2){
     // ------------つかう--------------
    case BAG_MENU_TSUKAU:  
      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );   // 戻るボタンをパッシブへ

      if( pWork->pocketno != BAG_POKE_EVENT){     //大切なもの以外はビーコンを飛ばす
        GAMEBEACON_Set_UseItem( pWork->ret_item );
      }
      if((pWork->pocketno==BAG_POKE_WAZA)){       //技マシン
        _CHANGE_STATE(pWork,_itemTecniqueUseInit);
      }else if(1==ITEM_GetParam( pWork->ret_item, ITEM_PRM_EVOLUTION, pWork->heapID )){
        pWork->ret_code = BAG_NEXTPROC_EVOLUTION; //進化アイテム
        _CHANGE_STATE(pWork,NULL);
      }else if(_hit_item(pWork->ret_item)>=0){    // どうぐ使用チェックを増やせる構造
        ItemUseFuncTable[_hit_item(pWork->ret_item)].check( pWork );
      }else {
        // バッグ内で使うアイテム判定
        int ret = BAG_IsInnerItem( pWork->ret_item );
        if( ret==INNER_USE_BAG )
        {
          _CHANGE_STATE(pWork,_itemInnerUse); //メッセージウェイトして通常メニューにもどる
        }
        else if(ret==INNER_USE_AFTER_OUTBAG)
        {
          _CHANGE_STATE(pWork,_itemInnerUseRightStone);
        }
        else
        {
          // 通常の「つかう」はWBではバッグを抜けた後に使う処理
          pWork->ret_code = BAG_NEXTPROC_ITEMUSE;
          _CHANGE_STATE(pWork,NULL);
        }
      }
      break;
    // ----------おりる---------------
    case BAG_MENU_ORIRU:
//      OS_Printf("おりるを選択された\n");
      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );   // 戻るボタンをパッシブへ
      if(ITEMUSE_GetItemUseCheck( pWork->icwk, ITEMCHECK_CYCLE)){
        pWork->ret_code = BAG_NEXTPROC_DROPCYCLE;  //おりる
        _CHANGE_STATE(pWork,NULL);
      }else{
        // おりれなかった
        GFL_MSG_GetString( pWork->MsgManager, MSGID_ITEMUSE_CYCLE_NOSTOP, pWork->pStrBuf );
        WORDSET_RegisterPlayerName( pWork->WordSet, 0, pWork->mystatus );
        WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
        // 文字列のみpExpStrBufに用意しておき遷移先で表示させる
        _CHANGE_STATE(pWork,_itemInnerUseError);
      }
      break;
    // ----------すてる---------------
    case BAG_MENU_SUTERU:   
      _CHANGE_STATE(pWork,_itemTrash);
      break;
    // ----------とうろく------------
    // ----------かいじょ------------
    case BAG_MENU_TOUROKU:  
    case BAG_MENU_KAIZYO:   
      if( ITEMMENU_CheckShortCutSetMode( pWork ) == TRUE ){
        SHORTCUT_SetEventItem( pWork, pWork->curpos );
        ITEMDISP_ChangeActive( pWork, TRUE );
        GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
//        _CHANGE_STATE(pWork, _itemKindSelectMenu);
        ChangeStateItemKindSelectItemMenu( pWork );
      }
      break;
    // ----------みる------------
    case BAG_MENU_MIRU:
      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );   // 戻るボタンをパッシブへ
      if(pWork->ret_item>=ITEM_HAZIMETEMEERU && pWork->ret_item<=ITEM_BURIZZIMEERUw){
        pWork->ret_code = BAG_NEXTPROC_MAILVIEW;  //メール閲覧
        _CHANGE_STATE(pWork,NULL);
      }else{
        GF_ASSERT(0);
      }
      
      break;
    // ----------やめる------------
    case BAG_MENU_YAMERU:
      // タッチ状態で抜けたのならクリア
      if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ) {
        ITEMDISP_upMessageClean( pWork ); // クリア
      }
      // タッチ遷移なら非表示に
      KTST_SetDraw( pWork, (GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY) );
      ITEMDISP_ChangeActive( pWork, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
//      _CHANGE_STATE(pWork, _itemKindSelectMenu);
      ChangeStateItemKindSelectItemMenu( pWork );
      break;
    // ----------うる------------
    case BAG_MENU_URU:
      pWork->ret_code = BAG_NEXTPROC_RETURN;
      _CHANGE_STATE(pWork,NULL);
      break;
    // ----------もたせる------------
    case BAG_MENU_MOTASERU:
      pWork->ret_code = BAG_NEXTPROC_HAVE;
      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );   // 戻るボタンをパッシブへ
      _CHANGE_STATE(pWork,NULL);
      break;
    default:
      GF_ASSERT(0 && "該当する挙動がない");
    }
    bClear = TRUE;
  }

  // 選択終了だったらメニュー面の消去
  if(bClear){
    ITEMDISP_ListPlateClear( pWork );
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }

}



//------------------------------------------------------------------------------
/**
 * @brief   アイテムを選択してどういう動作を行うかきいているところ
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemSelectState(FIELD_ITEMMENU_WORK* pWork)
{

  {  //選んだアイテムのセット
    ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) );
    pWork->ret_item = ITEM_DUMMY_DATA;
    if(item){
      pWork->ret_item = item->id;
    }
  }

  // 決定音
//  PMSND_PlaySE( SE_BAG_DECIDE );

  // ポケモンリスト「もたせる」
  if( pWork->mode == BAG_MODE_POKELIST ){
    // たいせつなもの、既にどうぐもたせてるチェック
    if( ITEM_GetParam( pWork->ret_item, ITEM_PRM_EVENT, pWork->heapID ) == 0 &&
        ITEM_CheckPokeAdd( pWork->ret_item ) == TRUE ){
      // メールかチェック
      if(ITEM_CheckMail( pWork->ret_item )){
        pWork->ret_code = BAG_NEXTPROC_MAILSET;
      }else{
        pWork->ret_code = BAG_NEXTPROC_ITEMEQUIP;
      }
      _CHANGE_STATE(pWork,NULL);
    }else{
      // エラー
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_046, pWork->pStrBuf );
      ITEMMENU_WordsetItemName( pWork, 0, pWork->ret_item );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf );
      ITEMDISP_ItemInfoWindowDisp( pWork );
      ITEMDISP_ChangeActive( pWork, FALSE );
      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );
      _CHANGE_STATE( pWork, _itemTrashEndWait );  // 特に問題が無いと思われるので、捨てたときのキー待ちに飛ばしておく
    }
  // 選択のみの場合
  }else if( pWork->mode == BAG_MODE_SELECT ){
    pWork->ret_code = BAG_NEXTPROC_RETURN;
    _CHANGE_STATE(pWork,NULL);
  // 売る
  }else if( pWork->mode == BAG_MODE_SELL ){
    _CHANGE_STATE(pWork,_itemSellInit);
  }else{
    _itemUseWindowRewrite(pWork);
    ITEMDISP_ItemInfoMessageMake( pWork,pWork->ret_item );
    ITEMDISP_ItemInfoWindowDisp( pWork );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, FALSE );
    _CHANGE_STATE(pWork,_itemSelectWait);
  }
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
  u32 ret=0;
//  BOOL bChange=FALSE;

  if( PRINTSYS_QUE_IsFinished( pWork->SysMsgQue ) == FALSE ){
    return;
  }

  // スクロール中フラグクリア
  if( GFL_UI_TP_GetCont() == FALSE ){
    pWork->scrollMode = FALSE;
  }

//  GFL_UI_KEY_SetRepeatSpeed(1, 6);

  GFL_BMN_Main( pWork->pButton );

  BLINKPALANM_Main( pWork->blwk );

	// 処理が切り替わった場合はキー無効
  if( pWork->state != _itemKindSelectMenu ){
    return;
  }

  // タッチスクロール
  if( _itemScrollCheck(pWork) )
  {
//    bChange = TRUE;
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
    return;
  }

  // カーソルなしの状態から入力があった場合、カーソルを表示して抜ける
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH )
  {
    if( GFL_UI_KEY_GetTrg() )
    {
			if( ITEMMENU_GetItemPocketNumber( pWork ) != 0 ){
	      PMSND_PlaySE( SE_BAG_CURSOR_MOVE );
	      ITEMDISP_upMessageRewrite(pWork); // 上画面表示
	      KTST_SetDraw( pWork, TRUE );
	      return;
			}
    }
		return;
  }

  // キャンセル
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
    pWork->ret_code = BAG_NEXTPROC_RETURN;
    pWork->ret_item = ITEM_DUMMY_DATA;
    GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
//    _CHANGE_STATE(pWork,NULL);
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, NULL );
    return;
  // 強制終了
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X ){
    if( pWork->mode != BAG_MODE_POKELIST ){
      pWork->ret_code = BAG_NEXTPROC_EXIT;
      pWork->ret_item = ITEM_DUMMY_DATA;
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
//      _CHANGE_STATE(pWork,NULL);
      SetEndButtonAnime( pWork, BAR_ICON_EXIT, NULL );
    }
    return;
  // 登録
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
    if( ITEMMENU_CheckShortCutSetMode( pWork ) == TRUE ){
      if( pWork->pocketno == BAG_POKE_EVENT ){
        SHORTCUT_SetEventItem( pWork, pWork->curpos );
      }else{
        SHORTCUT_SetPocket( pWork );
      }
      if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ){
        ITEMDISP_upMessageRewrite(pWork); // 上画面表示
        KTST_SetDraw( pWork, TRUE );
      }
    }
    return;
  // ソート
  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_START ){
    if( pWork->pocketno != BAG_POKE_WAZA && pWork->pocketno != BAG_POKE_NUTS &&
				ITEMMENU_GetItemPocketNumber( pWork ) > 1 ){
      SORT_Button( pWork );
      KTST_SetDraw( pWork, TRUE );
      pWork->oamlistpos_old = 0xffff;
      _windowRewrite(pWork);
      _CHANGE_STATE( pWork, _sortMessageSet );
      return;
    }
  }

  // 決定
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    // アイテム存在チェック
    if( ITEMMENU_GetItemPocketNumber( pWork ) > 0 )
    {
      // アイテムに対する動作選択画面へ
//      _CHANGE_STATE(pWork,_itemSelectState);
      _CHANGE_STATE( pWork, _listSelectAnime );
      return;
    }
  }
  // 並び替え
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT )
  {
    if( pWork->pocketno != BAG_POKE_WAZA && pWork->pocketno != BAG_POKE_NUTS &&
				ITEMMENU_GetItemPocketNumber( pWork ) > 1 ){
      GFL_STD_MemClear( pWork->ScrollItem, sizeof( pWork->ScrollItem ) );
      MYITEM_ITEM_STCopy( pWork->pMyItem, pWork->ScrollItem, pWork->pocketno, TRUE );  //取得
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur , 2 );
      ITEMDISP_ChangeMoveModeButton( pWork, FALSE );
      pWork->moveMode = TRUE;
//      _CHANGE_STATE(pWork,_itemMovePosition);
      ChangeStateItemMovePosition( pWork );
    }
    return;
  }
  // キー操作
  else if( _keyMoveCheck(pWork) )
  {
    ITEMDISP_ScrollCursorChangePos( pWork );
    PMSND_PlaySE( SE_BAG_SLIDE );
//    bChange = TRUE;
    _windowRewrite(pWork);
    return;
  }

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){
    u32 oldpocket = pWork->pocketno;
    pWork->pocketno++;
    if( pWork->pocketno >= BAG_POKE_MAX ){
      pWork->pocketno = 0;
    }
		_pocketCursorChange( pWork, oldpocket, pWork->pocketno );
	  GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    SetPageButtonAnime( pWork, oldpocket, BAR_ICON_RIGHT, _itemPocketChange );
    return;
  }else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){
    u32 oldpocket = pWork->pocketno;
    pWork->pocketno--;
    if( pWork->pocketno < 0 ){
      pWork->pocketno = BAG_POKE_MAX-1;
    }
		_pocketCursorChange( pWork, oldpocket, pWork->pocketno );
		GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
    SetPageButtonAnime( pWork, oldpocket, BAR_ICON_LEFT, _itemPocketChange );
    return;
  }
}

//=============================================================================
//
//
//  __TECNIQUE 技マシン シーケンス
//
//
//=============================================================================

//------------------------------------------------------------------------------
/**
 * @brief   技マシン確認中 はいいえまち
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTecniqueUseYesNo(FIELD_ITEMMENU_WORK* pWork)
{
  u32 ret = 0;//= TOUCH_SW_Main( pWork->pTouchSWSys );

  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      ITEMDISP_ListPlateClear( pWork );
      pWork->ret_code = BAG_NEXTPROC_WAZASET;
      _CHANGE_STATE(pWork,NULL);
    }
    else{
      ITEMDISP_ListPlateClear( pWork );
      GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
      ITEMDISP_ChangeActive( pWork, TRUE );
      if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH ) {
        ITEMDISP_upMessageClean( pWork );
        KTST_SetDraw( pWork, FALSE );
      }else{
        KTST_SetDraw( pWork, TRUE );
      }
//      _CHANGE_STATE(pWork,_itemKindSelectMenu);
      ChangeStateItemKindSelectItemMenu( pWork );
    }
    // YESNOの後始末
    ITEMDISP_YesNoExit(pWork);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   技マシン確認中
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTecniqueUseWait(FIELD_ITEMMENU_WORK* pWork)
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  ITEMDISP_YesNoStart(pWork);

  _CHANGE_STATE(pWork,_itemTecniqueUseYesNo);

}

static void _itemTecniqueUseWaitSE(FIELD_ITEMMENU_WORK* pWork)
{
  if( PMSND_CheckPlaySE() == TRUE ){
    return;
  }

  if( ( GFL_UI_KEY_GetTrg() & MSG_SKIP_BTN ) || GFL_UI_TP_GetTrg() ){
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_065, pWork->pStrBuf );
    WORDSET_RegisterWazaName(pWork->WordSet, 0, ITEM_GetWazaNo( pWork->ret_item ));
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
    _CHANGE_STATE(pWork,_itemTecniqueUseWait);
  }
}

static void _itemTecniqueUseStartSE(FIELD_ITEMMENU_WORK* pWork)
{
  if( !ITEMDISP_MessageEndCheck(pWork) ){
    return;
  }

  // 起動後のＳＥ
  PMSND_PlaySE( SE_BAG_WAZA );
  _CHANGE_STATE( pWork, _itemTecniqueUseWaitSE );
}

//------------------------------------------------------------------------------
/**
 * @brief   技マシン起動
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTecniqueUseInit(FIELD_ITEMMENU_WORK* pWork)
{
/*
  GFL_MSG_GetString( pWork->MsgManager, msg_bag_065, pWork->pStrBuf );
  WORDSET_RegisterWazaName(pWork->WordSet, 0, ITEM_GetWazaNo( pWork->ret_item ));
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
  _CHANGE_STATE(pWork,_itemTecniqueUseWait);
*/
  // 起動した
  if( ITEM_GetHidenMashineNo( pWork->ret_item ) == 0xff ){
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_063, pWork->pExpStrBuf );
  }else{
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_064, pWork->pExpStrBuf );
  }
  ITEMDISP_ItemInfoWindowDispEx( pWork, TRUE );
  _CHANGE_STATE( pWork, _itemTecniqueUseStartSE );
}

//=============================================================================
//
//
//  __TRASH 捨てる シーケンス
//
//
//=============================================================================

//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の操作 はいまち
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrashEndWait(FIELD_ITEMMENU_WORK* pWork)
{
  if( ( GFL_UI_KEY_GetTrg() & MSG_SKIP_BTN ) || GFL_UI_TP_GetTrg() )
  {
    // 再描画
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
		pWork->bCursorChange = 1;
		ITEMDISP_ScrollCursorChangePos( pWork );

//    InputNum_ButtonState( pWork, TRUE );

    GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
    ITEMDISP_ChangeActive( pWork, TRUE );
//    _CHANGE_STATE(pWork,_itemKindSelectMenu);
    ChangeStateItemKindSelectItemMenu( pWork );
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の操作 はいまち
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrashYesWait(FIELD_ITEMMENU_WORK* pWork)
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  _CHANGE_STATE(pWork,_itemTrashEndWait);

}

//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の操作 はいいいえまち
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrashYesNoWait(FIELD_ITEMMENU_WORK* pWork)
{
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    // YESNOの後始末
    ITEMDISP_YesNoExit(pWork);

    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);

    if(selectno==0){
      // 捨てる音
      PMSND_PlaySE( SE_BAG_TRASH );

      // アイテムを減らす
      ITEM_Sub( pWork, pWork->InputNum );

      GFL_MSG_GetString( pWork->MsgManager, msg_bag_055, pWork->pStrBuf );
      ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
      WORDSET_RegisterNumber(pWork->WordSet, 1, pWork->InputNum,
                             3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      
      ITEMDISP_ItemInfoWindowDisp( pWork );

      _CHANGE_STATE(pWork,_itemTrashYesWait);
    }
    else{
//      InputNum_ButtonState( pWork, TRUE );
      GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
      ITEMDISP_ChangeActive( pWork, TRUE );
      KTST_SetDraw( pWork, GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );
//      _CHANGE_STATE(pWork,_itemKindSelectMenu);
      ChangeStateItemKindSelectItemMenu( pWork );
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の操作
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrashYesNo(FIELD_ITEMMENU_WORK* pWork)
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }
  ITEMDISP_YesNoStart(pWork);
  _CHANGE_STATE(pWork,_itemTrashYesNoWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の表示
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrash(FIELD_ITEMMENU_WORK* pWork)
{
  pWork->InputNum = 1;  //初期化

//  InputNum_ButtonState( pWork, FALSE );

  // 数値入力開始
  InputNum_Start( pWork, BAG_INPUT_MODE_TRASH );

  GFL_MSG_GetString( pWork->MsgManager, msg_bag_054, pWork->pStrBuf );
  ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  ITEMDISP_ItemInfoWindowDisp( pWork );

  _CHANGE_STATE(pWork,_itemTrashWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテムを捨てる為の操作
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTrashWait(FIELD_ITEMMENU_WORK* pWork)
{
  int ret;

  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 数値入力主処理
  if( InputNum_Main( pWork ) == TRUE ){
    return;
  }

  ret = CheckNumSelTouch();
  if( ret == GFL_UI_TP_HIT_NONE ){
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      ret = 0;
    }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      ret = 1;
    }
  }

  // 決定
  if( ret == 0 ){
    PMSND_PlaySE( SE_BAG_DECIDE );

    InputNum_Exit( pWork );

    GFL_MSG_GetString( pWork->MsgManager, msg_bag_056, pWork->pStrBuf );
    ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
    WORDSET_RegisterNumber(pWork->WordSet, 1, pWork->InputNum,
                           3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    ITEMDISP_ItemInfoWindowDisp( pWork );

    _CHANGE_STATE(pWork,_itemTrashYesNo);
  // キャンセル
  }else if( ret == 1 ){
    PMSND_PlaySE( SE_BAG_CANCEL );
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, _itemTrashCancel );
  }
}

//=============================================================================
//
//
//  __SELL 売る シーケンス
//
//
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス 初期化
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellInit( FIELD_ITEMMENU_WORK* pWork )
{
//  InputNum_ButtonState( pWork, FALSE );

  GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, FALSE );

  // 買えないもの判定
  {
    s32 val;
    s32 event;

    val   = ITEM_GetParam( pWork->ret_item, ITEM_PRM_PRICE, pWork->heapID );
    event = ITEM_GetParam( pWork->ret_item, ITEM_PRM_EVENT, pWork->heapID );

    // 売値が0、もじくは重要フラグがONの時は売れない
    if( val == 0 || event )
    {
      ITEMDISP_ChangeActive( pWork, FALSE );
      // 「○○○を　かいとることは　できません」
      GFL_MSG_GetString( pWork->MsgManager, mes_shop_093, pWork->pStrBuf );
      ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf );
      ITEMDISP_ItemInfoWindowDisp( pWork );

      ITEMDISP_ChangeRetButtonActive( pWork, FALSE );

      _CHANGE_STATE( pWork, _itemSellEndMsgWait );
      return;
    }
  }

  pWork->InputNum = 1;  //初期化

  // おこづかい表示開始
  ITEMDISP_GoldDispIn( pWork );
  ITEMDISP_ChangeActive( pWork, FALSE );

  // アイテムがひとつしかない場合、入力をスキップ
  {
    ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) );

    HOSAKA_Printf("item->no:%d \n", item->no );

    if( item->no == 1 )
    {
      _CHANGE_STATE( pWork, _itemSellYesnoInit );
      return;
    }
  }

  // 数値入力開始
  InputNum_Start( pWork, BAG_INPUT_MODE_SELL );

  // 「○○○を　いくつ　うりますか？」
  GFL_MSG_GetString( pWork->MsgManager, mes_shop_094, pWork->pStrBuf );
  ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  ITEMDISP_ItemInfoWindowDisp( pWork );

  _CHANGE_STATE( pWork, _itemSellInputWait );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス 入力待ち
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellInputWait( FIELD_ITEMMENU_WORK* pWork )
{
  int ret;

  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 売るシーケンス入力処理
  if( InputNum_Main( pWork ) == TRUE ){
    return;
  }

  ret = CheckNumSelTouch();
  if( ret == GFL_UI_TP_HIT_NONE ){
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      ret = 0;
    }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
      GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
      ret = 1;
    }
  }

   // 決定
  if( ret == 0 ){
    PMSND_PlaySE( SE_BAG_DECIDE );

    // 数値入力終了
    InputNum_Exit( pWork );

    _CHANGE_STATE( pWork, _itemSellYesnoInit );
  // キャンセル
  }else if( ret == 1 ){
    PMSND_PlaySE( SE_BAG_CANCEL );
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, _itemSellInputCancel );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス YESNO開始
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellYesnoInit( FIELD_ITEMMENU_WORK* pWork )
{
  // YESNO開始
  ITEMDISP_YesNoStart(pWork);

  // それでしたら XXXXXX円で おひきとり　いたしましょう
  {
    s32 val;

    // 売値を取得
    val = ITEMMENU_SellPrice( pWork->ret_item, pWork->InputNum, pWork->heapID );

    GFL_MSG_GetString( pWork->MsgManager, mes_shop_095, pWork->pStrBuf );
    WORDSET_RegisterNumber(pWork->WordSet, 0, val,
                             7, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    ITEMDISP_ItemInfoWindowDisp( pWork );
  }

    _CHANGE_STATE( pWork, _itemSellYesnoInput );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス YESNO選択まち
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellYesnoInput( FIELD_ITEMMENU_WORK* pWork )
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  if(APP_TASKMENU_IsFinish(pWork->pAppTask))
  {
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    // YESNOの後始末
    ITEMDISP_YesNoExit(pWork);

    switch( selectno )
    {
      case 0 :
        // YES
        {
          s32 val;

          // 売値を取得
          val = ITEMMENU_SellPrice( pWork->ret_item, pWork->InputNum, pWork->heapID );

          // アイテムを減らす
          ITEM_Sub( pWork, pWork->InputNum );
					pWork->bCursorChange = 1;

          // 所持金増加
          MISC_AddGold( GAMEDATA_GetMiscWork(pWork->gamedata), val );

          // 売却音
          PMSND_PlaySE( SE_BAG_SELL );

          // タスクバーおこづかい表示しなおし
          ITEMDISP_GoldDispWrite( pWork );

          // 再描画
          pWork->oamlistpos_old = 0xffff;
          _windowRewrite(pWork);

          // 「○○○を　わたして XXXXXX円 うけとった」
          GFL_MSG_GetString( pWork->MsgManager, mes_shop_096, pWork->pStrBuf );
          ITEMMENU_WordsetItemName( pWork, 0,  pWork->ret_item );
          WORDSET_RegisterNumber(pWork->WordSet, 1, val, 7, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
          WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
          ITEMDISP_ItemInfoWindowDisp( pWork );

          _CHANGE_STATE( pWork, _itemSellEndMsgWait );
        }
        break;

      case 1 :
        // NO
        KTST_SetDraw( pWork, GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );
        _CHANGE_STATE( pWork, _itemSellExit );
        break;

      default : GF_ASSERT(0);
    }
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス 売った後のメッセージウィエト
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellEndMsgWait( FIELD_ITEMMENU_WORK* pWork )
{
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 入力待ち
  if( GFL_UI_KEY_GetTrg() & MSG_SKIP_BTN ){
    KTST_SetDraw( pWork, TRUE );
    _CHANGE_STATE( pWork, _itemSellExit );
  }else if( GFL_UI_TP_GetTrg() ){
    KTST_SetDraw( pWork, FALSE );
    _CHANGE_STATE( pWork, _itemSellExit );
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  売るシーケンス 終了
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemSellExit( FIELD_ITEMMENU_WORK* pWork )
{
  GFL_BG_ClearScreen(GFL_BG_FRAME3_M);

  // おこづかい表示終了
  ITEMDISP_GoldDispOut( pWork );

//  InputNum_ButtonState( pWork, TRUE );

  GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
  ITEMDISP_ChangeActive( pWork, TRUE );
//  _CHANGE_STATE( pWork, _itemKindSelectMenu );
  ChangeStateItemKindSelectItemMenu( pWork );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  捨てる処理  アイテムを削除
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void ITEM_Sub( FIELD_ITEMMENU_WORK* pWork, int sub_num )
{
  BOOL rslt;
  ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) ); // 選択中のアイテム

  // 手持ちからアイテム削除
  rslt = MYITEM_SubItem(pWork->pMyItem, pWork->ret_item, sub_num, pWork->heapID );

  GF_ASSERT( rslt == TRUE );
  
  // アイテムがなくなった場合
  if( MYITEM_GetItemNum( pWork->pMyItem, pWork->ret_item, pWork->heapID ) == 0 )
  {
    int length = ITEMMENU_GetItemPocketNumber( pWork) + 1;	// 捨てる前の数と比べないとダメなので＋１

    // スクロールバーが無効なら
    if( length < ITEMMENU_SCROLLBAR_ENABLE_NUM )
    {
      // カーソルが指す座標を下げる
      pWork->curpos = MATH_IMax( 0, pWork->curpos-1 );
    }
    else
    {
      HOSAKA_Printf("pre oamlistpos=%d\n", pWork->oamlistpos);
      // リストを下げる
      pWork->oamlistpos = MATH_IMax( pWork->oamlistpos-1 , -1 );
//      pWork->bChange = TRUE;
//      pWork->bCursorChange = 1;
//      pWork->bCellChange = 1;
      
      HOSAKA_Printf("oamlistpos=%d\n", pWork->oamlistpos);
    }
  }
}    

//-----------------------------------------------------------------------------
/**
 *  @brief  数値入力 開始
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void InputNum_Start( FIELD_ITEMMENU_WORK* pWork, BAG_INPUT_MODE mode )
{
  pWork->InputMode = mode;

  ITEMDISP_NumFrameDisp( pWork );
  ITEMDISP_InputNumDisp( pWork, pWork->InputNum );

  // アイコン表示
  GFL_CLACT_WK_SetDrawEnable( pWork->clwkBarIcon[ BAR_ICON_INPUT_U ], TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->clwkBarIcon[ BAR_ICON_INPUT_D ], TRUE );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  数値入力 終了
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void InputNum_Exit( FIELD_ITEMMENU_WORK* pWork )
{
  // 数値入力フレームクリア
  GFL_BG_ClearScreen(GFL_BG_FRAME3_M);

  // アイコン非表示
  GFL_CLACT_WK_SetDrawEnable( pWork->clwkBarIcon[ BAR_ICON_INPUT_U ], FALSE );
  GFL_CLACT_WK_SetDrawEnable( pWork->clwkBarIcon[ BAR_ICON_INPUT_D ], FALSE );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  数値入力 主処理
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static BOOL InputNum_Main( FIELD_ITEMMENU_WORK* pWork )
{
  int tp_result;
  int backup = pWork->InputNum;
  int anm_type;
  BOOL  ret = FALSE;
  ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) ); // 選択中のアイテム

  const GFL_UI_TP_HITTBL tp_tbl[] =
  {
    {
      // 上ボタン
      8*11+4, 8*11+8*3+4,
      8*29+2, 8*29+8*3,
    },
    {
      // 下ボタン
      8*14+4, 8*14+8*3+4,
      8*29+2, 8*29+8*3,
    },
    {
      //終了データ
      GFL_UI_TP_HIT_END,0,0,0,
    }
  };

//  GFL_BMN_Main( pWork->pButton );

  // タッチ入力
  tp_result = GFL_UI_TP_HitCont( tp_tbl );

  if( tp_result != GFL_UI_TP_HIT_NONE )
  {
    pWork->countTouch++;
    ret = TRUE;
  }
  else
  {
    pWork->countTouch = 0;
  }
  

  if( pWork->countTouch == 1 || (pWork->countTouch > BAG_UI_TP_CUR_REPEAT_SYNC /*&& (pWork->countTouch % 8) == 0*/ ) )
  {
    switch( tp_result )
    {
    case 0 :
      if( pWork->countTouch > BAG_UI_TP_CUR_HIGHSPEED_SYNC )
      {
        pWork->InputNum += 10;
      }
      else
      {
        pWork->InputNum++;
      }
      anm_type = 0;
      break;

    case 1 :
      if( pWork->countTouch > BAG_UI_TP_CUR_HIGHSPEED_SYNC )
      {
        pWork->InputNum -= 10;
      }
      else
      {
        pWork->InputNum--;
      }
      anm_type = 1;
      break;

    default : GF_ASSERT(0);

    }
  }

  // キー入力
  if( tp_result == GFL_UI_TP_HIT_NONE )
  {
    if(GFL_UI_KEY_GetRepeat() & PAD_KEY_UP){
      pWork->InputNum++;
      anm_type = 0;
      ret = TRUE;
    }
    else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN){
      pWork->InputNum--;
      anm_type = 1;
      ret = TRUE;
    }
    else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT){
      pWork->InputNum += 10;
      anm_type = 0;
      ret = TRUE;
    }
    else if(GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT){
      pWork->InputNum -= 10;
      anm_type = 1;
      ret = TRUE;
    }
  }

  // 回り込み
  if(item->no < pWork->InputNum)
  {
    pWork->InputNum = 1;
  }
  else if(pWork->InputNum < 1)
  {
    pWork->InputNum = item->no;
  }
  
  // 表示更新
  if(pWork->InputNum != backup)
  {
    // 移動音
    PMSND_PlaySE( SE_BAG_CURSOR_MOVE );

    // ボタンアニメ
    if( anm_type == 0 ){
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_INPUT_U ], 11 );
    }else{
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_INPUT_D ], 10 );
    }

    ITEMDISP_InputNumDisp(pWork,pWork->InputNum);
  }

  return ret;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ボタンの状態変化
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *  @param  on_off
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static void InputNum_ButtonState( FIELD_ITEMMENU_WORK* pWork, BOOL on_off )
{
  if( on_off )
  {
    // 押せる表現

    // ソートボタン
    SORT_ModeReset( pWork );
    // バーアイコン
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_LEFT ],  BAR_ICON_ANM_LEFT );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_RIGHT ], BAR_ICON_ANM_RIGHT );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_EXIT ],  BAR_ICON_ANM_EXIT );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_CHECK_BOX ],  BAR_ICON_ANM_CHECKBOX );
  }
  else
  {
    // 押せない表現

    // ソートボタン
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 4 );
    // バーアイコン各種
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_LEFT ],      BAR_ICON_ANM_LEFT_OFF );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_RIGHT ],     BAR_ICON_ANM_RIGHT_OFF );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_EXIT ],      BAR_ICON_ANM_EXIT_OFF );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_CHECK_BOX ], BAR_ICON_ANM_CHECKBOX_OFF );
  }
}
*/

#include "item/itemtype_def.h"
//--------------------------------------------------------------
/// 種類別ソート用ワーク
//==============================================================
typedef struct {
  ITEM_ST st;
  u32 type;
} ITEM_SORTDATA_TYPE;

//-----------------------------------------------------------------------------
/**
 *  @brief  MATH_QSort用関数 ポケット内のアイテム種類ソート
 *
 *  @param  elem1
 *  @param  elem2
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static s32 QSort_Type( void * elem1, void * elem2 )
{
  ITEM_SORTDATA_TYPE * p1 = (ITEM_SORTDATA_TYPE *)elem1;
  ITEM_SORTDATA_TYPE * p2 = (ITEM_SORTDATA_TYPE *)elem2;

  if( p1->type == p2->type ){
    return 0;
  }else if( p1->type > p2->type ){
    return 1;
  }
  return -1;
}

//-----------------------------------------------------------------------------
/**
 *  @brief  MATH_QSort用関数 ポケット内のアイテム昇順ソート
 *
 *  @param  void* elem1
 *  @param  elem2
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static s32 QSort_ABC( void* elem1, void* elem2 )
{
  ITEM_ST* p1 = (ITEM_ST*) elem1;
  ITEM_ST* p2 = (ITEM_ST*) elem2;
  u16 prio1 = SORT_GetABCPrio( p1->id );
  u16 prio2 = SORT_GetABCPrio( p2->id );

  if( prio1 == prio2 )
  {
    return 0;
  }
  else if( prio1 > prio2 )
  {
    return 1;
  }
  else
  {
    return -1;
  }
}
*/

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケット内のアイテムを種別ソート
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void SORT_Type( FIELD_ITEMMENU_WORK* pWork )
{
  int i;
  int length;
  ITEM_SORTDATA_TYPE * sort;
  ITEM_ST * item;
	ARCHANDLE * ah;

  // メモリ確保
  sort = GFL_HEAP_AllocMemory( pWork->heapID, sizeof(ITEM_SORTDATA_TYPE)*BAG_MYITEM_MAX );
  item = GFL_HEAP_AllocMemory( pWork->heapID, sizeof(ITEM_ST)*BAG_MYITEM_MAX );

  // 取得
  MYITEM_ITEM_STCopy( pWork->pMyItem, item, pWork->pocketno, TRUE );
  length = ITEMMENU_GetItemPocketNumber( pWork );

  // ソート用データ生成
  // カテゴリ << 28 + ソート番号 << 16 + アイテム番号
	ah = ITEM_OpenItemDataArcHandle( pWork->heapID );
  for( i=0; i<length; i++ ){
//    void * dat = ITEM_GetItemArcData( item[i].id, ITEM_GET_DATA, pWork->heapID );
		void * dat = ITEM_GetItemDataArcHandle( ah, item[i].id, pWork->heapID );
    sort[i].type = (ITEM_GetBufParam(dat,ITEM_PRM_ITEM_TYPE)<<28) + (ITEM_GetBufParam(dat,ITEM_PRM_SORT_NUMBER)<<16) + item[i].id;
    sort[i].st   = item[i];
    GFL_HEAP_FreeMemory( dat );
  }
	GFL_ARC_CloseDataHandle( ah );

  MATH_QSort( (void*)sort, length, sizeof(ITEM_SORTDATA_TYPE), QSort_Type, NULL );

  // セーブ用データに整形
  for( i=0; i<length; i++ ){
    item[i] = sort[i].st;
  }

  // 保存
  MYITEM_ITEM_STCopy( pWork->pMyItem, item, pWork->pocketno, FALSE );

  // メモリ解放
  GFL_HEAP_FreeMemory( item );
  GFL_HEAP_FreeMemory( sort );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケット内のアイテムをABCソート
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static void SORT_ABC( FIELD_ITEMMENU_WORK* pWork )
{
  int i;
  int length;
  ITEM_ST item[ BAG_MYITEM_MAX ];

  //取得
  MYITEM_ITEM_STCopy(pWork->pMyItem, item, pWork->pocketno, TRUE);
  length = ITEMMENU_GetItemPocketNumber( pWork );

  HOSAKA_Printf("ABCソート length=%d \n", length );

#ifdef PM_DEBUG
  {
    OS_TPrintf("=============================\n");
    OS_TPrintf(" SORT BEFORE \n");
    OS_TPrintf("=============================\n");

    for( i=0; i<length; i++ )
    {
      OS_TPrintf("[%d] id=%d prio=%d \n",i, item[i].id, SORT_GetABCPrio( item[i].id ) );
    }
  }
#endif

  // ※もしスタックを使い切ったらバッファ確保で対処する
  MATH_QSort( (void*)item, length, sizeof(ITEM_ST), QSort_ABC, NULL );

#ifdef PM_DEBUG
  {
    OS_TPrintf("=============================\n");
    OS_TPrintf(" SORT AFTER \n");
    OS_TPrintf("=============================\n");

    for( i=0; i<length; i++ )
    {
      OS_TPrintf("[%d] id=%d prio=%d \n",i, item[i].id, SORT_GetABCPrio( item[i].id ) );
    }
  }
#endif

  // 保存
  MYITEM_ITEM_STCopy(pWork->pMyItem, item, pWork->pocketno, FALSE);
}
*/

//#include "itemsort_abc_def.h"
//-----------------------------------------------------------------------------
/**
 *  @brief  50音ソート用プライオリティを取得
 *
 *  @param  u16 item_no
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static u16 SORT_GetABCPrio( u16 item_no )
{
  GF_ASSERT( item_no-1 < ITEM_DATA_MAX && item_no != ITEM_DUMMY_DATA );

  // itemsort_abc_def.h で定義
  return ItemSortByAbc[ item_no-1 ];
}
*/

//-----------------------------------------------------------------------------
/**
 *  @brief  ソートボタン処理
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void SORT_Button( FIELD_ITEMMENU_WORK* pWork )
{
  PMSND_PlaySE( SE_BAG_SORT );
  GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort, 2 );


/*
  if( pWork->sort_mode == 0 ){
    SORT_ABC( pWork );
  }else if( pWork->sort_mode == 1 ){
    SORT_Type( pWork );
  }
*/

/*
  // 大切なものはABCソートのみ
  if( pWork->pocketno != BAG_POKE_EVENT )
  {
    // モードチェンジ
    pWork->sort_mode ^= 1;
  }
*/


}

static void SORT_Main( FIELD_ITEMMENU_WORK* pWork )
{
  SORT_Type( pWork );
  // 再描画
  pWork->oamlistpos_old = 0xffff;
  _windowRewrite( pWork );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ソート モードをリセット（ポケット変更時など）
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void SORT_ModeReset( FIELD_ITEMMENU_WORK* pWork )
{
//  pWork->sort_mode = 0;

  HOSAKA_Printf("pocketno:%d\n", pWork->pocketno);

  ITEMDISP_ChangeSortButton( pWork );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ソート アイコン監視
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
/*
static void SORT_Draw( FIELD_ITEMMENU_WORK* pWork )
{
  if( GFL_CLACT_WK_CheckAnmActive( pWork->clwkSort ) == FALSE ){
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort, 0 );
  }
}
*/

//-----------------------------------------------------------------------------
/**
 *  @brief  キーステータスによる表示切替
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *  @param  on_off TRUE:表示(キー操作) FALSE:非表示（タッチ操作）
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void KTST_SetDraw( FIELD_ITEMMENU_WORK* pWork, BOOL on_off )
{
  // 移動モード時は強制的にキー操作にする
  if( pWork->moveMode == TRUE ){
    on_off = TRUE;
  }

  if( on_off )
  {
    GFL_UI_SetTouchOrKey( GFL_APP_END_KEY );
  }
  else
  {
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
  }

  // アイテム0個は強制的にカーソルを出さない
  if( ITEMMENU_GetItemPocketNumber(pWork) == 0 )
  {
    on_off = FALSE;
  }

  if( on_off == FALSE ){
    ITEMDISP_ChangeCursorPosPalette( pWork, 0 );
  }else{
    ITEMDISP_ChangeCursorPosPalette( pWork, 1 );
    BLINKPALANM_InitAnimeCount( pWork->blwk );
  }

  // 上画面表示切り替え
  ITEMDISP_ItemInfoVanishSet( pWork, on_off );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ショートカット設定
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *  @param  pos 表示上の位置
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
BOOL ITEMMENU_CheckShortCutSetMode( FIELD_ITEMMENU_WORK* pWork )
{
  if( pWork->mode == BAG_MODE_FIELD ||
      pWork->mode == BAG_MODE_UNION ||
      pWork->mode == BAG_MODE_COLOSSEUM ){
    return TRUE;
  }
  return FALSE;
}

static void SHORTCUT_SetEventItem( FIELD_ITEMMENU_WORK* pWork, int pos )
{
  // 表示上の位置から実体の位置へ座標変換
  pos += pWork->oamlistpos + 1;

  if(pWork->pocketno == BAG_POKE_EVENT){
    if( ITEMMENU_GetItemPocketNumber(pWork) <= pos ){
      return;
    }else{
      ITEM_ST * item = ITEMMENU_GetItem( pWork, pos );
      SHORTCUT_ID shortcut_id = SHORTCUT_DATA_GetItemToShortcutID( item->id );
      // 登録可能かチェック
      if( shortcut_id == SHORTCUT_ID_NULL ){
        return;
      }

      if( ITEMMENU_CheckCnvButtonItem(pWork,item->id) == TRUE ){
        GAMEDATA_SetShortCut( pWork->gamedata, shortcut_id, FALSE );
      }else{
        GAMEDATA_SetShortCut( pWork->gamedata, shortcut_id, TRUE );
      }
    }

    PMSND_PlaySE( SE_BAG_REGIST_Y ); // 登録音

    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
  }
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケット位置からショートカットIDを取得
 *
 *  @param  int pocketno
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static SHORTCUT_ID SHORTCUT_GetPocket( int pocketno )
{
  return ITEMDISP_GetPocketShortcut( pocketno );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  ポケットのYボタン登録
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void SHORTCUT_SetPocket( FIELD_ITEMMENU_WORK* pWork )
{
  SHORTCUT_ID id;
  BOOL is_active;

  id = SHORTCUT_GetPocket( pWork->pocketno );
  is_active = GAMEDATA_GetShortCut( pWork->gamedata, id );
  is_active ^= 1; ///< 反転
  GAMEDATA_SetShortCut( pWork->gamedata, id, is_active );

  PMSND_PlaySE( SE_BAG_REGIST_Y );

  BTN_DrawCheckBox( pWork );
}

//-----------------------------------------------------------------------------
/**
 *  @brief  チェックボックス描画
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void BTN_DrawCheckBox( FIELD_ITEMMENU_WORK* pWork )
{
  SHORTCUT_ID id;
  BOOL is_active;

  // 入れ替え時は処理しない
  if( pWork->moveMode == TRUE ){
    return;
  }

  id = SHORTCUT_GetPocket( pWork->pocketno );
  is_active = GAMEDATA_GetShortCut( pWork->gamedata, id );

  GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_CHECK_BOX ], 6 + is_active );
}

#if 0
//--------------------------------------------------------------------------------------------
/**
 * アイテム使用エラーメッセージセット
 *
 * @param myst  プレーヤーデータ
 * @param buf   メッセージ展開場所
 * @param item  アイテム番号
 * @param err   エラーID
 * @param heap  ヒープID
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BAG_ItemUseErrorMsgSet( MYSTATUS * myst, STRBUF * buf, u16 item, u32 err, FIELD_ITEMMENU_WORK * pWork )
{
  WORDSET * wset;
  STRBUF * str;

  switch( err ){
  case ITEMCHECK_ERR_CYCLE_OFF:   // 自転車を降りれない
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_058, buf );
    break;
  case ITEMCHECK_ERR_COMPANION:   // 使用不可・連れ歩き
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_err_pair, buf );
    break;
  case ITEMCHECK_ERR_DISGUISE:    // 使用不可・ロケット団変装中
    GFL_MSG_GetString( pWork->MsgManager, msg_bag_err_disguise, buf );
    break;
  default:              // 使用不可・博士の言葉
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
 * @param pWork   バッグ画面ワーク
 * @param type  BAG_TALK_FULL=画面一杯, BAG_TALK_MINI=画面2/3
 *
 * @return  メッセージID
 */
//--------------------------------------------------------------------------------------------
u8 Bag_TalkMsgPrint( BAG_WORK * pWork, int type )
{
  u8  idx;
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

  idx = GF_STR_PrintSimple( win, FONT_TALK,pWork->expb, 0, 0,
                            MSGSPEED_GetWait, BAG_TalkMsgCallBack );

  return idx;
}

//--------------------------------------------------------------------------------------------
/**
 * メニュー：つかう
 *
 * @param pWork   バッグ画面のワーク
 *
 * @return  移行するメインシーケンス
 */
//--------------------------------------------------------------------------------------------
static int Bag_MenuUse( FIELD_ITEMMENU_WORK * pWork )
{
  ITEMCHECK_FUNC  check;
  s32 id;

  // どうぐ使用処理取得
  id    = ITEM_GetParam( pWork->ret_item, ITEM_PRM_FIELD, pWork->heapID );
  // 使うチェックをする関数
  check = (ITEMCHECK_FUNC)ITEMUSE_GetFunc( ITEMUSE_PRM_CHECKFUNC, id );

  // どうぐ使用チェック
  if( check != NULL ){
    u32 ret = check( &pWork->icpWork );
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
 * @param pWork   バッグ画面のワーク
 * @return  none
 */
//--------------------------------------------------------------------------------------------

#if 0

int (*MenuFuncTbl[])( FIELD_ITEMMENU_WORK *pWork)={
  //Bag_MenuUse,    // つかう
  //Bag_MenuUse,    // おりる
  //Bag_MenuUse,    // みる
  //Bag_MenuUse,    // うめる
  //Bag_MenuUse,    // ひらく
  //Bag_MenuSub,    // すてる
  //Bag_MenuCnvSet,   // とうろく
  //Bag_MenuCnvDel,   // かいじょ
  //Bag_MenuSet,    // もたせる
  //Bag_MenuTag,    // タグをみる
  //Bag_MenuKettei,   // けってい
  NULL,       // やめる
  //Bag_MenuItemMove, // いどう
  NULL,       // うる
  //Bag_MenuPlanterUse, // つかう（木の実プランター用）
  //Bag_MenuUse,    // とめる
};

#endif

#if 1

//----------------------------------------------------------------------------------
/**
 * @brief 「つかう」「おりる」「みる」「ひらく」「うめる」「とめる」項目追加
 *
 * @param   pWork   
 * @param   itemdata    
 * @param   item    
 * @param   tbl     メニュー項目テーブル
 */
//----------------------------------------------------------------------------------
static void _tsukau_check( FIELD_ITEMMENU_WORK *pWork, void *itemdata, ITEM_ST * item, u8 *tbl )
{
  if( ITEM_GetBufParam( itemdata,  ITEM_PRM_FIELD ) != 0 )
  {
    if( ITEM_CheckMail( item->id ) == TRUE )
    {
      tbl[BAG_MENU_USE] = BAG_MENU_MIRU;
    }
    else if( item->id == ITEM_ZITENSYA && pWork->cycle_flg == 1 )
    {
      tbl[BAG_MENU_USE] = BAG_MENU_ORIRU;
    }
    else
    {
      tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU;
    }
  }

  // ダークストーン・ライトストーンは特殊な状況で使用可能になる
  if( (item->id == ITEM_RAITOSUTOON || item->id == ITEM_DAAKUSUTOON))
  {
    tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU;
  }

}

//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム選択時のメニューリストを生成
 *
 *  @param  FIELD_ITEMMENU_WORK * pWork
 *  @param  tbl 
 *
 *  @retval none
 */
//-----------------------------------------------------------------------------
static void ItemMenuMake( FIELD_ITEMMENU_WORK * pWork, u8* tbl )
{
  int  i;
  void * itemdata;
  u8  pocket;
  ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) );

  if((item==NULL) || (item->id==ITEM_DUMMY_DATA)){
    return;
  }

  itemdata = ITEM_GetItemArcData( item->id, ITEM_GET_DATA, pWork->heapID );
  pocket   = pWork->pocketno;

#if 0
  for(i=0;i<BAG_MENUTBL_MAX;i++){
    pWork->menu_func[i] = NULL;
  }
#endif

  //「つかう」など
  _tsukau_check( pWork, itemdata, item, tbl );
  // 特定の場所では「みる」のみ
  if( CheckEventItemUse( pWork, pocket ) == FALSE ){
    if( tbl[BAG_MENU_USE] != BAG_MENU_MIRU ){
      tbl[BAG_MENU_USE] = 255;
    }
  }

  // もたせる
  // すてる
  if( ITEM_GetBufParam( itemdata, ITEM_PRM_EVENT ) == 0 ){
    // ↑だいじなものは弾く
    if( ITEM_CheckPokeAdd( item->id ) == TRUE ){
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
    if( ITEMMENU_CheckCnvButtonItem( pWork, item->id  ) == TRUE ){
      tbl[BAG_MENU_SUB] = BAG_MENU_KAIZYO;
    }else{
      tbl[BAG_MENU_SUB] = BAG_MENU_TOUROKU;
    }
  }

  // やめる
  tbl[BAG_MENU_CANCEL] = BAG_MENU_YAMERU;

//  OS_Printf("pocket=%d\n",   pocket);
//  OS_Printf("tbl[BAG_MENU_USE]=%d\n",   tbl[BAG_MENU_USE]);
//  OS_Printf("tbl[BAG_MENU_GIVE]=%d\n",  tbl[BAG_MENU_GIVE]);
//  OS_Printf("tbl[BAG_MENU_SUB]=%d\n",   tbl[BAG_MENU_SUB]);

#if 0
  for(i=0;i<BAG_MENUTBL_MAX;i++){
    if(tbl[i]!=255){
      pWork->menu_func[i] = MenuFuncTbl[tbl[i]];
    }
//    OS_Printf("menu_func[%d]=%08x\n", i,pWork->menu_func[i]);
  }
#endif

  GFL_HEAP_FreeMemory( itemdata );
}

#endif


//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム使用ウィンドウ描画
 *
 *  @param  pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork)
{
  int strtbl[]= {
    msg_bag_001,   ///< つかう
    msg_bag_007,   ///< おりる
    msg_bag_017,   ///< みる
    msg_bag_002,   ///< すてる
    msg_bag_003,   ///< とうろく
    msg_bag_019,   ///< かいじょ
    msg_bag_004,   ///< もたせる
    msg_bag_006,   ///< けってい
    msg_bag_009,   ///< やめる
    mes_shop_103,  ///< うる
    msg_bag_001,   ///< つかう
  };
  u8  tbl[BAG_MENUTBL_MAX] = { 255, 255, 255, 255 };
    
  int stringbuff[BAG_MENUTBL_MAX];
  int i, j;

  ItemMenuMake( pWork, tbl );

  for( i=0,j=0; i<BAG_MENUTBL_MAX; i++ ){
    if( tbl[i] == 255 ){ continue; }
    pWork->submenuList[j] = tbl[i];
    stringbuff[j] = strtbl[tbl[i]];
    j++;
  }
  ITEMDISP_MenuWinDisp( pWork, stringbuff, j );
}


//----------------------------------------------------------------------------
/**
 *  @brief  便利登録されているか
 */
//-----------------------------------------------------------------------------

BOOL ITEMMENU_CheckCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  return GAMEDATA_GetShortCut( pWork->gamedata, SHORTCUT_DATA_GetItemToShortcutID(no) );
}

//----------------------------------------------------------------------------
/**
 *  @brief  チェックを追加
 */
//-----------------------------------------------------------------------------
/*
BOOL ITEMMENU_AddCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  int i;
  for(i = 0 ; i < DUMMY_SHORTCUT_MAX ; i++){
    if(0 == MYITEM_CnvButtonItemGet( pWork->pMyItem, i)){
      MYITEM_CnvButtonItemSet( pWork->pMyItem, i, no);
      return TRUE;
    }
  }
  return FALSE;
}
*/

//----------------------------------------------------------------------------
/**
 *  @brief  チェックを消す
 */
//-----------------------------------------------------------------------------
/*
void ITEMMENU_RemoveCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  int i=ITEMMENU_GetPosCnvButtonItem(pWork,no);

  if(i!=-1){
    MYITEM_CnvButtonItemSet( pWork->pMyItem, i, 0);
  }
}
*/


//----------------------------------------------------------------------------
/**
 *  @brief  ボタンイベントコールバック
 *
 *  @param  bttnid    ボタンID
 *  @param  event   イベント種類
 *  @param  p_work    ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  FIELD_ITEMMENU_WORK* pWork = p_work;
  int pocketno = -1;
  int length = ITEMMENU_GetItemPocketNumber(pWork);

  if(event!=GFL_BMN_EVENT_TOUCH){
    // タッチした瞬間でなければ抜ける
    return;
  }

  // バッグ
  if(BAG_POKE_MAX > bttnid){
    int id2no[]={1,2,3,4,0};
    if( pWork->pocketno != id2no[bttnid] ){
      pocketno = id2no[bttnid];  //どうぐのあたりが広い為、順番を最後にしている
      KTST_SetDraw( pWork, FALSE );
    }
  // ポケット切り替え左ボタン
  }else if(BUTTONID_LEFT == bttnid){
    u32 oldpocket = pWork->pocketno;
    pWork->pocketno--;
    if(pWork->pocketno < 0){
      pWork->pocketno = BAG_POKE_MAX-1;
    }
		_pocketCursorChange( pWork, oldpocket, pWork->pocketno );
		KTST_SetDraw( pWork, FALSE );
    SetPageButtonAnime( pWork, oldpocket, BAR_ICON_LEFT, _itemPocketChange );
    return;
  // ポケット切り替え右ボタン
  }else if(BUTTONID_RIGHT == bttnid){
    u32 oldpocket = pWork->pocketno;
    pWork->pocketno++;
    if(pWork->pocketno >= BAG_POKE_MAX){
      pWork->pocketno = 0;
    }
		_pocketCursorChange( pWork, oldpocket, pWork->pocketno );
		KTST_SetDraw( pWork, FALSE );
    SetPageButtonAnime( pWork, oldpocket, BAR_ICON_RIGHT, _itemPocketChange );
    return;
  // ソートボタン
  }else if(BUTTONID_SORT == bttnid){
    // 技マシン・木の実は処理なし
    if( pWork->pocketno != BAG_POKE_WAZA && pWork->pocketno != BAG_POKE_NUTS &&
				ITEMMENU_GetItemPocketNumber( pWork ) > 1 ){
      SORT_Button( pWork );
      KTST_SetDraw( pWork, FALSE );
      pWork->oamlistpos_old = 0xffff;
      _windowRewrite(pWork);
      _CHANGE_STATE( pWork, _sortMessageSet );
    }
  // Ｙ登録ボタン
  }else if(BUTTONID_CHECKBOX == bttnid){
    if( ITEMMENU_CheckShortCutSetMode( pWork ) == TRUE ){
      SHORTCUT_SetPocket( pWork );
      KTST_SetDraw( pWork, FALSE );
      pWork->oamlistpos_old = 0xffff;
      _windowRewrite(pWork);
    }
  // 終了ボタン
  }else if(BUTTONID_EXIT == bttnid){
    if( pWork->mode != BAG_MODE_POKELIST ){
      pWork->ret_code = BAG_NEXTPROC_EXIT;
      pWork->ret_item = ITEM_DUMMY_DATA;
      SetEndButtonAnime( pWork, BAR_ICON_EXIT, NULL );
    }
  // 戻るボタン
  }else if(BUTTONID_RETURN == bttnid){
    pWork->ret_code = BAG_NEXTPROC_RETURN;
    pWork->ret_item = ITEM_DUMMY_DATA;
    SetEndButtonAnime( pWork, BAR_ICON_RETURN, NULL );
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
  // リスト
  }else if((bttnid >= BUTTONID_ITEM_AREA) && (bttnid < BUTTONID_CHECK_AREA)){
    // アイテム選択シーケンスでの操作
    if( pWork->state == _itemKindSelectMenu ){
      int backup = pWork->curpos;
      int nowno = ITEMMENU_GetItemIndex(pWork);

      // アイテムがなかった場合
      if( ITEMMENU_GetItemPocketNumber( pWork ) == 0 )
      {
        // 処理なし
        return;
      }

      if(pWork->curpos != (bttnid-BUTTONID_ITEM_AREA))
      {
        pWork->curpos = bttnid - BUTTONID_ITEM_AREA;
        if( length <= ITEMMENU_GetItemIndex(pWork) )
        {
          pWork->curpos = backup;
          return;
        }
      }

//    ITEMDISP_ScrollCursorChangePos( pWork );
      _ItemChange(pWork, nowno, ITEMMENU_GetItemIndex(pWork));

      // タッチ通知
      KTST_SetDraw( pWork, FALSE ); 
      pWork->oamlistpos_old = 0xffff;
      _windowRewrite(pWork);

      // 上画面表示
      ITEMDISP_upMessageRewrite(pWork);

//      _CHANGE_STATE(pWork,_itemSelectState);
      _CHANGE_STATE( pWork, _listSelectAnime );
    }
    return;
  // チェックボックス
  }else if(bttnid >= BUTTONID_CHECK_AREA){
    if( ITEMMENU_CheckShortCutSetMode( pWork ) == TRUE ){
      int no = bttnid - BUTTONID_CHECK_AREA;
      SHORTCUT_SetEventItem( pWork, no );
    }
  }

  if(pocketno != -1){
		u32	old = pWork->pocketno;
		pWork->pocketno = pocketno;
    PMSND_PlaySE( SE_BAG_BAG_GRA );
    // ポケットカーソル移動
		_pocketCursorChange( pWork, old, pWork->pocketno );
    // ソートボタン表示切替
    SORT_ModeReset( pWork );

    KTST_SetDraw( pWork, FALSE );
    pWork->oamlistpos_old = 0xffff;
    _windowRewrite(pWork);
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
static void _VBlank( GFL_TCB *tcb, void *work )
{
  FIELD_ITEMMENU_WORK* pWork = work;
  
  // 上画面、表示非表示切替
  if( pWork->bDispUpReq == 1 ){
    ITEMDISP_upMessageSetDispVBlank( pWork, TRUE );
    pWork->bDispUpReq = 0;
  }else if( pWork->bDispUpReq == 2 ){
    ITEMDISP_upMessageSetDispVBlank( pWork, FALSE );
    pWork->bDispUpReq = 0;
  }

  PaletteFadeTrans( pWork->pfd );

  GFL_CLACT_SYS_VBlankFunc(); //セルアクターVBlank

/*
  // セルリスト更新
  if(pWork->bChange)
  {
    ITEMDISP_CellVramTrans(pWork);
    pWork->bChange = FALSE;
  }
*/
}


//-----------------------------------------------------------------------------
/**
 *  @brief  アイテム選択メッセージ
 *
 *  @param  pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _startState(FIELD_ITEMMENU_WORK* pWork)
{
  BOOL  wipe, obj;

  wipe = WIPE_SYS_EndCheck();
  obj  = ITEMDISP_InitBagMode( pWork );

  if( wipe == TRUE && obj == FALSE ){
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->clwkScroll, TRUE );
//    _CHANGE_STATE(pWork, _itemKindSelectMenu);
    ChangeStateItemKindSelectItemMenu( pWork );
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
  BAG_PARAM* pParam = ppWork;
  FIELD_ITEMMENU_WORK* pWork;

//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0);

  // ヒープ生成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ITEMMENU, BAG_HEAPSIZE );
  pWork = GFL_PROC_AllocWork( proc, sizeof(FIELD_ITEMMENU_WORK), HEAPID_ITEMMENU );
  GFL_STD_MemClear( pWork, sizeof(FIELD_ITEMMENU_WORK) );

  //-------------------------------------
  // メンバ初期化
  //-------------------------------------
  GFL_UI_KEY_GetRepeatSpeed( &pWork->key_repeat_speed, &pWork->key_repeat_wait );

  pWork->heapID = HEAPID_ITEMMENU;

  pWork->active = TRUE;

  // パラメータから取得
  pWork->gamedata   = pParam->p_gamedata;
  pWork->mystatus   = pParam->p_mystatus;
  pWork->config     = pParam->p_config;
  pWork->pBagCursor = pParam->p_bagcursor;
  pWork->pMyItem    = pParam->p_myitem;
  pWork->icwk       = &pParam->icwk;
  pWork->mode       = pParam->mode;
  pWork->cycle_flg  = pParam->cycle_flg;

  // アイテム破損データを修復
  MYITEM_CheckSafety( pWork->pMyItem );

  HOSAKA_Printf("bag mode =%d \n", pWork->mode );

// 保坂のみデバッグ押しながら起動でショップから起動したことにする
#ifdef DEBUG_ONLY_FOR_genya_hosaka
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG) )
  {
    pWork->mode = BAG_MODE_SELL;
    HOSAKA_Printf("hook sell\n");
  }
#endif

  // その他
  pWork->pocketno = MYITEM_FieldBagPocketGet(pWork->pBagCursor);

	{	// カーソル位置補正
		s16	cur, scr;
		u16	max;
		u16	i;

		for( i=0; i<BAG_POKE_MAX; i++ ){
			MYITEM_FieldBagCursorGet( pWork->pBagCursor, i, &cur, &scr );
			max = MYITEM_GetItemPocketNumber( pWork->pMyItem, i );
			while( 1 ){
				if( cur == 0 && scr == 0 ){
					break;
				}
				if( (cur+scr) >= max ){
					if( scr != 0 ){
						scr--;
					}else{
						cur--;
					}
				}else{
					break;
				}
			}
			if( i == pWork->pocketno ){
				pWork->curpos = cur;
				pWork->oamlistpos = scr - 1;
			}
			MYITEM_FieldBagCursorSet( pWork->pBagCursor, i, cur, scr );
		}
	}

  // 文字列初期化
  pWork->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                      NARC_message_bag_dat, pWork->heapID );

  pWork->pStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
  pWork->pExpStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
  pWork->pItemNameBuf = GFL_STR_CreateBuffer(64,pWork->heapID);
  pWork->WordSet    = WORDSET_Create( pWork->heapID );
//  pWork->SysMsgQue  = PRINTSYS_QUE_Create( pWork->heapID );
  pWork->SysMsgQue  = PRINTSYS_QUE_CreateEx( 1024*4, pWork->heapID );   // デフォルトの４倍
  pWork->MsgCursorWork =  APP_KEYCURSOR_Create( 15, TRUE, TRUE, pWork->heapID );
  pWork->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                       GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );
 
  // グラフィック初期化
  ITEMDISP_graphicInit(pWork);

  ITEMDISP_upMessageCreate(pWork);
  ITEMDISP_CellResourceCreate(pWork);
  ITEMDISP_InitPocketCell( pWork );

  _windowCreate(pWork);
  ITEMDISP_CellCreate(pWork);

  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );

  InitBlinkPalAnm( pWork );
  InitPaletteAnime( pWork );

  // ワイプ開始
  WIPE_SYS_Start(
    WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN,
    WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, pWork->heapID );

  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );

  //  pWork->pTouchSWSys = TOUCH_SW_AllocWork(pWork->heapID);

  ITEMDISP_BarMessageCreate( pWork );

  pWork->pAppTaskRes = APP_TASKMENU_RES_Create( GFL_BG_FRAME3_M, _SUBLIST_NORMAL_PAL, pWork->fontHandle, pWork->SysMsgQue, pWork->heapID );

  // ネットワーク初期化
	GFL_NET_ReloadIconTopOrBottom( TRUE, pWork->heapID );
  
  // 初期化
  KTST_SetDraw( pWork ,GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );
  
  ITEMDISP_SetVisible();

  PMSND_PlaySE( SE_BAG_SLIDE_IN );
  _CHANGE_STATE(pWork, _startState);
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
  FIELD_ITEMMENU_WORK* pWork = mypWork;
  StateFunc* state = pWork->state;

  if(state == NULL)
  {
    // ワイプ ブラックアウト開始
    WIPE_SYS_Start(
      WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT,
      WIPE_FADE_BLACK, WIPE_DEF_DIV, WIPE_DEF_SYNC, pWork->heapID );

    return GFL_PROC_RES_FINISH;
  }
/*
  else if( WIPE_SYS_EndCheck() != TRUE )
  {
    return GFL_PROC_RES_CONTINUE;
  }
*/

  pWork->oamlistpos_old = pWork->oamlistpos;

  state(pWork);
//  SORT_Draw(pWork);

  // セルリスト更新
/*
  if(pWork->bChange)
  {
    ITEMDISP_CellVramTrans(pWork);
    pWork->bChange = FALSE;
  }
*/
  // カーソル位置変更
  if( pWork->bCursorChange ){
    ITEMDISP_CangeCursorPos( pWork );
    pWork->bCursorChange = 0;
  }
  // リスト転送
  if( pWork->bCellChange ){
    ITEMDISP_CellVramTrans( pWork );
    pWork->bCellChange = 0;
  }

  _dispMain(pWork);

//  PRINTSYS_QUE_Main(pWork->SysMsgQue);
  ITEMDISP_PrintUtilTrans( pWork );

  GFL_TCBL_Main( pWork->pMsgTcblSys );

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
  FIELD_ITEMMENU_WORK* pWork = mypWork;

  if (WIPE_SYS_EndCheck() != TRUE) {
    return GFL_PROC_RES_CONTINUE;
  }

  if( PRINTSYS_QUE_IsFinished( pWork->SysMsgQue ) == FALSE ){
    return GFL_PROC_RES_CONTINUE;
  }

  // GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0);
  GFL_TCB_DeleteTask( pWork->g3dVintr );

  MYITEM_FieldBagPocketSet(pWork->pBagCursor, pWork->pocketno);
  MYITEM_FieldBagCursorSet(pWork->pBagCursor, pWork->pocketno, pWork->curpos, pWork->oamlistpos+1 );

  APP_TASKMENU_RES_Delete( pWork->pAppTaskRes );

  ITEMDISP_upMessageDelete(pWork);
  ITEMDISP_graphicDelete(pWork);

  //  TOUCH_SW_FreeWork(pWork->pTouchSWSys);

  GFL_TCBL_Exit(pWork->pMsgTcblSys);// = GFL_TCBL_Init( work->heapId , work->heapId , 1 , 0 );


  GFL_MSG_Delete(pWork->MsgManager);
//  GFL_MSG_Delete(pWork->MsgManagerItemInfo);
//  GFL_MSG_Delete(pWork->MsgManagerPocket);
//  GFL_MSG_Delete(pWork->MsgManagerItemName);

  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_STR_DeleteBuffer(pWork->pExpStrBuf);
  GFL_STR_DeleteBuffer(pWork->pItemNameBuf);
  WORDSET_Delete(pWork->WordSet);

  ITEMDISP_BarMessageDelete( pWork );

  if(pWork->itemInfoDispWin.win){

    GFL_BG_FreeCharacterArea(GFL_BG_FRAME3_M,
                             GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                             GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));

    GFL_BMPWIN_Delete(pWork->itemInfoDispWin.win);
  }

  GFL_BG_FillCharacterRelease(GFL_BG_FRAME3_M,1,0);

  GFL_FONT_Delete(pWork->fontHandle);
  APP_KEYCURSOR_Delete(pWork->MsgCursorWork);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
  ExitPaletteAnime( pWork );
  ExitBlinkPalAnm( pWork );
  GFL_BMN_Delete(pWork->pButton);
  //  ITEMDISP_ListPlateDelete(pWork);

  GFL_BMPWIN_Exit();
  GFL_BG_Exit();

  // 出力パラメータを設定
  {
    BAG_PARAM* pParam = ppWork;

    pParam->next_proc = pWork->ret_code;
    pParam->ret_item  = pWork->ret_item;
  }

  GFL_UI_KEY_SetRepeatSpeed( pWork->key_repeat_speed, pWork->key_repeat_wait );

  GFL_PROC_FreeWork( proc );

  GFL_HEAP_DeleteHeap(  HEAPID_ITEMMENU );

  return GFL_PROC_RES_FINISH;

}



// プロセス定義データ
const GFL_PROC_DATA ItemMenuProcData = {
  FieldItemMenuProc_Init,
  FieldItemMenuProc_Main,
  FieldItemMenuProc_End,
};





//--------------------------------------------------------------------------------------------
//  ↓ここから中村追加分
//--------------------------------------------------------------------------------------------
static void ChangeStateItemKindSelectItemMenu( FIELD_ITEMMENU_WORK * wk )
{
  _CHANGE_STATE( wk, _itemKindSelectMenu );
  GFL_UI_KEY_SetRepeatSpeed( 3, 6 );
}

static void ChangeStateItemMovePosition( FIELD_ITEMMENU_WORK * wk )
{
  _CHANGE_STATE( wk, _itemMovePosition );
  GFL_UI_KEY_SetRepeatSpeed( 3, 6 );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   リストカーソル点滅アニメ
 */
//--------------------------------------------------------------------------------------------
static void InitBlinkPalAnm( FIELD_ITEMMENU_WORK * wk )
{
  ARCHANDLE * ah;

  wk->blwk = BLINKPALANM_Create( (_PAL_WIN01_CELL+1)*16, 16, BLINKPALANM_MODE_MAIN_OBJ, wk->heapID );

  ah = GFL_ARC_OpenDataHandle( ARCID_BAG, wk->heapID );
  BLINKPALANM_SetPalBufferArcHDL( wk->blwk, ah, NARC_bag_bag_win01_d_NCLR, 16, 32 );
  GFL_ARC_CloseDataHandle( ah );
}

static void ExitBlinkPalAnm( FIELD_ITEMMENU_WORK * wk )
{
  BLINKPALANM_Exit( wk->blwk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   リスト選択アニメ
 */
//--------------------------------------------------------------------------------------------
static void _listSelectAnime( FIELD_ITEMMENU_WORK * wk )
{
  switch( wk->tmpSeq ){
  case 0:
    PMSND_PlaySE( SE_BAG_DECIDE );
  case 3:
    ITEMDISP_ChangeCursorPosPalette( wk, 0 );
    wk->tmpSeq++;
    break;

  case 1:
  case 4:
  case 6:
    wk->tmpCnt++;
    if( wk->tmpCnt == 4 ){
      wk->tmpCnt = 0;
      wk->tmpSeq++;
    }
    break;

  case 2:
  case 5:
    ITEMDISP_ChangeCursorPosPalette( wk, 2 );
    wk->tmpSeq++;
    break;

  case 7:
    wk->tmpSeq = 0;
    // メニュー終了時を考えて初期化しておく
    ITEMDISP_ChangeCursorPosPalette( wk, 1 );
    BLINKPALANM_InitAnimeCount( wk->blwk );
    BLINKPALANM_Main( wk->blwk );
    _CHANGE_STATE( wk, _itemSelectState );
    break;
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   タッチバーボタンアニメ
 */
//--------------------------------------------------------------------------------------------
static void SetEndButtonAnime( FIELD_ITEMMENU_WORK * wk, u16 type, StateFunc * next )
{
  u32 anm;

  if( type == BAR_ICON_RETURN ){
    PMSND_PlaySE( SE_BAG_CANCEL );
    anm = APP_COMMON_BARICON_RETURN_ON;
  }else if( type == BAR_ICON_EXIT ){
    PMSND_PlaySE( SE_BAG_CLOSE );
    anm = APP_COMMON_BARICON_EXIT_ON;
  }else if( type == BAR_ICON_RIGHT ){
    PMSND_PlaySE( SE_BAG_POCKET_MOVE );
    anm = APP_COMMON_BARICON_CURSOR_RIGHT_ON;
  }else if( type == BAR_ICON_LEFT ){
    PMSND_PlaySE( SE_BAG_POCKET_MOVE );
    anm = APP_COMMON_BARICON_CURSOR_LEFT_ON;
  }
  wk->tmpSeq = type;

  GFL_CLACT_WK_SetAnmFrame( wk->clwkBarIcon[wk->tmpSeq], 0 );
  GFL_CLACT_WK_SetAnmSeq( wk->clwkBarIcon[wk->tmpSeq], anm );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkBarIcon[wk->tmpSeq], TRUE );
  wk->chgState = next;

  _CHANGE_STATE( wk, _endButtonAnime );
}

static void _endButtonAnime( FIELD_ITEMMENU_WORK * wk )
{
  if( GFL_CLACT_WK_CheckAnmActive( wk->clwkBarIcon[wk->tmpSeq] ) == FALSE ){
    wk->tmpSeq = 0;
    _CHANGE_STATE( wk, wk->chgState );
  }
}

static void SetPageButtonAnime( FIELD_ITEMMENU_WORK * wk, u32 old, u16 type, StateFunc * next )
{
  u32 anm;

  // ポケットカーソル移動
//  _pocketCursorChange( wk, old, wk->pocketno );
  // ソートボタン表示切替
  SORT_ModeReset( wk );
  BTN_DrawCheckBox( wk );
  wk->oamlistpos_old = 0xffff;
  _windowRewrite( wk );

  if( type == BAR_ICON_RIGHT ){
    PMSND_PlaySE( SE_BAG_POCKET_MOVE );
    anm = APP_COMMON_BARICON_CURSOR_RIGHT_ON;
  }else if( type == BAR_ICON_LEFT ){
    PMSND_PlaySE( SE_BAG_POCKET_MOVE );
    anm = APP_COMMON_BARICON_CURSOR_LEFT_ON;
  }
  wk->tmpSeq = type;
  wk->tmpCnt = 6;

  GFL_CLACT_WK_SetAnmFrame( wk->clwkBarIcon[wk->tmpSeq], 0 );
  GFL_CLACT_WK_SetAnmSeq( wk->clwkBarIcon[wk->tmpSeq], anm );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkBarIcon[wk->tmpSeq], TRUE );
  wk->chgState = next;

  _CHANGE_STATE( wk, _waitCount );
}

static void _waitCount( FIELD_ITEMMENU_WORK * wk )
{
  if( wk->tmpCnt == 0 ){
    wk->tmpSeq = 0;
    _CHANGE_STATE( wk, wk->chgState );
  }else{
    wk->tmpCnt--;
  }
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   捨てる・売るのタッチチェック
 */
//--------------------------------------------------------------------------------------------

static int CheckNumSelTouch(void)
{
  static const GFL_UI_TP_HITTBL tbl[] =
  {
    { _WINNUM_SCR_INITY*8, _WINNUM_SCR_TP_END_Y*8-1, _WINNUM_SCR_INITX*8, _WINNUM_SCR_TP_END_X*8-1 },
    { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, _BAR_CELL_CURSOR_RETURN, _BAR_CELL_CURSOR_RETURN+TOUCHBAR_ICON_WIDTH-1 },  //リターン
    { GFL_UI_TP_HIT_END, 0, 0, 0 },
  };
  int ret = GFL_UI_TP_HitTrg( tbl );

  if( ret != GFL_UI_TP_HIT_NONE ){
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
  }
  return ret;
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   捨てるキャンセル処理
 */
//--------------------------------------------------------------------------------------------

static void _itemTrashCancel( FIELD_ITEMMENU_WORK * wk )
{
  InputNum_Exit( wk );

//  InputNum_ButtonState( wk, TRUE );

  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkScroll, TRUE );
  ITEMDISP_ChangeActive( wk, TRUE );
  KTST_SetDraw( wk, GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );
//  _CHANGE_STATE( wk, _itemKindSelectMenu );
  ChangeStateItemKindSelectItemMenu( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   売るキャンセル処理
 */
//--------------------------------------------------------------------------------------------

static void _itemSellInputCancel( FIELD_ITEMMENU_WORK * wk )
{
  InputNum_Exit( wk );

  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkScroll, TRUE );
  ITEMDISP_ChangeActive( wk, TRUE );
  KTST_SetDraw( wk, GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );

  _CHANGE_STATE( wk, _itemSellExit );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief   パレットアニメ
 */
//--------------------------------------------------------------------------------------------

static void InitPaletteAnime( FIELD_ITEMMENU_WORK * wk )
{
  wk->pfd = PaletteFadeInit( wk->heapID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_MAIN_OBJ, FADE_PAL_ALL_SIZE, wk->heapID );
  PaletteFadeWorkAllocSet( wk->pfd, FADE_MAIN_BG, FADE_PAL_ONE_SIZE, wk->heapID );
  PaletteWorkSet_VramCopy( wk->pfd, FADE_MAIN_BG, 0, FADE_PAL_ONE_SIZE );
}

static void ExitPaletteAnime( FIELD_ITEMMENU_WORK * wk )
{
  PaletteFadeWorkAllocFree( wk->pfd, FADE_MAIN_BG );
  PaletteFadeWorkAllocFree( wk->pfd, FADE_MAIN_OBJ );
  PaletteFadeFree( wk->pfd );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   ソートメッセージ表示
 */
//--------------------------------------------------------------------------------------------
static void _sortMessageSet( FIELD_ITEMMENU_WORK * wk )
{
  if( GFL_CLACT_WK_CheckAnmActive( wk->clwkSort ) == FALSE ){
    GFL_CLACT_WK_SetAnmSeq( wk->clwkSort, 0 );
    SORT_Main( wk );
    GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkScroll, FALSE );
    GFL_MSG_GetString( wk->MsgManager, mes_bag_113, wk->pExpStrBuf );
    ITEMDISP_ItemInfoWindowDispEx( wk, FALSE );
    ITEMDISP_ChangeRetButtonActive( wk, FALSE );
    ITEMDISP_ChangeActive( wk, FALSE );
    _CHANGE_STATE( wk, _sortMessageWait );
  }
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   ソートメッセージウェイト
 */
//--------------------------------------------------------------------------------------------
static void _sortMessageWait( FIELD_ITEMMENU_WORK * wk )
{
  if( GFL_UI_KEY_GetTrg() & MSG_SKIP_BTN ){
    KTST_SetDraw( wk, TRUE );
  }else if( GFL_UI_TP_GetTrg() ){
    KTST_SetDraw( wk, FALSE );
  }else{
    return;
  }

  wk->oamlistpos_old = 0xffff;
  _windowRewrite( wk );
  ITEMDISP_ListPlateClear( wk );
  GFL_CLACT_WK_SetAutoAnmFlag( wk->clwkScroll, TRUE );
  ITEMDISP_ChangeActive( wk, TRUE );
//  _CHANGE_STATE( wk, _itemKindSelectMenu );
  ChangeStateItemKindSelectItemMenu( wk );
}

//--------------------------------------------------------------------------------------------
/**
 * @brief   アイテム使用場所チェック
 */
//--------------------------------------------------------------------------------------------
static BOOL CheckEventItemUse( FIELD_ITEMMENU_WORK * wk, u8 pocket )
{
  // コロシアム・ユニオンルーム
  if( wk->mode == BAG_MODE_COLOSSEUM || wk->mode == BAG_MODE_UNION ){
    return FALSE;
  }
  // 裏フィールドで「たいせつなもの」ポケット
  if( GAMEDATA_GetIntrudeReverseArea( wk->gamedata ) == TRUE && pocket == BAG_POKE_EVENT ){
    return FALSE;
  }
  return TRUE;
}

static void _itemPocketChange( FIELD_ITEMMENU_WORK * wk )
{
/*
  // ポケットカーソル移動
  _pocketCursorChange( wk, wk->tmpCnt, wk->pocketno );
  // ソートボタン表示切替
  SORT_ModeReset( wk );
  BTN_DrawCheckBox( wk );
  wk->oamlistpos_old = 0xffff;
  _windowRewrite( wk );

  wk->tmpCnt = 0;
*/

  ChangeStateItemKindSelectItemMenu( wk );
}
