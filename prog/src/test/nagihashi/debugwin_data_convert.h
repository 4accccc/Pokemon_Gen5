//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debugwin_data_convert.h
 *	@brief
 *	@author		Toru=Nagihashi
 *	@date		2011.06.28
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG
#include "debug/debugwin_sys.h"
#include "savedata/data_convert_save.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define DATACONV_DEBUGWIN_GROUP     (20)
#define DATACONV_OWNER_GROUP        (21)
#define DATACONV_FLAG_GROUP         (22)
#define DATACONV_BADGE_GROUP        (23)
#define DATACONV_WORK_GROUP         (24)
#define DATACONV_MUSICAL_GROUP      (25)
#define DATACONV_DENDOU_GROUP       (26)
#define DATACONV_DENDOU_WAZA_GROUP  (27)

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

static u32 s_data_conv_pokemon_idx = 0;

//=============================================================================
/**
 *          デバッグウィンドウ関数
 */
//=============================================================================

static inline BOOL DebugWin_Util_DC_ChangeDataU32( u32 *p_param, u32 min, u32 max )
{ 
  BOOL is_update  = FALSE;
  u32 value  = 1;

  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
  {
    value = 10;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
    value = 100;
  }
  else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X )
  {
    value = 1000;
  }

  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT )
  { 
    if( (s64)*p_param - (s64)value >= (s64)min )
    { 
      (*p_param)-=value;
    }
    else
    {
      (*p_param)  = min;
    }
    is_update = TRUE;
  }
  if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT )
  { 
    if( *p_param + value <= max )
    { 
      (*p_param)+=value;
    }
    else
    {
      (*p_param)  = max;
    }
    is_update = TRUE;
  }

  return is_update;
}

static void DebugWin_U_DC_Owner_Name( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  //変えられない
}
static void DebugWin_D_DC_Owner_Name( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;
  char sjis[ PERSON_NAME_SIZE+EOM_SIZE ];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );
  GFL_STR_CONV_StrcodeToSjis( owner.name, sjis, PERSON_NAME_SIZE+EOM_SIZE );

  DEBUGWIN_ITEM_SetNameV( p_item , "なまえ=%s", sjis );
}

static void DebugWin_U_DC_Owner_TrainerID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );

  if( DebugWin_Util_DC_ChangeDataU32( &owner.trainerID, 0, 0xFFFFFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_OWNER, (u32)&owner );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Owner_TrainerID( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );

  DEBUGWIN_ITEM_SetNameV( p_item , "ID=%d", owner.trainerID );
}

static void DebugWin_U_DC_Owner_Sex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );

  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    owner.sex ^=  1;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_OWNER, (u32)&owner );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Owner_Sex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );

  DEBUGWIN_ITEM_SetNameV( p_item , "せいべつ=%d", owner.sex );
}
static void DebugWin_U_DC_Owner_Version( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );
  data = owner.version;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 100 ) )
  {
    owner.version = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_OWNER, (u32)&owner );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Owner_Version( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_OWNER owner;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_OWNER, &owner );

  DEBUGWIN_ITEM_SetNameV( p_item , "バージョン=%d", owner.version );
}

static void DebugWin_U_DC_Flag_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_GAME_CLEAR );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_GAME_CLEAR, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_GAME_CLEAR );
  DEBUGWIN_ITEM_SetNameV( p_item , "ゲームクリア=%d", is_flag );
}
static void DebugWin_U_DC_Flag_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_FIRST_POKE_GET );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_FIRST_POKE_GET, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_FIRST_POKE_GET );
  DEBUGWIN_ITEM_SetNameV( p_item , "ファーストポケゲット=%d", is_flag );
}
static void DebugWin_U_DC_Flag_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_CHAMPION_WIN );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_CHAMPION_WIN, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_CHAMPION_WIN );
  DEBUGWIN_ITEM_SetNameV( p_item , "チャンピオンかち=%d", is_flag );
}
static void DebugWin_U_DC_Flag_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_01 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_01, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_01 );
  DEBUGWIN_ITEM_SetNameV( p_item , "イッシュかんせい=%d", is_flag );
}
static void DebugWin_U_DC_Flag_04( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_02 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_02, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_04( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_AWARD_02 );
  DEBUGWIN_ITEM_SetNameV( p_item , "ぜんこくかんせい=%d", is_flag );
}
static void DebugWin_U_DC_Flag_05( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_01 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_01, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_05( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_01 );
  DEBUGWIN_ITEM_SetNameV( p_item , "サブウェイトロフィ１=%d", is_flag );
}
static void DebugWin_U_DC_Flag_06( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_02 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_02, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_06( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_02 );
  DEBUGWIN_ITEM_SetNameV( p_item , "サブウェイトロフィ２=%d", is_flag );
}
static void DebugWin_U_DC_Flag_07( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_03 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_03, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_07( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BSUBWAY_TROPHY_03 );
  DEBUGWIN_ITEM_SetNameV( p_item , "サブウェイトロフィ３=%d", is_flag );
}
static void DebugWin_U_DC_Flag_08( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_01 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_01, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_08( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_01 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ１=%d", is_flag );
}
static void DebugWin_U_DC_Flag_09( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_02 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_02, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_09( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_02 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ２=%d", is_flag );
}
static void DebugWin_U_DC_Flag_10( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_03 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_03, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_10( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_03 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ３=%d", is_flag );
}
static void DebugWin_U_DC_Flag_11( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_04 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_04, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_11( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_04 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ４=%d", is_flag );
}

static void DebugWin_U_DC_Flag_12( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_05 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_05, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_12( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_05 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ５=%d", is_flag );
}
static void DebugWin_U_DC_Flag_13( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_06 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_06, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_13( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_06 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ６=%d", is_flag );
}
static void DebugWin_U_DC_Flag_14( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_07 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_07, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_14( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_07 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ７=%d", is_flag );
}
static void DebugWin_U_DC_Flag_15( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_DECIDE )
  {
    BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_08 );
    DATA_CONVERT_SAVE_SetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_08, !is_flag );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Flag_15( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  BOOL is_flag = DATA_CONVERT_SAVE_GetFlag( p_sv, WB_SAVE_CONV_FLAG_BADGE_08 );
  DEBUGWIN_ITEM_SetNameV( p_item , "バッジ８=%d", is_flag );
}

static void DebugWin_U_DC_Work_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  u32 work = DATA_CONVERT_SAVE_GetWork( p_sv, WB_SAVE_CONV_WORK_N_BATTLE_RAIMON );

  if( DebugWin_Util_DC_ChangeDataU32( &work, 0, 0xFFFF ) )
  {
    DATA_CONVERT_SAVE_SetWork( p_sv, WB_SAVE_CONV_WORK_N_BATTLE_RAIMON, work );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Work_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u32 work = DATA_CONVERT_SAVE_GetWork( p_sv, WB_SAVE_CONV_WORK_N_BATTLE_RAIMON );
  DEBUGWIN_ITEM_SetNameV( p_item , "ライモンNバトル=%d", work );
}

static void DebugWin_U_DC_Musical_Point( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  u32 data = DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_POINT, NULL );

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 0xFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_POINT, data );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_Point( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u32 data = DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_POINT, NULL );
  DEBUGWIN_ITEM_SetNameV( p_item , "そうごうとくてん=%d", data );
}
static void DebugWin_U_DC_Musical_Entry( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  u32 data = DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ENTRY, NULL );

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 0xFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ENTRY, data );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_Entry( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u32 data = DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ENTRY, NULL );
  DEBUGWIN_ITEM_SetNameV( p_item , "さんかかいすう=%d", data );
}

static void DebugWin_U_DC_Musical_ItemBit_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 *p_data = (u32*)&item_bit[0];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  if( DebugWin_Util_DC_ChangeDataU32( p_data, 0, 0xFFFFFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, (u32)item_bit );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_ItemBit_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 * p_data = (u32*)&item_bit[0];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  DEBUGWIN_ITEM_SetNameV( p_item , "どうぐビット１=%x", *p_data );
}
static void DebugWin_U_DC_Musical_ItemBit_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 *p_data = (u32*)&item_bit[4];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  if( DebugWin_Util_DC_ChangeDataU32( p_data, 0, 0xFFFFFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, (u32)item_bit );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_ItemBit_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 * p_data = (u32*)&item_bit[4];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  DEBUGWIN_ITEM_SetNameV( p_item , "どうぐビット２=%x", *p_data );
}
static void DebugWin_U_DC_Musical_ItemBit_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 *p_data = (u32*)&item_bit[8];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  if( DebugWin_Util_DC_ChangeDataU32( p_data, 0, 0xFFFFFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, (u32)item_bit );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_ItemBit_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 * p_data = (u32*)&item_bit[8];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  DEBUGWIN_ITEM_SetNameV( p_item , "どうぐビット３=%x", *p_data );
}
static void DebugWin_U_DC_Musical_ItemBit_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 *p_data = (u32*)&item_bit[12];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  if( DebugWin_Util_DC_ChangeDataU32( p_data, 0, 0xFFFFFFFF ) )
  {
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, (u32)item_bit );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_Musical_ItemBit_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  u8 item_bit[WB_SAVE_CONV_MUSICAL_ITEM_BIT];
  u32 * p_data = (u32*)&item_bit[12];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_MUSICAL_ITEMBIT, item_bit );

  DEBUGWIN_ITEM_SetNameV( p_item , "どうぐビット４=%x", *p_data );
}
static void DebugWin_U_DC_DendouPoke_Index( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  if( DebugWin_Util_DC_ChangeDataU32( &s_data_conv_pokemon_idx, 0, 5 ) )
  {
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Index( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  DEBUGWIN_ITEM_SetNameV( p_item , "インデックス=%d", s_data_conv_pokemon_idx );
}
static void DebugWin_U_DC_DendouPoke_Monsno( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.monsno;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 649 ) )
  {
    poke.monsno = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Monsno( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "しゅぞく=%d", poke.monsno );
}
static void DebugWin_U_DC_DendouPoke_Lv( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.level;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 100 ) )
  {
    poke.level = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Lv( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "レベル=%d", poke.level );
}
static void DebugWin_U_DC_DendouPoke_Form( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.formNumber;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 32 ) )
  {
    poke.formNumber = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Form( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "フォルム=%d", poke.formNumber );
}
static void DebugWin_U_DC_DendouPoke_Sex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.sex;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 2 ) )
  {
    poke.sex = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Sex( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "せいべつ=%d", poke.sex );
}
static void DebugWin_U_DC_DendouPoke_Personal( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.personalRandom;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 0xFFFFFFFF ) )
  {
    poke.personalRandom = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Personal( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "パーソナル=%x", poke.personalRandom );
}
static void DebugWin_U_DC_DendouPoke_Id( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.idNumber;

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 0xFFFFFFFF ) )
  {
    poke.idNumber = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Id( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "ID=%d", poke.idNumber );
}
static void DebugWin_U_DC_DendouPoke_NickName( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  //変えられない
}
static void DebugWin_D_DC_DendouPoke_NickName( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  char  sjis[ MONS_NAME_SIZE+EOM_SIZE ];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  GFL_STR_CONV_StrcodeToSjis( poke.nickname, sjis, MONS_NAME_SIZE+EOM_SIZE);

  DEBUGWIN_ITEM_SetNameV( p_item , "なまえ=%s", sjis );
}
static void DebugWin_U_DC_DendouPoke_OyaName( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  //変えられない
}
static void DebugWin_D_DC_DendouPoke_OyaName( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  char  sjis[ MONS_NAME_SIZE+EOM_SIZE ];

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  GFL_STR_CONV_StrcodeToSjis( poke.oyaname, sjis, MONS_NAME_SIZE+EOM_SIZE);

  DEBUGWIN_ITEM_SetNameV( p_item , "おや=%s", sjis );
}
static void DebugWin_U_DC_DendouPoke_Waza_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.waza[0];

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 700 ) )
  {
    poke.waza[0] = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Waza_00( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "わざ１=%d", poke.waza[0] );
}
static void DebugWin_U_DC_DendouPoke_Waza_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.waza[1];

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 700 ) )
  {
    poke.waza[1] = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Waza_01( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "わざ２=%d", poke.waza[1] );
}
static void DebugWin_U_DC_DendouPoke_Waza_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.waza[2];

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 700 ) )
  {
    poke.waza[2] = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Waza_02( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "わざ３=%d", poke.waza[2] );
}
static void DebugWin_U_DC_DendouPoke_Waza_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;

  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;
  u32 data;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );
  data = poke.waza[3];

  if( DebugWin_Util_DC_ChangeDataU32( &data, 0, 700 ) )
  {
    poke.waza[3] = data;
    DATA_CONVERT_SAVE_SetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, (u32)&poke );
    DEBUGWIN_RefreshScreen();
  }
}
static void DebugWin_D_DC_DendouPoke_Waza_03( void* p_wk_adrs, DEBUGWIN_ITEM* p_item )
{
  DATA_CONVERT_SAVE *p_sv = p_wk_adrs;
  WB_SAVE_CONV_DENDOU_POKEMON_DATA  poke;

  DATA_CONVERT_SAVE_GetData( p_sv, DATA_CONVERT_GET_ID_DENDOU_POKE_00+s_data_conv_pokemon_idx, &poke );

  DEBUGWIN_ITEM_SetNameV( p_item , "わざ４=%d", poke.waza[3] );
}


static void DEBUGWIN_DATA_CONVERT_Init( DATA_CONVERT_SAVE *p_sv, HEAPID heapID )
{
  s_data_conv_pokemon_idx = 0;

  DEBUGWIN_AddGroupToTop( DATACONV_DEBUGWIN_GROUP, "データコンバート", heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_OWNER_GROUP, "オーナー", DATACONV_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Owner_TrainerID, 
                              DebugWin_D_DC_Owner_TrainerID, 
                              p_sv, 
                              DATACONV_OWNER_GROUP,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Owner_Name, 
                              DebugWin_D_DC_Owner_Name, 
                              p_sv, 
                              DATACONV_OWNER_GROUP,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Owner_Sex, 
                              DebugWin_D_DC_Owner_Sex, 
                              p_sv, 
                              DATACONV_OWNER_GROUP,
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Owner_Version, 
                              DebugWin_D_DC_Owner_Version, 
                              p_sv, 
                              DATACONV_OWNER_GROUP,
                              heapID );


  DEBUGWIN_AddGroupToGroup( DATACONV_FLAG_GROUP, "フラグ", DATACONV_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_00, 
                              DebugWin_D_DC_Flag_00, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_01, 
                              DebugWin_D_DC_Flag_01, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_02, 
                              DebugWin_D_DC_Flag_02, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_03, 
                              DebugWin_D_DC_Flag_03, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_04, 
                              DebugWin_D_DC_Flag_04, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_05, 
                              DebugWin_D_DC_Flag_05, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_06, 
                              DebugWin_D_DC_Flag_06, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_07, 
                              DebugWin_D_DC_Flag_07, 
                              p_sv, 
                              DATACONV_FLAG_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_BADGE_GROUP, "バッジ", DATACONV_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_08, 
                              DebugWin_D_DC_Flag_08, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_09, 
                              DebugWin_D_DC_Flag_09, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_10, 
                              DebugWin_D_DC_Flag_10, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_11, 
                              DebugWin_D_DC_Flag_11, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_12, 
                              DebugWin_D_DC_Flag_12, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_13, 
                              DebugWin_D_DC_Flag_13, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_14, 
                              DebugWin_D_DC_Flag_14, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Flag_15, 
                              DebugWin_D_DC_Flag_15, 
                              p_sv, 
                              DATACONV_BADGE_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_WORK_GROUP, "ワーク", DATACONV_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Work_00, 
                              DebugWin_D_DC_Work_00, 
                              p_sv, 
                              DATACONV_WORK_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_MUSICAL_GROUP, "ミュージカル", DATACONV_DEBUGWIN_GROUP , heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_Point, 
                              DebugWin_D_DC_Musical_Point, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_Entry, 
                              DebugWin_D_DC_Musical_Entry, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_ItemBit_00, 
                              DebugWin_D_DC_Musical_ItemBit_00, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_ItemBit_01, 
                              DebugWin_D_DC_Musical_ItemBit_01, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_ItemBit_02, 
                              DebugWin_D_DC_Musical_ItemBit_02, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_Musical_ItemBit_03, 
                              DebugWin_D_DC_Musical_ItemBit_03, 
                              p_sv, 
                              DATACONV_MUSICAL_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_DENDOU_GROUP, "でんどうポケ", DATACONV_DEBUGWIN_GROUP , heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Index,
                              DebugWin_D_DC_DendouPoke_Index, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Monsno,
                              DebugWin_D_DC_DendouPoke_Monsno, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Lv,
                              DebugWin_D_DC_DendouPoke_Lv, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Form,
                              DebugWin_D_DC_DendouPoke_Form, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Sex,
                              DebugWin_D_DC_DendouPoke_Sex, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Personal,
                              DebugWin_D_DC_DendouPoke_Personal, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Id,
                              DebugWin_D_DC_DendouPoke_Id, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_NickName,
                              DebugWin_D_DC_DendouPoke_NickName, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_OyaName,
                              DebugWin_D_DC_DendouPoke_OyaName, 
                              p_sv, 
                              DATACONV_DENDOU_GROUP, 
                              heapID );

  DEBUGWIN_AddGroupToGroup( DATACONV_DENDOU_WAZA_GROUP, "わざ", DATACONV_DENDOU_GROUP, heapID );

  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Waza_00,
                              DebugWin_D_DC_DendouPoke_Waza_00, 
                              p_sv, 
                              DATACONV_DENDOU_WAZA_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Waza_01,
                              DebugWin_D_DC_DendouPoke_Waza_01, 
                              p_sv, 
                              DATACONV_DENDOU_WAZA_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Waza_02,
                              DebugWin_D_DC_DendouPoke_Waza_02, 
                              p_sv, 
                              DATACONV_DENDOU_WAZA_GROUP, 
                              heapID );
  DEBUGWIN_AddItemToGroupEx(  DebugWin_U_DC_DendouPoke_Waza_03,
                              DebugWin_D_DC_DendouPoke_Waza_03, 
                              p_sv, 
                              DATACONV_DENDOU_WAZA_GROUP, 
                              heapID );
}

static void DEBUGWIN_DATA_CONVERT_Exit( void )
{
  DEBUGWIN_RemoveGroup( DATACONV_DEBUGWIN_GROUP );
}
#endif //PM_DEBUG
