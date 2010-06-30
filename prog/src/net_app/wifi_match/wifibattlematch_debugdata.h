//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		wifibattlematch_debugdata.h
 *	@brief  デバッグ用データ
 *	@author		Toru=Nagihashi
 *	@date		2010.06.19
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once

#ifdef PM_DEBUG

//-------------------------------------
///	レポート確認用
//=====================================
enum
{
  DEBUGWIN_REPORT_WIN,
  DEBUGWIN_REPORT_LOSE,
  DEBUGWIN_REPORT_DIRTY,
  DEBUGWIN_REPORT_DISCONNECT,

  DEBUGWIN_REPORT_MAX,
};
enum
{
  DEBUGWIN_REPORT_WHO_MY,
  DEBUGWIN_REPORT_WHO_YOU,

  DEBUGWIN_REPORT_WHO_MAX,
};
typedef struct
{
  s32   param[DEBUGWIN_REPORT_WHO_MAX][DEBUGWIN_REPORT_MAX];
} DEBUGWIN_REPORT_DATA;

extern DEBUGWIN_REPORT_DATA *DEBUGWIN_REPORT_DATA_GetInstance( void );

//-------------------------------------
///	戦闘曲指定
//=====================================
typedef struct 
{
  BOOL  is_use;
  u16   btl_bgm;
  u16   win_bgm;
} DEBUGWIN_BTLBGM_DATA;

extern DEBUGWIN_BTLBGM_DATA *DEBUGWIN_BTLBGM_DATA_GetInstance( void );

//-------------------------------------
///	不正フラグ
//=====================================
extern BOOL *DEBUGWIN_ATLASDIRTY_GetFlag( void );

//-------------------------------------
///	サーバー時間
//=====================================
extern BOOL *DEBUGWIN_SERVERTIME_GetFlag( void );
extern BOOL *DEBUGWIN_SERVERTIME_USER_GetFlag( void );

//-------------------------------------
///	バトルプレイ
//=====================================
extern BOOL *DEBUGWIN_BATTLE_GetFlag( void );

//-------------------------------------
///	レポート音
//=====================================
extern BOOL *DEBUGWIN_REPORTMARK_GetFlag( void );
#endif //PM_DEBUG
