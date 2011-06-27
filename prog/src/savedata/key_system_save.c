//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		key_system_save.h.c
 *	@brief  キーシステムセーブ
 *	@author	Toru=Nagihashi
 *	@data		2011.06.01
 *
 *	@note   キーシステムは本来単純なフラグのセーブだけですみますが、
 *	        PAR対策を考えて単純にフラグのONOFFで開放されないように
 *	        マジックキーをセーブしています。
 *	        暗号化はXORを使っていますが、そのもととなる数字には
 *	        乱数を使っています。
 *	        これはトレーナーID（内部セーブ作成前のセーブされる）や
 *	        MACアドレス（本体を変えると消えてしまう）が使用できないためです。
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#include <gflib.h>
#include "system/gfl_use.h"
#include "savedata/key_system_save.h"
//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	持っているキーのマジックキー（固有ならなんでもよい。ここでは円周率縦読み）
//=====================================
static const u32 sc_have_magic_num[KEY_SYSTEM_TYPE_MAX] =
{
  0x35691,//KEY_SYSTEM_TYPE_SELF_DIFFICULT,     //難易度変更
  0x18256,//KEY_SYSTEM_TYPE_SELF_CLOSEDOOR,     //開かずの扉
  0x59389,//KEY_SYSTEM_TYPE_SELF_INFINITYDUNGEON, //無限ダンジョン
};
//-------------------------------------
///	要素開放されたもののマジックキー（固有ならなんでもよい。ここでは円周率縦読み）
//=====================================
static const u32 sc_use_magic_num[KEY_SYSTEM_TYPE_MAX] =
{
  0x93389,//KEY_SYSTEM_TYPE_RECV_DIFFICULT,     //難易度変更
  0x22843,//KEY_SYSTEM_TYPE_RECV_CLOSEDOOR,     //開かずの扉
  0x34771,//KEY_SYSTEM_TYPE_RECV_INFINITYDUNGEON, //無限ダンジョン
};

//-------------------------------------
///	難易度のマジックキー（固有ならなんでもよい。ここでは円周率縦読み）
//=====================================
static const u32 sc_difficult_magic_num[KEY_SYSTEM_DIFFICULT_MAX] =
{
  0x31239,//KEY_SYSTEM_DIFFICULT_NORMAL,  //通常
  0x15657,//KEY_SYSTEM_DIFFICULT_EASY,    //簡単
  0x49589,//KEY_SYSTEM_DIFFICULT_HARD,    //難しい
};

//-------------------------------------
///	WFBCのマジックキー（固有ならなんでもよい。ここでは円周率縦読み）
//=====================================
static const u32 sc_wfbc_magic_num[KEY_SYSTEM_WFBC_MAX] =
{
  0x34525,//KEY_SYSTEM_WFBC_SELF,     //自分のバージョンの街
  0x11963,//KEY_SYSTEM_WFBC_REVERSE,  //反対バージョンの街
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	キーシステムセーブデータ
//=====================================
struct _KEY_SYSTEM_SAVE
{
  u32                   have[KEY_SYSTEM_TYPE_MAX];
  u32                   use[KEY_SYSTEM_TYPE_MAX];
  u32                   diffcult;
  u32                   wfbc;
  u32                   seed;
};

SDK_COMPILER_ASSERT( KEY_SYSTEM_SAVE_WORK_SIZE == sizeof(KEY_SYSTEM_SAVE) );

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief ワークサイズ取得
 *
 *	@return サイズ
 */
//-----------------------------------------------------------------------------
u32 KEY_SYSTEM_SAVE_GetWorkSize( void )
{
  return sizeof( KEY_SYSTEM_SAVE);
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブ初期化
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_Init( KEY_SYSTEM_SAVE *p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(KEY_SYSTEM_SAVE) );
  p_wk->seed = GFUser_GetPublicRand(0xFFFFFFFF);
}

//----------------------------------------------------------------------------
/**
 *	@brief　キーシステムセーブ  キーを所持にセット
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 *	@param	type  タイプ
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_SetHave( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 masic_num = sc_have_magic_num[ type ];
    u32 key = masic_num ^ p_wk->seed; //xorで暗号化

    p_wk->have[ type ] = key;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブ  キーをもっているかチェック
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_wk  ワーク
 *	@param	type 　タイプ
 *
 *	@return　TRUEならばそのキーがある FALSEならばない
 */
//-----------------------------------------------------------------------------
BOOL KEY_SYSTEM_SAVE_IsHave( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 key = cp_wk->have[ type ];
    u32 masic_num = key ^ cp_wk->seed;  //xorで復号化

    return masic_num == sc_have_magic_num[ type ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief　キーシステムセーブ  項目を開放
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 *	@param	type  タイプ
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_SetUse( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 masic_num = sc_use_magic_num[ type ];
    u32 key = masic_num ^ p_wk->seed; //xorで暗号化

    p_wk->use[ type ] = key;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブ  項目開放しているかチェック
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_wk  ワーク
 *	@param	type 　タイプ
 *
 *	@return　TRUEならばそのキーがある FALSEならばない
 */
//-----------------------------------------------------------------------------
BOOL KEY_SYSTEM_SAVE_IsUse( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 key = cp_wk->use[ type ];
    u32 masic_num = key ^ cp_wk->seed;  //xorで復号化

    return masic_num == sc_use_magic_num[ type ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief　難易度を設定
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 *	@param	diffcult              設定する難易度
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_SetDifficult( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_DIFFICULT diffcult )
{
  GF_ASSERT( diffcult < KEY_SYSTEM_DIFFICULT_MAX );
  {
    u32 masic_num = sc_difficult_magic_num[ diffcult ];
    u32 key = masic_num ^ p_wk->seed; //xorで暗号化

    p_wk->diffcult = key;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  難易度を取得
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_wk ワーク
 *
 *	@return 難易度
 */
//-----------------------------------------------------------------------------
KEY_SYSTEM_DIFFICULT KEY_SYSTEM_SAVE_GetDifficult( const KEY_SYSTEM_SAVE *cp_wk )
{
  int i;

  for( i = 0; i < KEY_SYSTEM_DIFFICULT_MAX; i ++ )
  {
    u32 key = cp_wk->diffcult;
    u32 masic_num = key ^ cp_wk->seed;  //xorで復号化

    if( masic_num == sc_difficult_magic_num[ i ] )
    {
      return i;
    }
  }

  return KEY_SYSTEM_DIFFICULT_NORMAL;
}

//----------------------------------------------------------------------------
/**
 *	@brief　WFBCを設定
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 *	@param	wfbc              設定するWFBC
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_SetWfbc( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_WFBC wfbc )
{
  GF_ASSERT( wfbc < KEY_SYSTEM_WFBC_MAX );
  {
    u32 masic_num = sc_wfbc_magic_num[ wfbc ];
    u32 key = masic_num ^ p_wk->seed; //xorで暗号化

    p_wk->wfbc = key;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  WFBCを取得
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_wk ワーク
 *
 *	@return WFBC
 */
//-----------------------------------------------------------------------------
KEY_SYSTEM_WFBC KEY_SYSTEM_SAVE_GetWfbc( const KEY_SYSTEM_SAVE *cp_wk )
{
  int i;

  for( i = 0; i < KEY_SYSTEM_WFBC_MAX; i ++ )
  {
    u32 key = cp_wk->wfbc;
    u32 masic_num = key ^ cp_wk->seed;  //xorで復号化

    if( masic_num == sc_wfbc_magic_num[ i ] )
    {
      return i;
    }
  }

  return KEY_SYSTEM_WFBC_SELF;
}

#ifdef PM_DEBUG
//----------------------------------------------------------------------------
/**
 *	@brief  キーの設定
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk
 *	@param	type
 *	@param	is_exits 
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_DEBUG_SetHave( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type, BOOL is_exits )
{
  if( is_exits )
  {
    KEY_SYSTEM_SAVE_SetHave( p_wk, type );
  }
  else
  {
    p_wk->have[ type ] = 0;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief 項目の設定
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk
 *	@param	type
 *	@param	is_exits 
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_DEBUG_SetUse( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type, BOOL is_exits )
{
  if( is_exits )
  {
    KEY_SYSTEM_SAVE_SetUse( p_wk, type );
  }
  else
  {
    p_wk->use[ type ] = 0;
  }
}
#endif 
