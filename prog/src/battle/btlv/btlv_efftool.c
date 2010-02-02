
//============================================================================================
/**
 * @file	btlv_efftool.c
 * @brief	戦闘エフェクトツール
 * @author	soga
 * @date	2008.11.27
 */
//============================================================================================

#include <gflib.h>

#include "btlv_effect.h"

//============================================================================================
/**
 *	プロトタイプ宣言
 */
//============================================================================================

//============================================================================================
/**
 *	移動量を計算
 *
 * @param[in]	  start	移動元
 * @param[in]	  endif	移動先
 * @param[out]	out		移動量を格納するワークへのポインタ
 * @param[in]	  frame	移動フレーム数
 */
//============================================================================================
void	BTLV_EFFTOOL_CalcMove( fx32 start, fx32 end, fx32 *out, fx32 frame )
{
	*out = 0;
	if( end - start ){
		*out = FX_Div( end - start, frame );
		if( *out == 0 ){
			*out = 1;
		}
	}
}

//============================================================================================
/**
 *	移動量を計算（3軸Ver）
 *
 * @param[in]	start	移動元
 * @param[in]	endif	移動先
 * @param[out]	out		移動量を格納するワークへのポインタ
 * @param[in]	frame	移動フレーム数
 */
//============================================================================================
void	BTLV_EFFTOOL_CalcMoveVector( VecFx32 *start, VecFx32 *end, VecFx32 *out, fx32 frame )
{
	out->x = 0;
	out->y = 0;
	out->z = 0;
	if( end->x - start->x ){
		out->x = FX_Div( end->x - start->x, frame );
		if( out->x == 0 ){
			out->x = 1;
		}
	}
	if( end->y - start->y ){
		out->y = FX_Div( end->y - start->y, frame );
		if( out->y == 0 ){
			out->y = 1;
		}
	}
	if( end->z - start->z ){
		out->z = FX_Div( end->z - start->z, frame );
		if( out->z == 0 ){
			out->z = 1;
		}
	}
}

//============================================================================================
/**
 *	もらった値から移動できるかチェックして移動できないときは何もしない
 *
 *	@param[in]	now_pos		現在値
 *	@param[in]	vec			  移動量
 *	@param[in]	move_pos	移動先
 *	@param[out]	ret			  移動したかどうかを返す（FALSE:移動しない）
 */
//============================================================================================
void	BTLV_EFFTOOL_CheckMove( fx32 *now_pos, fx32 *vec, fx32 *move_pos, BOOL *ret )
{
	*now_pos += *vec;

	if( *vec < 0 ){
		if( *now_pos <= *move_pos ){
			*now_pos = *move_pos;
		}
		else{
			*ret = FALSE;
		}
	}
	else{
		if( *now_pos >= *move_pos ){
			*now_pos = *move_pos;
		}
		else{
      if( ret != NULL )
      { 
			  *ret = FALSE;
      }
		}
	}
}

//============================================================================================
/**
 *	もらった値から移動できるかチェックして移動できないときは何もしない（3軸バージョン）
 *
 *	@param[in]	now_pos		現在値
 *	@param[in]	vec			  移動量
 *	@param[in]	move_pos	移動先
 *	@param[out]	ret			  移動したかどうかを返す（FALSE:移動しない）
 */
//============================================================================================
void	BTLV_EFFTOOL_CheckVecMove( VecFx32 *now_pos, VecFx32 *vec, VecFx32 *move_pos, BOOL *ret )
{
  BOOL  ret_vec_x = TRUE;
  BOOL  ret_vec_y = TRUE;
  BOOL  ret_vec_z = TRUE;

  BTLV_EFFTOOL_CheckMove( &now_pos->x, &vec->x, &move_pos->x, &ret_vec_x );
  BTLV_EFFTOOL_CheckMove( &now_pos->y, &vec->y, &move_pos->y, &ret_vec_y );
  BTLV_EFFTOOL_CheckMove( &now_pos->z, &vec->z, &move_pos->z, &ret_vec_z );

  if( ret != NULL )
  { 
    *ret = ret_vec_x | ret_vec_y | ret_vec_z; 
  }
}

//============================================================================================
/**
 *	もらったパラメータで計算する
 *
 *	@param[in]	emw			移動計算パラメータ構造体へのポインタ
 *	@param[in]	now_value	現在値
 *
 *	@retval	FALSE:計算継続　TRUE:計算終了
 */
//============================================================================================
BOOL	BTLV_EFFTOOL_CalcParam( EFFTOOL_MOVE_WORK *emw, VecFx32 *now_value )
{
	BOOL	ret = TRUE;

	switch( emw->move_type ){
	case EFFTOOL_CALCTYPE_DIRECT:			//直接値を代入
		now_value->x = emw->end_value.x;
		now_value->y = emw->end_value.y;
		now_value->z = emw->end_value.z;
		break;
	case EFFTOOL_CALCTYPE_INTERPOLATION:	//指定値までを補間しながら計算
		if( emw->wait == 0 ){
			emw->wait = emw->wait_tmp;
			BTLV_EFFTOOL_CheckMove( &now_value->x, &emw->vector.x, &emw->end_value.x, &ret );
			BTLV_EFFTOOL_CheckMove( &now_value->y, &emw->vector.y, &emw->end_value.y, &ret );
			BTLV_EFFTOOL_CheckMove( &now_value->z, &emw->vector.z, &emw->end_value.z, &ret );
		}
		else{
			emw->wait--;
			ret = FALSE;
		}
		break;
	case EFFTOOL_CALCTYPE_ROUNDTRIP:		//指定した区間を往復移動
	case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG:	//指定した区間を往復移動
		if( emw->wait == 0 ){
			emw->wait = emw->wait_tmp;
			now_value->x += emw->vector.x;
			now_value->y += emw->vector.y;
			now_value->z += emw->vector.z;
			if( --emw->vec_time == 0 ){
				emw->count--;
 				emw->vec_time = emw->vec_time_tmp;
        if( ( emw->move_type == EFFTOOL_CALCTYPE_ROUNDTRIP ) ||
            ( ( emw->move_type == EFFTOOL_CALCTYPE_ROUNDTRIP_LONG ) && ( emw->count & 1 ) ) )
        { 
  				emw->vector.x *= -1;
  				emw->vector.y *= -1;
  				emw->vector.z *= -1;
        }
			}
		}
		else{
			emw->wait--;
		}
		if( emw->count ){
			ret = FALSE;
		}
		else{
			now_value->x = emw->start_value.x;
			now_value->y = emw->start_value.y;
			now_value->z = emw->start_value.z;
		}
		break;
	}
	return ret;
}

//============================================================================================
/**
 *	ポケモンの立ち位置をビットに変換
 *
 *	@param[in]	pos	ポケモンの立ち位置
 *
 *	@retval	変換したビット
 */
//============================================================================================
u32	BTLV_EFFTOOL_Pos2Bit( BtlvMcssPos pos )
{
	GF_ASSERT( pos < BTLV_MCSS_POS_TOTAL );

	return ( 1 << pos );
}

//============================================================================================
/**
 *	3Dモデルのパレットフェードアニメ
 *
 *	@param[in]	epfw  パレットフェード管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_EFFTOOL_CalcPaletteFade( EFFTOOL_PAL_FADE_WORK *epfw )
{ 

	if(	epfw->pal_fade_flag == 0 )
  { 
    return;
  }
	if( epfw->pal_fade_wait == 0 )
  { 
  	NNSG3dResFileHeader*	header;
  	NNSG3dResTex*		    	pTex;
    u32                   size;
    const void*           pData_src;
    u32                   from;
    int                   i;

	  epfw->pal_fade_wait = epfw->pal_fade_wait_tmp;

    for( i = 0 ; i < epfw->pal_fade_count ; i++ )
    { 
    	//テクスチャリソースポインタの取得
      header = GFL_G3D_GetResourceFileHeader( epfw->g3DRES[ i ] );
      pTex = NNS_G3dGetTex( header ); 
  
      size = ( u32 )pTex->plttInfo.sizePltt << 3;
      pData_src = NNS_G3dGetPlttData( pTex );
      from = NNS_GfdGetTexKeyAddr( pTex->plttInfo.vramKey );

      SoftFade( pData_src, epfw->pData_dst[ i ], ( size / 2 ), epfw->pal_fade_start_evy, epfw->pal_fade_rgb );

      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, from, epfw->pData_dst[ i ], size );
    }
  	if( epfw->pal_fade_start_evy == epfw->pal_fade_end_evy )
 		{	
 			epfw->pal_fade_flag = 0;
 		}
 		else if( epfw->pal_fade_start_evy > epfw->pal_fade_end_evy )
 		{	
 			epfw->pal_fade_start_evy--;
 		}
 		else
 		{	
 			epfw->pal_fade_start_evy++;
 		}
  }
  else
  { 
 	  epfw->pal_fade_wait--;
  }
}
