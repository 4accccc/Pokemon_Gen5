
//============================================================================================
/**
 * @file	btlv_field.c
 * @brief	戦闘画面描画用関数（フィールド）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"

#include "battle/batt_bg_tbl.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"
#include "batt_bg_tbl.naix"

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

struct _BTLV_FIELD_WORK
{
	GFL_G3D_RES*          field_resource;
  int                   anm_count;
	GFL_G3D_RES**         field_anm_resource;
	GFL_G3D_ANM**         field_anm;
	GFL_G3D_RND*          field_render;
	GFL_G3D_OBJ*          field_obj;
	GFL_G3D_OBJSTATUS     field_status;
  EFFTOOL_PAL_FADE_WORK epfw;
  u32                   vanish_flag :1;
  u32                               :31;
	HEAPID                heapID;
};

typedef	struct
{
	BTLV_FIELD_WORK		*bfw;
	EFFTOOL_MOVE_WORK	emw;
}BTLV_FIELD_TCB_WORK;

#define	BTLV_FIELD_ANM_MAX        ( 0 )	    		//背景のアニメーション数
                                                //（初期化関数から１の時は０を要求されているので実際は１）

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	index   読み込むリソースのINDEX
 * @param[in]	season	季節INDEX
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTLV_FIELD_WORK	*BTLV_FIELD_Init( int index, u8 season, HEAPID heapID )
{
	BTLV_FIELD_WORK *bfw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_FIELD_WORK ) );
	BOOL	ret;
  BATT_BG_TBL_BG_TABLE* bbtbt = GFL_ARC_LoadDataAlloc( ARCID_BATT_BG_TBL, NARC_batt_bg_tbl_batt_bg_bin, heapID );

	bfw->heapID = heapID;

	//リソース読み込み
  if( bbtbt[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ season ] != BATT_BG_TBL_NO_FILE )
  { 
	  bfw->field_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtbt[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ season ] );
  }
  else
  { 
	  bfw->field_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtbt[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ 0 ] );
  }
	ret = GFL_G3D_TransVramTexture( bfw->field_resource );
	GF_ASSERT( ret == TRUE );

  bfw->anm_count = 0;
  { 
    int i;

    for( i = BATT_BG_TBL_FILE_NSBCA ; i < BATT_BG_TBL_FILE_NSBMA + 1 ; i++ )
    { 
	    if( bbtbt[ index ].file[ i ][ season ] != BATT_BG_TBL_NO_FILE )
      { 
        bfw->anm_count++;
      }
    }
  }

	//RENDER生成
	bfw->field_render = GFL_G3D_RENDER_Create( bfw->field_resource, 0, bfw->field_resource );

  if( bfw->anm_count )
  { 
    int i, cnt;

    bfw->field_anm_resource = GFL_HEAP_AllocMemory( bfw->heapID, 4 * bfw->anm_count );
    bfw->field_anm = GFL_HEAP_AllocMemory( bfw->heapID, 4 * bfw->anm_count );

    cnt = 0;

    for( i = BATT_BG_TBL_FILE_NSBCA ; i < BATT_BG_TBL_FILE_NSBMA + 1 ; i++ )
    { 
	    if( bbtbt[ index ].file[ i ][ season ] != BATT_BG_TBL_NO_FILE )
      { 
		    //ANIME生成
	      bfw->field_anm_resource[ cnt ] = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtbt[ index ].file[ i ][ season ] );
		    bfw->field_anm[ cnt ] = GFL_G3D_ANIME_Create( bfw->field_render, bfw->field_anm_resource[ cnt ], BTLV_FIELD_ANM_MAX ); 
        cnt++;
      }
    }
		//OBJ生成
		bfw->field_obj = GFL_G3D_OBJECT_Create( bfw->field_render, bfw->field_anm, bfw->anm_count );
    //ANIME起動
    for( i = 0 ; i < bfw->anm_count ; i++ )
    { 
		  GFL_G3D_OBJECT_EnableAnime( bfw->field_obj, i );
    }
  }
  else
  { 
		bfw->field_obj = GFL_G3D_OBJECT_Create( bfw->field_render, NULL, 0 );
  }

	bfw->field_status.trans.x = 0;
	bfw->field_status.trans.y = 0;
	bfw->field_status.trans.z = 0;
	bfw->field_status.scale.x = FX32_ONE;
	bfw->field_status.scale.y = FX32_ONE;
	bfw->field_status.scale.z = FX32_ONE;
	MTX_Identity33( &bfw->field_status.rotate );
  
  //パレットフェード用ワーク生成
  { 
  	NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( bfw->field_resource );
  	NNSG3dResTex*		    	pTex = NNS_G3dGetTex( header ); 
    u32                   size = (u32)pTex->plttInfo.sizePltt << 3;

    bfw->epfw.g3DRES     = GFL_HEAP_AllocMemory( bfw->heapID, 4 );
    bfw->epfw.pData_dst  = GFL_HEAP_AllocMemory( bfw->heapID, 4 );
    
    bfw->epfw.g3DRES[ 0 ]     = bfw->field_resource;
    bfw->epfw.pData_dst[ 0 ]  = GFL_HEAP_AllocMemory( bfw->heapID, size );
	  bfw->epfw.pal_fade_flag   = 0;
	  bfw->epfw.pal_fade_count  = 1;
  }

  GFL_HEAP_FreeMemory( bbtbt );

	return bfw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bfw	BTLV_FIELD管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_FIELD_Exit( BTLV_FIELD_WORK *bfw )
{
	GFL_G3D_OBJECT_Delete( bfw->field_obj );

  if( bfw->anm_count )
  { 
    int i;

    for( i = 0 ; i < bfw->anm_count ; i++ )
    { 
		  GFL_G3D_ANIME_Delete( bfw->field_anm[ i ] );
		  GFL_G3D_DeleteResource( bfw->field_anm_resource[ i ] );
    }
		GFL_HEAP_FreeMemory( bfw->field_anm );
		GFL_HEAP_FreeMemory( bfw->field_anm_resource );
  }

	GFL_G3D_RENDER_Delete( bfw->field_render );
	GFL_G3D_DeleteResource( bfw->field_resource );

  GFL_HEAP_FreeMemory( bfw->epfw.pData_dst[ 0 ] );
  GFL_HEAP_FreeMemory( bfw->epfw.pData_dst );
  GFL_HEAP_FreeMemory( bfw->epfw.g3DRES );

	GFL_HEAP_FreeMemory( bfw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bfw	BTLV_FIELD管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_FIELD_Main( BTLV_FIELD_WORK *bfw )
{
  //パレットフェード
  BTLV_EFFTOOL_CalcPaletteFade( &bfw->epfw );
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	bfw	BTLV_FIELD管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_FIELD_Draw( BTLV_FIELD_WORK *bfw )
{
	int	i;

  if( bfw->vanish_flag )
  { 
    return;
  }

	GFL_G3D_DRAW_DrawObject( bfw->field_obj, &bfw->field_status );
}

//============================================================================================
/**
 *	パレットフェードセット
 *
 * @param[in]	bfw	      BTLV_FIELD管理ワークへのポインタ
 * @param[in]	start_evy	セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in]	end_evy		セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in]	wait			セットするパラメータ（ウェイト）
 * @param[in]	rgb				セットするパラメータ（フェードさせる色）
 */
//============================================================================================
void	BTLV_FIELD_SetPaletteFade( BTLV_FIELD_WORK *bfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb )
{ 
	GF_ASSERT( bfw );

	bfw->epfw.pal_fade_flag      = 1;
	bfw->epfw.pal_fade_start_evy = start_evy;
	bfw->epfw.pal_fade_end_evy   = end_evy;
	bfw->epfw.pal_fade_wait      = 0;
	bfw->epfw.pal_fade_wait_tmp  = wait;
	bfw->epfw.pal_fade_rgb       = rgb;
}

//============================================================================================
/**
 *	パレットフェード実行中かチェックする
 *
 * @param[in]	bfw  BTLV_FIELD管理ワークへのポインタ
 *
 * @retval  TRUE:実行中　FALSE:終了
 */
//============================================================================================
BOOL	BTLV_FIELD_CheckExecutePaletteFade( BTLV_FIELD_WORK* bfw )
{ 
  return ( bfw->epfw.pal_fade_flag != 0 );
}

//============================================================================================
/**
 *	バニッシュフラグセット
 *
 * @param[in]	bsw   BTLV_FIELD管理ワークへのポインタ
 * @param[in]	flag  セットするフラグ( BTLV_FIELD_VANISH_ON BTLV_FIELD_VANISH_OFF )
 */
//============================================================================================
void	BTLV_FIELD_SetVanishFlag( BTLV_FIELD_WORK* bfw, BTLV_FIELD_VANISH flag )
{ 
  bfw->vanish_flag = flag;
}

