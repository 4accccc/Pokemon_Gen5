//パレットフェード関連

//--------------------------------------------------
/**
 * @brief    フェード動作関数
 * @param   	pD2Fade				フェードワーク
 */
//--------------------------------------------------
static void _setFadeMask(_D2_PAL_FADE_WORK* pD2Fade)
{
  if(!pD2Fade){
    return;
  }
  if(pD2Fade->pal_fade_time < pD2Fade->pal_fade_nowcount){
    return;
  }
  pD2Fade->pal_fade_nowcount++;
  {
    int upper = pD2Fade->pal_end - pD2Fade->pal_start;
    upper *= 4096;
    upper = upper / pD2Fade->pal_fade_time;
    upper = upper * pD2Fade->pal_fade_nowcount;
    upper /= 4096;

    G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                           GX_BLEND_PLANEMASK_OBJ, upper);
    G2S_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3|
                            GX_BLEND_PLANEMASK_OBJ, upper);
  }

}

//--------------------------------------------------
/**
 * @brief パレットフェードスタート
 * @param[in]	bfw	      _EFFTOOL_PAL_FADE_WORK管理ワークへのポインタ
 * @param[in]	start_evy	セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in]	end_evy		セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in]	wait			セットするパラメータ（ウェイト）
 * @param[in]	rgb				セットするパラメータ（フェードさせる色）
 */
//--------------------------------------------------
static void	_FIELD_StartPaletteFade( _EFFTOOL_PAL_FADE_WORK* epfw, u8 start_evy, u8 end_evy, u8 wait, u16 rgb )
{

  epfw->pal_fade_flag      = 1;
  epfw->pal_fade_start_evy = start_evy;
  epfw->pal_fade_end_evy   = end_evy;
  epfw->pal_fade_wait      = 0;
  epfw->pal_fade_wait_tmp  = wait;
  epfw->pal_fade_rgb       = rgb;
}


//----------------------------------------
/**
 * @brief パレットフェード構造体作成
 * @param[in]	g3DRES	  GFL_G3D_RES
 * @param[in]	heapID	  HEAPID
 * @retturn   パレットフェード構造体
 */
//----------------------------------------

static _EFFTOOL_PAL_FADE_WORK* _createPaletteFade(GFL_G3D_RES* g3DRES,HEAPID heapID)
{
  //パレットフェード用ワーク生成
  NNSG3dResFileHeader*	header = GFL_G3D_GetResourceFileHeader( g3DRES );
  NNSG3dResTex*		    	pTex = NNS_G3dGetTex( header );
  u32                   size = (u32)pTex->plttInfo.sizePltt << 3;
  _EFFTOOL_PAL_FADE_WORK* pwk = GFL_HEAP_AllocClearMemory(heapID,sizeof(_EFFTOOL_PAL_FADE_WORK));

  pwk->g3DRES = g3DRES;
  pwk->pData_dst  = GFL_HEAP_AllocClearMemory( heapID, size );
  return pwk;
}

//----------------------------------------
/**
 * @brief パレットフェード構造体開放
 * @param[in]	pwk	_EFFTOOL_PAL_FADE_WORKポインタ
 */
//----------------------------------------
static void	_freePaletteFade( _EFFTOOL_PAL_FADE_WORK* pwk )
{
  if(pwk){
    GFL_HEAP_FreeMemory( pwk->pData_dst );
    GFL_HEAP_FreeMemory( pwk );
  }
}



//-------------------------------------------------
/**
 *	@brief      3Dモデルのパレットフェードアニメ実行処理   BTLVから転用
 *	@param[in]	epfw  パレットフェード管理構造体へのポインタ
 */
//-------------------------------------------------
static void  _EFFTOOL_CalcPaletteFade( _EFFTOOL_PAL_FADE_WORK *epfw )
{

  if(	(epfw == NULL) || (epfw->pal_fade_flag == 0) )
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

    epfw->pal_fade_wait = epfw->pal_fade_wait_tmp;

    {
      //テクスチャリソースポインタの取得
      header = GFL_G3D_GetResourceFileHeader( epfw->g3DRES );
      pTex = NNS_G3dGetTex( header );

      size = ( u32 )pTex->plttInfo.sizePltt << 3;
      pData_src = NNS_G3dGetPlttData( pTex );
      from = NNS_GfdGetTexKeyAddr( pTex->plttInfo.vramKey );

      SoftFade( pData_src, epfw->pData_dst, ( size / 2 ), epfw->pal_fade_start_evy, epfw->pal_fade_rgb );

      NNS_GfdRegisterNewVramTransferTask( NNS_GFD_DST_3D_TEX_PLTT, from, epfw->pData_dst, size );
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


