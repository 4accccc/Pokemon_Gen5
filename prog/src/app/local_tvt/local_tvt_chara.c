//======================================================================
/**
 * @file	local_tvt_chara.c
 * @brief	非通信テレビトランシーバー キャラ
 * @author	ariizumi
 * @data	09/11/02
 *
 * モジュール名：LOCAL_TVT_CHARA
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "local_tvt.naix"

#include "local_tvt_local_def.h"
#include "local_tvt_chara.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define LTVT_CHARA_UPDATE_CNT (40)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  LTCTS_WAIT_REQ,
  LTCTS_CHARA1,
  LTCTS_CHARA2,
  LTCTS_BG1,
  LTCTS_BG2,
  LTCTS_WAIT_TRANS,
  
}LOCAL_TVT_CHARA_TRANS_STATE;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _LOCAL_TVT_CHARA
{
  u8 charaIdx;
  u8 transIdx;
  
  u8 charaType;
  u8 bgType;
  u8 charaAnmIdx;
  u8 bgAnmIdx;
  u8 anmCnt;
  
  LOCAL_TVT_CHARA_TRANS_STATE state;

  NNSG2dCharacterData *charaData;
  void *resData;

};

static const u8 charaResList[][2] = {
    { NARC_local_tvt_chara_01_NCLR , NARC_local_tvt_chara_01_01_NCGR },
    { NARC_local_tvt_chara_02_NCLR , NARC_local_tvt_chara_02_01_NCGR },
    { NARC_local_tvt_chara_03_NCLR , NARC_local_tvt_chara_03_01_NCGR },
    { NARC_local_tvt_chara_04_NCLR , NARC_local_tvt_chara_04_01_NCGR },
};


static const u8 bgResList[][2] = {
    { NARC_local_tvt_back_01_NCLR , NARC_local_tvt_back_01_01_NCGR },
    { NARC_local_tvt_back_02_NCLR , NARC_local_tvt_back_02_01_NCGR },
    { NARC_local_tvt_back_03_NCLR , NARC_local_tvt_back_03_01_NCGR },
    { NARC_local_tvt_back_04_NCLR , NARC_local_tvt_back_04_01_NCGR },
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

static void LOCAL_TVT_CHARA_LoadCommonResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_LoadCharaResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork );
static void LOCAL_TVT_CHARA_LoadNcgResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork , ARCDATID datId );
static void LOCAL_TVT_CHARA_TransNcgResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork , const u32 topOfs , const u32 transOfs , const u32 transSize );

//--------------------------------------------------------------
//	初期化
//--------------------------------------------------------------
LOCAL_TVT_CHARA* LOCAL_TVT_CHARA_Init( LOCAL_TVT_WORK *work , const u8 charaIdx )
{
  LOCAL_TVT_CHARA *charaWork = GFL_HEAP_AllocMemory( work->heapId , sizeof( LOCAL_TVT_CHARA ) );
  
  charaWork->charaIdx = charaIdx;
  charaWork->charaType = work->initWork->charaType[charaIdx];
  charaWork->bgType = work->initWork->bgType[charaIdx];
  charaWork->charaAnmIdx = 0;
  charaWork->bgAnmIdx = 0;
  
  LOCAL_TVT_CHARA_LoadCharaResource( work , charaWork );
  LOCAL_TVT_CHARA_LoadCommonResource( work , charaWork );
  
  charaWork->state = LTCTS_WAIT_REQ;
  charaWork->resData = NULL;
  
  charaWork->anmCnt = GFUser_GetPublicRand0(LTVT_CHARA_UPDATE_CNT);
  
  return charaWork;
}

//--------------------------------------------------------------
//	開放
//--------------------------------------------------------------
void LOCAL_TVT_CHARA_Term( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork )
{
  if( charaWork->resData != NULL )
  {
    GFL_HEAP_FreeMemory( charaWork->resData );
  }

  
  GFL_HEAP_FreeMemory( charaWork );
}

//--------------------------------------------------------------
//	更新
//--------------------------------------------------------------
void LOCAL_TVT_CHARA_Main( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork )
{
  const u32 charSize = (work->initWork->mode == LTM_2_MEMBER?0x6000:0x3000);
  const u32 bufOfs = (work->bufNo==0?0:LTVT_VRAM_PAGE_SIZE);

  switch( charaWork->state )
  {
  case LTCTS_WAIT_REQ:
    break;
  case LTCTS_CHARA1:
    charaWork->transIdx = 0;
    LOCAL_TVT_CHARA_LoadNcgResource( work , charaWork , charaResList[charaWork->charaType][1] + charaWork->charaAnmIdx );
    charaWork->state = LTCTS_CHARA2;
    break;
  case LTCTS_CHARA2:
    LOCAL_TVT_CHARA_TransNcgResource( work , charaWork ,
                                      charaWork->transIdx * LTVT_TRANS_SIZE ,
                                      LTVT_VRAM_ADR_CHARA + charSize * charaWork->charaIdx + charaWork->transIdx * LTVT_TRANS_SIZE + bufOfs ,
                                      LTVT_TRANS_SIZE );
    charaWork->transIdx++;
    if( charaWork->transIdx * LTVT_TRANS_SIZE >= charSize )
    {
      charaWork->state = LTCTS_BG1;
      GFL_HEAP_FreeMemory( charaWork->resData );
      charaWork->resData = NULL;
    }
    break;
  case LTCTS_BG1:
    charaWork->transIdx = 0;
    LOCAL_TVT_CHARA_LoadNcgResource( work , charaWork , bgResList[charaWork->charaType][1] + charaWork->bgAnmIdx );
    charaWork->state = LTCTS_BG2;
    break;
  case LTCTS_BG2:
    LOCAL_TVT_CHARA_TransNcgResource( work , charaWork ,
                                      charaWork->transIdx * LTVT_TRANS_SIZE ,
                                      LTVT_VRAM_ADR_BG + charSize * charaWork->charaIdx + charaWork->transIdx * LTVT_TRANS_SIZE + bufOfs ,
                                      LTVT_TRANS_SIZE );
    charaWork->transIdx++;
    if( charaWork->transIdx * LTVT_TRANS_SIZE >= charSize )
    {
      charaWork->state = LTCTS_WAIT_TRANS;
      GFL_HEAP_FreeMemory( charaWork->resData );
      charaWork->resData = NULL;
    }
    break;
  }
  if( charaWork->anmCnt < LTVT_CHARA_UPDATE_CNT )
  {
    charaWork->anmCnt++;
  }
  else
  {
    if( charaWork->state == LTCTS_WAIT_TRANS ||
        charaWork->state == LTCTS_WAIT_REQ )
    {
      charaWork->anmCnt = 0;
      charaWork->charaAnmIdx = !charaWork->charaAnmIdx;
      charaWork->bgAnmIdx = !charaWork->bgAnmIdx;
    }
  }
}

//--------------------------------------------------------------
//	絵の読み込み要求
//--------------------------------------------------------------
void LOCAL_TVT_CHARA_LoadChara( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork , const u8 bufNo)
{
  charaWork->state = LTCTS_CHARA1;
}
//--------------------------------------------------------------
//	絵の読み込み完了チェック
//--------------------------------------------------------------
const BOOL LOCAL_TVT_CHARA_isFinishTrans( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork )
{
  if( charaWork->state == LTCTS_WAIT_TRANS )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//	絵の読み込み(パレット・スクリーン
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_LoadCommonResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork )
{
  u8  screenHeight;
  if( work->initWork->mode == LTM_2_MEMBER )
  {
    screenHeight = LTVT_CHARA_SCREEN_HEIGHT;
  }
  else
  {
    screenHeight = LTVT_CHARA_SCREEN_HEIGHT/2;
  }
  
  GFL_ARCHDL_UTIL_TransVramPaletteEx( work->archandle , 
                                      charaResList[charaWork->charaType][0] ,
                                      PALTYPE_MAIN_BG_EX ,
                                      0 ,
                                      (2*16*16) * charaWork->charaIdx + 0x4000 ,
                                      (2*16*16) ,
                                      work->heapId );
  GFL_ARCHDL_UTIL_TransVramPaletteEx( work->archandle , 
                                      bgResList[charaWork->charaType][0] ,
                                      PALTYPE_MAIN_BG_EX ,
                                      0 ,
                                      (2*16*16) * charaWork->charaIdx + 0x6000 ,
                                      (2*16*16) ,
                                      work->heapId );

  GFL_BG_ChangeScreenPalette( LTVT_FRAME_CHARA , 
                              charaWork->charaIdx%2 * LTVT_CHARA_SCREEN_WIDTH,
                              charaWork->charaIdx/2 * screenHeight,
                              LTVT_CHARA_SCREEN_WIDTH , 
                              screenHeight , 
                              charaWork->charaIdx+LTVT_PLT_CHARA );
  GFL_BG_ChangeScreenPalette( LTVT_FRAME_BG , 
                              charaWork->charaIdx%2 * LTVT_CHARA_SCREEN_WIDTH,
                              charaWork->charaIdx/2 * screenHeight,
                              LTVT_CHARA_SCREEN_WIDTH , 
                              screenHeight , 
                              charaWork->charaIdx+LTVT_PLT_BG );

  GFL_BG_LoadScreenV_Req( LTVT_FRAME_BG );
  GFL_BG_LoadScreenV_Req( LTVT_FRAME_CHARA );
}


//--------------------------------------------------------------
//	絵の読み込み(キャラ
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_LoadCharaResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork )
{
  u32 charSize;
  if( work->initWork->mode == LTM_2_MEMBER )
  {
    charSize = 0x6000;
  }
  else
  {
    charSize = 0x3000;
  }

  LOCAL_TVT_CHARA_LoadNcgResource( work , charaWork , charaResList[charaWork->charaType][1] );
  LOCAL_TVT_CHARA_TransNcgResource( work , charaWork ,
                                    0 ,
                                    LTVT_VRAM_ADR_CHARA + charSize * charaWork->charaIdx ,
                                    charSize );
  GFL_HEAP_FreeMemory( charaWork->resData );
  
  LOCAL_TVT_CHARA_LoadNcgResource( work , charaWork , bgResList[charaWork->charaType][1] );
  LOCAL_TVT_CHARA_TransNcgResource( work , charaWork ,
                                    0 ,
                                    LTVT_VRAM_ADR_BG + charSize * charaWork->charaIdx ,
                                    charSize );
  GFL_HEAP_FreeMemory( charaWork->resData );
}

static void LOCAL_TVT_CHARA_LoadNcgResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork , ARCDATID datId )
{
  //OS_TPrintf("[%d]L\n",charaWork->charaIdx);
  charaWork->resData = GFL_ARCHDL_UTIL_LoadBGCharacter( work->archandle , 
                                   datId ,
                                   FALSE ,
                                   &charaWork->charaData ,
                                   work->heapId );
//  DC_FlushAll();
//  DC_FlushRange( charaWork->charaData->pRawData , charaWork->charaData->szByte );
}


//--------------------------------------------------------------
//	絵の読み込み機能部(キャラ
//--------------------------------------------------------------
static void LOCAL_TVT_CHARA_TransNcgResource( LOCAL_TVT_WORK *work , LOCAL_TVT_CHARA *charaWork ,
                                                   const u32 topOfs , const u32 transOfs , const u32 transSize )
{
  //OS_TPrintf("[%d]T\n",charaWork->charaIdx);
  GFL_STD_MemCopy32( (void*)((u32)charaWork->charaData->pRawData + topOfs) , (void*)(HW_BG_VRAM + transOfs ) , transSize );

}