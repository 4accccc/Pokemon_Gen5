//============================================================================
/**
 *  @file   zukan_detail_map.c
 *  @brief  図鑑詳細画面の姿画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_MAP
 */
//============================================================================
#define DEBUG_KAWADA


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_map.h"

// アーカイブ
#include "arc_def.h"
#include "townmap_gra.naix"

// サウンド

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// メインBGフレーム
#define BG_FRAME_M_ROOT           (GFL_BG_FRAME2_M)
#define BG_FRAME_M_MAP            (GFL_BG_FRAME3_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_ROOT       (1)
#define BG_FRAME_PRI_M_MAP        (2)


// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_FADE_OUT,
  SEQ_END,
};

// 終了命令
typedef enum
{
  END_CMD_NONE,
  END_CMD_INSIDE,
  END_CMD_OUTSIDE,
}
END_CMD;

// BG面設定
typedef struct
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
	BOOL								is_visible;
}
BGSETUP;

// GFL_BG_FRAME2_MとGFL_BG_FRAME3_Mをアフィン拡張BGにする
static const BGSETUP sc_bgsetup_affineex[]	=
{	
	//MAIN------------------------
	{	
		GFL_BG_FRAME2_M,	//設定するフレーム
		{
			0, 0, 0x0800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x128,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
          // 後ろから取らないこと！ 256x64が限界！  // 本当はGFL_BG_CHRSIZ256_256x128を指定してやるべきなんだろうなあ
          // GFL_BG_CHRSIZ_256x128は0x4000なのでちょうどサイズが一致している
          // 0x10000～0x14000    64バイト*32キャラ*8キャラ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
          // GFL_BG_MODE_256X16ではbgExtPlttを使用しないのでてきとーな値
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x14000, GFL_BG_CHRSIZ_256x256,//GFL_BG_CHRSIZ256_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
          // 後ろから取らないこと！ 256x192が限界！  // 本当はGFL_BG_CHRSIZ256_256x256を指定してやるべきなんだろうなあ
          // GFL_BG_CHRSIZ_256x256は0x8000なのでサイズが0xC000には0x4000ほど足りないが大丈夫かな？
          // GFL_BG_CHRSIZ256_256x256は0x10000なので128Kをはみ出してしまうので、はみ出すよりは足りないほうがいいかな？
          // 0x14000～0x20000    64バイト*32キャラ*24キャラ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
          // GFL_BG_MODE_256X16ではbgExtPlttを使用しないのでGX_BG_EXTPLTT_01はてきとーな値
		},
		GFL_BG_MODE_256X16,//BGの種類
		TRUE,	//初期表示
	},
};

// GFL_BG_FRAME2_MとGFL_BG_FRAME3_MをテキストBGにする
static const BGSETUP sc_bgsetup_text[]	=
{	
	//MAIN------------------------
	{	
		GFL_BG_FRAME2_M,	//設定するフレーム
		{
			0, 0, 0x0800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
	{	
		GFL_BG_FRAME3_M,	//設定するフレーム
		{
			0, 0, 0x800, 0,	//X,Y,ｽｸﾘｰﾝﾊﾞｯﾌｧ、ｽｸﾘｰﾝｵﾌｾｯﾄ
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,	//ｽｸﾘｰﾝｻｲｽﾞ、ｶﾗｰﾓｰﾄﾞ
			GX_BG_SCRBASE_0x3000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,//ｽｸﾘｰﾝﾍﾞｰｽ、ｷｬﾗﾍﾞｰｽ、ｷｬﾗｻｲｽﾞ
			GX_BG_EXTPLTT_01, 3, 0, 0, FALSE//拡張ﾊﾟﾚｯﾄｽﾛｯﾄ、表示優先度、ｴﾘｱｵｰﾊﾞｰ、ﾀﾞﾐｰ、ﾓｻﾞｲｸﾌﾗｸﾞ
		},
		GFL_BG_MODE_TEXT,//BGの種類
		TRUE,	//初期表示
	},
};

/*
TWL_ProgramingManual.pdf

p43
3.2 TWLプロセッサ内メモリ
図 3-15 BG 拡張パレットスロット メモリマップ
0x00008000
0x00006000  スロット3
0x00004000  スロット2
0x00002000  スロット1
0x00000000  スロット0

p100
6.2.2 BGコントロール
拡張パレットがイネーブルのとき、BG2 は拡張パレットスロット2 を使用し、BG3 は拡張パレットスロット3 を使用し
ます。BG2 および3 において、使用する拡張パレットNo.を変更することはできません。
*/


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // ここで作成するもの
  u32                         tmp_count;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;

  // 終了命令
  END_CMD                     end_cmd;
}
ZUKAN_DETAIL_MAP_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Map_VBlankFunc( GFL_TCB* tcb, void* wk );

// アフィン拡張BG
static void Zukan_Detail_Map_AffineExBGInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_AffineExBGExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn );


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Map_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_MAP_ProcData =
{
  Zukan_Detail_Map_ProcInit,
  Zukan_Detail_Map_ProcMain,
  Zukan_Detail_Map_ProcExit,
  Zukan_Detail_Map_CommandFunc,
  NULL,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             ZUKAN_DETAIL_MAP_InitParam(
                            ZUKAN_DETAIL_MAP_PARAM*  param,
                            HEAPID                    heap_id )
{
  param->heap_id = heap_id;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Map_ProcInit\n" );
  }
#endif

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_MAP_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_MAP_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Map_VBlankFunc, work, 1 );

  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Map_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Map_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_PREPARE;

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_5, GX_BG0_AS_2D );
        
        // BG面設定
        {	
          int i;
          for( i = 0; i < NELEMS(sc_bgsetup_affineex); i++ )
          {	
            GFL_BG_FreeBGControl( sc_bgsetup_affineex[i].frame );
            
            GFL_BG_SetBGControl( sc_bgsetup_affineex[i].frame, &sc_bgsetup_affineex[i].bgcnt_header, sc_bgsetup_affineex[i].mode );
            GFL_BG_ClearFrame( sc_bgsetup_affineex[i].frame );
            GFL_BG_SetVisible( sc_bgsetup_affineex[i].frame, sc_bgsetup_affineex[i].is_visible );
          }
        }
      }

      // BG
      {
        // クリア
        u8 i;
        for( i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++ )
        {
          if(    i != ZKNDTL_BG_FRAME_M_TOUCHBAR
              && i != ZKNDTL_BG_FRAME_S_HEADBAR )
          {
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_X_SET, 0 );
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_Y_SET, 0 );
			      GFL_BG_ClearFrame(i);
          }
        }

        // メインBG
        GFL_BG_SetPriority( BG_FRAME_M_ROOT, BG_FRAME_PRI_M_ROOT );
        GFL_BG_SetPriority( BG_FRAME_M_MAP, BG_FRAME_PRI_M_MAP );
        
        // サブBG
        //GFL_BG_SetPriority( BG_FRAME_S_, BG_FRAME_PRI_S_ );
      }

      // アフィン拡張BG
      Zukan_Detail_Map_AffineExBGInit( param, work, cmn );
    }
    break;
  case SEQ_PREPARE:
    {
      *seq = SEQ_FADE_IN;

      // フェード
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

      // タッチバー
      if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_SetType(
            touchbar,
            ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
            ZUKAN_DETAIL_TOUCHBAR_DISP_MAP );
        ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
      }
      // タイトルバー
      if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_HEADBAR_SetType(
            headbar,
            ZUKAN_DETAIL_HEADBAR_TYPE_MAP );
        ZUKAN_DETAIL_HEADBAR_Appear( headbar );
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );

        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        *seq = SEQ_FADE_OUT;

        // フェード
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
        
        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
      }
      else
      {
        // タッチ
        //Zukan_Detail_Map_Touch( param, work, cmn );
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      if(    (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          && (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            *seq = SEQ_END;
          }
        }
        else
        {
          *seq = SEQ_END;
        }
      }
    }
    break;
  case SEQ_END:
    {
      // アフィン拡張BG
      Zukan_Detail_Map_AffineExBGExit( param, work, cmn );
      
      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );

        // BG面設定
        {	
          int i;
          for( i = 0; i < NELEMS(sc_bgsetup_text); i++ )
          {	
            GFL_BG_FreeBGControl( sc_bgsetup_text[i].frame );
            
            GFL_BG_SetBGControl( sc_bgsetup_text[i].frame, &sc_bgsetup_text[i].bgcnt_header, sc_bgsetup_text[i].mode );
            GFL_BG_ClearFrame( sc_bgsetup_text[i].frame );
            GFL_BG_SetVisible( sc_bgsetup_text[i].frame, sc_bgsetup_text[i].is_visible );
          }
        }
      }

      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );

  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Map_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_MAP_PARAM*    param    = (ZUKAN_DETAIL_MAP_PARAM*)pwk;
    ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
    switch( cmd )
    {
    case ZKNDTL_CMD_NONE:
      {
      }
      break;
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
      {
        work->end_cmd = END_CMD_OUTSIDE;
      }
      break;
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_FORM:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToNextPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          //Zukan_Detail_Map_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
      {
        u16 monsno_curr;
        u16 monsno_go;
        monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
        ZKNDTL_COMMON_GoToPrevPoke(cmn);
        monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
        if( monsno_curr != monsno_go )
        {
          //Zukan_Detail_Map_ChangePoke(param, work, cmn);
        }
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    default:
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    }
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Map_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_MAP_WORK*     work     = (ZUKAN_DETAIL_MAP_WORK*)wk;
}

//-------------------------------------
/// アフィン拡張BG
//=====================================
static void Zukan_Detail_Map_AffineExBGInit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 読み込んで転送
  ARCHANDLE* handle;
  handle = GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, param->heap_id );

  // 2面で使用拡張パレットスロットが違うので2回転送している
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_townmap_gra_tmap_bg_d_NCLR,
      PALTYPE_MAIN_BG_EX, 0x6000, 2*16*16*16, param->heap_id );  // BG3用にスロット3(0x6000)へ  // 2バイト*16色*16ライン*16ページのパレット
  GFL_ARCHDL_UTIL_TransVramPalette( handle, NARC_townmap_gra_tmap_bg_d_NCLR,
      PALTYPE_MAIN_BG_EX, 0x4000, 2*16*16*16, param->heap_id );  // BG2用にスロット2(0x4000)へ

  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_townmap_gra_tmap_bg_d_NCGR,
      BG_FRAME_M_ROOT, 0, 64*32*8, FALSE, param->heap_id );  // 32キャラ*8キャラ=64バイト*32*8(これが限界の最大値)
  GFL_ARCHDL_UTIL_TransVramBgCharacter( handle, NARC_townmap_gra_tmap_map_d_NCGR,
      BG_FRAME_M_MAP, 0, 64*32*24, FALSE, param->heap_id );  // 32キャラ*24キャラ=64バイト*32*24(これが限界の最大値)

  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_townmap_gra_tmap_root_d_NSCR,
      BG_FRAME_M_ROOT, 0, 0x800, FALSE, param->heap_id );  // 32キャラ*32キャラ=2バイト*32*32=2K
  GFL_ARCHDL_UTIL_TransVramScreen( handle, NARC_townmap_gra_tmap_map_d_NSCR,
      BG_FRAME_M_MAP, 0, 0x800, FALSE, param->heap_id );  // 32キャラ*32キャラ=2バイト*32*32=2K

  GFL_ARC_CloseDataHandle( handle );
}
static void Zukan_Detail_Map_AffineExBGExit( ZUKAN_DETAIL_MAP_PARAM* param, ZUKAN_DETAIL_MAP_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 特に何もしなくてよい
}

