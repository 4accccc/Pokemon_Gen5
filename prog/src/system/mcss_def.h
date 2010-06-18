
//============================================================================================
/**
 * @file	mcss_def.h
 * @brief	マルチセルソフトウエアスプライト描画用関数
 * @author	soga
 * @date	2008.11.17
 */
//============================================================================================

//--------------------------------------------------------------------------
/**
 * 定数定義
 */
//--------------------------------------------------------------------------

#define SCREEN_WIDTH		(256)			// 画面幅
#define SCREEN_HEIGHT		(192)			// 画面高さ

#define SIZE_OF_RES_POOL	(1000)			// リソースプールのサイズ

#define	DEFAULT_Z_OFFSET	(-0x100)		//NNS_G2dSetRendererSpriteZoffsetに渡すセル描画毎に更新するZ値

#define	DOT_LENGTH			(FX32_HALF >> 5)	//セルデータ1ドットに対するポリゴンの長さの基準

//--------------------------------------------------------------------------
/**
 * 構造体定義
 */
//--------------------------------------------------------------------------
struct _MCSS_NCEC
{
	fx32	pos_x;		//セル描画X座標
	fx32	pos_y;		//セル描画Y座標
	fx32	size_x;		//セルサイズX
	fx32	size_y;		//セルサイズY
	fx32	tex_s;		//テクスチャs値
	fx32	tex_t;		//テクスチャt値

	//以下、メパチ用キャラ
	fx32	mepachi_pos_x;		//セル描画X座標
	fx32	mepachi_pos_y;		//セル描画Y座標
	fx32	mepachi_size_x;		//セルサイズX
	fx32	mepachi_size_y;		//セルサイズY
	fx32	mepachi_tex_s;		//テクスチャs値
	fx32	mepachi_tex_t;		//テクスチャt値

};

struct _MCSS_NCEC_WORK
{
	u32       cells;		//セル枚数
  u16       size_x;   //
  u16       size_y;
  s16       ofs_x;   //
  s16       ofs_y;
	MCSS_NCEC	ncec[1];	//セル情報（可変なので、1個分だけ確保）
};

struct _MCSS_NCEN_WORK
{
  u8    stop_cellanms;    //静止アニメでも動き続けるセルアニメの枚数
  u8    fly_flag;         //浮いているマルチセルかフラグ
  u8    stop_node[2];     //セルアニメノード情報（可変なので1個分だけ確保）
};

struct _MCSS_WORK
{
  GFL_TCB*                      tcb_load_resource;      //リソース読み込みtcb
  GFL_TCB*                      tcb_load_palette;       //リソース読み込みtcb
  GFL_TCB*                      tcb_load_base_palette;  //リソース読み込みtcb
	void*													mcss_ncer_buf;				//ファイルデータ読み込みバッファ
	NNSG2dCellDataBank*						mcss_ncer;						//ファイルデータ読み込みバッファからncerデータを抽出したデータ
	void*													mcss_nmcr_buf;				//ファイルデータ読み込みバッファ
	NNSG2dMultiCellDataBank*			mcss_nmcr;						//ファイルデータ読み込みバッファからnmcrデータを抽出したデータ
	void*													mcss_nanr_buf;				//ファイルデータ読み込みバッファ
	NNSG2dCellAnimBankData*				mcss_nanr;						//ファイルデータ読み込みバッファからnanrデータを抽出したデータ
	void*													mcss_nmar_buf;				//ファイルデータ読み込みバッファ
	NNSG2dMultiCellAnimBankData*	mcss_nmar;						//ファイルデータ読み込みバッファからnmarデータを抽出したデータ
	MCSS_NCEC_WORK*								mcss_ncec;						//1枚の板ポリで表示するための情報が格納された独自フォーマットデータ
  MCSS_NCEN_WORK*               mcss_ncen;            //静止アニメ用ノード情報
	NNSG2dMultiCellAnimation			mcss_mcanim;					//マルチセルアニメーションの実体
	void*													mcss_mcanim_buf;			//マルチセルアニメーションの実体の内部で使用するワーク領域
	NNSG2dImageProxy							mcss_image_proxy;			//テクスチャプロキシ
	NNSG2dImagePaletteProxy				mcss_palette_proxy;		//パレットプロキシ
	u16														*base_pltt_data;			//パレットデータ（パレットフェードのベースカラー）
	u16														*fade_pltt_data;			//パレットデータ（別のパレットフェードを適応してベースカラーにする）
	int														pltt_data_size;				//パレットデータサイズ
	VecFx32												pos;									//マルチセルのポジション
	VecFx32												scale;								//マルチセルのスケール
	VecFx32												rotate;								//マルチセルの回転
	VecFx32												shadow_scale;					//影のスケール
	VecFx32												ofs_pos;							//ポジションオフセット
	VecFx32												ofs_scale;						//スケールオフセット
	s8														pal_fade_start_evy;		//パレットフェード　START_EVY値
	s8														pal_fade_end_evy;			//パレットフェード　END_EVY値
	s8														pal_fade_wait;				//パレットフェード　wait値
	s8														pal_fade_wait_tmp;		//パレットフェード　wait_tmp値
	int														pal_fade_value;			  //パレットフェード　フェード値
	u32														pal_fade_rgb;					//パレットフェード　end_evy時のrgb値
	u32														alpha					      :8; //alpha値
	u32														mepachi_flag	      :1;	//メパチフラグ
	u32														anm_stop_flag	      :2;	//アニメストップフラグ
	u32														vanish_flag		      :1;	//バニッシュフラグ
	u32														pal_fade_flag	      :1;	//パレットフェードフラグ
	u32                           is_load_resource    :1; //リソース読み込み完了フラグ
  u32                           shadow_alpha        :9; //影アルファ(32で通常の半分を
	u32														shadow_vanish_flag  :1;    
	u32														mosaic				      :4; //モザイク    
	u32														fade_pltt_data_flag :1;    
	u32														ortho_mode          :1;    
	u32														reverse_draw        :1; //逆から描画    
	u32														                    :1;    
	int														index;								//登録INDEX
	int														heapID;								//使用するヒープID
  MCSS_ADD_WORK                 maw;

  fx32                          mcss_anm_frame;       //アニメーションを進めるフレーム数

  u16                           shadow_rotate;
  VecFx32                       shadow_offset;
};

struct _MCSS_SYS_WORK
{
  ARCHANDLE*              handle;               //リソースのARCHANDLE
  ARCID                   arcID;                //リソースのARCHANDLEをオープンしたARCID
  GFL_TCBSYS*             tcb_sys;              //リソース読み込みtcbsys（VBlank外での転送）
  GFL_TCB*                tcb_load_shadow;      //影リソース読み込みtcb
	int											mcss_max;							//登録数MAX
	MCSS_WORK**							mcss;									//登録マルチセル構造体
#ifdef USE_RENDER
	NNSG2dRendererInstance	mcss_render;					//描画用 Render　
	NNSG2dRenderSurface			mcss_surface;					//メイン画面 Surface
#endif //USE_RENDER
	NNSG2dImagePaletteProxy	shadow_palette_proxy;   	  //パレットプロキシ（影用）
  u32                     mcss_ortho_mode       :1;   //正射影描画モードフラグ
	u32                     perspective_far_flag  :1;   //透視射影FARフラグ
	u32                     ortho_far_flag        :1;   //正射影FARフラグ
  u32                                           :29;
	u32											texAdrs;							//テクスチャ転送開始アドレス
	u32											palAdrs;							//テクスチャパレット転送開始アドレス
  MCSS_CALLBACK_FUNC*     load_resource_callback;   //LoadResourceの前に呼ばれるコールバック（キャラデータ操作に使用）
  u32                     callback_work;        //コールバック関数を呼ぶ時の引数のポインタ
	int											heapID;								//使用するヒープID
	
	fx32                    mcAnimRate; //マルチセルアニメ倍率
  fx32                    scale_offset;
  fx32                    scale_offset_work;
};

