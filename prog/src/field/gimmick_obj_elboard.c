////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief 電光掲示板
 * @file  gimmick_obj_elboard.c
 * @author obata
 * @date   2009.10.22
 *
 */
////////////////////////////////////////////////////////////////////////////////////
#include <gflib.h>
#include "gimmick_obj_elboard.h"
#include "system/el_scoreboard.h"
#include "print/printsys.h"  // for PRINTSYS_GetStrWidth
#include "arc/arc_def.h"
#include "arc/font/font.naix"


//==================================================================================
// ■定数
//==================================================================================
#define WRITE_POS_Y  (1)  // 文字列書き込み先y座標
#define CHAR_WIDTH  (13)  // 文字幅[dot]
#define CHAR_HEIGHT (16)  // 文字高さ[dot]
#define MARGIN_SIZE  (1)  // 文字間スペース幅
#define TEX_WIDTH (1024)  // テクスチャの幅
#define TEX_HEIGHT  (16)  // テクスチャの高さ
#define MAX_STRLEN (128)  // ニュース1つあたりの最大文字数


//==================================================================================
// ■ニュース管理ワーク
//==================================================================================
typedef struct
{
  u8                index; // 自身の掲示板内インデックス
  BOOL           dispFlag; // 表示中かどうか
  BOOL         switchFlag; // 次のニュース表示スイッチを押したかどうか
  EL_SCOREBOARD_TEX*  tex; // テクスチャ
  GFL_G3D_OBJ*     g3dObj; // 掲示板の3Dオブジェクト
  u16          animeIndex; // アニメーション番号
  fx32           nowFrame; // 現在のアニメーションフレーム数
  fx32        switchFrame; // 次のニュース開始フレーム数
  fx32           endFrame; // 表示終了フレーム数
} NEWS;


//==================================================================================
// ■電光掲示板ワーク
//==================================================================================
struct _GOBJ_ELBOARD
{
  HEAPID                 heapID;  // 使用するヒープID
  u8                    newsNum;  // 表示するニュースの数
  u8                 maxNewsNum;  // 表示可能なニュースの最大数
  u8                   dispSize;  // 一画面に表示可能な文字数
  u8               newsInterval;  // ニュース間の間隔(文字数)
  NEWS**                   news;  // ニュース配列
  BOOL*              newsSwitch;  // ニュース表示スイッチ配列
  GFL_G3D_OBJ*           g3dObj;  // 掲示板の3Dオブジェクト
  GFL_FONT*                font;  // ニュース描画に使用するフォント
  fx32                    frame;  // 動作フレームカウンタ
};


//==================================================================================
// ■非公開関数のプロトタイプ宣言
//==================================================================================
static NEWS* NewsCreate( GOBJ_ELBOARD* elboard, const NEWS_PARAM* param, int index );
static void NewsDelete( NEWS* news );
static void NewsMove_Start( NEWS* news );
static void NewsMove_Update( GOBJ_ELBOARD* elboard, NEWS* news, fx32 frame );
static void NewsMove_End( NEWS* news );


//==================================================================================
// ■初期化
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を作成する
 *
 * @param param 掲示板パラメータ
 *
 * @return 作成した電光掲示板
 */
//----------------------------------------------------------------------------------
GOBJ_ELBOARD* GOBJ_ELBOARD_Create( ELBOARD_PARAM* param )
{
  int i;
  GOBJ_ELBOARD* elboard;

  // インスタンス生成
  elboard = GFL_HEAP_AllocMemory( param->heapID, sizeof(GOBJ_ELBOARD) );

  // 初期化
  elboard->heapID       = param->heapID;
  elboard->newsNum      = 0;
  elboard->maxNewsNum   = param->maxNewsNum;
  elboard->dispSize     = param->dispSize;
  elboard->newsInterval = param->newsInterval;
  elboard->g3dObj       = param->g3dObj;
  elboard->frame        = 0;

  // ニュース配列・表示スイッチ配列を確保
  elboard->news = GFL_HEAP_AllocMemory(
      param->heapID, sizeof(NEWS*)*param->maxNewsNum );
  elboard->newsSwitch = GFL_HEAP_AllocMemory(
      param->heapID, sizeof(BOOL)*param->maxNewsNum );

  // アニメーション状態を初期化
  for( i=0; i<param->maxNewsNum; i++ )
  {
    elboard->news[i]       = NULL;
    elboard->newsSwitch[i] = FALSE;
  }
  elboard->newsSwitch[0] = TRUE;  // 最初のニュースを表示する

  // ニュース描画に使用するフォントを作成
  elboard->font = GFL_FONT_Create(
      ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, elboard->heapID );

  return elboard;
}

//----------------------------------------------------------------------------------
/**
 * @brief 表示するニュースを追加する
 *
 * @param elboard    追加先の掲示板
 * @param news_param 追加するニュース
 */
//----------------------------------------------------------------------------------
void GOBJ_ELBOARD_AddNews( GOBJ_ELBOARD* elboard, const NEWS_PARAM* news_param )
{
  // すでに最大数を登録済み ==> 登録しない
  if( elboard->maxNewsNum <= elboard->newsNum )
  {
    OBATA_Printf( "------------------------------------------\n" );
    OBATA_Printf( "すでに最大数のニュースが登録されています。\n" );
    OBATA_Printf( "------------------------------------------\n" );
    return;
  }

  // 登録
  elboard->news[ elboard->newsNum ] =
    NewsCreate( elboard, news_param, elboard->newsNum );
  elboard->newsNum++;
}



//==================================================================================
// ■破棄
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 電光掲示板を破棄する
 *
 * @param elboard 破棄する掲示板
 */
//----------------------------------------------------------------------------------
void GOBJ_ELBOARD_Delete( GOBJ_ELBOARD* elboard )
{
  int i;

  // フォントを破棄
  GFL_FONT_Delete( elboard->font );

  // 各ニュースを破棄
  for( i=0; i<elboard->newsNum; i++ )
  {
    NewsDelete( elboard->news[i] );
  }
  GFL_HEAP_FreeMemory( elboard->news );

  // 表示スイッチ配列を破棄
  GFL_HEAP_FreeMemory( elboard->newsSwitch );

  // 掲示板ワーク本体を破棄
  GFL_HEAP_FreeMemory( elboard );
}


//==================================================================================
// ■動作
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 電光掲示板メイン動作
 *
 * @param elboard 動かす掲示板
 * @param frame   動作フレーム数
 */
//----------------------------------------------------------------------------------
void GOBJ_ELBOARD_Main( GOBJ_ELBOARD* elboard, fx32 frame )
{
  int i;

  // 各ニュースを動かす
  for( i=0; i<elboard->newsNum; i++ )
  {
    NewsMove_Update( elboard, elboard->news[i], frame );
  }

  // 動作フレーム数更新
  elboard->frame += frame;
}


//==================================================================================
// ■取得
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 現在の動作フレーム数を取得する
 *
 * @param elboard 取得対象の掲示板
 *
 * @return 指定した掲示板の現在の動作フレーム数
 */
//----------------------------------------------------------------------------------
fx32 GOBJ_ELBOARD_GetFrame( const GOBJ_ELBOARD* elboard )
{
  return elboard->frame;
}

//----------------------------------------------------------------------------------
/**
 * @brief 電光掲示板が使用するヒープIDを取得する
 *
 * @param elboard 取得対象の掲示板
 *
 * @return 掲示板が使用するヒープID
 */
//----------------------------------------------------------------------------------
HEAPID GOBJ_ELBOARD_GetHeapID( const GOBJ_ELBOARD* elboard )
{
  return elboard->heapID;
}

//----------------------------------------------------------------------------------
/**
 * @brief 表示するニュースの数を取得する
 *
 * @param elboard 取得対象の掲示板
 *
 * @return 指定した掲示板が表示するニュースの数
 */
//----------------------------------------------------------------------------------
u8 GOBJ_ELBOARD_GetNewsNum( const GOBJ_ELBOARD* elboard )
{
  return elboard->newsNum;
}


//==================================================================================
// ■設定
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief 動作フレーム数を設定する
 *
 * @param elboard 設定対象の掲示板
 * @param frame   設定するフレーム数
 */
//----------------------------------------------------------------------------------
void GOBJ_ELBOARD_SetFrame( GOBJ_ELBOARD* elboard, fx32 frame )
{
  int i;
  int set_frame = FX_Whole( frame );
  int one_round_frame = 0;  // 一周に要するフレーム数
  int* start_frame =        // 各ニュースのスタートフレーム
    GFL_HEAP_AllocMemoryLo( elboard->heapID, sizeof(int)*elboard->newsNum );
  int* now_frame =        // 各ニュースの現在フレーム
    GFL_HEAP_AllocMemoryLo( elboard->heapID, sizeof(int)*elboard->newsNum );

  // 現在フレーム数を記憶
  elboard->frame = frame;

  // 前準備
  for( i=0; i<elboard->newsNum; i++ )
  {
    // ニュース表示スイッチ
    elboard->newsSwitch[i] = FALSE;

    // 一周に要するフレーム数を計算
    one_round_frame += FX_Whole( elboard->news[i]->switchFrame );

    // スタートフレームを求める
    if( i == 0 )
    { // 最初のニュース = 0
      start_frame[i] = 0;
    }
    else
    {
      // 前ニュースのスタートフレーム + 前ニュースのスイッチフレーム数
      start_frame[i] =
        start_frame[i-1] + FX_Whole( elboard->news[i-1]->switchFrame );
    }
  }

  // 各ニュースの現在フレーム数を計算
  for( i=0; i<elboard->newsNum; i++ )
  {
    NEWS* news = elboard->news[i];
    now_frame[i] = (set_frame % one_round_frame) - start_frame[i];
    news->dispFlag = (0 <= now_frame[i]) && (now_frame[i] < FX_Whole(news->endFrame));
    news->switchFlag = (FX_Whole(news->switchFrame) < now_frame[i]) &&
      (now_frame[i] < FX_Whole(news->endFrame));
    if( now_frame[i] < 0 ) now_frame[i] = 0;
    news->nowFrame = now_frame[i] << FX32_SHIFT;
    GFL_G3D_OBJECT_SetAnimeFrame( news->g3dObj, news->animeIndex, (int*)&news->nowFrame );
    if( news->dispFlag )
    {
      GFL_G3D_OBJECT_EnableAnime( news->g3dObj, news->animeIndex );
    }
    else
    {
      GFL_G3D_OBJECT_DisableAnime( news->g3dObj, news->animeIndex );
    }
  }

  // ニューススイッチの状態を算出
  for( i=0; i<elboard->newsNum; i++ )
  {
    int next = (i+1)%elboard->newsNum;
    elboard->newsSwitch[next] =
      elboard->news[i]->switchFlag && !elboard->news[next]->dispFlag;
  }

  // DEBUG:
  OBATA_Printf( "set_frame = %d\n", set_frame );
  OBATA_Printf( "one_round_frame = %d\n", one_round_frame );
  for( i=0; i<elboard->newsNum; i++ )
    OBATA_Printf( "start_frame[%d] = %d\n", i, start_frame[i] );
  for( i=0; i<elboard->newsNum; i++ )
    OBATA_Printf( "now_frame[%d] = %d\n", i, now_frame[i] );
  for( i=0; i<elboard->newsNum; i++ )
    OBATA_Printf( "news[%d]->dispFlag = %d\n", i, elboard->news[i]->dispFlag );
  for( i=0; i<elboard->newsNum; i++ )
    OBATA_Printf( "newsSwitch[%d] = %d\n", i, elboard->newsSwitch[i] );

  // 後始末
  GFL_HEAP_FreeMemory( start_frame );
  GFL_HEAP_FreeMemory( now_frame );
}


//==================================================================================
// ■非公開関数
//==================================================================================

//----------------------------------------------------------------------------------
/**
 * @brief ニュースを作成する
 *
 * @param elboard 追加先の電光掲示板
 * @param param   ニュースパラメータ
 * @param index   ニュース番号
 *
 * @return 作成したニュース
 */
//----------------------------------------------------------------------------------
static NEWS* NewsCreate( GOBJ_ELBOARD* elboard, const NEWS_PARAM* param, int index )
{
  NEWS* news;
  u32 str_width; // 文字列の幅

  // インスタンス生成
  news = GFL_HEAP_AllocMemory( elboard->heapID, sizeof(NEWS) );

  // テクスチャを作成
  {
    GFL_MSGDATA* msg; // メッセージデータ
    STRBUF*   strbuf; // 描画文字列
    GFL_G3D_RND* rnd;
    GFL_G3D_RES* res;

    // 描画する文字列を取得
    msg = GFL_MSG_Create(
        GFL_MSG_LOAD_NORMAL, param->msgArcID, param->msgDatID, elboard->heapID );
    if( param->wordset )
    { // WORDSET展開
      STRBUF* strbuf_msg;
      strbuf = GFL_STR_CreateBuffer( MAX_STRLEN, elboard->heapID );
      strbuf_msg = GFL_MSG_CreateString( msg, param->msgStrID );
      WORDSET_ExpandStr( param->wordset, strbuf, strbuf_msg );
      GFL_HEAP_FreeMemory( strbuf_msg );
    }
    else
    { // メッセージをそのまま使用
      strbuf = GFL_MSG_CreateString( msg, param->msgStrID );
    }
    str_width = PRINTSYS_GetStrWidth( strbuf, elboard->font, MARGIN_SIZE );// 文字列の幅を取得
    // 描画先テクスチャを取得
    rnd = GFL_G3D_OBJECT_GetG3Drnd( elboard->g3dObj );
    res = GFL_G3D_RENDER_GetG3DresTex( rnd );
    // 描画
    news->tex = ELBOARD_TEX_Add_Ex( res, param->texName, param->pltName, strbuf,
                                    (CHAR_WIDTH + MARGIN_SIZE) * elboard->dispSize, 
                                    WRITE_POS_Y, elboard->heapID );
    // 後始末
    GFL_HEAP_FreeMemory( strbuf );
    GFL_MSG_Delete( msg );
  }

  // 制御に必要なアニメーションフレームを計算
  {
    GFL_G3D_ANM* anm;
    NNSG3dAnmObj* anm_obj;
    fx32 frame_size;  // 全アニメーションフレーム数
    int left_pos, right_pos, switch_pos;  // 左端, 右端, スイッチ座標

    // 全アニメーションフレーム数を取得
    anm        = GFL_G3D_OBJECT_GetG3Danm( elboard->g3dObj, param->animeIndex );
    anm_obj    = GFL_G3D_ANIME_GetAnmObj( anm );
    frame_size = NNS_G3dAnmObjGetNumFrame( anm_obj );
    // 計算に必要なテクスチャ座標を算出
    left_pos   = CHAR_WIDTH * elboard->dispSize;
    right_pos  = left_pos + str_width;
    switch_pos = right_pos - left_pos + (elboard->newsInterval * CHAR_WIDTH);
    // 各種制御フレーム数を計算
    news->nowFrame    = 0;
    news->switchFrame = FX_F32_TO_FX32( ((float)switch_pos/(float)TEX_WIDTH) );
    news->switchFrame = FX_Mul( news->switchFrame, frame_size );
    news->endFrame    = FX_F32_TO_FX32( ((float)right_pos/(float)TEX_WIDTH) );
    news->endFrame    = FX_Mul( news->endFrame, frame_size );
    OBATA_Printf( "switchFrame = %d\n", FX_Whole(news->switchFrame) );
    OBATA_Printf( "endFrame = %d\n", FX_Whole(news->endFrame) );
  }

  // その他の初期化
  news->index      = index;
  news->dispFlag   = FALSE;
  news->switchFlag = FALSE;
  news->g3dObj     = elboard->g3dObj;
  news->animeIndex = param->animeIndex;

  // 非表示状態にする
  GFL_G3D_OBJECT_DisableAnime( news->g3dObj, news->animeIndex );
  GFL_G3D_OBJECT_ResetAnimeFrame( news->g3dObj, news->animeIndex );

  // 作成したニュースを返す
  return news;
}

//----------------------------------------------------------------------------------
/**
 * @brief ニュースを破棄する
 *
 * @param news 破棄するニュース
 */
//----------------------------------------------------------------------------------
static void NewsDelete( NEWS* news )
{
  // テクスチャを破棄
  ELBOARD_TEX_Delete( news->tex );

  // ニュース本体を破棄
  GFL_HEAP_FreeMemory( news );
}

//----------------------------------------------------------------------------------
/**
 * @brief 指定したニュースの表示を開始する
 *
 * @param 表示を開始するニュース
 */
//----------------------------------------------------------------------------------
static void NewsMove_Start( NEWS* news )
{
  // 非表示状態なら, 表示開始
  if( news->dispFlag != TRUE )
  {
    news->dispFlag   = TRUE;
    news->switchFlag = FALSE;
    news->nowFrame   = 0;
    GFL_G3D_OBJECT_EnableAnime( news->g3dObj, news->animeIndex );
    GFL_G3D_OBJECT_ResetAnimeFrame( news->g3dObj, news->animeIndex );
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 指定したニュースを動かす
 *
 * @param elboard ニュースを管理する電光掲示板
 * @param news    動かすニュース
 * @param frame   更新フレーム数
 */
//----------------------------------------------------------------------------------
static void NewsMove_Update( GOBJ_ELBOARD* elboard, NEWS* news, fx32 frame )
{
  // 表示中の動作
  if( news->dispFlag == TRUE )
  {
    // アニメーションを更新
    ELBOARD_TEX_Main( news->tex );
    GFL_G3D_OBJECT_IncAnimeFrame( news->g3dObj, news->animeIndex, frame );
    news->nowFrame += frame;

    // スイッチフレームに到達 ==> 次のニュースのスイッチをON
    if( (news->switchFrame < news->nowFrame) &&
        (news->switchFlag != TRUE) )
    {
      int next = (news->index + 1) % elboard->newsNum;
      elboard->newsSwitch[next] = TRUE;
      news->switchFlag = TRUE;  // 一度だけ押す
    }

    // アニメーション終了フレームに到達 ==> 動作終了
    if( news->endFrame < news->nowFrame )
    {
      NewsMove_End( news );
    }
  }
  // 非表示中の動作
  else
  {
    // 自身のスイッチが押されていたら, 表示を開始する
    if( elboard->newsSwitch[news->index] == TRUE )
    {
      NewsMove_Start( news );
      elboard->newsSwitch[news->index] = FALSE;
    }
  }
}

//----------------------------------------------------------------------------------
/**
 * @brief 指定したニュースの表示を終了する
 *
 * @param news 表示を終了するニュース
 */
//----------------------------------------------------------------------------------
static void NewsMove_End( NEWS* news )
{
  // 表示中なら, 非表示にする
  if( news->dispFlag == TRUE )
  {
    news->dispFlag = FALSE;
    GFL_G3D_OBJECT_DisableAnime( news->g3dObj, news->animeIndex );
    GFL_G3D_OBJECT_ResetAnimeFrame( news->g3dObj, news->animeIndex );
  }
}
