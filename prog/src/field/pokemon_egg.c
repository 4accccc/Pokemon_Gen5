//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケモンタマゴ
 * @file   pokemon_egg.c
 * @author obata
 * @date   2010.04.21
 */
////////////////////////////////////////////////////////////////////////////////////////// 
#include <gflib.h> 
#include "gamesystem/game_data.h"
#include "savedata/mystatus.h"
#include "system/gfl_use.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/status_rcv.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_personal.h"
#include "poke_tool/poke_memo.h"
#include "poke_tool/tokusyu_def.h"// for TOKUSYU_xxxx
#include "item/itemsym.h"         // for ITEM_xxxx
#include "item/item.h"            // for ITEM_xxxx
#include "waza_tool/wazano_def.h" // for WAZANO_xxxx
#include "print/global_msg.h"     // for GlobalMsg_PokeName 
#include "arc/arc_def.h"          // for ARCID_PMSTBL

#include "pokemon_egg.h"

#include "bugfix.h"

#define DEBUG_PRINT_ON      // デバッグ出力スイッチ
#define PRINT_TARGET    (2) // デバッグ出力先
#define RARE_EGG_CHANCE (5) // レアタマゴ抽選回数

// 両親ポケモン
typedef struct {
  const POKEMON_PARAM* father;
  const POKEMON_PARAM* mother;
} PARENT;

// タマゴのパラメータ
typedef struct {
  u32 monsno;
  u32 formno;
  u32 level;
  u32 seikaku;
  u32 tokusei;
  u32 friend;
  u32 ability_rnd[ PTL_ABILITY_MAX ];
  u32 id_no;
  u32 personal_rnd;
  u32 waza_num;
  u32 wazano[ PTL_WAZA_MAX ];

  u32 tokusei_idx; // 何番目の特性か[0, 2]
  BOOL derive_seikaku_from_father;
  BOOL derive_seikaku_from_mother;
  BOOL derive_ability_from_father[ PTL_ABILITY_MAX ];
  BOOL derive_ability_from_mother[ PTL_ABILITY_MAX ];
  u32 derive_ability_num;
} EGG_PARAM;


// prototype /////////////////////////////////////////////////////////////////////////////

// 父母の決定
static void CorrectParent_bySex( PARENT* parent );
static void CorrectParent_byMetamon( PARENT* parent );
static void ReverseParent( PARENT* parent );
// タマゴパラメータの決定
static void EggCordinate_init( EGG_PARAM* egg );
static void EggCordinate_MonsNo_basic( const PARENT* parent, EGG_PARAM* egg ); // モンスターNo.
static void EggCordinate_MonsNo_nidoran( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_MonsNo_barubiito( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_MonsNo_fione( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_MonsNo_baby( const PARENT* parent, EGG_PARAM* egg );
static const POKEMON_PARAM* GetBasePokemon_for_monsno( const PARENT* parent );
static void EggCordinate_Form_basic( const PARENT* parent, EGG_PARAM* egg ); // フォルム
static void EggCordinate_Form_karanakusi( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_Form_minomutti( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_Form_basurao( const PARENT* parent, EGG_PARAM* egg );
static const POKEMON_PARAM* GetBasePokemon_for_form( const PARENT* parent );
static void EggCordinate_Seikaku_basic( const PARENT* parent, EGG_PARAM* egg ); // 性格
static void EggCordinate_Seikaku_kawarazunoisi( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_TokuseiIndex_basic( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id ); // 特性
static void EggCordinate_TokuseiIndex_metamon( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_Tokusei( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id );
static void EggCordinate_Friend( EGG_PARAM* egg, HEAPID heap_id ); // なつき度 ( 孵化カウント )
static void EggCordinate_AbilityRnd_select_by_item( const PARENT* parent, EGG_PARAM* egg ); // 個体乱数
static void EggCordinate_AbilityRnd_select_at_random( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_AbilityRnd_deirve_from_parent( const PARENT* parent, EGG_PARAM* egg );
static void EggCordinate_ID( const MYSTATUS* mystatus, EGG_PARAM* egg ); // ID
static void EggCordinate_PersonalRnd_at_random( const PARENT* parent, EGG_PARAM* egg ); // 個性乱数
static void EggCordinate_PersonalRnd_for_rare( const PARENT* parent, EGG_PARAM* egg );
static u32 GetPersonalRnd_at_random( void );
static void EggCordinate_Waza_default( EGG_PARAM* egg ); // デフォルト技セット
static void EggCordinate_Waza_egg( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id ); // タマゴ技セット
static void EggCordinate_Waza_parent( const PARENT* parent, EGG_PARAM* egg ); // 継承技 ( 両親 ) セット
static void EggCordinate_Waza_machine( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id ); // 継承技 ( 技マシン ) セット
static void EggCordinate_Waza_pityuu( const PARENT* parent, EGG_PARAM* egg ); // 特殊タマゴ処理 ( ピチュー )
static void PushWaza_to_EggParam( EGG_PARAM* egg, u32 wazano );
static void MakeEgg( POKEMON_PARAM* egg, const EGG_PARAM* egg_param, const MYSTATUS* owner, int memo, HEAPID heap_id );
// ユーティリティ
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno ); // 技マシンが使えるかどうかをチェックする
static u16 GetSeedMonsNo( u16 monsno ); // 種ポケモンを取得する
static u8 GetTokuseiIndex( u16 monsno, u16 formno, u16 tokusei, HEAPID heap_id ); // 何番目の特性なのかを取得する
// デバッグ
#ifdef PM_DEBUG
static POKEMON_PARAM* CreateDebugEgg( GAMEDATA* gameData, HEAPID heapID, u32 monsno ); // デバッグ用タマゴを作成する
#endif



// publid functions //////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを作成する
 *
 * @param heap_id  使用するヒープID
 * @param mystatus タマゴの所有者
 * @param memo     トレーナーメモの記載
 * @param poke1    親ポケモン1
 * @param poke2    親ポケモン2
 * @param egg      作成したタマゴの格納先
 */
//---------------------------------------------------------------------------------------- 
void POKEMON_EGG_Create( 
    HEAPID heap_id, const MYSTATUS* mystatus, int memo, 
    const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2, POKEMON_PARAM* egg )
{
  PARENT parent;
  EGG_PARAM egg_param;

  // 父母を決定
  parent.father = poke1;
  parent.mother = poke2;
  CorrectParent_bySex( &parent );
  CorrectParent_byMetamon( &parent );

  // タマゴのパラメータを決定
  EggCordinate_init( &egg_param );
  EggCordinate_MonsNo_basic( &parent, &egg_param ); // モンスターナンバー
  EggCordinate_MonsNo_nidoran( &parent, &egg_param );
  EggCordinate_MonsNo_barubiito( &parent, &egg_param );
  EggCordinate_MonsNo_fione( &parent, &egg_param );
  EggCordinate_MonsNo_baby( &parent, &egg_param ); 
  EggCordinate_Form_basic( &parent, &egg_param ); // フォルム
  EggCordinate_Form_karanakusi( &parent, &egg_param ); 
  EggCordinate_Form_minomutti( &parent, &egg_param );
  EggCordinate_Form_basurao( &parent, &egg_param );
  EggCordinate_Seikaku_basic( &parent, &egg_param ); // 性格
  EggCordinate_Seikaku_kawarazunoisi( &parent, &egg_param ); 
  EggCordinate_TokuseiIndex_basic( &parent, &egg_param, heap_id ); // 特性
  EggCordinate_TokuseiIndex_metamon( &parent, &egg_param );
  EggCordinate_Tokusei( &parent, &egg_param, heap_id );
  EggCordinate_Friend( &egg_param, heap_id ); // なつき度 ( 孵化カウント )
  EggCordinate_AbilityRnd_select_by_item( &parent, &egg_param ); // 個体乱数
  EggCordinate_AbilityRnd_select_at_random( &parent, &egg_param );
  EggCordinate_AbilityRnd_deirve_from_parent( &parent, &egg_param );
  EggCordinate_ID( mystatus, &egg_param ); // ID
  EggCordinate_PersonalRnd_at_random( &parent, &egg_param ); // 個性乱数
  EggCordinate_PersonalRnd_for_rare( &parent, &egg_param ); 
  EggCordinate_Waza_default( &egg_param ); // 技 ( デフォルト ) 
  EggCordinate_Waza_egg( &parent, &egg_param, heap_id ); // 技 ( タマゴ技 )
  EggCordinate_Waza_parent( &parent, &egg_param ); // 技 ( 両親共通の技 ) 
  EggCordinate_Waza_machine( &parent, &egg_param, heap_id ); // 技 ( 父のマシン技 ) 
  EggCordinate_Waza_pityuu( &parent, &egg_param ); // 技 ( ピチュー )

  // タマゴを作成
  MakeEgg( egg, &egg_param, mystatus, memo, heap_id ); 
}



// private functions /////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母の組み合わせを訂正する ( 性別 )
 */
//---------------------------------------------------------------------------------------- 
static void CorrectParent_bySex( PARENT* parent )
{
  u32 sex_father, sex_mother;

  // 父親チェック
  sex_father = PP_Get( parent->father, ID_PARA_sex, NULL );
  sex_mother = PP_Get( parent->mother, ID_PARA_sex, NULL ); 
  if( sex_father == PTL_SEX_FEMALE ) {
    ReverseParent( parent );
  }

  // 母親チェック
  sex_father = PP_Get( parent->father, ID_PARA_sex, NULL );
  sex_mother = PP_Get( parent->mother, ID_PARA_sex, NULL ); 
  if( sex_mother == PTL_SEX_MALE ) {
    ReverseParent( parent );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母の組み合わせを訂正する ( メタモン )
 *
 * メタモン × ♂ ==> ( 父, 母 ) = ( ♂, メタモン )
 * メタモン × ♀ ==> ( 父, 母 ) = ( メタモン, ♀ )
 * メタモン × ？ ==> ( 父, 母 ) = ( ？, メタモン )
 */
//---------------------------------------------------------------------------------------- 
static void CorrectParent_byMetamon( PARENT* parent )
{
  u32 monsno_father;
  u32 sex_mother;

  monsno_father = PP_Get( parent->father, ID_PARA_monsno, NULL );
  sex_mother = PP_Get( parent->mother, ID_PARA_sex, NULL );

  if( (monsno_father == MONSNO_METAMON) && (sex_mother == PTL_SEX_UNKNOWN) ) {
    ReverseParent( parent );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父母を入れ替える
 */
//---------------------------------------------------------------------------------------- 
static void ReverseParent( PARENT* parent )
{
  const POKEMON_PARAM* temp = parent->father;
  parent->father = parent->mother;
  parent->mother = temp;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのパラメータを初期化する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_init( EGG_PARAM* egg )
{
  GFL_STD_MemClear( egg, sizeof(EGG_PARAM) );
  egg->level = 1;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する ( 基本ルール )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_MonsNo_basic( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_monsno( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );
  egg->monsno = GetSeedMonsNo( base_monsno );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する ( 例外ケース：ニドラン )
 *
 * ニドラン♀
 * ニドラン♂
 * ニドリーノ
 * ニドキング　からは, ニドラン♂ ( 50% ) or ニドラン♀ ( 50% ) が産まれる
 *
 * ※ニドリーナ
 * 　ニドクイン　はタマゴグループが『無生殖』なので, タマゴは発見されない
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_MonsNo_nidoran( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_monsno( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( (base_monsno == MONSNO_NIDORAN_F) ||
      (base_monsno == MONSNO_NIDORAN_M) ||
      (base_monsno == MONSNO_NIDORIINO) ||
      (base_monsno == MONSNO_NIDOKINGU) ) {
    if( GFUser_GetPublicRand0(2) == 0 ) {
      egg->monsno = MONSNO_NIDORAN_F;
    }
    else {
      egg->monsno = MONSNO_NIDORAN_M;
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する ( 例外ケース：バルビート, イルミーゼ )
 *
 * バルビート
 * イルミーゼ からは, バルビート ( 50% ) or イルミーゼ ( 50% ) が産まれる
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_MonsNo_barubiito( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_monsno( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( (base_monsno == MONSNO_BARUBIITO) ||
      (base_monsno == MONSNO_IRUMIIZE) ) {
    if( GFUser_GetPublicRand0(2) == 0 ) {
      egg->monsno = MONSNO_BARUBIITO;
    }
    else {
      egg->monsno = MONSNO_IRUMIIZE;
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する ( 例外ケース：フィオネ )
 *
 * マナフィの子はフィオネ
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_MonsNo_fione( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_monsno( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( base_monsno == MONSNO_MANAFI ) {
    egg->monsno = MONSNO_FIONE;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのモンスターナンバーを決定する ( 例外ケース：べビィポケモン )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_MonsNo_baby( const PARENT* parent, EGG_PARAM* egg )
{
  int i;
  const POKEMON_PARAM* base_poke;
  u32 base_monsno, base_itemno;
  u32 father_item, mother_item;

  const int table_size = 12;
  const u32 exception_table[ table_size ][3] = // 例外テーブル
  {
    // べビィポケモン  // 必須アイテム      // 子ポケモン
    {MONSNO_RURIRI,    ITEM_USIONOOKOU,     MONSNO_MARIRU},
    {MONSNO_SOONANO,   ITEM_NONKINOOKOU,    MONSNO_SOONANSU},
    {MONSNO_MANENE,    ITEM_AYASIIOKOU,     MONSNO_BARIYAADO},
    {MONSNO_USOHATI,   ITEM_GANSEKIOKOU,    MONSNO_USOKKII},
    {MONSNO_GONBE,     ITEM_MANPUKUOKOU,    MONSNO_KABIGON},
    {MONSNO_TAMANTA,   ITEM_SAZANAMINOOKOU, MONSNO_MANTAIN},
    {MONSNO_SUBOMII,   ITEM_OHANANOOKOU,    MONSNO_ROZERIA},
    {MONSNO_PINPUKU,   ITEM_KOUUNNOOKOU,    MONSNO_RAKKII},
    {MONSNO_RIISYAN,   ITEM_KIYOMENOOKOU,   MONSNO_TIRIIN},
  };

  base_poke   = GetBasePokemon_for_monsno( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );
  base_itemno = PP_Get( base_poke, ID_PARA_item, NULL );

  father_item = PP_Get( parent->father, ID_PARA_item, NULL );
  mother_item = PP_Get( parent->mother, ID_PARA_item, NULL );

  // 例外テーブルとの照合
  for( i=0; i<table_size; i++ )
  {
    if( (egg->monsno == exception_table[i][0]) &&
        (father_item != exception_table[i][1]) &&
        (mother_item != exception_table[i][1]) ) {
      egg->monsno = exception_table[i][2];
      break;
    }
  }
}

/**
 * @brief 子ポケモンのモンスターNo.決定のベースとなる親ポケモンを取得する
 */
static const POKEMON_PARAM* GetBasePokemon_for_monsno( const PARENT* parent )
{
  u32 mother_monsno = PP_Get( parent->mother, ID_PARA_monsno, NULL );

  // 母親がメタモン ==> 父親
  if( mother_monsno == MONSNO_METAMON ) {
    return parent->father;
  }
  // 通常 ==> 母親
  else {
    return parent->mother;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのフォルムを決定する ( 基本ルール )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Form_basic( const PARENT* parent, EGG_PARAM* egg )
{
  egg->formno = 0;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのフォルムを決定する ( 例外ケース：カラナクシ )
 *
 * 母親が『カラナクシ』or『トリトドン』なら, フォルムを継承
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Form_karanakusi( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_form( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( (base_monsno == MONSNO_KARANAKUSI) ||
      (base_monsno == MONSNO_TORITODON) ) { 
    egg->formno = PP_Get( base_poke, ID_PARA_form_no, NULL );
  }
}


//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのフォルムを決定する ( 例外ケース：ミノムッチ )
 *
 * 母親が『ミノムッチ』or『ミノマダム』なら, フォルムを継承
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Form_minomutti( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_form( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( (base_monsno == MONSNO_MINOMUTTI) ||
      (base_monsno == MONSNO_MINOMADAMU) ) { 
    egg->formno = PP_Get( base_poke, ID_PARA_form_no, NULL );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのフォルムを決定する ( 例外ケース：バスラオ )
 *
 * 母親が『バスラオ』なら, フォルムを継承
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Form_basurao( const PARENT* parent, EGG_PARAM* egg )
{
  const POKEMON_PARAM* base_poke;
  u32 base_monsno;

  base_poke   = GetBasePokemon_for_form( parent );
  base_monsno = PP_Get( base_poke, ID_PARA_monsno, NULL );

  if( base_monsno == MONSNO_BASURAO ) {
    egg->formno = PP_Get( base_poke, ID_PARA_form_no, NULL );
  }
}

/**
 * @brief 子ポケモンのフォルム決定のベースとなる親ポケモンを取得する
 */
static const POKEMON_PARAM* GetBasePokemon_for_form( const PARENT* parent )
{
  u32 mother_monsno = PP_Get( parent->mother, ID_PARA_monsno, NULL );

  // 母親がメタモン ==> 父親
  if( mother_monsno == MONSNO_METAMON ) {
    return parent->father;
  }
  // 通常 ==> 母親
  else {
    return parent->mother;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの性格を決定する ( 基本ルール )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Seikaku_basic( const PARENT* parent, EGG_PARAM* egg )
{
  egg->seikaku = GFUser_GetPublicRand0( PTL_SEIKAKU_MAX );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの性格を決定する ( 例外ケース：かわらずのいし )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Seikaku_kawarazunoisi( const PARENT* parent, EGG_PARAM* egg )
{
  BOOL father_have_kawarazunoisi;
  BOOL mother_have_kawarazunoisi;
  u32 father_seikaku;
  u32 mother_seikaku;

  // 両親のデータを取得
  father_have_kawarazunoisi = ( PP_Get( parent->father, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );
  mother_have_kawarazunoisi = ( PP_Get( parent->mother, ID_PARA_item, NULL ) == ITEM_KAWARAZUNOISI );
  father_seikaku = PP_Get( parent->father, ID_PARA_seikaku, NULL );
  mother_seikaku = PP_Get( parent->mother, ID_PARA_seikaku, NULL );

  // どちらかが持っていても, 1/2で継承しない
  if( father_have_kawarazunoisi || mother_have_kawarazunoisi ) {
    if( GFUser_GetPublicRand0(2) == 0 ) {
      return;
    }
  }

  // 継承
  if( father_have_kawarazunoisi && mother_have_kawarazunoisi ) { // 両親とも所持
    if( GFUser_GetPublicRand0(2) == 0 )  {
      egg->seikaku = father_seikaku;
      egg->derive_seikaku_from_father = TRUE;
    }
    else {
      egg->seikaku = mother_seikaku;
      egg->derive_seikaku_from_mother = TRUE;
    }
  }
  else if( father_have_kawarazunoisi ) { // 父親のみ所持
    egg->seikaku = father_seikaku;
    egg->derive_seikaku_from_father = TRUE;
  }
  else if( mother_have_kawarazunoisi ) { // 母親のみ持所
    egg->seikaku = mother_seikaku;
    egg->derive_seikaku_from_mother = TRUE;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴが何番目の特性かを決定する ( 基本ルール )
 *
 * 母親の特性番号を継承しようとする
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_TokuseiIndex_basic( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id )
{
  u32 rnd;
  u32 mother_tokusei_idx;

  // 母ポケモンの特性インデックスを取得
  {
    u32 mother_monsno  = PP_Get( parent->mother, ID_PARA_monsno, NULL );
    u32 mother_formno  = PP_Get( parent->mother, ID_PARA_form_no, NULL );
    u32 mother_tokusei = PP_Get( parent->mother, ID_PARA_speabino, NULL );
    mother_tokusei_idx = GetTokuseiIndex( mother_monsno, mother_formno, mother_tokusei, heap_id );
  }

  rnd = GFUser_GetPublicRand0(100);  // 乱数取得[0, 99]

  // 特性番号を決定
  switch( mother_tokusei_idx ) {
  default:
  // 母が特性1
  case 0:
    if( rnd < 80 ) { egg->tokusei_idx = 0; } // 80% ==> 子の特性1
    else           { egg->tokusei_idx = 1; } // 20% ==> 子の特性2
    break;

  // 母が特性2
  case 1: 
    if( rnd < 20 ) { egg->tokusei_idx = 0; } // 20% ==> 子の特性1
    else           { egg->tokusei_idx = 1; } // 80% ==> 子の特性2
    break;

  // 母が特性3
  case 2: 
    if( rnd < 20 )      { egg->tokusei_idx = 0; } // 20% ==> 子の特性1
    else if( rnd < 40 ) { egg->tokusei_idx = 1; } // 20% ==> 子の特性2
    else                { egg->tokusei_idx = 2; } // 60% ==> 子の特性3
    break;
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴが何番目の特性かを決定する ( 例外ルール：メタモン )
 *
 * 両親のどちらかがメタモンの場合, タマゴの特性は
 * 50% ==> 特性1
 * 50% ==> 特性2 
 * になる
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_TokuseiIndex_metamon( const PARENT* parent, EGG_PARAM* egg )
{
  u32 father_monsno = PP_Get( parent->father, ID_PARA_monsno, NULL );
  u32 mother_monsno = PP_Get( parent->mother, ID_PARA_monsno, NULL );

  if( (father_monsno == MONSNO_METAMON) || (mother_monsno == MONSNO_METAMON) ) {
    u32 rnd = GFUser_GetPublicRand0(2); // 乱数取得
    if( rnd == 0 ) {
      egg->tokusei_idx = 0;
    }
    else {
      egg->tokusei_idx = 1;
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの特性を決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Tokusei( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id )
{
  u32 tokusei_list[3];

  // 特性リスト ( 種の特性 ) を取得
  {
    POKEMON_PERSONAL_DATA* personalData;
    personalData   = POKE_PERSONAL_OpenHandle( egg->monsno, egg->formno, heap_id );
    tokusei_list[0] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi1 );
    tokusei_list[1] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi2 );
    tokusei_list[2] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi3 );
    POKE_PERSONAL_CloseHandle( personalData );
  }

  if( egg->tokusei_idx < 2 ) { // 第一特性 or 第二特性

    egg->tokusei = tokusei_list[ egg->tokusei_idx ]; 

    // 決定した番号に特性がない場合, 特性0にする
    if( egg->tokusei == TOKUSYU_NULL ) {
      egg->tokusei = tokusei_list[0];
    } 
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのなつき度 ( 孵化カウント ) を決定する
 *
 * タマゴの間は, なつき度を孵化カウンタとして利用する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Friend( EGG_PARAM* egg, HEAPID heap_id )
{
  u32 birth_count;
  POKEMON_PERSONAL_DATA* personal;

  // ポケモンごとの孵化歩数を取得
  personal = POKE_PERSONAL_OpenHandle( egg->monsno, egg->formno, heap_id );
  birth_count = POKE_PERSONAL_GetParam( personal, POKEPER_ID_egg_birth );
  POKE_PERSONAL_CloseHandle( personal );

  egg->friend = birth_count;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個体乱数を決定する ( アイテム所持 )
 *
 * パワーxxxx系のアイテムを所持している場合, 
 * アイテムに対応する個体乱数を両親から計１つ継承する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_AbilityRnd_select_by_item( const PARENT* parent, EGG_PARAM* egg )
{
  int i;
  BOOL father_have_spitem = FALSE;
  BOOL mother_have_spitem = FALSE;
  int derive_ability_idx_from_father;
  int derive_ability_idx_from_mother;
  u32 father_item = PP_Get( parent->father, ID_PARA_item, NULL );
  u32 mother_item = PP_Get( parent->mother, ID_PARA_item, NULL );
  u32 sp_items[ PTL_ABILITY_MAX ] = 
  {
    ITEM_PAWAAUEITO,
    ITEM_PAWAARISUTO,
    ITEM_PAWAABERUTO,
    ITEM_PAWAARENZU,
    ITEM_PAWAABANDO,
    ITEM_PAWAAANKURU,
  };

  // 父親の所持アイテムをチェック
  for( i=0; i<PTL_ABILITY_MAX; i++ )
  {
    if( father_item == sp_items[i] ) {
      father_have_spitem = TRUE;
      derive_ability_idx_from_father = i;
      break;
    }
  }

  // 母親の所持アイテムをチェック
  for( i=0; i<PTL_ABILITY_MAX; i++ )
  {
    if( mother_item == sp_items[i] ) {
      mother_have_spitem = TRUE;
      derive_ability_idx_from_mother = i;
      break;
    }
  }

  if( father_have_spitem && mother_have_spitem ) { // 両親とも所持
    if( GFUser_GetPublicRand0(2) == 0 ) { 
      egg->derive_ability_from_father[ derive_ability_idx_from_father ] = TRUE;
      egg->derive_ability_num++;
    }
    else {
      egg->derive_ability_from_mother[ derive_ability_idx_from_mother ] = TRUE;
      egg->derive_ability_num++;
    }
  }
  else if( father_have_spitem ) { // 父親のみが所持
    egg->derive_ability_from_father[ derive_ability_idx_from_father ] = TRUE;
    egg->derive_ability_num++;
  }
  else if( mother_have_spitem ) { // 母親のみが所持
    egg->derive_ability_from_mother[ derive_ability_idx_from_mother ] = TRUE;
    egg->derive_ability_num++;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個体乱数を決定する ( 基本ルール )
 *
 * 両親から計３つの個体乱数をランダムに継承する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_AbilityRnd_select_at_random( const PARENT* parent, EGG_PARAM* egg )
{
  while( egg->derive_ability_num < 3 )
  {
    int derive_ability_idx = GFUser_GetPublicRand0( PTL_ABILITY_MAX );

    if( GFUser_GetPublicRand0(2) == 0 ) {
      if( (egg->derive_ability_from_father[ derive_ability_idx ] == FALSE) &&
          (egg->derive_ability_from_mother[ derive_ability_idx ] == FALSE) ) {
        egg->derive_ability_from_father[ derive_ability_idx ] = TRUE;
        egg->derive_ability_num++;
      }
    }
    else {
      if( (egg->derive_ability_from_father[ derive_ability_idx ] == FALSE) &&
          (egg->derive_ability_from_mother[ derive_ability_idx ] == FALSE) ) {
        egg->derive_ability_from_mother[ derive_ability_idx ] = TRUE;
        egg->derive_ability_num++;
      }
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個体乱数を決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_AbilityRnd_deirve_from_parent( const PARENT* parent, EGG_PARAM* egg )
{
  int i;
  u32 param_id[ PTL_ABILITY_MAX ] =
  {
    ID_PARA_hp_rnd,
    ID_PARA_pow_rnd,
    ID_PARA_def_rnd,
    ID_PARA_spepow_rnd,
    ID_PARA_spedef_rnd,
    ID_PARA_agi_rnd,
  };

  for( i=0; i<PTL_ABILITY_MAX; i++ )
  {
    if( egg->derive_ability_from_father[i] ) { 
      egg->ability_rnd[i] = PP_Get( parent->father, param_id[i], NULL );
    }
    if( egg->derive_ability_from_mother[i] ) {
      egg->ability_rnd[i] = PP_Get( parent->mother, param_id[i], NULL );
    }
  } 
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴのIDを決定する
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_ID( const MYSTATUS* mystatus, EGG_PARAM* egg )
{
  egg->id_no = MyStatus_GetID( mystatus );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個性乱数を決定する ( 基本ルール )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_PersonalRnd_at_random( const PARENT* parent, EGG_PARAM* egg )
{
  egg->personal_rnd = GetPersonalRnd_at_random();
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴの個性乱数を決定する ( レア抽選 )
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_PersonalRnd_for_rare( const PARENT* parent, EGG_PARAM* egg )
{
  int i;
  u32 father_country = PP_Get( parent->father, ID_PARA_country_code, NULL );
  u32 mother_country = PP_Get( parent->mother, ID_PARA_country_code, NULL );

  // 親同士の国コードが同じ
  if( father_country == mother_country ) { 
    return; 
  } 
  // すでにレア
  if( POKETOOL_CheckRare( egg->id_no, egg->personal_rnd ) ) { 
    return; 
  }

  // 抽選
  for( i=0; i<RARE_EGG_CHANCE; i++ )
  {
    egg->personal_rnd = GetPersonalRnd_at_random();

    // レアになった
    if( POKETOOL_CheckRare( egg->id_no, egg->personal_rnd ) ) { 
      return; 
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief ランダムな個体乱数を取得する
 */
//---------------------------------------------------------------------------------------- 
static u32 GetPersonalRnd_at_random( void )
{
  return GFUser_GetPublicRand0( 0xffffffff );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴにデフォルト技を覚えさせる
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Waza_default( EGG_PARAM* egg )
{
  int i;
  POKEPER_WAZAOBOE_CODE wazaTable[ POKEPER_WAZAOBOE_TABLE_ELEMS ];

  // Lv.1 で覚える技をセット
  POKE_PERSONAL_LoadWazaOboeTable( egg->monsno, egg->formno, wazaTable );
  i = 0;
  while( POKEPER_WAZAOBOE_IsEndCode( wazaTable[i] ) == FALSE )
  {
    int waza_lv = POKEPER_WAZAOBOE_GetLevel( wazaTable[i] );
    int waza_id = POKEPER_WAZAOBOE_GetWazaID( wazaTable[i] );
    if( egg->level < waza_lv ) { 
      break; 
    }
    PushWaza_to_EggParam( egg, waza_id );
    i++;
  }
}

//----------------------------------------------------------------------------------------
/**
 * @brief タマゴ技を覚えさせる
 *
 * タマゴから孵るポケモンのみが覚えられる技のうち, 父ポケモンが覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Waza_egg( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id )
{
  int i, j;
  u16 kowaza_num;
  u16* kowaza_table;
  u32 id_para[ PTL_WAZA_MAX ] = 
  { 
    ID_PARA_waza1, 
    ID_PARA_waza2, 
    ID_PARA_waza3, 
    ID_PARA_waza4 
  };

  // 子技テーブルを取得
  kowaza_table = GFL_ARC_LoadDataAlloc( ARCID_KOWAZA, egg->monsno, heap_id );
  kowaza_num   = kowaza_table[0];

  // 父親の全技をチェックする
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    u32 wazano = PP_Get( parent->father, id_para[i], NULL );

    // 父親の技がタマゴ技なら, 習得する
    for( j=1; j<=kowaza_num; j++ )
    {
      if( wazano == kowaza_table[j] ) {
        PushWaza_to_EggParam( egg, wazano );
        break;
      }
    }
  }

  // 子技テーブルを破棄
  GFL_HEAP_FreeMemory( kowaza_table );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 両親の技を受け継ぐ
 *
 * 自分もレベルを上げることで覚える技のうち, 両親ともに覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Waza_parent( const PARENT* parent, EGG_PARAM* egg )
{
  int i, j;
  POKEPER_WAZAOBOE_CODE wazaTable[ POKEPER_WAZAOBOE_TABLE_ELEMS ]; // 子の技覚えテーブル
  u32 id_para[ PTL_WAZA_MAX ] = 
  { 
    ID_PARA_waza1, 
    ID_PARA_waza2, 
    ID_PARA_waza3, 
    ID_PARA_waza4 
  };

  // 子の技覚えテーブルを取得
  POKE_PERSONAL_LoadWazaOboeTable( egg->monsno, egg->formno, wazaTable );

  // 子の全レベルアップ習得技をチェック
  i = 0;
  while( !POKEPER_WAZAOBOE_IsEndCode( wazaTable[i] ) )
  {
    u32  wazano      = POKEPER_WAZAOBOE_GetWazaID( wazaTable[i] );
    BOOL father_have = FALSE; // 父親が覚えている技かどうか
    BOOL mother_have = FALSE; // 母親が覚えている技かどうか

    // 父が覚えているかどうかチェック
    for( j=0; j<PTL_WAZA_MAX; j++ )
    {
      u32 wazano_father = PP_Get( parent->father, id_para[j], NULL );

      // 覚えている
      if( wazano_father == wazano ) {
        father_have = TRUE;
        break;
      }
    }

    // 母が覚えているかどうかチェック
    for( j=0; j<PTL_WAZA_MAX; j++ )
    {
      u32 wazano_mother = PP_Get( parent->mother, id_para[j], NULL );

      // 覚えている
      if( wazano_mother == wazano ) {
        mother_have = TRUE;
        break;
      }
    }

    // 両親とも覚えていたら継承する
    if( father_have && mother_have ) {
      PushWaza_to_EggParam( egg, wazano );
    }

    // 次の技へ
    i++;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 父ポケモンのマシン技を受け継ぐ
 *
 * 自分も覚えることの出来るマシン技のうち, 父ポケモンが覚えている技を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Waza_machine( const PARENT* parent, EGG_PARAM* egg, HEAPID heap_id )
{
  int i, itemno;
  u32 wazano;
  u32 id_para[ PTL_WAZA_MAX ] = 
  { 
    ID_PARA_waza1, 
    ID_PARA_waza2, 
    ID_PARA_waza3, 
    ID_PARA_waza4 
  };

  // 父の全技をチェック
  for( i=0; i<PTL_WAZA_MAX; i++ )
  {
    // 技コード取得
    wazano = PP_Get( parent->father, id_para[i], NULL );

    // 同技の技マシンが存在するかどうかチェック
    for( itemno=ITEM_WAZAMASIN01; itemno<=ITEM_DATA_MAX; itemno++ )
    {
      // 技マシンがあり, タマゴポケに使用できるなら, 習得する
      if( ( wazano == ITEM_GetWazaNo(itemno) ) &&
          ( IsWazaMachineAbleToUse(heap_id, egg->monsno, egg->formno, itemno) ) ) {
        PushWaza_to_EggParam( egg, wazano );
        break;
      }
    }
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 特殊タマゴ作成処理 ( ピチュー )
 *
 * 親ポケがアイテム『でんきだま』を持っていたら, 技『ボルテッカー』を習得
 */
//---------------------------------------------------------------------------------------- 
static void EggCordinate_Waza_pityuu( const PARENT* parent, EGG_PARAM* egg )
{
  if( egg->monsno != MONSNO_PITYUU ) { return; }

  // 両親のどちらかが『でんきだま』を持っていたら,『ボルテッカー』を習得
  if( ( PP_Get( parent->father, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) ||
      ( PP_Get( parent->mother, ID_PARA_item, NULL ) == ITEM_DENKIDAMA ) ) {
    PushWaza_to_EggParam( egg, WAZANO_BORUTEKKAA );
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 技を押し出し式に追加する
 */
//---------------------------------------------------------------------------------------- 
static void PushWaza_to_EggParam( EGG_PARAM* egg, u32 wazano )
{
  int i; 

  // 技番号が不正
  if( wazano == WAZANO_NULL ) { return; }

  // すでに覚えている
  for( i=0; i<egg->waza_num; i++ )
  {
    if( egg->wazano[i] == wazano ) { return; }
  }

  // push
  if( egg->waza_num < PTL_WAZA_MAX ) {
    egg->wazano[ egg->waza_num ] = wazano;
    egg->waza_num++;
  }
  else {
    for( i=0; i < PTL_WAZA_MAX - 1; i++ )
    {
      egg->wazano[i] = egg->wazano[i+1];
    }
    egg->wazano[i] = wazano;
  }
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを作成する
 *
 * @param egg      設定するタマゴ
 * @param mystatus タマゴの所有者
 * @param memo     トレーナーメモ指定
 * @param heap_id  ヒープID
 */
//---------------------------------------------------------------------------------------- 
static void MakeEgg(
    POKEMON_PARAM* egg, const EGG_PARAM* egg_param, 
    const MYSTATUS* owner, int memo, HEAPID heap_id )
{
  int i;
  u32 ability_rnd_param_id[ PTL_ABILITY_MAX ] =
  {
    ID_PARA_hp_rnd,
    ID_PARA_pow_rnd,
    ID_PARA_def_rnd,
    ID_PARA_spepow_rnd,
    ID_PARA_spedef_rnd,
    ID_PARA_agi_rnd,
  };

  PP_SetupEx( egg, egg_param->monsno, egg_param->level, 
      egg_param->id_no, PTL_SETUP_POW_AUTO, egg_param->personal_rnd );

  PP_Put( egg, ID_PARA_form_no, egg_param->formno ); 
  PP_Put( egg, ID_PARA_seikaku, egg_param->seikaku );
  PP_Put( egg, ID_PARA_friend, egg_param->friend );

  // 特性
  if( egg_param->tokusei_idx == 2 ) { // 第三特性
    PP_SetTokusei3( egg, egg_param->monsno, egg_param->formno );
  }
  else {
#ifdef  BUGFIX_BTS7955_20100728
    PP_Put( egg, ID_PARA_end, egg_param->tokusei );
#else
    PP_Put( egg, ID_PARA_speabino, egg_param->tokusei );
#endif
  }

  // 個体乱数
  for( i=0; i<PTL_ABILITY_MAX; i++ )
  {
    if( egg_param->derive_ability_from_father[i] || 
        egg_param->derive_ability_from_mother[i] ) {
      PP_Put( egg, ability_rnd_param_id[i], egg_param->ability_rnd[i] ); 
    }
  }

  // 技
  PP_Put( egg, ID_PARA_waza1, egg_param->wazano[0] );
  PP_Put( egg, ID_PARA_waza2, egg_param->wazano[1] );
  PP_Put( egg, ID_PARA_waza3, egg_param->wazano[2] );
  PP_Put( egg, ID_PARA_waza4, egg_param->wazano[3] );

  // 親の名前
  {
    STRBUF* name = MyStatus_CreateNameString( owner, heap_id );
    PP_Put( egg, ID_PARA_oyaname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // ニックネーム ( タマゴ )
  {
    STRBUF* name = GFL_MSG_CreateString( GlobalMsg_PokeName, MONSNO_TAMAGO );
    PP_Put( egg, ID_PARA_nickname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // トレーナーメモ
  POKE_MEMO_SetTrainerMemoPP( egg, POKE_MEMO_EGG_FIRST, owner, memo, heap_id );

  // 仕上げ
  PP_Put( egg, ID_PARA_tamago_flag, TRUE ); 
  PP_RecoverWazaPPAll( egg );
  PP_Renew( egg );
  STATUS_RCV_PokeParam_RecoverAll( egg );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 技マシンが使用できるかどうかを判定する
 *
 * @param heap_id 使用するヒープID
 * @param monsno  技マシン使用対象ポケモンのモンスターナンバー
 * @param formno  モンスターのフォームナンバー
 * @param itemno  使用する技マシンのアイテムナンバー
 * @return 技マシンが使用できる場合 TRUE
 */
//---------------------------------------------------------------------------------------- 
static BOOL IsWazaMachineAbleToUse( HEAPID heap_id, u16 monsno, u16 formno, u16 itemno )
{
  u16 seed_monsno;
  u32 bit_pos;
  u32 flag;
  u8 machine_no;
  POKEMON_PERSONAL_DATA* ppd;

  // モンスターナンバーをチェック
  if( MONSNO_END <= monsno ) {
    OBATA_Printf( "IsWazaMachineAbleToUse: 無効なモンスターナンバーが指定されました。\n" );
    GF_ASSERT(0);
    return FALSE;
  }

  // アイテムが技マシンかどうかをチェック
	if( ITEM_CheckWazaMachine( itemno ) == FALSE ) {
    OBATA_Printf( "IsWazaMachineAbleToUse: 技マシン以外のアイテムが指定されました。\n" );
    return FALSE;
  } 

  // パーソナルデータをオープン
  ppd = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );

  // 技マシン番号(ゼロオリジン)を取得
  machine_no = ITEM_GetWazaMashineNo( itemno );

  // 該当する技マシンフラグ・検査ビット位置を取得
  if( machine_no < 32*1 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine1 );
    bit_pos = machine_no;
  }
  else if( machine_no < 32*2 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine2 );
    bit_pos = machine_no - 32;
  }
  else if( machine_no < 32*3 ) {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine3 );
    bit_pos = machine_no - 32*2;
  }
  else {
    flag = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_machine4 );
    bit_pos = machine_no - 32*3;
  }

  // パーソナルデータをクローズ
  POKE_PERSONAL_CloseHandle( ppd );

  // フラグの特定ビットを検査
  return ( flag & (1 << bit_pos) );
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief 指定したモンスターの種ポケモンを取得する
 *
 * @param monsno 種ポケモンを調べるモンスター
 * @return 指定モンスターの種ポケモンのモンスターナンバー
 */
//---------------------------------------------------------------------------------------- 
static u16 GetSeedMonsNo( u16 monsno )
{
  u16 seed_monsno;

  // モンスターナンバーを検査
  if( MONSNO_END <= monsno ) {
    OBATA_Printf( "GetSeedMonsNo: 無効なモンスターナンバーが指定されました。\n" );
    GF_ASSERT(0);
    return monsno; // 不正値なら, そのまま返す
  } 

  // 種ポケモンNoを取得
  GFL_ARC_LoadDataOfs( &seed_monsno, ARCID_PMSTBL, monsno, 0, sizeof(u16) );
  return seed_monsno;
} 

//---------------------------------------------------------------------------------------- 
/**
 * @brief 何番目の特性なのかを取得する
 */
//---------------------------------------------------------------------------------------- 
static u8 GetTokuseiIndex( u16 monsno, u16 formno, u16 tokusei, HEAPID heap_id )
{
  u32 tokusei_idx;
  u32 tokusei_list[3];

  // 特性リスト ( 種の特性 ) を取得
  {
    POKEMON_PERSONAL_DATA* personalData;
    personalData = POKE_PERSONAL_OpenHandle( monsno, formno, heap_id );
    tokusei_list[0] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi1 );
    tokusei_list[1] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi2 );
    tokusei_list[2] = POKE_PERSONAL_GetParam( personalData, POKEPER_ID_speabi3 );
    POKE_PERSONAL_CloseHandle( personalData );
  }
  
  // 特性番号を調べる
  for( tokusei_idx=0; tokusei_idx<3; tokusei_idx++ )
  {
    if( tokusei == tokusei_list[ tokusei_idx ] ) { 
      break; 
    }
  }

  // 不正な特性だった場合
  if( 3 <= tokusei_idx ) { 
    GF_ASSERT(0);
    tokusei_idx = 0; // 不正値なら特性1とする
  }

  return tokusei_idx;
}


#ifdef PM_DEBUG
//========================================================================================
// ■debug functions
//========================================================================================

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( 産まれたて )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreatePlainEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( もうすぐ孵化 )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateQuickEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );
  PP_Put( egg, ID_PARA_friend, 0 );  // 孵化歩数

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する ( だめタマゴ )
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateIllegalEgg( GAMEDATA* gameData, HEAPID heap_id )
{
  POKEMON_PARAM* egg;

  egg = CreateDebugEgg( gameData, heap_id, MONSNO_HUSIGIDANE );
  PP_Put( egg, ID_PARA_fusei_tamago_flag, TRUE );  // ダメたまごフラグ

  return egg;
}

//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する
 *
 * @param gameData
 * @param heapID
 * @param monsno   モンスターNo.
 */
//---------------------------------------------------------------------------------------- 
static POKEMON_PARAM* CreateDebugEgg( GAMEDATA* gameData, HEAPID heapID, u32 monsno )
{
  POKEMON_PARAM* egg;

  egg = PP_Create( monsno, 1, 1, heapID );

  // 親の名前
  {
    MYSTATUS* myStatus;
    STRBUF* name;

    myStatus = GAMEDATA_GetMyStatus( gameData );
    name     = MyStatus_CreateNameString( myStatus, heapID );
    PP_Put( egg, ID_PARA_oyaname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  // 孵化歩数
  {
    u32 monsno, formno, birth;
    POKEMON_PERSONAL_DATA* personal;

    monsno   = PP_Get( egg, ID_PARA_monsno, NULL );
    formno   = PP_Get( egg, ID_PARA_form_no, NULL );
    personal = POKE_PERSONAL_OpenHandle( monsno, formno, heapID );
    birth    = POKE_PERSONAL_GetParam( personal, POKEPER_ID_egg_birth );
    POKE_PERSONAL_CloseHandle( personal );

    // タマゴの間は, なつき度を孵化カウンタとして利用する
    PP_Put( egg, ID_PARA_friend, birth );
  }

  // タマゴフラグ
  PP_Put( egg, ID_PARA_tamago_flag, TRUE );

  // ニックネーム ( タマゴ )
  {
    STRBUF* name;

    name = GFL_MSG_CreateString( GlobalMsg_PokeName, MONSNO_TAMAGO );
    PP_Put( egg, ID_PARA_nickname, (u32)name );
    GFL_STR_DeleteBuffer( name );
  } 

  PP_Renew( egg );
  return egg;
}


#endif // PM_DEBUG
