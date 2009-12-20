//==============================================================================
/**
 * @file    union_msg.c
 * @brief   ユニオンルーム：ツール類(オーバーレイ領域に配置されます)
 * @author  matsuda
 * @date    2009.07.13(月)
 */
//==============================================================================
#include <gflib.h>
#include "union_types.h"
#include "union_local.h"
#include "net_app/union/union_main.h"
#include "field/field_player.h"
#include "field/fieldmap.h"
#include "net_app/union/union_msg.h"
#include "message.naix"
#include "msg/msg_union_room.h"
#include "system/main.h"


//==============================================================================
//  定数定義
//==============================================================================
///レギュレーション：決定
#define UNION_BATTLE_REGULATION_DECIDE    (100)
///レギュレーション：ルールをみる
#define UNION_BATTLE_REGULATION_RULE      (150)

///レギュレーション：対戦形式
enum{
  UNION_BATTLE_REGULATION_MODE_SINGLE,      ///<シングル
  UNION_BATTLE_REGULATION_MODE_DOUBLE,      ///<ダブル
  UNION_BATTLE_REGULATION_MODE_TRIPLE,      ///<トリプル
  UNION_BATTLE_REGULATION_MODE_ROTATION,    ///<ローテーション
  
  UNION_BATTLE_REGULATION_MODE_MAX,
};

///レギュレーション：ルール
enum{
  UNION_BATTLE_REGULATION_RULE_FREE,        ///<制限無し
  UNION_BATTLE_REGULATION_RULE_FLAT,    ///<フラット
  
  UNION_BATTLE_REGULATION_RULE_MAX,
};

///レギュレーション：シューター
enum{
  UNION_BATTLE_REGULATION_SHOOTER_TRUE,     ///<シューター使用する
  UNION_BATTLE_REGULATION_SHOOTER_FALSE,    ///<シューター禁止
  
  UNION_BATTLE_REGULATION_SHOOTER_MAX,
};

///レギュレーション：ノーマル色
#define REG_NORMAL_COLOR      PRINTSYS_MACRO_LSB(1, 2, 0)
///レギュレーション：NG色
#define REG_FAIL_COLOR        PRINTSYS_MACRO_LSB(4, 2, 0)


//==============================================================================
//  プロトタイプ宣言：データ
//==============================================================================
static const FLDMENUFUNC_LIST BattleMenuList_Number[3];
static const FLDMENUFUNC_LIST BattleMenuList_Custom[6];
static const FLDMENUFUNC_LIST BattleMenuList_Mode[5];
static const FLDMENUFUNC_LIST BattleMenuList_Rule[3];
static const FLDMENUFUNC_LIST BattleMenuList_Shooter[3];


//==============================================================================
//  データ
//==============================================================================
//--------------------------------------------------------------
//  各ゲームの最大参加人数
//--------------------------------------------------------------
static const struct{
  u8 member_max;
  u8 padding[3];
}MenuMemberMax[] = {
  {2},      //UNION_PLAY_CATEGORY_UNION,          ///<ユニオンルーム
  {2},      //UNION_PLAY_CATEGORY_TALK,           ///<会話中
  {2},      //UNION_PLAY_CATEGORY_TRAINERCARD,    ///<トレーナーカード
  {4},      //UNION_PLAY_CATEGORY_PICTURE,        ///<お絵かき
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE,       ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER,   ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT,   ///<コロシアム
  {4},      //UNION_PLAY_CATEGORY_COLOSSEUM_MULTI,      ///<コロシアム
  {2},      //UNION_PLAY_CATEGORY_TRADE,          ///<ポケモン交換
  {4},      //UNION_PLAY_CATEGORY_GURUGURU,       ///<ぐるぐる交換
  {5},      //UNION_PLAY_CATEGORY_RECORD,         ///<レコードコーナー
};

//--------------------------------------------------------------
//  メニューヘッダー
//--------------------------------------------------------------
///メインメニューリスト
static const FLDMENUFUNC_LIST MainMenuList[] =
{
  {msg_union_select_01, (void*)UNION_PLAY_CATEGORY_TRAINERCARD},   //挨拶
  {msg_union_select_04, (void*)UNION_PLAY_CATEGORY_TRADE},   //交換
  {msg_union_select_03, (void*)UNION_MENU_NO_SEND_BATTLE},   //対戦
  {msg_union_select_06, (void*)UNION_PLAY_CATEGORY_GURUGURU}, //ぐるぐる交換
  {msg_union_select_07, (void*)UNION_PLAY_CATEGORY_RECORD},   //レコードコーナー
  {msg_union_select_02, (void*)UNION_PLAY_CATEGORY_PICTURE},   //お絵かき
  {msg_union_select_05, (void*)UNION_MENU_SELECT_CANCEL},   //やめる
};

///メニューヘッダー(メインメニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_MainMenu =
{
	7,		//リスト項目数
	7,		//表示最大項目数
	0,		//ラベル表示Ｘ座標
	13,		//項目表示Ｘ座標
	0,		//カーソル表示Ｘ座標
	0,		//表示Ｙ座標
	1,		//表示文字色
	15,		//表示背景色
	2,		//表示文字影色
	0,		//文字間隔Ｘ
	0,		//文字間隔Ｙ
	FLDMENUFUNC_SKIP_NON,	//ページスキップタイプ
	12,		//文字サイズX(ドット
	16,		//文字サイズY(ドット
	32-14,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	13,		//表示サイズX キャラ単位
	14,		//表示サイズY キャラ単位
};
//SDK_COMPILER_ASSERT(NELEMS(MainMenuList) == MenuHeader_MainMenu.line);

//--------------------------------------------------------------
//  対戦メニュー
//--------------------------------------------------------------
///二人対戦、四人対戦、やめる
static const FLDMENUFUNC_LIST BattleMenuList_Number[] =
{
  {msg_union_battle_01_01, (void*)BattleMenuList_Custom},   //二人対戦
  {msg_union_battle_01_02, (void*)UNION_PLAY_CATEGORY_COLOSSEUM_MULTI},    //四人対戦
  {msg_union_battle_01_03, (void*)FLDMENUFUNC_CANCEL},    //やめる
};

///二人対戦：カスタムメニュー
static const FLDMENUFUNC_LIST BattleMenuList_Custom[] =
{
  {msg_union_battle_01_04, (void*)BattleMenuList_Mode},      //対戦形式(シングル、ダブル…)
  {msg_union_battle_01_05, (void*)BattleMenuList_Rule},       //ルール(レベル50、制限無し…)
  {msg_union_battle_01_06, (void*)BattleMenuList_Shooter},   //シューター有無
  {msg_union_battle_01_07_01, (void*)UNION_BATTLE_REGULATION_DECIDE},    //決定
  {msg_union_battle_01_07_02, (void*)UNION_BATTLE_REGULATION_RULE},    //ルールをみる
  {msg_union_battle_01_08, (void*)BattleMenuList_Number},    //もどる
};

///二人対戦：シングル、ダブル、トリプル、ローテーション、もどる
static const FLDMENUFUNC_LIST BattleMenuList_Mode[] =
{
  {msg_union_battle_01_04, (void*)UNION_BATTLE_REGULATION_MODE_SINGLE},   //シングル
  {msg_union_battle_01_05, (void*)UNION_BATTLE_REGULATION_MODE_DOUBLE},   //ダブル
  {msg_union_battle_01_06, (void*)UNION_BATTLE_REGULATION_MODE_TRIPLE},   //トリプル
  {msg_union_battle_01_07, (void*)UNION_BATTLE_REGULATION_MODE_ROTATION}, //ローテーション
  {msg_union_battle_01_08, (void*)BattleMenuList_Custom},   //もどる
};

///二人対戦：制限なし、フラット、もどる
static const FLDMENUFUNC_LIST BattleMenuList_Rule[] =
{
  {msg_union_battle_01_10, (void*)UNION_BATTLE_REGULATION_RULE_FREE},     //制限なし
  {msg_union_battle_01_11, (void*)UNION_BATTLE_REGULATION_RULE_FLAT}, //フラット
  {msg_union_battle_01_12, (void*)BattleMenuList_Custom},   //もどる
};

///二人対戦：シューターあり、シューター無し
static const FLDMENUFUNC_LIST BattleMenuList_Shooter[] =
{
  {msg_union_battle_01_11_02, (void*)UNION_BATTLE_REGULATION_RULE_FREE},     //制限なし
  {msg_union_battle_01_12,    (void*)BattleMenuList_Custom},   //もどる
};

///対戦メニューのデータテーブル ※BATTLE_MENU_INDEX_???と並びを同じにしておくこと！
static const struct{
  const FLDMENUFUNC_LIST *list;
  u8 list_max;
}BattleMenuDataTbl[] = {
  {
    BattleMenuList_Number,
    3,
  },
  {
    BattleMenuList_Custom,
    6,
  },
  {
    BattleMenuList_Mode,
    5,
  },
  {
    BattleMenuList_Rule,
    3,
  },
  {
    BattleMenuList_Shooter,
    3,
  },
};
enum{ //※BattleMenuDataTblと並びを同じにしておくこと！
  BATTLE_MENU_INDEX_NUMBER,
  BATTLE_MENU_INDEX_CUSTOM,
  BATTLE_MENU_INDEX_MODE,
  BATTLE_MENU_INDEX_RULE,
  BATTLE_MENU_INDEX_SHOOTER,
};

///メニューヘッダー(対戦メニュー用)
static const FLDMENUFUNC_HEADER MenuHeader_Battle =
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
	32-20,		//表示座標X キャラ単位
	1,		//表示座標Y キャラ単位
	19,		//表示サイズX キャラ単位
	14,		//表示サイズY キャラ単位
};
//SDK_COMPILER_ASSERT(NELEMS(MainMenuList) == MenuHeader_MainMenu.line);

//--------------------------------------------------------------
//  POKEPARTY選択メニューヘッダー
//--------------------------------------------------------------
///POKEPARTY選択メニューリスト
static const FLDMENUFUNC_LIST PokePartySelectMenuList[] =
{
  {msg_union_battle_01_23, (void*)COLOSSEUM_SELECT_PARTY_TEMOTI},  //手持ち
  {msg_union_battle_01_24, (void*)COLOSSEUM_SELECT_PARTY_BOX},     //バトルボックス
  {msg_union_battle_01_25, (void*)COLOSSEUM_SELECT_PARTY_CANCEL},  //やめる
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

//--------------------------------------------------------------
///  メニュー選択レギュレーションをゲームカテゴリーに変換するテーブル
//--------------------------------------------------------------
ALIGN4 static const u8 MenuReg_to_PlayCategory[UNION_BATTLE_REGULATION_MODE_MAX][UNION_BATTLE_REGULATION_RULE_MAX][UNION_BATTLE_REGULATION_SHOOTER_MAX] = {
  {//UNION_BATTLE_REGULATION_MODE_SINGLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_SINGLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_DOUBLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_DOUBLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_TRIPLE
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_TRIPLE_FLAT,
    },
  },
  {//UNION_BATTLE_REGULATION_MODE_ROTATION
    {//UNION_BATTLE_REGULATION_RULE_FREE
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FREE,
    },
    {//UNION_BATTLE_REGULATION_RULE_FLAT
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT_SHOOTER,
      UNION_PLAY_CATEGORY_COLOSSEUM_1VS1_ROTATION_FLAT,
    },
  },
};

//==============================================================================
//
//  
//
//==============================================================================
//==================================================================
/**
 * 各メニューで選択したゲームの通信最大参加人数を取得する
 *
 * @param   menu_index		UNION_MSG_MENU_SELECT_???
 *
 * @retval  int		最大参加人数
 */
//==================================================================
int UnionMsg_GetMemberMax(UNION_PLAY_CATEGORY menu_index)
{
  if(menu_index >= UNION_PLAY_CATEGORY_MAX){
    GF_ASSERT(menu_index < UNION_PLAY_CATEGORY_MAX);
    return 0;
  }
  return MenuMemberMax[menu_index].member_max;
}

//==================================================================
/**
 * UnionMsg系のものを全て一括削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_AllDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_YesNo_Del(unisys);
  UnionMsg_Menu_MainMenuDel(unisys);
  UnionMsg_TalkStream_WindowDel(unisys);
}

//==================================================================
/**
 * 会話ウィンドウ：セットアップ
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;
  
  if(unisys->fld_msgwin_stream == NULL){
    GF_ASSERT(unisys->msgdata == NULL);
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->msgdata = FLDMSGBG_CreateMSGDATA( fldmsg_bg, NARC_message_union_room_dat );
    unisys->fld_msgwin_stream = FLDMSGWIN_STREAM_AddTalkWin(fldmsg_bg, unisys->msgdata);
  }
  else{ //既にウィンドウが表示されている場合はメッセージエリアのクリアを行う
    FLDMSGWIN_STREAM_ClearMessage(unisys->fld_msgwin_stream);
  }
}

//==================================================================
/**
 * 会話ウィンドウ：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_TalkStream_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream != NULL){
    FLDMSGWIN_STREAM_ClearWindow(unisys->fld_msgwin_stream);
    FLDMSGWIN_STREAM_Delete(unisys->fld_msgwin_stream);
    unisys->fld_msgwin_stream = NULL;
    unisys->fld_msgwin_stream_print_on = FALSE;
  }

  if(unisys->msgdata != NULL){
    FLDMSGBG_DeleteMSGDATA(unisys->msgdata);
    unisys->msgdata = NULL;
  }
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始
 *
 * @param   unisys		
 * @param   str_id		メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_Print(UNION_SYSTEM_PTR unisys, u32 str_id)
{
  GF_ASSERT(unisys->fld_msgwin_stream != NULL);
  FLDMSGWIN_STREAM_PrintStart(unisys->fld_msgwin_stream, 0, 0, str_id);
  unisys->fld_msgwin_stream_print_on = TRUE;
}

//==================================================================
/**
 * 会話ウィンドウ：終了チェック
 *
 * @param   unisys		
 *
 * @retval  BOOL		TRUE:終了。　FALSE:継続中
 */
//==================================================================
BOOL UnionMsg_TalkStream_Check(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fld_msgwin_stream == NULL || unisys->fld_msgwin_stream_print_on == FALSE){
    return TRUE;
  }
  return FLDMSGWIN_STREAM_Print(unisys->fld_msgwin_stream);
}

//==================================================================
/**
 * 会話ウィンドウ：出力開始に加え、メッセージウィンドウが無い場合は生成も行う
 *
 * @param   unisys		  
 * @param   fieldWork		
 * @param   str_id		  メッセージID
 */
//==================================================================
void UnionMsg_TalkStream_PrintPack(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, u32 str_id)
{
  UnionMsg_TalkStream_WindowSetup(unisys, fieldWork);
  UnionMsg_TalkStream_Print(unisys, str_id);
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウセットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_YesNo_Setup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg;

  if(unisys->fldmenu_yesno_func == NULL){
    fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
    unisys->fldmenu_yesno_func = FLDMENUFUNC_AddYesNoMenu(fldmsg_bg, FLDMENUFUNC_YESNO_YES);
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択ウィンドウ削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_YesNo_Del(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_yesno_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_yesno_func);
    unisys->fldmenu_yesno_func = NULL;
  }
}

//==================================================================
/**
 * 「はい」「いいえ」選択待ち
 *
 * @param   unisys		
 * @param   result		結果代入先(TRUE:はい、FALSE:いいえ)
 *
 * @retval  BOOL		TRUE:選択した。　FALSE:選択中
 */
//==================================================================
BOOL UnionMsg_YesNo_SelectLoop(UNION_SYSTEM_PTR unisys, BOOL *result)
{
  FLDMENUFUNC_YESNO yes_no;
  
  GF_ASSERT(unisys->fldmenu_yesno_func != NULL);
  
  yes_no = FLDMENUFUNC_ProcYesNoMenu(unisys->fldmenu_yesno_func);
  if(yes_no == FLDMENUFUNC_YESNO_NULL){
    return FALSE;
  }
  else if(yes_no == FLDMENUFUNC_YESNO_YES){
    *result = TRUE;
  }
  else{
    *result = FALSE;
  }
  return TRUE;
}

//--------------------------------------------------------------
/**
 * メニューリスト：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   menulist		
 * @param   list_max		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, const FLDMENUFUNC_LIST *menulist, int list_max, const FLDMENUFUNC_HEADER *menuhead)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  FLDMENUFUNC_LISTDATA *fldmenu_listdata;
  
  if(unisys->fldmenu_func == NULL){
    fldmenu_listdata = FLDMENUFUNC_CreateMakeListData(
      menulist, list_max, unisys->msgdata, HEAPID_UNION);
    unisys->fldmenu_func = FLDMENUFUNC_AddMenu(fldmsg_bg, menuhead, fldmenu_listdata);
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：削除
 *
 * @param   unisys		
 */
//--------------------------------------------------------------
static void UnionMsg_Menu_WindowDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->fldmenu_func != NULL){
    FLDMENUFUNC_DeleteMenu(unisys->fldmenu_func);
    unisys->fldmenu_func = NULL;
  }
}

//--------------------------------------------------------------
/**
 * メニューリスト：選択待ち
 *
 * @param   unisys		
 *
 * @retval  u32		結果
 */
//--------------------------------------------------------------
static u32 UnionMsg_Menu_SelectLoop(UNION_SYSTEM_PTR unisys)
{
  GF_ASSERT(unisys->fldmenu_func != NULL);
  return FLDMENUFUNC_ProcMenu(unisys->fldmenu_func);
}

//==================================================================
/**
 * メインメニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_MainMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
    MainMenuList, NELEMS(MainMenuList), &MenuHeader_MainMenu);
}

//==================================================================
/**
 * メインメニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_MainMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * メインメニュー：選択待ち
 *
 * @param   unisys		
 * 
 * @retval  u32		結果
 */
//==================================================================
u32 UnionMsg_Menu_MainMenuSelectLoop(UNION_SYSTEM_PTR unisys)
{
  return UnionMsg_Menu_SelectLoop(unisys);
}

//==================================================================
/**
 * 対戦メニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, int menu_index, UNION_MENU_REGULATION *menu_reg)
{
  FLDMENUFUNC_HEADER head;
  
  menu_reg->menu_index = menu_index;
  
  head = MenuHeader_Battle;
  head.count = BattleMenuDataTbl[menu_index].list_max;
  head.line = BattleMenuDataTbl[menu_index].list_max;
  head.bmpsize_y = head.count * 2;
  
  if(menu_index == BATTLE_MENU_INDEX_CUSTOM){
    FLDMENUFUNC_LIST *list;
    
    list = GFL_HEAP_AllocClearMemory(
      HEAPID_UNION, sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_Custom));
    GFL_STD_MemCopy(BattleMenuList_Custom, list, 
      sizeof(FLDMENUFUNC_LIST) * NELEMS(BattleMenuList_Custom));
    
    list[0].str_id = msg_union_battle_01_04 + menu_reg->mode;     //対戦形式(シングル、ダブル…)
    list[1].str_id = msg_union_battle_01_10 + menu_reg->rule;     //ルール(レベル50、制限無し…)
    list[2].str_id = msg_union_battle_01_11_01 + menu_reg->shooter;  //シューター有無
    
    UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
      list, BattleMenuDataTbl[menu_index].list_max, &head);
    GFL_HEAP_FreeMemory(list);
  }
  else{
    UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
      BattleMenuDataTbl[menu_index].list, BattleMenuDataTbl[menu_index].list_max, &head);
  }
}

//==================================================================
/**
 * 対戦メニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_BattleMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * 対戦メニュー：選択待ち
 *
 * @param   unisys		
 * @param   next_sub_menu		TRUE:次のサブメニューがある　FALSE:最終結果
 *
 * @retval  u32		next_sub_menuがTRUEの場合、次のメニューIndex
 * @retval  u32		next_sub_menuがFALSEの場合、最終結果番号(これを通信で送る)
 */
//==================================================================
u32 UnionMsg_Menu_BattleMenuSelectLoop(UNION_SYSTEM_PTR unisys, BOOL *next_sub_menu, UNION_MENU_REGULATION *menu_reg, BOOL *reg_look)
{
  u32 menu_ret;
  int i;
  
  *next_sub_menu = FALSE;
  *reg_look = FALSE;
  
  menu_ret = UnionMsg_Menu_SelectLoop(unisys);
  switch(menu_ret){
  case FLDMENUFUNC_NULL:
  case FLDMENUFUNC_CANCEL:
    return menu_ret;
  
  case UNION_BATTLE_REGULATION_DECIDE:
    return MenuReg_to_PlayCategory[menu_reg->mode][menu_reg->rule][menu_reg->shooter];
  case UNION_BATTLE_REGULATION_RULE:
    *reg_look = TRUE; //レギュレーションを見る
    return MenuReg_to_PlayCategory[menu_reg->mode][menu_reg->rule][menu_reg->shooter];
  }
  if(menu_reg->menu_index == BATTLE_MENU_INDEX_NUMBER && menu_ret < UNION_PLAY_CATEGORY_MAX){
    return menu_ret;  //マルチ選択
  }

  for(i = 0; i < NELEMS(BattleMenuDataTbl); i++){
    if(BattleMenuDataTbl[i].list == (void*)menu_ret){
      *next_sub_menu = TRUE;
      return i;
    }
    else{
      switch(menu_reg->menu_index){
      case BATTLE_MENU_INDEX_MODE:
        if(menu_ret < UNION_BATTLE_REGULATION_MODE_MAX){
          menu_reg->mode = menu_ret;
          *next_sub_menu = TRUE;
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      case BATTLE_MENU_INDEX_RULE:
        if(menu_ret < UNION_BATTLE_REGULATION_RULE_MAX){
          menu_reg->rule = menu_ret;
          *next_sub_menu = TRUE;
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      case BATTLE_MENU_INDEX_SHOOTER:
        if(menu_ret < UNION_BATTLE_REGULATION_SHOOTER_MAX){
          menu_reg->shooter = menu_ret;
          *next_sub_menu = TRUE;
          return BATTLE_MENU_INDEX_CUSTOM;
        }
        break;
      }
    }
  }
  
  GF_ASSERT_MSG(0, "menu_ret=%d\n", menu_ret);
  return 0;
}

//==================================================================
/**
 * POKEPARTY選択メニュー：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 */
//==================================================================
void UnionMsg_Menu_PokePartySelectMenuSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  UnionMsg_Menu_WindowSetup(unisys, fieldWork, 
    PokePartySelectMenuList, NELEMS(PokePartySelectMenuList), &MenuHeader_PokePartySelect);
}

//==================================================================
/**
 * POKEPARTY選択メニュー：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_PokePartySelectMenuDel(UNION_SYSTEM_PTR unisys)
{
  UnionMsg_Menu_WindowDel(unisys);
}

//==================================================================
/**
 * POKEPARTY選択メニュー：選択待ち
 *
 * @param   unisys		
 *
 * @retval  u32		結果
 */
//==================================================================
u32 UnionMsg_Menu_PokePartySelectMenuSelectLoop(UNION_SYSTEM_PTR unisys)
{
  return UnionMsg_Menu_SelectLoop(unisys);
}

//==================================================================
/**
 * NGレギュレーション：セットアップ
 *
 * @param   unisys		
 * @param   fieldWork		
 * @param   fail_bit    レギュレーションNGbit
 * @param   shooter_type  TRUE:シューター有　FALSE:シューター無し
 * @param   regwin_type    NGREG_TYPE_???
 */
//==================================================================
void UnionMsg_Menu_RegulationSetup(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork, u32 fail_bit, BOOL shooter_type, REGWIN_TYPE regwin_type)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  int category;
  PRINTSYS_LSB color;
  u16 title_msgid;
  
  GF_ASSERT(unisys->wordset == NULL);
  GF_ASSERT(unisys->fldmsgwin == NULL);
  GF_ASSERT(unisys->alloc.rpm == NULL);
  
  if(shooter_type == TRUE){
    shooter_type = REGULATION_SHOOTER_VALID;
  }
  else{
    shooter_type = REGULATION_SHOOTER_INVALID;
  }
  
  unisys->wordset = WORDSET_Create(HEAPID_UNION);
  unisys->fldmsgwin = FLDMSGWIN_Add(fldmsg_bg, unisys->msgdata, 
    1, 1, 26, (REGULATION_CATEGORY_MAX+2)*2);
  unisys->alloc.rpm = PokeRegulation_CreatePrintMsg(unisys->alloc.regulation,
    unisys->wordset, HEAPID_UNION, shooter_type);

  if(regwin_type == REGWIN_TYPE_RULE){
    STRBUF* cupname = Regulation_CreateCupName(unisys->alloc.regulation, HEAPID_UNION);
    FLDMSGWIN_PrintStrBuf( unisys->fldmsgwin, 0, 0, cupname );
    GFL_STR_DeleteBuffer(cupname);
  }
  else{
    if(regwin_type == REGWIN_TYPE_NG_TEMOTI){
      title_msgid = msg_union_ng_temoti;
    }
    else{
      title_msgid = msg_union_ng_bbox;
    }
    FLDMSGWIN_Print( unisys->fldmsgwin, 0, 0, title_msgid );
  }
  
  for(category = 0; category < REGULATION_CATEGORY_MAX; category++){
    if(fail_bit & (1 << category)){
      color = REG_FAIL_COLOR;
    }
    else{
      color = REG_NORMAL_COLOR;
    }
    FLDMSGWIN_PrintStrBufColor( unisys->fldmsgwin, 0, (4 + 2*category)*8, 
      unisys->alloc.rpm->category[category], color );
    FLDMSGWIN_PrintStrBufColor( unisys->fldmsgwin, 14*8, (4 + 2*category)*8, 
      unisys->alloc.rpm->prerequisite[category], color );
  }
}

//==================================================================
/**
 * NGレギュレーション：描画完了待ち
 *
 * @param   unisys		
 * @param   fieldWork		
 *
 * @retval  BOOL		TRUE:描画完了　FALSE:描画中
 */
//==================================================================
BOOL UnionMsg_Menu_RegulationWait(UNION_SYSTEM_PTR unisys, FIELDMAP_WORK *fieldWork)
{
  FLDMSGBG *fldmsg_bg = FIELDMAP_GetFldMsgBG(fieldWork);
  PRINT_QUE *print_que = FLDMSGBG_GetPrintQue( fldmsg_bg );

  if(unisys->alloc.rpm == NULL){
    return TRUE;
  }

  return PRINTSYS_QUE_IsFinished( print_que );
}

//==================================================================
/**
 * NGレギュレーション：削除
 *
 * @param   unisys		
 */
//==================================================================
void UnionMsg_Menu_RegulationDel(UNION_SYSTEM_PTR unisys)
{
  if(unisys->alloc.rpm == NULL){
    return;
  }
  
  WORDSET_Delete(unisys->wordset);
  unisys->wordset = NULL;
  
  FLDMSGWIN_Delete(unisys->fldmsgwin);
  unisys->fldmsgwin = NULL;

  PokeRegulation_DeletePrintMsg(unisys->alloc.rpm);
  unisys->alloc.rpm = NULL;
}

