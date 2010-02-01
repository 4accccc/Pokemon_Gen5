//============================================================================================
/**
 * @file	fld_faceup.c
 * @brief	顔アップ
 */
//============================================================================================
#include "arc_def.h"
#include "gamesystem/game_event.h"
#include "system/main.h"    //for HEAPID_FLD3DCUTIN 
#include "field_bg_def.h"
#include "field_place_name.h" //for   FIELD_PLACE_NAME
#include "field_debug.h"      //for FIELD_DEBUG～
#include "fld_faceup.h"
#include "fieldmap.h"

#include "arc/fieldmap/fld_faceup.naix"

#define BG_PLT_NO (10)
#define FACE_PLT_NO (0)
#define FACE_PLT_NUM  (2) 

#define SCR_SIZE_W  (32)
#define SCR_SIZE_H  (24)

typedef struct FACEUP_WORK_tag
{
  HEAPID HeapID;
  u8 BgPriority[4];
  GXPlaneMask Mask;
  GXBg23ControlText CntText;

  //背景BG
  //顔BG
}FACEUP_WORK;


static GMEVENT_RESULT SetupEvt( GMEVENT* event, int* seq, void* work );
static void Setup(FACEUP_WK_PTR ptr);
static GMEVENT_RESULT ReleaseEvt( GMEVENT* event, int* seq, void* work );
static void Release(FIELDMAP_WORK * fieldmap, FACEUP_WK_PTR ptr);

static void PushPriority(FACEUP_WK_PTR ptr);
static void PushDisp(FACEUP_WK_PTR ptr);
static void PopPriority(FACEUP_WK_PTR ptr);
static void PopDisp(FACEUP_WK_PTR ptr);

static void ChangeScreenPlt( void *rawData, u8 px, u8 py, u8 sx, u8 sy, u8 pal );

GMEVENT *FLD_FACEUP_Start(const int inBackNo, const int inCharNo, GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  HEAPID heapID;
  FACEUP_WK_PTR ptr;

  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  heapID = HEAPID_FLD3DCUTIN;
  //ワークをアロケーションして、フェイスアップデータポインタにセット
  ptr = GFL_HEAP_AllocClearMemory(heapID, sizeof(FACEUP_WORK));   //カットインヒープからアロケート

  if ( *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) != NULL )
  {
    GF_ASSERT(0);
    return NULL;
  }

  *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) = ptr;
  ptr->HeapID = heapID;

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, SetupEvt, 0 );
  return event;
}

//--------------------------------------------------------------
/**
 * セットアップイベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT SetupEvt( GMEVENT* event, int* seq, void* work )
{
  FACEUP_WK_PTR ptr;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);

  switch(*seq){
  case 0:
    //ブラックアウト開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, -3 );
    (*seq)++;
    break;
  case 1:
    //ブラックアウト待ち
    if ( GFL_FADE_CheckFade() ) break;
    //セットアップ
    Setup(ptr);
    //ブラックイン開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, -3 );
    (*seq)++;
    break;
  case 2:
    if ( GFL_FADE_CheckFade() == FALSE ){
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * セットアップ
 * @param     ptr	            ワークポインタ
 * @return    none
 */
//--------------------------------------------------------------
static void Setup(FACEUP_WK_PTR ptr)
{
  //プライオリティ保存
  PushPriority(ptr);
  //表示状態の保存
  PushDisp(ptr);
  //BG設定保存
  ptr->CntText = G2_GetBG3ControlText();
  //ＢＧ3セットアップ（顔BG256色）
  G2_SetBG3ControlText(
      GX_BG_SCRSIZE_TEXT_256x256,
      GX_BG_COLORMODE_256,
      GX_BG_SCRBASE_0x1000,
      GX_BG_CHARBASE_0x04000);

  G2_BlendNone();
  //タイプごとの背景、顔BGをロード
  {
    void *buf;
    //背景
    //キャラ
    {
      NNSG2dCharacterData *chr;
		  buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_test_ncgr, ptr->HeapID );
		  GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadCharacter( FLDBG_MFRM_EFF1, chr->pRawData, chr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //スクリーン
    {
      NNSG2dScreenData *scr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_test_nscr, ptr->HeapID );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedScreenData(buf,&scr) == FALSE ){
        GF_ASSERT( 0 );
      }
      ChangeScreenPlt( scr->rawData, 0, 0, SCR_SIZE_W, SCR_SIZE_H, BG_PLT_NO );
      GFL_BG_LoadScreen( FLDBG_MFRM_EFF1, scr->rawData, scr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //パレット
    {
      NNSG2dPaletteData *pal;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_test_nclr, ptr->HeapID );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadPalette( FLDBG_MFRM_EFF1, pal->pRawData, 32, BG_PLT_NO*32 );
      GFL_HEAP_FreeMemory( buf );
    }

    //顔
    //キャラ
    {
      NNSG2dCharacterData *chr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_face_test_ncgr, ptr->HeapID );
		  GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadCharacter( FLDBG_MFRM_EFF2, chr->pRawData, chr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //スクリーン
    {
      NNSG2dScreenData *scr;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_face_test_nscr, ptr->HeapID );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedScreenData(buf,&scr) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadScreen( FLDBG_MFRM_EFF2, scr->rawData, scr->szByte, 0 );
      GFL_HEAP_FreeMemory( buf );
    }
    //パレット
    {
      NNSG2dPaletteData *pal;
      buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_face_test_nclr, ptr->HeapID );
      GF_ASSERT( buf != NULL );
      if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
        GF_ASSERT( 0 );
      }
      GFL_BG_LoadPalette( FLDBG_MFRM_EFF2, pal->pRawData, FACE_PLT_NUM*32, FACE_PLT_NO*32 );
      GFL_HEAP_FreeMemory( buf );
    }
  }

  //表示
  GFL_BG_SetVisible( GFL_BG_FRAME2_M, VISIBLE_ON );
  GFL_BG_SetVisible( GFL_BG_FRAME3_M, VISIBLE_ON );
}

GMEVENT *FLD_FACEUP_End(GAMESYS_WORK *gsys)
{
  GMEVENT * event;
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);
  if ( *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) == NULL )
  {
    GF_ASSERT(0);
    return NULL;
  }

  //イベント作成
  event = GMEVENT_Create( gsys, NULL, ReleaseEvt, 0 );
  return event;
}

void FLD_FACEUP_Release( FIELDMAP_WORK *fieldmap )
{
  FACEUP_WK_PTR ptr;
  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);
  Release(fieldmap, ptr);
}

//--------------------------------------------------------------
/**
 * リリースイベント
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static GMEVENT_RESULT ReleaseEvt( GMEVENT* event, int* seq, void* work )
{
  FACEUP_WK_PTR ptr;
  GAMESYS_WORK *gsys = GMEVENT_GetGameSysWork(event);
  FIELDMAP_WORK *fieldmap = GAMESYSTEM_GetFieldMapWork(gsys);

  ptr = *FIELDMAP_GetFaceupWkPtrAdr(fieldmap);

  switch(*seq){
  case 0:
    //ブラックアウト開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 0, 16, -3 );
    (*seq)++;
    break;
  case 1:
    //ブラックアウト待ち
    if ( GFL_FADE_CheckFade() ) break;
    //リリース
    Release(fieldmap, ptr);
    //ブラックイン開始
    GFL_FADE_SetMasterBrightReq(
          GFL_FADE_MASTER_BRIGHT_BLACKOUT_MAIN, 16, 0, -3 );
    (*seq)++;
    break;
  case 2:
    if ( GFL_FADE_CheckFade() == FALSE ){
      return GMEVENT_RES_FINISH;
    }
  }
  return GMEVENT_RES_CONTINUE;
}

//--------------------------------------------------------------
/**
 * リリース
 * @param     event	            イベントポインタ
 * @param     seq               シーケンサ
 * @param     work              ワークポインタ
 * @return    GMEVENT_RESULT   イベント結果
 */
//--------------------------------------------------------------
static void Release(FIELDMAP_WORK * fieldmap, FACEUP_WK_PTR ptr)
{
  FIELD_PLACE_NAME *place_name_sys = FIELDMAP_GetPlaceNameSys( fieldmap );
  FIELD_DEBUG_WORK *debug = FIELDMAP_GetDebugWork( fieldmap );

  GFL_BG_ClearFrame( GFL_BG_FRAME2_M );

  //BG設定復帰
  G2_SetBG3ControlText(
      ptr->CntText.screenSize,
      ptr->CntText.colorMode,
      ptr->CntText.screenBase,
      ptr->CntText.charBase);
  //復帰
  FIELD_PLACE_NAME_RecoverBG( place_name_sys );
  FIELD_DEBUG_RecoverBgCont( debug );

  //プライオリティ復帰
  PopPriority(ptr);
  //表示状態復帰
  PopDisp(ptr);
  //ワーク解放
  GFL_HEAP_FreeMemory( ptr );
  *FIELDMAP_GetFaceupWkPtrAdr(fieldmap) = NULL;
}

//キャラ変更
void FLD_FACEUP_Change(void)
{
  void *buf;
  HEAPID heapID = HEAPID_FLD3DCUTIN;
  //キャラ
  {
    NNSG2dCharacterData *chr;
    buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_face2_test_ncgr, heapID );
		GF_ASSERT( buf != NULL );
    if( NNS_G2dGetUnpackedBGCharacterData(buf,&chr) == FALSE ){
      GF_ASSERT( 0 );
    }
    GFL_BG_LoadCharacter( FLDBG_MFRM_EFF2, chr->pRawData, chr->szByte, 0 );
    GFL_HEAP_FreeMemory( buf );
  }
  
  //パレット
  {
    NNSG2dPaletteData *pal;
    buf = GFL_ARC_LoadDataAlloc( ARCID_FLD_FACEUP, NARC_fld_faceup_face2_test_nclr, heapID );
    GF_ASSERT( buf != NULL );
    if( NNS_G2dGetUnpackedPaletteData(buf,&pal) == FALSE ){
      GF_ASSERT( 0 );
    }
    GFL_BG_LoadPalette( FLDBG_MFRM_EFF2, pal->pRawData, FACE_PLT_NUM*32, FACE_PLT_NO*32 );
    GFL_HEAP_FreeMemory( buf );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティプッシュ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushPriority(FACEUP_WK_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    ptr->BgPriority[i] = GFL_BG_GetPriority( i );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態プッシュ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PushDisp(FACEUP_WK_PTR ptr)
{
  ptr->Mask = GFL_DISP_GetMainVisible();
}

//----------------------------------------------------------------------------
/**
 *	@brief	プライオリティポップ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopPriority(FACEUP_WK_PTR ptr)
{
  int i;
  for(i=0;i<4;i++){
    GFL_BG_SetPriority( i, ptr->BgPriority[i] );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief	表示状態ポップ
 *
 *	@param	ptr       ワークポインタ
 *	@return none
 */
//-----------------------------------------------------------------------------
static void PopDisp(FACEUP_WK_PTR ptr)
{
  GFL_DISP_GX_SetVisibleControlDirect(ptr->Mask);
}



static void ChangeScreenPlt( void *rawData, u8 px, u8 py, u8 sx, u8 sy, u8 pal )
{
  u16 * scrn;
  u8  scr_sx, scr_sy;
  u8  i, j;

  scrn = (u16 *)rawData;

  scr_sx = 32;
  scr_sy = 32;

  for( i=py; i<py+sy; i++ ){
    if( i >= scr_sy ){ break; }
    for( j=px; j<px+sx; j++ ){
      if( j >= scr_sx ){ break; }
      {
        u16 scr_pos = i * 32 + j;

        scrn[scr_pos] = ( scrn[scr_pos] & 0xfff ) | ( pal << 12 );
      }
    }
  }
}
