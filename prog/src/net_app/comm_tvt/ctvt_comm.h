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
//======================================================================
//	define
//======================================================================
#pragma mark [> define
#define CTVT_COMM_INVALID_MEMBER (0xFF)
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
    
  //外で使う
  CCFT_REQ_TALK,  //会話要求通知
  CCFT_FINISH_TALK,  //会話終了通知
  CCFT_CANCEL_TALK,  //会話取り消し通知 //何かしらうまくいかなかったとき
  
  CCFT_MAX,
}CTVT_COMM_FLAG_TYPE;


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
  u8  pad[2];
}CTVT_COMM_WAVE_HEADER;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern CTVT_COMM_WORK* CTVT_COMM_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId );
extern void CTVT_COMM_TermSystem( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_Main( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

extern const BOOL CTVT_COMM_SendFlg( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , const u8 flg , const u32 value );
extern const BOOL CTVT_COMM_SendWave( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , void* sendData );

extern const u8 CTVT_COMM_GetSelfNetId( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const u8 CTVT_COMM_GetTalkMember( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_GetCommWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern const BOOL CTVT_COMM_ReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
extern void CTVT_COMM_ResetReqPlayWaveData( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );

//お絵描きバッファ取得
extern DRAW_SYS_PEN_INFO* CTVT_COMM_GetDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork , BOOL *isFull );
//お絵描きバッファ追加通知
extern void CTVT_COMM_AddDrawBuf( COMM_TVT_WORK *work , CTVT_COMM_WORK *commWork );
//--------------------------------------------------------------
//	
//--------------------------------------------------------------