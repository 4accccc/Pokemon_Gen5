//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_subscreen.c
 *	@brief
 *	@author	 
 *	@data		2009.03.26
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "infowin/infowin.h"
#include "field_subscreen.h"
#include "c_gear/c_gear.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

#define CGEAR_ON (1)

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

struct _FIELD_SUBSCREEN_WORK {
  C_GEAR_WORK* cgearWork;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------


//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの初期化
 *	
 *	@param	heapID	ヒープＩＤ
 */
//-----------------------------------------------------------------------------
FIELD_SUBSCREEN_WORK* FIELD_SUBSCREEN_Init( u32 heapID )
{
  FIELD_SUBSCREEN_WORK* pWork = GFL_HEAP_AllocClearMemory(heapID, sizeof(FIELD_SUBSCREEN_WORK));

  {
    // BG3 SUB (インフォバー
    static const GFL_BG_BGCNT_HEADER header_sub3 = {
      0, 0, 0x800, 0,	// scrX, scrY, scrbufSize, scrbufofs,
		GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
		GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000,0x6000,
		GX_BG_EXTPLTT_01, 0, 0, 0, FALSE	// pal, pri, areaover, dmy, mosaic
	};

	GFL_BG_SetBGControl( FIELD_SUBSCREEN_BGPLANE, &header_sub3, GFL_BG_MODE_TEXT );
	GFL_BG_SetVisible( FIELD_SUBSCREEN_BGPLANE, VISIBLE_ON );
	GFL_BG_ClearFrame( FIELD_SUBSCREEN_BGPLANE );
	
	GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);

	INFOWIN_Init( FIELD_SUBSCREEN_BGPLANE , FIELD_SUBSCREEN_PALLET , heapID);
	if( INFOWIN_IsInitComm() == TRUE )
	{
		GFL_NET_ReloadIcon();
	}
  }
#if CGEAR_ON
  pWork->cgearWork = CGEAR_Init();
#endif
  
  return pWork;
}

//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの破棄
 */
//-----------------------------------------------------------------------------
void FIELD_SUBSCREEN_Exit( FIELD_SUBSCREEN_WORK* pWork )
{
#if CGEAR_ON
  CGEAR_Exit(pWork->cgearWork);
#endif
	INFOWIN_Exit();
	GFL_BG_FreeBGControl(FIELD_SUBSCREEN_BGPLANE);
  GFL_HEAP_FreeMemory(pWork);
}

//----------------------------------------------------------------------------
/**
 *	@brief	インフォーバーの更新
 */
//-----------------------------------------------------------------------------
void FIELD_SUBSCREEN_Main( FIELD_SUBSCREEN_WORK* pWork )
{
#if CGEAR_ON
  CGEAR_Main(pWork->cgearWork);
#endif
	INFOWIN_Update();
}

