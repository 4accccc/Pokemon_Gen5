//=============================================================================================
/**
 * @file	g3d_system.c                                                  
 * @brief	３Ｄ描画管理システムプログラム
 * @date	2006/4/26
 */
//=============================================================================================
#include "gflib.h"
#include "g3d_system.h"

//=============================================================================================
//	型宣言
//=============================================================================================
#define TEX_BLOCKNUM	(128)
#define PLT_BLOCKNUM	(256)

#define TEX_SLOTSIZ		(0x20000)
#define PLT_SLOTSIZ		(0x2000)
//=============================================================================================
//	プロトタイプ宣言
//=============================================================================================
//--------------------------------------------------------------------------------------------
/**
 * システムマネージャ設定
 *
 * @param	heapID			メモリ取得用ヒープエリア
 * @param	texmanMode		テクスチャマネージャ使用モード
 * @param	texmanSize		テクスチャマネージャ使用領域サイズ（byte）
 * @param	palmanMode		パレットマネージャ使用モード
 * @param	palmanSize		パレットマネージャ使用領域サイズ（byte）
 * @param	setUp			セットアップ関数(NULLの時はDefaultSetUp)
 *
 * @return	マネージャワークポインタ
 */
//--------------------------------------------------------------------------------------------
GF_G3DMAN*
	GFL_G3D_sysInit
		( u32 heapID, GF_G3DMAN_MODE texmanMode, GF_G3DMAN_TEXSIZE texmanSize, 
			GF_G3DMAN_MODE pltmanMode, GF_G3DMAN_PLTSIZE pltmanSize, GF_G3DSETUP_FUNC setup )
{
	int	tex_size,plt_size;
	GF_G3DMAN* man = GFL_HEAP_AllocMemory( heapID, sizeof(GF_G3DMAN) );

	man->heapID = heapID;

	// NitroSystem:３Ｄエンジンの初期化
	NNS_G3dInit();
	// マトリクススタックの初期化
    G3X_InitMtxStack();

	// ジオメトリエンジン起動後必ず呼ばれなければならない
    G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	//ＶＲＡＭテクスチャマネージャの設定
	if( texmanMode == GF_G3DMAN_LNK ){
		//リンクドモードの設定
		tex_size = NNS_GfdGetLnkTexVramManagerWorkSize( TEX_BLOCKNUM * texmanSize );
		man->tex_memory = GFL_HEAP_AllocMemory( man->heapID, tex_size );

		// マネージャがテクスチャイメージスロットを指定スロット分管理できるようにする
		NNS_GfdInitLnkTexVramManager( TEX_SLOTSIZ*texmanSize, 0, man->tex_memory, tex_size, TRUE);
	} else {
		//フレームモードの設定
		NNS_GfdInitFrmTexVramManager( texmanSize, TRUE);
	}

	//ＶＲＡＭパレットマネージャの設定
	if( pltmanMode == GF_G3DMAN_LNK ){
		//リンクドモードの設定
		plt_size = NNS_GfdGetLnkPlttVramManagerWorkSize( PLT_BLOCKNUM * pltmanSize );
		man->plt_memory = GFL_HEAP_AllocMemory( man->heapID, plt_size );

		// マネージャがテクスチャイメージスロットを指定サイズ分管理できるようにする
		NNS_GfdInitLnkPlttVramManager( PLT_SLOTSIZ * pltmanSize, man->plt_memory, plt_size, TRUE);
	} else {
		//フレームモードの設定
		NNS_GfdInitFrmTexVramManager( PLT_SLOTSIZ * pltmanSize, TRUE);
	}

	if( setup != NULL ){
		setup();
	} else {	//各種モードデフォルトセットアップ
		// ３Ｄ使用面の設定(表示＆プライオリティー)
		//GF_Disp_GX_VisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
		//G2_SetBG0Priority(0);

		// 各種描画モードの設定(シェード＆アンチエイリアス＆半透明)
		G3X_SetShading( GX_SHADING_TOON );
		G3X_AntiAlias( FALSE );
		G3X_AlphaTest( FALSE, 0 );	// アルファテスト　　オフ
		G3X_AlphaBlend( FALSE );		// アルファブレンド　オン
		G3X_EdgeMarking( FALSE );
		G3X_SetFog( FALSE, GX_FOGBLEND_COLOR_ALPHA, GX_FOGSLOPE_0x8000, 0 );

		// クリアカラーの設定
		G3X_SetClearColor(GX_RGB(0,0,0),0,0x7fff,63,FALSE);	//color,alpha,depth,polygonID,fog

		// ビューポートの設定
		G3_ViewPort(0, 0, 255, 191);
	}
	return man;
}


//--------------------------------------------------------------------------------------------
/**
 * 終了
 *
 * @param	man		マネージャワークポインタ
 *
 * @return	none
 *
 */
//--------------------------------------------------------------------------------------------
void
	GFL_G3D_sysExit
		( GF_G3DMAN*  man )
{
	GFL_HEAP_FreeMemory( man->plt_memory );
	GFL_HEAP_FreeMemory( man->tex_memory );

	GFL_HEAP_FreeMemory( man );
}


//------------------------------------------------------------------
//  高速３ＤＢＧ描画関数
//  
//  @param		object_p		レンダーＯＢＪ
//  @param		trans_p			トランスレート
//  @param		rot_p			ローテーション
//  @param		scale_p			スケール
//
//  @return		none
//  
//------------------------------------------------------------------
void
	GFL_G3D_Draw1mat1shape
		(NNSG3dRenderObj* object_p, VecFx32* trans_p, MtxFx33* rot_p, VecFx32* scale_p)
{
	// 位置設定
	NNS_G3dGlbSetBaseTrans(trans_p);
	// 角度設定
	NNS_G3dGlbSetBaseRot(rot_p);
	// スケール設定
	NNS_G3dGlbSetBaseScale(scale_p);

	NNS_G3dGlbFlush();
    NNS_G3dDraw1Mat1Shp(object_p->resMdl,0,0,TRUE);
    NNS_G3dGeFlushBuffer();
}

