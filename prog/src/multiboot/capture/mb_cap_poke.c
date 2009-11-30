//======================================================================
/**
 * @file	  mb_cap_poke.c
 * @brief	  捕獲ゲーム・ポケモン
 * @author	ariizumi
 * @data	  09/11/24
 *
 * モジュール名：MB_CAP_POKE
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"

#include "mb_capture_gra.naix"

#include "multiboot/mb_poke_icon.h"

#include "./mb_cap_local_def.h"
#include "./mb_cap_poke.h"

//======================================================================
//	define
//======================================================================
#pragma mark [> define

#define MB_CAP_POKE_OFS_Y (8)


#define MB_CAP_POKE_HIDE_LOOK_TIME (60*3)
#define MB_CAP_POKE_HIDE_HIDE_TIME (60*4)
#define MB_CAP_POKE_HIDE_TOTAL_TIME (MB_CAP_POKE_HIDE_LOOK_TIME*2+MB_CAP_POKE_HIDE_HIDE_TIME*2)

#define MB_CAP_POKE_HIDE_LOOK_OFS (10)

#define MB_CAP_POKE_RUN_LOOK_TIME (60*3)
#define MB_CAP_POKE_RUN_HIDE_TIME (60*5)
#define MB_CAP_POKE_JUMP_HEIGHT (24)

#define MB_CAP_POKE_ANIM_SPEED (8)
#define MB_CAP_POKE_ANIM_FRAME (2)

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum
typedef enum
{
  MCPD_LEFT,
  MCPD_RIGHT,
  MCPD_UP,
  MCPD_DOWN,
}MB_CAP_POKE_DIR;

//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
typedef void (*MB_CAP_POKE_STATE_FUNC)(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );

struct _MB_CAP_POKE
{
  MB_CAP_POKE_STATE state;
  MB_CAP_POKE_STATE_FUNC stateFunc;
  u16 cnt;

  u16 anmFrame;
  u16 anmIdx;
  
  u8 posXidx;
  u8 posYidx;
  MB_CAP_POKE_DIR dir;
  VecFx32 pos;
  fx32 height;
  
  int resIdx;
  int objIdx;
  int objShadowIdx;
  
};


//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
static void MB_CAP_POKE_StateNone(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
static void MB_CAP_POKE_StateHide(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );
static void MB_CAP_POKE_StateRun(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork );


//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_POKE* MB_CAP_POKE_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  const DLPLAY_CARD_TYPE cardType = MB_CAPTURE_GetCardType( capWork );
  const int resShadowIdx = MB_CAPTURE_GetBbdResIdx( capWork , MCBR_SHADOW );

  MB_CAP_POKE *pokeWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_POKE ) );

  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( arcHandle , NARC_mb_capture_gra_cap_poke_dummy_nsbtx );
  const BOOL flg = TRUE;
  pokeWork->pos.x = 0;
  pokeWork->pos.y = 0;
  pokeWork->pos.z = 0;
  GFL_G3D_TransVramTexture( res );
  pokeWork->resIdx = GFL_BBD_AddResource( bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZDEF_32x64 ,
                                       32 , 32 );
  GFL_BBD_CutResourceData( bbdSys , pokeWork->resIdx );
  
  pokeWork->objIdx = GFL_BBD_AddObject( bbdSys , 
                                     pokeWork->resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &pokeWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  pokeWork->objShadowIdx = GFL_BBD_AddObject( bbdSys , 
                                     resShadowIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &pokeWork->pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( bbdSys , pokeWork->objIdx , &flg );

  //アイコンデータをVRAMへ送る
  {
    const u32 pltResNo = MB_ICON_GetPltResId( cardType );
    const u32 ncgResNo = MB_ICON_GetCharResId( initWork->ppp , cardType );
    const u8  palNo = MB_ICON_GetPalNumber( initWork->ppp );
    NNSG2dPaletteData *pltData;
    NNSG2dCharacterData *charData;
    void *pltRes = GFL_ARCHDL_UTIL_LoadPalette( initWork->pokeArcHandle , pltResNo , &pltData , GFL_HEAP_LOWID( heapId ) );
    void *ncgRes = GFL_ARCHDL_UTIL_LoadOBJCharacter( initWork->pokeArcHandle , ncgResNo , FALSE , &charData , GFL_HEAP_LOWID( heapId ) );
    u32 texAdr,pltAdr,pltDataAdr;
    GFL_BMP_DATA *bmpData = GFL_BMP_CreateWithData( charData->pRawData , 4 , 8 , GFL_BMP_16_COLOR , GFL_HEAP_LOWID( heapId ) );
    
    GFL_BMP_DataConv_to_BMPformat( bmpData , FALSE , GFL_HEAP_LOWID( heapId ) );
    
    GFL_BBD_GetResourceTexPlttAdrs( bbdSys , pokeWork->resIdx , &pltAdr );
    GFL_BBD_GetResourceTexDataAdrs( bbdSys , pokeWork->resIdx , &texAdr );
    //pltData->pRawData;
    //charData->pRawData;
    
    GX_BeginLoadTex();
    DC_FlushRange( charData->pRawData , 32*64*0x20 );
    GX_LoadTex( charData->pRawData , texAdr , 32*64*0x20 );
    GX_EndLoadTex();

    pltDataAdr = (u32)pltData->pRawData + palNo*32;
    GX_BeginLoadTexPltt();
    DC_FlushRange( (void*)pltDataAdr , 32 );
    GX_LoadTexPltt( (void*)pltDataAdr , pltAdr , 32 );
    GX_EndLoadTexPltt();
    
    GFL_HEAP_FreeMemory( pltRes );
    GFL_HEAP_FreeMemory( ncgRes );
    GFL_BMP_Delete( bmpData );
  }

  pokeWork->state = MCPS_NONE;
  pokeWork->stateFunc = MB_CAP_POKE_StateNone;
  pokeWork->anmFrame = 0;
  pokeWork->anmIdx = 0;

  pokeWork->posXidx = 0xFF;
  pokeWork->posYidx = 0xFF;
  pokeWork->height = 0;
  
  return pokeWork;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_POKE_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  GFL_BBD_RemoveObject( bbdSys , pokeWork->objShadowIdx );
  GFL_BBD_RemoveObject( bbdSys , pokeWork->objIdx );
  GFL_BBD_RemoveResource( bbdSys , pokeWork->resIdx );

  GFL_HEAP_FreeMemory( pokeWork );
}


//--------------------------------------------------------------
//	オブジェクト更新
//--------------------------------------------------------------
void MB_CAP_POKE_UpdateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  pokeWork->stateFunc( capWork , pokeWork);
  
  //アニメの更新
  pokeWork->anmFrame++;
  if( pokeWork->anmFrame >= MB_CAP_POKE_ANIM_SPEED )
  {
    pokeWork->anmFrame = 0;
    pokeWork->anmIdx++;
    if( pokeWork->anmIdx >= MB_CAP_POKE_ANIM_FRAME )
    {
      pokeWork->anmIdx = 0;
    }
    GFL_BBD_SetObjectCelIdx( bbdSys , pokeWork->objIdx , &pokeWork->anmIdx );
  }
  
}

//--------------------------------------------------------------
//	ポケモンセット(開始時隠れ用
//--------------------------------------------------------------
void MB_CAP_POKE_SetHide( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY )
{
  pokeWork->cnt = GFUser_GetPublicRand0(MB_CAP_POKE_HIDE_TOTAL_TIME);
  pokeWork->posXidx = idxX;
  pokeWork->posYidx = idxY;
  
  pokeWork->state = MCPS_HIDE;
  pokeWork->stateFunc = MB_CAP_POKE_StateHide;
}

//--------------------------------------------------------------
//	ポケモンセット(逃げる
//--------------------------------------------------------------
void MB_CAP_POKE_SetRun( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  if( pokeWork->cnt < MB_CAP_POKE_HIDE_LOOK_TIME )
  {
    pokeWork->dir = MCPD_LEFT;
  }
  else
  {
    pokeWork->dir = MCPD_RIGHT;
  }
  pokeWork->cnt = 0;
  pokeWork->stateFunc = MB_CAP_POKE_StateRun;
}

//--------------------------------------------------------------
//	ポケモンセット(捕獲
//--------------------------------------------------------------
void MB_CAP_POKE_SetCapture( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  const BOOL isDisp = FALSE;
  GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &isDisp );
  GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &isDisp );

  pokeWork->state = MCPS_CAPTURE;
  pokeWork->stateFunc = MB_CAP_POKE_StateNone;
}

#pragma mark [>state func
//--------------------------------------------------------------
//	ステート：無し
//--------------------------------------------------------------
static void MB_CAP_POKE_StateNone(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
}

//--------------------------------------------------------------
//	ステート：隠れている
//--------------------------------------------------------------
static void MB_CAP_POKE_StateHide(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  //ステートの変更を == にしているのは、初期設定値でいきなり出ないように
  pokeWork->cnt++;
  if( pokeWork->cnt >= MB_CAP_POKE_HIDE_TOTAL_TIME )
  {
    //左に出る
    const BOOL flg = TRUE;
    const BOOL flipFlg = FALSE;
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.x -= FX32_CONST( MB_CAP_POKE_HIDE_LOOK_OFS );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );
    GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );

    pokeWork->cnt = 0;
    pokeWork->state = MCPS_LOOK;
  }
  else
  if( pokeWork->cnt == MB_CAP_POKE_HIDE_LOOK_TIME+MB_CAP_POKE_HIDE_HIDE_TIME )
  {
    //右に出る
    const BOOL flg = TRUE;
    const BOOL flipFlg = TRUE;
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.x += FX32_CONST( MB_CAP_POKE_HIDE_LOOK_OFS );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );
    GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );

    pokeWork->state = MCPS_LOOK;
  }
  else
  if( pokeWork->cnt == MB_CAP_POKE_HIDE_LOOK_TIME ||
      pokeWork->cnt == MB_CAP_POKE_HIDE_LOOK_TIME*2+MB_CAP_POKE_HIDE_HIDE_TIME )
  {
    //隠れる
    const BOOL flg = FALSE;
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    pokeWork->pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &pokeWork->pos );

    pokeWork->state = MCPS_HIDE;
  }
}

//--------------------------------------------------------------
//	ステート：逃げる
//--------------------------------------------------------------
static void MB_CAP_POKE_StateRun(MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  pokeWork->cnt++;
  if( pokeWork->cnt < MB_CAP_POKE_RUN_LOOK_TIME )
  {
    //飛ぶ
    const BOOL flg = TRUE;
    BOOL flipFlg = FALSE;
    VecFx32 dispPos;
    //草からの基準位置
    MB_CAPTURE_GetGrassObjectPos( pokeWork->posXidx , pokeWork->posYidx , &pokeWork->pos );
    //pos.y -= FX32_CONST( MB_CAP_POKE_OFS_Y );
    pokeWork->pos.z -= FX32_CONST( MB_CAP_OBJ_BASE_Z-MB_CAP_POKE_BASE_Z );
    
    {
      //ジャンプの計算
      const u16 rot = 0x8000*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME;
      fx32 ofs;
      pokeWork->height = FX_SinIdx( rot ) * MB_CAP_POKE_JUMP_HEIGHT;
      switch( pokeWork->dir )
      {
      case MCPD_LEFT:
        ofs = -FX32_CONST(MB_CAP_OBJ_MAIN_X_SPACE);
        pokeWork->pos.x += ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME;
        break;
      case MCPD_RIGHT:
        ofs = FX32_CONST(MB_CAP_OBJ_MAIN_X_SPACE);
        pokeWork->pos.x += ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME;
        flipFlg = TRUE;
        break;
      case MCPD_UP:
        ofs = -FX32_CONST(MB_CAP_OBJ_MAIN_Y_SPACE);
        pokeWork->pos.y += ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME;
        flipFlg = TRUE;
        break;
      case MCPD_DOWN:
        ofs = FX32_CONST(MB_CAP_OBJ_MAIN_Y_SPACE);
        pokeWork->pos.y += ofs*pokeWork->cnt/MB_CAP_POKE_RUN_LOOK_TIME;
        break;
      }
    
      GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &flg );
      GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objShadowIdx , &pokeWork->pos );
      
      dispPos.x = pokeWork->pos.x;
      dispPos.y = pokeWork->pos.y-pokeWork->height;
      dispPos.z = pokeWork->pos.z;

      GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
      GFL_BBD_SetObjectTrans( bbdSys , pokeWork->objIdx , &dispPos );
      GFL_BBD_SetObjectFlipS( bbdSys , pokeWork->objIdx , &flipFlg );
    }

    pokeWork->state = MCPS_RUN_LOOK;
    
  }
  else
  if( pokeWork->cnt == MB_CAP_POKE_RUN_LOOK_TIME )
  {
    const BOOL flg = FALSE;
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objShadowIdx , &flg );
    GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
    switch( pokeWork->dir )
    {
    case MCPD_LEFT:
      pokeWork->posXidx--;
      break;
    case MCPD_RIGHT:
      pokeWork->posXidx++;
      break;
    case MCPD_UP:
      pokeWork->posYidx--;
      break;
    case MCPD_DOWN:
      pokeWork->posYidx++;
      break;
    }    
    pokeWork->state = MCPS_RUN_HIDE;
  }
  else
  if( pokeWork->cnt >= MB_CAP_POKE_RUN_LOOK_TIME+MB_CAP_POKE_RUN_HIDE_TIME )
  {
    pokeWork->cnt = 0;
  }
  

}

#pragma mark [>outer func
//--------------------------------------------------------------
//指定座標にいるか？
//--------------------------------------------------------------
const BOOL MB_CAP_POKE_CheckPos( const MB_CAP_POKE *pokeWork , const u8 idxX , const u8 idxY )
{
  if( pokeWork->posXidx == idxX &&
      pokeWork->posYidx == idxY )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
//
//--------------------------------------------------------------
const MB_CAP_POKE_STATE MB_CAP_POKE_GetState( const MB_CAP_POKE *pokeWork )
{
  return pokeWork->state;
}

//--------------------------------------------------------------
//  当たり判定作成
//--------------------------------------------------------------
void MB_CAP_POKE_GetHitWork( MB_CAP_POKE *pokeWork , MB_CAP_HIT_WORK *hitWork )
{
  hitWork->pos = &pokeWork->pos;
  hitWork->height = pokeWork->height;
  hitWork->size.x = FX32_CONST(MB_CAP_POKE_HITSIZE_X);
  hitWork->size.y = FX32_CONST(MB_CAP_POKE_HITSIZE_Y);
  hitWork->size.z = FX32_CONST(MB_CAP_POKE_HITSIZE_Z);
}
