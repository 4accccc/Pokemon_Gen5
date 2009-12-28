//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_storm.c
 *	@brief		砂嵐
 *	@author		tomoya takahshi
 *	@date		2009.03.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"
#include "arc/field_weather_light.naix"


#include "weather_storm.h"

//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
//	砂嵐管理関数定数
//=====================================
#define	WEATHER_STORM_TIMING_MIN		(2)							// 砂を出すタイミング最小
#define WEATHER_STORM_TIMING_MAX		(15)						// 砂を出すタイミング最大
#define WEATHER_STORM_TIMING_ADD		(2)							// タイミングを減らす数
#define WEATHER_STORM_TIMING_ADD_END	(1)							// タイミングを減らす数
#define WEATHER_STORM_ADD_START			(1)							// 最初の同時に雨を登録する数
#define WEATHER_STORM_ADD_TIMING		(4)							// 砂のタイミングをこれ回変更したら１回増やす
#define WEATHER_STORM_ADD				(1)							// 登録する数を増やす数
#define WEATHER_STORM_ADD_END			(-1)							// 登録する数を増やす数
#define	WEATHER_STORM_ADD_MAIN			(1)							// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_STORM_NOFADE_OBJ_START_NUM	( 24 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_STORM_NOFADE_OBJ_START_DUST_NUM		( 2 )			// 何個ずつずらして動作させるか
#define	WEATHER_STORM_NOFADE_OBJ_START_DUST_MOVE	( 2 )			// １ずつずらして動作させる

/*== フォグ ==*/
#define	WEATHER_STORM_FOG_TIMING		(200)							// に１回フォグテーブルを操作
#define	WEATHER_STORM_FOG_TIMING_END	(200)							// に１回フォグテーブルを操作
#define WEATHER_STORM_FOG_START			(1)							// このカウント動いてからフォグテーブルを操作
#define WEATHER_STORM_FOG_START_END		(31)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_STORM_FOG_OFS	( 0x100 )
#define WEATHER_STORM_FOG_SLOPE	( 4 )


/*== BG ==*/
#define WEATHER_STORM_BG_ALP_TIM_S	(32)		// アルファを１上げるタイミング
#define WEATHER_STORM_BG_ALP_TIM_E	(24)		// アルファを１下げるタイミング
#define WEATHER_STORM_BG_ALP_S_END	( 3 )
#define WEATHER_STORM_BG_ALP_E_END	( 0 )
#define WEATHER_STORM_BG_ALP_DEF	( 16 )

#define WEATHER_STORM_BG_ALP_STRONG_WIND	(4)	// すごい風のときのα値


/*== 砂オブジェクト ==*/
#define	WEATHER_STORM_END_MIN		(15)						// 終了カウンタ最小
#define WEATHER_STORM_END_MAX		(35)						// 終了カウンタ最大
#define WEATHER_STORM_END_NUM		(WEATHER_STORM_END_MAX - WEATHER_STORM_END_MIN)	// カウンタの値の数
#define	WEATHER_STORM_END_DIV		((WEATHER_STORM_END_NUM / 4)+1)	// オブジェクトの種類を計算するとき割る値
#define	WEATHER_STORM_START_X1		(262)						// ベースになるX開始座標	よこ用
#define	WEATHER_STORM_START_X1_RAN	(24)						// ベースになるX開始座標	よこ用
#define WEATHER_STORM_START_Y1_MIN	( -64 )						//  Y開始座標最小
#define	WEATHER_STORM_START_Y1_MAX	(192)						// Y開始座標最大
#define	WEATHER_STORM_SPEED_NUM		(8)							// 風向きの数
#define	WEATHER_STORM_SPEED_CNG_NUM	(40)						// 変えるタイミング
#define WEATHER_STORM_SPEED_ADD_TMG (5)						// 風の勢いを変えるタイミング

#define WEATHER_STORM_STRONG_CHG	(6)						// 風がこの値以上になった時勢いを強くする

static const int STORM_SPEED_X_TBL[WEATHER_STORM_SPEED_NUM] = {-6,-10,-10,-6,-10,-12,-20,-12};
static const int STORM_SPEED_Y_TBL[WEATHER_STORM_SPEED_NUM] = { 4, 4, 4, 8, 8, 4, 4, 4};	

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雪ワーク
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_STORM_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------



//-------------------------------------
///	砂嵐
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_STORM_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_STORM_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 


static void WEATHER_STORM_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_STORM_WORK* p_wk );


//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 砂嵐
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_STORM = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	TRUE,		// BGを使用するか？
	NARC_field_weather_storm_NCGR,			// OAM CG
	NARC_field_weather_storm_NCLR,			// OAM PLTT
	NARC_field_weather_storm_NCER,			// OAM CELL
	NARC_field_weather_storm_NANR,			// OAM CELLANM
	NARC_field_weather_storm_nsbtx,		// BGTEX
	GX_TEXSIZE_S32,		// GXTexSizeS
	GX_TEXSIZE_T32,		// GXTexSizeT
	GX_TEXREPEAT_ST,		// GXTexRepeat
	GX_TEXFLIP_NONE,		// GXTexFlip
	GX_TEXFMT_PLTT4,		// GXTexFmt
	GX_TEXPLTTCOLOR0_TRNS,		// GXTexPlttColor0

	// ワークサイズ
	sizeof(WEATHER_STORM_WORK),

	// 管理関数
	WEATHER_STORM_Init,		// 初期化
	WEATHER_STORM_FadeIn,		// フェードイン
	WEATHER_STORM_NoFade,		// フェードなし
	WEATHER_STORM_Main,		// メイン処理
	WEATHER_STORM_InitFadeOut,	// フェードアウト
	WEATHER_STORM_FadeOut,		// フェードアウト
	WEATHER_STORM_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_STORM_OBJ_Move,
};




//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_STORM_ADD_START,	// obj登録数
		WEATHER_STORM_TIMING_MAX,	// 登録タイミング
		WEATHER_STORM_ADD_MAIN,
		WEATHER_STORM_TIMING_MIN,
		-WEATHER_STORM_TIMING_ADD,
		WEATHER_STORM_ADD_TIMING,
		WEATHER_STORM_ADD,
		WEATHER_STORM_OBJ_Add );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_STORM_FOG_SLOPE, 
				WEATHER_FOG_DEPTH_DEFAULT_START, fog_cont );



	p_local_wk->work[0] = WEATHER_STORM_FOG_START;	// 同じくフォグ用
	p_local_wk->work[1] = 0;		// オブジェクト追加数カウンタ
	p_local_wk->work[2] = 0;		// BGカウンタ
	p_local_wk->work[3] = 0;		// 風のスピード

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

  // SE
  WEATHER_TASK_PlayLoopSnd( p_wk, WEATHER_SND_SE_STORM );
	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;


	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){
			WEATHER_TASK_FogFadeIn_Init( p_wk,
			WEATHER_STORM_FOG_SLOPE, 
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_STORM_FOG_OFS, 
			WEATHER_STORM_FOG_TIMING,
			fog_cont );


      // ライト変更
      WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_storm_dat, heapID );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ
			// BGON
			WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );

			// シーケンス変更
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	WEATHER_STORM_SCROLL_Main( p_wk, p_local_wk );


	return WEATHER_TASK_FUNC_RESULT_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_STORM_ADD_MAIN,	// obj登録数
		WEATHER_STORM_TIMING_MIN,	// 登録タイミング
		WEATHER_STORM_ADD_MAIN,
		WEATHER_STORM_TIMING_MIN,
		-WEATHER_STORM_TIMING_ADD,
		WEATHER_STORM_ADD_TIMING,
		WEATHER_STORM_ADD,
		WEATHER_STORM_OBJ_Add );


	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_STORM_FOG_SLOPE, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_STORM_FOG_OFS, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_STORM_OBJ_Add, WEATHER_STORM_NOFADE_OBJ_START_NUM, WEATHER_STORM_NOFADE_OBJ_START_DUST_NUM, WEATHER_STORM_NOFADE_OBJ_START_DUST_MOVE, heapID );

	
	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, TRUE );


  // ライト変更
  WEATHER_TASK_LIGHT_Set( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_storm_dat, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	u32 wind;
	
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	if( WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID ) ){

		// 風テーブルを取得
		wind = p_local_wk->work[1] / WEATHER_STORM_SPEED_CNG_NUM;
		// 風を設定する
		p_local_wk->work[3] = STORM_SPEED_X_TBL[wind];
		
		// α値とオブジェの出す量を風の強さで変える
		if( p_local_wk->work[3] <= -WEATHER_STORM_STRONG_CHG ){
			// 砂嵐登録
			WEATHER_STORM_OBJ_Add( p_wk, WEATHER_STORM_ADD_MAIN, heapID );
		
		}
	}

	// スクロール処理
	WEATHER_STORM_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_STORM_TIMING_MAX,
			WEATHER_STORM_TIMING_ADD,
			WEATHER_STORM_ADD_END );
	
	// フォグ
	p_local_wk->work[0] = WEATHER_STORM_FOG_START_END;	// 同じくフォグ用


	// BGON
	WEATHER_TASK_3DBG_SetVisible( p_wk, FALSE );


  // SE
  WEATHER_TASK_StopLoopSnd( p_wk );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;

	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// オブジェクトフェード
	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	// フォグ操作
	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワークが０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){

			WEATHER_TASK_FogFadeOut_Init( p_wk,
					WEATHER_FOG_DEPTH_DEFAULT_START, 
					WEATHER_STORM_FOG_TIMING_END, fog_cont );
		}
	}else{
	
		if( fog_cont ){
			fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		}else{
			fog_result = TRUE;
		}
	
		if( fog_result && result ){
			// 登録数が０になったら終了するかチェック
			// 自分の管理するあめが全て破棄されたら終了
			if( WEATHER_TASK_GetActiveObjNum( p_wk ) == 0 ){//*/
				
				return WEATHER_TASK_FUNC_RESULT_FINISH;
			}
		}
	}

	// スクロール処理
	WEATHER_STORM_SCROLL_Main( p_wk, p_local_wk );

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_STORM_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_STORM_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// FOG終了
	WEATHER_TASK_FogClear( p_wk, fog_cont );

	// ライト元に
	WEATHER_TASK_LIGHT_Back( p_wk, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ動作
 */
//-----------------------------------------------------------------------------
static void WEATHER_STORM_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	GFL_CLACTPOS mat;
	GFL_CLWK* p_clwk;
	s32* obj_w;		// オブジェワーク

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	WEATHER_OBJ_WORK_GetPos( p_wk, &mat );
	
	// 動作フラグをチェック
	if(!obj_w[3]){
		// 動かす
		mat.x += obj_w[4];
		mat.y += obj_w[2];

		if((obj_w[0] % WEATHER_STORM_SPEED_ADD_TMG) == 0){
			obj_w[4] += obj_w[5];
		}

		// 破棄するかチェック
		if(obj_w[0]++ > obj_w[1]){
			// 破棄
			obj_w[3] = 1;
		}

		WEATHER_OBJ_WORK_SetPos( p_wk, &mat );
	}else{
		WEATHER_TASK_DeleteObj( p_wk );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_STORM_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	int	wind;	// 風向き
	int	rand;
	WEATHER_STORM_WORK* sys_w;	// システムワーク
	s32* obj_w;	// オブジェワーク
	int frame;	// フレーム数
	GFL_CLWK* p_clwk;
	GFL_CLACTPOS mat;
	
	sys_w = WEATHER_TASK_GetWorkData( p_wk );

	
	
	// 風変更カウント
	sys_w->work[1] = (sys_w->work[1]+1) % (WEATHER_STORM_SPEED_CNG_NUM*WEATHER_STORM_SPEED_NUM);
		
	// 風向きを求める
	wind = sys_w->work[1] / WEATHER_STORM_SPEED_CNG_NUM;
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}

		// オブジェワーク
		obj_w = WEATHER_OBJ_WORK_GetWork( add_obj );
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );
		
		
		// 領域を初期化
		obj_w[0] = 0;			// カウンタ
		obj_w[1] = WEATHER_STORM_END_MIN + GFUser_GetPublicRand(WEATHER_STORM_END_MAX - WEATHER_STORM_END_MIN);	// 終了カウンタ
		
		frame = 3 - ((obj_w[1] - WEATHER_STORM_END_MIN) / WEATHER_STORM_END_DIV);	// 吹雪の種類
		
		obj_w[2] = (STORM_SPEED_Y_TBL[wind]) * (frame+1);
		obj_w[4] = (STORM_SPEED_X_TBL[wind]) * (frame+1);
		obj_w[3] = 0;			// 破棄アニメフラグ
		obj_w[5] = STORM_SPEED_X_TBL[wind];	// スピードを上げていく値

		// ほんとにたまにサボテン
		rand = GFUser_GetPublicRand(1000);
		if(rand == 777){
			frame = 4;
			obj_w[2] += obj_w[2] / 2;
		}
		
		GFL_CLACT_WK_SetAnmIndex( p_clwk, frame );
		
		mat.x = WEATHER_STORM_START_X1 + GFUser_GetPublicRand(WEATHER_STORM_START_X1_RAN);
		mat.y = WEATHER_STORM_START_Y1_MIN + GFUser_GetPublicRand(WEATHER_STORM_START_Y1_MAX);

		// 座標設定
		WEATHER_OBJ_WORK_SetPos( add_obj, &mat );
	}
}



//----------------------------------------------------------------------------
/**
 *	@brief	スクロール処理
 *
 *	@param	p_sys	システムワーク
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void WEATHER_STORM_SCROLL_Main( WEATHER_TASK* p_sys, WEATHER_STORM_WORK* p_wk )
{
	int x, y;
	WEATHER_TASK_GetScrollDist( p_sys, &x, &y );
	WEATHER_TASK_ScrollObj( p_sys, x, y );

	// BG面を斜め上に動かす
	p_wk->work[2] = (p_wk->work[2] + 6) % 256;
	WEATHER_TASK_3DBG_SetScrollX( p_sys, (p_wk->work[2]*2) - x );
	WEATHER_TASK_3DBG_SetScrollY( p_sys, (-p_wk->work[2]*2) + y );
}
