//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		field_gimmick_pkwd_field.c
 *	@brief  ポケウッド フィールド
 *	@author	Katsuhiko=Ichiraku
 *	@date		2011.08.25
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

// system
#include "fieldmap.h"
#include "system/main.h"    //for HEAPID_FIELDMAP
#include "savedata/gimmickwork.h"

// field
#include "field/field_const.h"  //for FIELD_CONST_GRID_FX32_SIZE

// gimmick
#include "field_gimmick_def.h"
#include "gmk_tmp_wk.h"

// arc
#include "arc/arc_def.h"
#include "arc/fieldmap/pkwd_field_gmk.naix" // 3Dモデル
#include "arc/pkwd_poster.naix"             // 看板のテクスチャデータ

// save
#include "savedata/pokewood_savedata.h"

// mine
#include "field_gimmick_pkwd_field.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
#define GIMMICK_UNIT_IDX        (0)
#define GIMMICK_TMP_ASSIGN_ID   (1)

// 看板系
#define POSTER_NUM    ( 4 )   // 映画館に存在する看板の総数

// 座標関係
#define GRID_HALF_SIZE  ((FIELD_CONST_GRID_SIZE/2)*FX32_ONE)

#define POSTER_1_X    ( 728 << FX32_SHIFT )
#define POSTER_1_Y    ( 16 << FX32_SHIFT )
#define POSTER_1_Z    ( 288 << FX32_SHIFT )

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	ギミック内部中の一時ワーク
//=====================================
typedef struct {
  FLD_EXP_OBJ_CNT_PTR exp_obj;        // 拡張3Dモデルコントローラ
  FIELDMAP_WORK       *p_fieldmap;    // フィールドマップ
} PKWD_FIELD_TMP;


//=============================================================================
/**
 *          3Dリソース
 */
//=============================================================================
// リソースの並び順
enum{
  RESID_POSTER_MDL,     // 看板モデル
};

// OBJインデックス
enum{
  OBJ_POSTER_1,         // 看板

  OBJ_NUM,
};

// RES
static const GFL_G3D_UTIL_RES res_3d_table[] =
{
  { ARCID_PKWD_FIELD_GIMMICK_DATA, NARC_pkwd_field_gmk_pokewood_poster_nsbmd, GFL_G3D_UTIL_RESARC },
};

// OBJ
static const GFL_G3D_UTIL_OBJ obj_3d_table[] =
{
  // OBJ_POSTER_1
  {
    RESID_POSTER_MDL,       // モデルリソースID
    0,                      // モデルデータID(リソース内部INDEX)
    RESID_POSTER_MDL,       // テクスチャリソースID
    NULL,                   // アニメテーブル(複数指定のため)
    0,                      // アニメリソース数
  },
};

// セットアップデータ
static const GFL_G3D_UTIL_SETUP g3Dutil_Setup = {
  res_3d_table,				    //リソーステーブル
	NELEMS(res_3d_table),		//リソース数
	obj_3d_table,				    //オブジェクト設定テーブル
	NELEMS(obj_3d_table),		//オブジェクト数
};


//-------------------------------------
/// テクスチャデータ	
//=====================================
typedef enum
{
  TEX_INDEX_1,
  TEX_INDEX_2,
  TEX_INDEX_3,
  TEX_INDEX_4,
}TEX_INDEX;
static char* tex_name[ POSTER_NUM ] =
{
  "c15_poster1",
  "c15_poster2",
  "c15_poster3",
  "c15_poster4",
};
static char* plt_name[ POSTER_NUM ] =
{
  "c15_poster1_pl",
  "c15_poster2_pl",
  "c15_poster3_pl",
  "c15_poster4_pl",
};
#define TEX_W   (8)   // テクスチャ幅(キャラ)
#define TEX_H   (8)   // テクスチャ幅(キャラ)

//=============================================================================
/**
 *					static関数宣言
*/
//=============================================================================

// EXPOBJ
static void EXPOBJ_ObjSet( PKWD_FIELD_TMP *p_wk );
static void EXPOBJ_ObjDell( PKWD_FIELD_TMP *p_wk );
static void EXPOBJ_ObjMain( PKWD_FIELD_TMP *p_wk );

// テクスチャ転送
static void POSTER_TexTrans( PKWD_FIELD_TMP *p_wk, const TEX_INDEX tex_index );
static void TexTransToGra( GFL_G3D_RES  *p_res,
                           const char   *tex_name,
                           const char   *plt_name,
                           const ARCDATID dat_id,
                           HEAPID heapID );
static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData);
static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData);
static void SetResName(NNSG3dResName *outName,const char *inName);
static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);
static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name);

//=============================================================================
/**
 *					外部公開関数
*/
//=============================================================================
//--------------------------------------------------------------
/**
 * @bried       セットアップ関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PKWD_FIELD_Setup( FIELDMAP_WORK *p_fieldmap )
{
  int i;

  // 汎用ワーク確保
  PKWD_FIELD_TMP *p_wk = GMK_TMP_WK_AllocWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID,
      FIELDMAP_GetHeapID(p_fieldmap), sizeof(PKWD_FIELD_TMP) );

  p_wk->exp_obj    = FIELDMAP_GetExpObjCntPtr( p_fieldmap );
  p_wk->p_fieldmap = p_fieldmap;

  // EXPOBJセット
  EXPOBJ_ObjSet( p_wk );

  // 4枚の看板にテクスチャを転送する
  for( i=0; i<POSTER_NUM; i++ )
  {
    // 看板にポスターのテクスチャを転送
    POSTER_TexTrans( p_wk, TEX_INDEX_1 + i );
  }
}

//--------------------------------------------------------------
/**
 * @bried       解放関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PKWD_FIELD_End( FIELDMAP_WORK *p_fieldmap )
{
  PKWD_FIELD_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

  // EXPOBJ破棄
  EXPOBJ_ObjDell( p_wk );

  //汎用ワーク解放
  GMK_TMP_WK_FreeWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );
}

//--------------------------------------------------------------
/**
 * @bried       動作関数
 * @param	      p_fieldmap     フィールドワークポインタ
 * @return      none
 */
//--------------------------------------------------------------
void PKWD_FIELD_Move( FIELDMAP_WORK *p_fieldmap )
{
  PKWD_FIELD_TMP *p_wk = GMK_TMP_WK_GetWork( p_fieldmap, GIMMICK_TMP_ASSIGN_ID );

   // EXPOBJアニメメイン
  EXPOBJ_ObjMain( p_wk );
}

//=============================================================================
/**
 *					static関数
*/
//=============================================================================
//=============================================================================
/**
 *          EXPOBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ設定
 *
 *	@param	PKWD_FIELD_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjSet( PKWD_FIELD_TMP *p_wk )
{
  //必要なリソースの用意
  FLD_EXP_OBJ_AddUnitByHandle( p_wk->exp_obj, &g3Dutil_Setup, GIMMICK_UNIT_IDX);

  //=====================================
  // ボタン
  {
    VecFx32 pos = { POSTER_1_X, POSTER_1_Y, POSTER_1_Z };

    int obj_idx = OBJ_POSTER_1;

    GFL_G3D_OBJSTATUS *status = FLD_EXP_OBJ_GetUnitObjStatus( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );

    // 座標
    VEC_Set( &status->trans, pos.x, pos.y, pos.z );

    // カリングする
    FLD_EXP_OBJ_SetCulling( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, TRUE );
    // 表示
    FLD_EXP_OBJ_SetVanish( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx, FALSE );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJ破棄
 *
 *	@param	PKWD_FIELD_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjDell( PKWD_FIELD_TMP *p_wk )
{
  FLD_EXP_OBJ_DelUnit( p_wk->exp_obj, GIMMICK_UNIT_IDX );
}
//----------------------------------------------------------------------------
/**
 *	@brief  3D拡張OBJメイン
 *
 *	@param	PKWD_FIELD_TMP *p_wk 
 */
//-----------------------------------------------------------------------------
static void EXPOBJ_ObjMain( PKWD_FIELD_TMP *p_wk )
{
  // アニメーション再生
  FLD_EXP_OBJ_PlayAnime( p_wk->exp_obj );
}


//----------------------------------------------------------------------------
/**
 *	@brief  看板にテクスチャを貼り付ける
 *
 *	@param	*p_wk       ギミックワーク
 *	@param	tex_index   テクスチャインデックス
 */
//-----------------------------------------------------------------------------
static void POSTER_TexTrans( PKWD_FIELD_TMP *p_wk, const TEX_INDEX tex_index )
{
  HEAPID heapID = GFL_HEAP_LOWID( FIELDMAP_GetHeapID(p_wk->p_fieldmap) );
  GAMESYS_WORK  *p_gsys = FIELDMAP_GetGameSysWork( p_wk->p_fieldmap );
  GAMEDATA      *p_gdata = GAMESYSTEM_GetGameData( p_gsys );
	POKEWOOD_SAVEDATA* pokewood_sv = GAMEDATA_GetPokewoodSavedata( p_gdata );
  u8            obj_idx = OBJ_POSTER_1;

  // OBJのリソース情報を取得
  {
    GFL_G3D_OBJ   *p_obj = FLD_EXP_OBJ_GetUnitObj( p_wk->exp_obj, GIMMICK_UNIT_IDX, obj_idx );
    GFL_G3D_RND   *p_rnd = GFL_G3D_OBJECT_GetG3Drnd( p_obj );
    GFL_G3D_RES   *p_res = GFL_G3D_RENDER_GetG3DresTex( p_rnd );

    int dat_id = POKEWOOD_SAVEDATA_GetKanbanNo( pokewood_sv, tex_index );
    
    ICHI_Printf("看板のNO %d \n", dat_id );

    // 0は何も上映していないのでSKIP
    if( dat_id > 0 )
    {
      dat_id -= 1;  // アーカイブデータは0オリジンなので-1する
      // テクスチャ転送
      TexTransToGra( p_res, tex_name[tex_index], plt_name[tex_index], dat_id, heapID );
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  テクスチャを転送する
 *
 *	@param	GFL_BMP_DATA *p_bmp   ビットマップデータ
 *	@param	GFL_G3D_RES  *p_res   オブジェのリソース
 *	@param  char *    tex_name    テクスチャ名
 *	@param  char *    plt_name    パレット名
 *	@param  ARCDATID  dat_id      ポスターの画像データのID
 *	@param  HEAPID heapID         ヒープ
 */
//-----------------------------------------------------------------------------
static void TexTransToGra( GFL_G3D_RES  *p_res,
                           const char   *tex_name,
                           const char   *plt_name,
                           const ARCDATID dat_id,
                           HEAPID heapID )
{
  GFL_BMP_DATA        *bmp_data;
  NNSG2dCharacterData *chr;
  NNSG2dPaletteData   *plt;
  void *chr_buf;
  void *pal_buf;

  u32 tex_adr;
  u32 pal_adr;
  u32 pal_size;

  const u8 cw = TEX_W;     // 切り抜いた幅
  const u8 ch = TEX_H;     // 切り抜いた幅
  const u8 ncg_cw = TEX_W;  // ncg画像の幅(キャラ単位)

  //リソースのＶＲＡＭアドレス取得
  {
    NNSG3dTexKey  texkey = GFL_G3D_GetTextureDataVramKey( p_res );
    NNSG3dPlttKey palkey = GFL_G3D_GetTexturePlttVramKey( p_res );
    {
      u8 * adr;
      NNSG3dResTex *resTex;
      NNSG3dResFileHeader* res_head = GFL_G3D_GetResourceFileHeader( p_res );
      resTex = NNS_G3dGetTex( res_head );
      adr = GetTexStartVRAMAdrByName(resTex, tex_name );
      tex_adr = (u32)adr;
      adr = GetPlttStartVRAMAdrByName(resTex, plt_name );
      pal_adr = (u32)adr;
      pal_size = (u32)resTex->plttInfo.sizePltt << 3;
    }

    ICHI_Printf("============================================\n");
    ICHI_Printf("txname = %s\n",tex_name);
    ICHI_Printf("plname = %s\n",plt_name);
    ICHI_Printf("key = %d %d\n",texkey, palkey);
    ICHI_Printf("adr = %x %x\n",tex_adr, pal_adr);
    ICHI_Printf("tex_size = %x \n", NNS_GfdGetTexKeySize(texkey));
    ICHI_Printf("pal_size = %x \n", pal_size);
    ICHI_Printf("============================================\n\n");
  }

  // 転送するテクスチャ読み込み
  {
    chr = NULL;
    plt = NULL;
    chr_buf = GFL_ARC_UTIL_LoadBGCharacter( ARCID_PKWD_POSTER, (dat_id*2+1), TRUE, &chr, heapID );
    pal_buf = GFL_ARC_UTIL_LoadPalette( ARCID_PKWD_POSTER, (dat_id*2), &plt, heapID );

    ICHI_Printf("chr w = %d h = %d szByte = %d ", chr->W,chr->H,chr->szByte);
    ICHI_Printf("pixelFmt=%d maping=%d charFmt=%d \n", chr->pixelFmt,chr->mapingType,chr->characterFmt);
    ICHI_Printf("plt szByte = %d \n", plt->szByte);
    
    bmp_data = GFL_BMP_CreateWithData( chr->pRawData, TEX_W, TEX_H, GFL_BMP_16_COLOR, heapID );

    // フォーマットデータをＢＭＰデータに変換
    GFL_BMP_DataConv_to_BMPformat( bmp_data, FALSE, heapID);
  }

  //転送
  {
    // VBlank転送ではなく、直転送に変更
    // テクスチャ転送
    GX_BeginLoadTex();
    DC_FlushRange( chr->pRawData, cw*ch*0x20 );
    GX_LoadTex( chr->pRawData, tex_adr, cw*ch*0x20 );
    GX_EndLoadTex();

    // パレットデータ転送
    GX_BeginLoadTexPltt();
    DC_FlushRange( plt->pRawData, 32 );
    GX_LoadTexPltt( plt->pRawData, pal_adr, 32 );
    GX_EndLoadTexPltt();
  }

  // 後始末
  GFL_HEAP_FreeMemory( chr_buf );
  GFL_HEAP_FreeMemory( pal_buf );
  GFL_BMP_Delete( bmp_data );
}

static void* GetTexStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictTexData* pData)
{
	// オフセット値
	u32 offset = (pData->texImageParam & NNS_G3D_TEXIMAGE_PARAM_TEX_ADDR_MASK)<<3;
	offset += NNS_GfdGetTexKeyAddr( inResTex->texInfo.vramKey );
	return (void*)(offset);

}

static void* GetPlttStartVRAMAdrSub(NNSG3dResTex *inResTex,const NNSG3dResDictPlttData* pData)
{
	// オフセット値
	u32 offset = pData->offset<<3;
	offset += NNS_GfdGetPlttKeyAddr( inResTex->plttInfo.vramKey );
	return (void*)(offset);

}

static void SetResName(NNSG3dResName *outName,const char *inName)
{
	u8 i;
  int len = STD_StrLen(inName);

	//初期化
	for (i=0;i<4;i++){
		outName->val[i] = 0;
	}
	for (i=0;i<len;i++){
		outName->name[i] = inName[i];
	}
}

static void* GetTexStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictTexData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetTexDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetTexStartVRAMAdrSub(inResTex, pData);
}

static void* GetPlttStartVRAMAdrByName(NNSG3dResTex *inResTex, const char *name)
{
	NNSG3dResName tmpResName;
	const NNSG3dResDictPlttData* pData;

	//テクスチャの実体をサーチ
	SetResName(&tmpResName, name);	//文字列をNNSG3dResNameに変換
	pData = NNS_G3dGetPlttDataByName( inResTex, &tmpResName );

	if (pData == NULL){
		return NULL;
	}

	//アドレスを算出
	return GetPlttStartVRAMAdrSub(inResTex, pData);
}

