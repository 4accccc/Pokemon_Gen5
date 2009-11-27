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

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct
struct _MB_CAP_POKE
{
  int resIdx;
  int objIdx;
};

typedef struct
{
  const u32 datId;
  const u32 texSize;
}MB_CAP_POKE_DATA;

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto


//--------------------------------------------------------------
//	オブジェクト作成
//--------------------------------------------------------------
MB_CAP_POKE *MB_CAP_POKE_CreateObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE_INIT_WORK *initWork )
{
  const HEAPID heapId = MB_CAPTURE_GetHeapId( capWork );
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );
  ARCHANDLE *arcHandle = MB_CAPTURE_GetArcHandle( capWork );
  const DLPLAY_CARD_TYPE cardType = MB_CAPTURE_GetCardType( capWork );

  MB_CAP_POKE *pokeWork = GFL_HEAP_AllocClearMemory( heapId , sizeof( MB_CAP_POKE ) );

  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( arcHandle , NARC_mb_capture_gra_cap_poke_dummy_nsbtx );
  const BOOL flg = TRUE;
  VecFx32 pos = {0,0,0};
#if DEB_ARI
  static u8 tempPos = 0;
  
  pos.x = FX32_CONST(tempPos*32+32);
  pos.y = FX32_CONST(80);
  pos.z = FX32_CONST(200);
  tempPos++;
#endif
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
                                     &pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( bbdSys , pokeWork->objIdx , &flg );
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
  }

  return pokeWork;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_POKE_DeleteObject( MB_CAPTURE_WORK *capWork , MB_CAP_POKE *pokeWork )
{
  GFL_BBD_SYS *bbdSys = MB_CAPTURE_GetBbdSys( capWork );

  GFL_BBD_RemoveObject( bbdSys , pokeWork->objIdx );
  GFL_BBD_RemoveResource( bbdSys , pokeWork->resIdx );

  GFL_HEAP_FreeMemory( pokeWork );
}



