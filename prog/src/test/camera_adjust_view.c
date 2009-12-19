//============================================================================================
/**
 * @file  camera_adjust_view.c
 * @brief カメラ調整用ビューワー
 * @author
 * @date
 */
//============================================================================================
#include <wchar.h>
#include "gflib.h"
#include "arc_def.h"

#include "font/font.naix"
#include "print/printsys.h"
#include "print/str_tool.h"

#include "test/camera_adjust_view.h"
//============================================================================================
/**
 *
 *
 *
 * @brief カメラ調整用ビューワー
 *
 *
 *
 */
//============================================================================================
// グラフィックアーカイブ内容ＩＤ定義
enum {
  NARC_ncgr = 0,    // NARC_camera_adjust_view_camera_cont_NCGR = 0,
  NARC_nclr = 1,    // NARC_camera_adjust_view_camera_cont_NCLR = 1,
  NARC_nscr = 2,    // NARC_camera_adjust_view_camera_cont_NSCR = 2
};

static const char arc_path[] = {"camera_adjust_view.narc"};

//============================================================================================
enum {
  BMP_ANGLEV = 0,
  BMP_ANGLEH,
  BMP_LENGTH,
  BMP_FOVY,
  BMP_FAR,
  BMP_WIPE,

  BMP_MAX,
};

struct _GFL_CAMADJUST {
  HEAPID          heapID;
  int           seq;
  GFL_CAMADJUST_SETUP   setup;
  GFL_DISPUT_VRAMSV*    vramSv;

  PRINT_QUE*        printQue;
  GFL_FONT*       fontHandle;

  u16*          pAngleV;
  u16*          pAngleH;
  fx32*         pLength;
  u16*          pFovy;
  fx32*         pFar;

  BOOL          wipeSw;
  fx32*         pWipeSize;

  GFL_BMP_DATA*     bmp[BMP_MAX];
  STRBUF*         strBufTmp;

  u16           scrnBuf[32*32];

  s32     touch_bttn;
  s32     touch_count;
};

#define CHR_SIZ     (0x20)
#define PLT_SIZ     (0x20)
#define PUSH_CGX_SIZ  (0x2000)
#define PUSH_SCR_SIZ  (0x800)

static void loadGraphicData( GFL_CAMADJUST* gflCamAdjust );

static BOOL CAMADJUST_ControlTrg( GFL_CAMADJUST* gflCamAdjust );
static BOOL CAMADJUST_ControlCont( GFL_CAMADJUST* gflCamAdjust );
static void CAMADJUST_NumPrint( GFL_CAMADJUST* gflCamAdjust, int bmpIdx, u16 num, u32 vramChr );
static void CAMADJUST_printWipeSw( GFL_CAMADJUST* gflCamAdjust );


#define TOUCH_REPEAT_WAIT_TYPE ( 2 )
static const u8 sc_TOUCH_REPEAT_WAIT[ TOUCH_REPEAT_WAIT_TYPE ] =
{
  8,
  0
};
static const s32 sc_TOUCH_REPEAT_WAIT_TYPE_COUNT[ TOUCH_REPEAT_WAIT_TYPE ] =
{
  32,
  90,
};
static void CAMADJUST_TOUCH_InitRepeat( GFL_CAMADJUST* gflCamAdjust );
static void CAMADJUST_TOUCH_Control( GFL_CAMADJUST* gflCamAdjust, int now_button );
static int CAMADJUST_TOUCH_GetRepeat( const GFL_CAMADJUST* gflCamAdjust );

//============================================================================================
/**
 *
 * @brief システム起動 & 終了
 *
 */
//============================================================================================
extern GFL_CAMADJUST* GFL_CAMADJUST_Create( const GFL_CAMADJUST_SETUP* setup, HEAPID heapID )
{
  GFL_CAMADJUST* gflCamAdjust;
  int i;

  gflCamAdjust = GFL_HEAP_AllocClearMemory( GetHeapLowID(heapID), sizeof(GFL_CAMADJUST) );

  gflCamAdjust->heapID = heapID;
  gflCamAdjust->setup = *setup;

  gflCamAdjust->seq = 0;
  {
    u16 bgPalMask = 0x0001 << gflCamAdjust->setup.bgPalID;
    gflCamAdjust->vramSv =  GFL_DISPUT_CreateVramSv(gflCamAdjust->setup.bgID,
                            PUSH_CGX_SIZ, PUSH_SCR_SIZ,
                            bgPalMask, heapID );
  }
  gflCamAdjust->pAngleV = NULL;
  gflCamAdjust->pAngleH = NULL;
  gflCamAdjust->pLength = NULL;
  gflCamAdjust->pFovy   = NULL;
  gflCamAdjust->pFar    = NULL;

  gflCamAdjust->wipeSw = FALSE;
  gflCamAdjust->pWipeSize = NULL;

  for( i=0; i<BMP_MAX; i++ ){
    gflCamAdjust->bmp[i] = GFL_BMP_Create( 4, 1, GFL_BMP_16_COLOR, gflCamAdjust->heapID );
  }
  gflCamAdjust->strBufTmp = GFL_STR_CreateBuffer(8, gflCamAdjust->heapID);

  gflCamAdjust->fontHandle = GFL_FONT_Create
    (ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE ,gflCamAdjust->heapID);
  gflCamAdjust->printQue = PRINTSYS_QUE_Create(gflCamAdjust->heapID);

  // 初期化
  CAMADJUST_TOUCH_InitRepeat( gflCamAdjust );

  return gflCamAdjust;
}

void  GFL_CAMADJUST_Delete( GFL_CAMADJUST* gflCamAdjust )
{
  int i;

  if( gflCamAdjust == NULL ) return;

  PRINTSYS_QUE_Clear( gflCamAdjust->printQue );
  PRINTSYS_QUE_Delete( gflCamAdjust->printQue );
  GFL_FONT_Delete( gflCamAdjust->fontHandle );

  GFL_STR_DeleteBuffer( gflCamAdjust->strBufTmp );
  for( i=0; i<BMP_MAX; i++ ){ GFL_BMP_Delete( gflCamAdjust->bmp[i] ); }

  GFL_DISPUT_DeleteVramSv( gflCamAdjust->vramSv );
  GFL_HEAP_FreeMemory( gflCamAdjust );
}

void  GFL_CAMADJUST_SetCameraParam( GFL_CAMADJUST* gflCamAdjust,
                                    u16* pAngleV, u16* pAngleH,
                                    fx32* pLength, u16* pFovy, fx32* pFar )
{
  if( gflCamAdjust == NULL ) return;

  gflCamAdjust->pAngleV = pAngleV;
  gflCamAdjust->pAngleH = pAngleH;
  gflCamAdjust->pLength = pLength;
  gflCamAdjust->pFovy   = pFovy;
  gflCamAdjust->pFar    = pFar;
}

void  GFL_CAMADJUST_SetWipeParam( GFL_CAMADJUST* gflCamAdjust, fx32* pWipeSize )
{
  if( gflCamAdjust == NULL ) return;

  gflCamAdjust->wipeSw = FALSE;
  gflCamAdjust->pWipeSize = pWipeSize;

  *(gflCamAdjust->pWipeSize) = 0;
}

//============================================================================================
/**
 *
 * @brief システムメイン
 *
 */
//============================================================================================
enum {
  SEQ_PUSHVRAM = 0,
  SEQ_MAKEDISP,
  SEQ_DISPON,
  SEQ_MAIN,
  SEQ_POPVRAM,
  SEQ_END,
};

BOOL  GFL_CAMADJUST_Main( GFL_CAMADJUST* gflCamAdjust )
{
  if( gflCamAdjust == NULL ){ return FALSE; }

  switch( gflCamAdjust->seq ){

  case SEQ_PUSHVRAM:
		if(gflCamAdjust->setup.bgID < GFL_DISPUT_BGID_S0){
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_M0 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_M1 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_M2 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_M3 );
		} else {
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_S0 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_S1 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_S2 );
			GFL_DISPUT_VisibleOff( GFL_DISPUT_BGID_S3 );
		}

    GFL_DISPUT_PushVram( gflCamAdjust->vramSv );

    gflCamAdjust->seq = SEQ_MAKEDISP;
    break;

  case SEQ_MAKEDISP:
    loadGraphicData( gflCamAdjust );
    GFL_DISPUT_SetAlpha( gflCamAdjust->setup.bgID, 31, 6 );

    gflCamAdjust->seq = SEQ_DISPON;
    break;

  case SEQ_DISPON:
    GFL_DISPUT_VisibleOn( gflCamAdjust->setup.bgID );
    gflCamAdjust->seq = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    if( gflCamAdjust->setup.cancelKey ){
      if( GFL_UI_KEY_GetTrg() == gflCamAdjust->setup.cancelKey ){
        gflCamAdjust->seq = SEQ_POPVRAM;
        break;
      }
    }
    if( CAMADJUST_ControlTrg( gflCamAdjust ) == FALSE ){
      gflCamAdjust->seq = SEQ_POPVRAM;
      break;
    }
    if( CAMADJUST_ControlCont( gflCamAdjust ) == FALSE ){
      gflCamAdjust->seq = SEQ_POPVRAM;
      break;
    }
    CAMADJUST_NumPrint( gflCamAdjust, BMP_ANGLEV, *gflCamAdjust->pAngleV, 0x54 );
    CAMADJUST_NumPrint( gflCamAdjust, BMP_ANGLEH, *gflCamAdjust->pAngleH, 0x64 );
    CAMADJUST_NumPrint( gflCamAdjust, BMP_LENGTH, *gflCamAdjust->pLength/FX32_ONE, 0x74 );
    CAMADJUST_NumPrint( gflCamAdjust, BMP_FOVY, *gflCamAdjust->pFovy, 0x84 );
    CAMADJUST_NumPrint( gflCamAdjust, BMP_FAR, *gflCamAdjust->pFar/FX32_ONE, 0x94 );
    if(gflCamAdjust->pWipeSize != NULL){
      CAMADJUST_NumPrint( gflCamAdjust, BMP_WIPE, *gflCamAdjust->pWipeSize, 0xbc );
    }

    CAMADJUST_printWipeSw( gflCamAdjust );
    GFL_DISPUT_LoadScr(gflCamAdjust->setup.bgID, gflCamAdjust->scrnBuf, 0, PUSH_SCR_SIZ);
    break;

  case SEQ_POPVRAM:
    GFL_DISPUT_VisibleOff( gflCamAdjust->setup.bgID );

    GFL_DISPUT_PopVram( gflCamAdjust->vramSv );

    gflCamAdjust->seq = SEQ_END;
    break;

  case SEQ_END:
    GFL_DISPUT_VisibleOn( gflCamAdjust->setup.bgID );
    return FALSE;
  }
  return TRUE;
}

//============================================================================================
/**
 *
 * @brief 描画関数
 *
 */
//============================================================================================
static void resetScrAttr( u16* scr, GFL_DISPUT_PALID palID )
{
  int i;
  u16 palMask = palID <<12;

  for( i=0; i<32*32; i++ ){
    scr[i] &= 0x0fff;
    scr[i] |= palMask;
  }
}

//============================================================================================
static void loadGraphicData( GFL_CAMADJUST* gflCamAdjust )
{
  void* binCgx = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_ncgr, gflCamAdjust->heapID );
  void* binScr = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_nscr, gflCamAdjust->heapID );
  void* binPlt = GFL_ARC_LoadDataAlloc( ARCID_CAMERACONT, NARC_nclr, gflCamAdjust->heapID );
  void* cgx = GFL_DISPUT_GetCgxDataNNSbin( binCgx );
  void* scr = GFL_DISPUT_GetScrDataNNSbin( binScr );
  void* plt = GFL_DISPUT_GetPltDataNNSbin( binPlt );

  GFL_STD_MemCopy32( scr, (void*)gflCamAdjust->scrnBuf, PUSH_SCR_SIZ );
  resetScrAttr( gflCamAdjust->scrnBuf, gflCamAdjust->setup.bgPalID );

  GFL_DISPUT_LoadCgx( gflCamAdjust->setup.bgID, cgx, 0, PUSH_CGX_SIZ );
  GFL_DISPUT_LoadScr( gflCamAdjust->setup.bgID, gflCamAdjust->scrnBuf, 0, PUSH_SCR_SIZ );
  GFL_DISPUT_LoadPlt( gflCamAdjust->setup.bgID, plt, PLT_SIZ * gflCamAdjust->setup.bgPalID );

  GFL_HEAP_FreeMemory( binPlt );
  GFL_HEAP_FreeMemory( binScr );
  GFL_HEAP_FreeMemory( binCgx );
}

//============================================================================================
/**
 *
 * @brief 情報の取得と表示
 *
 */
//============================================================================================




//============================================================================================
/**
 *
 * @brief タッチパネルイベント判定
 *
 */
//============================================================================================
#define BTN_SX  (32-1)
#define BTN_SY  (32-1)

#define EXIT_PX     (232)
#define EXIT_PY     (0)
//------------------------------------------------------------------
enum {
  TPBTN_ANGLE_U = 0,
  TPBTN_ANGLE_D,
  TPBTN_ANGLE_L,
  TPBTN_ANGLE_R,
  TPBTN_LENDOWN,
  TPBTN_LENUP,
  TPBTN_FOVYDOWN,
  TPBTN_FOVYUP,
  TPBTN_FARUP,
  TPBTN_FARDOWN,

  TPBTN_WIPEDOWN,
  TPBTN_WIPEUP,

  //TP_EXIT,

  TPBTN_CONT_MAX,
};

static const GFL_UI_TP_HITTBL eventContTouchPanelTable[TPBTN_CONT_MAX + 1] = {
  { 0x01*8, 0x01*8+BTN_SY, 0x0e*8, 0x0e*8+BTN_SX }, //TPBTN_ANGLE_U
  { 0x09*8, 0x09*8+BTN_SY, 0x0e*8, 0x0e*8+BTN_SX }, //TPBTN_ANGLE_D
  { 0x05*8, 0x05*8+BTN_SY, 0x0a*8, 0x0a*8+BTN_SX }, //TPBTN_ANGLE_L
  { 0x05*8, 0x05*8+BTN_SY, 0x12*8, 0x12*8+BTN_SX }, //TPBTN_ANGLE_R

  { 0x03*8, 0x03*8+BTN_SY, 0x1b*8, 0x1b*8+BTN_SX }, //TPBTN_LENDOWN
  { 0x07*8, 0x07*8+BTN_SY, 0x1b*8, 0x1b*8+BTN_SX }, //TPBTN_LENUP
  { 0x03*8, 0x03*8+BTN_SY, 0x17*8, 0x17*8+BTN_SX }, //TPBTN_FOVYDOWN
  { 0x07*8, 0x07*8+BTN_SY, 0x17*8, 0x17*8+BTN_SX }, //TPBTN_FOVYUP
  { 0x03*8, 0x03*8+BTN_SY, 0x05*8, 0x05*8+BTN_SX }, //TPBTN_FARUP
  { 0x07*8, 0x07*8+BTN_SY, 0x05*8, 0x05*8+BTN_SX }, //TPBTN_FARDOWN

  { 0x0e*8, 0x0e*8+BTN_SY, 0x10*8, 0x10*8+BTN_SX }, //TPBTN_WIPEDOWN
  { 0x12*8, 0x12*8+BTN_SY, 0x10*8, 0x10*8+BTN_SX }, //TPBTN_WIPEUP

  //{ EXIT_PY, EXIT_PY+7, EXIT_PX, EXIT_PX+23 },

  {GFL_UI_TP_HIT_END,0,0,0},      //終了データ
};

//------------------------------------------------------------------
#define ANGLE_ROTATE_SPD      (0x80)
#define CAMERA_TARGET_HEIGHT  (4 * FX32_ONE)
#define CAMLEN_MVSPD          (8 * FX32_ONE)
#define CAMANGLEH_MAX         (0x4000 - ANGLE_ROTATE_SPD)
#define CAMANGLEH_MIN         (-0x4000 + ANGLE_ROTATE_SPD)
#define CAMLEN_MAX            (4096 * FX32_ONE)
#define CAMLEN_MIN            (16 * FX32_ONE)
#define CAMANGFOVY_MAX        (0x4000 - ANGLE_ROTATE_SPD)
#define CAMANGFOVY_MIN        (0x0 + ANGLE_ROTATE_SPD)
#define WIPE_SPD              (FX32_ONE/32)
#define WIPE_MAX              (16 * FX32_ONE)
#define WIPE_MIN              (FX32_ONE)

static BOOL CAMADJUST_ControlCont( GFL_CAMADJUST* gflCamAdjust )
{
  int tblPos = GFL_UI_TP_HitCont(eventContTouchPanelTable);

  if(gflCamAdjust->pAngleV == NULL){ return TRUE; }
  if(gflCamAdjust->pAngleH == NULL){ return TRUE; }
  if(gflCamAdjust->pLength == NULL){ return TRUE; }
  if(gflCamAdjust->pFar == NULL){ return TRUE; }
  if(gflCamAdjust->pFovy == NULL){ return TRUE; }

  CAMADJUST_TOUCH_Control( gflCamAdjust, tblPos );
  tblPos = CAMADJUST_TOUCH_GetRepeat(gflCamAdjust);

  if(tblPos == GFL_UI_TP_HIT_NONE){
    return TRUE;
  }
  switch(tblPos){

  case TPBTN_ANGLE_U:
      *(gflCamAdjust->pAngleH) += ANGLE_ROTATE_SPD;
      {
        s16 value = (s16)(*gflCamAdjust->pAngleH);
        if( value > CAMANGLEH_MAX ){ *gflCamAdjust->pAngleH = CAMANGLEH_MAX; }
      }
      break;
  case TPBTN_ANGLE_D:
      *(gflCamAdjust->pAngleH) -= ANGLE_ROTATE_SPD;
      {
        s16 value = (s16)(*gflCamAdjust->pAngleH);
        if( value < CAMANGLEH_MIN ){ *gflCamAdjust->pAngleH = CAMANGLEH_MIN; }
      }
      break;
  case TPBTN_ANGLE_L:
      *(gflCamAdjust->pAngleV) += ANGLE_ROTATE_SPD;
      break;
  case TPBTN_ANGLE_R:
      *(gflCamAdjust->pAngleV) -= ANGLE_ROTATE_SPD;
      break;
  case TPBTN_LENDOWN:
      *(gflCamAdjust->pLength) -= CAMLEN_MVSPD;
      if( *gflCamAdjust->pLength < CAMLEN_MIN ){ *(gflCamAdjust->pLength) = CAMLEN_MIN; }
      break;
  case TPBTN_LENUP:
      *(gflCamAdjust->pLength) += CAMLEN_MVSPD;
      if( *gflCamAdjust->pLength > CAMLEN_MAX ){ *(gflCamAdjust->pLength) = CAMLEN_MAX; }
      break;
  case TPBTN_FOVYUP:
      *(gflCamAdjust->pFovy) += ANGLE_ROTATE_SPD;
      {
        s16 value = (s16)(*gflCamAdjust->pFovy);
        if( value > CAMANGFOVY_MAX ){ *gflCamAdjust->pFovy = CAMANGFOVY_MAX; }
      }
      break;
  case TPBTN_FOVYDOWN:
      *(gflCamAdjust->pFovy) -= ANGLE_ROTATE_SPD;
      {
        s16 value = (s16)(*gflCamAdjust->pFovy);
        if( value < CAMANGFOVY_MIN ){ *gflCamAdjust->pFovy = CAMANGFOVY_MIN; }
      }
      break;
  case TPBTN_FARUP:
      *(gflCamAdjust->pFar) += CAMLEN_MVSPD;
      if( *gflCamAdjust->pFar > CAMLEN_MAX ){ *(gflCamAdjust->pFar) = CAMLEN_MAX; }
      break;
  case TPBTN_FARDOWN:
      *(gflCamAdjust->pFar) -= CAMLEN_MVSPD;
      if( *gflCamAdjust->pFar < CAMLEN_MIN ){ *(gflCamAdjust->pFar) = CAMLEN_MIN; }
      break;

  case TPBTN_WIPEDOWN:
      if( gflCamAdjust->pWipeSize != NULL ){
        *(gflCamAdjust->pWipeSize) -= WIPE_SPD;
        if( *gflCamAdjust->pWipeSize < WIPE_MIN ){ *(gflCamAdjust->pWipeSize) = WIPE_MIN; }
      }
      break;
  case TPBTN_WIPEUP:
      if( gflCamAdjust->pWipeSize != NULL ){
        *(gflCamAdjust->pWipeSize) += WIPE_SPD;
        if( *gflCamAdjust->pWipeSize > WIPE_MAX ){ *(gflCamAdjust->pWipeSize) = WIPE_MAX; }
      }
      break;
  }
  return TRUE;
}

//============================================================================================
enum {
  TPBTN_WIPESW = 0,

  //TP_EXIT,

  TPBTN_TRG_MAX,
};

static const GFL_UI_TP_HITTBL eventTrgTouchPanelTable[TPBTN_TRG_MAX + 1] = {
  { 0x10*8, 0x10*8+(16-1), 0x0d*8, 0x0d*8+(16-1) }, //TPBTN_WIPESW

  //{ EXIT_PY, EXIT_PY+7, EXIT_PX, EXIT_PX+23 },

  {GFL_UI_TP_HIT_END,0,0,0},      //終了データ
};

//------------------------------------------------------------------
static BOOL CAMADJUST_ControlTrg( GFL_CAMADJUST* gflCamAdjust )
{
  int tblPos = GFL_UI_TP_HitTrg(eventTrgTouchPanelTable);

  if(tblPos == GFL_UI_TP_HIT_NONE){
    return TRUE;
  }
  switch(tblPos){

  case TPBTN_WIPESW:
      if( gflCamAdjust->pWipeSize != NULL ){
        if(gflCamAdjust->wipeSw == FALSE){
          gflCamAdjust->wipeSw = TRUE;
          *(gflCamAdjust->pWipeSize) = FX32_ONE;
        } else {
          gflCamAdjust->wipeSw = FALSE;
          *(gflCamAdjust->pWipeSize) = 0;
        }
      } else {
        gflCamAdjust->wipeSw = FALSE;
      }
      break;
  }
  return TRUE;
}

//============================================================================================
/**
 *
 * @brief 数値表示
 *
 */
//============================================================================================
static const u16 numStrData[16] =
{ L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9',L'A',L'B',L'C',L'D',L'E',L'F' };

static void CAMADJUST_NumPrint( GFL_CAMADJUST* gflCamAdjust, int bmpIdx, u16 num, u32 vramChr )
{
  u16   numStr[5];
  int   i;

  numStr[0] = numStrData[(num & 0xf000) >> 12];
  numStr[1] = numStrData[(num & 0x0f00) >> 8];
  numStr[2] = numStrData[(num & 0x00f0) >> 4];
  numStr[3] = numStrData[(num & 0x000f) >> 0];
  numStr[4] = GFL_STR_GetEOMCode();
  GFL_STR_SetStringCode( gflCamAdjust->strBufTmp, numStr );

  GFL_BMP_Clear( gflCamAdjust->bmp[bmpIdx], 15 );

  PRINTSYS_PrintQueColor( gflCamAdjust->printQue,
              gflCamAdjust->bmp[bmpIdx],
              0, 0,
              gflCamAdjust->strBufTmp,
              gflCamAdjust->fontHandle,
              PRINTSYS_LSB_Make(1,15,15));

  GFL_DISPUT_LoadCgx(   gflCamAdjust->setup.bgID,
              GFL_BMP_GetCharacterAdrs( gflCamAdjust->bmp[bmpIdx] ),
              CHR_SIZ * vramChr,
              GFL_BMP_GetBmpDataSize( gflCamAdjust->bmp[bmpIdx] ));
}

//============================================================================================
/**
 *
 * @brief Wipeスイッチ
 *
 */
//============================================================================================
static void CAMADJUST_printWipeSw( GFL_CAMADJUST* gflCamAdjust )
{
  u16 palMask = (gflCamAdjust->setup.bgPalID <<12);
  u16 scrData = (gflCamAdjust->wipeSw == TRUE)? 0x0c|palMask : 0x08|palMask;

  gflCamAdjust->scrnBuf[0x10 * 32 + 0x0d] = scrData + 0;
  gflCamAdjust->scrnBuf[0x10 * 32 + 0x0e] = scrData + 1;
  gflCamAdjust->scrnBuf[0x11 * 32 + 0x0d] = scrData + 2;
  gflCamAdjust->scrnBuf[0x11 * 32 + 0x0e] = scrData + 3;
}


//----------------------------------------------------------------------------
/**
 *  @brief  リピートワークのクリア
 */
//-----------------------------------------------------------------------------
static void CAMADJUST_TOUCH_InitRepeat( GFL_CAMADJUST* gflCamAdjust )
{
  gflCamAdjust->touch_bttn    = GFL_UI_TP_HIT_NONE;
  gflCamAdjust->touch_count   = 0;
}

//----------------------------------------------------------------------------
/**
 *  @brief  リピートコントロール
 */
//-----------------------------------------------------------------------------
static void CAMADJUST_TOUCH_Control( GFL_CAMADJUST* gflCamAdjust, int now_button )
{
  if(gflCamAdjust->touch_bttn != now_button)
  {
    gflCamAdjust->touch_bttn    = now_button;
    gflCamAdjust->touch_count   = 0;
  }
  else
  {
    if( (gflCamAdjust->touch_count + 1) <= sc_TOUCH_REPEAT_WAIT_TYPE_COUNT[ TOUCH_REPEAT_WAIT_TYPE-1 ] )
    {
      gflCamAdjust->touch_count ++;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  リピート取得
 *
 *  @param  gflCamAdjust
 */
//-----------------------------------------------------------------------------
static int CAMADJUST_TOUCH_GetRepeat( const GFL_CAMADJUST* gflCamAdjust )
{
  int i;
  for( i=0; i<TOUCH_REPEAT_WAIT_TYPE; i++ )
  {
    if( sc_TOUCH_REPEAT_WAIT_TYPE_COUNT[i] >= gflCamAdjust->touch_count )
    {
      if( (gflCamAdjust->touch_count % sc_TOUCH_REPEAT_WAIT[i]) == 0 )
      {
        return gflCamAdjust->touch_bttn;
      }
      break;
    }
  }
  return GFL_UI_TP_HIT_NONE;
}



