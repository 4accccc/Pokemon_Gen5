//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		dpw_rap.c
 *	@brief  DPWライブラリを共通で使うためのソース
 *	@author	Toru=Nagihashi
 *	@data		2010.02.18
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <dpw_tr.h>
#include "print/str_tool.h"

//セーブデータ
#include "savedata/mystatus.h"

//外部公開
#include "net/dpw_rap.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  Dpw_Common_Profileを作成
 *
 *	@param	*p_dc_profile         作成したPROFILE受け取り
 *	@param	MYSTATUS *p_mystatus  PROFILE作成のために必要なデータ
 */
//-----------------------------------------------------------------------------
void DPW_RAP_CreateProfile( Dpw_Common_Profile *p_dc_profile, const MYSTATUS *cp_mystatus )
{ 
  GFL_STD_MemClear( p_dc_profile, sizeof(Dpw_Common_Profile));

  p_dc_profile->version     = PM_VERSION;
  p_dc_profile->language    = PM_LANG;
  p_dc_profile->countryCode = MyStatus_GetMyNation(cp_mystatus);
  p_dc_profile->localCode   = MyStatus_GetMyArea(cp_mystatus);
  p_dc_profile->playerId    = MyStatus_GetID(cp_mystatus);

  STRTOOL_Copy( MyStatus_GetMyName(cp_mystatus), p_dc_profile->playerName, DPW_TR_NAME_SIZE );

  p_dc_profile->flag = 0;   //ハングル文字を表示できるか できるならDPW_PROFILE_FLAG_HANGEUL_AVAILABLE

  //p_dc_profile->macAddr   ライブラリ内で格納するのでセットの必要なし

  //以下メール
  p_dc_profile->mailAddr[0]   = '\0'; //シャチでは使用しない
  p_dc_profile->mailRecvFlag  = 0;  //メール受信するならDPW_PROFILE_MAILRECVFLAG_EXCHANGE
}
