//////////////////////////////////////////////////////////////////////
/**
 *
 * @brief  スクリプトコマンド：リゾートショップ関連
 * @file   scrcmd_resort_shop.h
 * @author Toru Nagihashi （scrcmd_shopを改造して作成）
 * @date   2011.03.03
 *
 */
//////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "system/vm_cmd.h"
#include "../scrcmd.h"
#include "system/main.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menu.h"
#include "sound/pm_sndsys.h"
#include "item/item.h"    // ITEM_GetIndex
#include "item_icon.naix"
#include "wifi_unionobj.naix"
#include "message.naix"
#include "message.naix"
#include "script_message.naix"
#include "gamesystem/g_power.h"  //  GPOWER_Calc_Sale

#include "gamesystem/game_data.h"
#include "gamesystem/msgspeed.h"
#include "savedata/bsubway_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/record.h"
#include "item/itemsym.h"
#include "../script_local.h"
#include "app/p_status.h"
#include "../event_fieldmap_control.h"
#include "scrcmd_resort_shop.h"
#include "../fieldmap.h"
#include "../script_def.h"
#include "../field_camera.h"
#include "field/field_msgbg.h"
#include "msg/script/msg_resort_scr.h"

#include "scrcmd_resort_def.h"
#include "arc/shop_gra.naix"
#include "msg/script/msg_resort_shop.h"

#include "savedata/resonance_resort_sv.h"
#include "resort_people.h"
#include "resort_binary.h"
#include "resort_reception.h"
#include "resort_field.h"
#include "../gmk_tmp_wk.h"
#include "savedata/resonance_resort_sv.h"
#include "wifi_unionobj_plt.cdat" //for sc_wifi_unionobj_plt

// BMPWIN定義テーブル

typedef struct {
  u16 id;       // インデックス
  u16 serviceID;
  u16 param1;
  u16 param2;
  u32 price;    // 値段
  STRBUF *p_name; //名前
  STRBUF *p_info; //情報
  void *p_data_adrs;
}RESORT_SHOP_ITEM;

// 一度に並べることができる商品数
#define RESORT_SHOP_ITEM_MAX   ( 40 )

// BMP登録用情報構造体
typedef struct {
  int frame;
  u8  x,y,w,h;
  u16 pal,chara;
}BMPWIN_DAT;
#include "../wbshopbmp.h"    // Window0BmpDataTable

#ifdef PM_DEBUG
#define SHOP_DEBUG_COMMAND  // LRでお金が手に入る。SELECTでバッジ８つのショップになる
#endif

//====================================================================
// ■定数宣言
//====================================================================

#define SHOP_BUY_OBJ_RES_NUM  ( 3 )
#define SHOP_YEN_KETA_MAX     ( 7 ) 

// BMPWIN
enum{
  SHOP_BUY_BMPWIN_OKODUKAI=0,
  SHOP_BUY_BMPWIN_MONEY,
  SHOP_BUY_BMPWIN_LIST,
  SHOP_BUY_BMPWIN_NUM,
  SHOP_BUY_BMPWIN_PRICE,
  SHOP_BUY_BMPWIN_ITEMINFO,
  SHOP_BUY_BMPWIN_TALKMSG,
  SHOP_BUY_BMPWIN_MAX,
};

// OBJ用
enum{
  SHOP_OBJ_CURSOR=0,
  SHOP_OBJ_ARROW_UP,
  SHOP_OBJ_ARROW_DOWN,
  SHOP_OBJ_ARROW_UPDOWN,
  SHOP_OBJ_ITEM,
  SHOP_OBJ_CURSOR_RED,
  SHOP_OBJ_MAX,
};

// メニューウインドウ枠のパレットを転送する位置
#define MENU_WINDOW_PAL_OFFSET    (   9 )
#define MENU_WINDOW_CHARA_OFFSET  ( 228 )

// 所持金用バッファの長さ
#define SHOP_MYGOLD_STR_MAX   ( 10 )

// 道具説明バッファの長さ
#define RESORT_SHOP_ITEMNUM_STR_MAX   ( 20 )

// 道具説明バッファの長さ
#define SHOP_STRING_MAX   ( 200 )

// スクリーンクリア関数で使用する定義
#define SCREEN_SUBMENU_AND_INFOWIN  ( 0 )
#define SCREEN_SUBMENU_ONLY         ( 1 )


#ifdef BUGFIX_AF_SYSTEM92_20110208
// アイテム説明を消去する際、必ず無効なアイテムIDと判断されるアイテムID
#define RESORT_SHOP_ITEMID_NONE   (0xffff)
#endif
//====================================================================
// ■構造体宣言１
//====================================================================

// ショップ買うアプリ制御用構造体
typedef struct _SHOP_BUY_APP_WORK{
  u8      seq;     // 買うアプリシーケンス制御用
  u8      next;    // 次のシーケンス予約
  u16     wait;    // ウェイト
  HEAPID  heapId;  // ヒープID
  GFL_FONT *font;  // 描画用のフォント
  
  MISC      *misc;    // 所持金にアクセスするため
  MYITEM    *myitem;  // 手持ちの道具アクセス
  RECORD    *record;  // レコードデータ
  
  RESORT_SHOP_ITEM lineup[RESORT_SHOP_ITEM_MAX];  // ショップで購入できるデータを格納するワーク
  u16       lineup_num;             // 購入できる商品の数
  u8        type;                   // ショップのタイプ（通常ショップorBPショップor技ショップ)
  u8        dummy;
  
  GFL_BMPWIN        *win[SHOP_BUY_BMPWIN_MAX];  // BMPWIN
  BMP_MENULIST_DATA *list;            // 買い物リストデータ
  BMPMENU_WORK      *yesnoWork;       // はい・いいえ画面ワーク
  GFL_MSGDATA       *shopMsgData;     // ショップ画面MSGDATA
  WORDSET           *wordSet;         // 文字列展開ワーク
  STRBUF            *priceStr;        // 値段文字列
  STRBUF            *expandBuf;       // expand展開用
  STRBUF            *HaveStr;         // 「ーーー」
  BMPMENULIST_WORK  *menuList;        // BMPMENULIST
  PRINT_UTIL         printUtil;       // 
  PRINT_QUE         *printQue;        // 
  GFL_TCBLSYS       *pMsgTcblSys;     // メッセージ表示用タスクSYS
  PRINT_STREAM      *printStream;     // 
  
  GFL_CLUNIT        *ClactUnit;                // セルアクターユニット
  GFL_CLWK*         ClactWork[SHOP_OBJ_MAX];   // OBJワークポインタ
  u32               clact_res[3][SHOP_BUY_OBJ_RES_NUM]; // OBJリソースハンドル

  ARCHANDLE         *iconHandle;    // アイテムアイコンハンドル

  const RESORT_SHOP_ITEM  *cp_select;           //選択したもの
  u16               FirstTrans;       // リストBMPWINの転送をおこなっているか？

  BOOL (*ShopCallFunc)( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );

  RESORT_BINARY_SHOP      *p_shop;
  const RESORT_BINARY_SHOP_DATA *cp_shop_data;
  RESORT_INSTITUTION_DATA *p_institution;
  RESORT_PEOPLE_ALL_DATA  *p_all_people;

  u16*  ret_service_type;
  u16*  ret_service_param;
  u16*  ret_service_idx;
  u16*  ret_price;

  //以下、モード別処理
  void (*List_Init)( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );
  void (*List_Exit)( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );
  void (*List_Change)( struct _SHOP_BUY_APP_WORK *wk, int type, int id ); //sortでしか使わない
  void (*List_Decide)( struct _SHOP_BUY_APP_WORK *wk, int type, int id ); //sortでしか使わない
  BMPMENULIST_CURSOR_CALLBACK call_back;
  BMPMENULIST_PRINT_CALLBACK icon;

  u16 msg_start;
  u16 msg_select;
  u16 msg_decide;

  //入れ替えにのみ使用
  u8 insert_idx;
  u8 now_idx;
  BOOL is_sort;
  u16 insert_list;
  u16 insert_cursor;
  BOOL is_change;

} SHOP_BUY_APP_WORK;

// ショップ買う起動時構造体
typedef struct {
  u8  seq;       // シーケンス制御ワーク
  u8  wait;      // ウェイト用
  u8  shopType;  // ショップタイプ(RESORT_CALL_LIST_TYPE_ITEM,  etc..)
  u8  shopParam;    // ショップの品物ID（shop_data.cdat参照。SHOP_TYOE_FIXの際に使用する)
  int camera_flag_push; // カメラの範囲制限フラグをPUSHするためのワーク
  SHOP_BUY_APP_WORK wk; // ショップ画面ワーク
  
} SHOP_BUY_CALL_WORK;



//====================================================================
// ■プロトタイプ宣言
//====================================================================

static BOOL EvShopBuyWait( VMHANDLE *core, void *wk );
static BOOL ShopCallFunc_Buy( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id );
static BOOL ShopCallFunc_Select( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id );
static BOOL ShopCallFunc_Sort( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id );
static int  init_work( SHOP_BUY_APP_WORK *wk, int type );
static void exit_work( SHOP_BUY_APP_WORK *wk );
static void shop_item_set( SHOP_BUY_APP_WORK *wk, int type, int id );
static void shop_secretary_set( SHOP_BUY_APP_WORK *wk, int type, int id );
static void shop_institution_set( SHOP_BUY_APP_WORK *wk, int type, int id );
static void shop_item_release( SHOP_BUY_APP_WORK *wk );
static void bg_trans( SHOP_BUY_APP_WORK *wk );
static void bg_clear( SHOP_BUY_APP_WORK *wk );
static void obj_init( SHOP_BUY_APP_WORK *wk );
static void obj_load_item( SHOP_BUY_APP_WORK *wk );
static void obj_load_people( SHOP_BUY_APP_WORK *wk );
static void obj_del( SHOP_BUY_APP_WORK *wk );
static void itemicon_resource_change( SHOP_BUY_APP_WORK *wk, u16 itemno );
static void people_resource_change( SHOP_BUY_APP_WORK *wk, u16 trainer_view );
static void bmpwin_init( SHOP_BUY_APP_WORK *wk );
static void print_money( SHOP_BUY_APP_WORK *wk );
static void bmpwin_exit( SHOP_BUY_APP_WORK *wk );
static void print_mygold( SHOP_BUY_APP_WORK *wk );
static void print_carry_item( SHOP_BUY_APP_WORK *wk, u16 itemno );
static void print_iteminfo( SHOP_BUY_APP_WORK *wk, const STRBUF *cp_info );
static void bmpwin_list_init( SHOP_BUY_APP_WORK *wk, BMPMENULIST_CURSOR_CALLBACK call_back, BMPMENULIST_PRINT_CALLBACK icon );
static void bmpwin_list_exit( SHOP_BUY_APP_WORK *wk );
static void line_callback(BMPMENULIST_WORK * wk, u32 param, u8 y );
static void line_callback_rest(BMPMENULIST_WORK * wk, u32 param, u8 y );
static void ShopPrintMsgItem( SHOP_BUY_APP_WORK *wk, int strId, u16 itemno );
static void ShopPrintMsg( SHOP_BUY_APP_WORK *wk, int strId );
static void ShopDecideMsg( SHOP_BUY_APP_WORK *wk, int strId, const RESORT_SHOP_ITEM  *cp_select );
static void submenu_screen_clear( int type );
static void item_shop_main( SHOP_BUY_APP_WORK *wk );
static void select_shop_main( SHOP_BUY_APP_WORK *wk );
static void sort_shop_main( SHOP_BUY_APP_WORK *wk );
static  int shop_buy_printwait( SHOP_BUY_APP_WORK *wk );
static void string_alloc( SHOP_BUY_APP_WORK *wk );  

static RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap );
static RESORT_BINARY_SHOP * Resort_Scrcmd_GetBinaryShop( FIELDMAP_WORK *p_fieldmap );

static void ItemList_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode);
static void SelectPoeple_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode);
static void SortPoeple_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode);
static void SortPoeple_Cursor_SetPos( SHOP_BUY_APP_WORK *wk );

//アイテム・サービスリストの場合
static void ItemList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );
static void ItemList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );

//秘書リストの場合
static void SecretaryList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );
static void SecretaryList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );

//施設リストの場合
static void InstitutionList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );
static void InstitutionList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id );

//入れ替え
static void SecretaryList_Change( SHOP_BUY_APP_WORK *wk, int type, int id );
static void InstitutionList_Change( SHOP_BUY_APP_WORK *wk, int type, int id );
static void SecretaryList_Decide( SHOP_BUY_APP_WORK *wk, int type, int id );
static void InstitutionList_Decide( SHOP_BUY_APP_WORK *wk, int type, int id );

//--------------------------------------------------------------------
/**
 * @brief ショップ購入イベント呼び出し 
 *
 * @param core 仮想マシン制御構造体へのポインタ
 * @param wk   SCRCMD_WORKへのポインタ
 *
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------------
VMCMD_RESULT EvCmdResortCallList( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work     = (SCRCMD_WORK*)wk;
  u16            shop_type  = SCRCMD_GetVMWorkValue( core, work );
  u16            shop_param  = SCRCMD_GetVMWorkValue( core, work );
  u16*           ret_service_type   = SCRCMD_GetVMWork( core, work );
  u16*           ret_service_param = SCRCMD_GetVMWork( core, work );
  u16*           ret_service_idx = SCRCMD_GetVMWork( core, work );
  u16*           ret_price        = SCRCMD_GetVMWork( core, work );
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gamedata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SCRIPT_WORK*   sc       = SCRCMD_WORK_GetScriptWork( work );
  void** scr_subproc_work = SCRIPT_SetSubProcWorkPointerAdrs( sc );
  FLDMSGBG *     fldmsg   = FIELDMAP_GetFldMsgBG( fieldmap );

  // ショップ用ワーク確保
  SHOP_BUY_CALL_WORK* sbw = GFL_HEAP_AllocClearMemory(HEAPID_PROC,sizeof(SHOP_BUY_CALL_WORK));

  *scr_subproc_work = sbw;
  sbw->wk.font      = FLDMSGBG_GetFontHandle( fldmsg );
  sbw->wk.misc      = GAMEDATA_GetMiscWork( gamedata );
  sbw->wk.heapId    = HEAPID_FIELDMAP;
  sbw->wk.myitem    = GAMEDATA_GetMyItem( gamedata );
  sbw->wk.record    = GAMEDATA_GetRecordPtr( gamedata );
  sbw->wk.ret_service_type = ret_service_type;
  sbw->wk.ret_service_param = ret_service_param;
  sbw->wk.ret_service_idx = ret_service_idx;
  sbw->wk.ret_price = ret_price;

  {
    RESORT_PEOPLE *p_people = Resort_Scrcmd_GetFrontPeople( fieldmap );
    u16 shop = 0;
    u16 type = 0; //TODO
    u16 lv = 0;
    
    GF_ASSERT( RESORT_PEOPLE_GetPeopleData( p_people, RESORT_PEOPLE_DATA_PARAM_TYPE ) == RESORT_PEOPLE_TYPE_RESIDENT );
    sbw->wk.p_institution = RESORT_RESIDENT_DATA_GetInstitution( RESORT_PEOPLE_GetDataAdrs( p_people) );

    shop = RESORT_PEOPLE_GetData( p_people, RESORT_DATA_PARAM_TYPE, NULL );
    lv = RESORT_INSTITUTION_DATA_GetData( sbw->wk.p_institution, RESORT_INSTITUTION_DATA_PARAM_LV );

    sbw->wk.p_shop    = Resort_Scrcmd_GetBinaryShop( fieldmap );
    sbw->wk.cp_shop_data = RESORT_BINARY_SHOP_SearchData( sbw->wk.p_shop, shop, type, lv );
  }

  {
    RESONANCE_RESORT_SV *p_resort_sv = RESONANCE_RESORT_SV_GetWork( GAMEDATA_GetSaveControlWork( gamedata) );
    sbw->wk.p_all_people = RESONANCE_RESORT_SV_GetPeopleAllData( p_resort_sv );
  }

  // スクリプトからのパラメータで分岐
  // ここで関数分岐等も行い、実際の処理に分岐を挟まないようにしている
  switch(shop_type){
  default:
    GF_ASSERT(0);
  case RESORT_LIST_TYPE_BUY:    //販売
    GAMEBEACON_Set_Shoping();
    sbw->shopType = shop_type;
    sbw->shopParam   = 0;    //人物から取得する
    sbw->wk.ShopCallFunc  = ShopCallFunc_Buy;
    sbw->wk.List_Init = ItemList_Init;
    sbw->wk.List_Exit = ItemList_Exit;
    sbw->wk.call_back = ItemList_move_callback;
    sbw->wk.icon = line_callback_rest;
    break;
  case RESORT_LIST_TYPE_SELECT:    //選択
    sbw->shopType = shop_type;
    sbw->shopParam = shop_param;
    sbw->wk.ShopCallFunc  = ShopCallFunc_Select;
    switch( shop_param )
    {
    default:
      GF_ASSERT(0);
    case RESORT_LIST_SELECT_SECRETARY_ADD:
      sbw->wk.List_Init = SecretaryList_Init;
      sbw->wk.List_Exit = SecretaryList_Exit;
      sbw->wk.msg_select  = mes_shop_09_09;
      sbw->wk.msg_decide  = mes_shop_09_10;
      sbw->wk.call_back = SelectPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    case RESORT_LIST_SELECT_SECRETARY_DEL:
      sbw->wk.List_Init = SecretaryList_Init;
      sbw->wk.List_Exit = SecretaryList_Exit;
      sbw->wk.msg_select  = mes_shop_09_06;
      sbw->wk.msg_decide  = mes_shop_09_07;
      sbw->wk.call_back = SelectPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    case RESORT_LIST_SELECT_INSTATITUTION:
      sbw->wk.List_Init = InstitutionList_Init;
      sbw->wk.List_Exit = InstitutionList_Exit;
      sbw->wk.msg_select  = mes_shop_09_12;
      sbw->wk.msg_decide  = mes_shop_09_13;
      sbw->wk.call_back = SelectPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    case RESORT_LIST_SELECT_INSTATITUTION_DEL:
      sbw->wk.List_Init = InstitutionList_Init;
      sbw->wk.List_Exit = InstitutionList_Exit;
      sbw->wk.msg_select  = mes_shop_09_15;
      sbw->wk.msg_decide  = mes_shop_09_16;
      sbw->wk.call_back = SelectPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    }
    break;
  case RESORT_LIST_TYPE_SORT:       //ソート
    sbw->shopType = shop_type;
    sbw->shopParam = shop_param;
    sbw->wk.ShopCallFunc  = ShopCallFunc_Sort;
    switch( shop_param )
    {
    case RESORT_LIST_SORT_SECRETARY:
      sbw->wk.List_Init = SecretaryList_Init;
      sbw->wk.List_Exit = SecretaryList_Exit;
      sbw->wk.List_Change = SecretaryList_Change;
      sbw->wk.List_Decide = SecretaryList_Decide;
      sbw->wk.msg_start   = mes_shop_09_17;
      sbw->wk.msg_select  = mes_shop_09_18;
      sbw->wk.msg_decide  = mes_shop_09_19;
      sbw->wk.call_back = SortPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    case RESORT_LIST_SORT_INSTATITUTION:
      sbw->wk.List_Init = InstitutionList_Init;
      sbw->wk.List_Exit = InstitutionList_Exit;
      sbw->wk.List_Change = InstitutionList_Change;
      sbw->wk.List_Decide = InstitutionList_Decide;
      sbw->wk.msg_start   = mes_shop_09_17;
      sbw->wk.msg_select  = mes_shop_09_18;
      sbw->wk.msg_decide  = mes_shop_09_19;
      sbw->wk.call_back = SortPoeple_move_callback;
      sbw->wk.icon = NULL;
      break;
    }
    break;
  } 
  // ショップ呼び出し
  VMCMD_SetWait( core, EvShopBuyWait );

  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------------
/**
 * @brief カメラ移動処理開始設定
 *
 * @param   sbw         SHOP_BUY_CALL_WORK
 * @param   fieldmap    フィールドマップワーク
 * @param   fieldcamera カメラ設定
 */
//----------------------------------------------------------------------------------
static void Start_CameraMove( SHOP_BUY_CALL_WORK *sbw, FIELDMAP_WORK* fieldmap, FIELD_CAMERA *fieldcamera )
{
  // ショップで買い物を開始するときのカメラ移動のオフセットデータ
  static const VecFx32 shopcamera_move_tbl[4]={
    {  FX32_ONE*16*4,  -FX32_ONE*8*0,   -FX32_ONE*16*1  },   // 上
    {  FX32_ONE*16*4,  -FX32_ONE*8*4,   0  },   // 下
    {  FX32_ONE*16*4,  -FX32_ONE*8*0,   0  },   // 左
    {  FX32_ONE*16*5,  -FX32_ONE*8*2,   0  },   // 右
  };

  FLD_CAM_MV_PARAM param;
  FIELD_PLAYER *fld_player;
  int dir;

  // カメラ移動制限処理を解除（ただし解除前の設定を保存しておく）
  sbw->camera_flag_push = FIELD_CAMERA_GetCameraAreaActive( fieldcamera );
  FIELD_CAMERA_SetCameraAreaActive( fieldcamera, FALSE );
  FIELD_CAMERA_SetRecvCamParamOv( fieldcamera );

  // 主人公の向きを取得
  fld_player = FIELDMAP_GetFieldPlayer( fieldmap );
  dir        = FIELD_PLAYER_GetDir( fld_player );  
  
  // カメラ現在位置を取得
  FIELD_CAMERA_GetCameraPos( fieldcamera, &param.Core.CamPos);
  FIELD_CAMERA_GetTargetPos( fieldcamera, &param.Core.TrgtPos );
  FIELD_CAMERA_FreeTarget(fieldcamera);

  // 向きに合わせて移動オフセット値の足しこみ
  param.Core.TrgtPos.x += shopcamera_move_tbl[dir].x;
  param.Core.TrgtPos.y += shopcamera_move_tbl[dir].y;
  param.Core.TrgtPos.z += shopcamera_move_tbl[dir].z;

  param.Chk.Shift = FALSE;
  param.Chk.Pitch = FALSE;
  param.Chk.Yaw   = FALSE;
  param.Chk.Dist  = FALSE;
  param.Chk.Fovy  = FALSE;
  param.Chk.Pos   = TRUE;

  // カメラ移動開始
  FIELD_CAMERA_SetLinerParamOv( fieldcamera, &param, 10 );

}
//----------------------------------------------------------------------------------
/**
 * @brief ショップ買うサブシーケンス
 *
 * @param   core  SCRCMD_WORK
 * @param   wk    SHOP_BUY_CALL_WORK
 *
 * @retval  BOOL  終了:TRUE   待ち:FALSE
 */
//----------------------------------------------------------------------------------
static BOOL EvShopBuyWait( VMHANDLE *core, void *wk )
{
  enum{
    SHOP_BUY_CALL_CAMERATRACE_STOP=0,
    SHOP_BUY_CALL_CAMERATRACE_STOP_WAIT,
    SHOP_BUY_CALL_CAMERA_MOVE,
    SHOP_BUY_CALL_CAMERA_MOVE_WAIT,
    SHOP_BUY_CALL,
    SHOP_BUY_CALL_CAMERA_RETURN,
    SHOP_BUY_CALL_CAMERA_RETURN_WAIT,
    SHOP_BUY_CALL_END,
  };

  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
  void **scr_subproc_work = SCRIPT_SetSubProcWorkPointerAdrs( sc );
  SHOP_BUY_CALL_WORK *sbw = *scr_subproc_work;
  GAMESYS_WORK*     gsys = SCRCMD_WORK_GetGameSysWork( work );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_CAMERA *fieldcamera = FIELDMAP_GetFieldCamera( fieldmap );

  // ショップ呼び出しイベントシーケンス
  switch(sbw->seq)
  {
    case SHOP_BUY_CALL_CAMERATRACE_STOP:  // カメラ移動停止（念のため）
      FIELD_CAMERA_StopTraceRequest( fieldcamera );
      sbw->seq++;
      break;
    case SHOP_BUY_CALL_CAMERATRACE_STOP_WAIT:
      if(FIELD_CAMERA_CheckTrace(fieldcamera)==FALSE){
        sbw->seq++;
      }
      break;
    case SHOP_BUY_CALL_CAMERA_MOVE:
      OS_Printf("ショップ起動\n");
      // カメラ移動開始
      Start_CameraMove( sbw, fieldmap, fieldcamera );   
      sbw->seq++;
      break;
    case SHOP_BUY_CALL_CAMERA_MOVE_WAIT:
      // カメラ移動終了待ち
      if(!FIELD_CAMERA_CheckMvFunc(fieldcamera)){
        sbw->seq++;
        sbw->wk.seq = 0;
        FLDMSGBG_SetBlendAlpha( FALSE );  // メッセージウインドウ半透明OFF
      }
      break;
    case SHOP_BUY_CALL:
      // ショップメイン処理
      if(sbw->wk.ShopCallFunc( gsys, &sbw->wk, sbw->shopType, sbw->shopParam  )){
        sbw->seq++;
      }
      break;
    case SHOP_BUY_CALL_CAMERA_RETURN:
      // 移動カメラ戻す処理
      {
        FLD_CAM_MV_PARAM_CHK chk = {FALSE, FALSE,FALSE,FALSE,FALSE,TRUE,};
        FIELD_CAMERA_RecvLinerParam(fieldcamera, &chk, 8);
      }
      sbw->seq++;
      sbw->wait=0;
      break;
    case SHOP_BUY_CALL_CAMERA_RETURN_WAIT:
      // カメラ移動終了待ち
      if(!FIELD_CAMERA_CheckMvFunc(fieldcamera)){
        FIELD_CAMERA_BindDefaultTarget(fieldcamera);
        FIELD_CAMERA_SetCameraAreaActive( fieldcamera, sbw->camera_flag_push );
        FIELD_CAMERA_RestartTrace( fieldcamera );
        FIELD_CAMERA_ClearRecvCamParam(fieldcamera);
        FLDMSGBG_SetBlendAlpha( TRUE );  // メッセージウインドウ半透明ON
        sbw->seq++;
      }
      break;
    // イベント終了
    case SHOP_BUY_CALL_END:
      GFL_HEAP_FreeMemory(*scr_subproc_work);
      return TRUE;
      break;
  }
  
  return FALSE;
}

// ここまでが呼び出し部。

//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------
//----------------------------------------------------------------------------------

// はい・いいえ画面描画処理
static const BMPWIN_YESNO_DAT yesno_data= {
  GFL_BG_FRAME1_M,
  24, 13, 
  11, 1+24
};

// ショップ買う画面遷移用定義
enum{
  SHOPBUY_SEQ_INIT=0,
  SHOPBUY_SEQ_MAIN,
  SHOPBUY_SEQ_SELECT,
  SHOPBUY_SEQ_BACK,
  SHOPBUY_SEQ_ONLY1_YESNO,
  SHOPBUY_SEQ_YESNO,
  SHOPBUY_SEQ_SELECT_YESNO_WAIT,
  SHOPBUY_SEQ_BUY,
  SHOPBUY_SEQ_MSG_WAIT,
  SHOPBUY_SEQ_END,
};

enum{
  SHOPSELECT_SEQ_INIT=0,
  SHOPSELECT_SEQ_MAIN,
  SHOPSELECT_SEQ_SELECT,
  SHOPSELECT_SEQ_BACK,
  SHOPSELECT_SEQ_ONLY1_YESNO,
  SHOPSELECT_SEQ_YESNO,
  SHOPSELECT_SEQ_SELECT_YESNO_WAIT,
  SHOPSELECT_SEQ_BUY,
  SHOPSELECT_SEQ_MSG_WAIT,
  SHOPSELECT_SEQ_END,
};


enum{
  SHOPSORT_SEQ_INIT=0,
  SHOPSORT_SEQ_INITWAIT,
  SHOPSORT_SEQ_MAIN,
  SHOPSORT_SEQ_SELECT,
  SHOPSORT_SEQ_BACK,
  SHOPSORT_SEQ_RESET,
  SHOPSORT_SEQ_ONLY1_YESNO,
  SHOPSORT_SEQ_YESNO,
  SHOPSORT_SEQ_SELECT_YESNO_WAIT,
  SHOPSORT_SEQ_MSG_WAIT,
  SHOPSORT_SEQ_END,
};

// テキストカラー定義
#define WHITE_TEXT_W_COL  (PRINTSYS_LSB_Make( 15,  2, 15))
#define BLACK_TEXT_W_COL  (PRINTSYS_LSB_Make(  1,  2, 15))
#define WHITE_TEXT_B_COL  (PRINTSYS_LSB_Make( 15,  2,  0))
#define BLUE_TEXT_COL     (PRINTSYS_LSB_Make( 12, 13,  0))


//----------------------------------------------------------------------------------
/**
 * @brief ショップ画面初期化
 *
 * @param   gsys  GAMESYS_WORK(セーブデータ引継ぎのため）
 * @param   wk    SHOP_BUY_APP_WORK
 * @param   type  ショップタイプ RESORT_LIST_TYPE_xxx
 * @param   id    上記タイプごとの値
 */
//----------------------------------------------------------------------------------
static void shop_call_init( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id )
{
  GAMEDATA *gamedata = GAMESYSTEM_GetGameData(gsys);
  MYSTATUS *my       = GAMEDATA_GetMyStatus(gamedata);

  // ワーク初期化
  type     = init_work( wk, type );

  string_alloc( wk );
  bmpwin_init( wk );  // BMPWIN初期化
  obj_init( wk );     // OAM初期化
}

//----------------------------------------------------------------------------------
/**
 * @brief どうぐ所持の最大数を返す
 *
 * @param   item  どうぐNO
 *
 * @retval  int   アイテムナンバー
 */
//----------------------------------------------------------------------------------
static int get_item_max( int item )
{
  // 技マシンは99個までしか持てない
//  if(item>=ITEM_WAZAMASIN01 && item < ITEM_HIDENMASIN01){
  if( ITEM_CheckWazaMachine( item ) == TRUE ){
    return 99;
  }
  
  return 999;
}

//----------------------------------------------------------------------------------
/**
 * @brief 値段表示用文字列確保
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void string_alloc( SHOP_BUY_APP_WORK *wk )
{
  // ○○○円
  wk->priceStr = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_02_07 );
  // 「ーーー」（購入済のマーク）
  wk->HaveStr    = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_02_07_02 );
}

//----------------------------------------------------------------------------------
/**
 * @brief 現在の持っている通貨（お金・BP)を取得する
 *
 * @param   wk    
 *
 * @retval  int   どっちかの通貨
 */
//----------------------------------------------------------------------------------
static int _get_gold( SHOP_BUY_APP_WORK *wk )
{
  return MISC_GetGold(wk->misc);
}

//----------------------------------------------------------------------------------
/**
 * @brief 道具選択時に何個買えるかで処理を分岐させる
 *        (持ちきれない・買えません・複数個選択へ・１つしか買えないのですぐYESNOへ)
 *
 * @param   wk    SHOP_BUY_APP_WORK
 *
 * @retval  none
 */
//----------------------------------------------------------------------------------
static void can_player_buy_item( SHOP_BUY_APP_WORK *wk )
{
  int gold = _get_gold(wk);

  //売り切れです
  if( RESORT_INSTITUTION_DATA_IsBit(wk->p_institution, wk->cp_select->id) == TRUE )
  {
    ShopPrintMsg( wk, mes_shop_02_07_04 );
    wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
    wk->next = SHOPBUY_SEQ_BACK;
  }
  // もちきれないですよ！
  else if(wk->cp_select->serviceID == RESORT_SERVICE_TYPE_ITEM
      && MYITEM_AddCheck( wk->myitem, wk->cp_select->param1, wk->cp_select->param2, wk->heapId )==FALSE)
  {
    ShopPrintMsg( wk, mes_shop_02_05 );
    wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
    wk->next = SHOPBUY_SEQ_BACK;

  // 「買えませんよ！」
  }else if(gold<wk->cp_select->price)
  {
    // お金が足りない
    ShopPrintMsg( wk, mes_shop_02_01 );
    wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
    wk->next = SHOPBUY_SEQ_BACK;

  // 一つだけ買えるのでそのまま「はい・いいえ」
  }else
  {
    ShopDecideMsg(wk,  mes_shop_02_03, wk->cp_select );
    wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
    wk->next = SHOPBUY_SEQ_ONLY1_YESNO;
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 買う画面どうぐ選択時メイン処理
 *
 * @param   wk    SHOP_BUY_APP_WORK
 *
 * @retval  none
 */
//----------------------------------------------------------------------------------
static void item_shop_main( SHOP_BUY_APP_WORK *wk )
{
  u32 ret;
  // printQueに処理が残っている場合はメインを通さない
  if(PRINTSYS_QUE_IsFinished( wk->printQue )==FALSE){
    return;
  }

  // BMPの転送待ちの場合があるので、BmpwinOnのタイミングをズラした
  if(wk->FirstTrans==0){
    GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_LIST] );
    wk->FirstTrans++;
  }

  // リスト処理メイン
  ret = BmpMenuList_Main( wk->menuList );
  if(ret!=BMPMENULIST_NULL)
  {
    if(ret==BMPMENULIST_CANCEL)
    {
      wk->cp_select = NULL;
      wk->seq = SHOPBUY_SEQ_END;
    } else {
      wk->cp_select = &wk->lineup[ret];
      // 購入できるか判定（買えない・一個買える・複数買える）
      can_player_buy_item( wk );
    }
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 買う画面どうぐ選択時メイン処理
 *
 * @param   wk    SHOP_BUY_APP_WORK
 *
 * @retval  none
 */
//----------------------------------------------------------------------------------
static void select_shop_main( SHOP_BUY_APP_WORK *wk )
{
  u32 ret;
  // printQueに処理が残っている場合はメインを通さない
  if(PRINTSYS_QUE_IsFinished( wk->printQue )==FALSE){
    return;
  }

  // BMPの転送待ちの場合があるので、BmpwinOnのタイミングをズラした
  if(wk->FirstTrans==0){
    GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_LIST] );
    wk->FirstTrans++;
  }

  // リスト処理メイン
  ret = BmpMenuList_Main( wk->menuList );
  if(ret!=BMPMENULIST_NULL)
  {
    if(ret==BMPMENULIST_CANCEL)
    {
      wk->cp_select = NULL;
      wk->seq = SHOPSELECT_SEQ_END;
    } else {
      wk->cp_select = &wk->lineup[ret];
      ShopDecideMsg(wk,  wk->msg_select, wk->cp_select );
      wk->seq  = SHOPSELECT_SEQ_MSG_WAIT;
      wk->next = SHOPSELECT_SEQ_ONLY1_YESNO;
    }
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief   インサート処理
 *
 * @param   wk    SHOP_BUY_APP_WORK
 *
 * @retval  none
 */
//----------------------------------------------------------------------------------
static void sort_shop_main( SHOP_BUY_APP_WORK *wk )
{
  u32 ret;
  // printQueに処理が残っている場合はメインを通さない
  if(PRINTSYS_QUE_IsFinished( wk->printQue )==FALSE){
    return;
  }

  // BMPの転送待ちの場合があるので、BmpwinOnのタイミングをズラした
  if(wk->FirstTrans==0){
    GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_LIST] );
    wk->FirstTrans++;
  }

  // リスト処理メイン
  ret = BmpMenuList_Main( wk->menuList );
  if(ret!=BMPMENULIST_NULL)
  {
    if(ret==BMPMENULIST_CANCEL)
    {
      if( wk->is_sort == FALSE )
      {
        wk->cp_select = NULL;
        wk->seq = SHOPSORT_SEQ_END;
      }
      else
      {
        wk->is_sort = FALSE;
        GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_CURSOR_RED], FALSE );
      }
    }
    else 
    {
      if( wk->is_sort )
      {
        wk->now_idx = ret;
        if( wk->now_idx != wk->insert_idx )
        {
          //ソート処理
          wk->is_change = TRUE;
#if 0 //はいいいいえをしないことになったが仕様変更があるかもしれないので残しておく
          ShopDecideMsg(wk,  wk->msg_select, wk->cp_select );
          wk->seq  = SHOPSORT_SEQ_MSG_WAIT;
          wk->next = SHOPSORT_SEQ_ONLY1_YESNO;  
#else
          wk->seq  = SHOPSORT_SEQ_RESET;
#endif
        }
      }
      else
      {
        wk->insert_idx = ret;
        wk->is_sort = TRUE;
        wk->cp_select = &wk->lineup[ret];
        BmpMenuList_PosGet( wk->menuList, &wk->insert_list, &wk->insert_cursor );
        SortPoeple_Cursor_SetPos( wk );
        GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_CURSOR_RED], TRUE );
      }
    }
  }

}

//----------------------------------------------------------------------------------
/**
 * @brief 文字列表示待ち処理
 *
 * @param   printStream   
 *
 * @retval  int   次のシーケンスを指定
 */
//----------------------------------------------------------------------------------
static int shop_buy_printwait( SHOP_BUY_APP_WORK *wk )
{
  PRINTSTREAM_STATE state = PRINTSYS_PrintStreamGetState( wk->printStream );

  if( state == PRINTSTREAM_STATE_DONE )
  {
    PRINTSYS_PrintStreamDelete( wk->printStream );
    return wk->next;
  }else if( state == PRINTSTREAM_STATE_RUNNING)
  {
    if(GFL_UI_KEY_GetCont()&PAD_BUTTON_DECIDE || GFL_UI_KEY_GetCont()&PAD_BUTTON_CANCEL)
    {
      PRINTSYS_PrintStreamShortWait( wk->printStream, 0 );
    }
  }else if( state == PRINTSTREAM_STATE_PAUSE )
  {
    if(GFL_UI_KEY_GetTrg()&PAD_BUTTON_DECIDE || GFL_UI_KEY_GetTrg()&PAD_BUTTON_CANCEL){
      PRINTSYS_PrintStreamReleasePause( wk->printStream );
      PMSND_PlaySE( SEQ_SE_MESSAGE );
    }
  }
  
  return wk->seq;
}

//----------------------------------------------------------------------------------
/**
 * @brief 支払方法で引く対象を替える
 *
 * @param   wk    
 * @param   pay   支払う金額（お金かBP,,個数を掛け算済）
 */
//----------------------------------------------------------------------------------
static void _sub_payment( SHOP_BUY_APP_WORK *wk, int pay )
{
  // お金
  MISC_SubGold( wk->misc , pay);
  // レコード：買い物した合計金額
  RECORD_Add( wk->record, RECID_SHOPPING_MONEY, pay);

  // レコード：買った回数
  RECORD_Inc( wk->record, RECID_SHOPPING_CNT );
}
//----------------------------------------------------------------------------------
/**
 * @brief ショップ買うメインルーチン
 *
 * @param   gsys  GAMESYS_WORK
 * @param   wk    SHOP_BUY_APP_WORK
 * @param   type  ショップタイプ(SCR_SHOPID_NULL or SHOP_TYPE_FIX)
 * @param   id    固定ショップの際のID（shop_data.cdat参照)
 *
 * @retval  BOOL  継続:FALSE  終了:TRUE
 */
//----------------------------------------------------------------------------------
static BOOL ShopCallFunc_Buy( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id )
{
  u32 ret;
  
  switch( wk->seq ){
  case SHOPBUY_SEQ_INIT:  // 初期化
    shop_call_init(gsys, wk, type, id);

    bg_trans( wk );     // BG転送
    print_money(wk);  //おこづかい表示
    print_mygold(wk);   // 所持金表示
    wk->List_Init( gsys, wk, type, id );
    wk->seq++;
    break;
  case SHOPBUY_SEQ_MAIN:
    item_shop_main(wk);
    break;
  case SHOPBUY_SEQ_BACK:    // 戻る
    submenu_screen_clear( SCREEN_SUBMENU_AND_INFOWIN );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_ITEMINFO] );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    BmpMenuList_Rewrite( wk->menuList );
    wk->seq = SHOPBUY_SEQ_MAIN;
    break;
  case SHOPBUY_SEQ_ONLY1_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );

    // 技マシンじゃなかったら現在の所持数を表示
    if( wk->cp_select->serviceID == RESORT_SERVICE_TYPE_ITEM )
    {
      print_carry_item( wk, wk->cp_select->param1 );
    }
    wk->seq = SHOPBUY_SEQ_SELECT_YESNO_WAIT;

    break;

  case SHOPBUY_SEQ_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );
    wk->seq = SHOPBUY_SEQ_SELECT_YESNO_WAIT;
    break;
  case SHOPBUY_SEQ_SELECT_YESNO_WAIT:
    ret = BmpMenu_YesNoSelectMain( wk->yesnoWork );
    if(ret!=BMPMENU_NULL){
      if(ret==0){
        wk->seq = SHOPBUY_SEQ_BUY;
        submenu_screen_clear( SCREEN_SUBMENU_ONLY );
        BmpMenuList_Rewrite( wk->menuList );
        GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
        GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
      }else if(ret==BMPMENU_CANCEL){
        wk->seq = SHOPBUY_SEQ_BACK;
      }
    }
    break;
  case SHOPBUY_SEQ_BUY:
    //アイテムの場合は、購入処理
    if( wk->cp_select->serviceID == RESORT_SERVICE_TYPE_ITEM )
    {
      PMSND_PlaySE( SEQ_SE_SYS_22 );
      ShopPrintMsgItem( wk, mes_shop_02_04, wk->cp_select->param1 );
      // 支払い方法によって引き算する対象を替える
      _sub_payment( wk, wk->cp_select->price );
      print_mygold( wk );
      MYITEM_AddItem( wk->myitem, wk->cp_select->param1, wk->cp_select->param2, wk->heapId);

      //買った場合、一日フラグをONにする
      RESORT_INSTITUTION_DATA_SetBit(wk->p_institution, wk->cp_select->id, TRUE );

      wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
      wk->next = SHOPBUY_SEQ_BACK;
    }
    else
    {
      //その他のサービスは選んだら終了。あとでスクリプト側から料金とフラグを建てる
      PMSND_PlaySE( SEQ_SE_SYS_22 );
      ShopPrintMsg( wk, mes_shop_02_07_03 );
      wk->seq  = SHOPBUY_SEQ_MSG_WAIT;
      wk->next = SHOPBUY_SEQ_END;
    }
    break;

  // メッセージ表示待ち
  case SHOPBUY_SEQ_MSG_WAIT:    
    wk->seq = shop_buy_printwait( wk );
    break;

  case SHOPBUY_SEQ_END:

    //終了時に選んだ物を取得
    if( wk->cp_select )
    {
      *wk->ret_service_type = wk->cp_select->serviceID;
      *wk->ret_service_param   = wk->cp_select->param1;
      *wk->ret_service_idx = wk->cp_select->id;
      *wk->ret_price = wk->cp_select->price;
    }
    else
    {
      *wk->ret_service_type = RESORT_LIST_CANCEL;
      *wk->ret_service_param   = 0;
      *wk->ret_service_idx = 0;
      *wk->ret_price = 0;
    }

    wk->List_Exit( gsys, wk, type, id );

    bmpwin_list_exit(wk);   // BMPLIST解放
    obj_del( wk );          // OBJ削除
    bmpwin_exit( wk );      // BMPWIN解放
    bg_clear(wk);           // BG消去
    
    // 品揃えデータ削除
    shop_item_release( wk);

    // ワーク解放
    exit_work(wk);
    return TRUE;
    break;
  }

  PRINTSYS_QUE_Main( wk->printQue );
  PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );

  GFL_TCBL_Main( wk->pMsgTcblSys ); // フィールド描画は１／３０なのでアプリと合わせるために２回呼ぶ
  GFL_TCBL_Main( wk->pMsgTcblSys );
  
  // 継続
  return FALSE;
}

static BOOL ShopCallFunc_Select( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id )
{
  u32 ret;
  
  switch( wk->seq ){
  case SHOPSELECT_SEQ_INIT:  // 初期化
    shop_call_init(gsys, wk, type, id);
    bg_trans( wk );     // BG転送
    GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0, 0, 0, 11, 17, GFL_BG_SCRWRT_PALNL );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
    wk->List_Init( gsys, wk, type, id );
    wk->seq++;
    break;
  case SHOPSELECT_SEQ_MAIN:
    select_shop_main(wk);
    break;
  case SHOPSELECT_SEQ_BACK:    // 戻る
    submenu_screen_clear( SCREEN_SUBMENU_AND_INFOWIN );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_ITEMINFO] );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    BmpMenuList_Rewrite( wk->menuList );
    wk->seq = SHOPSELECT_SEQ_MAIN;
    break;
  case SHOPSELECT_SEQ_ONLY1_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );
    wk->seq = SHOPSELECT_SEQ_SELECT_YESNO_WAIT;

    break;

  case SHOPSELECT_SEQ_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );
    wk->seq = SHOPSELECT_SEQ_SELECT_YESNO_WAIT;
    break;
  case SHOPSELECT_SEQ_SELECT_YESNO_WAIT:
    ret = BmpMenu_YesNoSelectMain( wk->yesnoWork );
    if(ret!=BMPMENU_NULL){
      if(ret==0){
        wk->seq = SHOPSELECT_SEQ_BUY;
        submenu_screen_clear( SCREEN_SUBMENU_ONLY );
        BmpMenuList_Rewrite( wk->menuList );
        GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
        GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
      }else if(ret==BMPMENU_CANCEL){
        wk->seq = SHOPSELECT_SEQ_BACK;
      }
    }
    break;
  case SHOPSELECT_SEQ_BUY:
    PMSND_PlaySE( SEQ_SE_SYS_22 );
    ShopPrintMsg( wk, wk->msg_decide );
    wk->seq  = SHOPSELECT_SEQ_MSG_WAIT;
    wk->next = SHOPSELECT_SEQ_END;
    break;

  // メッセージ表示待ち
  case SHOPSELECT_SEQ_MSG_WAIT:    
    wk->seq = shop_buy_printwait( wk );
    break;

  case SHOPSELECT_SEQ_END:

    //終了時に選んだ物を取得
    if( wk->cp_select )
    {
      *wk->ret_service_type = wk->cp_select->serviceID;
      *wk->ret_service_param   = 0;
      *wk->ret_service_idx = wk->cp_select->id;
      *wk->ret_price = wk->cp_select->price;
    }
    else
    {
      *wk->ret_service_type = RESORT_LIST_CANCEL;
      *wk->ret_service_param   = 0;
      *wk->ret_service_idx = 0;
      *wk->ret_price = 0;
    }


    bmpwin_list_exit(wk);   // BMPLIST解放
    obj_del( wk );          // OBJ削除
    bmpwin_exit( wk );      // BMPWIN解放
    bg_clear(wk);           // BG消去
    
    // 品揃えデータ削除
    shop_item_release( wk);

    // ワーク解放
    exit_work(wk);
    return TRUE;
    break;
  }

  PRINTSYS_QUE_Main( wk->printQue );
  PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );

  GFL_TCBL_Main( wk->pMsgTcblSys ); // フィールド描画は１／３０なのでアプリと合わせるために２回呼ぶ
  GFL_TCBL_Main( wk->pMsgTcblSys );
  
  // 継続
  return FALSE;
}
static BOOL ShopCallFunc_Sort( GAMESYS_WORK *gsys, SHOP_BUY_APP_WORK *wk, int type, int id )
{
  u32 ret;
  
  switch( wk->seq ){
  case SHOPSORT_SEQ_INIT:  // 初期化
    shop_call_init(gsys, wk, type, id);
    bg_trans( wk );     // BG転送
    GFL_BG_FillScreen( GFL_BG_FRAME2_M, 0, 0, 0, 11, 17, GFL_BG_SCRWRT_PALNL );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
    wk->List_Init( gsys, wk, type, id );
    wk->seq++;
    break;
  
  case SHOPSORT_SEQ_INITWAIT:
   if(PRINTSYS_QUE_IsFinished( wk->printQue )==FALSE){
     break;
    }
    // BMPの転送待ちの場合があるので、BmpwinOnのタイミングをズラした
    if(wk->FirstTrans==0){
      GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_LIST] );
      wk->FirstTrans++;
    }

    ShopPrintMsg( wk, wk->msg_start ); 
    wk->seq  = SHOPSORT_SEQ_MSG_WAIT;
    wk->next = SHOPSORT_SEQ_BACK;
    break;

  case SHOPSORT_SEQ_MAIN:
    sort_shop_main(wk);
    break;
  case SHOPSORT_SEQ_BACK:    // 戻る
    submenu_screen_clear( SCREEN_SUBMENU_AND_INFOWIN );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_ITEMINFO] );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    BmpMenuList_Rewrite( wk->menuList );
    wk->seq = SHOPSORT_SEQ_MAIN;
    break;
  case SHOPSORT_SEQ_RESET: //リスト作り直し
    wk->List_Change( wk, type, id );
    wk->is_sort = FALSE;
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_CURSOR_RED], FALSE );
    bmpwin_list_exit(wk);
    bmpwin_list_init( wk, wk->call_back, wk->icon );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_BG_LoadScreenV_Req( GFL_BG_FRAME1_M );

    ShopPrintMsg( wk, wk->msg_decide );
    wk->seq = SHOPSORT_SEQ_MSG_WAIT;
    wk->next = SHOPSORT_SEQ_BACK;
    break;
  case SHOPSORT_SEQ_ONLY1_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );
    wk->seq = SHOPSORT_SEQ_SELECT_YESNO_WAIT;

    break;

  case SHOPSORT_SEQ_YESNO:
    submenu_screen_clear( SCREEN_SUBMENU_ONLY );
    GFL_BMPWIN_MakeScreen( wk->win[SHOP_BUY_BMPWIN_LIST] );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );
    wk->yesnoWork = BmpMenu_YesNoSelectInit(  &yesno_data, 1, MENU_WINDOW_PAL_OFFSET, 0, wk->heapId );
    wk->seq = SHOPSORT_SEQ_SELECT_YESNO_WAIT;
    break;
  case SHOPSORT_SEQ_SELECT_YESNO_WAIT:
    ret = BmpMenu_YesNoSelectMain( wk->yesnoWork );
    if(ret!=BMPMENU_NULL){
      if(ret==0){
        wk->seq = SHOPSORT_SEQ_RESET;
      }else if(ret==BMPMENU_CANCEL){
        wk->seq = SHOPSORT_SEQ_BACK;
      }
    }
    break;
  // メッセージ表示待ち
  case SHOPSORT_SEQ_MSG_WAIT:    
    wk->seq = shop_buy_printwait( wk );
    break;

  case SHOPSORT_SEQ_END:

    //終了時に選んだ物を取得
    if( wk->cp_select )
    {
      *wk->ret_service_type = wk->cp_select->serviceID;
      *wk->ret_service_param   = 0;
      *wk->ret_service_idx = wk->cp_select->id;
      *wk->ret_price = wk->cp_select->price;
    }
    else
    {
      *wk->ret_service_type = RESORT_LIST_CANCEL;
      *wk->ret_service_param   = 0;
      *wk->ret_service_idx = 0;
      *wk->ret_price = 0;
    }

    wk->List_Decide( wk, type, id );

    bmpwin_list_exit(wk);   // BMPLIST解放
    obj_del( wk );          // OBJ削除
    bmpwin_exit( wk );      // BMPWIN解放
    bg_clear(wk);           // BG消去
    
    // 品揃えデータ削除
    shop_item_release( wk);

    // ワーク解放
    exit_work(wk);
    return TRUE;
    break;
  }

  PRINTSYS_QUE_Main( wk->printQue );
  PRINT_UTIL_Trans( &wk->printUtil, wk->printQue );

  GFL_TCBL_Main( wk->pMsgTcblSys ); // フィールド描画は１／３０なのでアプリと合わせるために２回呼ぶ
  GFL_TCBL_Main( wk->pMsgTcblSys );
  
  // 継続
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * @brief ショップアイテム登録
 *
 * @param   wk    SHOP_BUY_APP_WORK
 * @param   type  ショップタイプ(SCR_SHOPID_NULL,SCR_SHOPID_FIX)
 * @param   id    固定ショップの際のID（shop_data.cdat参照)
 */
//----------------------------------------------------------------------------------
static void shop_item_set( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i, itemnum;

  GFL_MSGDATA *itemMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                             NARC_message_itemname_dat, wk->heapId );

  GFL_MSGDATA *resortMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, 
                                             NARC_script_message_resort_scr_dat, wk->heapId );

  GFL_MSGDATA *itemInfoMsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
                                             NARC_message_iteminfo_dat, wk->heapId );

  // どうぐ名登録・BMPMENULISTデータ作成
  itemnum = RESORT_BINARY_SHOP_DATA_GetListMax( wk->p_shop, wk->cp_shop_data );

  wk->list = BmpMenuWork_ListCreate( itemnum+1, wk->heapId );
  GF_ASSERT_MSG( itemnum<RESORT_SHOP_ITEM_MAX, "ショップに一度に並べられる最大数を超えています\n" )


  for(i=0;i<itemnum;i++)
  {
    const RESORT_BINARY_SERVICE_DATA *cp_data = RESORT_BINARY_SHOP_DATA_GetList( wk->p_shop, wk->cp_shop_data, i );
    u16 price = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_PRICE );

    // 商品ラインナップテーブルにIDと値段を登録
    wk->lineup[i].id    = i;
    wk->lineup[i].serviceID = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_SERVICE_ID );
 
    // Gパワー係数を掛けてから値段を代入する
    wk->lineup[i].price =  GPOWER_Calc_Sale(price);
    wk->lineup[i].param1 = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_PARAM_1 );
    wk->lineup[i].param2 = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_PARAM_2 );

    //名前フラグがONならばデータから作成
    //OFFならば道具から作成
    if( RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_NAME_FLAG ) )
    {
      u16 name = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_NAME );
      u16 info = RESORT_BINARY_SERVICE_DATA_GetData( cp_data, RESORT_BINARY_SERVICE_PARAM_INFO );
      wk->lineup[i].p_name = GFL_MSG_CreateString( resortMsgData, name );
      wk->lineup[i].p_info = GFL_MSG_CreateString( resortMsgData, info );
    }
    else
    {
      wk->lineup[i].p_name = GFL_MSG_CreateString( itemMsgData, wk->lineup[i].param1 );
      wk->lineup[i].p_info = GFL_MSG_CreateString( itemInfoMsgData, wk->lineup[i].param1 );
    }
    
    //リストに加える
    BmpMenuWork_ListAddString( &wk->list[i], wk->lineup[i].p_name,
                                      i,                // 商品テーブルのインデックス
                                      wk->heapId );

  }
  wk->lineup_num = i;
  
  
  // 「やめる」登録
  BmpMenuWork_ListAddArchiveString( &wk->list[i], wk->shopMsgData, 
                                    mes_shop_02_06, 
                                    BMPMENULIST_CANCEL, 
                                    wk->heapId );

  GFL_MSG_Delete( itemInfoMsgData );
  GFL_MSG_Delete( resortMsgData );
  GFL_MSG_Delete( itemMsgData );
}


static void shop_secretary_set( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i, num, cnt;

  // どうぐ名登録・BMPMENULISTデータ作成
  num = RESORT_PEOPLE_ALL_DATA_GetSecretaryMax( wk->p_all_people );

  wk->list = BmpMenuWork_ListCreate( num+1, wk->heapId );
  GF_ASSERT_MSG( num<RESORT_SHOP_ITEM_MAX, "ショップに一度に並べられる最大数を超えています\n" );

  cnt = 0;
  for(i=0;i<RESORT_SECRETARY_DATA_NUM;i++)
  {
    RESORT_SECRETARY_DATA *p_secretary = RESORT_PEOPLE_ALL_DATA_GetSecretary( wk->p_all_people, i );

    if( !RESORT_SECRETARY_DATA_IsEmpty(p_secretary) )
    {
      // 商品ラインナップテーブルにIDと値段を登録
      wk->lineup[cnt].id    = i;
      wk->lineup[cnt].serviceID = 0;
      wk->lineup[cnt].price =  0;
      wk->lineup[cnt].param1 = RESORT_SECRETARY_DATA_GetData( p_secretary, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
      wk->lineup[cnt].param2 = 0;
      wk->lineup[cnt].p_data_adrs = p_secretary;

      {
        STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];
        STRBUF *p_name;
        STRBUF *p_name_str;
        STRBUF *p_info_str;
        STRBUF *p_name_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_01 );
        STRBUF *p_info_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_03 );
        u8 sex = RESORT_SECRETARY_DATA_GetData( p_secretary, RESORT_DATA_PARAM_SEX, NULL );
        RESORT_SECRETARY_DATA_GetData( p_secretary, RESORT_DATA_PARAM_NAME, name );
        p_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, wk->heapId );
        GFL_STR_SetStringCode( p_name ,name );

        p_name_str = GFL_STR_CreateBuffer( 64, wk->heapId );
        p_info_str = GFL_STR_CreateBuffer( 128, wk->heapId );
        WORDSET_RegisterNumber( wk->wordSet, 1, cnt+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterWord( wk->wordSet, 0, p_name, sex, TRUE, PM_LANG );

        WORDSET_ExpandStr( wk->wordSet, p_name_str, p_name_tmp );
        WORDSET_ExpandStr( wk->wordSet, p_info_str, p_info_tmp );

        wk->lineup[cnt].p_name =  p_name_str;
        wk->lineup[cnt].p_info =  p_info_str;

        GFL_STR_DeleteBuffer( p_name );
        GFL_STR_DeleteBuffer( p_info_tmp );
        GFL_STR_DeleteBuffer( p_name_tmp );

        //リストに加える
        BmpMenuWork_ListAddString( &wk->list[cnt], wk->lineup[cnt].p_name,
          cnt,                // 商品テーブルのインデックス
          wk->heapId );
        cnt++;
      }
    }
  }
  wk->lineup_num = num;
  
  // 「やめる」登録
  BmpMenuWork_ListAddArchiveString( &wk->list[i], wk->shopMsgData, 
                                    mes_shop_02_06, 
                                    BMPMENULIST_CANCEL, 
                                    wk->heapId );
}

static void shop_institution_set( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i, num, cnt;

  // どうぐ名登録・BMPMENULISTデータ作成
  num = RESORT_PEOPLE_ALL_DATA_GetResidentCount( wk->p_all_people );

  wk->list = BmpMenuWork_ListCreate( num+1, wk->heapId );
  GF_ASSERT_MSG( 0 < num && num<RESORT_SHOP_ITEM_MAX, "ショップに一度に並べられる最大数を超えています%d\n", num );

  cnt = 0;
  for(i=0;i<RESORT_RESIDENT_DATA_NUM;i++)
  {
    RESORT_RESIDENT_DATA *p_resident = RESORT_PEOPLE_ALL_DATA_GetResident( wk->p_all_people, i );

    if( !RESORT_RESIDENT_DATA_IsEmpty(p_resident) )
    {
      // 商品ラインナップテーブルにIDと値段を登録
      wk->lineup[cnt].id    = i;
      wk->lineup[cnt].serviceID = 0;
      wk->lineup[cnt].price =  0;
      wk->lineup[cnt].param1 = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_TRAINERVIEW, NULL );
      wk->lineup[cnt].param2 = 0;
      wk->lineup[cnt].p_data_adrs = p_resident;

      {
        u16 shop = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_TYPE, NULL );

        const RESORT_INSTITUTION_DATA * cp_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
        u16 lv = RESORT_INSTITUTION_DATA_GetData( cp_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
        u16 type = 0; //TODO
        const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchData( wk->p_shop, shop, type, lv );
        STRBUF *p_shopname = RESORT_DATA_MANAGER_CreateShopName( cp_shop_data, p_resident, wk->heapId );

        GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(wk->heapId) );
        STRBUF *p_shopinfo = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+shop );

        STRBUF *p_name_str;
        STRBUF *p_info_str;
        STRBUF *p_name_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_02 );
        STRBUF *p_info_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_04 );
        u8 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );

        p_name_str = GFL_STR_CreateBuffer( 64, wk->heapId );
        p_info_str = GFL_STR_CreateBuffer( 128, wk->heapId );
        WORDSET_RegisterNumber( wk->wordSet, 1, cnt+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterWord( wk->wordSet, 0, p_shopname, sex, TRUE, PM_LANG );

        WORDSET_ExpandStr( wk->wordSet, p_name_str, p_name_tmp );


        WORDSET_RegisterWord( wk->wordSet, 1, p_shopinfo, PM_NEUTRAL, TRUE, PM_LANG );
        WORDSET_ExpandStr( wk->wordSet, p_info_str, p_info_tmp );

        wk->lineup[cnt].p_name =  p_name_str;
        wk->lineup[cnt].p_info =  p_info_str;

        GFL_STR_DeleteBuffer( p_info_tmp );
        GFL_STR_DeleteBuffer( p_name_tmp );
        GFL_STR_DeleteBuffer( p_shopinfo );
        GFL_MSG_Delete( p_msg );
        GFL_STR_DeleteBuffer( p_shopname );


        //リストに加える
        BmpMenuWork_ListAddString( &wk->list[cnt], wk->lineup[cnt].p_name,
          cnt,                // 商品テーブルのインデックス
          wk->heapId );
        cnt++;
      }
    }
  }
  wk->lineup_num = num;
  
  // 「やめる」登録
  BmpMenuWork_ListAddArchiveString( &wk->list[i], wk->shopMsgData, 
                                    mes_shop_02_06, 
                                    BMPMENULIST_CANCEL, 
                                    wk->heapId );
}
static void SecretaryList_Change( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i;
  RESORT_SHOP_ITEM temp;
  
  temp = wk->lineup[ wk->insert_idx ];
  wk->lineup[ wk->insert_idx ]  = wk->lineup[ wk->now_idx ];
  wk->lineup[ wk->now_idx ] = temp;
  wk->cp_select = NULL;

  BmpMenuWork_ListDelete( wk->list );
  wk->list = BmpMenuWork_ListCreate( wk->lineup_num+1, wk->heapId );

  for(i=0;i<wk->lineup_num;i++)
  {
    RESORT_SECRETARY_DATA *p_secretary = wk->lineup[i].p_data_adrs;

    STRCODE name[PERSON_NAME_SIZE + EOM_SIZE];
    STRBUF *p_name;
    STRBUF *p_name_str;
    STRBUF *p_info_str;
    STRBUF *p_name_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_01 );
    STRBUF *p_info_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_03 );
    u8 sex = RESORT_SECRETARY_DATA_GetData( p_secretary, RESORT_DATA_PARAM_SEX, NULL );
    RESORT_SECRETARY_DATA_GetData( p_secretary, RESORT_DATA_PARAM_NAME, name );
    p_name = GFL_STR_CreateBuffer( PERSON_NAME_SIZE + EOM_SIZE, wk->heapId );
    GFL_STR_SetStringCode( p_name ,name );

    WORDSET_RegisterNumber( wk->wordSet, 1, i+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_RegisterWord( wk->wordSet, 0, p_name, sex, TRUE, PM_LANG );

    WORDSET_ExpandStr( wk->wordSet, wk->lineup[i].p_name, p_name_tmp );
    WORDSET_ExpandStr( wk->wordSet, wk->lineup[i].p_info, p_info_tmp );

    GFL_STR_DeleteBuffer( p_name );
    GFL_STR_DeleteBuffer( p_info_tmp );
    GFL_STR_DeleteBuffer( p_name_tmp );


    //リストに加える
    BmpMenuWork_ListAddString( &wk->list[i], wk->lineup[i].p_name,
        i,                // 商品テーブルのインデックス
        wk->heapId );
  }

  // 「やめる」登録
  BmpMenuWork_ListAddArchiveString( &wk->list[i], wk->shopMsgData, 
                                    mes_shop_02_06, 
                                    BMPMENULIST_CANCEL, 
                                    wk->heapId );
}

static void InstitutionList_Change( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i;
  RESORT_SHOP_ITEM temp;
  
  temp = wk->lineup[ wk->insert_idx ];
  wk->lineup[ wk->insert_idx ]  = wk->lineup[ wk->now_idx ];
  wk->lineup[ wk->now_idx ] = temp;
  wk->cp_select = NULL;

  BmpMenuWork_ListDelete( wk->list );
  wk->list = BmpMenuWork_ListCreate( wk->lineup_num+1, wk->heapId );

  for(i=0;i<wk->lineup_num;i++)
  {
    RESORT_RESIDENT_DATA *p_resident = wk->lineup[i].p_data_adrs;

        u16 shop = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_TYPE, NULL );

        const RESORT_INSTITUTION_DATA * cp_institution = RESORT_RESIDENT_DATA_GetInstitution( p_resident );
        u16 lv = RESORT_INSTITUTION_DATA_GetData( cp_institution, RESORT_INSTITUTION_DATA_PARAM_LV );
        u16 type = 0; //TODO
        const RESORT_BINARY_SHOP_DATA *cp_shop_data = RESORT_BINARY_SHOP_SearchData( wk->p_shop, shop, type, lv );
        STRBUF *p_shopname = RESORT_DATA_MANAGER_CreateShopName( cp_shop_data, p_resident, wk->heapId );

        GFL_MSGDATA* p_msg = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_resort_scr_dat, GFL_HEAP_LOWID(wk->heapId) );
        STRBUF *p_shopinfo = GFL_MSG_CreateString( p_msg, resort_shop_genre_00+shop );

        STRBUF *p_name_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_02 );
        STRBUF *p_info_tmp = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_09_04 );
        u8 sex = RESORT_RESIDENT_DATA_GetData( p_resident, RESORT_DATA_PARAM_SEX, NULL );

        WORDSET_RegisterNumber( wk->wordSet, 1, i+1, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
        WORDSET_RegisterWord( wk->wordSet, 0, p_shopname, sex, TRUE, PM_LANG );

        WORDSET_ExpandStr( wk->wordSet, wk->lineup[i].p_name, p_name_tmp );


        WORDSET_RegisterWord( wk->wordSet, 1, p_shopinfo, PM_NEUTRAL, TRUE, PM_LANG );
        WORDSET_ExpandStr( wk->wordSet, wk->lineup[i].p_info, p_info_tmp );

        GFL_STR_DeleteBuffer( p_info_tmp );
        GFL_STR_DeleteBuffer( p_name_tmp );
        GFL_STR_DeleteBuffer( p_shopinfo );
        GFL_MSG_Delete( p_msg );
        GFL_STR_DeleteBuffer( p_shopname );


    //リストに加える
    BmpMenuWork_ListAddString( &wk->list[i], wk->lineup[i].p_name,
        i,                // 商品テーブルのインデックス
        wk->heapId );
  }

  // 「やめる」登録
  BmpMenuWork_ListAddArchiveString( &wk->list[i], wk->shopMsgData, 
                                    mes_shop_02_06, 
                                    BMPMENULIST_CANCEL, 
                                    wk->heapId );
}

//----------------------------------------------------------------------------------
/**
 * @brief ショップアイテム情報解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void shop_item_release( SHOP_BUY_APP_WORK *wk )
{

  // メニューリスト解放
  BmpMenuWork_ListDelete( wk->list );


  {
    int i;
    for(i=0;i<RESORT_SHOP_ITEM_MAX;i++)
    {
      if( wk->lineup[i].p_info )
      {
        GFL_STR_DeleteBuffer( wk->lineup[i].p_info );
        wk->lineup[i].p_info  = NULL;
      }
      if( wk->lineup[i].p_name )
      {
        GFL_STR_DeleteBuffer( wk->lineup[i].p_name );
        wk->lineup[i].p_name = NULL;
      }
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief ワーク初期化
 *
 * @param   wk    SHOP_BUY_APP_WORK
 */
//----------------------------------------------------------------------------------
static int init_work( SHOP_BUY_APP_WORK *wk, int type )
{
  // メッセージデータ確保

  wk->shopMsgData     = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, 
                                             NARC_script_message_resort_shop_dat, wk->heapId );

  wk->wordSet   = WORDSET_Create( wk->heapId );

  // 展開用バッファ
  wk->expandBuf = GFL_STR_CreateBuffer( SHOP_STRING_MAX, wk->heapId );

  // 会話ウインドウ用タスク登録
  wk->pMsgTcblSys = GFL_TCBL_Init( wk->heapId, wk->heapId , 32 , 32 );


#ifdef SHOP_DEBUG_COMMAND

  if(GFL_UI_KEY_GetCont()&PAD_BUTTON_L){
    MISC_SetGold(wk->misc , 9999999);
  }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_X){
    type = RESORT_LIST_TYPE_BUY;
  }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_Y){
    type = RESORT_LIST_TYPE_SELECT;
  }else if(GFL_UI_KEY_GetCont()&PAD_KEY_UP){
    type = RESORT_LIST_TYPE_SORT;
  }else if(GFL_UI_KEY_GetCont()&PAD_BUTTON_R){
    MISC_SetGold(wk->misc , 5000);
  }
#endif

  return type;
}



//----------------------------------------------------------------------------------
/**
 * @brief ワーク解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void exit_work( SHOP_BUY_APP_WORK *wk )
{
  GFL_TCBL_Exit( wk->pMsgTcblSys );

  WORDSET_Delete(wk->wordSet);
  
  GFL_STR_DeleteBuffer( wk->expandBuf );
  GFL_STR_DeleteBuffer( wk->HaveStr );
  GFL_STR_DeleteBuffer( wk->priceStr );

  // メッセージデータ解放
  GFL_MSG_Delete( wk->shopMsgData );

}

// パレット転送本数(byte単位）
#define SHOP_BG_PLTT_NUM  ( 32*3 )

//----------------------------------------------------------------------------------
/**
 * @brief BG転送
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bg_trans( SHOP_BUY_APP_WORK *wk )
{
  // 会話ウインドウ半透明OFF
//  FLDMSGBG_SetBlendAlpha( FALSE );
  G2_BlendNone();
  
  // 会話ウインドウがBG1面BG2面両方使用する事になったので、ショップ画面の背景は
  // VRAMの最初に方に入れてある会話ウインドウのリソースを避けて転送する事にした。
  // 会話ウインドウのキャラVRAMはBG1とBG2が同じにしてある。
    
  // ショップ画面リソース
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_SHOP_GRA, wk->heapId );
  
    // MAIN BG面Pltt
    GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_shop_gra_shop_bg_NCLR, PALTYPE_MAIN_BG, 
                                      0, SHOP_BG_PLTT_NUM, wk->heapId );
    // BG2面Char(背景）
    GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_shop_gra_shop_bg_NCGR, GFL_BG_FRAME2_M, 
                                      100, 0, 0, wk->heapId);
    // BG2面Screen(背景）
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs( handle, NARC_shop_gra_shop_bg1_NSCR, GFL_BG_FRAME2_M, 
                                            0, 100, 0, 0, wk->heapId);
    GFL_ARC_CloseDataHandle( handle );
  }
  
  // ウインドウ枠
  BmpWinFrame_GraphicSet( GFL_BG_FRAME1_M, MENU_WINDOW_CHARA_OFFSET, MENU_WINDOW_PAL_OFFSET, 
                          MENU_TYPE_SYSTEM, wk->heapId );

  // テキスト面消去
  GFL_BG_ClearScreen( GFL_BG_FRAME1_M );



  // BG2面のプライオリティを１に変更
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, 1 );

}


//----------------------------------------------------------------------------------
/**
 * @brief BG消去
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bg_clear( SHOP_BUY_APP_WORK *wk )
{
  // ショップBG面消去
  GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
  GFL_BG_ClearScreen( GFL_BG_FRAME2_M );

  // ショップ用に変更していたBG2面のプライオリティを0に戻す
  GFL_BG_SetPriority( GFL_BG_FRAME2_M, 0 );

  // 会話ウインドウ半透明処理復帰
  FLDMSGBG_SetBlendAlpha( TRUE );

}

#define SHOP_BUY_OBJ_BG_PRI ( 1 )

enum{
  SHOP_OBJ_RES_PLTT=0,
  SHOP_OBJ_RES_CHAR,
  SHOP_OBJ_RES_CELL,
};

static const GFL_CLWK_DATA shop_obj_param[]={
  { 172,  22, 0, 0, 1 },// カーソル
  { 172,  92, 0, 0, 0 },// 上矢印
  { 172, 132, 1, 0, 0 },// 下矢印
  { 224, 128, 2, 0, 0 },// 上下カーソル
  {  22, 172, 0, 0, 1 },// どうぐアイコン
  {  18, 168, 0, 0, 1 },// 人物アイコン 
  {  18, 168, 1, 0, 1 },// カーソル赤
  // ｘ,ｙ座標,アニメーションシーケンス,ソフト優先順位  0>0xff, BG優先順位
};


//----------------------------------------------------------------------------------
/**
 * @brief ショップ画面OBJリソース読み込み
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void shop_resource_load( SHOP_BUY_APP_WORK *wk )
{
  ARCHANDLE *handle = GFL_ARC_OpenDataHandle( ARCID_SHOP_GRA, wk->heapId );

  // パレット
  wk->clact_res[0][ SHOP_OBJ_RES_PLTT ] = GFL_CLGRP_PLTT_RegisterEx( 
        handle, NARC_shop_gra_shop_obj_NCLR, CLSYS_DRAW_MAIN, 
        0, 0, 2, wk->heapId );

  // キャラ( 矢印 )
  wk->clact_res[0][ SHOP_OBJ_RES_CHAR ] = GFL_CLGRP_CGR_Register( 
        handle, NARC_shop_gra_shop_sel_NCGR, 0, CLSYS_DRAW_MAIN, wk->heapId );
  // キャラ( カーソル )
  wk->clact_res[1][ SHOP_OBJ_RES_CHAR ] = GFL_CLGRP_CGR_Register( 
        handle, NARC_shop_gra_shop_arw_NCGR, 0, CLSYS_DRAW_MAIN, wk->heapId );

  // セル（矢印）
  wk->clact_res[0][ SHOP_OBJ_RES_CELL ] = GFL_CLGRP_CELLANIM_Register( 
        handle, NARC_shop_gra_shop_sel_NCER, NARC_shop_gra_shop_sel_NANR, wk->heapId );
  // セル（カーソル）
  wk->clact_res[1][ SHOP_OBJ_RES_CELL ] = GFL_CLGRP_CELLANIM_Register( 
        handle, NARC_shop_gra_shop_arw_NCER, NARC_shop_gra_shop_arw_NANR, wk->heapId );

  GFL_ARC_CloseDataHandle( handle );
}


//----------------------------------------------------------------------------------
/**
 * @brief どうぐアイコン読み込み
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void itemicon_resource_load( SHOP_BUY_APP_WORK *wk )
{
  // パレット
  wk->clact_res[2][ SHOP_OBJ_RES_PLTT ] = GFL_CLGRP_PLTT_RegisterEx( 
        wk->iconHandle, ITEM_GetIndex( 1, ITEM_GET_ICON_PAL ), CLSYS_DRAW_MAIN, 
        0x20*2, 0, 1, wk->heapId );

  // キャラ
  wk->clact_res[2][ SHOP_OBJ_RES_CHAR ] = GFL_CLGRP_CGR_Register( 
        wk->iconHandle, ITEM_GetIndex( 1, ITEM_GET_ICON_CGX ), 0, CLSYS_DRAW_MAIN, wk->heapId );

  // セル
  wk->clact_res[2][ SHOP_OBJ_RES_CELL ] = GFL_CLGRP_CELLANIM_Register( 
        wk->iconHandle, NARC_item_icon_itemicon_NCER, NARC_item_icon_itemicon_NANR, wk->heapId );
 
}

//----------------------------------------------------------------------------------
/**
 * @brief どうぐアイコンリソース差し替え
 *
 * @param   wk      
 * @param   itemno  どうぐNO
 */
//----------------------------------------------------------------------------------
static void itemicon_resource_change( SHOP_BUY_APP_WORK *wk, u16 itemno )
{
  NNSG2dCharacterData* chara;
  NNSG2dPaletteData*   pltt;
  void *chara_buf,*pltt_buf;
  
  if(itemno<=ITEM_DATA_MAX){
    pltt_buf = GFL_ARCHDL_UTIL_LoadPalette( wk->iconHandle, 
                  ITEM_GetIndex( itemno, ITEM_GET_ICON_PAL ), &pltt, wk->heapId );
    chara_buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( wk->iconHandle, 
                  ITEM_GetIndex( itemno, ITEM_GET_ICON_CGX ), 0, &chara, wk->heapId );
  
    GFL_CLGRP_PLTT_Replace( wk->clact_res[2][ SHOP_OBJ_RES_PLTT ], pltt, 1 );
    GFL_CLGRP_CGR_Replace( wk->clact_res[2][ SHOP_OBJ_RES_CHAR ], chara );
  
    GFL_HEAP_FreeMemory( pltt_buf );
    GFL_HEAP_FreeMemory( chara_buf );
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ITEM], TRUE );
  }else{
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ITEM], FALSE );
   
  }
}
static void people_resource_change( SHOP_BUY_APP_WORK *wk, u16 trainer_view )
{
  NNSG2dCharacterData* chara;
  NNSG2dPaletteData*   pltt;
  void *chara_buf,*pltt_buf;
  const u16 *cp_plt_adrs;
  
  if(trainer_view<16){

    pltt_buf = GFL_ARCHDL_UTIL_LoadPalette( wk->iconHandle, 
                  NARC_wifi_unionobj_wifi_union_obj_NCLR, &pltt, wk->heapId );
    chara_buf = GFL_ARCHDL_UTIL_LoadOBJCharacter( wk->iconHandle, 
                  NARC_wifi_unionobj_front00_NCGR+trainer_view, 0, &chara, wk->heapId );
  
    //GFL_CLGRP_PLTT_Replace( wk->clact_res[2][ SHOP_OBJ_RES_PLTT ], pltt, 1 );

    //PLTTには転送元を変えられるReplaceがないので、SDKレベルで転送する
    cp_plt_adrs = (const u16*)pltt->pRawData + sc_wifi_unionobj_plt[ trainer_view ]*16;
    DC_FlushRange( cp_plt_adrs, 32 );
    GX_LoadOBJPltt( cp_plt_adrs, 0x20*2, 32 );

    GFL_CLGRP_CGR_Replace( wk->clact_res[2][ SHOP_OBJ_RES_CHAR ], chara );

    GFL_HEAP_FreeMemory( pltt_buf );
    GFL_HEAP_FreeMemory( chara_buf );

    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ITEM], TRUE );
  }else{
    GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ITEM], FALSE );
   
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief OBJ表示登録
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void obj_set( SHOP_BUY_APP_WORK *wk, const GFL_CLWK_DATA *cp_item_adta )
{
  // カーソル
  wk->ClactWork[SHOP_OBJ_CURSOR] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[0][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[0][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[0][ SHOP_OBJ_RES_CELL ], 
      &shop_obj_param[0], CLSYS_DEFREND_MAIN, wk->heapId );

  // 矢印上
  wk->ClactWork[SHOP_OBJ_ARROW_UP] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[1][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[0][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[1][ SHOP_OBJ_RES_CELL ], 
      &shop_obj_param[1], CLSYS_DEFREND_MAIN, wk->heapId );

  // 矢印下
  wk->ClactWork[SHOP_OBJ_ARROW_DOWN] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[1][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[0][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[1][ SHOP_OBJ_RES_CELL ], 
      &shop_obj_param[2], CLSYS_DEFREND_MAIN, wk->heapId );

  // 矢印上下
  wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[1][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[0][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[1][ SHOP_OBJ_RES_CELL ], 
      &shop_obj_param[3], CLSYS_DEFREND_MAIN, wk->heapId );

  GFL_CLACT_WK_SetAutoAnmFlag( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], TRUE );

  // どうぐアイコン
  wk->ClactWork[SHOP_OBJ_ITEM] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[2][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[2][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[2][ SHOP_OBJ_RES_CELL ], 
      cp_item_adta, CLSYS_DEFREND_MAIN, wk->heapId );
  GFL_CLACT_WK_SetPlttOffs( wk->ClactWork[SHOP_OBJ_ITEM], 0, CLWK_PLTTOFFS_MODE_PLTT_TOP);

  // カーソル(赤)
  wk->ClactWork[SHOP_OBJ_CURSOR_RED] = GFL_CLACT_WK_Create( wk->ClactUnit, 
      wk->clact_res[0][ SHOP_OBJ_RES_CHAR ], 
      wk->clact_res[0][ SHOP_OBJ_RES_PLTT ], 
      wk->clact_res[0][ SHOP_OBJ_RES_CELL ], 
      &shop_obj_param[6], CLSYS_DEFREND_MAIN, wk->heapId );
  GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_CURSOR_RED], FALSE );

  // 矢印は非表示
  GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UP],   FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_DOWN], FALSE );
  GFL_CLACT_WK_SetDrawEnable( wk->ClactWork[SHOP_OBJ_ARROW_UPDOWN], FALSE );

}

//----------------------------------------------------------------------------------
/**
 * @brief OBJ初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void obj_init( SHOP_BUY_APP_WORK *wk )
{
  // CLACT_UNIT作成
  wk->ClactUnit = GFL_CLACT_UNIT_Create( SHOP_OBJ_MAX, 
                                         SHOP_BUY_OBJ_BG_PRI, wk->heapId );
  // システムに連結
  GFL_CLACT_UNIT_SetDrawEnable( wk->ClactUnit, TRUE );

  // ショップ画面OBJリソース読み込み
  shop_resource_load( wk );
}

static void obj_load_item( SHOP_BUY_APP_WORK *wk )
{
  // アイテムアイコンのファイルハンドル開きっぱなし(obj_endで解放)
  wk->iconHandle = GFL_ARC_OpenDataHandle( ARCID_ITEMICON, wk->heapId );

  // 道具アイコンリソース読み込み
  itemicon_resource_load( wk );

  // OBJ登録
  obj_set( wk, &shop_obj_param[4] );
}

static void obj_load_people( SHOP_BUY_APP_WORK *wk )
{
  // 人物のファイルハンドル開きっぱなし(obj_endで解放)
  wk->iconHandle = GFL_ARC_OpenDataHandle( ARCID_WIFIUNIONCHAR, wk->heapId );

  // 道具アイコンリソース読み込み
  {
    // パレット
    wk->clact_res[2][ SHOP_OBJ_RES_PLTT ] = GFL_CLGRP_PLTT_RegisterEx( 
        wk->iconHandle, NARC_wifi_unionobj_wifi_union_obj_NCLR, CLSYS_DRAW_MAIN, 
        0x20*2, sc_wifi_unionobj_plt[ 0 ], 1, wk->heapId );

    // キャラ
    wk->clact_res[2][ SHOP_OBJ_RES_CHAR ] = GFL_CLGRP_CGR_Register( 
        wk->iconHandle, NARC_wifi_unionobj_front00_NCGR, 0, CLSYS_DRAW_MAIN, wk->heapId );

    // セル
    wk->clact_res[2][ SHOP_OBJ_RES_CELL ] = GFL_CLGRP_CELLANIM_Register( 
        wk->iconHandle, NARC_wifi_unionobj_front00_NCER, NARC_wifi_unionobj_front00_NANR, wk->heapId );
  }

  // OBJ登録
  obj_set( wk, &shop_obj_param[5] );
}


//----------------------------------------------------------------------------------
/**
 * @brief OBJ削除
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void obj_del( SHOP_BUY_APP_WORK *wk )
{

  // OBJリソース全解放
  GFL_CLGRP_CELLANIM_Release( wk->clact_res[2][ SHOP_OBJ_RES_CELL ] );
  GFL_CLGRP_CELLANIM_Release( wk->clact_res[1][ SHOP_OBJ_RES_CELL ] );
  GFL_CLGRP_CELLANIM_Release( wk->clact_res[0][ SHOP_OBJ_RES_CELL ] );

  GFL_CLGRP_CGR_Release( wk->clact_res[2][ SHOP_OBJ_RES_CHAR ] );
  GFL_CLGRP_CGR_Release( wk->clact_res[1][ SHOP_OBJ_RES_CHAR ] );
  GFL_CLGRP_CGR_Release( wk->clact_res[0][ SHOP_OBJ_RES_CHAR ] );

  GFL_CLGRP_PLTT_Release( wk->clact_res[2][ SHOP_OBJ_RES_PLTT ] );
  GFL_CLGRP_PLTT_Release( wk->clact_res[0][ SHOP_OBJ_RES_PLTT ] );

  // どうぐアイコンハンドル閉じる
  GFL_ARC_CloseDataHandle( wk->iconHandle );

  // ショップ用CLUNIT解放
  GFL_CLACT_UNIT_Delete( wk->ClactUnit );

}


//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bmpwin_init( SHOP_BUY_APP_WORK *wk )
{
  int i;
  const BMPWIN_DAT **tbl;
  const BMPWIN_DAT *dat;

  tbl = Window0BmpDataTable;

  for(i=0;i<SHOP_BUY_BMPWIN_MAX;i++){
    dat = tbl[i];
    wk->win[i] = GFL_BMPWIN_Create( dat->frame, dat->x, dat->y, dat->w, dat->h, 
                        dat->pal, GFL_BMP_CHRAREA_GET_B );
  }

  // PRINTUTILと関連付け
  PRINT_UTIL_Setup( &wk->printUtil, wk->win[SHOP_BUY_BMPWIN_LIST] );
  wk->printQue = PRINTSYS_QUE_Create( wk->heapId );

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_MONEY] );
}

static void print_money( SHOP_BUY_APP_WORK *wk )
{
  {
    // おこづかい
    STRBUF *str = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_05_01 );
    PRINTSYS_PrintColor( GFL_BMPWIN_GetBmp( wk->win[SHOP_BUY_BMPWIN_OKODUKAI]), 
                                            0, 0, str, wk->font, 
                                            WHITE_TEXT_W_COL );
    GFL_STR_DeleteBuffer( str );
  }

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_OKODUKAI] );
}

//----------------------------------------------------------------------------------
/**
 * @brief BMPWIN解放
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bmpwin_exit( SHOP_BUY_APP_WORK *wk )
{
  int i;

  PRINTSYS_QUE_Clear( wk->printQue );
  PRINTSYS_QUE_Delete( wk->printQue );

  for(i=0;i<SHOP_BUY_BMPWIN_MAX;i++){
    GFL_BMPWIN_Delete( wk->win[i] );
  }
}


//----------------------------------------------------------------------------------
/**
 * @brief 所持金表示
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void print_mygold( SHOP_BUY_APP_WORK *wk )
{
  STRBUF *str;
  STRBUF *expand;
  
  str    = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_05_02 );
  expand = GFL_STR_CreateBuffer( SHOP_MYGOLD_STR_MAX, wk->heapId );
  
  // 残金取得
  WORDSET_RegisterNumber( wk->wordSet, 0, MISC_GetGold(wk->misc), SHOP_YEN_KETA_MAX, 
                            STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );

  // 表示用文字列に展開
  WORDSET_ExpandStr( wk->wordSet, expand, str );

  // 描画
  {
    GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( wk->win[SHOP_BUY_BMPWIN_MONEY]);
    GFL_BMP_Clear( bmp, 0 );
    PRINTSYS_PrintColor( bmp,  0, 0, expand, wk->font, WHITE_TEXT_W_COL );
  
  }

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_MONEY] );

  GFL_STR_DeleteBuffer( expand );
  GFL_STR_DeleteBuffer( str );
}



//----------------------------------------------------------------------------------
/**
 * @brief 指定のアイテムを何個持っているか表示する
 *
 * @param   wk      
 * @param   itemno  どうぐNO
 */
//----------------------------------------------------------------------------------
static void print_carry_item( SHOP_BUY_APP_WORK *wk, u16 itemno )
{
  STRBUF *str    = GFL_MSG_CreateString( wk->shopMsgData, mes_shop_06_01 );
  STRBUF *expand = GFL_STR_CreateBuffer( RESORT_SHOP_ITEMNUM_STR_MAX, wk->heapId );


  BmpWinFrame_Write( wk->win[SHOP_BUY_BMPWIN_NUM], WINDOW_TRANS_OFF, 
                     MENU_WINDOW_CHARA_OFFSET, MENU_WINDOW_PAL_OFFSET );

  // 個数登録
  WORDSET_RegisterNumber( wk->wordSet, 0, MYITEM_GetItemNum(wk->myitem, itemno, wk->heapId), 
                          3, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
  WORDSET_ExpandStr( wk->wordSet, expand, str );

  // 描画
  {
    GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( wk->win[SHOP_BUY_BMPWIN_NUM]);
    GFL_BMP_Clear( bmp, 15 );
    PRINTSYS_PrintColor( bmp,  0, 0, expand, wk->font, BLACK_TEXT_W_COL );
  
  }

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_NUM] );

  GFL_STR_DeleteBuffer( expand );
  GFL_STR_DeleteBuffer( str );

}

//----------------------------------------------------------------------------------
/**
 * @brief どうぐ説明表示
 *
 * @param   wk    
 * @param   itemno    
 */
//----------------------------------------------------------------------------------
static void print_iteminfo( SHOP_BUY_APP_WORK *wk, const STRBUF *cp_info )
{
  GFL_BMP_DATA *bmp = GFL_BMPWIN_GetBmp( wk->win[SHOP_BUY_BMPWIN_ITEMINFO]);
  GFL_BMP_Clear( bmp, 0 );

  // 描画(存在しない場合はクリア)
  if(cp_info != NULL){
    PRINTSYS_PrintColor( bmp,  0, 0, cp_info, wk->font, WHITE_TEXT_B_COL );
  }

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_ITEMINFO] );

}

// リスト描画開始Y位置
#define SHOP_LIST_LINEY   ( 8 )

//----------------------------------------------------------------------------------
/**
 * @brief BMPWINリスト初期化
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bmpwin_list_init( SHOP_BUY_APP_WORK *wk, BMPMENULIST_CURSOR_CALLBACK call_back, BMPMENULIST_PRINT_CALLBACK icon )
{
  BMPMENULIST_HEADER header;

  
  header.list = wk->list;   //表示文字データポインタ
  header.call_back = call_back;
  header.icon      = icon;
  header.count     = BmpMenuWork_GetListMax(wk->list);    //リスト項目数
  header.line      = 7; //表示最大項目数
  header.label_x   = 0; //ラベル表示Ｘ座標
  header.data_x    = 0; //項目表示Ｘ座標
  header.cursor_x  = 0; //カーソル表示Ｘ座標
  header.line_y    = SHOP_LIST_LINEY; //表示Ｙ座標
  header.f_col     = 12; //表示文字色
  header.b_col     = 0;  //表示背景色
  header.s_col     = 13; //表示文字影色
  header.msg_spc   = 0;  //文字間隔Ｘ
  header.line_spc  = 0; //文字間隔Ｙ
  header.page_skip = BMPMENULIST_LRKEY_SKIP; //ページスキップタイプ
  header.font      = 0; //文字指定(本来はu8だけどそんなに作らないと思うので)
  header.c_disp_f  = 1; //ＢＧカーソル(allow)表示フラグ(0:ON,1:OFF)
  header.work      = wk;// callback時に参照するワーク
  
  header.font_size_x  = 12;   //文字サイズX(ドット
  header.font_size_y  = 16;   //文字サイズY(ドット
  header.msgdata      = NULL; //表示に使用するメッセージバッファ
  header.print_util   = &wk->printUtil; //表示に使用するプリントユーティリティ
  header.print_que    = wk->printQue; //表示に使用するプリントキュー
  header.font_handle  = wk->font; //表示に使用するフォントハンドル
  header.arrow_wait   = BMPMENULIST_ARROW_WAIT_MIN;

   wk->menuList = BmpMenuList_Set( &header, 0, 0, wk->heapId );
  
}

//----------------------------------------------------------------------------------
/**
 * @brief ハイリンク対策にprintQueを使用しない描画も混ぜたPrintColor
 *
 * @param   que   printQue
 * @param   bmp   BMP
 * @param   x     x座標
 * @param   y     y座標
 * @param   str   描画文字列
 * @param   font  GFL_FONT
 * @param   color PRINTSYS_LSB
 */
//----------------------------------------------------------------------------------
static void _callback_print( PRINT_QUE* que, GFL_BMP_DATA *bmp, int x, int y, STRBUF *str, GFL_FONT *font, PRINTSYS_LSB color )
{
  // コールバック内で画面にハミ出る部分を描画する時は、printQueではなく
  // PRINTSYSで直描きしないと間に合わないので分岐させる
  if(y<=SHOP_LIST_LINEY){
    PRINTSYS_PrintColor( bmp, x, y, str, font, color );
    
  }else{
    PRINTSYS_PrintQueColor( que, bmp, x, y, str, font, color );
  }
  
}

//----------------------------------------------------------------------------------
/**
 * @brief １行表示コールバック（主に値段表示処理)
 *
 * @param   wk      
 * @param   param   どうぐNO
 * @param   y       表示Y座標
 */
//----------------------------------------------------------------------------------
static void line_callback(BMPMENULIST_WORK * wk, u32 param, u8 y )
{
  SHOP_BUY_APP_WORK *sbw = BmpMenuList_GetWorkPtr( wk );

  // 「やめる」以外には値段を表示
  if(param!=BMPMENULIST_CANCEL){
    int length,bmp_w;
    // 値段セット
    WORDSET_RegisterNumber( sbw->wordSet, 1, sbw->lineup[param].price, 
                            5, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );  
    WORDSET_ExpandStr( sbw->wordSet, sbw->expandBuf, sbw->priceStr );
    
    // 右揃え用の数値取得
    length = PRINTSYS_GetStrWidth( sbw->expandBuf, sbw->font, 0 );
    bmp_w  = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]) );
    
    // 値段描画
    _callback_print(sbw->printQue, GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]), 
                            bmp_w-length, y, sbw->expandBuf, sbw->font, BLUE_TEXT_COL );
    OS_Printf("y=%d\n", y);
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief １行表示コールバック（主に値段表示処理)
 *
 * @param   wk      
 * @param   param   どうぐNO
 * @param   y       表示Y座標
 */
//----------------------------------------------------------------------------------
static void line_callback_rest(BMPMENULIST_WORK * wk, u32 param, u8 y )
{
  SHOP_BUY_APP_WORK *sbw = BmpMenuList_GetWorkPtr( wk );

  // 「やめる」以外には値段を表示
  if(param!=BMPMENULIST_CANCEL){
    // もう売ってしまった
    if(RESORT_INSTITUTION_DATA_IsBit(sbw->p_institution, param)){
      int length,bmp_w;
      length = PRINTSYS_GetStrWidth( sbw->HaveStr, sbw->font, 0 );
      bmp_w  = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]) );
      // 売れない描画
      _callback_print( sbw->printQue, GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]), 
                            bmp_w-length, y, sbw->HaveStr, sbw->font, BLUE_TEXT_COL );
    // 持ってない
    }else{
      int length,bmp_w;
      // 値段セット
      WORDSET_RegisterNumber( sbw->wordSet, 1, sbw->lineup[param].price, 
                              5, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );  
      WORDSET_ExpandStr( sbw->wordSet, sbw->expandBuf, sbw->priceStr );
      
      // 右揃え用の数値取得
      length = PRINTSYS_GetStrWidth( sbw->expandBuf, sbw->font, 0 );
      bmp_w  = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]) );
      
      // 値段描画
      _callback_print( sbw->printQue, GFL_BMPWIN_GetBmp( sbw->win[SHOP_BUY_BMPWIN_LIST]), 
                            bmp_w-length, y, sbw->expandBuf, sbw->font, BLUE_TEXT_COL );
      
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * @brief BMPLIST終了・解放処理
 *
 * @param   wk    
 */
//----------------------------------------------------------------------------------
static void bmpwin_list_exit( SHOP_BUY_APP_WORK *wk )
{
  u16 list, cursor;
  
  // 
  BmpMenuList_Exit( wk->menuList, &list, &cursor );

}


//----------------------------------------------------------------------------------
/**
 * @brief プリントコールバック
 *
 * @param   param   
 *
 * @retval  BOOL    
 */
//----------------------------------------------------------------------------------
static BOOL printBuyMsgCallBack( u32 param )
{
  if(param==1){
//    PMSND_PlaySE( SEQ_SE_SYS_22 );
  }

  return FALSE;
}


//----------------------------------------------------------------------------------
/**
 * @brief メッセージ表示
 *
 * @param   wk    
 * @param   itemno    
 */
//----------------------------------------------------------------------------------
static void ShopPrintMsgItem( SHOP_BUY_APP_WORK *wk, int strId, u16 itemno )
{
  u16 pocket;
  BOOL is_in = MYITEM_GetRepositoryFlag( wk->myitem, itemno );
  WORDSET_RegisterItemName( wk->wordSet, 0, itemno );


  if( is_in == TRUE )
  {
    pocket = (u16)BAG_POKE_REPOSITORY;
  }
  else
  {
    pocket = (u16)ITEM_GetParam( itemno, ITEM_PRM_POCKET, wk->heapId );
  }

  WORDSET_RegisterItemPocketName( wk->wordSet, 1, pocket);

  ShopPrintMsg( wk, strId );
}

static void ShopPrintMsg( SHOP_BUY_APP_WORK *wk, int strId )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->win[SHOP_BUY_BMPWIN_TALKMSG]), 15 );
  BmpWinFrame_Write( wk->win[SHOP_BUY_BMPWIN_TALKMSG], WINDOW_TRANS_ON, 
                     MENU_WINDOW_CHARA_OFFSET, MENU_WINDOW_PAL_OFFSET );

  {
    STRBUF *str    = GFL_MSG_CreateString( wk->shopMsgData, strId );
    WORDSET_ExpandStr( wk->wordSet, wk->expandBuf, str );
    GFL_STR_DeleteBuffer( str );
  }
  
  wk->printStream = PRINTSYS_PrintStreamCallBack(
    wk->win[SHOP_BUY_BMPWIN_TALKMSG], 0, 0, wk->expandBuf, wk->font,
    MSGSPEED_GetWait(), wk->pMsgTcblSys,
    0, wk->heapId, 0xffff, printBuyMsgCallBack );

  GFL_BMPWIN_MakeTransWindow(wk->win[SHOP_BUY_BMPWIN_TALKMSG]);
}

//----------------------------------------------------------------------------------
/**
 * @brief 文字描画用ウインドウクリア
 *
 * @param   win   
 */
//----------------------------------------------------------------------------------
static void _clear_msg_bmpwin( GFL_BMPWIN *win )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ), 15 );
  BmpWinFrame_Write( win, WINDOW_TRANS_ON, 
                     MENU_WINDOW_CHARA_OFFSET, MENU_WINDOW_PAL_OFFSET );
}

//----------------------------------------------------------------------------------
/**
 * @brief 登録された文字列を展開・描画
 *
 * @param   wk    
 * @param   strId   
 */
//----------------------------------------------------------------------------------
static void _expand_and_print_msg( SHOP_BUY_APP_WORK *wk, int strId )
{
  STRBUF *str = GFL_MSG_CreateString( wk->shopMsgData, strId );
  WORDSET_ExpandStr( wk->wordSet, wk->expandBuf, str );
  GFL_STR_DeleteBuffer( str );
  
  wk->printStream = PRINTSYS_PrintStreamCallBack(
    wk->win[SHOP_BUY_BMPWIN_TALKMSG], 0, 0, wk->expandBuf, wk->font,
    MSGSPEED_GetWait(), wk->pMsgTcblSys,
    0, wk->heapId, 0xffff, NULL );

  GFL_BMPWIN_MakeTransWindow( wk->win[SHOP_BUY_BMPWIN_TALKMSG] );

}

//----------------------------------------------------------------------------------
/**
 * @brief 購入メッセージ表示
 *
 * @param   wk      
 * @param   strId   「●個で○○円（BP)になります」
 * @param   itemno  アイテムNO
 * @param   price   値段（円orBP）
 * @param   num     個数
 */
//----------------------------------------------------------------------------------
static void ShopDecideMsg( SHOP_BUY_APP_WORK *wk, int strId, const RESORT_SHOP_ITEM  *cp_select )
{
  // BMPWINクリア
  _clear_msg_bmpwin( wk->win[SHOP_BUY_BMPWIN_TALKMSG] );

  // 表示文字列登録
  WORDSET_RegisterWord( wk->wordSet, 0, cp_select->p_name, PM_NEUTRAL, TRUE, PM_LANG );
  WORDSET_RegisterNumber( wk->wordSet, 1, cp_select->param2, 
                            2, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );  

  WORDSET_RegisterNumber( wk->wordSet, 2, cp_select->price,
                          SHOP_YEN_KETA_MAX, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );  

  // ウインドウ描画
  _expand_and_print_msg( wk, strId );
}


#define APP_NUMSEL_NONE ( 0 )
#define APP_NUMSEL_UP   ( 1 )
#define APP_NUMSEL_DOWN ( 2 )
//==============================================================================
/**
 * @brief   数値に算出とmax,minに対して回り込み計算を行う
 *
 * @param   num   数値ワークへのポインタ
 * @param   max   数値最大
 * @param   calc  +1,-1,+10,-10のみ受け付ける
 *
 * @retval  u8    計算結果
 */
//==============================================================================
static u8 NumArroundCheck( s16 *num, u16 max, int calc )
{
  s16 tmp;

  tmp = *num;

  switch(calc){
  case -1:
    *num -= 1;
    if( *num <= 0 ){ *num = max; }
    if( *num == tmp ){ return APP_NUMSEL_NONE; }
    return APP_NUMSEL_DOWN;
    break;
  case -10:
    *num -= 10;
    if( *num <= 0 ){ *num = 1; }
    if( *num == tmp ){ return APP_NUMSEL_NONE; }
    return APP_NUMSEL_DOWN;
    break;
  case 1:
    *num += 1;
    if( *num > max ){ *num = 1; }
    if( *num == tmp ){ return APP_NUMSEL_NONE; }
    return APP_NUMSEL_UP;
    break;
  case 10:
    *num += 10;
    if( *num > max ){ *num = max; }
    if( *num == tmp ){ return APP_NUMSEL_NONE; }
    return APP_NUMSEL_UP;
    break;
  }
  return APP_NUMSEL_NONE;
}

//----------------------------------------------------------------------------------
/**
 * @brief サブメニューのスクリーンクリア
 *
 * @param   type    
 */
//----------------------------------------------------------------------------------
static void submenu_screen_clear( int type )
{
  static const clear_rect[][4]={
    { 0,12,32,12 },   // SCREEN_SUBMENU_AND_INFOWIN
    { 0,12,32, 6 },   // SCREEN_SUBMENU_ONLY
  };

  // 指定の範囲のスクリーンクリア
  GFL_BG_FillScreen( GFL_BG_FRAME1_M, 0, 
    clear_rect[type][0], clear_rect[type][1],
    clear_rect[type][2], clear_rect[type][3], 0  );

}

//----------------------------------------------------------------------------
/**
 *	@brief  目の前の人物を取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap フィールド
 *
 *	@return 目の前の人物を取得
 */
//-----------------------------------------------------------------------------
static RESORT_PEOPLE * Resort_Scrcmd_GetFrontPeople( FIELDMAP_WORK *p_fieldmap )
{
  RESORT_PEOPLE_SYS *p_people_sys = NULL;
  {

    if(GMK_TMP_CheckAssignID( p_fieldmap, RESORT_RECEPTION_ASSIGN_ID ) )
    {
      p_people_sys = RESORT_RECEPTION_GetPeopleSys( p_fieldmap );
    }
    else if( GMK_TMP_CheckAssignID( p_fieldmap, RESORT_FIELD_ASSIGN_ID ) )
    {
      p_people_sys = RESORT_FIELD_GetPeopleSys( p_fieldmap );
    }
    GF_ASSERT( p_people_sys != NULL );
  }

  {
    s16 gx, gy, gz;
    FIELD_PLAYER* p_player = FIELDMAP_GetFieldPlayer( p_fieldmap );
    const MMDLSYS * cp_mmdlsys = FIELDMAP_GetMMdlSys( p_fieldmap );
    MMDL* p_frontmmdl;

    // 目の前のグリッド取得
    FIELD_PLAYER_GetFrontGridPos( p_player, &gx, &gy, &gz );

    // 目の前にいる人物を検索
    p_frontmmdl = MMDLSYS_SearchGridPos( cp_mmdlsys, gx, gz, FALSE );

    return RESORT_PEOPLE_SYS_GetPeopleByMmdl( p_people_sys, p_frontmmdl );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  ショップを取得
 *
 *	@param	FIELDMAP_WORK *p_fieldmap   フィールド
 *
 *	@return ショップ
 */
//-----------------------------------------------------------------------------
static RESORT_BINARY_SHOP * Resort_Scrcmd_GetBinaryShop( FIELDMAP_WORK *p_fieldmap )
{
  if( GMK_TMP_CheckAssignID( p_fieldmap, RESORT_FIELD_ASSIGN_ID ) )
  {
    return RESORT_FIELD_GetBinaryShop( p_fieldmap );
  }
  else
  {
    GF_ASSERT(0);
    return NULL;
  }
}

//アイテム・サービスリストの場合
static void ItemList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
  //アイテム読み込み
  obj_load_item( wk );

  //アイテムを陳列
  shop_item_set( wk, type, id );

  //アイテム読み込みコールバックを設定
  bmpwin_list_init( wk, wk->call_back, wk->icon );
}
static void ItemList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
}
static void ItemList_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode)
{
  SHOP_BUY_APP_WORK *sbw = BmpMenuList_GetWorkPtr( wk );
  GFL_CLACTPOS clpos;
  u16 list, cursor;
  
  // カーソル位置取得
  BmpMenuList_PosGet( wk, &list, &cursor );

  clpos.x = shop_obj_param[0].pos_x; 
  clpos.y = shop_obj_param[0].pos_y+16*cursor;

  GFL_CLACT_WK_SetPos( sbw->ClactWork[SHOP_OBJ_CURSOR], &clpos, CLSYS_DEFREND_MAIN );
  
  // どうぐ説明書き換え

  if (param < RESORT_SHOP_ITEM_MAX)
  {
    print_iteminfo( sbw, sbw->lineup[param].p_info );

    if( sbw->lineup[param].serviceID == RESORT_SERVICE_TYPE_ITEM )
    {
      itemicon_resource_change( sbw, sbw->lineup[param].param1 );
    }
    else
    {
      itemicon_resource_change( sbw, RESORT_SHOP_ITEMID_NONE );
    }
  }
  else
  {
    print_iteminfo( sbw, NULL );
    itemicon_resource_change( sbw, RESORT_SHOP_ITEMID_NONE );
  }

}

//秘書リストの場合
static void SecretaryList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
  //アイテム読み込み
  obj_load_people( wk );

  //アイテムを陳列
  shop_secretary_set( wk, type, id );

  //アイテム読み込みコールバックを設定
  bmpwin_list_init( wk, wk->call_back, wk->icon );
}
static void SecretaryList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
}

//施設リストの場合
static void InstitutionList_Init( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
  //アイテム読み込み
  obj_load_people( wk );

  //アイテムを陳列
  shop_institution_set( wk, type, id );

  //アイテム読み込みコールバックを設定
  bmpwin_list_init( wk, wk->call_back, wk->icon );
}
static void InstitutionList_Exit( GAMESYS_WORK *gsys, struct _SHOP_BUY_APP_WORK *wk, int type, int id )
{
}
//----------------------------------------------------------------------------------
/**
 * @brief カーソル移動時のコールバック
 *
 * @param   wk      
 * @param   param   
 * @param   mode    
 */
//----------------------------------------------------------------------------------
static void SelectPoeple_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode)
{
  SHOP_BUY_APP_WORK *sbw = BmpMenuList_GetWorkPtr( wk );
  GFL_CLACTPOS clpos;
  u16 list, cursor;
  
  // カーソル位置取得
  BmpMenuList_PosGet( wk, &list, &cursor );

  clpos.x = shop_obj_param[0].pos_x; 
  clpos.y = shop_obj_param[0].pos_y+16*cursor;

  GFL_CLACT_WK_SetPos( sbw->ClactWork[SHOP_OBJ_CURSOR], &clpos, CLSYS_DEFREND_MAIN );
  
  // 秘書説明書き換え
  if (param < RESORT_SHOP_ITEM_MAX)
  {
    print_iteminfo( sbw, sbw->lineup[param].p_info );
    people_resource_change( sbw, sbw->lineup[param].param1 );
  }
  else
  {
    print_iteminfo( sbw, NULL );
    people_resource_change( sbw, RESORT_SHOP_ITEMID_NONE );
  }
}
static void SortPoeple_move_callback( BMPMENULIST_WORK * wk, u32 param, u8 mode)
{
  SHOP_BUY_APP_WORK *sbw = BmpMenuList_GetWorkPtr( wk );
  GFL_CLACTPOS clpos;
  u16 list, cursor;
  
  // カーソル位置取得
  BmpMenuList_PosGet( wk, &list, &cursor );

  clpos.x = shop_obj_param[0].pos_x; 
  clpos.y = shop_obj_param[0].pos_y+16*cursor;

  GFL_CLACT_WK_SetPos( sbw->ClactWork[SHOP_OBJ_CURSOR], &clpos, CLSYS_DEFREND_MAIN );
  
  // 説明書き換え
  if (param < RESORT_SHOP_ITEM_MAX)
  {
    print_iteminfo( sbw, sbw->lineup[param].p_info );
    people_resource_change( sbw, sbw->lineup[param].param1 );
  }
  else
  {
    print_iteminfo( sbw, NULL );
    people_resource_change( sbw, RESORT_SHOP_ITEMID_NONE );
  }


  if( sbw->is_sort )
  {
    GF_ASSERT( sbw->cp_select != NULL );

    SortPoeple_Cursor_SetPos( sbw );
    GFL_CLACT_WK_SetDrawEnable( sbw->ClactWork[SHOP_OBJ_CURSOR_RED], TRUE );
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( sbw->ClactWork[SHOP_OBJ_CURSOR_RED], FALSE );
  }
}

static void SortPoeple_Cursor_SetPos( SHOP_BUY_APP_WORK *wk )
{
  GFL_CLACTPOS clpos;
  u16 list, cursor;
  s32 insert_y;
  
  // カーソル位置取得
  BmpMenuList_PosGet( wk->menuList, &list, &cursor );
  
  insert_y= (wk->insert_cursor + wk->insert_list - list) 
        * BmpListParamGet( wk->menuList, BMPMENULIST_ID_LINE_YBLK );

  clpos.x = shop_obj_param[0].pos_x; 
  clpos.y = shop_obj_param[0].pos_y+insert_y;
  //以前選択したものの位置
  GFL_CLACT_WK_SetPos( wk->ClactWork[SHOP_OBJ_CURSOR_RED], &clpos, CLSYS_DEFREND_MAIN );

}
static void SecretaryList_Decide( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i;
  int temp_num;
  u8 temp[RESORT_SECRETARY_DATA_SIZE];

  if( !wk->is_change )
  {
    return;
  }

#ifdef PM_DEBUG
  OS_Printf("before\n");
  for( i = 0; i< wk->lineup_num; i++ )
  {
    OS_Printf( "%d ", wk->lineup[i].id );
  }
  OS_Printf("\n");
#endif 

  for( i = 0; i < wk->lineup_num; i++ )
  {
    if( i != wk->lineup[i].id )
    {
      RESORT_SECRETARY_DATA *p_a = RESORT_PEOPLE_ALL_DATA_GetSecretary( wk->p_all_people, wk->lineup[i].id );
      RESORT_PLACEID a_placeID = RESORT_SECRETARY_DATA_GetData( p_a, RESORT_DATA_PARAM_PLACE_ID, NULL );
      u16 a_peopleID = RESORT_SECRETARY_DATA_GetData( p_a, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_SECRETARY_DATA *p_b = RESORT_PEOPLE_ALL_DATA_GetSecretary( wk->p_all_people, i );
      RESORT_PLACEID b_placeID = RESORT_SECRETARY_DATA_GetData( p_b, RESORT_DATA_PARAM_PLACE_ID, NULL );
      u16 b_peopleID = RESORT_SECRETARY_DATA_GetData( p_b, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

      RESORT_SECRETARY_DATA_SetData(p_a, RESORT_DATA_PARAM_PEOPLE_ID, b_peopleID );
      RESORT_SECRETARY_DATA_SetData(p_a, RESORT_DATA_PARAM_PLACE_ID, b_placeID );
      RESORT_SECRETARY_DATA_SetData(p_b, RESORT_DATA_PARAM_PEOPLE_ID, a_peopleID );
      RESORT_SECRETARY_DATA_SetData(p_b, RESORT_DATA_PARAM_PLACE_ID, a_placeID );

      GFL_STD_MemCopy( p_a, temp, RESORT_SECRETARY_DATA_SIZE );
      GFL_STD_MemCopy( p_b, p_a, RESORT_SECRETARY_DATA_SIZE );
      GFL_STD_MemCopy( temp, p_b, RESORT_SECRETARY_DATA_SIZE );


      temp_num = wk->lineup[ wk->lineup[i].id ].id;
      wk->lineup[ wk->lineup[i].id ].id  = wk->lineup[i].id;
      wk->lineup[i].id  = temp_num;
    }
  }

#ifdef PM_DEBUG
  OS_Printf("after\n");
  for( i = 0; i< wk->lineup_num; i++ )
  {
    OS_Printf( "%d ", wk->lineup[i].id );
  }
  OS_Printf("\n");
#endif 
}

static void InstitutionList_Decide( SHOP_BUY_APP_WORK *wk, int type, int id )
{
  int i;
  int temp_num;
  u8 temp[RESORT_RESIDENT_DATA_SIZE];

  if( !wk->is_change )
  {
    return;
  }


#ifdef PM_DEBUG
  OS_Printf("before\n");
  for( i = 0; i < wk->lineup_num; i++ )
  {
    OS_Printf( "%d ", wk->lineup[i].id );
  }
  OS_Printf("\n");
#endif 

  for( i = 0; i < wk->lineup_num; i++ )
  {
    if( i != wk->lineup[i].id )
    {
      RESORT_RESIDENT_DATA *p_a = RESORT_PEOPLE_ALL_DATA_GetResident( wk->p_all_people, wk->lineup[i].id );
      RESORT_PLACEID a_placeID = RESORT_RESIDENT_DATA_GetData( p_a, RESORT_DATA_PARAM_PLACE_ID, NULL );
      u16 a_peopleID = RESORT_RESIDENT_DATA_GetData( p_a, RESORT_DATA_PARAM_PEOPLE_ID, NULL );
      RESORT_RESIDENT_DATA *p_b = RESORT_PEOPLE_ALL_DATA_GetResident( wk->p_all_people, i );
      RESORT_PLACEID b_placeID = RESORT_RESIDENT_DATA_GetData( p_b, RESORT_DATA_PARAM_PLACE_ID, NULL );
      u16 b_peopleID = RESORT_RESIDENT_DATA_GetData( p_b, RESORT_DATA_PARAM_PEOPLE_ID, NULL );

      RESORT_RESIDENT_DATA_SetData(p_a, RESORT_DATA_PARAM_PEOPLE_ID, b_peopleID );
      RESORT_RESIDENT_DATA_SetData(p_a, RESORT_DATA_PARAM_PLACE_ID, b_placeID );
      RESORT_RESIDENT_DATA_SetData(p_b, RESORT_DATA_PARAM_PEOPLE_ID, a_peopleID );
      RESORT_RESIDENT_DATA_SetData(p_b, RESORT_DATA_PARAM_PLACE_ID, a_placeID );

      GFL_STD_MemCopy( p_a, temp, RESORT_RESIDENT_DATA_SIZE );
      GFL_STD_MemCopy( p_b, p_a, RESORT_RESIDENT_DATA_SIZE );
      GFL_STD_MemCopy( temp, p_b, RESORT_RESIDENT_DATA_SIZE );


      temp_num = wk->lineup[ wk->lineup[i].id ].id;
      wk->lineup[ wk->lineup[i].id ].id  = wk->lineup[i].id;
      wk->lineup[i].id  = temp_num;
    }
  }
#ifdef PM_DEBUG
  OS_Printf("after\n");
  for( i = 0; i< wk->lineup_num; i++ )
  {
    OS_Printf( "%d ", wk->lineup[i].id );
  }
  OS_Printf("\n");
#endif 
}
