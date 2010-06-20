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

#endif //PM_DEBUG
