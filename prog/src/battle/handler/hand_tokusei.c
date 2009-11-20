//=============================================================================================
/**
 * @file  hand_tokusei.c
 * @brief ポケモンWB バトルシステム イベントファクター[とくせい]追加処理
 * @author  taya
 *
 * @date  2008.11.11  作成
 */
//=============================================================================================

#include "poke_tool\poketype.h"
#include "poke_tool\monsno_def.h"
#include "waza_tool\wazano_def.h"

#include "..\btl_common.h"
#include "..\btl_calc.h"
#include "..\btl_field.h"
#include "..\btl_client.h"
#include "..\btl_event_factor.h"

#include "hand_tokusei.h"


/*--------------------------------------------------------------------------*/
/* Typedefs                                                                 */
/*--------------------------------------------------------------------------*/
typedef BTL_EVENT_FACTOR* (*pTokuseiEventAddFunc)( u16, u16, u8 );


/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Ikaku_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seisinryoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Seisinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukutsuno( u16 pri, u16 tokID, u8 pokeID );
static void handler_FukutsunoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AtuiSibou( u16 pri, u16 tokID, u8 pokeID );
static void handler_AtuiSibou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikaramoti( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tikaramoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Suisui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Suisui( u16 pri, u16 tokID, u8 pokeID );
static void handler_Youryokuso( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Youryokuso( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hayaasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hayaasi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tidoriasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tidoriasi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Harikiri_HitRatio( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Harikiri_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Harikiri( u16 pri, u16 tokID, u8 pokeID );
static void handler_Atodasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Atodasi( u16 pri, u16 tokID, u8 pokeID );
static void handler_SlowStart_Agility( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SlowStart_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SlowStart( u16 pri, u16 tokID, u8 pokeID );
static void handler_Fukugan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukugan( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sunagakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunagakure( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yukigakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukigakure( u16 pri, u16 tokID, u8 pokeID );
static void handler_Iromegane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Iromegane( u16 pri, u16 tokID, u8 pokeID );
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HardRock( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sniper( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sniper( u16 pri, u16 tokID, u8 pokeID );
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kasoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tekiouryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tekiouryoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Mouka( u16 pri, u16 tokID, u8 pokeID );
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Gekiryu( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinryoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MusinoSirase( u16 pri, u16 tokID, u8 pokeID );
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
static void handler_Konjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Konjou( u16 pri, u16 tokID, u8 pokeID );
static void handler_Plus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Plus( u16 pri, u16 tokID, u8 pokeID );
static void handler_Minus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Minus( u16 pri, u16 tokID, u8 pokeID );
static void handler_FlowerGift_Power( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FlowerGift( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tousousin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tousousin( u16 pri, u16 tokID, u8 pokeID );
static void handler_Technician( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Technician( u16 pri, u16 tokID, u8 pokeID );
static void handler_TetunoKobusi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_TetunoKobusi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sutemi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sutemi( u16 pri, u16 tokID, u8 pokeID );
static void handler_FusiginaUroko( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaUroko( u16 pri, u16 tokID, u8 pokeID );
static void handler_SkillLink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SkillLink( u16 pri, u16 tokID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KairikiBasami( u16 pri, u16 tokID, u8 pokeID );
static void handler_KairikiBasami_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_KairikiBasami_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surudoime( u16 pri, u16 tokID, u8 pokeID );
static void handler_Surudoime_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Surudoime_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ClearBody( u16 pri, u16 tokID, u8 pokeID );
static void handler_ClearBody_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_ClearBody_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_RankDownGuard_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, WazaRankEffect rankType );
static void common_RankDownGuard_Fixed( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, u16 strID );
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tanjun( u16 pri, u16 tokID, u8 pokeID );
static void handler_ReafGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_AddSickFailCommon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ReafGuard( u16 pri, u16 tokID, u8 pokeID );
static void handler_Juunan_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Juunan_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Juunan( u16 pri, u16 tokID, u8 pokeID );
static void handler_Fumin_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Fumin_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fumin( u16 pri, u16 tokID, u8 pokeID );
static void handler_MagumaNoYoroi_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagumaNoYoroi_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MagumaNoYoroi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Meneki_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Meneki_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Meneki( u16 pri, u16 tokID, u8 pokeID );
static void handler_MyPace_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MyPace_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MyPace( u16 pri, u16 tokID, u8 pokeID );
static void handler_MizuNoBale_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MizuNoBale_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MizuNoBale( u16 pri, u16 tokID, u8 pokeID );
static BOOL common_DiscardWazaSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick );
static void common_Swap_CureSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick );
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Donkan( u16 pri, u16 tokID, u8 pokeID );
static void handler_Amefurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amefurasi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hideri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hideri( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sunaokosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunaokosi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yukifurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukifurasi( u16 pri, u16 tokID, u8 pokeID );
static void common_weather_change( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_AirLock_Appear( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AirLock( u16 pri, u16 tokID, u8 pokeID );
static void handler_IceBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_IcoBody( u16 pri, u16 tokID, u8 pokeID );
static void handler_AmeukeZara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AmeukeZara( u16 pri, u16 tokID, u8 pokeID );
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_SunPower_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SunPower_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunPower( u16 pri, u16 tokID, u8 pokeID );
static void handler_Rinpun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Rinpun_Shrink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Rinpun( u16 pri, u16 tokID, u8 pokeID );
static void handler_TennoMegumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_TennoMegumi( u16 pri, u16 tokID, u8 pokeID );
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_UruoiBody( u16 pri, u16 tokID, u8 pokeID );
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dappi( u16 pri, u16 tokID, u8 pokeID );
static void handler_PoisonHeal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_PoisonHeal( u16 pri, u16 tokID, u8 pokeID );
static void handler_KabutoArmor( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KabutoArmor( u16 pri, u16 tokID, u8 pokeID );
static void handler_Kyouun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kyouun( u16 pri, u16 tokID, u8 pokeID );
static void handler_IkarinoTubo( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_IkarinoTubo( u16 pri, u16 tokID, u8 pokeID );
static void handler_DokunoToge( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DokunoToge( u16 pri, u16 tokID, u8 pokeID );
static void handler_Seidenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seidenki( u16 pri, u16 tokID, u8 pokeID );
static void handler_HonoNoKarada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HonoNoKarada( u16 pri, u16 tokID, u8 pokeID );
static void handler_MeromeroBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MeromeroBody( u16 pri, u16 tokID, u8 pokeID );
static void handler_Housi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Housi( u16 pri, u16 tokID, u8 pokeID );
static void common_touchAddSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick, BPP_SICK_CONT sickCont, u8 per );
static void handler_Samehada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Samehada( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yuubaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yuubaku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hensyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hensyoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Syncro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Syncro( u16 pri, u16 tokID, u8 pokeID );
static void handler_Isiatama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Isiatama( u16 pri, u16 tokID, u8 pokeID );
static void handler_NormalSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NormalSkin( u16 pri, u16 tokID, u8 pokeID );
static void handler_MultiType( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MultiType( u16 pri, u16 tokID, u8 pokeID );
static void handler_Trace( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Trace( u16 pri, u16 tokID, u8 pokeID );
static void handler_SizenKaifuku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SizenKaifuku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Download( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DownLoad( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yotimu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yotimu( u16 pri, u16 tokID, u8 pokeID );
static void handler_KikenYoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL _check_kikenyoti_poke( const BTL_POKEPARAM* bppUser, const BTL_POKEPARAM* bppEnemy );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KikenYoti( u16 pri, u16 tokID, u8 pokeID );
static void handler_Omitoosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Omitoosi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Ganjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ganjou( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tennen_hitRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tennen_AtkRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tennen_DefRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tennen( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tainetsu_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tainetsu_SickDmg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tainetu( u16 pri, u16 tokID, u8 pokeID );
static void common_DmgToRecover_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType matchType );
static void common_DmgToRecover_Fix( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, u8 denomHP );
static void handler_Kansouhada_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kansouhada_DmgRecover( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kansouhada_DmgRecoverFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kansouhada( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tyosui_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tyosui_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tyosui( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tikuden_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tikuden_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikuden( u16 pri, u16 tokID, u8 pokeID );
static void handler_DenkiEngine_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_DenkiEngine_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DenkiEngine( u16 pri, u16 tokID, u8 pokeID );
static void handler_NoGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NoGuard( u16 pri, u16 tokID, u8 pokeID );
static void handler_Kimottama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kimottama( u16 pri, u16 tokID, u8 pokeID );
static void handler_Bouon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bouon( u16 pri, u16 tokID, u8 pokeID );
static void handler_Fuyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fuyuu( u16 pri, u16 tokID, u8 pokeID );
static void handler_FusiginaMamori( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaMamori( u16 pri, u16 tokID, u8 pokeID );
static void handler_Namake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Namake( u16 pri, u16 tokID, u8 pokeID );
static void handler_Simerike( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Simerike( u16 pri, u16 tokID, u8 pokeID );
static void handler_Moraibi_NoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Moraibi_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Moraibi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Nightmare( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nightmare( u16 pri, u16 tokID, u8 pokeID );
static void handler_Nigeasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nigeasi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Arijigoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Arijigoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Kagefumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kagefumi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Jiryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Jiryoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Katayaburi_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Katayaburi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tenkiya_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tenkiya_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_TenkiFormChange( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tenkiya( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yobimizu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yobimizu( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hiraisin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hiraisin( u16 pri, u16 tokID, u8 pokeID );
static void common_WazaTargetChangeToMe( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
static void handler_Kyuuban( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kyuuban( u16 pri, u16 tokID, u8 pokeID );
static void handler_HedoroEki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HedoroEki( u16 pri, u16 tokID, u8 pokeID );
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bukiyou( u16 pri, u16 tokID, u8 pokeID );
static void handler_Nenchaku_NoEff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Nenchaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nenchaku( u16 pri, u16 tokID, u8 pokeID );





//=============================================================================================
/**
 * ポケモンの「とくせい」ハンドラをシステムに追加
 *
 * @param   pp
 *
 * @retval  BTL_EVENT_FACTOR*
 */
//=============================================================================================
BTL_EVENT_FACTOR*  BTL_HANDLER_TOKUSEI_Add( const BTL_POKEPARAM* pp )
{
  static const struct {
    PokeTokusei           tokusei;
    pTokuseiEventAddFunc  func;
  }funcTbl[] = {
    { POKETOKUSEI_IKAKU,          HAND_TOK_ADD_Ikaku },
    { POKETOKUSEI_KURIABODY,      HAND_TOK_ADD_ClearBody },
    { POKETOKUSEI_SIROIKEMURI,    HAND_TOK_ADD_ClearBody }, // しろいけむり=クリアボディと等価
    { POKETOKUSEI_SEISINRYOKU,    HAND_TOK_ADD_Seisinryoku },
    { POKETOKUSEI_FUKUTUNOKOKORO, HAND_TOK_ADD_Fukutsuno },
    { POKETOKUSEI_ATUISIBOU,      HAND_TOK_ADD_AtuiSibou },
    { POKETOKUSEI_KAIRIKIBASAMI,  HAND_TOK_ADD_KairikiBasami },
    { POKETOKUSEI_TIKARAMOTI,     HAND_TOK_ADD_Tikaramoti },
    { POKETOKUSEI_YOGAPAWAA,      HAND_TOK_ADD_Tikaramoti },  // ヨガパワー = ちからもちと等価
    { POKETOKUSEI_IROMEGANE,      HAND_TOK_ADD_Iromegane },
    { POKETOKUSEI_KASOKU,         HAND_TOK_ADD_Kasoku },
    { POKETOKUSEI_MOUKA,          HAND_TOK_ADD_Mouka },
    { POKETOKUSEI_GEKIRYUU,       HAND_TOK_ADD_Gekiryu },
    { POKETOKUSEI_SINRYOKU,       HAND_TOK_ADD_Sinryoku },
    { POKETOKUSEI_MUSINOSIRASE,   HAND_TOK_ADD_MusinoSirase },
    { POKETOKUSEI_KONJOU,         HAND_TOK_ADD_Konjou },
    { POKETOKUSEI_SUKIRURINKU,    HAND_TOK_ADD_SkillLink },
    { POKETOKUSEI_SURUDOIME,      HAND_TOK_ADD_Surudoime },
    { POKETOKUSEI_TANJUN,         HAND_TOK_ADD_Tanjun },
    { POKETOKUSEI_HAADOROKKU,     HAND_TOK_ADD_HardRock },
    { POKETOKUSEI_FIRUTAA,        HAND_TOK_ADD_HardRock },  // フィルター = ハードロックと等価
    { POKETOKUSEI_FUSIGINAUROKO,  HAND_TOK_ADD_FusiginaUroko },
    { POKETOKUSEI_TOUSOUSIN,      HAND_TOK_ADD_Tousousin },
    { POKETOKUSEI_RIIFUGAADO,     HAND_TOK_ADD_ReafGuard },
    { POKETOKUSEI_AMEFURASI,      HAND_TOK_ADD_Amefurasi },
    { POKETOKUSEI_HIDERI,         HAND_TOK_ADD_Hideri },
    { POKETOKUSEI_SUNAOKOSI,      HAND_TOK_ADD_Sunaokosi },
    { POKETOKUSEI_YUKIFURASI,     HAND_TOK_ADD_Yukifurasi },
    { POKETOKUSEI_EAROKKU,        HAND_TOK_ADD_AirLock },
    { POKETOKUSEI_NOOTENKI,       HAND_TOK_ADD_AirLock }, // ノーてんき = エアロックと等価
    { POKETOKUSEI_TEKUNISYAN,     HAND_TOK_ADD_Technician },
    { POKETOKUSEI_DONKAN,         HAND_TOK_ADD_Donkan },
    { POKETOKUSEI_URUOIBODY,      HAND_TOK_ADD_UruoiBody },
    { POKETOKUSEI_POIZUNHIIRU,    HAND_TOK_ADD_PoisonHeal },
    { POKETOKUSEI_AISUBODY,       HAND_TOK_ADD_IcoBody },
    { POKETOKUSEI_AMEUKEZARA,     HAND_TOK_ADD_AmeukeZara },
    { POKETOKUSEI_RINPUN,         HAND_TOK_ADD_Rinpun },
    { POKETOKUSEI_TEKIOURYOKU,    HAND_TOK_ADD_Tekiouryoku },
    { POKETOKUSEI_TENNOMEGUMI,    HAND_TOK_ADD_TennoMegumi },
    { POKETOKUSEI_SANPAWAA,       HAND_TOK_ADD_SunPower },
    { POKETOKUSEI_SUISUI,         HAND_TOK_ADD_Suisui },
    { POKETOKUSEI_YOURYOKUSO,     HAND_TOK_ADD_Youryokuso },
    { POKETOKUSEI_DAPPI,          HAND_TOK_ADD_Dappi },
    { POKETOKUSEI_TIDORIASI,      HAND_TOK_ADD_Tidoriasi },
    { POKETOKUSEI_HAYAASI,        HAND_TOK_ADD_Hayaasi },
    { POKETOKUSEI_HARIKIRI,       HAND_TOK_ADD_Harikiri },
    { POKETOKUSEI_KABUTOAAMAA,    HAND_TOK_ADD_KabutoArmor },
    { POKETOKUSEI_SHERUAAMAA,     HAND_TOK_ADD_KabutoArmor }, // シェルアーマー=カブトアーマーと等価
    { POKETOKUSEI_KYOUUN,         HAND_TOK_ADD_Kyouun },
    { POKETOKUSEI_IKARINOTUBO,    HAND_TOK_ADD_IkarinoTubo },
    { POKETOKUSEI_SUNAIPAA,       HAND_TOK_ADD_Sniper },
    { POKETOKUSEI_TETUNOKOBUSI,   HAND_TOK_ADD_TetunoKobusi },
    { POKETOKUSEI_FUKUGAN,        HAND_TOK_ADD_Fukugan },
    { POKETOKUSEI_ISIATAMA,       HAND_TOK_ADD_Isiatama },
    { POKETOKUSEI_SUTEMI ,        HAND_TOK_ADD_Sutemi },
    { POKETOKUSEI_SEIDENKI ,      HAND_TOK_ADD_Seidenki },
    { POKETOKUSEI_DOKUNOTOGE,     HAND_TOK_ADD_DokunoToge },
    { POKETOKUSEI_HONOONOKARADA,  HAND_TOK_ADD_HonoNoKarada },
    { POKETOKUSEI_HOUSI,          HAND_TOK_ADD_Housi },
    { POKETOKUSEI_PURASU,         HAND_TOK_ADD_Plus },
    { POKETOKUSEI_MAINASU,        HAND_TOK_ADD_Minus },
    { POKETOKUSEI_MEROMEROBODY,   HAND_TOK_ADD_MeromeroBody },
    { POKETOKUSEI_SUNAGAKURE ,    HAND_TOK_ADD_Sunagakure },
    { POKETOKUSEI_YUKIGAKURE ,    HAND_TOK_ADD_Yukigakure },
    { POKETOKUSEI_TOREESU,        HAND_TOK_ADD_Trace },
    { POKETOKUSEI_NOOMARUSUKIN,   HAND_TOK_ADD_NormalSkin },
    { POKETOKUSEI_SAMEHADA,       HAND_TOK_ADD_Samehada },
    { POKETOKUSEI_SIZENKAIFUKU,   HAND_TOK_ADD_SizenKaifuku },
    { POKETOKUSEI_SINKURO,        HAND_TOK_ADD_Syncro },
    { POKETOKUSEI_DAUNROODO,      HAND_TOK_ADD_DownLoad },
    { POKETOKUSEI_GANJOU,         HAND_TOK_ADD_Ganjou },
    { POKETOKUSEI_TAINETU,        HAND_TOK_ADD_Tainetu },
    { POKETOKUSEI_TENNEN,         HAND_TOK_ADD_Tennen },
    { POKETOKUSEI_KANSOUHADA,     HAND_TOK_ADD_Kansouhada },
    { POKETOKUSEI_TIKUDEN,        HAND_TOK_ADD_Tikuden },
    { POKETOKUSEI_TYOSUI,         HAND_TOK_ADD_Tyosui },
    { POKETOKUSEI_DENKIENJIN,     HAND_TOK_ADD_DenkiEngine },
    { POKETOKUSEI_JUUNAN,         HAND_TOK_ADD_Juunan },
    { POKETOKUSEI_FUMIN,          HAND_TOK_ADD_Fumin },
    { POKETOKUSEI_YARUKI,         HAND_TOK_ADD_Fumin },   // やるき=ふみんと等価
    { POKETOKUSEI_MAIPEESU,       HAND_TOK_ADD_MyPace },
    { POKETOKUSEI_MAGUMANOYOROI,  HAND_TOK_ADD_MagumaNoYoroi },
    { POKETOKUSEI_MIZUNOBEERU,    HAND_TOK_ADD_MizuNoBale },
    { POKETOKUSEI_MENEKI,         HAND_TOK_ADD_Meneki },
    { POKETOKUSEI_NOOGAADO,       HAND_TOK_ADD_NoGuard },
    { POKETOKUSEI_KIMOTTAMA,      HAND_TOK_ADD_Kimottama },
    { POKETOKUSEI_BOUON,          HAND_TOK_ADD_Bouon },
    { POKETOKUSEI_FUYUU,          HAND_TOK_ADD_Fuyuu },
    { POKETOKUSEI_FURAWAAGIFUTO,  HAND_TOK_ADD_FlowerGift },
    { POKETOKUSEI_MORAIBI,        HAND_TOK_ADD_Moraibi },
    { POKETOKUSEI_YOTIMU,         HAND_TOK_ADD_Yotimu },
    { POKETOKUSEI_KIKENYOTI,      HAND_TOK_ADD_KikenYoti },
    { POKETOKUSEI_OMITOOSI,       HAND_TOK_ADD_Omitoosi },
    { POKETOKUSEI_YOBIMIZU,       HAND_TOK_ADD_Yuubaku },
    { POKETOKUSEI_NIGEASI,        HAND_TOK_ADD_Nigeasi },
    { POKETOKUSEI_HENSYOKU,       HAND_TOK_ADD_Hensyoku },
    { POKETOKUSEI_KATAYABURI,     HAND_TOK_ADD_Katayaburi },
    { POKETOKUSEI_NAMAKE,         HAND_TOK_ADD_Namake },
    { POKETOKUSEI_HIRAISIN,       HAND_TOK_ADD_Hiraisin },
    { POKETOKUSEI_YOBIMIZU,       HAND_TOK_ADD_Yobimizu },
    { POKETOKUSEI_SUROOSUTAATO,   HAND_TOK_ADD_SlowStart },
    { POKETOKUSEI_ARIJIGOKU,      HAND_TOK_ADD_Arijigoku },
    { POKETOKUSEI_KAGEFUMI,       HAND_TOK_ADD_Kagefumi },
    { POKETOKUSEI_JIRYOKU,        HAND_TOK_ADD_Jiryoku },
    { POKETOKUSEI_SIMERIKE,       HAND_TOK_ADD_Simerike },
    { POKETOKUSEI_MARUTITAIPU,    HAND_TOK_ADD_MultiType },
    { POKETOKUSEI_FUSIGINAMAMORI, HAND_TOK_ADD_FusiginaMamori },
    { POKETOKUSEI_ATODASI,        HAND_TOK_ADD_Atodasi },
    { POKETOKUSEI_TENKIYA,        HAND_TOK_ADD_Tenkiya },
    { POKETOKUSEI_KYUUBAN,        HAND_TOK_ADD_Kyuuban },
    { POKETOKUSEI_HEDOROEKI,      HAND_TOK_ADD_HedoroEki },
    { POKETOKUSEI_BUKIYOU,        HAND_TOK_ADD_Bukiyou },
    { POKETOKUSEI_NENCHAKU,       HAND_TOK_ADD_Nenchaku },
  };


  if( !BPP_CheckSick(pp, WAZASICK_IEKI) ) // "いえき" 状態のポケモンはとくせい追加しない
  {
    u16 tokusei = BPP_GetValue( pp, BPP_TOKUSEI );
    int i;

    for(i=0; i<NELEMS(funcTbl); i++)
    {
      if( funcTbl[i].tokusei == tokusei )
      {
        u16 agi = BPP_GetValue( pp, BPP_AGILITY );
        u8 pokeID = BPP_GetID( pp );

        BTL_Printf("ポケモン[%d]の とくせい(%d)ハンドラを追加\n", pokeID, tokusei);
        return funcTbl[i].func( agi, tokusei, pokeID );
      }
    }
    BTL_Printf("とくせい(%d)はハンドラが用意されていない\n", tokusei);
  }
  return NULL;
}

//=============================================================================================
/**
 * ポケモンの「とくせい」ハンドラをシステムから削除
 *
 * @param   pp
 *
 */
//=============================================================================================
void BTL_HANDLER_TOKUSEI_Remove( const BTL_POKEPARAM* pp )
{
  BTL_EVENT_FACTOR* factor;
  u8 pokeID = BPP_GetID( pp );

  while( (factor = BTL_EVENT_SeekFactor(BTL_EVENT_FACTOR_TOKUSEI, pokeID)) != NULL )
  {
    BTL_Printf("ポケモン[%d]の とくせいハンドラを除去\n", pokeID);
    BTL_EVENT_FACTOR_Remove( factor );
  }
}


void BTL_HANDLER_TOKUSEI_Pause( const BTL_POKEPARAM* pp )
{

}

void BTL_HANDLER_TOKUSEI_Swap( const BTL_POKEPARAM* pp1, const BTL_POKEPARAM* pp2 )
{
  BTL_EVENT_FACTOR *factor1, *factor2;
  u8 ID_1, ID_2;

  ID_1 = BPP_GetID( pp1 );
  ID_2 = BPP_GetID( pp2 );
  factor1 = BTL_EVENT_SeekFactor( BTL_EVENT_FACTOR_TOKUSEI, ID_1 );
  factor2 = BTL_EVENT_SeekFactor( BTL_EVENT_FACTOR_TOKUSEI, ID_2 );

  if( factor1 && factor2 )
  {
    u16 pri_1 = BPP_GetValue( pp1, BPP_AGILITY );
    u16 pri_2 = BPP_GetValue( pp2, BPP_AGILITY );

    BTL_EVENT_FACTOR_ChangePokeParam( factor1, ID_2, pri_2 );
    BTL_EVENT_FACTOR_ChangePokeParam( factor2, ID_1, pri_1 );
  }
}

//=============================================================================================
/**
 * とくせいハンドラ管理用にターンの初回に必ず呼び出される
 */
//=============================================================================================
/*
void BTL_HANDLER_TOKUSEI_InitTurn( void )
{
  GFL_STD_MemClear( DisableTokTable, sizeof(DisableTokTable) );
}
*/

//------------------------------------------------------------------------------
/**
 *  とくせい「いかく」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ikaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Ikaku_MemberIn },
    { BTL_EVENT_NULL, NULL },
  };

  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ポケモンが出場した時のハンドラ
static void handler_Ikaku_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

      param->rankType = BPP_ATTACK;
      param->rankVolume = -1;
      param->fAlmost = TRUE;

      {
        BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );
        BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_ENEMY_ALL, myPos );

        param->poke_cnt = BTL_SERVERFLOW_RECEPT_GetTargetPokeID( flowWk, expos, param->pokeID );
      }
    }
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「せいしんりょく」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seisinryoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SHRINK_CHECK, handler_Seisinryoku },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// BTL_EVENT_SHRINK_CHECK:ひるみ発生チェック
static void handler_Seisinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分がひるまされる対象
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 強制失敗フラグをＯＮ
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ふくつのこころ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukutsuno( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_FAIL, handler_FukutsunoKokoro },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ワザ出し失敗確定
static void handler_FukutsunoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // ワザ失敗理由=ひるみ, 対象ポケ=自分で発動
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_CAUSE) == SV_WAZAFAIL_SHRINK)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  ){
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
    BTL_HANDEX_PARAM_HEADER* header;

    header = (BTL_HANDEX_PARAM_HEADER*)param;
    header->tokwin_flag = TRUE;

    param->rankType = BPP_AGILITY;
    param->rankVolume = 1;
    param->fAlmost = FALSE;
    param->poke_cnt = 1;
    param->pokeID[0] = pokeID;
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あついしぼう」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AtuiSibou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_AtuiSibou },
    { BTL_EVENT_NULL, NULL },
  };

  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// 攻撃側の攻撃値決定
static void handler_AtuiSibou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // ダメージ対象が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    PokeType  type = WAZADATA_GetType( waza );
    // ワザタイプが氷か炎
    if( (type == POKETYPE_KOORI) || (type == POKETYPE_HONOO) )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちからもち」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikaramoti( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Tikaramoti },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// 攻撃側の攻撃値決定
static void handler_Tikaramoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃するのが自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID)
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    // ダメージタイプが物理
    if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すいすい」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Suisui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 雨のとき、すばやさ２倍
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_RAIN )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Suisui( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Suisui },   // すばやさ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ようりょくそ」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Youryokuso( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // はれの時、すばやさ２倍
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Youryokuso( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Youryokuso },   // すばやさ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「はやあし」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Hayaasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 状態異常の時、すばやさ1.5倍＆マヒによる減衰を無視する
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_AGILITY, BTL_CALC_TOK_HAYAASI_AGIRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, FALSE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hayaasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Hayaasi },    // すばやさ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちどりあし」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Tidoriasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_CheckSick(bpp, WAZASICK_KONRAN) )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_CHIDORI_HITRATIO );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tidoriasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Tidoriasi },    // 命中率計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「はりきり」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Harikiri_HitRatio( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 物理ワザは命中率減少
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_HARIKIRI_HITRATIO );
    }
  }
}
// 攻撃力計算ハンドラ
static void handler_Harikiri_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 物理ワザは威力増加
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
    {
      u32 power = BTL_EVENTVAR_GetValue( BTL_EVAR_POWER );
      power = BTL_CALC_MulRatio( power, BTL_CALC_TOK_HARIKIRI_POWRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_POWER, power );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Harikiri( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Harikiri_HitRatio },  // 命中率計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER, handler_Harikiri_AtkPower },  // 攻撃力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あとだし」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_Atodasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_SP_PRIORITY_B, BTL_SPPRI_B_LOW );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Atodasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_SP_PRIORITY,   handler_Atodasi },  // 特殊優先度チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「スロースタート」
 */
//------------------------------------------------------------------------------
// すばやさ計算ハンドラ
static void handler_SlowStart_Agility( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( (BPP_GetMonsNo(bpp) == MONSNO_REZIGIGASU)
    &&  (BPP_GetTurnCount(bpp) < BTL_CALC_TOK_SLOWSTART_ENABLE_TURN )
    ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SLOWSTART_AGIRATIO );
    }
  }
}
// 攻撃力計算ハンドラ
static void handler_SlowStart_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( (BPP_GetMonsNo(bpp) == MONSNO_REZIGIGASU)
    &&  (BPP_GetTurnCount(bpp) < BTL_CALC_TOK_SLOWSTART_ENABLE_TURN )
    ){
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SLOWSTART_ATKRATIO );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SlowStart( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY,   handler_SlowStart_Agility },  // すばやさ計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER, handler_SlowStart_AtkPower }, // 攻撃力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ふくがん」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Fukugan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FUKUGAN_HITRATIO );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fukugan( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Fukugan },  // 命中率計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すながくれ」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Sunagakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SAND )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SUNAGAKURE_HITRATIO );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunagakure( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Sunagakure }, // 命中率計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ゆきがくれ」
 */
//------------------------------------------------------------------------------
// 命中率計算ハンドラ
static void handler_Yukigakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SNOW )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SUNAGAKURE_HITRATIO );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukigakure( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Yukigakure }, // 命中率計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「いろめがね」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_Iromegane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 相性イマイチの時
    BtlTypeAffAbout aff = BTL_CALC_TypeAffAbout( BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEAFF) );
    if( aff == BTL_TYPEAFF_ABOUT_DISADVANTAGE )
    {
      // ダメージ２倍
      u32 dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
      dmg *= 2;
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, dmg );
      BTL_Printf("ポケ[%d]の いろめがね でダメージ２倍\n", pokeID);
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Iromegane( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_Iromegane },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ハードロック」＆「フィルター」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 防御側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // こうかバツグンの時
    BtlTypeAffAbout aff = BTL_CALC_TypeAffAbout( BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEAFF) );
    if( aff == BTL_TYPEAFF_ABOUT_ADVANTAGE )
    {
      // ダメージ75％
      u32 dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE );
      dmg = BTL_CALC_MulRatio_OverZero( dmg, FX32_CONST(75) );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, dmg );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HardRock( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_HardRock },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「スナイパー」
 */
//------------------------------------------------------------------------------
// ダメージ計算最終段階のハンドラ
static void handler_Sniper( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // クリティカルの時
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_CRITICAL_FLAG) )
    {
      // ダメージ２倍
      u32 dmg = BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE ) * 2;
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, dmg );
      BTL_Printf("ポケ[%d]の スナイパ でダメージ２倍\n", pokeID);
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sniper( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_Sniper },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


//------------------------------------------------------------------------------
/**
 *  とくせい「かそく」
 */
//------------------------------------------------------------------------------
// ターンチェックのハンドラ
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

  param->header.tokwin_flag = TRUE;
  param->rankType = BPP_AGILITY;
  param->poke_cnt = 1;
  param->pokeID[0] = pokeID;
  param->rankVolume = 1;
  param->fAlmost = TRUE;
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kasoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END, handler_Kasoku },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「てきおうりょく」
 */
//------------------------------------------------------------------------------
// タイプ一致補正率計算ハンドラ
static void handler_Tekiouryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) )
    {
      BTL_EVENTVAR_RewriteValue(BTL_EVAR_RATIO, FX32_CONST(2));
      BTL_Printf("ポケ[%d]の てきおうりょく でタイプ一致補正率２倍に\n", pokeID);
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tekiouryoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TYPEMATCH_RATIO, handler_Tekiouryoku }, // タイプ一致補正率計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「もうか」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_hpborder_powerup( flowWk, pokeID, POKETYPE_HONOO );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Mouka( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Mouka },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「げきりゅう」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_hpborder_powerup( flowWk, pokeID, POKETYPE_MIZU );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Gekiryu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Gekiryu },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しんりょく」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_hpborder_powerup( flowWk, pokeID, POKETYPE_KUSA );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinryoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Sinryoku },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「むしのしらせ」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_hpborder_powerup( flowWk, pokeID, POKETYPE_MUSHI );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MusinoSirase( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_MusinoSirase },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//--------------------------------------------------------------------------
/**
 * HPが1/3の時に、特定タイプのワザ威力を上げるとくせいの共通処理
 *
 * @param   flowWk
 * @param   pokeID
 * @param   wazaType
 *
 */
//--------------------------------------------------------------------------
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // HP 1/3 以下で
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetHPBorder(bpp) <= BPP_HPBORDER_YELLOW )
    {
      // 使うのが指定タイプワザなら
      if( WAZADATA_GetType( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZAID) ) == wazaType )
      {
        // 威力２倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「こんじょう」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Konjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 状態異常で
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      // ダメージタイプが物理の時
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
      {
        // 威力２倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Konjou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Konjou },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「プラス」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Plus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 場に「マイナス」持ちがいて
    if( BTL_SVFLOW_RECEPT_CheckExistTokuseiPokemon(flowWk, POKETOKUSEI_MAINASU) )
    {
      // ダメージタイプが特殊の時
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        // とくこう1.5倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_PLUS_POWRATIO );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Plus( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Plus }, // 攻撃力決定のハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マイナス」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_Minus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 場に「プラス」持ちがいて
    if( BTL_SVFLOW_RECEPT_CheckExistTokuseiPokemon(flowWk, POKETOKUSEI_PURASU) )
    {
      // ダメージタイプが特殊の時
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        // とくこう1.5倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_PLUS_POWRATIO );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Minus( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Minus },  // 攻撃力決定のハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「フラワーギフト」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_FlowerGift_Power( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 天気「はれ」で
  if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
  {
    // 攻撃側が自分か味方のとき
    u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) )
    {
      // こうげき1.5倍
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FLOWERGIFT_POWRATIO );
      }
    }
  }
}
// 防御力決定のハンドラ
static void handler_FlowerGift_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 天気「はれ」で
  if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
  {
    // 防御側が自分か味方のとき
    u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
    if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) )
    {
      // とくぼう1.5倍
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        u32 guard = BTL_EVENTVAR_GetValue( BTL_EVAR_GUARD );
        guard = BTL_CALC_MulRatio( guard, BTL_CALC_TOK_FLOWERGIFT_GUARDRATIO );
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_GUARD, guard );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FlowerGift( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_FlowerGift_Power }, // 攻撃力決定のハンドラ
    { BTL_EVENT_DEFENDER_GUARD, handler_FlowerGift_Guard }, // 防御力決定のハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}



//------------------------------------------------------------------------------
/**
 *  とくせい「とうそうしん」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_Tousousin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    const BTL_POKEPARAM* myParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    const BTL_POKEPARAM* targetParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk,
          BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) );

    u8 mySex = BPP_GetValue( myParam, BPP_SEX );
    u8 targetSex = BPP_GetValue( targetParam, BPP_SEX );

    // 互いに性別不明じゃない場合
    if( (mySex!=PTL_SEX_UNKNOWN) && (targetSex!=PTL_SEX_UNKNOWN) )
    {
      u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
      if( mySex == targetSex ){
        pow = (pow * FX32_CONST(1.25f)) >> FX32_SHIFT;
      }else{
        pow = (pow * FX32_CONST(0.75f)) >> FX32_SHIFT;
      }
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tousousin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Tousousin },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「テクニシャン」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_Technician( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 素の威力60以下なワザなら1.5倍にする
    WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetPower(waza) <= 60 )
    {
      u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
      pow = BTL_CALC_MulRatio( pow, BTL_CALC_TOK_TECKNICIAN_POWRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Technician( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Technician },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「てつのこぶし」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_TetunoKobusi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // パンチ系イメージのワザなら威力増加
    WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Punch) )
    {
      u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
      BTL_CALC_MulRatio( pow, BTL_CALC_TOK_TETUNOKOBUSI_POWRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_TetunoKobusi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_TetunoKobusi },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すてみ」
 */
//------------------------------------------------------------------------------
// ワザ威力決定のハンドラ
static void handler_Sutemi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 反動のある攻撃なら威力増加
    WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetParam(waza, WAZAPARAM_DAMAGE_REACTION_RATIO) )
    {
      u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
      BTL_CALC_MulRatio( pow, BTL_CALC_TOK_SUTEMI_POWRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sutemi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Sutemi },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「ふしぎなうろこ」
 */
//------------------------------------------------------------------------------
///< 防御能力決定のハンドラ
static void handler_FusiginaUroko( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 防御側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 状態異常で
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      // ダメージタイプが特殊の時
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        // 防御２倍
        u32 guard = BTL_EVENTVAR_GetValue( BTL_EVAR_GUARD );
        guard = BTL_CALC_MulRatio( guard, BTL_CALC_TOK_FUSIGINAUROKO_GDRATIO );
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_GUARD, guard );
        BTL_Printf("ポケ[%d]の ふしぎなうろこ で防御1.5倍\n", pokeID);
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaUroko( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DEFENDER_GUARD, handler_FusiginaUroko },  ///< 防御能力決定のハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「スキルリンク」
 */
//------------------------------------------------------------------------------
// 攻撃回数決定のハンドラ
static void handler_SkillLink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // ヒット回数を最大に
    u16 max = BTL_EVENTVAR_GetValue( BTL_EVAR_HITCOUNT_MAX );
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_HITCOUNT, max );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SkillLink( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_COUNT, handler_SkillLink },  // 攻撃回数決定のハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「かいりきばさみ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KairikiBasami( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_KairikiBasami_Check },
    { BTL_EVENT_RANKEFF_FAILED,     handler_KairikiBasami_Guard },

    { BTL_EVENT_NULL, NULL },
  };

  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
static void handler_KairikiBasami_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Check( flowWk, pokeID, work, WAZA_RANKEFF_ATTACK );
}
static void handler_KairikiBasami_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Fixed( flowWk, pokeID, work, BTL_STRID_SET_RankdownFail_ATK );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「するどいめ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surudoime( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_Surudoime_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_Surudoime_Guard },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

// ランクダウン直前処理のハンドラ
static void handler_Surudoime_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Check( flowWk, pokeID, work, WAZA_RANKEFF_HIT );
}
static void handler_Surudoime_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Fixed( flowWk, pokeID, work, BTL_STRID_SET_RankdownFail_HIT );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「クリアボディ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ClearBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_ClearBody_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_ClearBody_Guard },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

// ランクダウン直前処理のハンドラ
static void handler_ClearBody_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Check( flowWk, pokeID, work, WAZA_RANKEFF_MAX );
}
static void handler_ClearBody_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Fixed( flowWk, pokeID, work, BTL_STRID_SET_RankdownFail );
}

//----------------------------------------------------------------------------------
/**
 * ランクダウンを防ぐとくせいの共通処理（チェック時）
 *
 * @param   flowWk
 * @param   pokeID
 * @param   work
 * @param   rankType    防ぐランクタイプ（WAZA_RANKEFF_MAXなら全て）
 */
//----------------------------------------------------------------------------------
static void common_RankDownGuard_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, WazaRankEffect rankType )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( (rankType == WAZA_RANKEFF_MAX)
    ||  (BTL_EVENTVAR_GetValue(BTL_EVAR_STATUS_TYPE) == rankType)
    ){
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_VOLUME) < 0 ){
        work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
      }
    }
  }
}
//----------------------------------------------------------------------------------
/**
 * ランクダウンを防ぐとくせいの共通処理（確定時）
 *
 * @param   flowWk
 * @param   pokeID
 * @param   work
 * @param   strID   表示メッセージID
 */
//----------------------------------------------------------------------------------
static void common_RankDownGuard_Fixed( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, u16 strID )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (work[0])
  ){
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );

    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
    HANDEX_STR_AddArg( &param->str, pokeID );

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    work[0] = 0;
  }
}



//------------------------------------------------------------------------------
/**
 *  とくせい「たんじゅん」
 */
//------------------------------------------------------------------------------
// ワザによるランク増減効果チェックハンドラ
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    int volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    volume *= 2;
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, volume );
    BTL_Printf("ポケ[%d]の たんじゅん により効果倍増\n");
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tanjun( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_GET_RANKEFF_VALUE, handler_Tanjun },  // ワザによるランク増減効果チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「リーフガード」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_ReafGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // くらう側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 天候が晴れ
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
    {
      // ポケモン系状態異常にはならない
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) < POKESICK_MAX )
      {
        work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
      }
    }
  }
}
static void handler_AddSickFailCommon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (work[0] == TRUE)
  ){
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );

    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
    HANDEX_STR_AddArg( &param->str, pokeID );

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    work[0] = FALSE;
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ReafGuard( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL, handler_ReafGuard         }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,    handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「じゅうなん」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_Juunan_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, POKESICK_MAHI );
}
// とくせい書き換えハンドラ
static void handler_Juunan_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_MAHI );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Juunan( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,  handler_Juunan_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,     handler_Juunan_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,     handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ふみん」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_Fumin_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, WAZASICK_NEMURI );
  if( work[0] == 0 ){
    work[0] = common_DiscardWazaSick( flowWk, pokeID, WAZASICK_AKUBI );
  }
}
// とくせい書き換えハンドラ
static void handler_Fumin_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_NEMURI );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fumin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,  handler_Fumin_PokeSick }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,     handler_Fumin_Swap },     // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,     handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マグマのよろい」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_MagumaNoYoroi_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, POKESICK_YAKEDO );
}
// とくせい書き換えハンドラ
static void handler_MagumaNoYoroi_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_YAKEDO );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MagumaNoYoroi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_MagumaNoYoroi_PokeSick }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,       handler_MagumaNoYoroi_Swap },     // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「めんえき」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_Meneki_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, POKESICK_DOKU );
}
// とくせい書き換えハンドラ
static void handler_Meneki_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_DOKU );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Meneki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,  handler_Meneki_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,     handler_Meneki_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,     handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マイペース」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_MyPace_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, WAZASICK_KONRAN );
}
// とくせい書き換えハンドラ
static void handler_MyPace_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, WAZASICK_KONRAN );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MyPace( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MAKE_POKESICK,    handler_MyPace_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,   handler_MyPace_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,   handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「みずのベール」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_MizuNoBale_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_DiscardWazaSick( flowWk, pokeID, WAZASICK_YAKEDO );
}
// とくせい書き換えハンドラ
static void handler_MizuNoBale_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, WAZASICK_YAKEDO );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MizuNoBale( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,  handler_MizuNoBale_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI,     handler_MizuNoBale_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,     handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//----------------------------------------------------------------------------------
/**
 * 状態異常を受け付けないとくせいの共通処理
 *
 * @param   flowWk
 * @param   pokeID
 * @param   sick
 *
 * @retval  BOOL      この処理により受け付け拒否した場合TRUE
 */
//----------------------------------------------------------------------------------
static BOOL common_DiscardWazaSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick )
{
  // くらう側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // くらう病気が指定通り
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) == sick )
    {
      return BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
    }
  }
  return FALSE;
}
static void common_Swap_CureSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // @todo 古い書き方だ。
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_CheckSick(bpp, sick) )
    {
      BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
      if( sick < POKESICK_MAX )
      {
        BTL_SVFLOW_RECEPT_CurePokeSick( flowWk, pokeID );
      }
      else
      {
        BTL_SVFLOW_RECEPT_CureWazaSick( flowWk, pokeID, sick );
      }
      BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
    }
  }
}


//------------------------------------------------------------------------------
/**
 *  とくせい「どんかん」
 */
//------------------------------------------------------------------------------
// ワザ系状態異常ハンドラ
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // くらう側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // メロメロ
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) == WAZASICK_MEROMERO )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Donkan( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL, handler_Donkan },  // 状態異常失敗チェックハンドラ
    { BTL_EVENT_ADDSICK_FAILED,    handler_AddSickFailCommon },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「あめふらし」
 */
//------------------------------------------------------------------------------
static void handler_Amefurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_change( flowWk, pokeID, BTL_WEATHER_RAIN );
  BTL_EVENT_FACTOR_Remove( myHandle );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amefurasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Amefurasi }, // 自分が出場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ひでり」
 */
//------------------------------------------------------------------------------
static void handler_Hideri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_change( flowWk, pokeID, BTL_WEATHER_SHINE );
  BTL_EVENT_FACTOR_Remove( myHandle );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hideri( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Hideri },  // 自分が出場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すなおこし」
 */
//------------------------------------------------------------------------------
static void handler_Sunaokosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_change( flowWk, pokeID, BTL_WEATHER_SAND );
  BTL_EVENT_FACTOR_Remove( myHandle );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunaokosi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Sunaokosi }, // 自分が出場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ゆきふらし」
 */
//------------------------------------------------------------------------------
static void handler_Yukifurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_change( flowWk, pokeID, BTL_WEATHER_SNOW );
  BTL_EVENT_FACTOR_Remove( myHandle );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yukifurasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Yukifurasi },  // 自分が出場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//-------------------------------------------
/**
 *  入場時に天候変化させるとくせいの共通処理
 */
//-------------------------------------------
static void common_weather_change( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_CHANGE_WEATHER* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CHANGE_WEATHER, pokeID );

    param->header.tokwin_flag = TRUE;
    param->weather = weather;
    param->turn = BTL_WEATHER_TURN_PERMANENT;
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「エアロック」&「ノーてんき」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
static void handler_AirLock_Appear( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_FIELD_GetWeather() != BTL_WEATHER_NONE )
    {
      BTL_HANDEX_PARAM_CHANGE_WEATHER* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CHANGE_WEATHER, pokeID );

      param->header.tokwin_flag = TRUE;
      param->weather = BTL_WEATHER_NONE;
      param->turn = BTL_WEATHER_TURN_PERMANENT;
    }
  }
}
// 天候変化ハンドラ
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_FLAG, TRUE) )
  {
    BTL_HANDEX_PARAM_MESSAGE* param;
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_WeatherLocked );
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AirLock( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,      handler_AirLock_Appear },         // 自分が出場ハンドラ
    { BTL_EVENT_WEATHER_CHANGE, handler_AirLock_ChangeWeather },  // 天候変化ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「アイスボディ」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_IceBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_recover( flowWk, pokeID, BTL_WEATHER_SNOW );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_IcoBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_WEATHER_DAMAGE, handler_IceBody }, // 天候ダメージ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あめうけざら」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_AmeukeZara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_recover( flowWk, pokeID, BTL_WEATHER_RAIN );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AmeukeZara( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_WEATHER_DAMAGE, handler_AmeukeZara },  // 天候ダメージ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

/**
 *  天候に応じてHP回復するとくせいの共通処理
 */
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == weather )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
      int recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, recoverHP );
    }
  }
}

//------------------------------------------------------------------------------
/**
 *  とくせい「サンパワー」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_SunPower_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 晴れの時、MAXHPの 1/8 減る
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == BTL_WEATHER_SHINE )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
      int damage = BTL_CALC_QuotMaxHP( bpp, 8 );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, damage );
    }
  }
}
// 攻撃力決定ハンドラ
static void handler_SunPower_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 天候が晴れで
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SHINE )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      // ダメージタイプが特殊の時
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        // 威力1.5倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.5f) );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunPower( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_WEATHER_DAMAGE, handler_SunPower_Weather },// 天候ダメージ計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER,  handler_SunPower_AtkPower },   // 攻撃力決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「りんぷん」
 */
//------------------------------------------------------------------------------
// 追加効果（状態異常，ランク効果共通）ハンドラ
static void handler_Rinpun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_ADD_PER, 0 );
  }
}
// ひるみチェックハンドラ
static void handler_Rinpun_Shrink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Rinpun( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADD_SICK,       handler_Rinpun }, // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET,handler_Rinpun }, // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_SHRINK_CHECK,   handler_Rinpun_Shrink },  // ひるみチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「てんのめぐみ」
 */
//------------------------------------------------------------------------------
// 追加効果（状態異常，ランク効果共通）ハンドラ
static void handler_TennoMegumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) )
  {
    u16 per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER ) * 2;
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_ADD_PER, per );
    BTL_Printf("ポケ[%d]の てんのめぐみ で追加発生率２倍=%d%%\n", pokeID, per);
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_TennoMegumi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADD_SICK,         handler_TennoMegumi },  // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET,  handler_TennoMegumi },  // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_WAZA_SHRINK_PER,  handler_TennoMegumi },  // ひるみチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「うるおいボディ」
 */
//------------------------------------------------------------------------------
// 状態異常ダメージハンドラ
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_FIELD_GetWeather() == BTL_WEATHER_RAIN )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      BTL_HANDEX_PARAM_CURE_SICK* param;

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
      param->sickCode = WAZASICK_EX_POKEFULL;
      param->pokeID[0] = pokeID;
      param->poke_cnt = 1;

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_UruoiBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,      handler_UruoiBody },  // ターンチェック開始
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「だっぴ」
 */
//------------------------------------------------------------------------------
// ターンチェック開始ハンドラ
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
  if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
  {
    if( BTL_CALC_IsOccurPer(33) )
    {
      BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );

      param->header.tokwin_flag = TRUE;
      param->sickCode = WAZASICK_EX_POKEFULL;
      param->pokeID[0] = pokeID;
      param->poke_cnt = 1;
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dappi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,      handler_Dappi },  // ターンチェック開始
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「ポイズンヒール」
 */
//------------------------------------------------------------------------------
// 状態異常ダメージハンドラ
static void handler_PoisonHeal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) )
  {
    WazaSick sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
    if( sick == WAZASICK_DOKU )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
      int recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, 0 );
      BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
      BTL_SERVER_RECEPT_HP_Add( flowWk, pokeID, recoverHP );
      BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_PoisonHeal( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SICK_DAMAGE,      handler_PoisonHeal }, // 状態異常ダメージハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「カブトアーマー」＆「シェルアーマー」
 */
//------------------------------------------------------------------------------
// クリティカルチェックダメージハンドラ
static void handler_KabutoArmor( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KabutoArmor( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CRITICAL_CHECK,     handler_KabutoArmor },  // クリティカルチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きょううん」
 */
//------------------------------------------------------------------------------
// クリティカルチェックダメージハンドラ
static void handler_Kyouun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    u8 rank = BTL_EVENTVAR_GetValue( BTL_EVAR_CRITICAL_RANK ) + 1;
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_CRITICAL_RANK, rank );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kyouun( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CRITICAL_CHECK,     handler_Kyouun }, // クリティカルチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「いかりのつぼ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_IkarinoTubo( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    // クリティカルだったら攻撃１段アップ
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_CRITICAL_FLAG) )
    {
      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->rankType = WAZA_RANKEFF_ATTACK;
        param->rankVolume = 1;
        param->fAlmost = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
      }
      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_IkarinoTubo( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_IkarinoTubo },  // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どくのトゲ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_DokunoToge( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  PokeSick sick = POKESICK_DOKU;
  BPP_SICK_CONT cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
  common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_DOKUNOTOGE_PER );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DokunoToge( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_DokunoToge }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「せいでんき」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Seidenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  PokeSick sick = POKESICK_MAHI;
  BPP_SICK_CONT cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
  common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_DOKUNOTOGE_PER );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Seidenki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_Seidenki }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ほのおのからだ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_HonoNoKarada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  PokeSick sick = POKESICK_YAKEDO;
  BPP_SICK_CONT cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
  common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_DOKUNOTOGE_PER );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HonoNoKarada( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_HonoNoKarada }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「メロメロボディ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_MeromeroBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF)==pokeID )
  {
    const BTL_POKEPARAM* myParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    const BTL_POKEPARAM* targetParam = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk,
          BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) );

    u8 mySex = BPP_GetValue( myParam, BPP_SEX );
    u8 targetSex = BPP_GetValue( targetParam, BPP_SEX );

    if( ((mySex!=PTL_SEX_UNKNOWN) && (targetSex!=PTL_SEX_UNKNOWN))
    &&  (mySex != targetSex)
    ){
      BPP_SICK_CONT cont;
      BTL_CALC_MakeDefaultWazaSickCont( WAZASICK_MEROMERO, myParam, &cont );
      common_touchAddSick( flowWk, pokeID, WAZASICK_MEROMERO, cont, BTL_CALC_TOK_MEROMEROBODY_PER );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MeromeroBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_MeromeroBody }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ほうし」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Housi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF)==pokeID )
  {
    PokeSick sick;
    BPP_SICK_CONT cont;
    u32 rand = GFL_STD_MtRand(90);
    if( rand < 30 ){
      sick = WAZASICK_DOKU;
    }else if( rand < 60 ){
      sick = WAZASICK_MAHI;
    }else{
      sick = WAZASICK_YAKEDO;
    }
    cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
    common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_HOUSI_PER );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Housi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_Housi },  // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//--------------------------------------------------------------------------
/**
 * 接触ワザによるダメージ後、相手に対して状態異常を発症させるとくせいの共通処理
 *
 * @param   flowWk
 * @param   pokeID
 * @param   sick      状態異常ID
 * @param   per       発生確率
 *
 */
//--------------------------------------------------------------------------
static void common_touchAddSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick, BPP_SICK_CONT sickCont, u8 per )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      if( BTL_CALC_IsOccurPer(per) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );

        param->sickID = sick;
        param->sickCont = sickCont;
        param->fAlmost = FALSE;
        param->poke_cnt = 1;
        param->pokeID[0] = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「さめはだ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Samehada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF)==pokeID )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      u8 attackerPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, attackerPokeID );

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = attackerPokeID;
        param->damage[0] = BTL_CALC_QuotMaxHP( bpp, 16 );
      }
      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Samehada( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_Samehada }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ゆうばく」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Yuubaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF)==pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_IsDead(bpp) )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
      {
        u8 attackerPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
        const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, attackerPokeID );

        BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = attackerPokeID;
          param->damage[0] = BTL_CALC_QuotMaxHP( bpp, 4 );
        }
        BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yuubaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_Yuubaku },  // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「へんしょく」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Hensyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( !BPP_IsDead(bpp) )
    {
      PokeType type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
      if( !BPP_IsMatchType(bpp, type) )
      {
        BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_CHANGE_TYPE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CHANGE_TYPE, pokeID );
          param->next_type = PokeTypePair_MakePure( type );
          param->pokeID = pokeID;
        }
        BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hensyoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_AFTER,     handler_Hensyoku }, // ダメージ直後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「シンクロ」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常ハンドラ
static void handler_Syncro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF)==pokeID )
  {
    PokeSick sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
    if( (sick==POKESICK_DOKU)||(sick==POKESICK_MAHI) ||(sick==POKESICK_YAKEDO) )
    {
      u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      if( targetPokeID != BTL_POKEID_NULL )
      {
        BTL_EVWK_ADDSICK* evwk = (BTL_EVWK_ADDSICK*)BTL_EVENTVAR_GetValue( BTL_EVAR_WORK_ADRS );
        evwk->reaction = BTL_EV_SICK_REACTION_REFRECT;
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Syncro( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MAKE_POKESICK,      handler_Syncro }, // ポケモン系状態異常ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


//------------------------------------------------------------------------------
/**
 *  とくせい「いしあたま」
 */
//------------------------------------------------------------------------------
// 反動計算ハンドラ
static void handler_Isiatama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_RATIO, 0 );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Isiatama( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_KICKBACK,      handler_Isiatama }, // 反動計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ノーマルスキン」
 */
//------------------------------------------------------------------------------
// ワザタイプ決定ハンドラ
static void handler_NormalSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_TYPE, POKETYPE_NORMAL );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NormalSkin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_PARAM,   handler_NormalSkin }, // ワザタイプ決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マルチタイプ」
 */
//------------------------------------------------------------------------------
// ワザタイプ決定ハンドラ
static void handler_MultiType( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID)
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetMonsNo(bpp) == MONSNO_ARUSEUSU )
    {
      // @@@ 持っているアイテムでタイプを変える
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_TYPE, POKETYPE_NORMAL );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MultiType( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_PARAM,   handler_MultiType },  // ワザタイプ決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「トレース」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Trace( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Trace },     // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// 入場ハンドラ
static void handler_Trace( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    u8  allPokeID[ BTL_POSIDX_MAX ];
    u8  targetPokeID[ BTL_POSIDX_MAX ];
    const BTL_POKEPARAM* bpp;
    u8  pokeCnt, targetCnt, i;
    PokeTokusei  tok;

    targetCnt = 0;
    pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, allPokeID );
    for(i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, allPokeID[i] );
      tok = BPP_GetValue( bpp, BPP_TOKUSEI );
      if( !BTL_CALC_TOK_CheckCantChange(tok) && (tok != POKETOKUSEI_TOREESU) )
      {
        targetPokeID[ targetCnt++ ] = allPokeID[i];
      }
    }
    if( targetCnt )
    {
      BTL_HANDEX_PARAM_CHANGE_TOKUSEI*  param;

      u8 idx = (targetCnt==1)? 0 : GFL_STD_MtRand( targetCnt );
      bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, targetPokeID[idx] );

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_CHANGE_TOKUSEI, pokeID );
      param->pokeID = pokeID;
      param->tokuseiID = BPP_GetValue( bpp, BPP_TOKUSEI );
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Trace );
      HANDEX_STR_AddArg( &param->exStr, targetPokeID[idx] );
      HANDEX_STR_AddArg( &param->exStr, param->tokuseiID );

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しぜんかいふく」
 */
//------------------------------------------------------------------------------
// メンバー退出ハンドラ
static void handler_SizenKaifuku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVFLOW_RECEPT_CurePokeSick( flowWk, pokeID );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SizenKaifuku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_OUT, handler_SizenKaifuku },   // メンバー退出ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ダウンロード」
 */
//------------------------------------------------------------------------------
// メンバー入場ハンドラ
static void handler_Download( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    u8 ePokeID[BTL_POSIDX_MAX];
    u8 pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    if( pokeCnt ){
      u8 idx = 0;
      if( pokeCnt > 1 ){
        idx = GFL_STD_MtRand( pokeCnt );
      }

      // 相手のぼうぎょ・とくぼうを比較して、弱い方に対応した自分のこうげき・とくこうをアップ
      {
        const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, ePokeID[idx] );
        u16 def = BPP_GetValue( bpp, BPP_DEFENCE );
        u16 sp_def = BPP_GetValue( bpp, BPP_SP_DEFENCE );
        u8 valueID;
        if( def > sp_def ){
          valueID = BPP_SP_ATTACK;
        }else if( def < sp_def ) {
          valueID = BPP_ATTACK;
        }else {
          valueID = (GFL_STD_MtRand(2))? BPP_SP_ATTACK : BPP_ATTACK;
        }

        {
          BtlPokePos myPos = BTL_SVFLOW_CheckExistFrontPokeID( flowWk, pokeID );
          BTL_EVENT_FACTOR_Remove( myHandle );
        }
      }
    }
    BTL_EVENT_FACTOR_Remove( myHandle );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DownLoad( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Download },    // メンバー入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「よちむ」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Yotimu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp;
    u8 ePokeID[BTL_POSIDX_MAX];
    u8 pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 pokeIdx = GFL_STD_MtRand( pokeCnt );
    bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, ePokeID[pokeIdx] );

    {
      u8 wazaCnt = BPP_WAZA_GetCount( bpp );
      u8 wazaIdx = GFL_STD_MtRand( wazaCnt );
      WazaID waza = BPP_WAZA_GetID( bpp, wazaIdx );

      BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
      BTL_SERVER_RECTPT_SetMessageEx( flowWk, BTL_STRID_SET_YotimuExe, pokeID, waza );
      BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );

      BTL_EVENT_FACTOR_Remove( myHandle );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yotimu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Yotimu }, // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きけんよち」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_KikenYoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bppUser;
    const BTL_POKEPARAM* bppEnemy;
    u8 ePokeID[BTL_POSIDX_MAX];
    u8 pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 i;
    bppUser = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    for(i=0; i<pokeCnt; ++i)
    {
      bppEnemy = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, ePokeID[i] );
      if( _check_kikenyoti_poke(bppUser, bppEnemy) )
      {
        break;
      }
    }
    if( i != pokeCnt )
    {
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Kikenyoti );
      HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BOOL _check_kikenyoti_poke( const BTL_POKEPARAM* bppUser, const BTL_POKEPARAM* bppEnemy )
{
  WazaID  waza;
  PokeTypePair user_type;
  PokeType     waza_type;
  u8 waza_cnt, i;

  user_type = BPP_GetPokeType( bppUser );
  waza_cnt = BPP_WAZA_GetCount( bppEnemy );
  for(i=0; i<waza_cnt; ++i)
  {
    // 一撃必殺ワザはキケン
    waza = BPP_WAZA_GetID( bppEnemy, i );
    if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_ICHIGEKI ){ return TRUE; }

    // 自分に相性バツグンな奴もキケン
    waza_type = WAZADATA_GetType( waza );
    if( BTL_CALC_TypeAff(waza_type, user_type) > BTL_TYPEAFF_100 ){ return TRUE; }

    // @@@ まだ自爆系を見ていない
  }
  return FALSE;
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KikenYoti( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_KikenYoti },  // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「おみとおし」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Omitoosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp;
    u16 itemID[ BTL_POSIDX_MAX ];
    u8  ePokeID[ BTL_POSIDX_MAX ];
    u8 pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 itemCnt, i;
    for(itemCnt=0, i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, ePokeID[i] );
      itemID[ itemCnt ] = BPP_GetItem( bpp );
      if( itemID[itemCnt] ){
        ++itemCnt;
      }
    }

    if( itemCnt ){
      i = GFL_STD_MtRand( itemCnt );
      BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
      BTL_SERVER_RECTPT_SetMessageEx( flowWk, BTL_STRID_SET_Omitoosi, pokeID, itemID[i] );
      BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
    }

    BTL_EVENT_FACTOR_Remove( myHandle );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Omitoosi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Omitoosi }, // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「がんじょう」
 */
//------------------------------------------------------------------------------
// 一撃チェックハンドラ
static void handler_Ganjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Ganjou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ICHIGEKI_CHECK, handler_Ganjou },   // 一撃チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「てんねん」
 */
//------------------------------------------------------------------------------
// 命中率・回避率チェックハンドラ
static void handler_Tennen_hitRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側の時、防御側の回避率をフラットに
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_AVOID_RANK, BTL_CALC_HITRATIO_MID );
  }
  // 自分が防御側の時、攻撃側の命中率をフラットに
  else if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_HIT_RANK, BTL_CALC_HITRATIO_MID );
  }
}
// 攻撃ランクチェックハンドラ
static void handler_Tennen_AtkRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時、攻撃側のランクをフラットに
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
// 防御ランクチェックハンドラ
static void handler_Tennen_DefRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側の時、防御側のランクをフラットに
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tennen( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ICHIGEKI_CHECK, handler_Tennen_hitRank },   // 命中率・回避率チェックハンドラ
    { BTL_EVENT_ATTACKER_POWER_PREV, handler_Tennen_AtkRank }, // 攻撃ランクチェックハンドラ
    { BTL_EVENT_DEFENDER_GUARD_PREV, handler_Tennen_DefRank }, // 防御ランクチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「たいねつ」
 */
//------------------------------------------------------------------------------
// ワザ威力計算ハンドラ
static void handler_Tainetsu_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時、炎ワザの威力を半分に
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetType(waza) == POKETYPE_HONOO )
    {
      u32 pow = BTL_CALC_Roundup( BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER ) / 2, 1 );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_POWER, pow );
    }
  }
}
// 状態異常ダメージ計算ハンドラ
static void handler_Tainetsu_SickDmg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が喰らう時、やけどならダメージ半分に
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    WazaSick sick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
    if( sick == WAZASICK_YAKEDO )
    {
      u32 dmg = BTL_CALC_Roundup( BTL_EVENTVAR_GetValue( BTL_EVAR_DAMAGE ) / 2, 1 );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, dmg );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tainetu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,   handler_Tainetsu_WazaPow },   // ワザ威力計算ハンドラ
    { BTL_EVENT_SICK_DAMAGE,  handler_Tainetsu_SickDmg },   // 状態異常ダメージ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}





static void common_DmgToRecover_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType matchType )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == matchType)
  ){
    work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );;
  }
}
static void common_DmgToRecover_Fix( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, u8 denomHP )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (work[0])
  ){
    BTL_HANDEX_PARAM_MESSAGE* msg_param;
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

    if( !BPP_IsHPFull(bpp) )
    {
      BTL_HANDEX_PARAM_RECOVER_HP*  param = BTL_SVFLOW_HANDLERWORK_Push( flowWk ,BTL_HANDEX_RECOVER_HP, pokeID );
      param->pokeID = pokeID;
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, denomHP );

      msg_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_HP_Recover );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    }
    else
    {
      msg_param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    }

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「かんそうはだ」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_Kansouhada_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    int damage = BTL_CALC_QuotMaxHP( bpp, 8 );
    BtlWeather weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );

    // 晴れの時、MAXHPの 1/8 減る
    if( weather == BTL_WEATHER_SHINE ){
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, damage );
    // 雨の時、MAXHPの 1/8 回復
    }else if( weather == BTL_WEATHER_RAIN ) {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, -damage );
    }
  }
}
// ダメージワザ回復化チェックハンドラ
static void handler_Kansouhada_DmgRecover( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_MIZU );
}
// ダメージワザ回復化決定ハンドラ
static void handler_Kansouhada_DmgRecoverFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Fix( flowWk, pokeID, work, 4 );
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kansouhada( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_WEATHER_DAMAGE,    handler_Kansouhada_Weather },       // 天候ダメージ計算ハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Kansouhada_DmgRecover },   // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_Kansouhada_DmgRecoverFix },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちょすい」
 */
//------------------------------------------------------------------------------
// ダメージワザ回復化チェックハンドラ
static void handler_Tyosui_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_MIZU );
}
// ダメージワザ回復化決定ハンドラ
static void handler_Tyosui_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Fix( flowWk, pokeID, work, 4 );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tyosui( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Tyosui_Check }, // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_Tyosui_Fix },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちくでん」
 */
//------------------------------------------------------------------------------
// ダメージワザ回復化チェックハンドラ
static void handler_Tikuden_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_DENKI );
}
// ダメージワザ回復化決定ハンドラ
static void handler_Tikuden_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Fix( flowWk, pokeID, work, 4 );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikuden( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Tikuden_Check }, // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_Tikuden_Fix },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「でんきエンジン」
 */
//------------------------------------------------------------------------------
// ダメージワザ回復化チェックハンドラ
static void handler_DenkiEngine_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_DENKI );
}
// ダメージワザ回復化決定ハンドラ
static void handler_DenkiEngine_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Fix( flowWk, pokeID, work, 4 );
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

    param->poke_cnt = 1;
    param->pokeID[0] = pokeID;
    param->fAlmost = TRUE;
    param->rankType = BPP_AGILITY;
    param->rankVolume = 1;
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_DenkiEngine( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_DenkiEngine_Check }, // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_DenkiEngine_Fix   },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ノーガード」
 */
//------------------------------------------------------------------------------
// タイプによる無効化チェックハンドラ
static void handler_NoGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が誰であれ、必中状態にする
  u8 atkID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
  const BTL_POKEPARAM* attacker = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, atkID );
  if( !BPP_CheckSick(attacker, WAZASICK_MUSTHIT) )
  {
    BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
    param->poke_cnt = 1;
    param->pokeID[0] = atkID;
    param->sickID = WAZASICK_MUSTHIT;;
    param->sickCont = BPP_SICKCONT_MakeTurn( 1 );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NoGuard( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_FIX,        handler_NoGuard },  // タイプによる無効化チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きもったま」
 */
//------------------------------------------------------------------------------
// タイプによる無効化チェックハンドラ
static void handler_Kimottama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // ゴーストタイプへの無効化を解除
    u8 defID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
    const BTL_POKEPARAM* defender = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, defID );
    if( BPP_IsMatchType(defender, POKETYPE_GHOST ) )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, FALSE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kimottama( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
     { BTL_EVENT_NOEFFECT_TYPE_CHECK,        handler_Kimottama },  // タイプによる無効化チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ぼうおん」
 */
//------------------------------------------------------------------------------
// 無効化チェックLv1ハンドラ
static void handler_Bouon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // サウンドワザ無効
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
    if( WAZADATA_GetFlag( waza, WAZAFLAG_Sound ) )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bouon( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L1,        handler_Bouon },  // 無効化チェックLv1ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ふゆう」
 */
//------------------------------------------------------------------------------
// 無効化チェックLv1ハンドラ
static void handler_Fuyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 場に「じゅうりょく」が効いていなければ
    if( BTL_FIELD_CheckEffect(BTL_FLDEFF_JURYOKU) )
    {
      // じめんタイプワザ無効
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
      if( WAZADATA_GetType(waza) == POKETYPE_JIMEN )
      {
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fuyuu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L1,        handler_Fuyuu },  // 無効化チェックLv1ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ふしぎなまもり」
 */
//------------------------------------------------------------------------------
// 無効化チェックLv2ハンドラ
static void handler_FusiginaMamori( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // ヌケニンなら
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    if( BPP_GetMonsNo(bpp) == MONSNO_NUKENIN )
    {
      // 効果バツグン以外は無効
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      PokeType waza_type = WAZADATA_GetType( waza );
      BtlTypeAff aff = BTL_CALC_TypeAff( waza_type, BPP_GetPokeType(bpp) );
      if( aff <= BTL_TYPEAFF_100 )
      {
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_FusiginaMamori( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L2,        handler_FusiginaMamori }, // 無効化チェックLv2ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「なまけ」
 */
//------------------------------------------------------------------------------
// ワザ出し成否チェックハンドラ
static void handler_Namake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
    u8 poke_turn = BPP_GetTurnCount( bpp );
    u8 sys_turn = BTL_SVFLOW_GetTurnCount( flowWk );
    if( (poke_turn&1) != (sys_turn&1) )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_NAMAKE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Namake( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_CHECK,       handler_Namake }, // ワザ出し成否チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「しめりけ」
 */
//------------------------------------------------------------------------------
// ワザ出し成否チェックハンドラ
static void handler_Simerike( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );

  if( (waza == WAZANO_DAIBAKUHATU)
  ||  (waza == WAZANO_ZIBAKU)
  ){
    SV_WazaFailCause cause = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_CAUSE );
    if( cause != SV_WAZAFAIL_NULL )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_TOKUSEI );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Simerike( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_CHECK,       handler_Simerike }, // ワザ出し成否チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「もらいび」
 */
//------------------------------------------------------------------------------
// 無効化チェックLv1ハンドラ
static void handler_Moraibi_NoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 炎ワザは無効＆「もらいび」発動
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    PokeType waza_type = WAZADATA_GetType( waza );
    if( waza_type == POKETYPE_HONOO )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
      if( work[0] == 0 )
      {
        work[0] = 1;  // 「もらいび」発動フラグとして利用
        BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
        BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_MoraibiExe, pokeID );
        BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
      }
    }
  }
}
// 攻撃力決定ハンドラ
static void handler_Moraibi_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 「もらいび」状態の時
    if( work[0] )
    {
      // 炎ワザの威力1.5倍
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetType(waza) == POKETYPE_HONOO )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_MORAIBI_POWRATIO );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Moraibi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L1,    handler_Moraibi_NoEffect }, // 無効化チェックLv1ハンドラ
    { BTL_EVENT_ATTACKER_POWER,       handler_Moraibi_AtkPower }, // 攻撃力決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ナイトメア」
 */
//------------------------------------------------------------------------------
// ターンチェック最終ハンドラ
static void handler_Nightmare( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  const BTL_POKEPARAM* bpp;
  u8 ePokeID[ BTL_POSIDX_MAX ];
  u8 pokeCnt = BTL_SVFLOW_RECEPT_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
  u8 exe_flag = FALSE;
  u8 i;
  for(i=0; i<pokeCnt; ++i)
  {
    bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, ePokeID[i] );
    if( BPP_CheckSick(bpp, WAZASICK_NEMURI) )
    {
      int dmg = BTL_CALC_QuotMaxHP( bpp, 8 ) * -1;

      if( exe_flag == FALSE ){
        BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
        exe_flag = TRUE;
      }
      BTL_SERVER_RECEPT_HP_Add( flowWk, pokeID, dmg );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nightmare( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,        handler_Nightmare },  // ターンチェック最終ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「にげあし」
 */
//------------------------------------------------------------------------------
// にげるチェックハンドラ
static void handler_Nigeasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nigeasi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SKIP_NIGERU_CALC,       handler_Nigeasi },  // にげるチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ありじごく」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Arijigoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVFLOW_RECEPT_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_ARIJIGOKU );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Arijigoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Arijigoku },  // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「かげふみ」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Kagefumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVFLOW_RECEPT_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_KAGEFUMI );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kagefumi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Kagefumi }, // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「じりょく」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Jiryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVFLOW_RECEPT_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_JIRYOKU );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Jiryoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Jiryoku },  // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

//------------------------------------------------------------------------------
/**
 *  とくせい「かたやぶり」
 */
//------------------------------------------------------------------------------
// ワザパラメータ決定ハンドラ
static void handler_Katayaburi_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Katayaburi );
    HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// スキップチェックハンドラ
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( factorType == BTL_EVENT_FACTOR_TOKUSEI )
  {
    // ターン無効化するとくせい群
    static const u16 disableTokTable[] = {
      POKETOKUSEI_FUSIGINAMAMORI, POKETOKUSEI_BOUON,      POKETOKUSEI_FUYUU,
      POKETOKUSEI_SUNAGAKURE,     POKETOKUSEI_YUKIGAKURE, POKETOKUSEI_TYOSUI,
      POKETOKUSEI_KABUTOAAMAA,    POKETOKUSEI_HIRAISIN,   POKETOKUSEI_YOBIMIZU,
      POKETOKUSEI_SHERUAAMAA,     POKETOKUSEI_TENNEN,     POKETOKUSEI_HIRAISIN,
      POKETOKUSEI_YOBIMIZU,       POKETOKUSEI_KYUUBAN,    POKETOKUSEI_TANJUN,
      POKETOKUSEI_TIDORIASI,      POKETOKUSEI_HAADOROKKU, POKETOKUSEI_FIRUTAA,
      POKETOKUSEI_MORAIBI,        POKETOKUSEI_DENKIENJIN, POKETOKUSEI_NENCHAKU,
      POKETOKUSEI_FUSIGINAUROKO,  POKETOKUSEI_ATUISIBOU,  POKETOKUSEI_TAINETU,
      POKETOKUSEI_SIROIKEMURI,    POKETOKUSEI_KURIABODY,  POKETOKUSEI_SURUDOIME,
      POKETOKUSEI_KAIRIKIBASAMI,  POKETOKUSEI_SEISINRYOKU,POKETOKUSEI_RINPUN,
      POKETOKUSEI_GANJOU,         POKETOKUSEI_SIMERIKE,   POKETOKUSEI_DONKAN,
      POKETOKUSEI_JUUNAN,         POKETOKUSEI_MAIPEESU,   POKETOKUSEI_MIZUNOBEERU,
      POKETOKUSEI_HONOONOKARADA,  POKETOKUSEI_RIIFUGAADO, POKETOKUSEI_FUMIN,
      POKETOKUSEI_YARUKI,         POKETOKUSEI_MENEKI,
    };
    u32 i;
    for(i=0; i<NELEMS(disableTokTable); ++i)
    {
      if( disableTokTable[i] == subID ){
        return TRUE;
      }
    }
  }
  return FALSE;
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Katayaburi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Katayaburi_MemberIn },  // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };

  BTL_EVENT_FACTOR* factor = BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
  if( factor ){
    BTL_EVENT_FACTOR_AttachSkipCheckHandler( factor, handler_Katayaburi_SkipCheck );
  }
  return factor;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「てんきや」
 */
//------------------------------------------------------------------------------
// ポケ入場ハンドラ
static void handler_Tenkiya_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BtlWeather weather = BTL_FIELD_GetWeather();
    common_TenkiFormChange( flowWk, pokeID, weather );
  }
}
// 天候変化ハンドラ
static void handler_Tenkiya_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BtlWeather weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );
    common_TenkiFormChange( flowWk, pokeID, weather );
  }
}
static void common_TenkiFormChange( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  const BTL_POKEPARAM* bpp = BTL_SVFLOW_RECEPT_GetPokeParam( flowWk, pokeID );
  if( BPP_GetMonsNo(bpp) == MONSNO_POWARUN )
  {
    u8 form_now, form_next;

    form_now = BPP_GetValue( bpp, BPP_FORM );
    switch( weather ){
    case BTL_WEATHER_NONE:  form_next = FORMNO_POWARUN_NORMAL; break;
    case BTL_WEATHER_SHINE: form_next = FORMNO_POWARUN_SUN; break;
    case BTL_WEATHER_RAIN:  form_next = FORMNO_POWARUN_RAIN; break;
    case BTL_WEATHER_SNOW:  form_next = FORMNO_POWARUN_SNOW; break;
    default:
      form_next = form_now;
    }

    if( form_next != form_now )
    {
      BTL_SERVER_RECEPT_TokuseiWinIn( flowWk, pokeID );
      BTL_SERVER_RECTPT_SetMessage( flowWk, BTL_STRID_SET_MoraibiExe, pokeID );
      BTL_SERVER_RECEPT_TokuseiWinOut( flowWk, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tenkiya( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Tenkiya_MemberIn  },  // ポケ入場ハンドラ
    { BTL_EVENT_WEATHER_CHANGE_AFTER, handler_Tenkiya_Weather   },  // 天候変化後ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「よびみず」
 */
//------------------------------------------------------------------------------
// ワザターゲット決定ハンドラ
static void handler_Yobimizu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_WazaTargetChangeToMe( flowWk, pokeID, POKETYPE_MIZU );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yobimizu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DECIDE_TARGET,    handler_Yobimizu  },  // ワザターゲット決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ひらいしん」
 */
//------------------------------------------------------------------------------
// ワザターゲット決定ハンドラ
static void handler_Hiraisin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_WazaTargetChangeToMe( flowWk, pokeID, POKETYPE_DENKI );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hiraisin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DECIDE_TARGET,    handler_Hiraisin  },  // ワザターゲット決定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//--------------------------------------------------------------------------
/**
 * とくていタイプのワザの対象を強制的に自分にするとくせいの共通処理
 *
 * @param   flowWk
 * @param   pokeID
 * @param   wazaType
 *
 */
//--------------------------------------------------------------------------
static void common_WazaTargetChangeToMe( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) != pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == wazaType )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_POKEID_DEF, pokeID );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きゅうばん」
 */
//------------------------------------------------------------------------------
// ふきとばしチェックハンドラ
static void handler_Kyuuban( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kyuuban( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_PUSHOUT,    handler_Kyuuban   },  // ふきとばしチェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ヘドロえき」
 */
//------------------------------------------------------------------------------
// ドレイン量計算ハンドラ
static void handler_HedoroEki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    u16 damage = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, 0 );
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      param->damage[0] = damage;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_HedoroEki );
      HANDEX_STR_AddArg( &param->exStr, param->pokeID[0] );

    }
    BTL_SVFLOW_HANDLERWORK_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_HedoroEki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_DRAIN,    handler_HedoroEki   },  // ドレイン量計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ぶきよう」
 */
//------------------------------------------------------------------------------
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactor factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( factorType == BTL_EVENT_FACTOR_ITEM ){
    if( BTL_EVENT_FACTOR_GetPokeID(myHandle) == pokeID ){
      return TRUE;
    }
  }
  return FALSE;
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bukiyou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NULL, NULL },
  };
  BTL_EVENT_FACTOR* factor = BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
  if( factor )
  {
    BTL_EVENT_FACTOR_AttachSkipCheckHandler( factor, handler_Bukiyou_SkipCheck );
  }
  return factor;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ねんちゃく」
 */
//------------------------------------------------------------------------------
static void handler_Nenchaku_NoEff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側のとき
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // トリック・すりかえは無効
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( (waza == WAZANO_TORIKKU)
    ||  (waza == WAZANO_SURIKAE)
    ){
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
    }
  }
}
static void handler_Nenchaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nenchaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L2,    handler_Nenchaku_NoEff },  // 無効化チェックLv2ハンドラ
    { BTL_EVENT_SET_ITEM_BEFORE,      handler_Nenchaku   },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}


