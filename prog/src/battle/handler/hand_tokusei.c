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
static BOOL common_GuardWazaSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick );
static void handler_AddSickFailCommon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
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
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Trace( u16 pri, u16 tokID, u8 pokeID );
static void handler_Trace( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
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
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Katayaburi_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Katayaburi_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
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
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bukiyou( u16 pri, u16 tokID, u8 pokeID );
static void handler_Nenchaku_NoEff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Nenchaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Nenchaku( u16 pri, u16 tokID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Pressure( u16 pri, u16 tokID, u8 pokeID );
static void handler_Pressure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_WaruiTeguse( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_WaruiTeguse( u16 pri, u16 tokID, u8 pokeID );
static void handler_NorowareBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NorowareBody( u16 pri, u16 tokID, u8 pokeID );
static void handler_KudakeruYoroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KudakeruYoroi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Tikarazuku_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tikarazuku_CheckFail( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikarazuku( u16 pri, u16 tokID, u8 pokeID );
static BOOL IsTikarazukuEffecive( WazaID waza );
static void handler_Makenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Makenki( u16 pri, u16 tokID, u8 pokeID );
static void handler_Yowaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yowaki( u16 pri, u16 tokID, u8 pokeID );
static void handler_MultiScale( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MultiScale( u16 pri, u16 tokID, u8 pokeID );
static void handler_NakamaIsiki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NakamaIsiki( u16 pri, u16 tokID, u8 pokeID );
static void handler_Moraterapi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Moraterapi( u16 pri, u16 tokID, u8 pokeID );
static void handler_Dokubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dokubousou( u16 pri, u16 tokID, u8 pokeID );
static void handler_Netubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Netubousou( u16 pri, u16 tokID, u8 pokeID );
static void handler_AunNoIki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AunNoIki( u16 pri, u16 tokID, u8 pokeID );
static void handler_Murakke( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* sys_work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Murakke( u16 pri, u16 tokID, u8 pokeID );
static void handler_Boujin_CalcDamage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Boujin_Reaction( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Boujin( u16 pri, u16 tokID, u8 pokeID );
static void handler_Dokusyu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dokusyu( u16 pri, u16 tokID, u8 pokeID );
static void handler_SaiseiRyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SaiseiRyoku( u16 pri, u16 tokID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hatomune( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hatomune_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hatomune_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunakaki( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sunakaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MilacreSkin( u16 pri, u16 tokID, u8 pokeID );
static void handler_MilacreSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinuti( u16 pri, u16 tokID, u8 pokeID );
static void handler_Sinuti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunanoTikara( u16 pri, u16 tokID, u8 pokeID );
static void handler_SunanoTikara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL handler_Surinuke_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Surinuke_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Surinuke_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surinuke( u16 pri, u16 tokID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_JisinKajou( u16 pri, u16 tokID, u8 pokeID );
static void handler_JisinKajou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SeiginoKokoro( u16 pri, u16 tokID, u8 pokeID );
static void handler_SeiginoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bibiri( u16 pri, u16 tokID, u8 pokeID );
static void handler_Bibiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Miira( u16 pri, u16 tokID, u8 pokeID );
static void handler_Miira( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sousyoku_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sousyoku_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sousyoku( u16 pri, u16 tokID, u8 pokeID );
static void handler_ItazuraGokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ItazuraGokoro( u16 pri, u16 tokID, u8 pokeID );
static void handler_MagicMirror_CheckRob( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagicMirror_Reflect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MagicMirror( u16 pri, u16 tokID, u8 pokeID );
static void handler_Syuukaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Syuukaku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Amanojaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amanojaku( u16 pri, u16 tokID, u8 pokeID );
static void handler_Kinchoukan_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_CheckItemEquip( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_MemberOutFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_KinchoukanOff( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kinchoukan( u16 pri, u16 tokID, u8 pokeID );
static void handler_Hensin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hensin( u16 pri, u16 tokID, u8 pokeID );
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Illusion( u16 pri, u16 tokID, u8 pokeID );
static void handler_Illusion_Damage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Illusion_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Illusion_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_IllusionBreak( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID );
static BOOL Tokusei_IsExePer( BTL_SVFLOW_WORK* flowWk, u8 per );


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
    { POKETOKUSEI_IKAKU,            HAND_TOK_ADD_Ikaku         },
    { POKETOKUSEI_KURIABODY,        HAND_TOK_ADD_ClearBody     },
    { POKETOKUSEI_SIROIKEMURI,      HAND_TOK_ADD_ClearBody     }, // しろいけむり=クリアボディと等価
    { POKETOKUSEI_SEISINRYOKU,      HAND_TOK_ADD_Seisinryoku   },
    { POKETOKUSEI_FUKUTUNOKOKORO,   HAND_TOK_ADD_Fukutsuno     },
    { POKETOKUSEI_ATUISIBOU,        HAND_TOK_ADD_AtuiSibou     },
    { POKETOKUSEI_KAIRIKIBASAMI,    HAND_TOK_ADD_KairikiBasami },
    { POKETOKUSEI_TIKARAMOTI,       HAND_TOK_ADD_Tikaramoti    },
    { POKETOKUSEI_YOGAPAWAA,        HAND_TOK_ADD_Tikaramoti    }, // ヨガパワー = ちからもちと等価
    { POKETOKUSEI_IROMEGANE,        HAND_TOK_ADD_Iromegane     },
    { POKETOKUSEI_KASOKU,           HAND_TOK_ADD_Kasoku        },
    { POKETOKUSEI_MOUKA,            HAND_TOK_ADD_Mouka         },
    { POKETOKUSEI_GEKIRYUU,         HAND_TOK_ADD_Gekiryu       },
    { POKETOKUSEI_SINRYOKU,         HAND_TOK_ADD_Sinryoku      },
    { POKETOKUSEI_MUSINOSIRASE,     HAND_TOK_ADD_MusinoSirase  },
    { POKETOKUSEI_KONJOU,           HAND_TOK_ADD_Konjou        },
    { POKETOKUSEI_SUKIRURINKU,      HAND_TOK_ADD_SkillLink     },
    { POKETOKUSEI_SURUDOIME,        HAND_TOK_ADD_Surudoime     },
    { POKETOKUSEI_TANJUN,           HAND_TOK_ADD_Tanjun        },
    { POKETOKUSEI_HAADOROKKU,       HAND_TOK_ADD_HardRock      },
    { POKETOKUSEI_FIRUTAA,          HAND_TOK_ADD_HardRock      }, // フィルター = ハードロックと等価
    { POKETOKUSEI_FUSIGINAUROKO,    HAND_TOK_ADD_FusiginaUroko },
    { POKETOKUSEI_TOUSOUSIN,        HAND_TOK_ADD_Tousousin     },
    { POKETOKUSEI_RIIFUGAADO,       HAND_TOK_ADD_ReafGuard     },
    { POKETOKUSEI_AMEFURASI,        HAND_TOK_ADD_Amefurasi     },
    { POKETOKUSEI_HIDERI,           HAND_TOK_ADD_Hideri        },
    { POKETOKUSEI_SUNAOKOSI,        HAND_TOK_ADD_Sunaokosi     },
    { POKETOKUSEI_YUKIFURASI,       HAND_TOK_ADD_Yukifurasi    },
    { POKETOKUSEI_EAROKKU,          HAND_TOK_ADD_AirLock       },
    { POKETOKUSEI_NOOTENKI,         HAND_TOK_ADD_AirLock       }, // ノーてんき = エアロックと等価
    { POKETOKUSEI_TEKUNISYAN,       HAND_TOK_ADD_Technician    },
    { POKETOKUSEI_DONKAN,           HAND_TOK_ADD_Donkan        },
    { POKETOKUSEI_URUOIBODY,        HAND_TOK_ADD_UruoiBody     },
    { POKETOKUSEI_POIZUNHIIRU,      HAND_TOK_ADD_PoisonHeal    },
    { POKETOKUSEI_AISUBODY,         HAND_TOK_ADD_IcoBody       },
    { POKETOKUSEI_AMEUKEZARA,       HAND_TOK_ADD_AmeukeZara    },
    { POKETOKUSEI_RINPUN,           HAND_TOK_ADD_Rinpun        },
    { POKETOKUSEI_TEKIOURYOKU,      HAND_TOK_ADD_Tekiouryoku   },
    { POKETOKUSEI_TENNOMEGUMI,      HAND_TOK_ADD_TennoMegumi   },
    { POKETOKUSEI_SANPAWAA,         HAND_TOK_ADD_SunPower      },
    { POKETOKUSEI_SUISUI,           HAND_TOK_ADD_Suisui        },
    { POKETOKUSEI_YOURYOKUSO,       HAND_TOK_ADD_Youryokuso    },
    { POKETOKUSEI_DAPPI,            HAND_TOK_ADD_Dappi         },
    { POKETOKUSEI_TIDORIASI,        HAND_TOK_ADD_Tidoriasi     },
    { POKETOKUSEI_HAYAASI,          HAND_TOK_ADD_Hayaasi       },
    { POKETOKUSEI_HARIKIRI,         HAND_TOK_ADD_Harikiri      },
    { POKETOKUSEI_KABUTOAAMAA,      HAND_TOK_ADD_KabutoArmor   },
    { POKETOKUSEI_SHERUAAMAA,       HAND_TOK_ADD_KabutoArmor   }, // シェルアーマー=カブトアーマーと等価
    { POKETOKUSEI_KYOUUN,           HAND_TOK_ADD_Kyouun        },
    { POKETOKUSEI_IKARINOTUBO,      HAND_TOK_ADD_IkarinoTubo   },
    { POKETOKUSEI_SUNAIPAA,         HAND_TOK_ADD_Sniper        },
    { POKETOKUSEI_TETUNOKOBUSI,     HAND_TOK_ADD_TetunoKobusi  },
    { POKETOKUSEI_FUKUGAN,          HAND_TOK_ADD_Fukugan       },
    { POKETOKUSEI_ISIATAMA,         HAND_TOK_ADD_Isiatama      },
    { POKETOKUSEI_SUTEMI ,          HAND_TOK_ADD_Sutemi        },
    { POKETOKUSEI_SEIDENKI ,        HAND_TOK_ADD_Seidenki      },
    { POKETOKUSEI_DOKUNOTOGE,       HAND_TOK_ADD_DokunoToge    },
    { POKETOKUSEI_HONOONOKARADA,    HAND_TOK_ADD_HonoNoKarada  },
    { POKETOKUSEI_HOUSI,            HAND_TOK_ADD_Housi         },
    { POKETOKUSEI_PURASU,           HAND_TOK_ADD_Plus          },
    { POKETOKUSEI_MAINASU,          HAND_TOK_ADD_Minus         },
    { POKETOKUSEI_MEROMEROBODY,     HAND_TOK_ADD_MeromeroBody  },
    { POKETOKUSEI_SUNAGAKURE ,      HAND_TOK_ADD_Sunagakure    },
    { POKETOKUSEI_YUKIGAKURE ,      HAND_TOK_ADD_Yukigakure    },
    { POKETOKUSEI_TOREESU,          HAND_TOK_ADD_Trace         },
    { POKETOKUSEI_NOOMARUSUKIN,     HAND_TOK_ADD_NormalSkin    },
    { POKETOKUSEI_SAMEHADA,         HAND_TOK_ADD_Samehada      },
    { POKETOKUSEI_SIZENKAIFUKU,     HAND_TOK_ADD_SizenKaifuku  },
    { POKETOKUSEI_SINKURO,          HAND_TOK_ADD_Syncro        },
    { POKETOKUSEI_DAUNROODO,        HAND_TOK_ADD_DownLoad      },
    { POKETOKUSEI_GANJOU,           HAND_TOK_ADD_Ganjou        },
    { POKETOKUSEI_TAINETU,          HAND_TOK_ADD_Tainetu       },
    { POKETOKUSEI_TENNEN,           HAND_TOK_ADD_Tennen        },
    { POKETOKUSEI_KANSOUHADA,       HAND_TOK_ADD_Kansouhada    },
    { POKETOKUSEI_TIKUDEN,          HAND_TOK_ADD_Tikuden       },
    { POKETOKUSEI_TYOSUI,           HAND_TOK_ADD_Tyosui        },
    { POKETOKUSEI_DENKIENJIN,       HAND_TOK_ADD_DenkiEngine   },
    { POKETOKUSEI_JUUNAN,           HAND_TOK_ADD_Juunan        },
    { POKETOKUSEI_FUMIN,            HAND_TOK_ADD_Fumin         },
    { POKETOKUSEI_YARUKI,           HAND_TOK_ADD_Fumin         }, // やるき=ふみんと等価
    { POKETOKUSEI_MAIPEESU,         HAND_TOK_ADD_MyPace        },
    { POKETOKUSEI_MAGUMANOYOROI,    HAND_TOK_ADD_MagumaNoYoroi },
    { POKETOKUSEI_MIZUNOBEERU,      HAND_TOK_ADD_MizuNoBale    },
    { POKETOKUSEI_MENEKI,           HAND_TOK_ADD_Meneki        },
    { POKETOKUSEI_NOOGAADO,         HAND_TOK_ADD_NoGuard       },
    { POKETOKUSEI_KIMOTTAMA,        HAND_TOK_ADD_Kimottama     },
    { POKETOKUSEI_BOUON,            HAND_TOK_ADD_Bouon         },
    { POKETOKUSEI_FUYUU,            HAND_TOK_ADD_Fuyuu         },
    { POKETOKUSEI_FURAWAAGIFUTO,    HAND_TOK_ADD_FlowerGift    },
    { POKETOKUSEI_MORAIBI,          HAND_TOK_ADD_Moraibi       },
    { POKETOKUSEI_YOTIMU,           HAND_TOK_ADD_Yotimu        },
    { POKETOKUSEI_KIKENYOTI,        HAND_TOK_ADD_KikenYoti     },
    { POKETOKUSEI_OMITOOSI,         HAND_TOK_ADD_Omitoosi      },
    { POKETOKUSEI_YOBIMIZU,         HAND_TOK_ADD_Yuubaku       },
    { POKETOKUSEI_NIGEASI,          HAND_TOK_ADD_Nigeasi       },
    { POKETOKUSEI_HENSYOKU,         HAND_TOK_ADD_Hensyoku      },
    { POKETOKUSEI_KATAYABURI,       HAND_TOK_ADD_Katayaburi    },
    { POKETOKUSEI_NAMAKE,           HAND_TOK_ADD_Namake        },
    { POKETOKUSEI_HIRAISIN,         HAND_TOK_ADD_Hiraisin      },
    { POKETOKUSEI_YOBIMIZU,         HAND_TOK_ADD_Yobimizu      },
    { POKETOKUSEI_SUROOSUTAATO,     HAND_TOK_ADD_SlowStart     },
    { POKETOKUSEI_ARIJIGOKU,        HAND_TOK_ADD_Arijigoku     },
    { POKETOKUSEI_KAGEFUMI,         HAND_TOK_ADD_Kagefumi      },
    { POKETOKUSEI_JIRYOKU,          HAND_TOK_ADD_Jiryoku       },
    { POKETOKUSEI_SIMERIKE,         HAND_TOK_ADD_Simerike      },
    { POKETOKUSEI_MARUTITAIPU,      HAND_TOK_ADD_MultiType     },
    { POKETOKUSEI_FUSIGINAMAMORI,   HAND_TOK_ADD_FusiginaMamori},
    { POKETOKUSEI_ATODASI,          HAND_TOK_ADD_Atodasi       },
    { POKETOKUSEI_TENKIYA,          HAND_TOK_ADD_Tenkiya       },
    { POKETOKUSEI_KYUUBAN,          HAND_TOK_ADD_Kyuuban       },
    { POKETOKUSEI_HEDOROEKI,        HAND_TOK_ADD_HedoroEki     },
    { POKETOKUSEI_BUKIYOU,          HAND_TOK_ADD_Bukiyou       },
    { POKETOKUSEI_NENCHAKU,         HAND_TOK_ADD_Nenchaku      },
    { POKETOKUSEI_PURESSYAA,        HAND_TOK_ADD_Pressure      },

    { POKETOKUSEI_WARUITEGUSE,      HAND_TOK_ADD_WaruiTeguse   },
    { POKETOKUSEI_TIKARAZUKU,       HAND_TOK_ADD_Tikarazuku    },
    { POKETOKUSEI_MAKENKI,          HAND_TOK_ADD_Makenki       },
    { POKETOKUSEI_YOWAKI,           HAND_TOK_ADD_Yowaki        },
    { POKETOKUSEI_MARUTISUKEIRU,    HAND_TOK_ADD_MultiScale    },
    { POKETOKUSEI_AMANOJAKU,        HAND_TOK_ADD_Amanojaku     }, // あまのじゃく
    { POKETOKUSEI_KINCHOUKAN,       HAND_TOK_ADD_Kinchoukan    }, // きんちょうかん
    { POKETOKUSEI_NOROWAREBODY,     HAND_TOK_ADD_NorowareBody  }, // のろわれボディ
    { POKETOKUSEI_MORATERAPII,      HAND_TOK_ADD_Moraterapi    }, // モラテラピー
    { POKETOKUSEI_NAKAMAISIKI,      HAND_TOK_ADD_NakamaIsiki   }, // なかまいしき
    { POKETOKUSEI_KUDAKERUYOROI,    HAND_TOK_ADD_KudakeruYoroi }, // くだけるよろい
    { POKETOKUSEI_DOKUBOUSOU,       HAND_TOK_ADD_Dokubousou    }, // どくぼうそう
    { POKETOKUSEI_NETUBOUSOU,       HAND_TOK_ADD_Netubousou    }, // ねつぼうそう
    { POKETOKUSEI_SYUUKAKU,         HAND_TOK_ADD_Syuukaku      }, // しゅうかく
    { POKETOKUSEI_AUNNOIKI,         HAND_TOK_ADD_AunNoIki      }, // あうんのいき
    { POKETOKUSEI_MURAKKE,          HAND_TOK_ADD_Murakke       }, // ムラっけ
    { POKETOKUSEI_BOUJIN,           HAND_TOK_ADD_Boujin        }, // ぼうじん
    { POKETOKUSEI_DOKUSYU,          HAND_TOK_ADD_Dokusyu       }, // どくしゅ
    { POKETOKUSEI_SAISEIRYOKU,      HAND_TOK_ADD_SaiseiRyoku   }, // さいせいりょく
    { POKETOKUSEI_HATOMUNE,         HAND_TOK_ADD_Hatomune      }, // はとむね
    { POKETOKUSEI_SUNAKAKI,         HAND_TOK_ADD_Sunakaki      }, // すなかき
    { POKETOKUSEI_MIRAKURUSUKIN,    HAND_TOK_ADD_MilacreSkin   }, // ミラクルスキン
    { POKETOKUSEI_SINUTI,           HAND_TOK_ADD_Sinuti        }, // しんうち
    { POKETOKUSEI_IRYUUJON,         HAND_TOK_ADD_Illusion      }, // イリュージョン
    { POKETOKUSEI_HENSIN,           HAND_TOK_ADD_Hensin        }, // へんしん
    { POKETOKUSEI_SURINUKE,         HAND_TOK_ADD_Surinuke      }, // すりぬけ
    { POKETOKUSEI_MIIRA,            HAND_TOK_ADD_Miira         }, // ミイラ
    { POKETOKUSEI_JISINKAJOU,       HAND_TOK_ADD_JisinKajou    }, // じしんかじょう
    { POKETOKUSEI_SEIGINOKOKORO,    HAND_TOK_ADD_SeiginoKokoro }, // せいぎのこころ
    { POKETOKUSEI_BIBIRI,           HAND_TOK_ADD_Bibiri        }, // びびり
    { POKETOKUSEI_MAJIKKUMIRAA,     HAND_TOK_ADD_MagicMirror   }, // マジックミラー
    { POKETOKUSEI_SOUSYOKU,         HAND_TOK_ADD_Sousyoku      }, // そうしょく
    { POKETOKUSEI_ITAZURAGOKORO,    HAND_TOK_ADD_ItazuraGokoro }, // いたずらごころ
    { POKETOKUSEI_SUNANOTIKARA,     HAND_TOK_ADD_SunanoTikara  }, // すなのちから
  };

//  if( !BPP_CheckSick(pp, WAZASICK_IEKI) )
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

//=============================================================================================
/**
 * ランダム発動とくせいの共通チェックルーチン
 */
//=============================================================================================
static BOOL Tokusei_IsExePer( BTL_SVFLOW_WORK* flowWk, u8 per )
{
  if( BTL_CALC_IsOccurPer(per) ){
    return TRUE;
  }
  if( BTL_SVFTOOL_GetDebugFlag(flowWk, BTL_DEBUGFLAG_MUST_TOKUSEI) ){
    return TRUE;
  }
  return FALSE;
}


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
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

      param->rankType = BPP_ATTACK;
      param->rankVolume = -1;
      param->fAlmost = TRUE;

      {
        BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokeID( flowWk, pokeID );
        BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );

        param->poke_cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, param->pokeID );
      }
    }
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.75) );
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
      // ダメージ1.5倍（クリティカルなので実質３倍）
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.5) );
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
  BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    u32  borderHP = BTL_CALC_QuotMaxHP( bpp, 3 );
    u32  hp = BPP_GetValue( bpp, BPP_HP );
    BTL_Printf("hp=%d, borderHP=%d\n", hp, borderHP);
    if( hp <= borderHP )
    {
      // 使うのが指定タイプワザなら
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == wazaType )
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
    if( BTL_SVFTOOL_CheckExistTokuseiPokemon(flowWk, POKETOKUSEI_MAINASU) )
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
    if( BTL_SVFTOOL_CheckExistTokuseiPokemon(flowWk, POKETOKUSEI_PURASU) )
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
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FLOWERGIFT_GUARDRATIO );
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
    const BTL_POKEPARAM* myParam = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    const BTL_POKEPARAM* targetParam = BTL_SVFTOOL_GetPokeParam( flowWk,
          BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) );

    u8 mySex = BPP_GetValue( myParam, BPP_SEX );
    u8 targetSex = BPP_GetValue( targetParam, BPP_SEX );

    // 互いに性別不明じゃない場合
    if( (mySex!=PTL_SEX_UNKNOWN) && (targetSex!=PTL_SEX_UNKNOWN) )
    {
      if( mySex == targetSex ){
        BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.25) );

      }else{
        BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.75) );
      }
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, BTL_CALC_TOK_TECKNICIAN_POWRATIO );
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, BTL_CALC_TOK_TETUNOKOBUSI_POWRATIO );
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, BTL_CALC_TOK_SUTEMI_POWRATIO );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      // ダメージタイプが特殊の時、特防上昇
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_SPECIAL )
      {
        BTL_Printf("ふしぎなうろこ発動したよ\n");
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FUSIGINAUROKO_GDRATIO );
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
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) != pokeID)
  ){
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

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );

    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
    HANDEX_STR_AddArg( &param->str, pokeID );

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

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
  work[0] = common_GuardWazaSick( flowWk, pokeID, POKESICK_MAHI );
}
// とくせい書き換えハンドラ
static void handler_Juunan_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_MAHI );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Juunan( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_Juunan_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Juunan_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
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
  work[0] = common_GuardWazaSick( flowWk, pokeID, WAZASICK_NEMURI );
  if( work[0] == 0 ){
    work[0] = common_GuardWazaSick( flowWk, pokeID, WAZASICK_AKUBI );
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
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_Fumin_PokeSick }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Fumin_Swap },     // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
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
  work[0] = common_GuardWazaSick( flowWk, pokeID, POKESICK_YAKEDO );
}
// とくせい書き換えハンドラ
static void handler_MagumaNoYoroi_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_YAKEDO );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MagumaNoYoroi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_MagumaNoYoroi_PokeSick }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_MagumaNoYoroi_Swap },     // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon },
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
  work[0] = common_GuardWazaSick( flowWk, pokeID, POKESICK_DOKU );
}
// とくせい書き換えハンドラ
static void handler_Meneki_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, POKESICK_DOKU );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Meneki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_Meneki_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Meneki_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon },
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
  work[0] = common_GuardWazaSick( flowWk, pokeID, WAZASICK_KONRAN );
}
// とくせい書き換えハンドラ
static void handler_MyPace_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, WAZASICK_KONRAN );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MyPace( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_MyPace_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_MyPace_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon },
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
  work[0] = common_GuardWazaSick( flowWk, pokeID, WAZASICK_YAKEDO );
}
// とくせい書き換えハンドラ
static void handler_MizuNoBale_Swap( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_Swap_CureSick( flowWk, pokeID, WAZASICK_YAKEDO );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MizuNoBale( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_MizuNoBale_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_MizuNoBale_Swap },      // とくせい書き換えハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
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
static BOOL common_GuardWazaSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick guardSick )
{
  // くらう側が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // くらう病気が指定通り
    WazaSick  attackSick = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
    if( (guardSick == attackSick)
    ||  ((guardSick == WAZASICK_DOKU) && (attackSick == WAZASICK_DOKUDOKU))
    ){
      return BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
    }
  }
  return FALSE;
}
static void handler_AddSickFailCommon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (work[0] == TRUE)
  ){
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );

    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
    HANDEX_STR_AddArg( &param->str, pokeID );

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    work[0] = FALSE;
  }
}
static void common_Swap_CureSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CheckSick(bpp, sick) )
    {
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
        param->sickCode = sick;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    BTL_HANDEX_PARAM_CHANGE_WEATHER* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_WEATHER, pokeID );

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
      BTL_HANDEX_PARAM_CHANGE_WEATHER* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_WEATHER, pokeID );

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
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_WeatherLocked );
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    { BTL_EVENT_WEATHER_REACTION, handler_IceBody },   // ターンチェック開始ハンドラ
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
    { BTL_EVENT_WEATHER_REACTION, handler_AmeukeZara },  // 天候ダメージ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

/**
 *  天候に応じてHP回復するとくせいの共通処理
 */
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == weather )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    {
      BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
      param->header.tokwin_flag = TRUE;
      param->pokeID = pokeID;
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
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
  // 晴れの時、MAXHPの 1/8 減る
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == BTL_WEATHER_SHINE )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      int damage = BTL_CALC_QuotMaxHP( bpp, 8 );
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
        param->pokeID = pokeID;
        param->damage = damage;
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    { BTL_EVENT_WEATHER_REACTION, handler_SunPower_Weather },  // 天候ダメージ計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER,  handler_SunPower_AtkPower },  // 攻撃力決定ハンドラ
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
  if( (pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF))
  &&  (pokeID != BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK))
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      BTL_HANDEX_PARAM_CURE_SICK* param;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
      param->sickCode = WAZASICK_EX_POKEFULL;
      param->pokeID[0] = pokeID;
      param->poke_cnt = 1;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

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
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
  {
    if( Tokusei_IsExePer(flowWk, 33) )
    {
      BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );

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
    if( (sick == WAZASICK_DOKU) || (sick == WAZASICK_DOKUDOKU) )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      BTL_HANDEX_PARAM_RECOVER_HP* param;

      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, 0 );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
      param->pokeID = pokeID;
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

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
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->rankType = WAZA_RANKEFF_ATTACK;
        param->rankVolume = 1;
        param->fAlmost = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_IkarinoTubo( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_IkarinoTubo },  // ダメージ直後ハンドラ
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
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_DokunoToge }, // ダメージ直後ハンドラ
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
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Seidenki }, // ダメージ直後ハンドラ
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
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_HonoNoKarada }, // ダメージ直後ハンドラ
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
    const BTL_POKEPARAM* myParam = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    const BTL_POKEPARAM* targetParam = BTL_SVFTOOL_GetPokeParam( flowWk,
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
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_MeromeroBody }, // ダメージ直後ハンドラ
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
    u32 rand = BTL_SVFTOOL_GetRand( flowWk, 90);
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
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Housi },  // ダメージ直後ハンドラ
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
      if( Tokusei_IsExePer(flowWk, per) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );

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
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, attackerPokeID );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
        param->pokeID = attackerPokeID;
        param->damage = BTL_CALC_QuotMaxHP( bpp, 16 );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Samehada( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Samehada }, // ダメージ直後ハンドラ
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_IsDead(bpp) )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
      {
        u8 attackerPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
        const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, attackerPokeID );

        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
          param->pokeID = attackerPokeID;
          param->damage = BTL_CALC_QuotMaxHP( bpp, 4 );
        }
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yuubaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Yuubaku },  // ダメージ直後ハンドラ
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( !BPP_IsDead(bpp) )
    {
      PokeType type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
      if( !BPP_IsMatchType(bpp, type) )
      {
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_CHANGE_TYPE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TYPE, pokeID );
          param->next_type = PokeTypePair_MakePure( type );
          param->pokeID = pokeID;
        }
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hensyoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Hensyoku }, // ダメージ直後ハンドラ
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
      u8 attackPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      if( attackPokeID != BTL_POKEID_NULL )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param;

        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = attackPokeID;
        param->sickID = sick;
        param->sickCont = BPP_SICKCONT_MakePermanent();
        param->fAlmost = TRUE;

        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Syncro( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_POKESICK_FIXED,      handler_Syncro }, // ポケモン系状態異常ハンドラ
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
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
    pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, allPokeID );
    for(i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, allPokeID[i] );
      tok = BPP_GetValue( bpp, BPP_TOKUSEI );
      if( !BTL_CALC_TOK_CheckCantChange(tok) && (tok != POKETOKUSEI_TOREESU) )
      {
        targetPokeID[ targetCnt++ ] = allPokeID[i];
      }
    }
    if( targetCnt )
    {
      BTL_HANDEX_PARAM_CHANGE_TOKUSEI*  param;

      u8 idx = (targetCnt==1)? 0 : BTL_SVFTOOL_GetRand( flowWk, targetCnt );
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID[idx] );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TOKUSEI, pokeID );
      param->pokeID = pokeID;
      param->tokuseiID = BPP_GetValue( bpp, BPP_TOKUSEI );
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Trace );
      HANDEX_STR_AddArg( &param->exStr, targetPokeID[idx] );
      HANDEX_STR_AddArg( &param->exStr, param->tokuseiID );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
    param->sickCode = WAZASICK_EX_POKEFULL;
    param->poke_cnt = 1;
    param->pokeID[0] = pokeID;
    param->fStdMsgDisable = TRUE;
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SizenKaifuku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_OUT_FIXED, handler_SizenKaifuku },   // メンバー退出ハンドラ
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
    u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    if( pokeCnt ){
      u8 idx = 0;
      if( pokeCnt > 1 ){
        idx = BTL_SVFTOOL_GetRand( flowWk, pokeCnt );
      }

      // 相手のぼうぎょ・とくぼうを比較して、弱い方に対応した自分のこうげき・とくこうをアップ
      {
        const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[idx] );
        u16 def = BPP_GetValue( bpp, BPP_DEFENCE );
        u16 sp_def = BPP_GetValue( bpp, BPP_SP_DEFENCE );
        u8 valueID;
        if( def > sp_def ){
          valueID = BPP_SP_ATTACK;
        }else if( def < sp_def ) {
          valueID = BPP_ATTACK;
        }else {
          valueID = (BTL_SVFTOOL_GetRand( flowWk, 2))? BPP_SP_ATTACK : BPP_ATTACK;
        }

        {
          BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokeID( flowWk, pokeID );
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
    u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 pokeIdx = BTL_SVFTOOL_GetRand( flowWk, pokeCnt );
    bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[pokeIdx] );

    {
      u8 wazaCnt = BPP_WAZA_GetCount( bpp );
      u8 wazaIdx = BTL_SVFTOOL_GetRand( flowWk, wazaCnt );
      WazaID waza = BPP_WAZA_GetID( bpp, wazaIdx );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_YotimuExe );
        HANDEX_STR_AddArg( &param->str, pokeID );
        HANDEX_STR_AddArg( &param->str, waza );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );


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
    u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 i;
    bppUser = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    for(i=0; i<pokeCnt; ++i)
    {
      bppEnemy = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
      if( _check_kikenyoti_poke(bppUser, bppEnemy) )
      {
        break;
      }
    }
    if( i != pokeCnt )
    {
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Kikenyoti );
      HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 itemCnt, i;
    for(itemCnt=0, i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
      itemID[ itemCnt ] = BPP_GetItem( bpp );
      if( itemID[itemCnt] ){
        ++itemCnt;
      }
    }

    if( itemCnt ){
      i = BTL_SVFTOOL_GetRand( flowWk, itemCnt );
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Omitoosi );
        HANDEX_STR_AddArg( &param->str, pokeID );
        HANDEX_STR_AddArg( &param->str, itemID[i] );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_FLAG, TRUE) )
    {
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
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
    { BTL_EVENT_WAZA_HIT_RANK,       handler_Tennen_hitRank }, // 命中率・回避率チェックハンドラ
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(0.5) );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

    if( !BPP_IsHPFull(bpp) )
    {
      BTL_HANDEX_PARAM_RECOVER_HP*  param = BTL_SVF_HANDEX_Push( flowWk ,BTL_HANDEX_RECOVER_HP, pokeID );
      param->pokeID = pokeID;
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, denomHP );

      msg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_HP_Recover );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    }
    else
    {
      msg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
    }

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    BtlWeather weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );

    // 晴れの時、MAXHPの 1/8 減る
    if( weather == BTL_WEATHER_SHINE )
    {
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
        param->pokeID = pokeID;
        param->damage = BTL_CALC_QuotMaxHP( bpp, 8 );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    // 雨の時、MAXHPの 1/8 回復
    }
    else if( weather == BTL_WEATHER_RAIN )
    {
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
        param->pokeID = pokeID;
        param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    { BTL_EVENT_WEATHER_REACTION,    handler_Kansouhada_Weather },      // 天候ダメージ計算ハンドラ
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
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

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
  const BTL_POKEPARAM* attacker = BTL_SVFTOOL_GetPokeParam( flowWk, atkID );
  if( !BPP_CheckSick(attacker, WAZASICK_MUSTHIT) )
  {
    BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
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
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKE_TYPE) == POKETYPE_GHOST )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_FLAT_FLAG, TRUE );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kimottama( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_AFFINITY,        handler_Kimottama },  // タイプによる無効化チェックハンドラ
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
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag( waza, WAZAFLAG_Sound ) )
    {
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
          HANDEX_STR_AddArg( &param->str, pokeID );
        }
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
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
// 飛行フラグチェックハンドラ
static void handler_Fuyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Fuyuu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_FLYING,        handler_Fuyuu },  // 飛行フラグチェックハンドラ
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetMonsNo(bpp) == MONSNO_NUKENIN )
    {
      // 効果バツグン以外は無効
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      PokeType waza_type = WAZADATA_GetType( waza );
      BtlTypeAff aff = BTL_CALC_TypeAff( waza_type, BPP_GetPokeType(bpp) );
      if( aff <= BTL_TYPEAFF_100 )
      {
        if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
        {
          BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
          {
            BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
            HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
            HANDEX_STR_AddArg( &param->str, pokeID );
          }
          BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
        }
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    u8 poke_turn = BPP_GetTurnCount( bpp );
    u8 sys_turn = BTL_SVFTOOL_GetTurnCount( flowWk );
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
    if( cause == SV_WAZAFAIL_NULL )
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
// ダメージワザ回復化チェックハンドラ
static void handler_Moraibi_NoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 炎ワザは無効＆「もらいび」発動
    PokeType waza_type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
    if( waza_type == POKETYPE_HONOO )
    {
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
    }
  }
}
// ダメージワザ回復化決定ハンドラ
static void handler_Moraibi_DmgRecoverFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    if( work[0] == 0 )
    {
      work[0] = 1;  // 「もらいび」発動フラグとして利用

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MoraibiExe );
        HANDEX_STR_AddArg( &param->str, pokeID );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    { BTL_EVENT_DMG_TO_RECOVER_CHECK, handler_Moraibi_NoEffect      }, // ダメージワザ回復化チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,   handler_Moraibi_DmgRecoverFix }, // ダメージ回復化決定ハンドラ
    { BTL_EVENT_ATTACKER_POWER,       handler_Moraibi_AtkPower      }, // 攻撃力決定ハンドラ
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
  u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
  u8 exe_flag = FALSE;
  u8 i;
  for(i=0; i<pokeCnt; ++i)
  {
    bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
    if( BPP_CheckSick(bpp, WAZASICK_NEMURI) )
    {
      BTL_HANDEX_PARAM_SHIFT_HP* param;

      if( exe_flag == FALSE ){
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        exe_flag = TRUE;
      }

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SHIFT_HP, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->volume[0] = BTL_CALC_QuotMaxHP( bpp, 8 ) * -1;

    }
  }
  if( exe_flag ){
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    BTL_SVFRET_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_ARIJIGOKU );
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
    BTL_SVFRET_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_KAGEFUMI );
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
    BTL_SVFRET_CantEscapeAdd( flowWk, pokeID, BTL_CANTESC_JIRYOKU );
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
// メンバー入場ハンドラ
static void handler_Katayaburi_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Katayaburi );
    HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// スキップチェックハンドラ
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  BTL_Printf("かたやぶりのスキップチェックハンドラが呼ばれた\n");
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
        BTL_Printf("これはスキップさせるとくせいだ\n");
        return TRUE;
      }
    }
  }
  return FALSE;
}
// ワザ処理開始ハンドラ
static void handler_Katayaburi_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側ならスキップチェックハンドラをアタッチする
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Katayaburi_SkipCheck );
  }
}
// ワザ処理終了ハンドラ
static void handler_Katayaburi_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側ならスキップチェックハンドラをデタッチする
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
  }
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Katayaburi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Katayaburi_MemberIn },  // 入場ハンドラ
    { BTL_EVENT_WAZASEQ_START,    handler_Katayaburi_Start },     // ワザ処理開始ハンドラ
    { BTL_EVENT_WAZASEQ_END,      handler_Katayaburi_End   },     // ワザ処理終了ハンドラ
    { BTL_EVENT_NULL, NULL },
  };

  BTL_EVENT_FACTOR* factor = BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
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
  BtlWeather weather = BTL_EVENTVAR_GetValue( BTL_EVAR_WEATHER );
  common_TenkiFormChange( flowWk, pokeID, weather );
}
static void common_TenkiFormChange( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
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
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MoraibiExe );
        HANDEX_STR_AddArg( &param->str, pokeID );
      }
      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
      param->pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      param->damage = damage;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_HedoroEki );
      HANDEX_STR_AddArg( &param->exStr, param->pokeID );

    }
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
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
    ||  (waza == WAZANO_DOROBOU)
    ){
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
          HANDEX_STR_AddArg( &param->str, pokeID );
        }
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
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
    { BTL_EVENT_ITEMSET_CHECK,        handler_Nenchaku   },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「プレッシャー」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Pressure( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DECREMENT_PP_VOLUME,  handler_Pressure },  // PP消費チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// PP消費チェックハンドラ
static void handler_Pressure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u32 cnt, i;
  cnt = BTL_EVENTVAR_GetValue( BTL_EVAR_TARGET_POKECNT );
  for(i=0; i<cnt; ++i)
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_TARGET1+i) == pokeID )
    {
      u8 dec = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
      ++dec;
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, dec );
      break;
    }
  }
}


//------------------------------------------------------------------------------
/**
 *  とくせい「わるいてぐせ」
 */
//------------------------------------------------------------------------------
static void handler_WaruiTeguse( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    const BTL_POKEPARAM* me = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    const BTL_POKEPARAM* attacker = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );

    if( (BPP_GetItem(me) == ITEM_DUMMY_DATA) && (BPP_GetItem(attacker) != ITEM_DUMMY_DATA) )
    {
      BTL_HANDEX_PARAM_SWAP_ITEM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SWAP_ITEM, pokeID );
      param->header.tokwin_flag = TRUE;
      param->pokeID = targetPokeID;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_WaruiTeguse );
      HANDEX_STR_AddArg( &param->exStr, targetPokeID );
      HANDEX_STR_AddArg( &param->exStr, BPP_GetItem(attacker) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_WaruiTeguse( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,    handler_WaruiTeguse },  // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「のろわれボディ」
 */
//------------------------------------------------------------------------------
static void handler_NorowareBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    const BTL_POKEPARAM* target = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );

    if( !BPP_CheckSick(target, WAZASICK_KANASIBARI) )
    {
      if( Tokusei_IsExePer(flowWk, 30) )
      {
        WazaID  prevWaza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
        if( prevWaza != WAZANO_NULL )
        {
          BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );

          param->sickID = WAZASICK_KANASIBARI;
          param->sickCont = BPP_SICKCONT_MakeTurnParam( 4, prevWaza );
          param->poke_cnt = 1;
          param->pokeID[0] = targetPokeID;
          param->header.tokwin_flag = TRUE;
          BTL_Printf("のろわれボディ発動\n");
        }
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NorowareBody( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,    handler_NorowareBody },  // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「くだけるよろい」
 */
//------------------------------------------------------------------------------
static void handler_KudakeruYoroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    BOOL fEffective = FALSE;

    if( BPP_IsRankEffectValid(bpp, BPP_DEFENCE, -1)
    ||  BPP_IsRankEffectValid(bpp, BPP_AGILITY,  1)
    ){
      fEffective = TRUE;
    }

    if( fEffective )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->fAlmost = TRUE;
        param->rankType = BPP_DEFENCE;
        param->rankVolume = -1;

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->fAlmost = TRUE;
        param->rankType = BPP_AGILITY;
        param->rankVolume = 1;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_KudakeruYoroi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,    handler_KudakeruYoroi },  // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちからずく」
 */
//------------------------------------------------------------------------------
// 威力計算ハンドラ
static void handler_Tikarazuku_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( IsTikarazukuEffecive(waza) )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.3) );
    }
  }
}
// 追加効果チェックハンドラ
static void handler_Tikarazuku_CheckFail( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) )
  {
    WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( IsTikarazukuEffecive(waza) )
    {
      BTL_Printf("ちからずくなので効果が発生しません\n");
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
    }
  }
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Tikarazuku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Tikarazuku_WazaPow   }, // ワザ威力計算ハンドラ
    { BTL_EVENT_ADD_SICK,        handler_Tikarazuku_CheckFail }, // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET, handler_Tikarazuku_CheckFail }, // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

/**
 *  「ちからずく」が効くワザかどうか判定
 */
static BOOL IsTikarazukuEffecive( WazaID waza )
{
  WazaCategory category = WAZADATA_GetCategory( waza );

  switch( category ){
  // 殴った相手に効果が発動するタイプは無条件に有効
  case WAZADATA_CATEGORY_DAMAGE_SICK:
  case WAZADATA_CATEGORY_DAMAGE_EFFECT:
    return TRUE;

  // 自分に効果が発動するタイプは、ランクダウンの効果があるワザのみ無効
  case WAZADATA_CATEGORY_DAMAGE_EFFECT_USER:
    {
      u32 numEffects = WAZADATA_GetRankEffectCount( waza );
      int value;
      u32 i;
      for(i=0; i<numEffects; ++i)
      {
        WAZADATA_GetRankEffect( waza, i, &value );
        if( value < 0 ){
          return FALSE;
        }
      }
      return TRUE;
    }
    break;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「まけんき」
 */
//------------------------------------------------------------------------------
static void handler_Makenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    int volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    if( volume < 0 )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

      param->header.tokwin_flag = TRUE;
      param->rankType = BPP_ATTACK;
      param->rankVolume = 2;
      param->fAlmost = TRUE;
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Makenki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_FIXED,    handler_Makenki },  // ランク効果確定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「よわき」
 */
//------------------------------------------------------------------------------
static void handler_Yowaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    u32 hp = BPP_GetValue( bpp, BPP_HP );
    u32 hp_half = BTL_CALC_QuotMaxHP( bpp, 2 );
    if( hp <= hp_half )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5f) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Yowaki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER,    handler_Yowaki },  // 攻撃側能力の計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マルチスケイル」
 */
//------------------------------------------------------------------------------
static void handler_MultiScale( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // HP満タンならダメージ半減
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_IsHPFull(bpp) )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5f) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MultiScale( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,    handler_MultiScale },  // ダメージ計算最終チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「なかまいしき」
 */
//------------------------------------------------------------------------------
static void handler_NakamaIsiki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 defPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );

  // 自分以外の味方へのダメージ半減
  if( (defPokeID != pokeID)
  &&  BTL_MAINUTIL_IsFriendPokeID(pokeID, defPokeID)
  ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5f) );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_NakamaIsiki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,    handler_NakamaIsiki },  // ダメージ計算最終チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「モラテラピー」
 */
//------------------------------------------------------------------------------
static void handler_Moraterapi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BtlPokePos  myPos = BTL_SVFTOOL_PokeIDtoPokePos( flowWk, pokeID );
  BtlExPos    exPos = EXPOS_MAKE( BTL_EXPOS_AREA_FRIENDS, myPos );

  const BTL_POKEPARAM* bpp;
  u8 targetID[ BTL_POSIDX_MAX ];
  u8 cnt, i;

  cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, exPos, targetID );
  for(i=0; i<cnt; ++i)
  {
    if( targetID[ i ] == pokeID ){ continue; }
    bpp = BTL_SVFTOOL_GetPokeParam( flowWk, targetID[i] );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      if( Tokusei_IsExePer(flowWk, 30) )
      {
        BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );

        param->header.tokwin_flag = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = targetID[i];
        param->sickCode = WAZASICK_EX_POKEFULL;
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Moraterapi( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,    handler_Moraterapi },  // ダメージ計算最終チェックハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どくぼうそう」
 */
//------------------------------------------------------------------------------
// 威力計算ハンドラ
static void handler_Dokubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 自分が「どく」なら物理攻撃の威力が倍
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CheckSick( bpp, WAZASICK_DOKU )
    &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_PHYSIC)
    ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(2) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dokubousou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Dokubousou   }, // ワザ威力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ねつぼうそう」
 */
//------------------------------------------------------------------------------
// 威力計算ハンドラ
static void handler_Netubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 自分が「やけど」なら特殊攻撃の威力が倍
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CheckSick( bpp, WAZASICK_YAKEDO )
    &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_SPECIAL)
    ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(2) );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Netubousou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Netubousou   }, // ワザ威力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あうんのいき」
 */
//------------------------------------------------------------------------------
// 無効化チェックLv2ハンドラ
static void handler_AunNoIki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 攻撃側が味方なら無効
    u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) )
    {
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_AunNoIki );
          HANDEX_STR_AddArg( &param->str, pokeID );
        }
        BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_AunNoIki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L2,        handler_AunNoIki }, // 無効化チェックLv2ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「むらっけ」
 */
//------------------------------------------------------------------------------
// ターンチェック最終ハンドラ
static void handler_Murakke( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* sys_work )
{
  enum {
    PATTERN_MAX = ((BPP_RANKVALUE_RANGE * BPP_RANKVALUE_RANGE) - BPP_RANKVALUE_RANGE) * 2,
  };

  // 重複しない「上昇・下降」の組み合わせを列挙するための計算用ワーク
  // ※必ず、異なる２つの能力ランクが「上昇」「下降」するようにする。
  // （全能力ランクが上がりきっている（下がりきっている）場合は除く）
  typedef struct {

    u16 patterns[ PATTERN_MAX ];
    u8  upEffects[ BPP_RANKVALUE_RANGE ];
    u8  downEffects[ BPP_RANKVALUE_RANGE ];

  }CALC_WORK;

  CALC_WORK* work = BTL_SVFTOOL_GetTmpWork( flowWk, sizeof(CALC_WORK) );
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  u8 upEffect, downEffect, upCnt, downCnt, ptnCnt;
  u32 i;

//  BTL_Printf("pokeID=%d\n, bpp=%p\n", pokeID, bpp);

  // 上がりうる能力、下がりうる能力を列挙、カウントする
  upCnt = downCnt = 0;
  for(i=0; i<BPP_RANKVALUE_RANGE; ++i){
    if( BPP_IsRankEffectValid(bpp, BPP_ATTACK+i, 1) ){
      work->upEffects[ upCnt++ ] = BPP_ATTACK+i;
    }
    if( BPP_IsRankEffectValid(bpp, BPP_ATTACK+i, -1) ){
      work->downEffects[ downCnt++ ] = BPP_ATTACK+i;
    }
  }


  // 上がりうる（下がりうる）能力が０の場合、ランダムで１つだけ上がる（下がる）
  upEffect = downEffect = BPP_VALUE_NULL;
  if( (upCnt == 0) && (downCnt != 0) ){
    i = BTL_SVFTOOL_GetRand( flowWk, downCnt );
    downEffect = work->downEffects[ i ];
  }
  else if( (upCnt != 0) && (downCnt == 0) ){
    i = BTL_SVFTOOL_GetRand( flowWk, upCnt );
    upEffect = work->upEffects[ i ];
  }
  // 上がりうる能力、下がりうる能力が共に１以上の場合、考えられる組み合わせを列挙、カウントする
  else if( (upCnt != 0) && (downCnt != 0) )
  {
    u32 j;
    ptnCnt = 0;
    for(i=0; i<upCnt; ++i)
    {
      for(j=0; j<downCnt; ++j)
      {
        if( work->upEffects[i] != work->downEffects[j] )
        {
          work->patterns[ ptnCnt++ ] = (work->upEffects[i] << 8) | (work->downEffects[j]);
          if( ptnCnt >= NELEMS(work->patterns) ){
            j = downCnt;  // for loop out
            break;
          }
        }
      }
    }
    /*
    BTL_Printf("*** 組み合わせ総数=%d ***\n", ptnCnt);
    for(i=0; i<ptnCnt; ++i)
    {
      BTL_Printf("UP:%d : DOWN:%d\n", (work->patterns[i]>>8), (work->patterns[i]&0xff));
    }
    */
    // 組み合わせの中から１つをランダム決定
    i = BTL_SVFTOOL_GetRand( flowWk, ptnCnt );
    upEffect = (work->patterns[i] >> 8) & 0xff;
    downEffect = work->patterns[i] & 0xff;
//    BTL_Printf("-------\n決定 UP=%d, down=%d\n", upEffect, downEffect);
  }

  BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param;

    if( upEffect != BPP_VALUE_NULL )
    {
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->rankType = upEffect;
      param->rankVolume = 1;
    }
    if( downEffect != BPP_VALUE_NULL )
    {
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->rankType = downEffect;
      param->rankVolume = -1;
    }
  }
  BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Murakke( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,        handler_Murakke },  // ターンチェック最終ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ぼうじん」
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_Boujin_CalcDamage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE) > 0)
  ){
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_FLAG, TRUE) )
    {
//      BTL_EVENT
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Boujin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION,    handler_Boujin_CalcDamage   }, // 天候ダメージ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どくしゅ」
 */
//------------------------------------------------------------------------------
// ダメージ反応ハンドラ
static void handler_Dokusyu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 接触ワザなら
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      // ３割の確率で相手をどくにする
      if( Tokusei_IsExePer(flowWk, 30) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
        param->header.tokwin_flag = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
        param->sickID = WAZASICK_DOKU;
        param->sickCont = BTL_CALC_MakeDefaultPokeSickCont( WAZASICK_DOKU );

        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Dokusyu );
        HANDEX_STR_AddArg( &param->exStr, param->pokeID[0] );
      }
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Dokusyu( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,        handler_Dokusyu }, // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「さいせいりょく」
 */
//------------------------------------------------------------------------------
// メンバー退場確定ハンドラ
static void handler_SaiseiRyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( !BPP_IsDead(bpp) && !BPP_IsHPFull(bpp) )
    {
      BTL_HANDEX_PARAM_SHIFT_HP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SHIFT_HP, pokeID );
      u32 recoverHP = BTL_CALC_QuotMaxHP( bpp, 3 );
      u32 recoverLimit = BPP_GetValue(bpp, BPP_MAX_HP) - BPP_GetValue(bpp, BPP_HP);
      if( recoverHP > recoverLimit ){
        recoverHP = recoverLimit;
      }
      param->pokeID[0] = pokeID;
      param->volume[0] = recoverHP;
      param->poke_cnt = 1;
      param->fEffectDisable = TRUE;
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SaiseiRyoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_OUT_FIXED, handler_SaiseiRyoku },   // メンバー退場確定ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「はとむね」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hatomune( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_Hatomune_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_Hatomune_Guard },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

// ランクダウン直前処理のハンドラ
static void handler_Hatomune_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Check( flowWk, pokeID, work, WAZA_RANKEFF_DEFENCE );
}
static void handler_Hatomune_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_RankDownGuard_Fixed( flowWk, pokeID, work, BTL_STRID_SET_RankdownFail_DEF );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すなかき」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sunakaki( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Sunakaki   },    /// すばやさ計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}

// すばやさ計算ハンドラ
static void handler_Sunakaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 天候がすなあらしなら素早さ２倍
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SAND ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ミラクルスキン」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MilacreSkin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_MilacreSkin   },    /// ワザ命中率取得ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
/// ワザ命中率取得ハンドラ
static void handler_MilacreSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 非ダメージワザなら命中率50%
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_NONE )
    {
      u32 per = BTL_EVENTVAR_GetValue( BTL_EVAR_HIT_PER );
      if( per > 50 ){
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_HIT_PER, 50 );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しんうち」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sinuti( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Sinuti   },    // ワザ威力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ワザ威力計算ハンドラ
static void handler_Sinuti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    u8 myOrder, totalAction;
    // 順番最後なら威力増加
    if( BTL_SVFTOOL_GetMyActionOrder(flowWk, pokeID, &myOrder, &totalAction) ){
      if( (myOrder+1) == totalAction )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.3) );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すなのちから」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SunanoTikara( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_SunanoTikara   },    // ワザ威力計算ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ワザ威力計算ハンドラ
static void handler_SunanoTikara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 天候すなあらしなら威力増加
    if( BTL_FIELD_GetWeather() == BTL_WEATHER_SAND )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.3) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すりぬけ」
 */
//------------------------------------------------------------------------------
// 「すりぬけ」スキップチェックハンドラ
static BOOL handler_Surinuke_SkipCheck( BTL_EVENT_FACTOR* myHandle, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( factorType == BTL_EVENT_FACTOR_SIDE )
  {
    if( (subID == BTL_SIDEEFF_REFRECTOR)        // リフレクター無効
    ||  (subID == BTL_SIDEEFF_HIKARINOKABE)     // ひかりのかべ無効
    ||  (subID == BTL_SIDEEFF_SINPINOMAMORI)    // しんぴのまもり無効
    ){
      return TRUE;
    }
  }
  return FALSE;
}
// ワザ処理開始
static void handler_Surinuke_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側ならスキップチェックハンドラをアタッチする
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Surinuke_SkipCheck );
  }
}
// ワザ処理終了
static void handler_Surinuke_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側ならスキップチェックハンドラをデタッチする
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Surinuke( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZASEQ_START,      handler_Surinuke_Start },  // ワザ処理開始
    { BTL_EVENT_WAZASEQ_END,        handler_Surinuke_End   },  // ワザ処理終了
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「じしんかじょう」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_JisinKajou( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DAMAGEPROC_END, handler_JisinKajou   },    // ワザダメージプロセス最終ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ワザダメージプロセス最終ハンドラ
static void handler_JisinKajou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 死んだポケいるなら攻撃ランクアップ
    const BTL_POKEPARAM* bpp;
    u32 i, targetCnt = BTL_EVENTVAR_GetValue( BTL_EVAR_TARGET_POKECNT );
    for(i=0; i<targetCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_TARGET1+i) );
      if( BPP_IsDead(bpp) )
      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->header.tokwin_flag = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->rankType = BPP_ATTACK;
        param->rankVolume = 1;
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「せいぎのこころ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_SeiginoKokoro( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_SeiginoKokoro   },    // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ダメージ反応ハンドラ
static void handler_SeiginoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 受けたワザタイプが悪なら攻撃ランクアップ
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_AKU )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->header.tokwin_flag = TRUE;
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->rankType = BPP_ATTACK;
      param->rankVolume = 1;
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「びびり」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Bibiri( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_Bibiri   },    // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ダメージ反応ハンドラ
static void handler_Bibiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 受けたワザタイプが悪・虫・ゴーストなら素早さランクアップ
    PokeType  type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
    if( (type == POKETYPE_AKU)
    ||  (type == POKETYPE_MUSHI)
    ||  (type == POKETYPE_GHOST)
    ){
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->header.tokwin_flag = TRUE;
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->rankType = BPP_AGILITY;
      param->rankVolume = 1;
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ミイラ」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Miira( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_Miira   },    // ダメージ反応ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ダメージ反応ハンドラ
static void handler_Miira( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 接触ワザなら相手のとくせいも「ミイラ」に
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      BTL_HANDEX_PARAM_CHANGE_TOKUSEI* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TOKUSEI, pokeID );
      param->tokuseiID = POKETOKUSEI_MIIRA;
      param->pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Miira );
      HANDEX_STR_AddArg( &param->exStr, param->pokeID );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「そうしょく」
 */
//------------------------------------------------------------------------------
// ダメージワザ回復化チェックハンドラ
static void handler_Sousyoku_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_KUSA );
}
// ダメージワザ回復化決定ハンドラ
static void handler_Sousyoku_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Fix( flowWk, pokeID, work, 4 );
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

    param->poke_cnt = 1;
    param->pokeID[0] = pokeID;
    param->rankType = BPP_SP_ATTACK;
    param->rankVolume = 1;
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Sousyoku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Sousyoku_Check }, // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_Sousyoku_Fix   },
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「いたずらごころ」
 */
//------------------------------------------------------------------------------
// ワザプライオリティ取得ハンドラ
static void handler_ItazuraGokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // ダメージワザ以外ならプライオリティアップ
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_NONE )
    {
      u8 pri = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_PRI );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_WAZA_PRI, pri+1 );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_ItazuraGokoro( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_GET_WAZA_PRI,   handler_ItazuraGokoro }, // ワザプライオリティ取得ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マジックミラー」
 */
//------------------------------------------------------------------------------
// わざ乗っ取りハンドラ
static void handler_MagicMirror_CheckRob( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
  u8 fEnable = FALSE;

  work[0] = 0;

  if( atkPokeID != pokeID )
  {
    u8 i, targetCnt = BTL_EVENTVAR_GetValue( BTL_EVAR_TARGET_POKECNT );
    for(i=0; i<targetCnt; ++i)
    {
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_TARGET1+i) == pokeID ){
        fEnable = TRUE;
        break;
      }
    }
  }

  if( fEnable )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_MagicCoat) )
    {
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_POKEID, pokeID) ){
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_POKEID_DEF, atkPokeID );
        work[0] = 1;
      }
    }
  }
}
//
static void handler_MagicMirror_Reflect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (work[0] == 1)
  ){
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MajicMirror );
      HANDEX_STR_AddArg( &param->str, BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) );
      HANDEX_STR_AddArg( &param->str, BTL_EVENTVAR_GetValue(BTL_EVAR_WAZAID) );
    }
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    work[0] = 0;
  }
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_MagicMirror( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_WAZA_ROB,     handler_MagicMirror_CheckRob },  // わざ乗っ取り判定ハンドラ
    { BTL_EVENT_WAZASEQ_REFRECT,    handler_MagicMirror_Reflect  },  // わざ跳ね返し確定
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しゅうかく」
 */
//------------------------------------------------------------------------------
// ターンチェック開始ハンドラ
static void handler_Syuukaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_TURNFLAG_Get(bpp, BPP_TURNFLG_ITEM_REMOVED) )
  {
    u16 usedItem = BPP_GetUsedItem( bpp );
    if( ITEM_CheckNuts(usedItem) )
    {
      BTL_HANDEX_PARAM_SET_ITEM* param;

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SET_ITEM, pokeID );
      param->itemID = usedItem;
      param->pokeID = pokeID;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Syuukaku );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
      HANDEX_STR_AddArg( &param->exStr, usedItem );

      BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Syuukaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,        handler_Syuukaku },  // ターンチェック開始ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あまのじゃく」
 */
//------------------------------------------------------------------------------
// ランク増減値修正ハンドラ
static void handler_Amanojaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    int volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    if( (volume > 0)
    ||  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) != pokeID)
    ){
      volume *= -1;
      work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, volume );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Amanojaku( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKVALUE_CHANGE,        handler_Amanojaku },  // ランク増減値修正ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きんちょうかん」
 */
//------------------------------------------------------------------------------
// メンバー入場
static void handler_Kinchoukan_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_MESSAGE* param;
    BtlSide side = BTL_MAINUTIL_GetOpponentSide( BTL_MAINUTIL_PokeIDtoSide(pokeID) );

    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
    HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_Kinchoukan );
    HANDEX_STR_AddArg( &param->str, side );
    BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// 装備アイテム使用チェックハンドラ
static void handler_Kinchoukan_CheckItemEquip( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // work[0] が初期状態なら相手側の装備アイテム使用を失敗させる
  if( work[0] == 0 )
  {
    u8 userPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );
    if( userPokeID != pokeID )
    {
      u16 itemID = BTL_EVENTVAR_GetValue( BTL_EVAR_ITEM );
      if( ITEM_CheckNuts(itemID) )
      {
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
      }
    }
  }
}
// メンバー退場確定ハンドラ
static void handler_Kinchoukan_MemberOutFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    common_KinchoukanOff( flowWk, pokeID, work );
  }
}
// いえき確定ハンドラ
static void handler_Kinchoukan_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    common_KinchoukanOff( flowWk, pokeID, work );
  }
}
// とくせい変更確定ハンドラ
static void handler_Kinchoukan_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI) != BTL_EVENT_FACTOR_GetSubID(myHandle))
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  ){
    common_KinchoukanOff( flowWk, pokeID, work );
  }
}
// きんちょうかん無効化共通
static void common_KinchoukanOff( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  const BTL_POKEPARAM* bpp;
  u8 targetPokeID[ BTL_POSIDX_MAX ];
  u8 targetCnt, i;

  BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokeID( flowWk, pokeID );
  BtlExPos   exPos = EXPOS_MAKE( BTL_EXPOS_FULL_ENEMY, myPos );

  // work[0] に値を入れて装備アイテム使用チェック機能をオフ
  work[0] = 1;

  targetCnt = BTL_SVFTOOL_ExpandPokeID( flowWk, exPos, targetPokeID );
  for(i=0; i<targetCnt; ++i)
  {
    bpp = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID[i] );
    if( !BPP_IsDead(bpp) )
    {
      BTL_HANDEX_PARAM_EQUIP_ITEM* param;
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_EQUIP_ITEM, pokeID );
      param->pokeID = targetPokeID[ i ];
    }
  }
}

static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Kinchoukan( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_Kinchoukan_MemberIn       }, // メンバー入場ハンドラ
    { BTL_EVENT_CHECK_ITEMEQUIP_FAIL,  handler_Kinchoukan_CheckItemEquip }, // 装備アイテム使用チェックハンドラ
    { BTL_EVENT_MEMBER_OUT_FIXED,      handler_Kinchoukan_MemberOutFixed }, // メンバー退場確定ハンドラ
    { BTL_EVENT_IEKI_FIXED,            handler_Kinchoukan_Ieki           }, // いえき確定ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Kinchoukan_ChangeTok      }, // とくせい変換直前

    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「へんしん」
 */
//------------------------------------------------------------------------------
// 入場ハンドラ
static void handler_Hensin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BtlPokePos myPos = BTL_SVFTOOL_PokeIDtoPokePos( flowWk, pokeID );
    BtlRule rule = BTL_SVFTOOL_GetRule( flowWk );
    BtlPokePos tgtPos = BTL_MAINUTIL_GetFacedPokePos( rule, myPos );

    u8 tgtPokeID = BTL_SVFTOOL_PokePosToPokeID( flowWk, tgtPos );
    const BTL_POKEPARAM* target = BTL_SVFTOOL_GetPokeParam( flowWk, tgtPokeID );
    if( !BPP_IsDead(target) )
    {
      BTL_HANDEX_PARAM_HENSIN* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_HENSIN, pokeID );
      param->header.tokwin_flag = TRUE;
      param->pokeID = tgtPokeID;

      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Hensin );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
      HANDEX_STR_AddArg( &param->exStr, param->pokeID );
    }
  }
}
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Hensin( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,        handler_Hensin },  // 入場ハンドラ
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
//------------------------------------------------------------------------------
/**
 *  とくせい「イリュージョン」
 */
//------------------------------------------------------------------------------
static BTL_EVENT_FACTOR*  HAND_TOK_ADD_Illusion( u16 pri, u16 tokID, u8 pokeID )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Illusion_Damage    },   // ダメージ反応ハンドラ
    { BTL_EVENT_IEKI_FIXED,            handler_Illusion_Ieki      },   // いえき確定ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Illusion_ChangeTok },   // とくせい変換直前
    { BTL_EVENT_NULL, NULL },
  };
  return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokID, pri, pokeID, HandlerTable );
}
// ダメージ反応ハンドラ
static void handler_Illusion_Damage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側ならイリュージョン解除
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    common_IllusionBreak( myHandle, flowWk, pokeID );
  }
}
// いえき確定ハンドラ
static void handler_Illusion_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    common_IllusionBreak( myHandle, flowWk, pokeID );
  }
}
// とくせい変換直前ハンドラ
static void handler_Illusion_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI) != BTL_EVENT_FACTOR_GetSubID(myHandle))
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  ){
    common_IllusionBreak( myHandle, flowWk, pokeID );
  }
}

static void common_IllusionBreak( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  // イリュージョン継続中なら正体を明かす
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_IsFakeEnable(bpp) )
  {
    BTL_HANDEX_PARAM_FAKE_BREAK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_FAKE_BREAK, pokeID );
    param->pokeID = pokeID;
    HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Illusion_Break );
    HANDEX_STR_AddArg( &param->exStr, pokeID );
  }
}

