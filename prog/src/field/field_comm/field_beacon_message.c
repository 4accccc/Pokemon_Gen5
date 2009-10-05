//======================================================================
/**
 * @file	
 * @brief	
 * @author	ariizumi
 * @data	
 *
 * モジュール名：
 */
//======================================================================
#include <gflib.h>
#include "buflen.h"
#include "system/main.h"
#include "system/gfl_use.h"

#include "field_beacon_message.h"

//======================================================================
//  define
//======================================================================
#pragma mark [> define

//======================================================================
//  enum
//======================================================================
#pragma mark [> enum


//======================================================================
//  typedef struct
//======================================================================
#pragma mark [> struct
struct _FB_MESSAGE_SYS
{
  u8 dummy;
};

//メッセージの表示データ
struct _FB_MESSAGE_DATA
{
  u16 work[4];
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];  //8

  u8  senderIdx;
};

//メッセージ発信者のデータ
struct _FB_SENDER_STATE
{
  u8 macAddress[6];
  u8 cnt;
  u8 dataIdx;
};

//======================================================================
//  proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	ビーコンのチェック
//--------------------------------------------------------------
void FIELD_BEACON_MSG_CheckBeacon( FB_MESSAGE_SYS *fbmSys , GBS_BEACON *beacon )
{
  if( beacon->beacon_type == GBS_BEACONN_TYPE_MESSAGE )
  {
    OS_TPrintf("I got message!!");
  }
}

