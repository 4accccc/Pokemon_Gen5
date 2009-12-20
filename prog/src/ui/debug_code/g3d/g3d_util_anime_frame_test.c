//=============================================================================
/**
 *
 *	@file		mcss_pos_test.c
 *	@brief  GFL_G3D_UTILを用いたアニメーションフレーム指定テストコード
 *	@author	hosaka genya
 *	@data		2009.12.18
 *
 *	<コード説明>
 *	毎ループ呼ぶことでアニメーションフレームを操作できます。
 *
 *	<操作説明>
 *　  ←→：フレーム値操作(1.0ずつ)
 */
//=============================================================================

static void debug_g3d_util_anime_frame_test( GFL_G3D_UTIL* util, int unit_id )
{
  
  int j;
  int anime_count;
  GFL_G3D_OBJ* obj;
  static int frame;
  
  obj = GFL_G3D_UTIL_GetObjHandle( util, unit_id );
  anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );

  if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )
  {
    frame += FX32_ONE;
    OS_TPrintf("frame=%d \n",frame>>FX32_SHIFT);
  }
  else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT )
  {
    frame -= FX32_ONE;
    OS_TPrintf("frame=%d \n",frame>>FX32_SHIFT);
  }

  for( j=0; j<anime_count; j++ )
  {
    GFL_G3D_OBJECT_SetAnimeFrame( obj, j, &frame );
  }
}

