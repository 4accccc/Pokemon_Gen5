
//============================================================================================
/**
 * @file	btlv_stage.c
 * @brief	戦闘画面描画用関数（お盆）
 * @author	soga
 * @date	2008.11.18
 */
//============================================================================================
#include <gflib.h>

#include "btlv_stage.h"
#include "btlv_efftool.h"

#include "battle/batt_bg_tbl.h"

#include "arc_def.h"
#include "battle/battgra_wb.naix"
#include "batt_bg_tbl.naix"

//============================================================================================
/**
 *	定数宣言
 */
//============================================================================================

#define	BTLV_STAGE_DEFAULT_SCALE  ( FX32_ONE )	//お盆のデフォルトスケール値
#define	BTLV_STAGE_ANM_MAX        ( 0 )	    		//お盆のアニメーション数
                                                //（初期化関数から１の時は０を要求されているので実際は１）

#define	BTLV_STAGE_ANM_NO         ( 0 )         //お盆のアニメーションナンバー
#define	BTLV_STAGE_ANM_WAIT       ( FX32_ONE )  //お盆のアニメーションウェイト

typedef enum
{ 
  BTLV_STAGE_ANM_TYPE_LOOP = 0,   //アニメーションタイプ：ループ
  BTLV_STAGE_ANM_TYPE_REQ,        //アニメーションタイプ：リクエスト
}BTLV_STAGE_ANM_TYPE;

//============================================================================================
/**
 *	構造体宣言
 */
//============================================================================================

typedef struct
{ 
  BTLV_STAGE_ANM_TYPE anm_type;
  BOOL                anm_req_flag;
  fx32                anm_speed;
  fx32                anm_distance;
}BTLV_STAGE_ANM_WORK;

struct _BTLV_STAGE_WORK
{
	GFL_G3D_RES*            stage_resource;
  int                     anm_count;
  BTLV_STAGE_ANM_WORK*    stage_anm_work[ BTLV_STAGE_MAX ];
	GFL_G3D_RES**           stage_anm_resource;
	GFL_G3D_ANM**           stage_anm[ BTLV_STAGE_MAX ];
	GFL_G3D_RND*            stage_render[ BTLV_STAGE_MAX ];
	GFL_G3D_OBJ*            stage_obj[ BTLV_STAGE_MAX ];
	GFL_G3D_OBJSTATUS       stage_status[ BTLV_STAGE_MAX ];
  EFFTOOL_PAL_FADE_WORK   epfw;
  u32                     vanish_flag :1;
  u32                                 :31;
	HEAPID		  	          heapID;
};

//============================================================================================
/**
 *	お盆の位置テーブル
 */
//============================================================================================
static	const	VecFx32	stage_pos_table[]={
//  { 0, 0, FX_F32_TO_FX32( 6.845f ) },
//  { 0, 0, FX_F32_TO_FX32( -12.0f ) },
  { 0, 0, FX_F32_TO_FX32( 5.449f ) },
  { 0, 0, FX_F32_TO_FX32( -13.718f ) },
};

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_STAGE_CalcPaletteFade( BTLV_STAGE_WORK* bsw );

//============================================================================================
/**
 *	システム初期化
 *
 * @param[in]	rule    戦闘ルール
 * @param[in]	index   読み込むリソースのINDEX
 * @param[in]	season	季節INDEX
 * @param[in]	heapID	ヒープID
 */
//============================================================================================
BTLV_STAGE_WORK	*BTLV_STAGE_Init( BtlRule rule, int index, u8 season, HEAPID heapID )
{
	BTLV_STAGE_WORK *bsw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_STAGE_WORK ) );
	BOOL	ret;
  BATT_BG_TBL_STAGE_TABLE* bbtst = GFL_ARC_LoadDataAlloc( ARCID_BATT_BG_TBL, NARC_batt_bg_tbl_batt_stage_bin, heapID );

	bsw->heapID = heapID;

	//リソース読み込み
  if( rule == BTL_RULE_ROTATION )
  { 
    //ローテーションバトルは専用地面
	  bsw->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, NARC_battgra_wb_batt_st_vs3_nsbmd );
  }
	else if( bbtst[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ season ] != BATT_BG_TBL_NO_FILE )
  { 
	  bsw->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtst[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ season ] );
  }
  else
  { 
	  bsw->stage_resource = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtst[ index ].file[ BATT_BG_TBL_FILE_NSBMD ][ 0 ] );
  }
	ret = GFL_G3D_TransVramTexture( bsw->stage_resource );
	GF_ASSERT( ret == TRUE );

  bsw->anm_count = 0;
  { 
    int i;

    for( i = BATT_BG_TBL_FILE_NSBCA ; i < BATT_BG_TBL_FILE_NSBMA + 1 ; i++ )
    { 
	    if( bbtst[ index ].file[ i ][ season ] != BATT_BG_TBL_NO_FILE )
      { 
        bsw->anm_count++;
      }
    }
  }

	//RENDER生成
	bsw->stage_render[ BTLV_STAGE_MINE ] = GFL_G3D_RENDER_Create( bsw->stage_resource, 0, bsw->stage_resource );
	bsw->stage_render[ BTLV_STAGE_ENEMY ] = GFL_G3D_RENDER_Create( bsw->stage_resource, 0, bsw->stage_resource );

  //ローテーションは専用アニメ
  if( rule == BTL_RULE_ROTATION )
  { 
    int i;

    bsw->anm_count = 1;

    bsw->stage_anm_resource = GFL_HEAP_AllocMemory( bsw->heapID, 4 );
    bsw->stage_anm_resource[ 0 ] = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, NARC_battgra_wb_batt_st_vs3_nsbca );

    for( i = 0 ; i < BTLV_STAGE_MAX ; i++ )
    { 
      bsw->stage_anm[ i ] = GFL_HEAP_AllocMemory( bsw->heapID, 4 );
      bsw->stage_anm_work[ i ] = GFL_HEAP_AllocClearMemory( bsw->heapID, sizeof( BTLV_STAGE_ANM_WORK ) );

      bsw->stage_anm[ i ][ 0 ] = GFL_G3D_ANIME_Create( bsw->stage_render[ i ], bsw->stage_anm_resource[ 0 ], BTLV_STAGE_ANM_MAX ); 

      bsw->stage_anm_work[ i ][ 0 ].anm_type = BTLV_STAGE_ANM_TYPE_REQ;
		  bsw->stage_obj[ i ] = GFL_G3D_OBJECT_Create( bsw->stage_render[ i ], bsw->stage_anm[ i ], bsw->anm_count );
		  GFL_G3D_OBJECT_EnableAnime( bsw->stage_obj[ i ], 0 );
    }

  }
  else if( bsw->anm_count )
  { 
    int i, j, cnt;

    bsw->stage_anm_resource  = GFL_HEAP_AllocMemory( bsw->heapID, 4 * bsw->anm_count );

    for( i = 0 ; i < BTLV_STAGE_MAX ; i++ )
    { 
      bsw->stage_anm[ i ]           = GFL_HEAP_AllocMemory( bsw->heapID, 4 * bsw->anm_count );
      bsw->stage_anm_work[ i ]      = GFL_HEAP_AllocClearMemory( bsw->heapID, sizeof( BTLV_STAGE_ANM_WORK ) * bsw->anm_count );
    }

    cnt = 0;

    for( i = BATT_BG_TBL_FILE_NSBCA ; i < BATT_BG_TBL_FILE_NSBMA + 1 ; i++ )
    { 
	    if( bbtst[ index ].file[ i ][ season ] != BATT_BG_TBL_NO_FILE )
      { 
	      bsw->stage_anm_resource[ cnt ] = GFL_G3D_CreateResourceArc( ARCID_BATTGRA, bbtst[ index ].file[ i ][ season ] );
        for( j = 0 ; j < BTLV_STAGE_MAX ; j++ )
        { 
		      //ANIME生成
		      bsw->stage_anm[ j ][ cnt ] = GFL_G3D_ANIME_Create( bsw->stage_render[ j ], bsw->stage_anm_resource[ cnt ], BTLV_STAGE_ANM_MAX ); 
          bsw->stage_anm_work[ j ][ cnt ].anm_type = BTLV_STAGE_ANM_TYPE_LOOP;
        }
        cnt++;
      }
    }

    for( j = 0 ; j < BTLV_STAGE_MAX ; j++ )
    { 
		  //OBJ生成
		  bsw->stage_obj[ j ] = GFL_G3D_OBJECT_Create( bsw->stage_render[ j ], bsw->stage_anm[ j ], bsw->anm_count );
      //ANIME起動
      for( i = 0 ; i < bsw->anm_count ; i++ )
      { 
		    GFL_G3D_OBJECT_EnableAnime( bsw->stage_obj[ j ], i );
      }
    }
  }
  else
  { 
		bsw->stage_obj[ BTLV_STAGE_MINE ] = GFL_G3D_OBJECT_Create( bsw->stage_render[ BTLV_STAGE_MINE ], NULL, 0 );
		bsw->stage_obj[ BTLV_STAGE_ENEMY ] = GFL_G3D_OBJECT_Create( bsw->stage_render[ BTLV_STAGE_ENEMY ], NULL, 0 );
  }
  //パレットフェード用ワーク生成
  { 
  	NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( bsw->stage_resource );
  	NNSG3dResTex*		    	pTex = NNS_G3dGetTex( header ); 
    u32                   size = (u32)pTex->plttInfo.sizePltt << 3;

    bsw->epfw.g3DRES     = GFL_HEAP_AllocMemory( bsw->heapID, 4 );
    bsw->epfw.pData_dst  = GFL_HEAP_AllocMemory( bsw->heapID, 4 );
    
    bsw->epfw.g3DRES[ 0 ]     = bsw->stage_resource;
    bsw->epfw.pData_dst[ 0 ]  = GFL_HEAP_AllocMemory( bsw->heapID, size );
	  bsw->epfw.pal_fade_flag   = 0;
	  bsw->epfw.pal_fade_count  = 1;
  }

	//自分側お盆
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.x = stage_pos_table[ BTLV_STAGE_MINE ].x;
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.y = stage_pos_table[ BTLV_STAGE_MINE ].y;
	bsw->stage_status[ BTLV_STAGE_MINE ].trans.z = stage_pos_table[ BTLV_STAGE_MINE ].z;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.x = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.y = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_MINE ].scale.z = BTLV_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTLV_STAGE_MINE ].rotate );

	//相手側お盆
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.x = stage_pos_table[ BTLV_STAGE_ENEMY ].x;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.y = stage_pos_table[ BTLV_STAGE_ENEMY ].y;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.z = stage_pos_table[ BTLV_STAGE_ENEMY ].z;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.x = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.y = BTLV_STAGE_DEFAULT_SCALE;
	bsw->stage_status[ BTLV_STAGE_ENEMY ].scale.z = BTLV_STAGE_DEFAULT_SCALE;
	MTX_Identity33( &bsw->stage_status[ BTLV_STAGE_ENEMY ].rotate );

  if( rule == BTL_RULE_ROTATION )
  { 
	  bsw->stage_status[ BTLV_STAGE_MINE ].trans.z = FX32_ONE * 10;
	  bsw->stage_status[ BTLV_STAGE_ENEMY ].trans.z = FX32_ONE * -15;
  }

	//エッジマーキングカラーセット
	G3X_SetEdgeColorTable( &bbtst[ index ].edge_color );
  
  GFL_HEAP_FreeMemory( bbtst );

	return bsw;
}

//============================================================================================
/**
 *	システム終了
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Exit( BTLV_STAGE_WORK* bsw )
{
  int i, j;

  if( bsw->anm_count )
  { 

    for( i = 0 ; i < bsw->anm_count ; i++ )
    { 
      for( j = 0 ; j < BTLV_STAGE_MAX ; j++ )
      { 
		    GFL_G3D_ANIME_Delete( bsw->stage_anm[ j ][ i ] );
		    GFL_G3D_DeleteResource( bsw->stage_anm_resource[ i ] );
      }
    }
    for( i = 0 ; i < BTLV_STAGE_MAX ; i++ )
    { 
		  GFL_HEAP_FreeMemory( bsw->stage_anm[ i ] );
		  GFL_HEAP_FreeMemory( bsw->stage_anm_work[ i ] );
    }
	  GFL_HEAP_FreeMemory( bsw->stage_anm_resource );
  }

  for( i = 0 ; i < BTLV_STAGE_MAX ; i++ )
  { 
	  GFL_G3D_OBJECT_Delete( bsw->stage_obj[ i ] );
	  GFL_G3D_RENDER_Delete( bsw->stage_render[ i ] );
  }

	GFL_G3D_DeleteResource( bsw->stage_resource );

  GFL_HEAP_FreeMemory( bsw->epfw.pData_dst[ 0 ] );
  GFL_HEAP_FreeMemory( bsw->epfw.pData_dst );
  GFL_HEAP_FreeMemory( bsw->epfw.g3DRES );

	GFL_HEAP_FreeMemory( bsw );
}

//============================================================================================
/**
 *	システムメイン
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Main( BTLV_STAGE_WORK* bsw )
{
  //アニメーション
	if(	bsw->anm_count ){
    int i, j;
    for( i = 0 ; i < bsw->anm_count ; i++ )
    { 
      for( j = 0 ; j < BTLV_STAGE_MAX ; j++ )
      { 
        fx32 speed = 0;
        switch( bsw->stage_anm_work[ j ][ i ].anm_type ){ 
        case BTLV_STAGE_ANM_TYPE_LOOP:
          speed = BTLV_STAGE_ANM_WAIT; 
          break;
        case BTLV_STAGE_ANM_TYPE_REQ:
          if( bsw->stage_anm_work[ j ][ i ].anm_req_flag == TRUE )
          { 
            speed = bsw->stage_anm_work[ j ][ i ].anm_speed;
            if( --bsw->stage_anm_work[ j ][ i ].anm_distance == 0 )
            { 
              bsw->stage_anm_work[ j ][ i ].anm_req_flag = FALSE;
            }
          }
          break;
        }
        if( speed )
        { 
		      GFL_G3D_OBJECT_LoopAnimeFrame( bsw->stage_obj[ j ], i, speed ); 
        }
      }
    }
	}
  //パレットフェード
  BTLV_EFFTOOL_CalcPaletteFade( &bsw->epfw );
}

//============================================================================================
/**
 *	描画
 *
 * @param[in]	bsw	BTLV_STAGE管理ワークへのポインタ
 */
//============================================================================================
void	BTLV_STAGE_Draw( BTLV_STAGE_WORK* bsw )
{
	int	i;

  if( bsw->vanish_flag )
  { 
    return;
  }

	for( i = 0 ; i < BTLV_STAGE_MAX ; i++ ){
		GFL_G3D_DRAW_DrawObject( bsw->stage_obj[ i ], &bsw->stage_status[ i ] );
	}
}

//============================================================================================
/**
 *	パレットフェードセット
 *
 * @param[in]	bsw       BTLV_STAGE管理ワークへのポインタ
 * @param[in]	start_evy	セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in]	end_evy		セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in]	wait			セットするパラメータ（ウェイト）
 * @param[in]	rgb				セットするパラメータ（フェードさせる色）
 */
//============================================================================================
void	BTLV_STAGE_SetPaletteFade( BTLV_STAGE_WORK* bsw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb )
{ 
	GF_ASSERT( bsw );

	bsw->epfw.pal_fade_flag      = 1;
	bsw->epfw.pal_fade_start_evy = start_evy;
	bsw->epfw.pal_fade_end_evy   = end_evy;
	bsw->epfw.pal_fade_wait      = 0;
	bsw->epfw.pal_fade_wait_tmp  = wait;
	bsw->epfw.pal_fade_rgb       = rgb;
}

//============================================================================================
/**
 *	パレットフェード実行中かチェックする
 *
 * @param[in]	bsw  BTLV_STAGE管理ワークへのポインタ
 *
 * @retval  TRUE:実行中　FALSE:終了
 */
//============================================================================================
BOOL	BTLV_STAGE_CheckExecutePaletteFade( BTLV_STAGE_WORK* bsw )
{ 
  return ( bsw->epfw.pal_fade_flag != 0 );
}

//============================================================================================
/**
 *	バニッシュフラグセット
 *
 * @param[in]	bsw   BTLV_STAGE管理ワークへのポインタ
 * @param[in]	flag  セットするフラグ( BTLV_STAGE_VANISH_ON BTLV_STAGE_VANISH_OFF )
 */
//============================================================================================
void	BTLV_STAGE_SetVanishFlag( BTLV_STAGE_WORK* bsw, BTLV_STAGE_VANISH flag )
{ 
  bsw->vanish_flag = flag;
}

//============================================================================================
/**
 *	アニメリクエスト
 *
 * @param[in]	bsw       BTLV_STAGE管理ワークへのポインタ
 * @param[in]	side      セットするステージの側
 * @param[in]	index     セットするアニメインデックス
 * @param[in]	speed     アニメスピード
 * @param[in]	distance  アニメフレーム数
 */
//============================================================================================
void	BTLV_STAGE_SetAnmReq( BTLV_STAGE_WORK* bsw, int side, int index, fx32 speed, int distance )
{ 
  GF_ASSERT( side < BTLV_STAGE_MAX );
  GF_ASSERT( bsw->stage_anm_work[ side ] );
  if( bsw->stage_anm_work[ side ][ index ].anm_type == BTLV_STAGE_ANM_TYPE_REQ )
  { 
    bsw->stage_anm_work[ side ][ index ].anm_req_flag = TRUE;
    bsw->stage_anm_work[ side ][ index ].anm_speed = speed;
    bsw->stage_anm_work[ side ][ index ].anm_distance = distance;
  }
}

//============================================================================================
/**
 *	リクエストしたアニメが実行中かチェックする
 *
 * @param[in]	bsw  BTLV_STAGE管理ワークへのポインタ
 *
 * @retval  TRUE:実行中　FALSE:終了
 */
//============================================================================================
BOOL	BTLV_STAGE_CheckExecuteAnmReq( BTLV_STAGE_WORK* bsw )
{ 
  int i;

  for( i = 0 ; i < bsw->anm_count ; i++ )
  { 
    if( ( bsw->stage_anm_work[ BTLV_STAGE_MINE ][ i ].anm_req_flag == TRUE ) ||
        ( bsw->stage_anm_work[ BTLV_STAGE_ENEMY ][ i ].anm_req_flag == TRUE ) )
    { 
      return TRUE;
    }
  }
  return FALSE;
}

