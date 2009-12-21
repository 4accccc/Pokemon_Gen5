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
#include "app/app_menu_common.h"
#include "print/wordset.h"

#include "msg/msg_comm_tvt.h"

#include "comm_tvt.naix"

#include "ctvt_call.h"
#include "ctvt_comm.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

//表示バーの個数
#define CTVT_CALL_BAR_NUM (7) //とりあえず画面分
//データ個数
#define CTVT_CALL_SEARCH_NUM (7) //とりあえず画面分
#define CTVT_CALL_INVALID_NO (0xFF)


#define CTVT_CALL_BAR_X (128)
#define CTVT_CALL_BAR_Y (16)
#define CTVT_CALL_BAR_HEIGHT (32)

#define CTVT_CALL_CHECK_X (16)

#define CTVT_STRBMP_X (6)
#define CTVT_STRBMP_Y (0)
#define CTVT_STRBMP_WIDTH (22)
#define CTVT_STRBMP_HEIGHT (32)

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
  CTVT_COMM_BEACON beacon;
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
  CTVT_CALL_STATE state;
  
  CTVT_CALL_BAR_WORK barWork[CTVT_CALL_BAR_NUM];
  CTVT_CALL_MEMBER_WORK memberData[CTVT_CALL_SEARCH_NUM];
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void CTVT_CALL_UpdateTP( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );

static void CTVT_CALL_UpdateBeacon( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );
static void CTVT_CALL_UpdateBar( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork );


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
    //スライダー
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
  }
  {
    u8 i;
    for( i=0;i<CTVT_CALL_SEARCH_NUM;i++ )
    {
      callWork->memberData[i].isEnable = FALSE;
      callWork->memberData[i].isLost = FALSE;
      callWork->memberData[i].barWorkNo = CTVT_CALL_INVALID_NO;
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

}

//--------------------------------------------------------------
//	モード切替時開放
//--------------------------------------------------------------
void CTVT_CALL_TermMode( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork )
{
  u8 i;
  
  for( i=0;i<CTVT_CALL_BAR_NUM;i++ )
  {
    GFL_BMPWIN_Delete( callWork->barWork[i].strWin );
    GFL_CLACT_WK_Remove( callWork->barWork[i].clwkCheck );
    GFL_CLACT_WK_Remove( callWork->barWork[i].clwkBar );
  }

  GFL_BG_ClearScreen( CTVT_FRAME_SUB_MISC );
  GFL_BG_LoadScreenReq( CTVT_FRAME_SUB_MISC );
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
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
    {
      CTVT_COMM_WORK *commWork = COMM_TVT_GetCommWork( work );
      callWork->state = CCS_FADEOUT;
      CTVT_COMM_SetMode( work , commWork , CCIM_PARENT );
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
      const u8 idx = tpy/CTVT_CALL_BAR_HEIGHT;
      const u8 memIdx = callWork->barWork[idx].memberWorkNo;
      const CTVT_CALL_MEMBER_WORK *memberWork = &callWork->memberData[memIdx];
      
      if( memberWork->isEnable == TRUE )
      {
        CTVT_COMM_SetMode( work , commWork , CCIM_CHILD );
        CTVT_COMM_SetMacAddress( work , commWork , memberWork->macAddress );
        callWork->state = CCS_WAIT_CONNECT;
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
          GFL_STD_MemCopy( beaconData , &callWork->memberData[emptyNo].beacon , sizeof(CTVT_COMM_BEACON) );
          
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
      callWork->memberData[i].isEnable = FALSE;
      callWork->barWork[j].memberWorkNo = CTVT_CALL_INVALID_NO;
      isUpdate = TRUE;
    }
  }
  
  
  if( isUpdate == TRUE )
  {
    //*
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
    
  }
}
//--------------------------------------------------------------
//	バー更新
//--------------------------------------------------------------
static void CTVT_CALL_UpdateBarFunc( COMM_TVT_WORK *work , CTVT_CALL_WORK *callWork , CTVT_CALL_BAR_WORK *barWork , const u8 idx );

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
      GFL_STR_SetStringCodeOrderLength( str , memberWork->beacon.name , CTVT_COMM_NAME_LEN );
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              0 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );

      //ID
      str = GFL_MSG_CreateString( msgHandle , COMM_TVT_CALL_01 );
      {
        STRBUF *tempStr = GFL_STR_CreateBuffer( 32 , heapId );
        WORDSET_RegisterNumber( wordSet , 0 , memberWork->beacon.id , 5 , STR_NUM_DISP_SPACE , STR_NUM_CODE_DEFAULT );
        WORDSET_ExpandStr( wordSet , tempStr , str );
        GFL_STR_DeleteBuffer( str );
        str = tempStr;
      }
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              80 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );

      //人数
      str = GFL_MSG_CreateString( msgHandle , COMM_TVT_CALL_03 );
      {
        STRBUF *tempStr = GFL_STR_CreateBuffer( 32 , heapId );
        WORDSET_RegisterNumber( wordSet , 0 , memberWork->beacon.connectNum , 1 , STR_NUM_DISP_LEFT , STR_NUM_CODE_DEFAULT );
        WORDSET_ExpandStr( wordSet , tempStr , str );
        GFL_STR_DeleteBuffer( str );
        str = tempStr;
      }
      PRINTSYS_PrintQueColor( printQue , GFL_BMPWIN_GetBmp( barWork->strWin ) , 
              152 , 0 , str , fontHandle ,CTVT_FONT_COLOR_WHITE );
      GFL_STR_DeleteBuffer( str );
      
      
      WORDSET_Delete( wordSet );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkBar , TRUE );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkCheck , TRUE );

      barWork->isUpdateStr = TRUE;
    }
    else
    {
      GFL_BMPWIN_ClearTransWindow_VBlank( barWork->strWin );
      
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkBar , FALSE );
      GFL_CLACT_WK_SetDrawEnable( barWork->clwkCheck , FALSE );
    }
    barWork->isUpdate = FALSE;
  }
}
