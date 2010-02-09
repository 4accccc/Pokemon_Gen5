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
//汎用お絵描きシステムを目指す。現状ダイレクトカラーBGのみ対応


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
typedef struct _DRAW_SYS_WORK DRAW_SYS_WORK;

typedef struct
{
  HEAPID heapId;
  
  u8 frame;
  u16 bufferNum;
  
  u16 areaLeft;
  u16 areaRight;
  u16 areaTop;
  u16 areaBottom;
}DRAW_SYS_INIT_WORK;


//１描画情報
//32bit転送してるのでサイズ注意
typedef struct
{
  u8 startX;
  u8 startY;
  u8 endX;
  u8 endY;
  u16 col;
  u8 penType;
  u8 pad;
}DRAW_SYS_PEN_INFO;

typedef enum
{
  DSPS_CIRCLE_1,
  DSPS_CIRCLE_4,
  DSPS_CIRCLE_8,

  DSPS_STAMP_HEART,
  DSPS_STAMP_EXUS,
  DSPS_STAMP_QUES,
  DSPS_STAMP_STAR,
  DSPS_STAMP_DROP,

  DSPS_MAX,
}DRAW_SYS_PEN_SIZE;
//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
extern DRAW_SYS_WORK* DRAW_SYS_CreateSystem( const DRAW_SYS_INIT_WORK *initWork );
extern void DRAW_SYS_DeleteSystem( DRAW_SYS_WORK* work );
extern void DRAW_SYS_UpdateSystem( DRAW_SYS_WORK* work );
extern void DRAW_SYS_UpdateSystemVBlank( DRAW_SYS_WORK* work );

extern void DRAW_SYS_SetPenInfo( DRAW_SYS_WORK* work , const DRAW_SYS_PEN_INFO *info );
extern void* DRAW_SYS_GetBufferAddress( DRAW_SYS_WORK* work );
extern u16 DRAW_SYS_GetBufferTopPos( DRAW_SYS_WORK* work );
extern void DRAW_SYS_SetRedrawBuffer( DRAW_SYS_WORK* work , u16 finishBufNo );
