//======================================================================
/**
 * @file	  mb_cap_demo.c
 * @brief	  捕獲ゲーム・開始終了デモ
 * @author	ariizumi
 * @data	  09/02/01
 *
 * モジュール名：MB_CAP_DEMO
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "msg/msg_multiboot_child.h"
#include "system/bmp_winframe.h"

#include "./mb_cap_demo.h"
#include "./mb_cap_down.h"
#include "./mb_cap_snd_def.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_CAP_MSGWIN_LEFT (4)
#define MB_CAP_MSGWIN_TOP  (8)
#define MB_CAP_MSGWIN_WIDTH  (24)
#define MB_CAP_MSGWIN_HEIGHT (4)

#define MB_CAP_MSG_POS_X (FX32_CONST(128.0f))
#define MB_CAP_MSG_POS_Y (FX32_CONST(92.0f))
#define MB_CAP_MSG_POS_Z (FX32_CONST(290.0f))

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  CDOI_READY,
  CDOI_START,
  CDOI_TIMEUP,
  CDOI_FINISH,
  CDOI_SCORE,
  
  CDOI_MAX,
}MB_CAP_DEMO_OBJ_IDX;

typedef enum
{
//開始デモ用ステート
  CDS_MSG_INIT,
  CDS_MSG_INIT_WAIT,
  CDS_MSG_DISP,

  CDS_READY_DISP_INIT,
  CDS_READY_DISP_PULL_BALL,

  CDS_START_DISP_INIT,
  CDS_START_DISP_WAIT,
  CDS_START_DISP_EXIT,

//終了デモ用ステート
  CDF_MSG_INIT,
  CDF_MSG_DISP,
  CDF_MSG_WAIT,
  
  CDF_SCORE_INIT,
  CDF_SCORE_DISP,
  CDF_SCORE_WAIT,
  
}MB_CAP_DEMO_STATE_START;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_CAP_DEMO
{
  BOOL befTpTouch;
  BOOL tpTouch;
  u32  tpx;
  u32  tpy;
  
  BOOL isTimeUp;
  BOOL isHighScore;

  u8  state;
  u16 cnt;

  int objIdx[CDOI_MAX];
  
  GFL_BMPWIN *msgWin;
};
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
MB_CAP_DEMO* MB_CAP_DEMO_InitSystem( MB_CAPTURE_WORK *capWork )
{
  u8 i;
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  
  MB_CAP_DEMO *demoWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_DEMO ) );
  
  for( i=0;i<CDOI_MAX;i++ )
  {
    const BOOL isDisp = FALSE;
    const BOOL isFlip = TRUE;
    VecFx32 pos = {0,0,0};
    const int resIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_DEMO_READY + i );

  //文字演出のためサイズを2倍にしてるのでデフォは半分
    demoWork->objIdx[i] = GFL_BBD_AddObject( bbdSys , 
                                     resIdx ,
                                     FX32_HALF*4 , 
                                     FX32_HALF , 
                                     &pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
    GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[i] , &isDisp );
    //3D設定の関係で反転させる・・・
    GFL_BBD_SetObjectFlipT( bbdSys , demoWork->objIdx[i] , &isFlip );
  }
  demoWork->msgWin = NULL;
  
  return demoWork;
}

void MB_CAP_DEMO_DeleteSystem( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
  if( demoWork->msgWin != NULL )
  {
    GFL_BMPWIN_Delete( demoWork->msgWin );
  }
  
  GFL_HEAP_FreeMemory( demoWork );
}


#pragma mark [> StartDemo
//--------------------------------------------------------------
//	開始Demo初期化
//--------------------------------------------------------------
void MC_CAP_DEMO_StartDemoInit( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
  demoWork->state = CDS_MSG_INIT;
  demoWork->cnt = 0;
  demoWork->befTpTouch = FALSE;
  demoWork->tpTouch = FALSE;
  demoWork->tpx = 0;
  demoWork->tpy = 0;

  demoWork->msgWin = GFL_BMPWIN_Create( MB_CAPTURE_FRAME_MSG , 
                                        MB_CAP_MSGWIN_LEFT , 
                                        MB_CAP_MSGWIN_TOP ,
                                        MB_CAP_MSGWIN_WIDTH , 
                                        MB_CAP_MSGWIN_HEIGHT , 
                                        MB_CAPTURE_PAL_MAIN_BG_FONT ,
                                        GFL_BMP_CHRAREA_GET_B );

}

//--------------------------------------------------------------
//	開始Demo開放
//--------------------------------------------------------------
void MC_CAP_DEMO_StartDemoTerm( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
  GFL_BMPWIN_Delete( demoWork->msgWin );
  demoWork->msgWin = NULL;
}

//--------------------------------------------------------------
//	開始Demoメイン
//--------------------------------------------------------------
const BOOL MC_CAP_DEMO_StartDemoMain( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  MB_CAP_DOWN *downWork = MB_CAPTURE_GetDownWork( capWork );
  
  switch( demoWork->state )
  {
  case CDS_MSG_INIT:
    {
      GFL_FONT *fontHandle = MB_CAPTURE_GetFontHandle( capWork );
      GFL_MSGDATA *msgHandle = MB_CAPTURE_GetMsgHandle( capWork );
      PRINT_QUE *printQue = MB_CAPTURE_GetPrintQue( capWork );
      STRBUF *str;
      
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( demoWork->msgWin) , 0xF );
      str = GFL_MSG_CreateString( msgHandle , MSG_MB_CHILD_CAP_01 );
      PRINTSYS_PrintQue( printQue , GFL_BMPWIN_GetBmp( demoWork->msgWin ) ,
              0 , 0 , str , fontHandle );
      GFL_STR_DeleteBuffer( str );

      BmpWinFrame_Write( demoWork->msgWin , WINDOW_TRANS_OFF , 
                          MB_CAPTURE_MSGWIN_CGX , MB_CAPTURE_PAL_MAIN_BG_MSGWIN );
      demoWork->state = CDS_MSG_INIT_WAIT;
    }
    break;
  
  case CDS_MSG_INIT_WAIT:
    {
      PRINT_QUE *printQue = MB_CAPTURE_GetPrintQue( capWork );
      if( PRINTSYS_QUE_IsFinished( printQue ) == TRUE )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( demoWork->msgWin );
        demoWork->state = CDS_MSG_DISP;
        demoWork->cnt = 0;
      }
    }
    break;
  
  case CDS_MSG_DISP:
    demoWork->cnt++;
    if( demoWork->cnt > 180 )
    {
      VecFx32 pos = { MB_CAP_MSG_POS_X ,
                      MB_CAP_MSG_POS_Y-FX32_CONST(16.0f) ,
                      MB_CAP_MSG_POS_Z };
      
      BmpWinFrame_Clear( demoWork->msgWin , WINDOW_TRANS_OFF );
      GFL_BMPWIN_ClearTransWindow_VBlank( demoWork->msgWin );
      
      GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_READY] , &pos );
      
      demoWork->state = CDS_READY_DISP_INIT;
      demoWork->cnt = 0;
      
      //弾の補充
      MB_CAP_DOWN_ReloadBall( downWork , FALSE );
    }
    break;
  
  //Ready登場
  case CDS_READY_DISP_INIT:
    {
      /*
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y -FX32_CONST(16.0f),
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4;
        fx16 sizeY = FX16_HALF * demoWork->cnt / cntMax;
        
        pos.y += FX32_CONST( demoWork->cnt * 16 / cntMax );
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_READY] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_READY] , &sizeX , &sizeY );
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_READY] , &isDisp );
      }
      else
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4;
        fx16 sizeY = FX16_HALF;
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_READY] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_READY] , &sizeX , &sizeY );
        
        demoWork->state = CDS_READY_DISP_PULL_BALL;
        demoWork->cnt = 0;
      }
      */
      {
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4;
        fx16 sizeY = FX16_HALF;
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_READY] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_READY] , &sizeX , &sizeY );
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_READY] , &isDisp );
        
        PMSND_PlaySE( MB_SND_POKE_READY );

        demoWork->state = CDS_READY_DISP_PULL_BALL;
        demoWork->cnt = 0;
      }
    }
    break;
  
  //ボールを引く
  case CDS_READY_DISP_PULL_BALL:
    {
      const u8 cntMax = 120;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const u16 anmIdx = demoWork->cnt/40;
        GFL_BBD_SetObjectCelIdx( bbdSys , demoWork->objIdx[CDOI_READY] , &anmIdx );
        if( demoWork->cnt > 30 && demoWork->cnt < 90 )
        {
          demoWork->tpTouch = TRUE;
          demoWork->tpx = 128;
          demoWork->tpy = 88 + (demoWork->cnt-30)/2;
        }
        
      }
      else
      {
        const BOOL isDisp = FALSE;
        demoWork->tpTouch = FALSE;
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_READY] , &isDisp );
        PMSND_PlaySE( MB_SND_POKE_START );

        demoWork->state = CDS_START_DISP_INIT;
        demoWork->cnt = 0;
      }
    }
    break;
  
  case CDS_START_DISP_INIT:
    {
      /*
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        //Readyが縮む
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4;
        fx16 sizeY = FX16_HALF * (cntMax-demoWork->cnt) / cntMax;
        pos.y += FX32_CONST( (cntMax-demoWork->cnt) * 16 / cntMax ) - FX32_CONST(16.0f) ;

        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_READY] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_READY] , &sizeX , &sizeY );

        //Startが伸びる
        sizeX = FX16_HALF*4;
        sizeY = FX16_HALF * demoWork->cnt / cntMax;
        pos.y += FX32_CONST( demoWork->cnt * 16 / cntMax );

        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_START] , &sizeX , &sizeY );
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_START] , &isDisp );


      }
      else
      {
        const BOOL isDisp = FALSE;
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4;
        fx16 sizeY = FX16_HALF;
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_START] , &sizeX , &sizeY );
        
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_READY] , &isDisp );

        demoWork->state = CDS_START_DISP_WAIT;
        demoWork->cnt = 0;
      }
      */
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X + FX32_CONST(128.0f+64.0f),
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        
        pos.x -= demoWork->cnt * FX32_CONST(128.0f+64.0f) / cntMax;
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_START] , &isDisp );
      }
      else
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
        demoWork->state = CDS_START_DISP_WAIT;
        demoWork->cnt = 0;
      }
    }    
    break;
  case CDS_START_DISP_WAIT:
    {
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const fx32 ofsArr[4] = {FX32_CONST(2.0f),0,FX32_CONST(-2.0f),0};
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        
        pos.x += ofsArr[demoWork->cnt%4];
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
      }
      else
      {
        demoWork->state = CDS_START_DISP_EXIT;
        demoWork->cnt = 0;
      }
    }
    break;
  case CDS_START_DISP_EXIT:
    {
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        fx16 sizeX = FX16_HALF*4 + FX16_HALF * demoWork->cnt / cntMax * 2;
        fx16 sizeY = FX16_HALF * (cntMax-demoWork->cnt) / cntMax;
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_START] , &pos );
        GFL_BBD_SetObjectSiz( bbdSys , demoWork->objIdx[CDOI_START] , &sizeX , &sizeY );
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_START] , &isDisp );
      }
      else
      {
        const BOOL isDisp = FALSE;
        GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_START] , &isDisp );
        return TRUE;
      }
    }
    break;
  }
  
  {
    BOOL isTrg = FALSE;
    if( demoWork->befTpTouch == FALSE &&
        demoWork->tpTouch == TRUE )
    {
      isTrg = TRUE;
    }
    MB_CAP_DOWN_UpdateSystem_Demo( capWork , downWork , isTrg , demoWork->tpTouch , demoWork->tpx , demoWork->tpy );
    demoWork->befTpTouch = demoWork->tpTouch;
  }
  
  return FALSE;
}

#pragma mark [> EndDemo
//--------------------------------------------------------------
//	終了Demo初期化
//--------------------------------------------------------------
void MC_CAP_DEMO_FinishDemoInit( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork , const BOOL isTimeUp , const BOOL isHighScore )
{
  demoWork->state = CDF_MSG_INIT;
  demoWork->cnt = 0;
  demoWork->isTimeUp = isTimeUp;
  demoWork->isHighScore = isHighScore;
}

//--------------------------------------------------------------
//	終了Demo開放
//--------------------------------------------------------------
void MC_CAP_DEMO_FinishDemoTerm( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
}

//--------------------------------------------------------------
//	終了Demoメイン
//--------------------------------------------------------------
const BOOL MC_CAP_DEMO_FinishDemoMain( MB_CAPTURE_WORK *capWork , MB_CAP_DEMO *demoWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  MB_CAP_DOWN *downWork = MB_CAPTURE_GetDownWork( capWork );
  const u8 strIdx = (demoWork->isTimeUp == TRUE ? CDOI_TIMEUP : CDOI_FINISH);
  
  switch( demoWork->state )
  {
  //Finish or TimeUp 出現
  case CDF_MSG_INIT:
    {
      const BOOL isDisp = TRUE;
      VecFx32 pos = { MB_CAP_MSG_POS_X + FX32_CONST(128.0f+64.0f),
                      MB_CAP_MSG_POS_Y ,
                      MB_CAP_MSG_POS_Z };
      GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[strIdx] , &pos );
      GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[strIdx] , &isDisp );
      demoWork->state = CDF_MSG_DISP;
      PMSND_PlaySE( MB_SND_POKE_FINISH );
    }

    break;
  case CDF_MSG_DISP:
    {
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        const BOOL isDisp = TRUE;
        VecFx32 pos = { MB_CAP_MSG_POS_X + FX32_CONST(128.0f+64.0f),
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };

        pos.x -= demoWork->cnt * FX32_CONST(128.0f+64.0f) / cntMax;

        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[strIdx] , &pos );
      }
      else
      {
        demoWork->state = CDF_MSG_WAIT;
        demoWork->cnt = 0;
      }
    }
    break;
    
  case CDF_MSG_WAIT:
    {
      const u8 cntMax = 120;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        if( demoWork->cnt < 30 )
        {
          const fx32 ofsArr[4] = {FX32_CONST(2.0f),0,FX32_CONST(-2.0f),0};
          
          pos.x += ofsArr[demoWork->cnt%4];
        }
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[strIdx] , &pos );
      }
      else
      {
        if( demoWork->isHighScore == FALSE )
        {
          return TRUE;
        }
        else
        {
          const BOOL isDisp = FALSE;

          GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[strIdx] , &isDisp );
  
          demoWork->state = CDF_SCORE_INIT;
          demoWork->cnt = 0;
        }
      }
    }
    break;
  
  //ハイスコア降りてくる
  case CDF_SCORE_INIT:
    {
      const BOOL isDisp = TRUE;
      VecFx32 pos = { MB_CAP_MSG_POS_X ,
                      0,
                      MB_CAP_MSG_POS_Z };
      GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_SCORE] , &pos );
      GFL_BBD_SetObjectDrawEnable( bbdSys , demoWork->objIdx[CDOI_SCORE] , &isDisp );

      demoWork->state = CDF_SCORE_DISP;
    }
    break;

  case CDF_SCORE_DISP:
    {
      const u8 cntMax = 15;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        0 ,
                        MB_CAP_MSG_POS_Z };
        pos.y += ( MB_CAP_MSG_POS_Y+FX32_CONST(16.0f) )* demoWork->cnt / cntMax;
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_SCORE] , &pos );
      }
      else
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_SCORE] , &pos );
        demoWork->state = CDF_SCORE_WAIT;
        demoWork->cnt = 0;

        PMSND_PlaySE( MB_SND_POKE_HIGH_SCORE );
      }
    }
    break;

  case CDF_SCORE_WAIT:
    {
      const u8 cntMax = 90;
      demoWork->cnt++;
      if( demoWork->cnt < cntMax )
      {
        VecFx32 pos = { MB_CAP_MSG_POS_X ,
                        MB_CAP_MSG_POS_Y ,
                        MB_CAP_MSG_POS_Z };
        if( demoWork->cnt < 30 )
        {
          const fx32 ofsArr[4] = {FX32_CONST(2.0f),0,FX32_CONST(-2.0f),0};
          
          pos.y += ofsArr[demoWork->cnt%4];
        }
        
        GFL_BBD_SetObjectTrans( bbdSys , demoWork->objIdx[CDOI_SCORE] , &pos );      
      }
      else
      {
        return TRUE;
      }
    }
    break;
  }
  
  //スコア更新のため置いておく
  MB_CAP_DOWN_UpdateSystem_Demo( capWork , downWork , FALSE , FALSE , 0 , 0);
  return FALSE;
}
