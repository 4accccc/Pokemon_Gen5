//==============================================================================
/**
 * @file    party_select_list.c
 * @brief   手持ち or バトルボックス 選択リスト
 * @author  matsuda
 * @date    2009.12.18(金)
 */
//==============================================================================
#include <gflib.h>
#include "field_msgbg.h"
#include "pm_define.h"


//==============================================================================
//  定数定義
//==============================================================================
///アクター最大数
#define PSL_ACTOR_MAX   (32)
///セルアクターユニットのプライオリティ
#define PSL_ACTOR_UNIT_PRI  (10)

enum{
  POKEICON_POS_START_X = 32,    ///<ポケモンアイコン配置開始位置X
  POKEICON_POS_SPACE_X = 32,    ///<ポケモンアイコン毎の配置間隔X
  POKEICON_POS_Y = 64,          ///<ポケモンアイコンY座標

  ITEMICON_POS_START_X = 32,    ///<アイテムアイコン配置開始位置X
  ITEMICON_POS_SPACE_X = POKEICON_POS_SPACE_X,    ///<アイテムアイコン毎の配置間隔X
  ITEMICON_POS_Y = 64,          ///<アイテムアイコンY座標
  
  LV_POS_X = 8,
  LV_POS_SPACE_X = POKEICON_POS_SPACE_X,
  LV_POS_Y = 7*8,
  
  SEX_POS_X = 5*8,
  SEX_POS_SPACE_X = POKEICON_POS_SPACE_X,
  SEX_POS_Y = 3*8,
};

///フォント：ノーマル色
#define FONT_NORMAL_COLOR   (PRINTSYS_MACRO_LSB(1,2,0))
///フォント：灰色
#define FONT_GRAY_COLOR     (PRINTSYS_MACRO_LSB(2,1,0))

///ソフトプライオリティ
enum{
  SOFTPRI_ITEMICON = 20,
  SOFTPRI_POKEICON,
};

///BGプライオリティ
enum{
  BGPRI_ITEMICON = 1,
  BGPRI_POKEICON = 1,
};


//==============================================================================
//  構造体定義
//==============================================================================
typedef struct _PARTY_SELECT_LIST{
  FIELD_MAIN_WORK *fieldWork;
  const POKEPARTY *temoti_party;  ///<手持ちパーティ
  const POKEPARTY *bbox_party;    ///<バトルボックスパーティ(作成していない場合はNULL)
  HEAPID heap_id;
  BOOL temoti_reg_fail;           ///<TRUE:手持ちがレギュレーションNG
  BOOL bbox_reg_fail;             ///<TRUE:バトルボックスがレギュレーションNG
  
  GFL_CLUNIT *clunit;
  GFL_CLWK *temoti_clwk[TEMOTI_POKEMAX];
  GFL_CLWK *bbox_clwk[TEMOTI_POKEMAX];
  
  FLDMSGBG *msgBG;
  GFL_MSGDATA *msgdata;
  FLDMENUFUNC *fldmenufunc;
  FLDMSGWIN_STREAM *msgwin_stream;
  FLDMSGWIN *fldmsgwin_poke;
  GFL_FONT *font_handle_small;
}PARTY_SELECT_LIST;


//==============================================================================
//  プロトタイプ宣言
//==============================================================================
static void _PokeIcon_ResourceSet(PARTY_SELECT_LIST_PTR psl);
static void _PokeIcon_ResourceFree(PARTY_SELECT_LIST_PTR psl);
static void _PokeIcon_ActorAdd(PARTY_SELECT_LIST_PTR psl);
static void _PokeIcon_ActorDel(PARTY_SELECT_LIST_PTR psl);
static void _PokeIcon_DrawSet(SELECT_PARTY select_party);
static void _Item_ResourceSet(PARTY_SELECT_LIST_PTR psl);
static void _Item_ResourceFree(PARTY_SELECT_LIST_PTR psl);
static void _Item_ActorAdd(PARTY_SELECT_LIST_PTR psl);
static void _Item_ActorDel(PARTY_SELECT_LIST_PTR psl);
static void _Item_DrawSet(SELECT_PARTY select_party);
static void _Print_SetupFieldMsg(PARTY_SELECT_LIST_PTR psl);
static void _Print_ExitFieldMsg(PARTY_SELECT_LIST_PTR psl);
static void _Print_AddMenuList(PARTY_SELECT_LIST_PTR psl);
static void _Print_DelMenuList(PARTY_SELECT_LIST_PTR psl);
static u32 _Print_SelectMenu(PARTY_SELECT_LIST_PTR psl);
static void _MenuList_CursorCallback(BMPMENULIST_WORK * wk, u32 param, u8 mode);
static void	_MenuList_PrintCallback(BMPMENULIST_WORK * wk, u32 param, u8 y);
static void _Print_AddTalkWin(PARTY_SELECT_LIST_PTR psl);
static void _Print_DelTalkWin(PARTY_SELECT_LIST_PTR psl);
static void _Print_StartStream(PARTY_SELECT_LIST_PTR psl, u32 msg_id);
static BOOL _Print_WaitStream(PARTY_SELECT_LIST_PTR psl);
static void _Print_AddPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl);
static void _Print_DelPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl);
static void _Print_DrawPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl, const POKEPARTY *party);
static void _PokeStatusDraw(PARTY_SELECT_LIST_PTR psl, SELECT_PARTY select_party);


//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  POKEPARTY選択メニューヘッダー
//--------------------------------------------------------------
///POKEPARTY選択メニューリスト
static const FLDMENUFUNC_LIST PokePartySelectMenuList[] =
{
  {msg_psl_select_000, (void*)SELECT_PARTY_TEMOTI},   //手持ち
  {msg_psl_select_001, (void*)SELECT_PARTY_BBOX},     //バトルボックス
  {msg_psl_select_002, (void*)SELECT_PARTY_CANCEL},   //やめる
};

///メニューヘッダー(POKEPARTY選択メニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_PokePartySelect =
{
	3,		//リスト項目数
	3,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	1,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	32-14,		//表示座標X キャラ単位
	24-6-7,		//表示座標Y キャラ単位
	13,		//表示サイズX キャラ単位
	6,		//表示サイズY キャラ単位
};


//==============================================================================
//
//  
//
//==============================================================================

//==================================================================
/**
 * 手持ち or バトルボックス、選択リスト：初期設定
 *
 * @param   fieldWork		
 * @param   temoti		
 * @param   bbox		
 * @param   temoti_reg_fail		
 * @param   bbox_reg_fail		
 * @param   heap_id		
 *
 * @retval  PARTY_SELECT_LIST_PTR		
 */
//==================================================================
PARTY_SELECT_LIST_PTR PARTY_SELECT_LIST_Setup(FIELD_MAIN_WORK *fieldWork, const POKEPARTY *temoti, const POKEPARTY *bbox, BOOL temoti_reg_fail, BOOL bbox_reg_fail, HEAPID heap_id)
{
  PARTY_SELECT_LIST_PTR psl;
  
  psl = GFL_HEAP_AllocClearMemory(heap_id, sizeof(PARTY_SELECT_LIST));
  psl->fieldWork = fieldWork;
  psl->temoti_party = temoti;
  psl->bbox_party = bbox;
  psl->heap_id = heap_id;
  psl->temoti_reg_fail = temoti_reg_fail;
  psl->bbox_reg_fail = bbox_reg_fail;

  psl->clunit = GFL_CLACT_UNIT_Create( PSL_ACTOR_MAX, PSL_ACTOR_UNIT_PRI, psl->heap_id );
  _Print_SetupFieldMsg(psl);

  _PokeIcon_ResourceSet(psl);
  _PokeIcon_ActorAdd(psl);
  _Item_ResourceSet(psl);
  _Item_ActorAdd(psl);
}

//==================================================================
/**
 * 手持ち or バトルボックス、選択リスト：終了処理
 *
 * @param   psl		
 *
 * @retval  SELECT_PARTY		決定した項目
 */
//==================================================================
SELECT_PARTY PARTY_SELECT_LIST_Exit(PARTY_SELECT_LIST_PTR psl)
{
  SELECT_PARTY select_party = psl->return_select;
  
  GF_ASSERT(psl->finish == TRUE); //途中終了は出来ない

  _Item_ActorDel(psl);
  _Item_ResourceFree(psl);
  _PokeIcon_ActorDel(psl);
  _PokeIcon_ResourceFree(psl);
  
  _Print_ExitFieldMsg(psl);
  GFL_CLACT_UNIT_Delete(psl->clunit);
  
  GFL_HEAP_FreeMemory(psl);
  return select_party;
}

//==================================================================
/**
 * 手持ち or バトルボックス、選択リスト：メイン処理
 *
 * @param   psl		
 *
 * @retval  BOOL		TRUE:選択が決定された　FALSE:選択中
 */
//==================================================================
BOOL PARTY_SELECT_LIST_Main(PARTY_SELECT_LIST_PTR psl)
{
  switch(psl->seq){
  case 0:
    _Print_AddTalkWin(psl);
    _Print_StartStream(psl, msg_psl_talk_000);
    psl->seq++;
    break;
  case 1: //メッセージストリーム出力待ち
    if(_Print_WaitStream(psl) == TRUE){
      _Print_AddPokeStatusBmpWin(psl);
      _Print_AddMenuList(psl);  //選択リスト作成
//      _PokeStatusDraw(psl, psl->select_party);  //最初は手持ちの表示
      psl->seq++;
    }
    break;
  case 2: //選択待ち
    {
      u32 menu_ret = _Print_SelectMenu(psl);
      
      if(menu_ret != FLDMENUFUNC_NULL){
        _Print_DelMenuList(psl);
        _Print_DelPokeStatusBmpWin(psl);
        _Print_DelTalkWin(psl);
        switch(menu_ret){
        case FLDMENUFUNC_NULL:
          break;
        case FLDMENUFUNC_CANCEL:
        case SELECT_PARTY_CANCEL:
          psl->return_select = SELECT_PARTY_CANCEL;
          psl->seq++;
          break;
        default:
          psl->return_select = menu_ret;
          psl->seq++;
          break;
        }
      }
    }
    break;
  default:  //終了
    psl->finish = TRUE;
    return TRUE;
  }
  
  return FALSE;
}

//==================================================================
/**
 * 決定した項目を取得する
 *
 * @param   psl		
 *
 * @retval  SELECT_PARTY		
 */
//==================================================================
SELECT_PARTY PARTY_SELECT_LIST_GetSelect(PARTY_SELECT_LIST_PTR psl)
{
  GF_ASSERT(psl->finish == TRUE);
  return psl->return_select;
}


//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * ポケモンアイコンリソース全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ResourceSet(PARTY_SELECT_LIST_PTR psl)
{
  ARCHANDLE *handle;
  int temoti_max, bbox_max;
  POKEMON_PARAM *pp;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_POKEICON, psl->heap_id);

  psl->pokeicon_pltt_index = GFL_CLGRP_PLTT_RegisterEx( handle, POKEICON_GetPalArcIndex(), 
    CLSYS_DRAW_MAIN, FLDOAM_PALNO_FREE_PSL_POKEICON * 0x20, 0, POKEICON_PAL_MAX, psl->heap_id );
  psl->pokeicon_cell_index = GFL_CLGRP_CELLANIM_Register( handle, POKEICON_GetCellArcIndex(), 
    POKEICON_GetAnmArcIndex(), psl->heap_id );
  
  //手持ちCGX
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    pp = PokeParty_GetMemberPointer(psl->temoti_party, i);
    psl->temoti_cgr_index[i] = GFL_CLGRP_CGR_Register( handle, 
      POKEICON_GetCgxArcIndex( PP_GetPPPPointer(pp) ), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );
  }
  
  //バトルボックスCGX
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      pp = PokeParty_GetMemberPointer(psl->bbox_party, i);
      psl->bbox_cgr_index[i] = GFL_CLGRP_CGR_Register( handle, 
        POKEICON_GetCgxArcIndex( PP_GetPPPPointer(pp) ), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );
    }
  }

  GFL_ARC_CloseDataHandle(handle);
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンリソース全解放
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ResourceFree(PARTY_SELECT_LIST_PTR psl)
{
  int i;
  
  //バトルボックスCGX
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      GFL_CLGRP_CGR_Release(psl->bbox_cgr_index[i]);
    }
  }
  
  //手持ちCGX
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    GFL_CLGRP_CGR_Release(psl->temoti_cgr_index[i]);
  }
  
  GFL_CLGRP_CELLANIM_Release(psl->pokeicon_cell_index);
  GFL_CLGRP_PLTT_Release(psl->pokeicon_pltt_index);
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンアクター全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ActorAdd(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;
  GFL_CLWK_DATA head = {
    0, POKEICON_POS_Y, 
    0,
    SOFTPRI_POKEICON,
    BGPRI_POKEICON,
  };
  
  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    head.pos_x = POKEICON_POS_START_X + POKEICON_POS_SPACE_X * i;
    psl->temoti_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->temoti_cgr_index[i], 
      psl->pokeicon_pltt_index, psl->pokeicon_cell_index, 
      &head, CLSYS_DRAW_MAIN, psl->heap_id);
    GFL_CLACT_WK_SetDrawEnable( psl->temoti_clwk[i], FALSE );
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      head.pos_x = POKEICON_POS_START_X + POKEICON_POS_SPACE_X * i;
      psl->bbox_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->bbox_cgr_index[i], 
        psl->pokeicon_pltt_index, psl->pokeicon_cell_index, 
        &head, CLSYS_DRAW_MAIN, psl->heap_id);
      GFL_CLACT_WK_SetDrawEnable( psl->bbox_clwk[i], FALSE );
    }
  }
}

//--------------------------------------------------------------
/**
 * ポケモンアイコンアクター全削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _PokeIcon_ActorDel(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;

  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    GFL_CLACT_WK_Remove(psl->temoti_clwk[i]);
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      GFL_CLACT_WK_Remove(psl->bbox_clwk[i]);
    }
  }
}

//--------------------------------------------------------------
/**
 * ポケモンアイコン：表示ON
 *
 * @param   select_party		
 */
//--------------------------------------------------------------
static void _PokeIcon_DrawSet(SELECT_PARTY select_party)
{
  int i;

  for(i = 0; i < TEMOTI_POKEMAX; i++){
    if(psl->temoti_clwk[i] != NULL){
      GFL_CLACT_WK_SetDrawEnable( psl->temoti_clwk[i], FALSE );
    }
    if(psl->bbox_clwk[i] != NULL){
      GFL_CLACT_WK_SetDrawEnable( psl->bbox_clwk[i], FALSE );
    }
  }
  
  if(select_party == SELECT_PARTY_TEMOTI){
    for(i = 0; i < TEMOTI_POKEMAX; i++){
      if(psl->temoti_clwk[i] != NULL){
        GFL_CLACT_WK_SetDrawEnable( psl->temoti_clwk[i], TRUE );
      }
    }
  }
  else{
    for(i = 0; i < TEMOTI_POKEMAX; i++){
      if(psl->bbox_clwk[i] != NULL){
        GFL_CLACT_WK_SetDrawEnable( psl->bbox_clwk[i], TRUE );
      }
    }
  }
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * 道具アイコンリソース全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ResourceSet(PARTY_SELECT_LIST_PTR psl)
{
  ARCHANDLE *handle;
  
  handle = GFL_ARC_OpenDataHandle(ARCID_APP_MENU_COMMON, psl->heap_id);

  psl->item_pltt_index = GFL_CLGRP_PLTT_RegisterEx( handle, 
    APP_COMMON_GetPokeItemIconPltArcIdx(), 
    CLSYS_DRAW_MAIN, FLDOAM_PALNO_FREE_PSL_ITEM * 0x20, 0, APP_COMMON_ITEMICON_PLT_NUM, 
    psl->heap_id );
  
  psl->item_cell_index = GFL_CLGRP_CELLANIM_Register( handle, 
    APP_COMMON_GetPokeItemIconCellArcIdx(APP_COMMON_MAPPING_64K), 
    APP_COMMON_GetPokeItemIconAnimeArcIdx(APP_COMMON_MAPPING_64K), psl->heap_id );
  
  psl->item_cgr_index = GFL_CLGRP_CGR_Register( handle, 
    APP_COMMON_GetPokeItemIconCharArcIdx(), FALSE, CLSYS_DRAW_MAIN, psl->heap_id );

  GFL_ARC_CloseDataHandle(handle);
}

//--------------------------------------------------------------
/**
 * 道具アイコンリソース全解放
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ResourceFree(PARTY_SELECT_LIST_PTR psl)
{
  GFL_CLGRP_CGR_Release(psl->item_cgr_index);
  GFL_CLGRP_CELLANIM_Release(psl->item_cell_index);
  GFL_CLGRP_PLTT_Release(psl->item_pltt_index);
}

//--------------------------------------------------------------
/**
 * 道具アイコンアクター全登録
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ActorAdd(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max, item;
  GFL_CLWK_DATA head = {
    0, ITEMICON_POS_Y, 
    APP_COMMON_ITEMICON_ITEM,   //anm_seq
    SOFTPRI_ITEMICON,
    BGPRI_ITEMICON,
  };
  
  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    if(item == 0){
      continue;
    }
    else if(ITEM_CheckMail( item ) == TRUE){
      head.anm_seq = APP_COMMON_ITEMICON_MAIL;
    }
    else{
      head.anm_seq = APP_COMMON_ITEMICON_ITEM;
    }
    head.pos_x = ITEMICON_POS_START_X + ITEMICON_POS_SPACE_X * i;
    psl->item_temoti_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->item_cgr_index, 
      psl->item_pltt_index, psl->item_cell_index, 
      &head, CLSYS_DRAW_MAIN, psl->heap_id);
    GFL_CLACT_WK_SetDrawEnable( psl->item_temoti_clwk[i], FALSE );
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      if(item == 0){
        continue;
      }
      else if(ITEM_CheckMail( item ) == TRUE){
        head.anm_seq = APP_COMMON_ITEMICON_MAIL;
      }
      else{
        head.anm_seq = APP_COMMON_ITEMICON_ITEM;
      }
      head.pos_x = ITEMICON_POS_START_X + ITEMICON_POS_SPACE_X * i;
      psl->item_bbox_clwk[i] = GFL_CLACT_WK_Create( psl->clunit, psl->item_cgr_index, 
        psl->item_pltt_index, psl->item_cell_index, 
        &head, CLSYS_DRAW_MAIN, psl->heap_id);
      GFL_CLACT_WK_SetDrawEnable( psl->item_bbox_clwk[i], FALSE );
    }
  }
}

//--------------------------------------------------------------
/**
 * 道具アイコンアクター全削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Item_ActorDel(PARTY_SELECT_LIST_PTR psl)
{
  int i, temoti_max, bbox_max;

  //手持ち
  temoti_max = PokeParty_GetPokeCount(psl->temoti_party);
  for(i = 0; i < temoti_max; i++){
    if(psl->item_temoti_clwk[i] != NULL){
      GFL_CLACT_WK_Remove(psl->item_temoti_clwk[i]);
    }
  }
  
  //バトルボックス
  if(psl->bbox_party != NULL){
    bbox_max = PokeParty_GetPokeCount(psl->bbox_party);
    for(i = 0; i < bbox_max; i++){
      if(psl->item_bbox_clwk[i] != NULL){
        GFL_CLACT_WK_Remove(psl->item_bbox_clwk[i]);
      }
    }
  }
}

//--------------------------------------------------------------
/**
 * 道具アイコン：表示ON
 *
 * @param   select_party		
 */
//--------------------------------------------------------------
static void _Item_DrawSet(SELECT_PARTY select_party)
{
  int i;

  for(i = 0; i < TEMOTI_POKEMAX; i++){
    if(psl->item_temoti_clwk[i] != NULL){
      GFL_CLACT_WK_SetDrawEnable( psl->temoti_clwk[i], FALSE );
    }
    if(psl->item_bbox_clwk[i] != NULL){
      GFL_CLACT_WK_SetDrawEnable( psl->bbox_clwk[i], FALSE );
    }
  }
  
  if(select_party == SELECT_PARTY_TEMOTI){
    for(i = 0; i < TEMOTI_POKEMAX; i++){
      if(psl->item_temoti_clwk[i] != NULL){
        GFL_CLACT_WK_SetDrawEnable( psl->temoti_clwk[i], TRUE );
      }
    }
  }
  else{
    for(i = 0; i < TEMOTI_POKEMAX; i++){
      if(psl->item_bbox_clwk[i] != NULL){
        GFL_CLACT_WK_SetDrawEnable( psl->bbox_clwk[i], TRUE );
      }
    }
  }
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * プリント制御セットアップ
 *
 * @param   iem		
 * @param   gsys		
 */
//--------------------------------------------------------------
static void _Print_SetupFieldMsg(PARTY_SELECT_LIST_PTR psl)
{
  FLDMSGBG *msgBG = FIELDMAP_GetFldMsgBG(psl->fieldWork);
  
  psl->msgdata = FLDMSGBG_CreateMSGDATA( msgBG, NARC_message_party_select_list_dat );
  psl->msgBG = msgBG;
}

//--------------------------------------------------------------
/**
 * プリント制御削除
 *
 * @param   iem		
 */
//--------------------------------------------------------------
static void _Print_ExitFieldMsg(PARTY_SELECT_LIST_PTR psl)
{
  FLDMSGBG_DeleteMSGDATA(psl->msgdata);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * メニューリスト追加
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_AddMenuList(PARTY_SELECT_LIST_PTR psl)
{
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;

  fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
    PokePartySelectMenuList, NELEMS(PokePartySelectMenuList), psl->msgdata, psl->heap_id);

  psl->fldmenufunc = FLDMENUFUNC_AddMenuListEx( psl->msgBG,
  	MenuHeader_PokePartySelect, fldmenu_listdata,
    0, 0, _MenuList_CursorCallback, _MenuList_PrintCallback, psl);
}

//--------------------------------------------------------------
/**
 * メニューリスト削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_DelMenuList(PARTY_SELECT_LIST_PTR psl)
{
  FLDMENUFUNC_DeleteMenu(psl->fldmenufunc);
}

//--------------------------------------------------------------
/**
 * メニューリスト選択待ち
 *
 * @param   psl		
 *
 * @retval  u32		結果
 */
//--------------------------------------------------------------
static u32 _Print_SelectMenu(PARTY_SELECT_LIST_PTR psl)
{
  return FLDMENUFUNC_ProcMenu(psl->fldmenufunc);
}

//--------------------------------------------------------------
/**
 * カーソル動作毎のコールバック関数
 *
 * @param   wk		
 * @param   param		
 * @param   mode		初期化時 mode = 1,通常動作時 mode = 0
 */
//--------------------------------------------------------------
static void _MenuList_CursorCallback(BMPMENULIST_WORK * wk, u32 param, u8 mode)
{
  PARTY_SELECT_LIST_PTR psl = BmpMenuList_GetWorkPtr(wk);
  
  if(param < SELECT_PARTY_CANCEL){
    _PokeStatusDraw(PARTY_SELECT_LIST_PTR psl, SELECT_PARTY select_party);
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト1列表示毎のコールバック関数
 *
 * @param   wk		
 * @param   param		
 * @param   y		
 *
 * @retval  void			
 */
//--------------------------------------------------------------
static void	_MenuList_PrintCallback(BMPMENULIST_WORK * wk, u32 param, u8 y)
{
  PARTY_SELECT_LIST_PTR psl = BmpMenuList_GetWorkPtr(wk);
  
  if(y == 0 && psl->temoti_reg_fail == TRUE){
    BmpMenuList_TmpColorChange( wk, PRINTSYS_LSB_GetL(FONT_GRAY_COLOR), 
      PRINTSYS_LSB_GetS(FONT_GRAY_COLOR), PRINTSYS_LSB_GetB(FONT_GRAY_COLOR) );
  }
  else if(y == 1 && psl->bbox_reg_fail == TRUE){
    BmpMenuList_TmpColorChange( wk, PRINTSYS_LSB_GetL(FONT_GRAY_COLOR), 
      PRINTSYS_LSB_GetS(FONT_GRAY_COLOR), PRINTSYS_LSB_GetB(FONT_GRAY_COLOR) );
  }
  else{
    BmpMenuList_TmpColorChange( wk, PRINTSYS_LSB_GetL(FONT_NORMAL_COLOR), 
      PRINTSYS_LSB_GetS(FONT_NORMAL_COLOR), PRINTSYS_LSB_GetB(FONT_NORMAL_COLOR) );
  }
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * プリントストリーム用の会話ウィンドウを追加
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_AddTalkWin(PARTY_SELECT_LIST_PTR psl)
{
  psl->msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin( psl->msgBG, psl->msgdata );
}

//--------------------------------------------------------------
/**
 * プリントストリーム用の会話ウィンドウを削除
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_DelTalkWin(PARTY_SELECT_LIST_PTR psl)
{
  FLDMSGWIN_STREAM_Delete(psl->msgwin_stream);
}

//--------------------------------------------------------------
/**
 * プリントストリーム：出力開始
 *
 * @param   psl		
 * @param   msg_id		メッセージID
 */
//--------------------------------------------------------------
static void _Print_StartStream(PARTY_SELECT_LIST_PTR psl, u32 msg_id)
{
  FLDMSGWIN_STREAM_PrintStart(psl->msgwin_stream, 0, 0, msg_id);
}

//--------------------------------------------------------------
/**
 * プリントストリーム：出力待ち
 * @param   psl		
 * @retval  BOOL		TRUE:終了　FALSE:出力中
 */
//--------------------------------------------------------------
static BOOL _Print_WaitStream(PARTY_SELECT_LIST_PTR psl)
{
  return FLDMSGWIN_STREAM_Print(psl->msgwin_stream);
}

//==============================================================================
//  
//==============================================================================
//--------------------------------------------------------------
/**
 * ポケモンステータス欄のBMPWIN作成
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_AddPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl)
{
  psl->fldmsgwin_poke = FLDMSGWIN_Add(psl->msgBG, psl->msgdata, bmp_x, bmp_y, size_x, size_y);
  psl->font_handle_small = GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, 
    GFL_FONT_LOADTYPE_FILE, FALSE, psl->heap_id );

}

//--------------------------------------------------------------
/**
 * ポケモンステータス欄のBMPWIN削除
 *
 * @param   psl		
 */
//--------------------------------------------------------------
static void _Print_DelPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl)
{
  FLDMSGWIN_Delete(psl->fldmsgwin_poke);
  GFL_FONT_Delete(psl->font_handle_small);
}

//--------------------------------------------------------------
/**
 * 指定POKEPARTYのステータス情報を書き込む
 *
 * @param   psl		
 * @param   party		
 */
//--------------------------------------------------------------
static void _Print_DrawPokeStatusBmpWin(PARTY_SELECT_LIST_PTR psl, const POKEPARTY *party)
{
  STRBUF * buf_lv, buf_sex, tempbuf;
  int i, party_max;
  u32 monsno, level, sex, nidoran_nickname;
  POKEMON_PARAM *pp;
  
  FLDMSGWIN_ClearWindow(psl->fldmsgwin_poke);
  
  buf_lv = GFL_STR_CreateBuffer(64, psl->heap_id);
  tempbuf = GFL_STR_CreateBuffer(64, psl->heap_id);
  
  //レベル＆性別描画
  GFL_MSG_GetString( msgdata, msg_psl_lv, tempbuf );
  WORDSET_ExpandStr( psl->wordset, buf_lv, tempbuf );

  GFL_MSG_GetString( msgdata, msg_psl_lv, tempbuf );
  WORDSET_ExpandStr( psl->wordset, buf_lv, tempbuf );

  party_max = PokeParty_GetPokeCount(party);
  for(i = 0; i < party_max; i++){
    pp = PokeParty_GetMemberPointer(party, i);
    monsno = PP_Get( pp, ID_PARA_monsno, NULL);
    nidoran_nickname = PP_Get(pp, ID_PARA_nidoran_nickname, NULL);
    level = PP_Get( pp, ID_PARA_level, NULL);
    sex = PP_Get( pp, ID_PARA_sex, NULL);

    //レベル
    GFL_MSG_GetString( psl->msgdata, msg_psl_lv, tempbuf );
    WORDSET_RegisterNumber( psl->wordset, 0, level, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( psl->wordset, buf_lv, tempbuf );
    FLDMSGWIN_PrintStrBufColorFontHandle( psl->fldmsgwin_poke, 
      LV_POS_X + LV_POS_SPACE_X*i, LV_POS_Y, buf_lv, FONT_NORMAL_COLOR, 
      psl->font_handle_small );
    
    //性別
    if(sex == PM_NEUTRAL 
        || ((monsno==MONSNO_NIDORAN_F || monsno==MONSNO_NIDORAN_M) && nidoran_nickname==FALSE)){
      ; //何も描画しない
    }
    else if(sex == PM_MALE){
      FLDMSGWIN_Print(psl->fldmsgwin_poke, SEX_POS_X+SEX_POS_SPACE_X*i, SEX_POS_Y, msg_psl_male);
    }
    else{
      FLDMSGWIN_Print(psl->fldmsgwin_poke, SEX_POS_X+SEX_POS_SPACE_X*i,SEX_POS_Y, msg_psl_female);
    }
  }
  
  GFL_STR_DeleteBuffer(buf_lv);
  GFL_STR_DeleteBuffer(tempbuf);
}

//--------------------------------------------------------------
/**
 * 手持ちかバトルボックス、選択した方のポケモンステータス表示を一括ON
 *
 * @param   psl		
 * @param   select_party		
 */
//--------------------------------------------------------------
static void _PokeStatusDraw(PARTY_SELECT_LIST_PTR psl, SELECT_PARTY select_party)
{
  const POKEPARTY *party;
  
  OS_TPrintf("select_party = %d\n", select_party);
  
  if(select_party == SELECT_PARTY_TEMOTI){
    party = psl->temoti_party;
  }
  else{
    party = psl->bbox_party;
  }
  
  _Print_DrawPokeStatusBmpWin(psl, party);
  _PokeIcon_DrawSet(select_party);
  _Item_DrawSet(select_party);
}

