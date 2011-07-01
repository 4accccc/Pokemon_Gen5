//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		debug_wfbc.c
 *	@brief  WFBCオートデバック機能
 *	@author	tomoya takahashi
 *	@date		2010.05.24
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#ifdef PM_DEBUG

#include "debug_wfbc.h"

#include "field/field_wfbc_data.h" 

#include "fieldmap/zone_id.h"
#include "event_mapchange.h"

#include "system/bmp_winframe.h"

#include "arc_def.h"
#include "debug_message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "msg/debug/msg_d_tomoya.h"

#include "font/font.naix"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	WFBC_View
//=====================================
typedef struct {

  
  FIELD_WFBC_CORE wfbc_data_tmp;
  FIELD_WFBC_CORE* p_wfbc_data;
  GAMESYS_WORK* p_gamesys;
  FIELDMAP_WORK* p_fieldmap;

  WORDSET*    p_wordset;
  GFL_FONT*   p_font;
  GFL_MSGDATA*  p_msgdata;
  STRBUF*     p_strbuff;
  STRBUF*     p_strbuff_tmp;


  GFL_BMPWIN* p_win;

  u16 page;
  u16 change;
  
  
  HEAPID heapID;
  
} EVENT_DEBUG_WFBC_VIEW;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief  WFBCcontrollerを起動
 *
 *	@param	gsys  ゲームシステム
 *
 *	@return イベントデータ
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_View( GAMESYS_WORK *gsys )
{

  return NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  人物全PATTERNチェック
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_PeopleCheck(
    GAMESYS_WORK *gsys )
{

  return NULL;
}


//----------------------------------------------------------------------------
/**
 *	@brief  100回SetUpを読んでみる。
 */
//-----------------------------------------------------------------------------
GMEVENT * DEBUG_WFBC_100SetUp(
    GAMESYS_WORK *gsys )
{

  return NULL;
}


#endif //PM_DEBUG

