//======================================================================
/**
 * @file	ctvt_call.c
 * @brief	通信TVTシステム：呼び出し時下画面
 * @author	ariizumi
 * @data	09/12/21
 *
 * モジュール名：CTVT_CALL
 */
//======================================================================
#include <gflib.h>

#include "system/main.h"
#include "system/gfl_use.h"
#include "system/wipe.h"
#include "system/camera_system.h"
#include "net/network_define.h"
#include "net/net_whpipe.h"
#include "app/app_menu_common.h"
#include "print/wordset.h"
#include "field/game_beacon_search.h"

#include "msg/msg_comm_tvt.h"

#include "comm_tvt.naix"

#include "ctvt_call.h"
#include "ctvt_comm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//表示バーの個数
#define CTVT_CALL_BAR_NUM (16) //とりあえず画面分
//データ個数
#define CTVT_CALL_SEARCH_NUM (16) //とりあえず画面分
#define CTVT_CALL_INVALID_NO (0xFF)


#define CTVT_CALL_BAR_X (128)
#define CTVT_CALL_BAR_Y (16)
#define CTVT_CALL_BAR_HEIGHT (32)

#define CTVT_CALL_SCROLL_LEN (CTVT_CALL_BAR_HEIGHT*CTVT_CALL_SEARCH_NUM-20*8)

#define CTVT_CALL_CHECK_X (16)

#define CTVT_CALL_BEACONSIZE (sizeof(GBS_BEACON)<sizeof(CTVT_COMM_BEACON)?sizeof(CTVT_COMM_BEACON):sizeof(GBS_BEACON))


#define CTVT_STRBMP_X (6)
#define CTVT_STRBMP_Y (0)
#define CTVT_STRBMP_WIDTH (22)
#define CTVT_STRBMP_HEIGHT (32)

#define CTVT_CALL_RETURN_X (224)


//======================================================================
//	enum
//======================================================================
#pragma mark [> enum

typedef enum
{
  CCS_FADEIN,
  CCS_FADEIN_WAIT,
  CCS_FADEOUT,
  CCS_FADEOUT_WAIT,

  CCS_MAIN,
  CCS_WAIT_CONNECT,

}CTVT_CALL_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef struct _CTVT_CALL_MEMBER_WORK CTVT_CALL_MEMBER_WORK;
typedef struct _CTVT_CALL_BAR_WORK CTVT_CALL_BAR_WORK;

//メンバー情報
struct _CTVT_CALL_MEMBER_WORK
{
  BOOL isEnable;
  BOOL isLost;
  BOOL isCheck;
  void *beacon;
  GameServiceID serviceType;
  u8 macAddress[6];
  
  u8 barWorkNo;
};

//バーワーク
struct _CTVT_CALL_BAR_WORK
{
  BOOL isUpdate;
  BOOL isUpdateStr;
  
  GFL_CLWK *clwkBar;
  GFL_CLWK *clwkCheck;

  GFL_BMPWIN *strWin;
  
  u8 memberWorkNo;
};


//ワーク
struct _CTVT_CALL_WORK
{
  u8 checkIdx[3];
  u16 scrollOfs;
  CTVT_CALL_STATE state;
  BOOL isUpdateBarPos;

  CTVT_CALL_BAR_WORK barWork[CTVT_CALL_BAR_NUM];
  CTVT_CALL_MEMBER_WORK memberData[CTVT_CALL_SEARCH_NUM];

  GFL_CLWK *clwkReturn;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_CALL_UpdateTP( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );

static void CTVT_CALL_UpdateBeacon( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );
static void CTVT_CALL_UpdateBar( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );
static void CTVT_CALL_UpdateBarFunc( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork , CTVT_CALL_BAR_WORK *barWork , const u8 idx );
static void CTVT_CALL_UpdateBarPosFunc( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork , CTVT_CALL_BAR_WORK *barWork , const u8 idx );


//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
CTVT_CALL_WORK* CTVT_CALL_InitSystem( COMM_TVT_WORK *work , const HEAPID heapId )
{
  u8 i;
  CTVT_CALL_WORK* callWork = GFL_HEAP_AllocClearMemory( heapId , sizeof(CTVT_CALL_WORK) );
  
  return callWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void CTVT_CALL_TermSystem( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  
  GFL_HEAP_FreeMemory( callWork );
}
//--------------------------------------------------------------
//	モード切替時初期化
//--------------------------------------------------------------
void CTVT_CALL_InitMode( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  ARCHANDLE* arcHandle = COMM_TVT_GetArcHandle( work );
  
  {
    u8 i;
    GFL_CLWK_DATA cellInitData;
    cellInitData.bgpri = 3;
    
    for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
    {
      cellInitData.pos_x = CTVT_CALL_BAR_X;
      cellInitData.pos_y = CTVT_CALL_BAR_Y + CTVT_CALL_BAR_HEIGHT*i;
      cellInitData.anmseq = CTOAS_YOBIDASHI_BAR;
      cellInitData.softpri = 32;
      callWork->barWork[i].clwkBar = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
                &cellInitData ,CLSYS_DRAW_SUB , heapId );
      GFL_CLACT_WK_SetDrawEnable( callWork->barWork[i].clwkBar , FALSE );

      cellInitData.pos_x = CTVT_CALL_CHECK_X;
      cellInitData.anmseq = CTOAS_CHECK;
      cellInitData.softpri = 16;
      callWork->barWork[i].clwkCheck = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_NCG ),
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_PLT ),
                COMM_TVT_GetObjResIdx( work, CTOR_COMMON_S_ANM ),
                &cellInitData ,CLSYS_DRAW_SUB , heapId );
      GFL_CLACT_WK_SetDrawEnable( callWork->barWork[i].clwkCheck , FALSE );
      
    }

    //戻るボタン
    cellInitData.pos_x = CTVT_CALL_RETURN_X;
    cellInitData.pos_y = 192-24;
    cellInitData.anmseq = APP_COMMON_BARICON_RETURN;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    
    callWork->clwkReturn = GFL_CLACT_WK_Create( COMM_TVT_GetCellUnit(work) ,
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_NCG ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_S_PLT ),
              COMM_TVT_GetObjResIdx( work, CTOR_BAR_BUTTON_ANM ),
              &cellInitData ,CLSYS_DRAW_SUB , heapId );
    GFL_CLACT_WK_SetDrawEnable( callWork->clwkReturn , TRUE );
  }
  {
    u8 i;
    for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
    {
      callWork->memberData[i].isEnable = FALSE;
      callWork->memberData[i].isLost = FALSE;
      callWork->memberData[i].isCheck = FALSE;
      callWork->memberData[i].barWorkNo = CTVT_CALL_INVALID_NO;
      callWork->memberData[i].beacon = GFL_HEAP_AllocClearMemory( heapId , CTVT_CALL_BEACONSIZE );
    }
    for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
    {
      callWork->barWork[i].isUpdate = FALSE;
      callWork->barWork[i].isUpdateStr = FALSE;
      callWork->barWork[i].memberWorkNo = CTVT_CALL_INVALID_NO;
      callWork->barWork[i].strWin = GFL_BMPWIN_Create( CTVT_FRAME_SUB_MISC , 
                                          3 , i*(CTVT_CALL_BAR_HEIGHT/8)+1 , 25 , 2 ,
                                          CTVT_PAL_BG_SUB_FONT ,
                                          GFL_BMP_CHRAREA_GET_B );
    }
  }
  {
    u8 i;
    for( i=0;i<3;i++ )
    {
      callWork->checkIdx[i] = CTVT_CALL_INVALID_NO;
    }
  }
  callWork->scrollOfs = 0;
  callWork->isUpdateBarPos = FALSE;

  GFL_BG_SetScrollReq( CTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , 0 );

}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_CALL_TermMode( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  u8 i;
  
  GFL_CLACT_WK_Remove( callWork->clwkReturn );
  for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
  {
    GFL_BMPWIN_Delete( callWork->barWork[i].strWin );
    GFL_CLACT_WK_Remove( callWork->barWork[i].clwkCheck );
    GFL_CLACT_WK_Remove( callWork->barWork[i].clwkBar );
  }
  for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
  {
    GFL_HEAP_FreeMemory( callWork->memberData[i].beacon );
  }

  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );

  GFL_BG_SetScrollReq( CTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , 0 );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
const COMM_TVT_MODE CTVT_CALL_Main( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  const HEAPID heapId = COMM_TVT_GetHeapId( work );
  
  switch( callWork->state )
  {
  case CCS_FADEIN:
    if( COMM_TVT_GetUpperFade( work ) == TRUE )
    {
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
              WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    else
    {
      WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
              WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    }
    callWork->state = CCS_FADEIN_WAIT;
    break;
  case CCS_FADEIN_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      callWork->state = CCS_MAIN;
    }
    break;

  case CCS_FADEOUT:
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , heapId );
    COMM_TVT_SetUpperFade( work , FALSE );
    callWork->state = CCS_FADEOUT_WAIT;
    break;

  case CCS_FADEOUT_WAIT:
    if( WIPE_SYS_EndCheck() == TRUE )
    {
      return CTM_TALK;
    }
    break;
    
  case CCS_MAIN:
    CTVT_CALL_UpdateTP( work , callWork );
    {
      static const GFL_UI_TP_HITTBL hitTbl[2] = 
      {
        {
          168 , 192 ,
          CTVT_CALL_RETURN_X , CTVT_CALL_RETURN_X+24 ,
        },
        {GFL_UI_TP_HIT_END,0,0,0}
      };
      const int ret = GFL_UI_TP_HitTrg( hitTbl );
      if( ret == 0 || GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
      {
        CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
        callWork->state = CCS_FADEOUT;
        CTVT_COMM_SetMode( work , commWork , CCIM_PARENT );
        //ビーコンのセット
        {
          u8 i;
          CTVT_COMM_BEACON *beacon = CTVT_COMM_GetCtvtBeaconData( work , commWork );
          for( i=0;i<3;i++ )
          {
            u8 j;
            if( callWork->checkIdx[i] == CTVT_CALL_INVALID_NO )
            {
              for( j=0;j<6;j++ )
              {
                beacon->callTarget[i][j] = 0xFF;
              }
            }
            else
            {
              for( j=0;j<6;j++ )
              {
                beacon->callTarget[i][j] = callWork->memberData[ callWork->checkIdx[i] ].macAddress[j];
              }
            }
          }
          for( i=0;i<3;i++ )
          {
            u8 j;
            OS_TFPrintf(3,"[%d][",callWork->checkIdx[i]);
            for( j=0;j<6;j++ )
            {
              OS_TFPrintf(3,"%02x",beacon->callTarget[i][j]);
            }
            OS_TFPrintf(3,"]\n");
          }
        }
      }
    }
    if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )
    {
      if( callWork->scrollOfs < CTVT_CALL_SCROLL_LEN )
      {
        callWork->isUpdateBarPos = TRUE;
        callWork->scrollOfs += 2;
        GFL_BG_SetScrollReq( CTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , callWork->scrollOfs );
      }
    }
    if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )
    {
      if( callWork->scrollOfs > 0 )
      {
        callWork->isUpdateBarPos = TRUE;
        callWork->scrollOfs -= 2;
        GFL_BG_SetScrollReq( CTVT_FRAME_SUB_MISC , GFL_BG_SCROLL_Y_SET , callWork->scrollOfs );
      }
    }
    break;
  case CCS_WAIT_CONNECT:
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      if( COMM_TVT_GetConnectNum( work ) >= 2 )
      {
        callWork->state = CCS_FADEOUT;
      }
    }
    break;
  
  }
  
  CTVT_CALL_UpdateBeacon( work , callWork );
  {
    u8 i;
    for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
    {
      if( callWork->barWork[i].isUpdateStr == TRUE )
      {
        PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
        if( PRINTSYS_QUE_IsExistTarget( printQue , GFL_BMPWIN_GetBmp( callWork->barWork[i].strWin )) == FALSE )
        {
          GFL_BMPWIN_MakeTransWindow_VBlank( callWork->barWork[i].strWin );
          callWork->barWork[i].isUpdateStr = FALSE;
        }
      }
    }
  }
  if( callWork->isUpdateBarPos == TRUE )
  {
    u8 i;
    for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
    {
      CTVT_CALL_UpdateBarPosFunc( work , callWork , &callWork->barWork[i] , i );
    }
  }

  return CTM_CALL;
}

//--------------------------------------------------------------
//	TP更新
//--------------------------------------------------------------
static void CTVT_CALL_UpdateTP( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  u32 tpx,tpy;
  const BOOL isTrg = GFL_UI_TP_GetPointTrg( &tpx,&tpy );
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  
  if( isTrg == TRUE )
  {
    if( tpx > 8 && tpx < 256-(8*4) )
    {
      const s16 idx = (tpy+callWork->scrollOfs)/CTVT_CALL_BAR_HEIGHT;
      OS_TFPrintf(3,"IDX[%d]\n",idx);
      if( idx >= 0 && idx < CTVT_CALL_BAR_NUM )
      {
        const u8 memIdx = callWork->barWork[idx].memberWorkNo;
        CTVT_CALL_MEMBER_WORK *memberWork = &callWork->memberData[memIdx];
        
        if( memberWork->isEnable == TRUE )
        {
          if( memberWork->serviceType == WB_NET_COMM_TVT )
          {
            CTVT_COMM_SetMode( work , commWork , CCIM_CHILD );
            CTVT_COMM_SetMacAddress( work , commWork , memberWork->macAddress );
            callWork->state = CCS_WAIT_CONNECT;
          }
          else
          {
            if( memberWork->isCheck == FALSE )
            {
              u8 i;
              for( i=0;i<3;i++ )
              {
                if( callWork->checkIdx[i] == CTVT_CALL_INVALID_NO )
                {
                  callWork->checkIdx[i] = memIdx;
                  break;
                }
              }
              if( i<3 )
              {
                memberWork->isCheck = TRUE;
                GFL_CLACT_WK_SetAnmSeq( callWork->barWork[idx].clwkCheck , CTOAS_CHECK_SELECT );
              }
            }
            else
            {
              u8 i;
              for( i=0;i<3;i++ )
              {
                if( callWork->checkIdx[i] == memIdx )
                {
                  callWork->checkIdx[i] = CTVT_CALL_INVALID_NO;
                  break;
                }
              }
              memberWork->isCheck = FALSE;
              GFL_CLACT_WK_SetAnmSeq( callWork->barWork[idx].clwkCheck , CTOAS_CHECK );              
            }
          }
        }
      }
    }
  }
  
}

//--------------------------------------------------------------
//	ビーコン更新
//--------------------------------------------------------------
static void CTVT_CALL_UpdateBeacon( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  u8 i;
  BOOL isUpdate = FALSE;
  CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
  if( CTVT_COMM_IsInitNet( work , commWork ) == FALSE )
  {
    return;
  }
  
  for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
  {
    if( callWork->memberData[i].isEnable == TRUE )
    {
      callWork->memberData[i].isLost = TRUE;
    }
  }

  for( i=0;i<CTVT_COMM_BEACON_NUM;i++ )
  {
    BOOL isEntry = FALSE;
    void *beaconData = GFL_NET_GetBeaconData(i);
    if( beaconData != NULL )
    {
      u8 emptyNo = 0xFF;
      u8 mem;
      u8 *macAddress = GFL_NET_GetBeaconMacAddress( i );
      const GameServiceID serviceType = GFL_NET_WLGetUserGameServiceId( i );
      if( serviceType == WB_NET_FIELDMOVE_SERVICEID )
      {
        //フィールドビーコンをはじく
        GBS_BEACON *becData = beaconData;
        if( becData->beacon_type != GBS_BEACONN_TYPE_PALACE &&
            becData->beacon_type != GBS_BEACONN_TYPE_INFO )
        {
          continue;
        }
      }

      for( mem=0;mem<CTVT_CALL_SEARCH_NUM;mem++ )
      {
        if( callWork->memberData[mem].isEnable == TRUE )
        {
          //すでに登録済みか？
          u8 k;
          BOOL isSame = TRUE;
          for( k=0;k<6;k++ )
          {
            if( macAddress[k] != callWork->memberData[mem].macAddress[k] )
            {
              isSame = FALSE;
              break;
            }
          }
          if( isSame == TRUE )
          {
            //登録済だった
            callWork->memberData[mem].isLost = FALSE;
            isEntry = TRUE;
            break;
          }
        }
        else
        {
          //空き番号を確保しておく
          if( emptyNo == 0xFF )
          {
            emptyNo = mem;
          }
        }
      }
      
      if( isEntry == FALSE )
      {
        //未登録だった
        if( emptyNo != 0xFF )
        {
          //空きがあったので登録
          u8 k;

          callWork->memberData[emptyNo].isEnable = TRUE;
          callWork->memberData[emptyNo].isLost = FALSE;
          for( k=0;k<6;k++ )
          {
            callWork->memberData[emptyNo].macAddress[k] = macAddress[k];
          }
          if( serviceType == WB_NET_COMM_TVT )
          {
            callWork->memberData[emptyNo].serviceType = WB_NET_COMM_TVT;
            GFL_STD_MemCopy( beaconData , callWork->memberData[emptyNo].beacon , sizeof(CTVT_COMM_BEACON) );
          }
          else
          {
            callWork->memberData[emptyNo].serviceType = WB_NET_FIELDMOVE_SERVICEID;
            GFL_STD_MemCopy( beaconData , callWork->memberData[emptyNo].beacon , sizeof(GBS_BEACON) );
          }
          
          for( k=0;k<CTVT_CALL_BAR_NUM;k++ )
          {
            if( callWork->barWork[k].memberWorkNo == CTVT_CALL_INVALID_NO )
            {
              callWork->barWork[k].memberWorkNo = emptyNo;
              callWork->memberData[emptyNo].barWorkNo = k;
              callWork->barWork[k].isUpdate = TRUE;
              break;
            }
          }
          isUpdate = TRUE;
        }
      }
    }
  }
  
  //消える処理
  for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
  {
    if( callWork->memberData[i].isEnable == TRUE &&
        callWork->memberData[i].isLost   == TRUE )
    {
      u8 j;
      for( j=callWork->memberData[i].barWorkNo ; j<CTVT_CALL_BAR_NUM-1 ; j++ )
      {
        if( callWork->barWork[j+1].memberWorkNo != CTVT_CALL_INVALID_NO )
        {
          callWork->barWork[j].memberWorkNo = callWork->barWork[j+1].memberWorkNo;
          callWork->barWork[j].isUpdate = TRUE;
        }
        else
        {
          break;
        }
      }
      for( j=0;j<3;j++ )
      {
        if( callWork->checkIdx[j] == i )
        {
          callWork->checkIdx[j] = CTVT_CALL_INVALID_NO;
        }
      }
      callWork->memberData[i].isEnable = FALSE;
      callWork->barWork[j].memberWorkNo = CTVT_CALL_INVALID_NO;
      isUpdate = TRUE;
    }
  }
  
  
  if( isUpdate == TRUE )
  {
    /*
    u8 i;
    OS_TFPrintf(3,"--------------------------------\n");
    for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
    {
      OS_TFPrintf(3,"[%d][%x:%x:%x:%x:%x:%x]\n",
        callWork->memberData[i].isEnable ,
        callWork->memberData[i].macAddress[0] ,
        callWork->memberData[i].macAddress[1] ,
        callWork->memberData[i].macAddress[2] ,
        callWork->memberData[i].macAddress[3] ,
        callWork->memberData[i].macAddress[4] ,
        callWork->memberData[i].macAddress[5] 
        );
    }
    //*/
    CTVT_TPrintf("UpdateBar\n");
    CTVT_CALL_UpdateBar( work , callWork );
    callWork->isUpdateBarPos = TRUE;
    
  }
}
//--------------------------------------------------------------
//	バー更新
//--------------------------------------------------------------

static void CTVT_CALL_UpdateBar( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  u8 i;
  for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
  {
    CTVT_CALL_UpdateBarFunc( work , callWork , &callWork->barWork[i] , i );
  }
}

static void CTVT_CALL_UpdateBarFunc( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork , CTVT_CALL_BAR_WORK *barWork , const u8 idx )
{
  if( barWork->isUpdate == TRUE )
  {
    if( barWork->memberWorkNo != CTVT_CALL_INVALID_NO )
    {
      const HEAPID heapId = COMM_TVT_GetHeapId( work );
      GFL_FONT *fontHandle = COMM_TVT_GetFontHandle( work );
      GFL_MSGDATA *msgHandle = COMM_TVT_GetMegHandle( work );
      PRINT_QUE *printQue = COMM_TVT_GetPrintQue( work );
      const CTVT_CALL_MEMBER_WORK *memberWork = &callWork->memberData[barWork->memberWorkNo];
      WORDSET *wordSet = WORDSET_Create( heapId );
      STRBUF *str;
      
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( barWork->strWin ) , 0x0 );
      
      //名前
      str = GFL_STR_CreateBuffer( 32 , heapId );
      if( memberWork->serviceType == WB_NET_COMM_TVT )
      {
        CTVT_COMM_BEACON *becData = memberWork->beacon;
        GFL_STR_SetStringCodeOrderLength( str , becData->name , CTVT_COMM_NAME_LEN );
      }
      else
      {
        GBS_BEACON *becData = memberWork->beacon;
        GFL_STR_SetStringCodeOrderLength( str , becData->info.name , CTVT_COMM_NAME_LEN );
      }
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              0 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );

      //ID
      str = GFL_MSG_CreateString( msgHandle , COMM_TVT_CALL_01 );
      {
        STRBUF *tempStr = GFL_STR_CreateBuffer( 32 , heapId );
        if( memberWork->serviceType == WB_NET_COMM_TVT )
        {
          CTVT_COMM_BEACON *becData = memberWork->beacon;
          WORDSET_RegisterNumber( wordSet , 0 , becData->id , 5 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
        }
        else
        {
          GBS_BEACON *becData = memberWork->beacon;
          u16 id = (becData->info.trainer_id & 0xFFFF);
          WORDSET_RegisterNumber( wordSet , 0 , id , 5 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
        }
        WORDSET_ExpandStr( wordSet , tempStr , str );
        GFL_STR_DeleteBuffer( str );
        str = tempStr;
      }
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              80 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );

      //人数
      if( memberWork->serviceType == WB_NET_COMM_TVT )
      {
        CTVT_COMM_BEACON *becData = memberWork->beacon;
        str = GFL_MSG_CreateString( msgHandle , COMM_TVT_CALL_03 );
        {
          STRBUF *tempStr = GFL_STR_CreateBuffer( 32 , heapId );
          WORDSET_RegisterNumber( wordSet , 0 , becData->connectNum , 1 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
          WORDSET_ExpandStr( wordSet , tempStr , str );
          GFL_STR_DeleteBuffer( str );
          str = tempStr;
        }
      }
      else
      {
        str = GFL_MSG_CreateString( msgHandle , COMM_TVT_CALL_02 );
      }

      
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              152 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );
      
      
      WORDSET_Delete( wordSet );
      barWork->isUpdateStr = TRUE;
    }
    else
    {
      GFL_BMPWIN_ClearTransWindow_VBlank( barWork->strWin );
    }
    barWork->isUpdate = FALSE;
  }
}

static void CTVT_CALL_UpdateBarPosFunc( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork , CTVT_CALL_BAR_WORK *barWork , const u8 idx )
{
  if( barWork->memberWorkNo == CTVT_CALL_INVALID_NO )
  {
    GFL_CLACT_WK_SetDrawEnable( barWork->clwkBar , FALSE );
    GFL_CLACT_WK_SetDrawEnable( barWork->clwkCheck , FALSE );
  }
  else
  {
    s16 posY = idx*CTVT_CALL_BAR_HEIGHT - callWork->scrollOfs;
    if( posY >= -CTVT_CALL_BAR_HEIGHT &&
        posY <= 192-CTVT_CALL_BAR_HEIGHT )
    {
      GFL_CLACTPOS pos;
      pos.x = CTVT_CALL_BAR_X;
      pos.y = CTVT_CALL_BAR_Y + posY;
      GFL_CLACT_WK_SetPos( barWork->clwkBar , &pos , CLSYS_DRAW_SUB );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkBar , TRUE );
      pos.x = CTVT_CALL_CHECK_X;
      GFL_CLACT_WK_SetPos( barWork->clwkCheck , &pos , CLSYS_DRAW_SUB );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkCheck , TRUE );

      if( callWork->memberData[barWork->memberWorkNo].isCheck == TRUE )
      {
        GFL_CLACT_WK_SetAnmSeq( callWork->barWork[idx].clwkCheck , CTOAS_CHECK_SELECT );
      }
      else
      {
        GFL_CLACT_WK_SetAnmSeq( callWork->barWork[idx].clwkCheck , CTOAS_CHECK );
      }
    }
    else
    {
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkBar , FALSE );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkCheck , FALSE );
    }
    
  }
}
