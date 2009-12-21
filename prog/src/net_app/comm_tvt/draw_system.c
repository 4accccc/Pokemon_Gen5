//======================================================================
/**
 * @file	draw_system.c
 * @brief	画面お絵描きシステム
 * @author	ariizumi
 * @data	09/12/20
 *
 * モジュール名：DRAW_SYS
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "draw_system.h"
//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _DRAW_SYS_WORK
{
  u8 frame;
  BOOL isLoop;    //バッファが一周したか？
  
  u16 bufNum;
  u16 topBuffer;  //セットされている位置
  u16 nowBuffer;  //描画されている位置
  
  u16 areaLeft;
  u16 areaRight;
  u16 areaTop;
  u16 areaBottom;

  DRAW_SYS_PEN_INFO *infoBuf;
};

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void DRAW_SYS_DrawInfo( DRAW_SYS_WORK* work , const DRAW_SYS_PEN_INFO* info );
static void DRAW_SYS_DrawLine( DRAW_SYS_WORK* work , void *vramAdr , const u16 x1 , const u16 x2 , const u16 y1 , const u16 y2 , const DRAW_SYS_PEN_SIZE penType , const u16 col );
static void DRAW_SYS_DrawDot_DirectBmp( DRAW_SYS_WORK* work , void *vramAdr , const u16 x , const u16 y , const u16 col );

static void* DRAW_SYS_GetVramAdr( DRAW_SYS_WORK* work );
static DRAW_SYS_PEN_INFO* DRAW_SYS_GetInfoBuf( DRAW_SYS_WORK* work , const u16 pos );
static void DRAW_SYS_IncBufCnt( DRAW_SYS_WORK* work , u16 *pos );

//--------------------------------------------------------------
//	システム作成
//--------------------------------------------------------------
DRAW_SYS_WORK* DRAW_SYS_CreateSystem( const DRAW_SYS_INIT_WORK *initWork )
{
  DRAW_SYS_WORK* work = GFL_HEAP_AllocClearMemory( initWork->heapId , sizeof(DRAW_SYS_WORK) );
  work->infoBuf = GFL_NET_Align32Alloc( initWork->heapId , sizeof(DRAW_SYS_PEN_INFO)*initWork->bufferNum );

  work->frame = initWork->frame;
  work->isLoop = FALSE;
  
  work->bufNum     = initWork->bufferNum;
  work->areaLeft   = initWork->areaLeft;
  work->areaRight  = initWork->areaRight;
  work->areaTop    = initWork->areaTop;
  work->areaBottom = initWork->areaBottom;

  work->topBuffer = 0;
  work->nowBuffer = 0;
  return work;
}

//--------------------------------------------------------------
//	システム開放
//--------------------------------------------------------------
void DRAW_SYS_DeleteSystem( DRAW_SYS_WORK* work )
{
  GFL_NET_Align32Free( work->infoBuf );
  GFL_HEAP_FreeMemory( work );
}

//--------------------------------------------------------------
//	システム更新
//--------------------------------------------------------------
void DRAW_SYS_UpdateSystem( DRAW_SYS_WORK* work )
{
}

//--------------------------------------------------------------
//	システム更新(VBlank
//--------------------------------------------------------------
void DRAW_SYS_UpdateSystemVBlank( DRAW_SYS_WORK* work )
{
  while( work->nowBuffer != work->topBuffer )
  {
    DRAW_SYS_PEN_INFO *drawBuf = DRAW_SYS_GetInfoBuf(work,work->nowBuffer);
    
    DRAW_SYS_DrawInfo( work , drawBuf );
    
    DRAW_SYS_IncBufCnt( work , &work->nowBuffer );
  }
}

//--------------------------------------------------------------
//	ペン情報から描画
//--------------------------------------------------------------
#include "draw_system_pen.cdat"
static void DRAW_SYS_DrawInfo( DRAW_SYS_WORK* work , const DRAW_SYS_PEN_INFO* info )
{
  void *vramAdr = DRAW_SYS_GetVramAdr( work );
  DRAW_SYS_DrawLine( work , vramAdr , info->startX , info->endX , info->startY , info->endY , info->penType , info->col );
}

static void DRAW_SYS_DrawLine( DRAW_SYS_WORK* work , void *vramAdr , const u16 x1 , const u16 x2 , const u16 y1 , const u16 y2 , const DRAW_SYS_PEN_SIZE penType , const u16 col )
{
  const DRAW_SYS_PEN_DATA *penData = &DRAW_SYS_PEN_ARR[penType];

  const int xLen = MATH_ABS( x1-x2 );
  const int yLen = MATH_ABS( y1-y2 );
  const int loopNum = ( xLen > yLen ? xLen : yLen )+1;
  fx32 addX = FX32_CONST(x2-x1) / loopNum;
  fx32 addY = FX32_CONST(y2-y1) / loopNum;
  fx32 subX = 0;
  fx32 subY = 0;
  
  const int baseX = x1 - (penData->sizeX/2);
  const int baseY = y1 - (penData->sizeY/2);
  
  u8 i;
  
  for( i=0;i<loopNum;i++ )
  {
    u8 x,y;
    const int posX = baseX + (subX>>FX32_SHIFT);
    const int posY = baseY + (subY>>FX32_SHIFT);
    for( x=0;x<penData->sizeX;x++ )
    {
      for( y=0;y<penData->sizeY;y++ )
      {
        if( penData->penData[y][x] == 1 )
        {
          DRAW_SYS_DrawDot_DirectBmp( work , vramAdr , posX+x , posY+y , col );
        }
      }
    }
    subX += addX;
    subY += addY;
  }
  
}

static void DRAW_SYS_DrawDot_DirectBmp( DRAW_SYS_WORK* work , void *vramAdr , const u16 x , const u16 y , const u16 col )
{
  if( x >= work->areaLeft &&
      x <  work->areaRight &&
      y >= work->areaTop &&
      y <  work->areaBottom )
  {
    u16 *trgAdr = (u16*)((u32)vramAdr + x*2 + y*256*2);
    *trgAdr = col;
  }
}

//--------------------------------------------------------------
//	ペン情報登録
//--------------------------------------------------------------
void DRAW_SYS_SetPenInfo( DRAW_SYS_WORK* work , const DRAW_SYS_PEN_INFO *info )
{
  DRAW_SYS_PEN_INFO *setPos = DRAW_SYS_GetInfoBuf(work,work->topBuffer);

  GFL_STD_MemCopy( info , setPos , sizeof(DRAW_SYS_PEN_INFO) );
  
  //OS_TPrintf("[%d][%d][%d][%d][%x]\n",setPos->startX,setPos->startY,setPos->endX,setPos->endX,setPos->col);
  OS_TPrintf("[%d][%d][%d][%d][%x]\n",info->startX,info->startY,info->endX,info->endX,info->col);
  DRAW_SYS_IncBufCnt( work , &work->topBuffer );

}

//--------------------------------------------------------------
//	VRAM取得
//--------------------------------------------------------------
static void* DRAW_SYS_GetVramAdr( DRAW_SYS_WORK* work )
{
  if( work->frame == GFL_BG_FRAME2_M )
  {
    return G2_GetBG2ScrPtr();
  }
  else
  {
    GF_ASSERT_MSG(0,"未対応！\n");
    return G2_GetBG2ScrPtr();
  }
}

//--------------------------------------------------------------
//	バッファ位置取得
//--------------------------------------------------------------
static DRAW_SYS_PEN_INFO* DRAW_SYS_GetInfoBuf( DRAW_SYS_WORK* work , const u16 pos )
{
  return (DRAW_SYS_PEN_INFO*)((u32)work->infoBuf + pos*sizeof(DRAW_SYS_PEN_INFO) );
}

//--------------------------------------------------------------
//	バッファ位置を進める
//--------------------------------------------------------------
static void DRAW_SYS_IncBufCnt( DRAW_SYS_WORK* work , u16 *pos )
{
  *pos += 1;
  if( *pos >= work->bufNum )
  {
    *pos = 0;
  }
  
}
