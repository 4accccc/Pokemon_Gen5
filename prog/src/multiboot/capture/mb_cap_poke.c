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
  GFL_BBD_SYS *bbdSys;
  
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
MB_CAP_POKE *MB_CAP_POKE_CreateObject( MB_CAP_POKE_INIT_WORK *initWork )
{
  MB_CAP_POKE *mbPoke = GFL_HEAP_AllocClearMemory( initWork->heapId , sizeof( MB_CAP_POKE ) );

  GFL_G3D_RES* res = GFL_G3D_CreateResourceHandle( initWork->arcHandle , NARC_mb_capture_gra_cap_poke_dummy_nsbtx );
  const BOOL flg = TRUE;
  VecFx32 pos = {0,0,0};
  static u8 tempPos = 0;
  
  pos.x = FX32_CONST(tempPos*32+32);
  pos.y = FX32_CONST(80);
  pos.z = FX32_CONST(200);
  tempPos++;
  
  mbPoke->bbdSys = initWork->bbdSys;
  
  GFL_G3D_TransVramTexture( res );
  mbPoke->resIdx = GFL_BBD_AddResource( mbPoke->bbdSys , 
                                       res , 
                                       GFL_BBD_TEXFMT_PAL16 ,
                                       GFL_BBD_TEXSIZDEF_32x64 ,
                                       32 , 32 );
  GFL_BBD_CutResourceData( mbPoke->bbdSys , mbPoke->resIdx );
  
  mbPoke->objIdx = GFL_BBD_AddObject( mbPoke->bbdSys , 
                                     mbPoke->resIdx ,
                                     FX32_ONE , 
                                     FX32_ONE , 
                                     &pos ,
                                     31 ,
                                     GFL_BBD_LIGHT_NONE );
  GFL_BBD_SetObjectDrawEnable( mbPoke->bbdSys , mbPoke->objIdx , &flg );
  //3D設定の関係で反転させる・・・
  GFL_BBD_SetObjectFlipT( mbPoke->bbdSys , mbPoke->objIdx , &flg );

  //アイコンデータをVRAMへ送る
  {
    const u32 pltResNo = MB_ICON_GetPltResId( initWork->cardType );
    const u32 ncgResNo = MB_ICON_GetCharResId( initWork->ppp , initWork->cardType );
    const u8  palNo = MB_ICON_GetPalNumber( initWork->ppp );
    NNSG2dPaletteData *pltData;
    NNSG2dCharacterData *charData;
    void *pltRes = GFL_ARCHDL_UTIL_LoadPalette( initWork->pokeArcHandle , pltResNo , &pltData , GFL_HEAP_LOWID( initWork->heapId ) );
    void *ncgRes = GFL_ARCHDL_UTIL_LoadOBJCharacter( initWork->pokeArcHandle , ncgResNo , FALSE , &charData , GFL_HEAP_LOWID( initWork->heapId ) );
    u32 texAdr,pltAdr,pltDataAdr;
    GFL_BMP_DATA *bmpData = GFL_BMP_CreateWithData( charData->pRawData , 4 , 8 , GFL_BMP_16_COLOR , GFL_HEAP_LOWID( initWork->heapId ) );
    
    GFL_BMP_DataConv_to_BMPformat( bmpData , FALSE , GFL_HEAP_LOWID( initWork->heapId ) );
    
    GFL_BBD_GetResourceTexPlttAdrs( mbPoke->bbdSys , mbPoke->resIdx , &pltAdr );
    GFL_BBD_GetResourceTexDataAdrs( mbPoke->bbdSys , mbPoke->resIdx , &texAdr );
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

  return mbPoke;
}

//--------------------------------------------------------------
//	オブジェクト開放
//--------------------------------------------------------------
void MB_CAP_POKE_DeleteObject( MB_CAP_POKE *CAP_POKE )
{
 
  GFL_HEAP_FreeMemory( CAP_POKE );
}



