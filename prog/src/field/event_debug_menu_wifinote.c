//======================================================================================
/**
 * @file  event_debug_menu_wifinote.c
 * @author  tamada
 * @date  2010.07.10
 *
 */
//======================================================================================
#ifdef  PM_DEBUG

#include <gflib.h>
#include "gamesystem/gamesystem.h"
#include "gamesystem/game_data.h"
#include "event_debug_menu_wifinote.h"
#include "net/dwc_rapfriend.h"

#include "system/main.h"  //HEAPID_
#include "debug/debug_str_conv.h"

//======================================================================================
//======================================================================================
//-------------------------------------
/// 友達データ新規設定関数　戻り値
//=====================================
enum{
  RCODE_NEWFRIEND_SET_OK, // 設定できた
  RCODE_NEWFRIEND_SET_FRIENDKEYNG,  // 友達コードが不正
  RCODE_NEWFRIEND_SET_SETING, // もう設定してあった
  RCODE_NEWFRIEND_SET_FULL,   // 手帳がいっぱい
  RCODE_NEWFRIEND_SET_NUM

};

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
typedef struct {
  u64 code;
  const char * name;
}DEBUG_WIFINOTE_CONST_DATA, DWCD;

#include "../../../resource/wifinote_list/gamefreak_list.cdat"

static const char * const DebugResultStr[] = {
  "SET_OK ",
  "NG CODE",
  "ALREADY",
  "FULL   ",
};
//======================================================================================
//======================================================================================
static u64 WFNOTE_STRBUF_GetNumber( STRBUF *str , BOOL *pbFlag );


static void entryCDat( GAMEDATA * gamedata );
static u32 DEBUG_SetNewFriendData( GAMEDATA * gamedata, u64 friendcode, STRBUF* p_name );

//======================================================================================
//======================================================================================
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
void DEBUG_EntryWifiNote( GAMESYS_WORK * gsys )
{
  GAMEDATA * gamedata = GAMESYSTEM_GetGameData( gsys );
  entryCDat( gamedata );
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
static void entryCDat( GAMEDATA * gamedata )
{ 
  const DEBUG_WIFINOTE_CONST_DATA * dwncd = DebugWifiNoteConstData;
  const int dwncd_max = NELEMS(DebugWifiNoteConstData);
  STRBUF * tempBuf;
  u16 pStrCode[64];
  int i;
  int result;
  
  // 友達情報全クリア
  {
    WIFI_LIST* p_list = GAMEDATA_GetWiFiList(gamedata); 
    for( i=0; i<WIFILIST_FRIEND_MAX; i++ ){
      WifiList_ResetData( p_list, 0 );
    }
  }

  tempBuf = GFL_STR_CreateBuffer( 16, HEAPID_FIELDMAP );

  for ( i = 0; i < dwncd_max; dwncd ++, i ++ )
  {
    DEB_STR_CONV_SjisToStrcode( dwncd->name, pStrCode, 64 );
    GFL_STR_SetStringCode( tempBuf, pStrCode );
    result = DEBUG_SetNewFriendData( gamedata, dwncd->code, tempBuf );
    OS_Printf("RESULT:%d ID:%012d\n", result, dwncd->code );
  }

  GFL_STR_DeleteBuffer( tempBuf );
}

//----------------------------------------------------------------------------
/**
 *  @brief  最新の友達データ設定
 *
 *  @param  p_data    ワーク
 *  @param  p_code    コード文字列
 *  @param  p_name    名前文字列
 */
//-----------------------------------------------------------------------------
static u32 DEBUG_SetNewFriendData( GAMEDATA * gamedata, u64 friendcode, STRBUF* p_name )
{
  int i;
  WIFI_LIST* p_list;
  u64 mycode;

  p_list = GAMEDATA_GetWiFiList(gamedata); //SaveData_GetWifiListData( p_data->p_save );

  mycode = DWC_CreateFriendKey( WifiList_GetMyUserInfo(p_list) );

  if ( mycode == friendcode ){
    // 友達コードが違う
    return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
  }

  for( i=0; i<WIFILIST_FRIEND_MAX; i++ ){
    BOOL result;
    result = WifiList_IsFriendData( p_list, i );
    if( result == FALSE ){
      DWCFriendData *p_dwc_list = NULL;
      DWCUserData *p_dwc_userdata = NULL;
      int ret;
      int pos;

      p_dwc_userdata = WifiList_GetMyUserInfo( p_list );
      if(!DWC_CheckFriendKey(p_dwc_userdata, friendcode))
      {
        // 友達コードが違う
        return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
      }
      ret = GFL_NET_DWC_CheckFriendCodeByToken(p_list, friendcode, &pos);
      if(ret == DWCFRIEND_INLIST){
        // もう設定ずみ
        return RCODE_NEWFRIEND_SET_SETING;
      }
      // 成功
      p_dwc_list = WifiList_GetDwcDataPtr(p_list, i);
      DWC_CreateFriendKeyToken(p_dwc_list, friendcode);
      // 名前と性別設定 中性で設定
      WifiList_SetFriendName(p_list, i, p_name);
      WifiList_SetFriendInfo(p_list, i, WIFILIST_FRIEND_SEX, PM_NEUTRAL);
      return RCODE_NEWFRIEND_SET_OK;
    }
  }
  return RCODE_NEWFRIEND_SET_FULL;
}
//----------------------------------------------------------------------------
/**
 *  @brief  最新の友達データ設定
 *
 *  @param  p_data    ワーク
 *  @param  p_code    コード文字列
 *  @param  p_name    名前文字列
 */
//-----------------------------------------------------------------------------
static u32 Data_NewFriendDataSet( GAMEDATA * gamedata, STRBUF* p_code, STRBUF* p_name )
{
  u64 friendcode;
  BOOL set_check;
  friendcode = WFNOTE_STRBUF_GetNumber(p_code, &set_check);
  if (set_check == FALSE )
  {
    return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
  }
  return DEBUG_SetNewFriendData( gamedata, friendcode, p_name );
#if 0
  int i;
  WIFI_LIST* p_list;
  BOOL result;
  BOOL set_check;
  u64 friendcode;
  u64 mycode;
  DWCFriendData *p_dwc_list = NULL;
  DWCUserData *p_dwc_userdata = NULL;
  int ret;
  int pos;

  p_list = GAMEDATA_GetWiFiList(gamedata); //SaveData_GetWifiListData( p_data->p_save );

  mycode = DWC_CreateFriendKey( WifiList_GetMyUserInfo(p_list) );

  for( i=0; i<WIFILIST_FRIEND_MAX; i++ ){
    result = WifiList_IsFriendData( p_list, i );
    if( result == FALSE ){
      friendcode = WFNOTE_STRBUF_GetNumber(p_code, &set_check);

      if( (set_check == TRUE) && (friendcode !=mycode) ){

        p_dwc_userdata = WifiList_GetMyUserInfo( p_list );
        if(!DWC_CheckFriendKey(p_dwc_userdata, friendcode))
        {
          // 友達コードが違う
          return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
        }
        ret = GFL_NET_DWC_CheckFriendCodeByToken(p_list, friendcode, &pos);
        if(ret == DWCFRIEND_INLIST){
          // もう設定ずみ
          return RCODE_NEWFRIEND_SET_SETING;
        }
        // 成功
        p_dwc_list = WifiList_GetDwcDataPtr(p_list, i);
        DWC_CreateFriendKeyToken(p_dwc_list, friendcode);
        // 名前と性別設定 中性で設定
        WifiList_SetFriendName(p_list, i, p_name);
        WifiList_SetFriendInfo(p_list, i, WIFILIST_FRIEND_SEX, PM_NEUTRAL);
        return RCODE_NEWFRIEND_SET_OK;
      }else{
        // 友達コードが違う
        return RCODE_NEWFRIEND_SET_FRIENDKEYNG;
      }
    }
  }

  GF_ASSERT(0);
  // ここはとおらないはず
  return RCODE_NEWFRIEND_SET_OK;
#endif
}

//------------------------------------------------------------------
/**
 * 文字数字を数字に変換する  18けた対応  この関数は文字コードに左右される
 *
 * @param src     変換したい数字が入っているバッファ
 * @param pbFlag    変換できたかどうか
 * @return  成功した場合変換数字
 *
 *  GSではWFNOTE内でしか使われていないのでここに移植
 *  ルーチンも文字コードからUNICODEに変わったので変更
 */
//------------------------------------------------------------------
static u64 WFNOTE_STRBUF_GetNumber( STRBUF *str , BOOL *pbFlag )
{
  const STRCODE eonCode = GFL_STR_GetEOMCode();
  const STRCODE *code = GFL_STR_GetStringCodePointer( str );

  u64 num = 0;
  u64 digit = 1;
  u16 arrDigit = 0;
  u16 tempNo;

  static const u8 MAX_DIGIT = 18;

  *pbFlag = FALSE;
  while( code[arrDigit] != eonCode )
  {
    if( arrDigit >= MAX_DIGIT )
    {
      //桁オーバー
      return num;
    }

    tempNo = code[arrDigit] - L'0';
    if( tempNo >= 10 )
    {
      tempNo = code[arrDigit] - L'０';
      if( tempNo >= 10 )
      {
        //想定外の文字が入っている
        return num;
      }
    }
    /*
    num += tempNo * digit;
    digit *= 10;
    */
    num *= 10;
    num += tempNo;
    arrDigit++;
  }
  *pbFlag = TRUE;
  return num;
}

#endif  //PM_DEBUG
