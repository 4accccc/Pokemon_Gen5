//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		br_sidebar.c
 *	@brief  サイドバー制御
 *	@author	Toru=Nagihashi
 *	@data		2009.12.20
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//ライブラリ
#include <gflib.h>
#include <calctool.h>

//バトルレコーダー内モジュール
#include "br_res.h"
#include "br_fade.h"

//外部公開
#include "br_sidebar.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	個数等
//=====================================
#define BR_SIDEBAR_DISPLAY_MAX  (2)
#define BR_SIDEBAR_MAX          (6*BR_SIDEBAR_DISPLAY_MAX)
#define BR_SIDEBAR_MOVE_F       (10)			///< サイドバーの移動フレーム

//-------------------------------------
///	サイドバー１つ
//=====================================
typedef enum
{ 
  BR_SIDEBAR_ONE_MOVE_MODE_NONE,  //動作しない
  BR_SIDEBAR_ONE_MOVE_MODE_BOOT,  //起動時の動き
  BR_SIDEBAR_ONE_MOVE_MODE_SHAKE, //横でゆらゆら揺れている動き
  BR_SIDEBAR_ONE_MOVE_MODE_END,   //終了時の動き
}BR_SIDEBAR_ONE_MOVE_MODE;

//-------------------------------------
///	サイドバーの方向
//=====================================
typedef enum
{ 
  BR_SIDEBAR_DIR_LEFT,
  BR_SIDEBAR_DIR_RIGHT,
}BR_SIDEBAR_DIR;

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///サイドバーのデータ	
//=====================================
typedef struct 
{
  s16 boot_init_x;            //初期値
  s16 boot_init_y;            //初期値
  s16 boot_scale_x;           //横方向スケール
  s16 boot_move_f;            //移動フレーム
  s16 boot_end_x;             //目的値
  s16 boot_end_y;             //目的値
  s16 shake_init_deg;         //初期値
  s16 shake_deg_dir;          //揺れる方向
  CLSYS_DRAW_TYPE draw_type;  //描画タイプ
  BR_SIDEBAR_DIR  dir;        //方向
} BR_SIDEBAR_DATA;

//-------------------------------------
///	サイドバー１つ
//=====================================
typedef struct _BR_SIDEBAR_ONE BR_SIDEBAR_ONE;
typedef void (*BR_SIDEBAR_MOVE_FUNCTION)( BR_SIDEBAR_ONE *p_wk );
struct _BR_SIDEBAR_ONE 
{
  BOOL  is_exist;
  GFL_CLWK  *p_clwk;
  fx32  x;
  fx32  y;
  fx32  init_x;
  fx32  scale;
  BR_SIDEBAR_MOVE_FUNCTION  move_function;
  u32 state;
  s32 deg;
  s32 wait;
  s32 vconst;
  s32 val_x;
  fx32 val_scale;
  BR_SIDEBAR_DATA data;
} ;

//-------------------------------------
///	サイドバー制御
//=====================================
struct _BR_SIDEBAR_WORK
{ 
  BR_SIDEBAR_ONE  sidebar[ BR_SIDEBAR_MAX ];
};

//=============================================================================
/**
 *					プロトタイプ
*/
//=============================================================================
//-------------------------------------
///	BR_SIDEBAR_ONE
//=====================================
static void BR_SIDEBAR_ONE_Init( BR_SIDEBAR_ONE *p_wk, GFL_CLUNIT *p_clunit, const BR_RES_WORK *cp_res, const BR_SIDEBAR_DATA *cp_data, HEAPID heapID );
static void BR_SIDEBAR_ONE_Exit( BR_SIDEBAR_ONE *p_wk );
static void BR_SIDEBAR_ONE_Main( BR_SIDEBAR_ONE *p_wk );
static BOOL BR_SIDEBAR_ONE_IsExist( const BR_SIDEBAR_ONE *cp_wk );
static void BR_SIDEBAR_ONE_SetMoveMode( BR_SIDEBAR_ONE *p_wk, BR_SIDEBAR_ONE_MOVE_MODE mode );
static BOOL BR_SIDEBAR_ONE_IsMoveNone( const BR_SIDEBAR_ONE *cp_wk );
static void Br_SideBar_One_Move_Boot( BR_SIDEBAR_ONE *p_wk );
static void Br_SideBar_One_Move_Shake( BR_SIDEBAR_ONE *p_wk );
static void Br_SideBar_One_Move_End( BR_SIDEBAR_ONE *p_wk );

//=============================================================================
/**
 *					データ
*/
//=============================================================================
//-------------------------------------
/// サイドバーのデータ	
//=====================================
static const BR_SIDEBAR_DATA sc_sidebar_data[ BR_SIDEBAR_MAX ] =
{ 
  //------------ 上画面 ---------------
  { 
    82,
    96,
    FX32_CONST(1.4f),
    BR_SIDEBAR_MOVE_F -2 * 2,
    8 -2,
    96,
    90,
    +4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    178,
    96,
    FX32_CONST(1.8f),
    BR_SIDEBAR_MOVE_F -3 * 2,
    8 - 3,
    96,
    135,
    -4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    118,
    96,
    FX32_CONST(2.0f),
    BR_SIDEBAR_MOVE_F +2 * 2,
    8 + 2,
    96,
    270,
    +4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    42,
    96,
    FX32_CONST(1.6f),
    BR_SIDEBAR_MOVE_F -1 * 2,
    255- 8 - 1,
    96,
    90,
    +4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_RIGHT,
  },
  { 
    150,
    96,
    FX32_CONST(1.4f),
    BR_SIDEBAR_MOVE_F -3 * 2,
    255- 8 - 3,
    96,
    135,
    -4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_RIGHT,
  },
  { 
    210,
    96,
    FX32_CONST(1.6f),
    BR_SIDEBAR_MOVE_F +2 * 2,
    255- 8 + 2,
    96,
    270,
    +4,
    CLSYS_DRAW_MAIN,
    BR_SIDEBAR_DIR_RIGHT,
  },

  //------------ 下画面 ---------------
  { 
    82,
    96,
    FX32_CONST(1.4f),
    BR_SIDEBAR_MOVE_F -2 * 2,
    8 - 2,
    96,
    90,
    +4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    178,
    96,
    FX32_CONST(1.8f),
    BR_SIDEBAR_MOVE_F -3 * 2,
    8 - 3,
    96,
    135,
    -4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    118,
    96,
    FX32_CONST(2.0f),
    BR_SIDEBAR_MOVE_F +2 * 2,
    8 + 2,
    96,
    270,
    +4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_LEFT,
  },
  { 
    42,
    96,
    FX32_CONST(1.6f),
    BR_SIDEBAR_MOVE_F -1 * 2,
    255- 8 - 1,
    96,
    90,
    +4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_RIGHT,
  },
  { 
    150,
    96,
    FX32_CONST(1.4f),
    BR_SIDEBAR_MOVE_F -3 * 2,
    255- 8 - 3,
    96,
    135,
    -4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_RIGHT,
  },
  { 
    210,
    96,
    FX32_CONST(1.6f),
    BR_SIDEBAR_MOVE_F +2 * 2,
    255- 8 + 2,
    96,270,
    +4,
    CLSYS_DRAW_SUB,
    BR_SIDEBAR_DIR_RIGHT,
  },
};


//=============================================================================
/**
 *					外部公開
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー  初期化
 *
 *	@param	GFL_CLUNIT *p_clunit  OBJ生成用ユニット
 *	@param	*p_fade               フェードモジュール
 *	@param	*p_res                リソース管理
 *	@param	heapID                ヒープID
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
BR_SIDEBAR_WORK * BR_SIDEBAR_Init( GFL_CLUNIT *p_clunit, BR_FADE_WORK *p_fade, BR_RES_WORK *p_res, HEAPID heapID )
{ 
  BR_SIDEBAR_WORK *p_wk;
  p_wk  = GFL_HEAP_AllocMemory( heapID, sizeof(BR_SIDEBAR_WORK) );
  GFL_STD_MemClear( p_wk, sizeof(BR_SIDEBAR_WORK));

  //リソース読み込み
  BR_RES_LoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_M, heapID );
  BR_RES_LoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_S, heapID );

  //サイドバーの初期化
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    { 
      BR_SIDEBAR_ONE_Init( &p_wk->sidebar[i], p_clunit, p_res, &sc_sidebar_data[i], heapID );
    }
  }

  return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー  破棄
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_Exit( BR_SIDEBAR_WORK *p_wk, BR_RES_WORK *p_res )
{ 
  //サイドバーの破棄
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( BR_SIDEBAR_ONE_IsExist( &p_wk->sidebar[i]) )
      { 
        BR_SIDEBAR_ONE_Exit( &p_wk->sidebar[i] );
      }
    }
  }

  //リソース開放
  BR_RES_UnLoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_M );
  BR_RES_UnLoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_S );

  GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー  メイン処理
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_Main( BR_SIDEBAR_WORK *p_wk )
{ 
  //サイドバーメイン処理
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( BR_SIDEBAR_ONE_IsExist( &p_wk->sidebar[i]) )
      { 
        BR_SIDEBAR_ONE_Main( &p_wk->sidebar[i] );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動時の動き開始リクエスト
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_StartBoot( BR_SIDEBAR_WORK *p_wk )
{ 
 //サイドバー起動モードへ
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( BR_SIDEBAR_ONE_IsExist( &p_wk->sidebar[i]) )
      { 
        BR_SIDEBAR_ONE_SetMoveMode( &p_wk->sidebar[i], BR_SIDEBAR_ONE_MOVE_MODE_BOOT );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  通常時の動き開始リクエスト
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_StartShake( BR_SIDEBAR_WORK *p_wk )
{ 
 //サイドバー起動モードへ
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( BR_SIDEBAR_ONE_IsExist( &p_wk->sidebar[i]) )
      { 
        BR_SIDEBAR_ONE_SetMoveMode( &p_wk->sidebar[i], BR_SIDEBAR_ONE_MOVE_MODE_SHAKE );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  終了リクエスト
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_StartClose( BR_SIDEBAR_WORK *p_wk )
{ 
 //サイドバー起動モードへ
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( BR_SIDEBAR_ONE_IsExist( &p_wk->sidebar[i]) )
      { 
        BR_SIDEBAR_ONE_SetMoveMode( &p_wk->sidebar[i], BR_SIDEBAR_ONE_MOVE_MODE_END );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  メイン画面のリソース破棄
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_UnLoadMain( BR_SIDEBAR_WORK *p_wk, BR_RES_WORK *p_res )
{
  //サイドバーの破棄
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( sc_sidebar_data[i].draw_type == CLSYS_DRAW_MAIN )
      { 
        BR_SIDEBAR_ONE_Exit( &p_wk->sidebar[i] );
      }
    }
  }

  //リソース破棄
  BR_RES_UnLoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_M );
}
//----------------------------------------------------------------------------
/**
 *	@brief  メイン画面のリソース読み込み
 *
 *	@param	BR_SIDEBAR_WORK *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void BR_SIDEBAR_LoadMain( BR_SIDEBAR_WORK *p_wk, GFL_CLUNIT *p_clunit, BR_FADE_WORK *p_fade, BR_RES_WORK *p_res, HEAPID heapID )
{ 
  //リソース読み込み
  BR_RES_LoadOBJ( p_res, BR_RES_OBJ_SIDEBAR_M, heapID );

  //サイドバーの作成
  { 
    int i;
    for( i = 0; i < BR_SIDEBAR_MAX; i++ )
    {
      if( sc_sidebar_data[i].draw_type == CLSYS_DRAW_MAIN )
      { 
        BR_SIDEBAR_ONE_Init( &p_wk->sidebar[i], p_clunit, p_res, &sc_sidebar_data[i], heapID );
      }
    }
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  動作完了チェック
 *
 *	@param	const BR_SIDEBAR_WORK *cp_wk  ワーク
 *
 *	@return TRUEで動作終了  FALSEで動作中
 */
//-----------------------------------------------------------------------------
BOOL BR_SIDEBAR_IsMoveEnd( const BR_SIDEBAR_WORK *cp_wk )
{ 
  int i;
  BOOL is_end  = TRUE;

  for( i = 0; i < BR_SIDEBAR_MAX; i++ )
  {
    if( BR_SIDEBAR_ONE_IsExist( &cp_wk->sidebar[i]) )
    { 
      is_end &= BR_SIDEBAR_ONE_IsMoveNone( &cp_wk->sidebar[i]);
    }
  }

  return is_end;
}

//=============================================================================
/**
 *    サイドバー１つ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ　初期化
 *
 *	@param	BR_SIDEBAR_ONE *p_wk  ワーク
 *	@param	*p_clunit             セル作成用ユニット
 *	@param	*p_res                リソース管理
 *	@param	BR_SIDEBAR_DATA *cp_data  データ
 *	@param	heapID                    ヒープ
 */
//-----------------------------------------------------------------------------
static void BR_SIDEBAR_ONE_Init( BR_SIDEBAR_ONE *p_wk, GFL_CLUNIT *p_clunit, const BR_RES_WORK *cp_res, const BR_SIDEBAR_DATA *cp_data, HEAPID heapID )
{ 
  //初期化
  GFL_STD_MemClear( p_wk, sizeof(BR_SIDEBAR_ONE) );
  p_wk->data  = *cp_data;

  //CLWK作成
  { 
    BR_RES_OBJ_DATA res;

    //リソース取得
    { 
      BOOL ret;
      BR_RES_OBJID objID;

      if( cp_data->draw_type == CLSYS_DRAW_MAIN )
      { 
        objID = BR_RES_OBJ_SIDEBAR_M;
      }
      else
      { 
        objID = BR_RES_OBJ_SIDEBAR_S;
      }
      ret = BR_RES_GetOBJRes( cp_res, objID, &res );
      GF_ASSERT( ret );
    }

    //CLWK作成
    { 
      GFL_CLWK_DATA data;
      GFL_STD_MemClear( &data, sizeof(GFL_CLWK_DATA) );
      data.pos_x  = cp_data->boot_init_x;
      data.pos_y  = cp_data->boot_init_y;
      p_wk->x = FX32_CONST( data.pos_x );
      p_wk->y = FX32_CONST( data.pos_y );
      p_wk->scale = cp_data->boot_scale_x;

      p_wk->p_clwk  = GFL_CLACT_WK_Create( p_clunit,
          res.ncg, res.ncl, res.nce,
          &data, p_wk->data.draw_type, heapID );

      GFL_CLACT_WK_SetAffineParam( p_wk->p_clwk, CLSYS_AFFINETYPE_DOUBLE );
      GFL_CLACT_WK_SetTypeScale( p_wk->p_clwk, p_wk->scale, CLSYS_MAT_X );
      GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, FALSE );
    }

  }

  p_wk->is_exist  = TRUE;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  破棄
 *
 *	@param	BR_SIDEBAR_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void BR_SIDEBAR_ONE_Exit( BR_SIDEBAR_ONE *p_wk )
{ 

  GFL_CLACT_WK_Remove( p_wk->p_clwk );
  GFL_STD_MemClear( p_wk, sizeof(BR_SIDEBAR_ONE) );
}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  メイン処理
 *
 *	@param	BR_SIDEBAR_ONE *p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void BR_SIDEBAR_ONE_Main( BR_SIDEBAR_ONE *p_wk )
{ 
  if( p_wk->move_function )
  { 
    p_wk->move_function( p_wk );
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  存在しているかチェック
 *
 *	@param	const BR_SIDEBAR_ONE *cp_wk ワーク
 *
 *	@return TRUEならば存在  FALSEならば存在していない
 */
//-----------------------------------------------------------------------------
static BOOL BR_SIDEBAR_ONE_IsExist( const BR_SIDEBAR_ONE *cp_wk )
{ 
  return cp_wk->is_exist;
}
//----------------------------------------------------------------------------
/**
 *	@brief  動作しているかチェック
 *
 *	@param	const BR_SIDEBAR_ONE *cp_wk   ワーク
 *
 *	@return TRUEで動作していない  FALSEで動作中
 */
//-----------------------------------------------------------------------------
static BOOL BR_SIDEBAR_ONE_IsMoveNone( const BR_SIDEBAR_ONE *cp_wk )
{ 
  return cp_wk->move_function == NULL;
}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  移動モードセット
 *
 *	@param	BR_SIDEBAR_ONE *p_wk  ワーク
 *	@param	mode                  移動モード
 */
//-----------------------------------------------------------------------------
static void BR_SIDEBAR_ONE_SetMoveMode( BR_SIDEBAR_ONE *p_wk, BR_SIDEBAR_ONE_MOVE_MODE mode )
{ 
  switch( mode )
  { 
  case BR_SIDEBAR_ONE_MOVE_MODE_NONE: //何も行わない
    p_wk->move_function = NULL;
    break;

  case BR_SIDEBAR_ONE_MOVE_MODE_BOOT:  //起動時の動き
    p_wk->move_function = Br_SideBar_One_Move_Boot;
    break;

  case BR_SIDEBAR_ONE_MOVE_MODE_SHAKE: //横でゆらゆら揺れている動き
    p_wk->move_function = Br_SideBar_One_Move_Shake;
    break;

  case BR_SIDEBAR_ONE_MOVE_MODE_END:  //終了時
    p_wk->move_function = Br_SideBar_One_Move_End;
    break;
  }

  p_wk->state = 0;
}
//-------------------------------------
///	移動関数
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  起動時の移動
 *
 *	@param	BR_SIDEBAR_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_SideBar_One_Move_Boot( BR_SIDEBAR_ONE *p_wk )
{ 
  enum 
  { 
    SEQ_INIT,
    SEQ_WAIT,
    SEQ_MOVE1,
    SEQ_MOVE2,
    SEQ_END,
  };

  static const int SIDE_BAR_WAIT        =  3;
  static const int SIDE_BAR_SIN_MOVE_F  = 4;

  p_wk->vconst++;
  p_wk->vconst %=2;
  if( p_wk->vconst )
  { 
    return;
  }

  switch( p_wk->state )
  { 
  case SEQ_INIT:
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
    p_wk->wait  = 0;
    p_wk->scale = p_wk->data.boot_scale_x;
    p_wk->state = SEQ_WAIT;
    break;

  case SEQ_WAIT:
    if ( p_wk->wait == SIDE_BAR_WAIT ){
      p_wk->state   = SEQ_MOVE1;
      p_wk->init_x  = FX32_CONST( GFL_CLACT_WK_GetTypePos( p_wk->p_clwk, p_wk->data.draw_type, CLSYS_MAT_X ) );
      p_wk->wait  = 0;
    }
    else {
      p_wk->wait++;
    }
    break;

	case SEQ_MOVE1:
		{	
      const s8 move = (p_wk->data.dir == BR_SIDEBAR_DIR_LEFT ) ? -1: +1;

			if ( p_wk->wait == SIDE_BAR_SIN_MOVE_F ){
				p_wk->state = SEQ_MOVE2;
				p_wk->wait  = 0;
			}
			else {				
				
        p_wk->init_x = p_wk->init_x + ( ( GFL_CALC_Sin360R( GFL_CALC_GET_ROTA_NUM( p_wk->deg ) ) * 1 ) * ( move * -1 ) );


				
				p_wk->deg += 32;
				p_wk->deg %= 360;
				
        GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, p_wk->init_x >> FX32_SHIFT, p_wk->data.draw_type, CLSYS_MAT_X );
				
				p_wk->wait++;
			}	
		}
		break;
	
	case SEQ_MOVE2:
		{
			s16 x;

      x = GFL_CLACT_WK_GetTypePos( p_wk->p_clwk, p_wk->data.draw_type, CLSYS_MAT_X );
			
			if ( p_wk->wait == 0 ){
				
				p_wk->val_x  = p_wk->data.boot_end_x - x;
				p_wk->val_x /= p_wk->data.boot_move_f;
				p_wk->wait++;
			}
			else if ( p_wk->wait == p_wk->data.boot_move_f + 1 ){
				x = p_wk->data.boot_end_x;
        p_wk->state = SEQ_END;
			}
			else {
				x += p_wk->val_x;
				p_wk->wait++;
			}
			
      GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, x, p_wk->data.draw_type, CLSYS_MAT_X );
		}	
		break;
  case SEQ_END:
    BR_SIDEBAR_ONE_SetMoveMode( p_wk, BR_SIDEBAR_ONE_MOVE_MODE_SHAKE );
    break;
  }

  if ( p_wk->state == SEQ_MOVE2 && p_wk->scale > FX32_CONST(0.1f) ){
		 p_wk->scale -= FX32_CONST(0.10f);
		 
     GFL_CLACT_WK_SetTypeScale( p_wk->p_clwk, p_wk->scale, CLSYS_MAT_X );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  よこでゆらゆら揺れる移動
 *
 *	@param	BR_SIDEBAR_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_SideBar_One_Move_Shake( BR_SIDEBAR_ONE *p_wk )
{ 
  enum 
  { 
    SEQ_INIT,
    SEQ_MAIN,
  };

  p_wk->vconst++;
  p_wk->vconst %=2;
  if( p_wk->vconst )
  { 
    return;
  }

  switch( p_wk->state )
  { 
  case SEQ_INIT:
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
    GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, p_wk->data.boot_end_x, p_wk->data.draw_type, CLSYS_MAT_X );
    //GFL_CLACT_WK_SetTypeScale( p_wk->p_clwk, FX32_CONST(0.1f), CLSYS_MAT_X );
    p_wk->init_x  = FX32_CONST( p_wk->data.boot_end_x );
    p_wk->deg     = p_wk->data.shake_init_deg;
    p_wk->state   = SEQ_MAIN;
    break;

  case SEQ_MAIN:
    { 
      const s8 move = (p_wk->data.dir == BR_SIDEBAR_DIR_LEFT ) ? -1: +1;
      fx32 now;

      //なぜか、度からラジアンにした値を、さらにラジアンに変換する式を用いて、Sin値を取り出しているが、
      //もともとのバトルレコーダーでもこうなっていた。再現のため、そのまま。
      now = p_wk->init_x + ( ( GFL_CALC_Sin360R( GFL_CALC_GET_ROTA_NUM( p_wk->deg ) ) * 3 ) * ( move * -1 ) );
      //NAGI_Printf( "0x%x\n", ( ( GFL_CALC_Sin360R( GFL_CALC_GET_ROTA_NUM( p_wk->deg ) ) * 3 ) * ( move * -1 ) ) );

      p_wk->deg += p_wk->data.shake_deg_dir;
      p_wk->deg %= 360;

      GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, now >> FX32_SHIFT, p_wk->data.draw_type, CLSYS_MAT_X );
    }
    break;
  }

  if (  p_wk->scale > FX32_CONST(0.1f) ){
		 p_wk->scale -= FX32_CONST(0.10f);
		 
     GFL_CLACT_WK_SetTypeScale( p_wk->p_clwk, p_wk->scale, CLSYS_MAT_X );
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief  サイドバー１つ  終了時
 *
 *	@param	BR_SIDEBAR_ONE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
static void Br_SideBar_One_Move_End( BR_SIDEBAR_ONE *p_wk )
{ 
 enum 
  { 
    SEQ_INIT,
    SEQ_MOVE,
    SEQ_END,
  };

  static const int SIDE_BAR_WAIT        = 3;
  static const int SIDE_BAR_SIN_MOVE_F  = 4;

  p_wk->vconst++;
  p_wk->vconst %=2;
  if( p_wk->vconst )
  { 
    return;
  }

  switch( p_wk->state )
  { 
  case SEQ_INIT:
    GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk, TRUE );
    p_wk->wait  = 0;
    p_wk->state = SEQ_MOVE;
    break;

	case SEQ_MOVE:
    {
      s16 x;
      fx32 scale;

      x     = GFL_CLACT_WK_GetTypePos( p_wk->p_clwk, p_wk->data.draw_type, CLSYS_MAT_X );
      scale = GFL_CLACT_WK_GetTypeScale( p_wk->p_clwk, CLSYS_MAT_X );

      if ( p_wk->wait == 0 ){

        p_wk->val_x  = p_wk->data.boot_init_x - x;
        p_wk->val_x /= p_wk->data.boot_move_f;
        p_wk->wait++;

        p_wk->val_scale = p_wk->data.boot_scale_x - scale;
        p_wk->val_scale /= p_wk->data.boot_move_f;
      }
      else if ( p_wk->wait == p_wk->data.boot_move_f + 1 ){
        x = p_wk->data.boot_init_x;
        scale = p_wk->data.boot_scale_x;
        p_wk->state = SEQ_END;
      }
      else {
				x += p_wk->val_x;
        scale += p_wk->val_scale;
        p_wk->wait++;
      }
			
      GFL_CLACT_WK_SetTypePos( p_wk->p_clwk, x, p_wk->data.draw_type, CLSYS_MAT_X );
      GFL_CLACT_WK_SetTypeScale( p_wk->p_clwk, scale, CLSYS_MAT_X );
		}	
		break;
	
	case SEQ_END:
    BR_SIDEBAR_ONE_SetMoveMode( p_wk, BR_SIDEBAR_ONE_MOVE_MODE_NONE );
		break;
  }

}
