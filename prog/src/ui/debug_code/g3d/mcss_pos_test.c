//=============================================================================
/**
 *
 *	@file		mcss_pos_test.c
 *	@brief  MCSS_の座標調整用テストコード
 *	@author	hosaka genya
 *	@data		2009.12.18
 *
 *	<コード説明>
 *	毎ループ呼ぶことでMCSSの座標調整ができます。コピペしてお使いください。
 *
 *	<操作説明>
 *	  XY：座標の変化量を変更
 *	  LR：z+,z-;
 *    ↑↓：y+,y-;
 *　  ←→：x+,x-;
 */
//=============================================================================

#ifdef PM_DEBUG

#define CHECK_KEY_CONT( key ) ( (GFL_UI_KEY_GetCont() & (key) ) == (key) )

static void debug_mcss_camera( MCSS_WORK* mcss_work )
{
  VecFx32 pos;
  static int num = 1;

  GF_ASSERT( mcss_work );
    
  MCSS_GetPosition( mcss_work, &pos );

  if( CHECK_KEY_CONT( PAD_BUTTON_X ) )
  {
    num++;
    HOSAKA_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_Y ) )
  {
    num--;
    HOSAKA_Printf("num=%d \n",num);
  }
  else if( CHECK_KEY_CONT( PAD_KEY_UP ) )
  {
    pos.y += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_DOWN ) )
  {
    pos.y -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }    
  else if( CHECK_KEY_CONT( PAD_KEY_LEFT ) )
  {
    pos.x += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_KEY_RIGHT ) )
  {
    pos.x -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_L ) )
  {
    pos.z += num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  else if( CHECK_KEY_CONT( PAD_BUTTON_R ) )
  {
    pos.z -= num;
    HOSAKA_Printf("pos{ 0x%x, 0x%x, 0x%x } \n", pos.x, pos.y, pos.z );
  }
  
  MCSS_SetPosition( mcss_work, &pos );
}


#endif
