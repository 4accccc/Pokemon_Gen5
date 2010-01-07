//============================================================================================
/**
 * @file    itemmenu.h
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
#include "system/bmp_winframe.h"

#include "message.naix"
#include "bag.naix"
#include "msg/msg_d_field.h"
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

//============================================================================================
//============================================================================================

#define _1CHAR (8)

//=============================================================================
// 目次
//=============================================================================
//  __TECNIQUE  技マシン シーケンス
//  __TRASH     捨てる シーケンス
//  __SELL      売る シーケンス

//@TODO 一旦RepeatをContにしておく
static int _GFL_UI_KEY_GetRepeat( void )
{
  return GFL_UI_KEY_GetCont();
}

//=============================================================================
// データテーブル
//=============================================================================
//ならびは savedata/myitem_savedata.hに準拠
// コールバックは_BttnCallBack
static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右

#include "bag_btnpos.h"  //バッグの絵のタッチエリア

  { 21*_1CHAR,  24*_1CHAR,   0*_1CHAR,  3*_1CHAR },  //左
  { 21*_1CHAR,  24*_1CHAR,  15*_1CHAR, 18*_1CHAR },  //右

  { 21*_1CHAR,  24*_1CHAR,  19*_1CHAR, 22*_1CHAR },  //ソート
  { 21*_1CHAR,  24*_1CHAR,  22*_1CHAR, 24*_1CHAR },  //チェックボタン

  { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, _BAR_CELL_CURSOR_EXIT, _BAR_CELL_CURSOR_EXIT+TOUCHBAR_ICON_WIDTH-1 },  //x
  { TOUCHBAR_ICON_Y, TOUCHBAR_ICON_Y+TOUCHBAR_ICON_HEIGHT-1, _BAR_CELL_CURSOR_RETURN, _BAR_CELL_CURSOR_RETURN+TOUCHBAR_ICON_WIDTH-1 },  //リターン

  { 1*_1CHAR+4,   4*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア1
  { 4*_1CHAR+4,   7*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア2
  { 7*_1CHAR+4,  11*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア3
  {11*_1CHAR+4,  14*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア4
  {14*_1CHAR+4,  17*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア5
  {17*_1CHAR+4,  20*_1CHAR+3,  18*_1CHAR, 28*_1CHAR },  //アイテム一覧エリア6

  { 1*_1CHAR+4,   4*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア1
  { 4*_1CHAR+4,   7*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア2
  { 7*_1CHAR+4,  11*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア3
  {11*_1CHAR+4,  14*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア4
  {14*_1CHAR+4,  17*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア5
  {17*_1CHAR+4,  20*_1CHAR+3,  15*_1CHAR, 18*_1CHAR },  //だいじなものチェックエリア6

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
int ITEMMENU_GetItemPocketNumber(FIELD_ITEMMENU_WORK* pWork);
static void _ItemChangeSingle(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 );
static void _ItemChange(FIELD_ITEMMENU_WORK* pWork, int no1, int no2 );
static void _pocketCursorChange(FIELD_ITEMMENU_WORK* pWork,int oldpocket, int newpocket);
int ITEMMENU_GetItemIndex(FIELD_ITEMMENU_WORK* pWork);
static BOOL _posplus(FIELD_ITEMMENU_WORK* pWork, int length);
static BOOL _posminus(FIELD_ITEMMENU_WORK* pWork, int length);
static BOOL _itemScrollCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _keyChangeItemCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _keyMoveCheck(FIELD_ITEMMENU_WORK* pWork);
static BOOL _itemMovePositionTouchItem(FIELD_ITEMMENU_WORK* pWork);
static void _itemMovePosition(FIELD_ITEMMENU_WORK* pWork);
static void _itemInnerUse( FIELD_ITEMMENU_WORK* pWork );
static void _itemInnerUseWait( FIELD_ITEMMENU_WORK* pWork );
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
static void ITEM_Sub( FIELD_ITEMMENU_WORK* pWork, u8 sub_num );
static void InputNum_Start( FIELD_ITEMMENU_WORK* pWork, BAG_INPUT_MODE mode );
static void InputNum_Exit( FIELD_ITEMMENU_WORK* pWork );
static void InputNum_Proc( FIELD_ITEMMENU_WORK* pWork );
static void InputNum_ButtonState( FIELD_ITEMMENU_WORK* pWork, BOOL on_off );
static void SORT_Type( FIELD_ITEMMENU_WORK* pWork );
static void SORT_ABC( FIELD_ITEMMENU_WORK* pWork );
static u16 SORT_GetABCPrio( u16 item_no );
static void SORT_Button( FIELD_ITEMMENU_WORK* pWork );
static void SORT_ModeReset( FIELD_ITEMMENU_WORK* pWork );
static void SORT_Draw( FIELD_ITEMMENU_WORK* pWork );
static void KTST_SetDraw( FIELD_ITEMMENU_WORK* pWork, BOOL on_off );
static void SHORTCUT_SetEventItem( FIELD_ITEMMENU_WORK* pWork, int pos );
static void SHORTCUT_SetPocket( FIELD_ITEMMENU_WORK* pWork );
static void BTN_DrawCheckBox( FIELD_ITEMMENU_WORK* pWork );
//static void BAG_ItemUseErrorMsgSet( MYSTATUS * myst, STRBUF * buf, u16 item, u32 err, FIELD_ITEMMENU_WORK * pWork );
//u8 Bag_TalkMsgPrint( BAG_WORK * pWork, int type );
//static int Bag_MenuUse( FIELD_ITEMMENU_WORK * pWork );
static BOOL BAGMAIN_NPlanterUseCheck( u16 pocket, u16 item );
static void ItemMenuMake( FIELD_ITEMMENU_WORK * pWork, u8* tbl );
static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork);
int ITEMMENU_GetPosCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
BOOL ITEMMENU_AddCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
void ITEMMENU_RemoveCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no);
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work );
static void _VBlank( GFL_TCB *tcb, void *work );
static void _startState(FIELD_ITEMMENU_WORK* pWork);

#ifdef PM_DEBUG
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
  GFL_UI_KEY_SetRepeatSpeed(8, 15);  //元に戻す
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
  ITEMDISP_upMessageRewrite(pWork);
  ITEMDISP_WazaInfoWindowChange(pWork);
  ITEMDISP_CellMessagePrint(pWork);
  BTN_DrawCheckBox( pWork );
  pWork->bChange = TRUE;
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
  ITEMDISP_scrollCursorChangePos(pWork, ITEMMENU_GetItemIndex(pWork));
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
    //リストの終端まで来たのでカーソルが移動
    pWork->curpos++;
    bChange = TRUE;
  }

  return bChange;
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
static BOOL _posminus(FIELD_ITEMMENU_WORK* pWork, int length)
{
  BOOL bChange = FALSE;

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
  enum
  {
    Y_MAX = _SCROLL_BOTTOM_Y - _SCROLL_TOP_Y,
  };

  u32 x,y,i;

  if(GFL_UI_TP_GetPointCont(&x, &y) == TRUE)
  {
    // 範囲判定
    if((y <= _SCROLL_TOP_Y) || (y >= _SCROLL_BOTTOM_Y))
    {
      return FALSE;
    }

    if((x >= (32*8)) || (x <= (28*8)) )
    {
      return FALSE;
    }

    {
      int length = ITEMMENU_GetItemPocketNumber( pWork);
      int num = (length * (y-_SCROLL_TOP_Y)) / Y_MAX;
      int prelistpos = pWork->oamlistpos;

      pWork->curpos = 0;
      pWork->oamlistpos = -1;

      HOSAKA_Printf( "===== length=%d num=%d \n", length, num );

      for(i = 0 ; i < num ; i++)
      {
        _posplus(pWork, length);
        HOSAKA_Printf( "[%d] curpos=%d \n", i, pWork->curpos );
      }

      // リストが移動した時のみSE
      if( prelistpos != pWork->oamlistpos )
      {
        GFL_SOUND_PlaySE( SE_BAG_SRIDE );
      }
    }

    KTST_SetDraw( pWork, FALSE );
    return TRUE;
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

    if(_GFL_UI_KEY_GetRepeat()== PAD_KEY_DOWN){
      bChange = _posplus(pWork, length);
    }
    if(_GFL_UI_KEY_GetRepeat()== PAD_KEY_UP){
      bChange = _posminus(pWork, length);
    }
  }
  if(bChange){
    int newno = ITEMMENU_GetItemIndex(pWork);
     GFL_SOUND_PlaySE( SE_BAG_CURSOR_MOVE );
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

    if(_GFL_UI_KEY_GetRepeat()== PAD_KEY_DOWN){
      bChange = _posplus(pWork, length);
    }
    if(_GFL_UI_KEY_GetRepeat()== PAD_KEY_UP){
      bChange = _posminus(pWork, length);
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

static BOOL _itemMovePositionTouchItem(FIELD_ITEMMENU_WORK* pWork)
{
  int nowno = ITEMMENU_GetItemIndex(pWork);
  u32 x,y,i;
  int ymax = _SCROLL_BOTTOM_Y - _SCROLL_TOP_Y;

  if(GFL_UI_TP_GetPointCont(&x, &y) == TRUE){
    if((y <= _SCROLL_TOP_Y)  || (y >= _SCROLL_BOTTOM_Y)){
      return FALSE;
    }
    if((x >= (30*8)) || (x <= (18*8)) ){
      return FALSE;
    }
    {
      int length = ITEMMENU_GetItemPocketNumber( pWork);
      BOOL bChange = FALSE;
      int num = (6 * (y-_SCROLL_TOP_Y)) / ymax;  //カーソルを移動させたい量

      if(pWork->curpos == num){
        return FALSE;
      }
      else if(pWork->curpos < num){
        for(i = pWork->curpos ; i < num; i++){
          bChange += _posplus(pWork, length);
        }
      }
      else{
        for(i = pWork->curpos ; i > num; i--){
          bChange += _posminus(pWork, length);
        }
      }
    }
    if(0){
      int length = ITEMMENU_GetItemPocketNumber( pWork);
      int num = (length * (y-_SCROLL_TOP_Y)) / ymax;

      pWork->curpos = 0;
      pWork->oamlistpos = -1;
      for(i = 0 ; i < num ; i++){
        _posplus(pWork, length);
      }
    }
    {
      int newno = ITEMMENU_GetItemIndex(pWork);
      _ItemChange(pWork, nowno, newno);
    }
    KTST_SetDraw( pWork, FALSE );
    return TRUE;
  }
  return FALSE;
}

//------------------------------------------------------------------------------
/**
 * @brief   アイテム移動モード時のキーの処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemMovePosition(FIELD_ITEMMENU_WORK* pWork)
{
  BOOL bChange=FALSE;
  u32 trg = GFL_UI_KEY_GetTrg();

  GFL_UI_KEY_SetRepeatSpeed(1, 6);

  if((trg == PAD_BUTTON_A) || (trg == PAD_BUTTON_SELECT)){  //反映
    MYITEM_ITEM_STCopy(pWork->pMyItem, pWork->ScrollItem, pWork->pocketno, FALSE);
  }
  if((trg == PAD_BUTTON_B) || (trg == PAD_BUTTON_A) || (trg == PAD_BUTTON_SELECT) ){  //戻る
    pWork->moveMode = FALSE;
    _CHANGE_STATE(pWork,_itemKindSelectMenu);
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur , 1 );
    return;
  }

  if( _itemScrollCheck(pWork) ){   // スクロールバーの操作
    ITEMDISP_scrollCursorMove(pWork);

    // 負荷軽減のためセルメッセージだけ更新
//  ITEMDISP_CellMessagePrint(pWork);
  }
  else if(_itemMovePositionTouchItem(pWork)){ // アイテム移動モード時のアイテム部分タッチ処理
    ITEMDISP_scrollCursorChangePos(pWork, ITEMMENU_GetItemIndex(pWork));
    bChange = TRUE;
  }
  else if(_keyChangeItemCheck(pWork)){   // キーの操作
    ITEMDISP_scrollCursorChangePos(pWork, ITEMMENU_GetItemIndex(pWork));
    bChange = TRUE;
  }
  if(bChange){
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

//-----------------------------------------------------------------------------
/**
 *  @brief  バッグ内で使うアイテムか判定
 *
 *  @param  FIELD_ITEMMENU_WORK* pWork
 *
 *  @retval TURE:バッグ内で使うアイテム
 */
//-----------------------------------------------------------------------------
static BOOL BAG_IsInnerItem( u16 item_id )
{
  // スプレー
  if(  _check_spray( item_id ) )
  {
    return TRUE;
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

      // つかった
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_066, pWork->pStrBuf );
      WORDSET_RegisterPlayerName(pWork->WordSet, 0,  pWork->mystatus );
      WORDSET_RegisterItemName(pWork->WordSet, 1,  pWork->ret_item );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDisp( pWork );

      // @TODO スプレー音が見当たらないのでとりあえずテキトーな音
      GFL_SOUND_PlaySE( SE_BAG_DECIDE );
    }
    else
    {
      // つかえなかった
      GFL_MSG_GetString( pWork->MsgManager, msg_bag_067, pWork->pStrBuf );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      ITEMDISP_ItemInfoWindowDisp( pWork );
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
  if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ) || GFL_UI_TP_GetTrg() )
  {
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);

    _CHANGE_STATE(pWork,_itemKindSelectMenu);
  }
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
    OS_Printf("ret_code2 %d %d\n", pWork->ret_code2,selectno);

    if((BAG_MENU_TSUKAU==pWork->ret_code2)&&(pWork->pocketno==BAG_POKE_WAZA)){ //技マシン
      _CHANGE_STATE(pWork,_itemTecniqueUseInit);
    }
    else if((BAG_MENU_TSUKAU==pWork->ret_code2)&&( 1==ITEM_GetParam( pWork->ret_item, ITEM_PRM_EVOLUTION, pWork->heapID )  )){
      pWork->ret_code = BAG_NEXTPROC_EVOLUTION;  //進化
      _CHANGE_STATE(pWork,NULL);
    }
    else if((ITEM_ZITENSYA == pWork->ret_item) && (BAG_MENU_TSUKAU==pWork->ret_code2)){
      pWork->ret_code = BAG_NEXTPROC_RIDECYCLE;  //のる
      _CHANGE_STATE(pWork,NULL);
    }
    else if((ITEM_ZITENSYA == pWork->ret_item) && (BAG_MENU_ORIRU==pWork->ret_code2)){
      pWork->ret_code = BAG_NEXTPROC_DROPCYCLE;  //おりる
      _CHANGE_STATE(pWork,NULL);
    }
    else if(BAG_MENU_SUTERU==pWork->ret_code2){  //すてる
      _CHANGE_STATE(pWork,_itemTrash);
    }
    else if(BAG_MENU_YAMERU==pWork->ret_code2){  //やめる
      // タッチ遷移なら非表示に
      KTST_SetDraw( pWork, (GFL_UI_CheckTouchOrKey == GFL_APP_END_KEY) );
      _CHANGE_STATE(pWork, _itemKindSelectMenu);
    }
    else if( BAG_MENU_TOUROKU==pWork->ret_code2 || BAG_MENU_KAIZYO==pWork->ret_code2 ){  //とうろく
      SHORTCUT_SetEventItem( pWork, pWork->curpos );
      _CHANGE_STATE(pWork, _itemKindSelectMenu);
    }
    else if(BAG_MENU_KAIZYO==pWork->ret_code2){  //解除
      SHORTCUT_SetEventItem( pWork, pWork->curpos );
      _CHANGE_STATE(pWork, _itemKindSelectMenu);
    }
    else if(pWork->ret_item == ITEM_TAUNMAPPU){
      pWork->ret_code = BAG_NEXTPROC_TOWNMAP;  //タウンマップ
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_TOMODATITETYOU){
      pWork->ret_code = BAG_NEXTPROC_FRIENDNOTE;  //ともだち手帳
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_ANANUKENOHIMO){
      pWork->ret_code = BAG_NEXTPROC_ANANUKENOHIMO;  //あなぬけのヒモ
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_AMAIMITU){
      pWork->ret_code = BAG_NEXTPROC_AMAIMITU;  //あまいミツ
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_BORONOTURIZAO){
      pWork->ret_code = BAG_NEXTPROC_TURIZAO;  //つりざお
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_BATORUREKOODAA){
      pWork->ret_code = BAG_NEXTPROC_BATTLERECORDER;  //バトルレコーダー
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item == ITEM_PARESUHEGOO){
      pWork->ret_code = BAG_NEXTPROC_PALACEJUMP;  //@TODO 仮 パレスへゴー
      _CHANGE_STATE(pWork,NULL);
    }
    else if(pWork->ret_item>=ITEM_GURASUMEERU && pWork->ret_item<=ITEM_BURIKKUMEERU){
      pWork->ret_code = BAG_NEXTPROC_MAILVIEW;  //メール閲覧
      _CHANGE_STATE(pWork,NULL);
    }
    else if(BAG_MENU_TSUKAU==pWork->ret_code2){ //つかう
      // バッグ内で使うアイテム判定
      if( BAG_IsInnerItem( pWork->ret_item ) )
      {
        _CHANGE_STATE(pWork,_itemInnerUse); //メッセージウェイトして通常メニューにもどる
      }
      else
      {
        // 通常の「つかう」はWBではバッグを抜けた後に使う処理
        pWork->ret_code = BAG_NEXTPROC_ITEMUSE;
        _CHANGE_STATE(pWork,NULL);
      }
    }
    else if(BAG_MENU_URU==pWork->ret_code2){ //うる
      pWork->ret_code = BAG_NEXTPROC_RETURN;
      _CHANGE_STATE(pWork,NULL);
    }
    else{
      //@TODO アサーションかけるべき
      pWork->ret_code = BAG_NEXTPROC_HAVE;
      _CHANGE_STATE(pWork,NULL);
    }
    bClear = TRUE;
  }
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_B){
    bClear = TRUE;
    _CHANGE_STATE(pWork,_itemKindSelectMenu);
  }
  if(bClear){
    ITEMDISP_ListPlateClear( pWork );
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 0 );
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
  GFL_SOUND_PlaySE( SE_BAG_DECIDE );

  if(pWork->mode == BAG_MODE_POKELIST)
  {
    pWork->ret_code = BAG_NEXTPROC_ITEMEQUIP;
    _CHANGE_STATE(pWork,NULL);
  }
  else if( pWork->mode == BAG_MODE_SELL )
  {
    _CHANGE_STATE(pWork,_itemSellInit);
  }
  else
  {
    _itemUseWindowRewrite(pWork);
    ITEMDISP_ItemInfoMessageMake( pWork,pWork->ret_item );
    ITEMDISP_ItemInfoWindowDisp( pWork );
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
  BOOL bChange=FALSE;

  GFL_UI_KEY_SetRepeatSpeed(1, 6);

  GFL_BMN_Main( pWork->pButton );

  if(pWork->state == NULL){
    // 終了時はキー無効
    return;
  }

  // カーソルなしの状態から入力があった場合、カーソルを表示して抜ける
  if( GFL_UI_CheckTouchOrKey() == GFL_APP_END_TOUCH )
  {
    if( GFL_UI_KEY_GetTrg() )
    {
      GFL_SOUND_PlaySE( SE_BAG_CURSOR_MOVE );
      KTST_SetDraw( pWork, TRUE );
      return;
    }
  }

  // 決定
  if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE)
  {
    // アイテム存在チェック
    if( ITEMMENU_GetItemPocketNumber( pWork ) > 0 )
    {
      // アイテムに対する動作選択画面へ
      _CHANGE_STATE(pWork,_itemSelectState);
      return;
    }
  }
  // キャンセル
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL)
  {
    pWork->ret_code = BAG_NEXTPROC_RETURN;
    _CHANGE_STATE(pWork,NULL);
    return;
  }
  // 強制終了
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_X)
  {
    pWork->ret_code = BAG_NEXTPROC_EXIT;
    _CHANGE_STATE(pWork,NULL);
    return;
  }
  // 登録
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_Y)
  {
    if( pWork->pocketno == BAG_POKE_EVENT )
    {
      SHORTCUT_SetEventItem( pWork, pWork->curpos );
    }
    else
    {
      SHORTCUT_SetPocket( pWork );
    }
  }
  // 並び替え
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_SELECT)
  {
    GFL_STD_MemClear(pWork->ScrollItem, sizeof(pWork->ScrollItem));
    MYITEM_ITEM_STCopy(pWork->pMyItem, pWork->ScrollItem, pWork->pocketno, TRUE);  //取得
    pWork->moveMode = TRUE;
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkCur , 2 );

    _CHANGE_STATE(pWork,_itemMovePosition);
    return;
  }
  // タッチスクロール
  else if( _itemScrollCheck(pWork) )
  {
    ITEMDISP_scrollCursorMove(pWork);
    bChange = TRUE;
  }
  // キー操作
  else if( _keyMoveCheck(pWork) )
  {
    ITEMDISP_scrollCursorChangePos(pWork, ITEMMENU_GetItemIndex(pWork));
    bChange = TRUE;
  }

  {
    int oldpocket = pWork->pocketno;
    if(GFL_UI_KEY_GetTrg() == PAD_KEY_RIGHT)
    {
      GFL_SOUND_PlaySE( SE_BAG_POCKET_MOVE );
      pWork->pocketno++;
    }
    if(GFL_UI_KEY_GetTrg() == PAD_KEY_LEFT)
    {
      GFL_SOUND_PlaySE( SE_BAG_POCKET_MOVE );
      pWork->pocketno--;
    }
    if(pWork->pocketno >= BAG_POKE_MAX)
    {
      pWork->pocketno = 0;
    }
    if(pWork->pocketno < 0)
    {
      pWork->pocketno = BAG_POKE_MAX-1;
    }
    if(oldpocket != pWork->pocketno)
    {
      // ポケットカーソル移動
      _pocketCursorChange(pWork, oldpocket, pWork->pocketno);
      // ソートボタン表示切替
      SORT_ModeReset( pWork );
      BTN_DrawCheckBox( pWork );
      bChange = TRUE;
    }
  }

  if(bChange)
  {
    _windowRewrite(pWork);
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
      _CHANGE_STATE(pWork,_itemKindSelectMenu);
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

//------------------------------------------------------------------------------
/**
 * @brief   技マシン起動
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _itemTecniqueUseInit(FIELD_ITEMMENU_WORK* pWork)
{
  GFL_SOUND_PlaySE( SE_BAG_WAZA );

  GFL_MSG_GetString( pWork->MsgManager, msg_bag_065, pWork->pStrBuf );
  WORDSET_RegisterWazaName(pWork->WordSet, 0, ITEM_GetWazaNo( pWork->ret_item ));
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  ITEMDISP_ItemInfoWindowDisp( pWork );
  _CHANGE_STATE(pWork,_itemTecniqueUseWait);
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
  if(PAD_BUTTON_DECIDE == GFL_UI_KEY_GetTrg())
  {
    // 再描画
    GFL_BG_ClearScreen(GFL_BG_FRAME3_M);
    _windowRewrite(pWork);

    InputNum_ButtonState( pWork, TRUE );

    _CHANGE_STATE(pWork,_itemKindSelectMenu);
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
      GFL_SOUND_PlaySE( SE_BAG_TRASH );

      // アイテムを減らす
      ITEM_Sub( pWork, pWork->InputNum );

      GFL_MSG_GetString( pWork->MsgManager, msg_bag_055, pWork->pStrBuf );
      WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
      WORDSET_RegisterNumber(pWork->WordSet, 1, pWork->InputNum,
                             3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
      
      ITEMDISP_ItemInfoWindowDisp( pWork );

      _CHANGE_STATE(pWork,_itemTrashYesWait);
    }
    else{
      InputNum_ButtonState( pWork, TRUE );
      _CHANGE_STATE(pWork,_itemKindSelectMenu);
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

  InputNum_ButtonState( pWork, FALSE );

  // 数値入力開始
  InputNum_Start( pWork, BAG_INPUT_MODE_TRASH );

  GFL_MSG_GetString( pWork->MsgManager, msg_bag_054, pWork->pStrBuf );
  WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
  WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
  ITEMDISP_ItemInfoWindowDisp( pWork );

  //@TODO リストとBAR双方にOBJが使われているためバッティングする。OAMの半透明を切替る必要あり
//  G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 /*| GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1*/ | GX_BLEND_PLANEMASK_OBJ , -8 );

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
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 数値入力主処理
  InputNum_Proc( pWork );

  if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_DECIDE)
  {
    GFL_SOUND_PlaySE( SE_BAG_DECIDE );

    InputNum_Exit( pWork );

    GFL_MSG_GetString( pWork->MsgManager, msg_bag_056, pWork->pStrBuf );
    WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
    WORDSET_RegisterNumber(pWork->WordSet, 1, pWork->InputNum,
                           3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
    WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
    ITEMDISP_ItemInfoWindowDisp( pWork );

    _CHANGE_STATE(pWork,_itemTrashYesNo);
  }
  else if(GFL_UI_KEY_GetTrg() == PAD_BUTTON_CANCEL)
  {
    GFL_SOUND_PlaySE( SE_BAG_CANCEL );

    InputNum_Exit( pWork );

    InputNum_ButtonState( pWork, TRUE );

    //@TODO ここでパッシブ解除していいのか？
    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_OBJ , 0 );

    _CHANGE_STATE(pWork,_itemKindSelectMenu);
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
  InputNum_ButtonState( pWork, FALSE );

  // 買えないもの判定
  {
    s32 val;
    s32 event;

    val   = ITEM_GetParam( pWork->ret_item, ITEM_PRM_PRICE, pWork->heapID );
    event = ITEM_GetParam( pWork->ret_item, ITEM_PRM_EVENT, pWork->heapID );

    // 売値が0、もじくは重要フラグがONの時は売れない
    if( val == 0 || event )
    {
      // 「○○○を　かいとることは　できません」
      GFL_MSG_GetString( pWork->MsgManager, mes_shop_093, pWork->pStrBuf );
      WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
      WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf );
      ITEMDISP_ItemInfoWindowDisp( pWork );

      _CHANGE_STATE( pWork, _itemSellEndMsgWait );
      return;
    }
  }

  pWork->InputNum = 1;  //初期化

  // おこづかい表示開始
  ITEMDISP_GoldDispIn( pWork );

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
  WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
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
  if(!ITEMDISP_MessageEndCheck(pWork)){
    return;
  }

  // 売るシーケンス入力処理
  InputNum_Proc( pWork );

  // @TODO バーアイコンなどタッチ対応も含めるのでラップする
  // 強制終了

   // 決定
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    GFL_SOUND_PlaySE( SE_BAG_DECIDE );

    // 数値入力終了
    InputNum_Exit( pWork );

    _CHANGE_STATE( pWork, _itemSellYesnoInit );
  }
  // キャンセル
  else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL )
  {
    GFL_SOUND_PlaySE( SE_BAG_CANCEL );

    // 数値入力終了
    InputNum_Exit( pWork );

    _CHANGE_STATE( pWork, _itemSellExit );
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
                             6, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
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

          // 所持金増加
          Mystatus_AddGold( pWork->mystatus, val );

          // 売却音
          GFL_SOUND_PlaySE( SE_BAG_SELL );

          // タスクバーおこづかい表示しなおし
          ITEMDISP_GoldDispWrite( pWork );

          // 再描画
          _windowRewrite(pWork);

          // 「○○○を　わたして XXXXXX円 うけとった」
          GFL_MSG_GetString( pWork->MsgManager, mes_shop_096, pWork->pStrBuf );
          WORDSET_RegisterItemName(pWork->WordSet, 0,  pWork->ret_item );
          WORDSET_RegisterNumber(pWork->WordSet, 1, val, 6, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT);
          WORDSET_ExpandStr( pWork->WordSet, pWork->pExpStrBuf, pWork->pStrBuf  );
          ITEMDISP_ItemInfoWindowDisp( pWork );

          _CHANGE_STATE( pWork, _itemSellEndMsgWait );
        }
        break;

      case 1 :
        // NO
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
  if( ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE ) || GFL_UI_TP_GetTrg() )
  {
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

  InputNum_ButtonState( pWork, TRUE );

  _CHANGE_STATE( pWork, _itemKindSelectMenu );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  捨てる処理  アイテムを削除
 *	@retval none
 */
//-----------------------------------------------------------------------------
static void ITEM_Sub( FIELD_ITEMMENU_WORK* pWork, u8 sub_num )
{
  // 手持ちからアイテム削除
  MYITEM_SubItem(pWork->pMyItem, pWork->ret_item, sub_num, pWork->heapID );

  // カーソルが指す座標を下げる
  pWork->curpos = MATH_IMax( 0, pWork->curpos-1 );
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
static void InputNum_Proc( FIELD_ITEMMENU_WORK* pWork )
{
  ITEM_ST * item = ITEMMENU_GetItem( pWork,ITEMMENU_GetItemIndex(pWork) );
  int backup = pWork->InputNum;
  int tp_result;

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

  // タッチ入力
  tp_result = GFL_UI_TP_HitCont( tp_tbl );

  if( tp_result != GFL_UI_TP_HIT_NONE )
  {
    pWork->countTouch++;
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
      break;

    default : GF_ASSERT(0);

    }
  }

  // キー入力
  if( tp_result == GFL_UI_TP_HIT_NONE )
  {
    if(_GFL_UI_KEY_GetRepeat() == PAD_KEY_UP){
      pWork->InputNum++;
    }
    else if(_GFL_UI_KEY_GetRepeat() == PAD_KEY_DOWN){
      pWork->InputNum--;
    }
    else if(_GFL_UI_KEY_GetRepeat() == PAD_KEY_RIGHT){
      pWork->InputNum += 10;
    }
    else if(_GFL_UI_KEY_GetRepeat() == PAD_KEY_LEFT){
      pWork->InputNum -= 10;
    }
  }

  // カンストチェック
  if(item->no < pWork->InputNum){
    pWork->InputNum = item->no;
  }
  else if(pWork->InputNum < 1){
    pWork->InputNum = 1;
  }

  // 表示更新
  if(pWork->InputNum != backup)
  {
    // 移動音
    GFL_SOUND_PlaySE( SE_BAG_CURSOR_MOVE );

    // ボタンアニメ
    if( pWork->InputNum < backup )
    {
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_INPUT_D ], 10 );
    }
    else
    {
      GFL_CLACT_WK_SetAnmSeq( pWork->clwkBarIcon[ BAR_ICON_INPUT_U ], 11 );
    }

    ITEMDISP_InputNumDisp(pWork,pWork->InputNum);
  }
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

#include "item/itemtype_def.h"
//--------------------------------------------------------------
/// 種類別ソート用ワーク
//==============================================================
typedef struct {
  ITEM_ST st;
  s32 type;
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
static s32 QSort_Type( void* elem1, void* elem2 )
{
  ITEM_SORTDATA_TYPE* p1 = (ITEM_SORTDATA_TYPE*)elem1;
  ITEM_SORTDATA_TYPE* p2 = (ITEM_SORTDATA_TYPE*)elem2;

  if( p1->type == p2->type )
  {
    // 種類が一致した場合は純粋なID比較
    if( p1->st.id == p2->st.id )
    {
      return 0;
    }
    else if( p1->st.id > p2->st.id )
    {
      return 1;
    }
    else
    {
      return -1;
    }
  }
  else if( p1->type > p2->type )
  {
    return 1;
  }
  else
  {
    return -1;
  }
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
  ITEM_SORTDATA_TYPE sort[ BAG_MYITEM_MAX ];
  ITEM_ST item[ BAG_MYITEM_MAX ];

  //取得
  MYITEM_ITEM_STCopy(pWork->pMyItem, item, pWork->pocketno, TRUE);
  length = ITEMMENU_GetItemPocketNumber( pWork );

  HOSAKA_Printf("種別ソート length=%d \n", length );

  // ソート用データ生成
  for( i=0; i<length; i++ )
  {
    sort[i].type  = ITEM_GetParam( item[i].id, ITEM_PRM_ITEM_TYPE, pWork->heapID );

    GFL_STD_MemCopy(
        &item[i],
        &sort[i].st,
        sizeof(ITEM_ST) );
  }

#ifdef PM_DEBUG
  {
    OS_TPrintf("=============================\n");
    OS_TPrintf(" SORT BEFORE \n");
    OS_TPrintf("=============================\n");

    for( i=0; i<length; i++ )
    {
      OS_TPrintf("[%d] id=%d \n",i, item[i].id );
    }
  }
#endif

  MATH_QSort( (void*)sort, length, sizeof(ITEM_SORTDATA_TYPE), QSort_Type, NULL );

#ifdef PM_DEBUG
  {
    OS_TPrintf("=============================\n");
    OS_TPrintf(" SORT AFTER \n");
    OS_TPrintf("=============================\n");

    for( i=0; i<length; i++ )
    {
      OS_TPrintf("[%d] id=%d \n",i, sort[i].st.id );
    }
  }
#endif

  // セーブ用データに整形
  for( i=0; i<length; i++ )
  {
    GFL_STD_MemCopy(
        &sort[i].st,
        &item[i],
        sizeof(ITEM_ST) );
  }

  // 保存
  MYITEM_ITEM_STCopy(pWork->pMyItem, item, pWork->pocketno, FALSE);

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

#include "itemsort_abc_def.h"
//-----------------------------------------------------------------------------
/**
 *  @brief  50音ソート用プライオリティを取得
 *
 *  @param  u16 item_no
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static u16 SORT_GetABCPrio( u16 item_no )
{
  GF_ASSERT( item_no-1 < ITEM_DATA_MAX && item_no != ITEM_DUMMY_DATA );

  // itemsort_abc_def.h で定義
  return ItemSortByAbc[ item_no-1 ];
}

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
  // ワザマシンは処理なし
  if( pWork->pocketno == BAG_POKE_WAZA )
  {
    return;
  }

  if( pWork->sort_mode == 0 )
  {
    SORT_ABC( pWork );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 2 );
  }
  else if( pWork->sort_mode == 1 )
  {
    SORT_Type( pWork );
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 3 );
  }

  // 大切なものはABCソートのみ
  if( pWork->pocketno != BAG_POKE_EVENT )
  {
    // モードチェンジ
    pWork->sort_mode ^= 1;
  }

  // 再描画
  _windowRewrite( pWork );

  GFL_SOUND_PlaySE( SE_BAG_SORT );
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
  pWork->sort_mode = 0;

  HOSAKA_Printf("pocketno:%d\n", pWork->pocketno);

  // ワザマシンはソート不可能
  if( pWork->pocketno == BAG_POKE_WAZA )
  {
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 4 );
  }
  else
  {
    GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 0 );
  }
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
static void SORT_Draw( FIELD_ITEMMENU_WORK* pWork )
{
  if( GFL_CLACT_WK_CheckAnmActive( pWork->clwkSort ) == FALSE )
  {
    // yellow
    if( pWork->sort_mode == 1 )
    {
        GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 1 );
    }
    // green
    else
    {
        GFL_CLACT_WK_SetAnmSeq( pWork->clwkSort , 0 );
    }
  }
}

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

  // カーソルを消す
  GFL_CLACT_WK_SetDrawEnable( pWork->clwkCur, on_off );

  // 上画面を消す
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , on_off );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 , on_off );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2 , on_off );
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
static void SHORTCUT_SetEventItem( FIELD_ITEMMENU_WORK* pWork, int pos )
{
  // 表示上の位置から実体の位置へ座標変換
  pos += pWork->oamlistpos + 1;

  if(pWork->pocketno == BAG_POKE_EVENT){
    int length = ITEMMENU_GetItemPocketNumber( pWork );
    ITEM_ST * item = ITEMMENU_GetItem( pWork, pos );
    SHORTCUT_ID shortcut_id = SHORTCUT_DATA_GetItemToShortcutID( item->id );

    if(length <= pos){
      return;
    }

    // 登録可能かチェック
    {
      void * itemdata;
      int ret;
      itemdata = ITEM_GetItemArcData( item->id, ITEM_GET_DATA, pWork->heapID );
      ret = ITEM_GetBufParam( itemdata, ITEM_PRM_CNV );
      GFL_HEAP_FreeMemory( itemdata );
      if( ret == 0 ){
        return;
      }
    }

    if(ITEMMENU_GetPosCnvButtonItem(pWork,item->id)!=-1){//チェックつき
      GAMEDATA_SetShortCut( pWork->gamedata, shortcut_id, FALSE );
      ITEMMENU_RemoveCnvButtonItem(pWork,item->id);
    }
    else{
      GAMEDATA_SetShortCut( pWork->gamedata, shortcut_id, TRUE );
      ITEMMENU_AddCnvButtonItem(pWork,item->id);
    }

    GFL_SOUND_PlaySE( SE_BAG_REGIST_Y ); // 登録音

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
  SHORTCUT_ID id;

  switch( pocketno )
  {
  case BAG_POKE_NORMAL:
    id = SHORTCUT_ID_BAG_ITEM;
    break;
  case BAG_POKE_NUTS:
    id = SHORTCUT_ID_BAG_NUTS;
    break;
  case BAG_POKE_DRUG:
    id = SHORTCUT_ID_BAG_RECOVERY;
    break;
  case BAG_POKE_WAZA:
    id = SHORTCUT_ID_BAG_WAZAMACHINE;
    break;
  case BAG_POKE_EVENT:
    id = SHORTCUT_ID_BAG_IMPORTANT;
    break;
  default : GF_ASSERT(0);
  }

  return id;
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

  GFL_SOUND_PlaySE( SE_BAG_REGIST_Y );

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
        if( item->id == ITEM_ZITENSYA && pWork->cycle_flg == 1 )
        {
          tbl[BAG_MENU_USE] = BAG_MENU_ORIRU;
        }
        else if( item->id == ITEM_POFINKEESU )
        {
          tbl[BAG_MENU_USE] = BAG_MENU_HIRAKU;
        }
        //      else if( pWork->ret_item == ITEM_gbPUREIYAA && Snd_GameBoyFlagCheck() == 1 )
        //    {
        //    tbl[BAG_MENU_USE] = BAG_MENU_TOMERU;
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
      if( ITEMMENU_GetPosCnvButtonItem( pWork, item->id  ) != -1 ){
        tbl[BAG_MENU_SUB] = BAG_MENU_KAIZYO;
      }else{
        tbl[BAG_MENU_SUB] = BAG_MENU_TOUROKU;
      }
    }
  }
  // 木の実プランター
  else if( pWork->mode == BAG_MODE_N_PLANTER ){
    if( BAGMAIN_NPlanterUseCheck( pocket, item->id ) == TRUE ){
      tbl[BAG_MENU_USE] = BAG_MENU_TSUKAU_NP;
    }
  }

  // やめる
  tbl[BAG_MENU_CANCEL] = BAG_MENU_YAMERU;

  OS_Printf("pocket=%d\n",   pocket);
  OS_Printf("tbl[BAG_MENU_USE]=%d\n",   tbl[BAG_MENU_USE]);
  OS_Printf("tbl[BAG_MENU_GIVE]=%d\n",  tbl[BAG_MENU_GIVE]);
  OS_Printf("tbl[BAG_MENU_SUB]=%d\n",   tbl[BAG_MENU_SUB]);
  OS_Printf("tbl[BAG_MENU_MOVE]=%d\n",   tbl[BAG_MENU_ITEMMOVE]);

#if 0
  for(i=0;i<BAG_MENUTBL_MAX;i++){
    if(tbl[i]!=255){
      pWork->menu_func[i] = MenuFuncTbl[tbl[i]];
    }
    OS_Printf("menu_func[%d]=%08x\n", i,pWork->menu_func[i]);
  }
#endif

  GFL_HEAP_FreeMemory( itemdata );
}

#endif


static void _itemUseWindowRewrite(FIELD_ITEMMENU_WORK* pWork)
{
  BMPMENULIST_HEADER list_h = _itemMenuListHeader;
  int length,i,j;


  length = BAG_MENUTBL_MAX;


  {
    u8  tbl[BAG_MENUTBL_MAX]={255, 255, 255, 255, 255};
    int strtbl[]={msg_bag_001,msg_bag_007,msg_bag_017,mes_bag_104,mes_bag_105,
      msg_bag_002,msg_bag_003,msg_bag_019,msg_bag_004,msg_bag_005,msg_bag_006,
      msg_bag_009, msg_bag_094, mes_shop_103,msg_bag_001,msg_bag_menu_ex_01};
    int stringbuff[BAG_MENUTBL_MAX];

    ItemMenuMake(pWork, tbl);

    for(i=0,j=0; i< BAG_MENUTBL_MAX ; i++){
      if(tbl[i]==255){
        continue;
      }
      pWork->submenuList[j]=tbl[i];
      stringbuff[j] = strtbl[tbl[i]];
      j++;
    }
    ITEMDISP_MenuWinDisp(pWork,stringbuff,j);


  }
}


//----------------------------------------------------------------------------
/**
 *  @brief  便利登録されていたら順番を返す
 */
//-----------------------------------------------------------------------------

int ITEMMENU_GetPosCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  int i;
  for(i = 0 ; i < DUMMY_SHORTCUT_MAX ; i++){
    if(no == MYITEM_CnvButtonItemGet( pWork->pMyItem, i)){
      return i;
    }
  }
  return -1;
}

//----------------------------------------------------------------------------
/**
 *  @brief  チェックを追加
 */
//-----------------------------------------------------------------------------

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

//----------------------------------------------------------------------------
/**
 *  @brief  チェックを消す
 */
//-----------------------------------------------------------------------------

void ITEMMENU_RemoveCnvButtonItem(FIELD_ITEMMENU_WORK* pWork, int no)
{
  int i=ITEMMENU_GetPosCnvButtonItem(pWork,no);

  if(i!=-1){
    MYITEM_CnvButtonItemSet( pWork->pMyItem, i, 0);
  }
}



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


  if(BAG_POKE_MAX > bttnid){
    int id2no[]={1,2,3,4,0};
    pocketno = id2no[bttnid];  //どうぐのあたりが広い為、順番を最後にしている
    KTST_SetDraw( pWork, FALSE );
  }
  else if(BUTTONID_LEFT == bttnid){
    pocketno = pWork->pocketno;
    pocketno--;
    if(pocketno < 0){
      pocketno = BAG_POKE_MAX-1;
    }
  }
  else if(BUTTONID_RIGHT == bttnid){
    pocketno = pWork->pocketno;
    pocketno++;
    if(pocketno >= BAG_POKE_MAX){
      pocketno = 0;
    }
  }
  else if(BUTTONID_SORT == bttnid){
    // ソートボタン
    SORT_Button( pWork );
  }
  else if(BUTTONID_CHECKBOX == bttnid){
    SHORTCUT_SetPocket( pWork );
  }
  else if(BUTTONID_EXIT == bttnid){
    pWork->ret_code = BAG_NEXTPROC_EXIT;
    _CHANGE_STATE(pWork, NULL);
  }
  else if(BUTTONID_RETURN == bttnid){
    pWork->ret_code = BAG_NEXTPROC_RETURN;
    _CHANGE_STATE(pWork, NULL);
  }
  else if((bttnid >= BUTTONID_ITEM_AREA) && (bttnid < BUTTONID_CHECK_AREA)){
    // アイテム選択シーケンスでの操作
    if(_itemKindSelectMenu == pWork->state){
      int backup = pWork->curpos;
      int nowno = ITEMMENU_GetItemIndex(pWork);

      // アイテムがなかった場合
      if( ITEMMENU_GetItemPocketNumber( pWork ) == 0 )
      {
        // 処理なし
        return;
      }

      if(pWork->curpos != (bttnid-BUTTONID_ITEM_AREA)){
        pWork->curpos = bttnid - BUTTONID_ITEM_AREA;
        if(length <= ITEMMENU_GetItemIndex(pWork)){
          pWork->curpos = backup;
          return;
        }
      }

      _ItemChange(pWork, nowno, ITEMMENU_GetItemIndex(pWork));
      //      ITEMDISP_scrollCursorChangePos(pWork, ITEMMENU_GetItemIndex(pWork));
      _windowRewrite(pWork);

      KTST_SetDraw( pWork, FALSE );

      // 上画面だけは表示
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ , TRUE );
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1 , TRUE );
      GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG2 , TRUE );

      _CHANGE_STATE(pWork,_itemSelectState);
    }
    return;
  }
  // チェックボックス
  else if(bttnid >= BUTTONID_CHECK_AREA){
    int no = bttnid - BUTTONID_CHECK_AREA;
    SHORTCUT_SetEventItem( pWork, no );
  }

  if(pocketno != -1){
    // ポケットカーソル移動
    _pocketCursorChange(pWork, pWork->pocketno, pocketno);
    pWork->pocketno = pocketno;
    // ソートボタン表示切替
    SORT_ModeReset( pWork );
    _windowRewrite(pWork);

    KTST_SetDraw( pWork, FALSE );
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

  GFL_CLACT_SYS_VBlankFunc(); //セルアクターVBlank
  if(pWork->bChange){
    ITEMDISP_CellVramTrans(pWork);
    pWork->bChange = FALSE;
  }
}


//-----------------------------------------------------------------------------
/**
 *  @brief
 *
 *  @param  pWork
 *
 *  @retval
 */
//-----------------------------------------------------------------------------
static void _startState(FIELD_ITEMMENU_WORK* pWork)
{
  ITEMDISP_SetVisible();

  // 初期化
  KTST_SetDraw( pWork ,GFL_UI_CheckTouchOrKey() == GFL_APP_END_KEY );

  if (GFL_FADE_CheckFade() == FALSE) {
    _CHANGE_STATE(pWork, _itemKindSelectMenu);
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

  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, 0);

  // ヒープ生成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_ITEMMENU, 0x28000 );
  pWork = GFL_PROC_AllocWork( proc, sizeof(FIELD_ITEMMENU_WORK), HEAPID_ITEMMENU );
  GFL_STD_MemClear( pWork, sizeof(FIELD_ITEMMENU_WORK) );

  //-------------------------------------
  // メンバ初期化
  //-------------------------------------
  pWork->heapID = HEAPID_ITEMMENU;

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

// @TODO 保坂のみデバッグ押しながら起動でショップから起動したことにする
#ifdef DEBUG_ONLY_FOR_genya_hosaka
  if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_DEBUG) )
  {
    pWork->mode = BAG_MODE_SELL;
    HOSAKA_Printf("hook sell\n");
  }
#endif

//  pWork->pBagCursor = GAMEDATA_GetBagCursor(GAMESYSTEM_GetGameData(pWork->gsys));
//  pWork->pMyItem = GAMEDATA_GetMyItem(GAMESYSTEM_GetGameData(pWork->gsys));

  // その他
  pWork->pocketno = MYITEM_FieldBagPocketGet(pWork->pBagCursor);

  {
    s16 cur,scr;
    MYITEM_FieldBagCursorGet(pWork->pBagCursor, pWork->pocketno, &cur, &scr );
    pWork->curpos = cur;
    pWork->oamlistpos = scr - 1;
  }

  // グラフィック初期化
  ITEMDISP_graphicInit(pWork);

  // 文字列初期化
  pWork->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                      NARC_message_bag_dat, pWork->heapID );

  pWork->MsgManagerItemInfo = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                              NARC_message_iteminfo_dat, pWork->heapID );

  pWork->MsgManagerPocket = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE,
                                            NARC_message_itempocket_dat, pWork->heapID );

  pWork->pStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
  pWork->pExpStrBuf = GFL_STR_CreateBuffer(200,pWork->heapID);
  pWork->WordSet    = WORDSET_Create( pWork->heapID );
  pWork->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr ,
                                       GFL_FONT_LOADTYPE_FILE , FALSE , pWork->heapID );

  ITEMDISP_upMessageCreate(pWork);
  ITEMDISP_CellResourceCreate(pWork);
  ITEMDISP_InitPocketCell( pWork );

  _windowCreate(pWork);
  ITEMDISP_CellCreate(pWork);

  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );

  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , FLD_SUBSCR_FADE_DIV , FLD_SUBSCR_FADE_SYNC , pWork->heapID );

  pWork->g3dVintr = GFUser_VIntr_CreateTCB( _VBlank, (void*)pWork, 0 );

  pWork->pMsgTcblSys = GFL_TCBL_Init( pWork->heapID , pWork->heapID , 1 , 0 );

  //  pWork->pTouchSWSys = TOUCH_SW_AllocWork(pWork->heapID);

  ITEMDISP_BarMessageCreate( pWork );

  pWork->pAppTaskRes  = APP_TASKMENU_RES_Create( GFL_BG_FRAME3_M, _SUBLIST_NORMAL_PAL,pWork->fontHandle, pWork->SysMsgQue, pWork->heapID  );

  // ネットワーク初期化
  GFL_NET_WirelessIconEasy_HoldLCD( TRUE, pWork->heapID );
  GFL_NET_ChangeIconPosition(256-16, 0);
  GFL_NET_ReloadIcon();

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

  if(state == NULL){
    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 16, 0);
    return GFL_PROC_RES_FINISH;
  }
  if( WIPE_SYS_EndCheck() != TRUE ){
    return GFL_PROC_RES_CONTINUE;
  }

  state(pWork);
  _dispMain(pWork);
  SORT_Draw(pWork);

  PRINTSYS_QUE_Main(pWork->SysMsgQue);

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

  if (GFL_FADE_CheckFade() == TRUE) {
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
  GFL_MSG_Delete(pWork->MsgManagerItemInfo);
  GFL_MSG_Delete(pWork->MsgManagerPocket);

  GFL_STR_DeleteBuffer(pWork->pStrBuf);
  GFL_STR_DeleteBuffer(pWork->pExpStrBuf);
  WORDSET_Delete(pWork->WordSet);

  ITEMDISP_BarMessageDelete( pWork );

  if(pWork->itemInfoDispWin){

    GFL_BG_FreeCharacterArea(GFL_BG_FRAME3_M,
                             GFL_ARCUTIL_TRANSINFO_GetPos(pWork->bgchar),
                             GFL_ARCUTIL_TRANSINFO_GetSize(pWork->bgchar));

    GFL_BMPWIN_Delete(pWork->itemInfoDispWin);
  }

  GFL_BG_FillCharacterRelease(GFL_BG_FRAME3_M,1,0);

  GFL_FONT_Delete(pWork->fontHandle);
  PRINTSYS_QUE_Clear(pWork->SysMsgQue);
  PRINTSYS_QUE_Delete(pWork->SysMsgQue);
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
