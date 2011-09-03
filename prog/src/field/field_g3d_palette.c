//============================================================================================
/**
 * @brief field_g3d_palette.c
 * @brief
 * @date  2011.09.03
 * @author  tamada
 *
 * ハイリンク進入時に使用していたグレースケール処理を拡張して
 * セピア化も管理する仕組みにするようにした
 */
//============================================================================================
#include <gflib.h>
#include "field_g3d_palette.h"

#include "system/palanm.h"

//--------------------------------------------------------------
//  初期化
//--------------------------------------------------------------
void FLD_G3DPAL_Init( FLD_G3DPAL * g3dPal, const u8 * gray_scale, BOOL is_sepia )
{
  g3dPal->gray_scale = gray_scale;
  g3dPal->is_sepia = is_sepia;
}

//--------------------------------------------------------------
//  テクスチャカラー変更適用
//--------------------------------------------------------------
void FLD_G3DPAL_Filter( const FLD_G3DPAL * g3dPal, GFL_G3D_RES *g3Dres )
{
  if( g3dPal == NULL || g3Dres == NULL )
  {
    GF_ASSERT( 0 );
  }
  else
  {
    if( GFL_G3D_CheckResourceType(g3Dres, GFL_G3D_RES_CHKTYPE_TEX) )
    {
      NNSG3dResFileHeader*	header;
      NNSG3dResTex*			texture;
      u32 sz;
      void* pData;
    
      header = GFL_G3D_GetResourceFileHeader(g3Dres);
      texture = NNS_G3dGetTex( header ); 

      if ( texture )
      {
        sz = ( (u32)texture->plttInfo.sizePltt << 3 ) / sizeof(u16);
        pData = (u8*)texture + texture->plttInfo.ofsPlttData;
        if ( g3dPal->gray_scale )
        {
          GF_ASSERT( g3dPal->is_sepia == FALSE );
          PaletteGrayScaleShadeTable(pData, sz, g3dPal->gray_scale);
        } else if ( g3dPal->is_sepia )
        {
          PaletteSepia( pData, sz );
        }
      }
    }
  }

}


