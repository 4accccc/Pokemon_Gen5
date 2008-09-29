
//--------------------------------------------------------------------------
/**
 * 定数定義
 */
//--------------------------------------------------------------------------

#define SCREEN_WIDTH		(256)	// 画面幅
#define SCREEN_HEIGHT		(192)	// 画面高さ

#define SIZE_OF_RES_POOL	(1000)	// リソースプールのサイズ

#define	DEFAULT_Z_OFFSET	(-0x100)	//NNS_G2dSetRendererSpriteZoffsetに渡すセル描画毎に更新するZ値

//--------------------------------------------------------------------------
/**
 * 構造体定義
 */
//--------------------------------------------------------------------------
typedef struct
{
	void						*mcss_ncer_buf;					//ファイルデータ読み込みバッファ
	NNSG2dCellDataBank			*mcss_ncer;						//ファイルデータ読み込みバッファからncerデータを抽出したデータ
	void						*mcss_nmcr_buf;					//ファイルデータ読み込みバッファ
	NNSG2dMultiCellDataBank		*mcss_nmcr;						//ファイルデータ読み込みバッファからnmcrデータを抽出したデータ
	void						*mcss_nanr_buf;					//ファイルデータ読み込みバッファ
	NNSG2dCellAnimBankData		*mcss_nanr;						//ファイルデータ読み込みバッファからnanrデータを抽出したデータ
	void						*mcss_nmar_buf;					//ファイルデータ読み込みバッファ
	NNSG2dMultiCellAnimBankData	*mcss_nmar;						//ファイルデータ読み込みバッファからnmarデータを抽出したデータ
	NNSG2dMultiCellAnimation	mcss_mcanim;					//マルチセルアニメーションの実体
	void						*mcss_mcanim_buf;				//マルチセルアニメーションの実体の内部で使用するワーク領域
	NNSG2dImageProxy			mcss_image_proxy;				//テクスチャプロキシ
	NNSG2dImagePaletteProxy		mcss_palette_proxy;				//パレットプロキシ
	VecFx32						pos;							//マルチセルのポジション
	int							index;							//登録INDEX
	int							heapID;							//使用するヒープID
}MCSS_WORK;

typedef struct
{
	int						max;				//登録数MAX
	MCSS_WORK				**mcss;				//登録マルチセル構造体
	NNSG2dRendererInstance	mcss_render;		//描画用 Render　
	NNSG2dRenderSurface		mcss_surface;		//メイン画面 Surface
	VecFx32					*camPos;			// カメラの位置(＝視点)
	VecFx32					*target;			// カメラの焦点(＝注視点)
	VecFx32					*camUp;				//
	int						heapID;				//使用するヒープID
}MCSS_SYS_WORK;

extern	MCSS_SYS_WORK*	MCSS_Init( int max, VecFx32 *camPos,VecFx32 *target, VecFx32 *camUp,HEAPID heapID );
extern	void	MCSS_Exit( MCSS_SYS_WORK *mcss_sys );
extern	void	MCSS_Main( MCSS_SYS_WORK *mcss_sys );
extern	void	MCSS_Draw( MCSS_SYS_WORK *mcss_sys );
extern	MCSS_WORK*	MCSS_Add( MCSS_SYS_WORK *mcss_sys,
					  fx32			pos_x,
					  fx32			pos_y,
					  fx32			pos_z,
					  ARCID			arcID,
					  ARCDATID		ncbr,
					  ARCDATID		nclr,
					  ARCDATID		ncer,
					  ARCDATID		nanr,
					  ARCDATID		nmcr,
					  ARCDATID		nmar );
extern	void	MCSS_Del( MCSS_SYS_WORK *mcss_sys, MCSS_WORK *mcss );
