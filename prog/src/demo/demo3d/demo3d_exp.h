/*
 *  @file   demo3d_exp.h
 *  @brief  demo3d　例外処理定義用まとめヘッダ
 *  @author hosaka,iwasawa
 *  @date   10.02.25
 */

// 不完全型
typedef struct _APP_EXCEPTION_WORK APP_EXCEPTION_WORK;

//--------------------------------------------------------------
/// 例外処理 関数定義
//==============================================================
typedef void (*APP_EXCEPTION_FUNC)( APP_EXCEPTION_WORK* wk );

//--------------------------------------------------------------
/// 例外処理 関数テーブル
//==============================================================
typedef struct {
  APP_EXCEPTION_FUNC Init;
  APP_EXCEPTION_FUNC Main;
  APP_EXCEPTION_FUNC End;
} APP_EXCEPTION_FUNC_SET;

//--------------------------------------------------------------
///	アプリ例外処理
//==============================================================
struct _APP_EXCEPTION_WORK {
  //[IN]
  HEAPID heapID;
  const DEMO3D_GRAPHIC_WORK* graphic;
  const DEMO3D_ENGINE_WORK* engine;
  //[PRIVATE]
  const APP_EXCEPTION_FUNC_SET* p_funcset;
  void *userwork;
  int  key_skip;  // キースキップ
};

//////////////////////////////////////////////////////////////
//デモ別定義


//遊覧船デモ例外処理テーブル
extern const APP_EXCEPTION_FUNC_SET DATA_c_exp_funcset_c_cruiser;
