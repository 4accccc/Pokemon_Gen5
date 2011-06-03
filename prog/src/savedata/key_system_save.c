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
///	フラグのマジックキー（固有ならなんでもよい。ここでは円周率縦読み）
//=====================================
static const u32 sc_key_magic_num[KEY_SYSTEM_TYPE_MAX] =
{
  0x35691,//KEY_SYSTEM_TYPE_SELF_DIFFICULT,     //難易度変更
  0x18256,//KEY_SYSTEM_TYPE_SELF_CLOSEDOOR,     //開かずの扉
  0x49609,//KEY_SYSTEM_TYPE_SELF_CELEBRATION,   //祭り
  0x17423,//KEY_SYSTEM_TYPE_SELF_BGMCHANGE,     //BGM変更
  0x59389,//KEY_SYSTEM_TYPE_SELF_INFINITYDUNGEON, //無限ダンジョン
  0x93389,//KEY_SYSTEM_TYPE_RECV_DIFFICULT,     //難易度変更
  0x22843,//KEY_SYSTEM_TYPE_RECV_CLOSEDOOR,     //開かずの扉
  0x63317,//KEY_SYSTEM_TYPE_RECV_CELEBRATION,   //祭り
  0x58295,//KEY_SYSTEM_TYPE_RECV_BGMCHANGE,     //BGM変更
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
  u32                   key[KEY_SYSTEM_TYPE_MAX];
  u32                   diffcult;
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
 *	@brief　キーシステムセーブ  キーをセット
 *
 *	@param	KEY_SYSTEM_SAVE *p_wk ワーク
 *	@param	type  タイプ
 */
//-----------------------------------------------------------------------------
void KEY_SYSTEM_SAVE_SetKey( KEY_SYSTEM_SAVE *p_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 masic_num = sc_key_magic_num[ type ];
    u32 key = masic_num ^ p_wk->seed; //xorで暗号化

    p_wk->key[ type ] = key;
  }
}
//----------------------------------------------------------------------------
/**
 *	@brief  キーシステムセーブ  キーがあるかチェック
 *
 *	@param	const KEY_SYSTEM_SAVE *cp_wk  ワーク
 *	@param	type 　タイプ
 *
 *	@return　TRUEならばそのキーがある FALSEならばない
 */
//-----------------------------------------------------------------------------
BOOL KEY_SYSTEM_SAVE_IsExits( const KEY_SYSTEM_SAVE *cp_wk, KEY_SYSTEM_TYPE type )
{
  GF_ASSERT( type < KEY_SYSTEM_TYPE_MAX );
  {
    u32 key = cp_wk->key[ type ];
    u32 masic_num = key ^ cp_wk->seed;  //xorで復号化

    return masic_num == sc_key_magic_num[ type ];
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

    if( masic_num == sc_key_magic_num[ i ] )
    {
      return i;
    }
  }

  return KEY_SYSTEM_DIFFICULT_NORMAL;
}
