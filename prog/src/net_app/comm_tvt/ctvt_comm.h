//======================================================================
/**
 * @file	ctvt_comm.h
 * @brief	通信TVTシステム：通信管
 * @author	ariizumi
 * @data	09/12/17
 *
 * モジュール名：CTVT_COMM
 */

//======================================================================
#pragma once

#include "comm_tvt_local_def.h"
#include "savedata/mystatus.h"
#include "net/ctvt_beacon.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define CTVT_COMM_INVALID_MEMBER (0xFF)

//ビーコン収集数
#define CTVT_COMM_BEACON_NUM (16)

#define CTVT_COMM_TIMING_END (47)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
//フラグのタイプ
typedef enum
{
  //COMMが使う
  CCFT_REQ_PHOTO,

  //COMMが使う＆親機が子に通知する用
  CCFT_TALK_MEMBER,
  CCFT_CHANGE_DOUBLE,
  CCFT_DRAW_BUFFER_NO,
    
  //外で使う
  CCFT_REQ_TALK,  //会話要求通知
  CCFT_FINISH_TALK,  //会話終了通知
  CCFT_CANCEL_TALK,  //会話取り消し通知 //何かしらうまくいかなかったとき

  CCFT_FINISH_PARENT,  //親が終了を要求する
  
  CCFT_MAX,
}CTVT_COMM_FLAG_TYPE;

typedef enum
{
  CCIM_NONE,       //無し(システム用
  CCIM_END,        //終了(システム用

  CCIM_SCAN,       //サーチ中
  CCIM_PARENT,     //親になる
  CCIM_CHILD,      //子になる
  CCIM_CONNECTED,  //すでにつながってる(Wifi
}CTVT_COMM_INIT_MODE;
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct
{
  u16 encSize;
  u16 recSize;
  u8  isLast;
  u8  dataNo;
  u8  pitch;
  u8  pad;
}CTVT_COMM_WAVE_HEADER;


typedef struct
{
  u8  myStatusBuff[MYSTATUS_SAVE_SIZE];
  u8  canUseCamera;
}CTVT_COMM_MEMBER_DATA;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern CTVT_COMM_WORK* CTVT_COMM_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId );
extern void CTVT_COMM_TermSystem( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_Main( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

extern void CTVT_COMM_ExitComm( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_IsExit( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_SetMode( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , CTVT_COMM_INIT_MODE mode );
extern const CTVT_COMM_INIT_MODE CTVT_COMM_GetMode( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_SetMacAddress( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 *address );

extern const BOOL CTVT_COMM_IsInitNet( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

extern const void CTVT_COMM_SendFlgReq_Double( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const BOOL flg );
extern const BOOL CTVT_COMM_SendFlg( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 flg , const u32 value );
extern const BOOL CTVT_COMM_SendWave( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , void* sendData );

extern void CTVT_COMM_SendTimingCommnad( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 number );
extern const BOOL CTVT_COMM_CheckTimingCommnad( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 number );

extern const u8 CTVT_COMM_GetSelfNetId( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const u8 CTVT_COMM_GetTalkMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_GetCommWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_ReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_ResetReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

extern const BOOL CTVT_COMM_IsEnableMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx );
extern CTVT_COMM_MEMBER_DATA* CTVT_COMM_GetMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx );
extern CTVT_COMM_MEMBER_DATA* CTVT_COMM_GetSelfMemberData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_IsEnableMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx );
extern const BOOL CTVT_COMM_CanUseCameraMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 idx );

extern CTVT_COMM_BEACON* CTVT_COMM_GetCtvtBeaconData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
//お絵描きバッファ取得
extern DRAW_SYS_PEN_INFO* CTVT_COMM_GetDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , BOOL *isFull );
//お絵描きバッファ追加通知
extern void CTVT_COMM_AddDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
//--------------------------------------------------------------
//	
//--------------------------------------------------------------