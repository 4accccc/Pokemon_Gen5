//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		weather_rain.c
 *	@brief		天気雨
 *	@author		tomoya takahashi
 *	@data		2009.03.27
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "arc/arc_def.h"
#include "arc/field_weather.naix"
#include "arc/field_weather_light.naix"


#include "weather_rain.h"

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
//	雨管理関数定数
//=====================================
#define	WEATHER_RAIN_TIMING_MIN		(0)							// 雨を出すタイミング最小
#define WEATHER_RAIN_TIMING_MAX		(8)						// 雨を出すタイミング最大
#define WEATHER_RAIN_TIMING_ADD		(1)							// タイミングを減らす数
#define WEATHER_RAIN_ADD_START		(1)							// 最初の同時に雨を登録する数
#define WEATHER_RAIN_ADD_TIMING		(1)							// 雨のタイミングをこれ回変更したら１回増やす
#define WEATHER_RAIN_ADD			(1)							// 登録する数を増やす数
#define	WEATHER_RAIN_ADD_MAIN		(2)							// メインシーケンスでの登録する数

/*== フェード無し開始の時 ==*/
#define WEATHER_RAIN_NOFADE_OBJ_START_NUM	( 20 )				// 開始時の散布するオブジェクトの数
#define	WEATHER_RAIN_NOFADE_OBJ_START_DUST_NUM	( 10 )			// 何個単位で変化させるか
#define	WEATHER_RAIN_NOFADE_OBJ_START_DUST_MOVE	( 1 )			// ずらして動作させる値

/*== フォグ ==*/
#define	WEATHER_RAIN_FOG_TIMING		(300)							// に１回フォグテーブルを操作
#define	WEATHER_RAIN_FOG_TIMING_END	(300)							// に１回フォグテーブルを操作
#define WEATHER_RAIN_FOG_START		(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_RAIN_FOG_START_END	(1)						// このカウント動いてからフォグテーブルを操作
#define WEATHER_RAIN_FOG_OFS		(0x300)
#define WEATHER_RAIN_FOG_OFS_START	(0x800)

/*== 雨オブジェクト ==*/
#define WEATHER_RAIN_SPEED_X		(-5)						// 横に進むスピード
#define WEATHER_RAIN_SPEED_Y		(10)						// たてに進むスピードベース
#define	WEATHER_RAIN_END_MIN		(1)							// 終了カウンタ最小
#define WEATHER_RAIN_END_MAX		(3)							// 終了カウンタ最大
#define	WEATHER_RAIN_START_X_BASE	(0)							// ベースになるX開始座標
#define	WEATHER_RAIN_MUL_X			(15)						// ベースに雨の種類分足す値
#define	WEATHER_RAIN_START_X_MAX	(270)						// X開始座標乱数の最大値
#define	WEATHER_RAIN_START_Y		(-96)						// Y開始座標
#define	WEATHER_RAIN_SPEED_ERR		(20)						// スピード補正値


//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	普通の雨
//=====================================
typedef struct {
	s32 work[10];
} WEATHER_RAIN_WORK;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	通常の雨
//=====================================
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID ); 
static void WEATHER_RAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk ); 
static void WEATHER_RAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID ); 



//-----------------------------------------------------------------------------
/**
 *			天気データ
 */
//-----------------------------------------------------------------------------
// 通常の雨
WEATHER_TASK_DATA c_WEATHER_TASK_DATA_RAIN = {
	//	グラフィック情報
	ARCID_FIELD_WEATHER,			// アークID
	TRUE,		// OAMを使用するか？
	FALSE,		// BGを使用するか？
	NARC_field_weather_rain_NCGR,			// OAM CG
	NARC_field_weather_rain_NCLR,			// OAM PLTT
	NARC_field_weather_rain_NCER,			// OAM CELL
	NARC_field_weather_rain_NANR,			// OAM CELLANM
	0,			// BG CG
	0,			// BG PLTT
	0,			// BG SCRN

	// ワークサイズ
	sizeof(WEATHER_RAIN_WORK),

	// 管理関数
	WEATHER_RAIN_Init,		// 初期化
	WEATHER_RAIN_FadeIn,		// フェードイン
	WEATHER_RAIN_NoFade,		// フェードなし
	WEATHER_RAIN_Main,		// メイン処理
	WEATHER_RAIN_InitFadeOut,	// フェードアウト
	WEATHER_RAIN_FadeOut,		// フェードアウト
	WEATHER_RAIN_Exit,		// 破棄

	// オブジェ動作関数
	WEATHER_RAIN_OBJ_Move,
};






//----------------------------------------------------------------------------
/**
 *	@brief	初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Init( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_RAIN_ADD_START,
		WEATHER_RAIN_TIMING_MAX,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		-WEATHER_RAIN_TIMING_ADD,
		WEATHER_RAIN_ADD_TIMING,
		WEATHER_RAIN_ADD,
		WEATHER_RAIN_OBJ_Add );

	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS_START, fog_cont );



	p_local_wk->work[0] = WEATHER_RAIN_FOG_START;	// 同じくフォグ用

	// スクロール処理の初期化
	WEATHER_TASK_InitScrollDist( p_wk );

	// 音
//	WeatherLoopSndPlay( sys_work, SEQ_SE_DP_T_AME );	

	// ライト変更
	WEATHER_TASK_LIGHT_Change( p_wk, ARCID_FIELD_WEATHER_LIGHT, NARC_field_weather_light_light_rain_dat, heapID );
	

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードイン
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeIn( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	BOOL result;
	BOOL fog_result;


	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );


	result = WEATHER_TASK_ObjFade_Main( p_wk, heapID );	// 実行

	if(p_local_wk->work[0] > 0){
		p_local_wk->work[0]--;			// ワーク6が０になったらフォグを動かす
		if( p_local_wk->work[0] == 0 ){
			WEATHER_TASK_FogFadeIn_Init( p_wk,
			WEATHER_FOG_SLOPE_DEFAULT, 
			WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS, 
			WEATHER_RAIN_FOG_TIMING,
			fog_cont );
		}
	}else{
		
		fog_result = WEATHER_TASK_FogFade_IsFade( p_wk );
		
		// タイミングが最小になったらメインへ
		if( fog_result && result ){		// フェードリザルトが完了ならばメインへ
			// シーケンス変更
			return WEATHER_TASK_FUNC_RESULT_FINISH;
		}
	}

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;

}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードなし
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_NoFade( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// 作業領域の初期化
	// オブジェクトフェード用
	WEATHER_TASK_ObjFade_Init( p_wk,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		WEATHER_RAIN_ADD_MAIN,
		WEATHER_RAIN_TIMING_MIN,
		-WEATHER_RAIN_TIMING_ADD,
		WEATHER_RAIN_ADD_TIMING,
		WEATHER_RAIN_ADD,
		WEATHER_RAIN_OBJ_Add );


	// フォグの設定
	WEATHER_TASK_FogSet( p_wk, WEATHER_FOG_SLOPE_DEFAULT, WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS, fog_cont );

	// オブジェクトを散らばす
	WEATHER_TASK_DustObj( p_wk, WEATHER_RAIN_OBJ_Add, WEATHER_RAIN_NOFADE_OBJ_START_NUM, WEATHER_RAIN_NOFADE_OBJ_START_DUST_NUM, WEATHER_RAIN_NOFADE_OBJ_START_DUST_MOVE, heapID );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	メイン処理
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Main( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	WEATHER_TASK_ObjFade_NoFadeMain( p_wk, heapID );

	// スクロール処理
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト初期化
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_InitFadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
	// ローカルワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );

	// obj
	// フェードアウト設定
	WEATHER_TASK_ObjFade_SetOut( p_wk,
			0,
			WEATHER_RAIN_TIMING_MAX,
			WEATHER_RAIN_TIMING_ADD,
			-WEATHER_RAIN_ADD );
	
	// フォグ
	p_local_wk->work[0] = WEATHER_RAIN_FOG_START_END;	// 同じくフォグ用

	// 音
//	WeatherLoopSndStop( sys_work );

	return WEATHER_TASK_FUNC_RESULT_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	フェードアウト
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_FadeOut( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
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
					WEATHER_FOG_DEPTH_DEFAULT + WEATHER_RAIN_FOG_OFS_START, 
					WEATHER_RAIN_FOG_TIMING_END, fog_cont );
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
	{
		int x, y;
		WEATHER_TASK_GetScrollDist( p_wk, &x, &y );
		WEATHER_TASK_ScrollObj( p_wk, x, y );
	}

	return WEATHER_TASK_FUNC_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief	破棄
 */
//-----------------------------------------------------------------------------
static WEATHER_TASK_FUNC_RESULT WEATHER_RAIN_Exit( WEATHER_TASK* p_wk, WEATHER_TASK_FOG_MODE fog_cont, u32 heapID )
{
	WEATHER_RAIN_WORK* p_local_wk;
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
static void WEATHER_RAIN_OBJ_Move( WEATHER_OBJ_WORK* p_wk )
{
	int i;
	GFL_CLACTPOS mat;
	GFL_CLWK* p_clwk;
	s32*	obj_w;

	obj_w = WEATHER_OBJ_WORK_GetWork( p_wk );
	p_clwk = WEATHER_OBJ_WORK_GetClWk( p_wk );

	GFL_CLACT_WK_GetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	
	// 動作フラグをチェック
	switch(obj_w[3]){
	case 0:		// 動作
		// 動かす
		mat.x += (obj_w[4]*2);
		mat.y += (obj_w[2]*2);
	
		// 破棄するかチェック
		obj_w[0] += 2;
		if(obj_w[0] > obj_w[1]){
			// アニメーションさせるかチェック
			if(GFUser_GetPublicRand(10) < 7){		// 7/10は破棄
				// 破棄
				obj_w[3] = 2;
			}else{
			
				// 破棄アニメ
				obj_w[3] = 1;		
				obj_w[0] = 4;			// 破棄アニメカウンタ
				GFL_CLACT_WK_SetAnmFrame( p_clwk, 3 );
			}
		}
		// 座標設定
		GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );

		break;
	case 1:		// 破棄アニメ
		if(obj_w[0]-- <= 0){
			obj_w[3] = 2;		// 破棄
		}
		break;

	case 2:		// 破棄
		WEATHER_TASK_DeleteObj( p_wk );
		break;
	}		
}

//----------------------------------------------------------------------------
/**
 *	@brief	雨オブジェ登録
 */
//-----------------------------------------------------------------------------
static void WEATHER_RAIN_OBJ_Add( WEATHER_TASK* p_wk, int num, u32 heapID )
{
	int i;		// ループ用
	WEATHER_OBJ_WORK* add_obj;		// 登録オブジェ
	WEATHER_RAIN_WORK*	p_local_wk;	// システムワーク
	int	err;	// 補正値
	s32* obj_w;	// オブジェクトワーク
	int frame;	// 描画フレーム
	GFL_CLACTPOS mat;	// 設定座標
	GFL_CLWK* p_clwk;
	u32 rand;

	// ユーザワーク取得
	p_local_wk = WEATHER_TASK_GetWorkData( p_wk );
	
	// num分オブジェクトを登録
	for(i=0;i<num;i++){

		add_obj = WEATHER_TASK_CreateObj( p_wk, heapID );		// 登録
		if(add_obj == NULL){			// 失敗したら終了
			break;
		}
		obj_w = WEATHER_OBJ_WORK_GetWork( add_obj );		// オブジェワーク作成
		p_clwk	= WEATHER_OBJ_WORK_GetClWk( add_obj );

		// 乱数取得
		rand = GFUser_GetPublicRand(0);
		
		// 領域を初期化
		obj_w[0] = 0;			// カウンタ
		frame = rand%3;	// 雨の種類
		GFL_CLACT_WK_SetAnmFrame( p_clwk, frame );

		err = (rand % WEATHER_RAIN_SPEED_ERR);
		obj_w[2] = WEATHER_RAIN_SPEED_Y * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[2] += WEATHER_RAIN_SPEED_Y;
		}
		
		err /= WEATHER_RAIN_SPEED_X;
		obj_w[4] = WEATHER_RAIN_SPEED_X * (frame+1) + err;
		if(frame == 2){		// 一番前の雨はスピードを１段階上げる
			obj_w[4] += WEATHER_RAIN_SPEED_X;
		}
		obj_w[3] = 0;			// 破棄アニメフラグ
		
		// 破棄タイミング	全部共通の範囲の値
		obj_w[1] = WEATHER_RAIN_END_MIN  + (rand % WEATHER_RAIN_END_MAX);
		
		// 座標を設定
		mat.x = ( WEATHER_RAIN_START_X_BASE + (frame * WEATHER_RAIN_MUL_X) + (rand % WEATHER_RAIN_START_X_MAX) );
		mat.y = WEATHER_RAIN_START_Y;
		GFL_CLACT_WK_SetPos( p_clwk, &mat, CLSYS_DEFREND_MAIN );
	}
}

