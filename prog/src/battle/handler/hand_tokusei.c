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

#include "hand_common.h"
#include "hand_tokusei.h"




/*--------------------------------------------------------------------------*/
/* Prototypes                                                               */
/*--------------------------------------------------------------------------*/
static u32 numHandlersWithHandlerPri( u16 pri, u16 numHandlers );
static u16 devideNumHandersAndPri( u32* numHandlers );
static u16 CalcTokHandlerSubPriority( const BTL_POKEPARAM* bpp );
static BOOL Tokusei_IsExePer( BTL_SVFLOW_WORK* flowWk, u8 per );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Ikaku( u32* numElems );
static void handler_Ikaku_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Seisinryoku( u32* numElems );
static void handler_Seisinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fukutsuno( u32* numElems );
static void handler_FukutsunoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AtuiSibou( u32* numElems );
static void handler_AtuiSibou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikaramoti( u32* numElems );
static void handler_Tikaramoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Suisui( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Suisui( u32* numElems );
static void handler_Youryokuso( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Youryokuso( u32* numElems );
static void handler_Hayaasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hayaasi( u32* numElems );
static void handler_Tidoriasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tidoriasi( u32* numElems );
static void handler_Harikiri_HitRatio( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Harikiri_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Harikiri( u32* numElems );
static void handler_Atodasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Atodasi( u32* numElems );
static void handler_SlowStart_Agility( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SlowStart_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SlowStart_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SlowStart_Get( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SlowStart_TurnCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SlowStart( u32* numElems );
static void handler_Fukugan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fukugan( u32* numElems );
static void handler_Sunagakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sunagakure_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunagakure( u32* numElems );
static void handler_Yukigakure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Yukigakure_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yukigakure( u32* numElems );
static void common_weather_guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, BtlWeather weather );
static void handler_Iromegane( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Iromegane( u32* numElems );
static void handler_HardRock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HardRock( u32* numElems );
static void handler_Sniper( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sniper( u32* numElems );
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kasoku( u32* numElems );
static void handler_Tekiouryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tekiouryoku( u32* numElems );
static void handler_Mouka( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Mouka( u32* numElems );
static void handler_Gekiryu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Gekiryu( u32* numElems );
static void handler_Sinryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sinryoku( u32* numElems );
static void handler_MusinoSirase( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MusinoSirase( u32* numElems );
static void common_hpborder_powerup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
static void handler_Konjou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Konjou( u32* numElems );
static void handler_PlusMinus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Plus( u32* numElems );
static BOOL checkExistTokuseiFriend( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeTokusei tokuseiID );
static BOOL checkFlowerGiftEnablePokemon( BTL_SVFLOW_WORK* flowWk, u8 pokeID );
static void common_FlowerGift_FormChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, u8 nextForm, u8 fTokWin );
static void handler_FlowerGift_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_TokOff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_AirLock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_TokChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_Power( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_FlowerGift_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FlowerGift( u32* numElems );
static void handler_Tousousin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tousousin( u32* numElems );
static void handler_Technician( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Technician( u32* numElems );
static void handler_TetunoKobusi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_TetunoKobusi( u32* numElems );
static void handler_Sutemi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sutemi( u32* numElems );
static void handler_FusiginaUroko( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FusiginaUroko( u32* numElems );
static void handler_SkillLink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SkillLink( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KairikiBasami( u32* numElems );
static void handler_KairikiBasami_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_KairikiBasami_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Surudoime( u32* numElems );
static void handler_Surudoime_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Surudoime_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ClearBody( u32* numElems );
static void handler_ClearBody_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_ClearBody_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_RankDownGuard_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, WazaRankEffect rankType );
static void common_RankDownGuard_Fixed( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, u16 strID );
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tanjun( u32* numElems );
static void handler_ReafGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ReafGuard( u32* numElems );
static void handler_Juunan_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Juunan_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Juunan_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Juunan( u32* numElems );
static void handler_Fumin_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Fumin_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Fumin_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fumin( u32* numElems );
static void handler_MagumaNoYoroi_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagumaNoYoroi_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagumaNoYoroi_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagumaNoYoroi( u32* numElems );
static void handler_Meneki_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Meneki_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Meneki_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Meneki( u32* numElems );
static void handler_MizuNoBale_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MizuNoBale_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MizuNoBale_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MizuNoBale( u32* numElems );
static void handler_MyPace_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MyPace_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MyPace_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MyPace( u32* numElems );
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Donkan_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Donkan_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Donkan_NoEffCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Donkan( u32* numElems );
static BOOL common_GuardWazaSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick guardSick );
static void handler_AddSickFailCommon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_TokuseiWake_CureSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick );
static void common_TokuseiWake_CureSickCore( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick );
static void handler_Amefurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Amefurasi( u32* numElems );
static void handler_Hideri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hideri( u32* numElems );
static void handler_Sunaokosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunaokosi( u32* numElems );
static void handler_Yukifurasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yukifurasi( u32* numElems );
static void common_weather_change( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_AirLock_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AirLock( u32* numElems );
static void handler_IceBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IcoBody( u32* numElems );
static void handler_AmeukeZara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AmeukeZara( u32* numElems );
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static void handler_SunPower_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SunPower_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SunPower( u32* numElems );
static void handler_Rinpun_Sick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Rinpun_Rank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Rinpun_Shrink( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Rinpun_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Rinpun( u32* numElems );
static void handler_TennoMegumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_TennoMegumi( u32* numElems );
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_UruoiBody( u32* numElems );
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dappi( u32* numElems );
static void handler_PoisonHeal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_PoisonHeal( u32* numElems );
static void handler_KabutoArmor( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KabutoArmor( u32* numElems );
static void handler_Kyouun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kyouun( u32* numElems );
static void handler_IkarinoTubo( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IkarinoTubo( u32* numElems );
static void handler_DokunoToge( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DokunoToge( u32* numElems );
static void handler_Seidenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Seidenki( u32* numElems );
static void handler_HonoNoKarada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HonoNoKarada( u32* numElems );
static void handler_MeromeroBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MeromeroBody( u32* numElems );
static void handler_Housi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Housi( u32* numElems );
static void common_touchAddSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick, BPP_SICK_CONT sickCont, u8 per );
static void handler_Samehada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Samehada( u32* numElems );
static void handler_Yuubaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yuubaku( u32* numElems );
static void handler_Hensyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hensyoku( u32* numElems );
static void handler_Syncro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Syncro( u32* numElems );
static void handler_Isiatama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Isiatama( u32* numElems );
static void handler_NormalSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NormalSkin( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Trace( u32* numElems );
static void handler_Trace( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_SizenKaifuku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SizenKaifuku( u32* numElems );
static void handler_Download( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DownLoad( u32* numElems );
static void handler_Yotimu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yotimu( u32* numElems );
static void handler_KikenYoti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL check_kikenyoti_poke( const BTL_POKEPARAM* bppUser, const BTL_POKEPARAM* bppEnemy );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KikenYoti( u32* numElems );
static void handler_Omitoosi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Omitoosi( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Ganjou( u32* numElems );
static void handler_Ganjou_Ichigeki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Ganjou_KoraeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Ganjou_KoraeExe( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tennen_hitRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tennen_AtkRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tennen_DefRank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tennen( u32* numElems );
static void handler_Tainetsu_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tainetsu_SickDmg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tainetu( u32* numElems );
static BOOL common_DmgToRecover_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType matchType );
static BOOL common_TypeNoEffect( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType );
static void common_TypeRecoverHP( BTL_SVFLOW_WORK* flowWk, u8 pokeID, u8 denomHP );
static void common_TypeNoEffect_Rankup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaRankEffect rankType, u8 volume );
static void handler_Kansouhada_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kansouhada_DmgRecover( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kansouhada_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kansouhada_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kansouhada( u32* numElems );
static void handler_Tyosui_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tyosui( u32* numElems );
static void handler_Tikuden_CheckEx( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikuden( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DenkiEngine( u32* numElems );
static void handler_DenkiEngine_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_DenkiEngine_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_DenkiEngine_CheckEx( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_NoGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NoGuard( u32* numElems );
static void handler_Kimottama( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kimottama( u32* numElems );
static void handler_Bouon( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bouon( u32* numElems );
static void handler_Fuyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Fuyuu_Disp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Fuyuu_TurnCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fuyuu( u32* numElems );
static void handler_FusiginaMamori( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FusiginaMamori( u32* numElems );
static void handler_Namake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Namake_Get( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Nameke_Failed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Namake( u32* numElems );
static void handler_Simerike( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Simerike_Effective( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Simerike_StartSeq( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Simerike_EndSeq( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Simerike_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL handler_Simerike_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Simerike( u32* numElems );
static void handler_Moraibi_NoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Moraibi_DmgRecoverFix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Moraibi_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Moraibi_Remove( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Moraibi_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Moraibi( u32* numElems );
static void handler_Nightmare( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nightmare( u32* numElems );
static void handler_Nigeasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Nigeasi_Msg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nigeasi( u32* numElems );
static void handler_Katayaburi_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Katayaburi_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Katayaburi_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Katayaburi_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Katayaburi( u32* numElems );
static void handler_Tenkiya_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tenkiya_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tenkiya_AirLock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tenkiya_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tenkiya_TokOff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_Tenkiya_Off( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID );
static void common_TenkiFormChange( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tenkiya( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yobimizu( u32* numElems );
static void handler_Yobimizu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Yobimizu_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hiraisin( u32* numElems );
static void handler_Hiraisin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hiraisin_WazaExeStart( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hiraisin_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL common_WazaTargetChangeToMe( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType wazaType );
static void handler_Kyuuban( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kyuuban( u32* numElems );
static void handler_HedoroEki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_HedoroEki_Dead( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HedoroEki( u32* numElems );
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Bukiyou_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Bukiyou_PreChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Bukiyou_IekiFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Bukiyou_ExeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Bukiyou_ExeFail( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bukiyou( u32* numElems );
static void handler_Nenchaku_NoEff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Nenchaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nenchaku( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Pressure( u32* numElems );
static void handler_Pressure_MemberIN( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Pressure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagicGuard( u32* numElems );
static void handler_MagicGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Akusyuu( u32* numElems );
static void handler_Akusyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static const BtlEventHandlerTable*  HAND_TOK_ADD_Kagefumi( u32* numElems );
static void handler_Kagefumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static const BtlEventHandlerTable*  HAND_TOK_ADD_Arijigoku( u32* numElems );
static void handler_Arijigoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static const BtlEventHandlerTable*  HAND_TOK_ADD_Jiryoku( u32* numElems );
static void handler_Jiryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Karuwaza( u32* numElems );
static void handler_Karuwaza_BeforeItemSet( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Karuwaza_Agility( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Monohiroi( u32* numElems );
static void handler_Monohiroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_WaruiTeguse( u32* numElems );
static void handler_WaruiTeguse( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_NorowareBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NorowareBody( u32* numElems );
static void handler_KudakeruYoroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KudakeruYoroi( u32* numElems );
static void handler_Tikarazuku_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tikarazuku_CheckFail( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Tikarazuku_ShrinkCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikarazuku( u32* numElems );
static BOOL IsTikarazukuEffecive( WazaID waza );
static void handler_Makenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Makenki( u32* numElems );
static void handler_Yowaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yowaki( u32* numElems );
static void handler_MultiScale( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MultiScale( u32* numElems );
static void handler_NakamaIsiki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FriendGuard( u32* numElems );
static void handler_Moraterapi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IyasiNoKokoro( u32* numElems );
static void handler_Dokubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dokubousou( u32* numElems );
static void handler_Netubousou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Netubousou( u32* numElems );
static void handler_AunNoIki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Telepassy( u32* numElems );
static void handler_Murakke( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* sys_work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Murakke( u32* numElems );
static void handler_Boujin_CalcDamage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Boujin( u32* numElems );
static void handler_Dokusyu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dokusyu( u32* numElems );
static void handler_SaiseiRyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SaiseiRyoku( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hatomune( u32* numElems );
static void handler_Hatomune_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Hatomune_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunakaki( u32* numElems );
static void handler_Sunakaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MilacreSkin( u32* numElems );
static void handler_MilacreSkin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Analyze( u32* numElems );
static void handler_Sinuti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SunanoTikara( u32* numElems );
static void handler_SunanoTikara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_DarumaMode( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DarumaMode( u32* numElems );
static BOOL handler_Surinuke_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Surinuke_Start( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Surinuke_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Surinuke( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_JisinKajou( u32* numElems );
static void handler_JisinKajou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SeiginoKokoro( u32* numElems );
static void handler_SeiginoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bibiri( u32* numElems );
static void handler_Bibiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Miira( u32* numElems );
static void handler_Miira( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sousyoku_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sousyoku_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Sousyoku_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sousyoku( u32* numElems );
static void handler_ItazuraGokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ItazuraGokoro( u32* numElems );
static void handler_MagicMirror_ExeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagicMirror_Wait( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_MagicMirror_Reflect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagicMirror( u32* numElems );
static void handler_Syuukaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Syuukaku( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HeavyMetal( u32* numElems );
static void handler_HeavyMetal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_LightMetal( u32* numElems );
static void handler_LightMetal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Amanojaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Amanojaku( u32* numElems );
static void handler_Kinchoukan_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static BOOL handler_Kinchoukan_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID );
static void handler_Kinchoukan_CheckItemEquip( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_MemberOutFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Kinchoukan_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_KinchoukanOff( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kinchoukan( u32* numElems );
static void handler_Hensin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kawarimono( u32* numElems );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Illusion( u32* numElems );
static void handler_Illusion_Damage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Illusion_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void handler_Illusion_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );
static void common_IllusionBreak( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID );
static  const BtlEventHandlerTable*  HAND_TOK_ADD_GoodLuck( u32* numElems );
static void handler_GoodLuck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work );


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
  typedef const BtlEventHandlerTable* (*pTokuseiEventAddFunc)( u32* );

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
    { POKETOKUSEI_MAINASU,          HAND_TOK_ADD_Plus          }, // 今作よりプラスとマイナスは等価
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
//    { POKETOKUSEI_NOOGAADO,         HAND_TOK_ADD_NoGuard       }, svflow側で対処
    { POKETOKUSEI_KIMOTTAMA,        HAND_TOK_ADD_Kimottama     },
    { POKETOKUSEI_BOUON,            HAND_TOK_ADD_Bouon         },
    { POKETOKUSEI_FUYUU,            HAND_TOK_ADD_Fuyuu         },
    { POKETOKUSEI_FURAWAAGIFUTO,    HAND_TOK_ADD_FlowerGift    },
    { POKETOKUSEI_MORAIBI,          HAND_TOK_ADD_Moraibi       },
    { POKETOKUSEI_YOTIMU,           HAND_TOK_ADD_Yotimu        },
    { POKETOKUSEI_KIKENYOTI,        HAND_TOK_ADD_KikenYoti     },
    { POKETOKUSEI_OMITOOSI,         HAND_TOK_ADD_Omitoosi      },
    { POKETOKUSEI_YUUBAKU,          HAND_TOK_ADD_Yuubaku       },
    { POKETOKUSEI_NIGEASI,          HAND_TOK_ADD_Nigeasi       },
    { POKETOKUSEI_HENSYOKU,         HAND_TOK_ADD_Hensyoku      },
    { POKETOKUSEI_KATAYABURI,       HAND_TOK_ADD_Katayaburi    },
    { POKETOKUSEI_NAMAKE,           HAND_TOK_ADD_Namake        },
    { POKETOKUSEI_HIRAISIN,         HAND_TOK_ADD_Hiraisin      },
    { POKETOKUSEI_YOBIMIZU,         HAND_TOK_ADD_Yobimizu      },
    { POKETOKUSEI_SUROOSUTAATO,     HAND_TOK_ADD_SlowStart     },
    { POKETOKUSEI_SIMERIKE,         HAND_TOK_ADD_Simerike      },
    { POKETOKUSEI_FUSIGINAMAMORI,   HAND_TOK_ADD_FusiginaMamori},
    { POKETOKUSEI_ATODASI,          HAND_TOK_ADD_Atodasi       },
    { POKETOKUSEI_TENKIYA,          HAND_TOK_ADD_Tenkiya       },
    { POKETOKUSEI_KYUUBAN,          HAND_TOK_ADD_Kyuuban       },
    { POKETOKUSEI_HEDOROEKI,        HAND_TOK_ADD_HedoroEki     },
    { POKETOKUSEI_BUKIYOU,          HAND_TOK_ADD_Bukiyou       },
    { POKETOKUSEI_NENCHAKU,         HAND_TOK_ADD_Nenchaku      },
    { POKETOKUSEI_PURESSYAA,        HAND_TOK_ADD_Pressure      },
    { POKETOKUSEI_MAJIKKUGAADO,     HAND_TOK_ADD_MagicGuard    },
    { POKETOKUSEI_NAITOMEA,         HAND_TOK_ADD_Nightmare     },
    { POKETOKUSEI_MONOHIROI,        HAND_TOK_ADD_Monohiroi     },
    { POKETOKUSEI_KARUWAZA,         HAND_TOK_ADD_Karuwaza      },
    { POKETOKUSEI_AKUSYUU,          HAND_TOK_ADD_Akusyuu       },
    { POKETOKUSEI_KAGEFUMI,         HAND_TOK_ADD_Kagefumi      },
    { POKETOKUSEI_ARIJIGOKU,        HAND_TOK_ADD_Arijigoku     },
    { POKETOKUSEI_JIRYOKU,          HAND_TOK_ADD_Jiryoku       },

    { POKETOKUSEI_WARUITEGUSE,      HAND_TOK_ADD_WaruiTeguse   },
    { POKETOKUSEI_TIKARAZUKU,       HAND_TOK_ADD_Tikarazuku    },
    { POKETOKUSEI_MAKENKI,          HAND_TOK_ADD_Makenki       },
    { POKETOKUSEI_YOWAKI,           HAND_TOK_ADD_Yowaki        },
    { POKETOKUSEI_MARUTISUKEIRU,    HAND_TOK_ADD_MultiScale    },
    { POKETOKUSEI_HEVIMETARU,       HAND_TOK_ADD_HeavyMetal    }, // ヘヴィメタル
    { POKETOKUSEI_RAITOMETARU,      HAND_TOK_ADD_LightMetal    }, // ライトメタル
    { POKETOKUSEI_AMANOJAKU,        HAND_TOK_ADD_Amanojaku     }, // あまのじゃく
    { POKETOKUSEI_KINCHOUKAN,       HAND_TOK_ADD_Kinchoukan    }, // きんちょうかん
    { POKETOKUSEI_NOROWAREBODY,     HAND_TOK_ADD_NorowareBody  }, // のろわれボディ
    { POKETOKUSEI_IYASINOKOKORO,    HAND_TOK_ADD_IyasiNoKokoro }, // いやしのこころ
    { POKETOKUSEI_FURENDOGAADO,     HAND_TOK_ADD_FriendGuard   }, // フレンドガード
    { POKETOKUSEI_KUDAKERUYOROI,    HAND_TOK_ADD_KudakeruYoroi }, // くだけるよろい
    { POKETOKUSEI_DOKUBOUSOU,       HAND_TOK_ADD_Dokubousou    }, // どくぼうそう
    { POKETOKUSEI_NETUBOUSOU,       HAND_TOK_ADD_Netubousou    }, // ねつぼうそう
    { POKETOKUSEI_SYUUKAKU,         HAND_TOK_ADD_Syuukaku      }, // しゅうかく
    { POKETOKUSEI_TEREPASII,        HAND_TOK_ADD_Telepassy     }, // テレパシー（旧あうんのいき）
    { POKETOKUSEI_MURAKKE,          HAND_TOK_ADD_Murakke       }, // ムラっけ
    { POKETOKUSEI_BOUJIN,           HAND_TOK_ADD_Boujin        }, // ぼうじん
    { POKETOKUSEI_DOKUSYU,          HAND_TOK_ADD_Dokusyu       }, // どくしゅ
    { POKETOKUSEI_SAISEIRYOKU,      HAND_TOK_ADD_SaiseiRyoku   }, // さいせいりょく
    { POKETOKUSEI_HATOMUNE,         HAND_TOK_ADD_Hatomune      }, // はとむね
    { POKETOKUSEI_SUNAKAKI,         HAND_TOK_ADD_Sunakaki      }, // すなかき
    { POKETOKUSEI_MIRAKURUSUKIN,    HAND_TOK_ADD_MilacreSkin   }, // ミラクルスキン
    { POKETOKUSEI_ANARAIZU,         HAND_TOK_ADD_Analyze       }, // アナライズ
    { POKETOKUSEI_IRYUUJON,         HAND_TOK_ADD_Illusion      }, // イリュージョン
    { POKETOKUSEI_KAWARIMONO,       HAND_TOK_ADD_Kawarimono    }, // かわりもの
    { POKETOKUSEI_SURINUKE,         HAND_TOK_ADD_Surinuke      }, // すりぬけ
    { POKETOKUSEI_MIIRA,            HAND_TOK_ADD_Miira         }, // ミイラ
    { POKETOKUSEI_JISINKAJOU,       HAND_TOK_ADD_JisinKajou    }, // じしんかじょう
    { POKETOKUSEI_SEIGINOKOKORO,    HAND_TOK_ADD_SeiginoKokoro }, // せいぎのこころ
    { POKETOKUSEI_BIBIRI,           HAND_TOK_ADD_Bibiri        }, // びびり
    { POKETOKUSEI_MAJIKKUMIRAA,     HAND_TOK_ADD_MagicMirror   }, // マジックミラー
    { POKETOKUSEI_SOUSYOKU,         HAND_TOK_ADD_Sousyoku      }, // そうしょく
    { POKETOKUSEI_ITAZURAGOKORO,    HAND_TOK_ADD_ItazuraGokoro }, // いたずらごころ
    { POKETOKUSEI_SUNANOTIKARA,     HAND_TOK_ADD_SunanoTikara  }, // すなのちから
    { POKETOKUSEI_TETUNOTOGE,       HAND_TOK_ADD_Samehada      }, // てつのとげ = さめはだ
    { POKETOKUSEI_DARUMAMOODO,      HAND_TOK_ADD_DarumaMode    }, // ダルマモード
    { POKETOKUSEI_SYOURINOHOSI,     HAND_TOK_ADD_GoodLuck      }, // しょうりのほし
    { POKETOKUSEI_TAABOBUREIZU,     HAND_TOK_ADD_Katayaburi    }, // ターボブレイズ
    { POKETOKUSEI_TERABORUTEEZI,    HAND_TOK_ADD_Katayaburi    }, // テラボルテージ

  };

//  if( !BPP_CheckSick(pp, WAZASICK_IEKI) )
  {
    u16 tokusei = BPP_GetValue( pp, BPP_TOKUSEI );
    int i;

    for(i=0; i<NELEMS(funcTbl); i++)
    {
      if( funcTbl[i].tokusei == tokusei )
      {
        u16 agi = CalcTokHandlerSubPriority( pp );
        u8 pokeID = BPP_GetID( pp );
        u32 numHandlers;
        u16 mainPri;
        const BtlEventHandlerTable* handlerTable;

        handlerTable = funcTbl[i].func( &numHandlers );
        mainPri = devideNumHandersAndPri( &numHandlers );

        TAYA_Printf("MainPri=%d, numHandlers=%d\n", mainPri, numHandlers );

        return BTL_EVENT_AddFactor( BTL_EVENT_FACTOR_TOKUSEI, tokusei,
                  mainPri, agi, pokeID, handlerTable, numHandlers );
      }
    }
    BTL_Printf("とくせい(%d)はハンドラが用意されていない\n", tokusei);
  }
  return NULL;
}

//----------------------------------------------------------------------------------
/**
 * ハンドラ数にハンドラプライオリティの値を合わせてu32変換
 */
//----------------------------------------------------------------------------------
static u32 numHandlersWithHandlerPri( u16 pri, u16 numHandlers )
{
  u32 result = (pri << 16) | (numHandlers);
  return result;
}
//----------------------------------------------------------------------------------
/**
 * ハンドラ登録数＆ハンドラプライオリティの合成値を元の値にバラす
 */
//----------------------------------------------------------------------------------
static u16 devideNumHandersAndPri( u32* numHandlers )
{
  u16 mainPri = ((*numHandlers) >> 16) & 0xffff;
  if( mainPri == 0 ){
    mainPri = BTL_EVPRI_TOKUSEI_DEFAULT;
  }
  (*numHandlers) &= 0xffff;
  return mainPri;
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
    u16 tokuseiID = BTL_EVENT_FACTOR_GetSubID( factor );
    BTL_EVENT_FACTOR_Remove( factor );
  }
}


void BTL_HANDLER_TOKUSEI_Pause( const BTL_POKEPARAM* pp )
{

}

void BTL_HANDLER_TOKUSEI_Swap( const BTL_POKEPARAM* pp1, const BTL_POKEPARAM* pp2 )
{
  BTL_EVENT_FACTOR *factor1, *factor2;
  u8  ID_1, ID_2;

  ID_1 = BPP_GetID( pp1 );
  ID_2 = BPP_GetID( pp2 );
  factor1 = BTL_EVENT_SeekFactor( BTL_EVENT_FACTOR_TOKUSEI, ID_1 );
  factor2 = BTL_EVENT_SeekFactor( BTL_EVENT_FACTOR_TOKUSEI, ID_2 );

  if( factor1 )
  {
    BTL_EVENT_FACTOR_Remove( factor1 );
  }
  if( factor2 )
  {
    BTL_EVENT_FACTOR_Remove( factor2 );
  }

  BTL_HANDLER_TOKUSEI_Add( pp1 );
  BTL_HANDLER_TOKUSEI_Add( pp2 );

}

static u16 CalcTokHandlerSubPriority( const BTL_POKEPARAM* bpp )
{
  return BPP_GetValue_Base( bpp, BPP_AGILITY );
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Ikaku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Ikaku_MemberIn },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Ikaku_MemberIn },
  };

  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ポケモンが出場した時のハンドラ
static void handler_Ikaku_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->rankType = BPP_ATTACK_RANK;
        param->rankVolume = -1;
        param->fAlmost = TRUE;
        {
          BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
          BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );

          param->poke_cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, param->pokeID );
        }
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}

//------------------------------------------------------------------------------
/**
 *  とくせい「せいしんりょく」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Seisinryoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SHRINK_CHECK, handler_Seisinryoku },    // ひるみ発生チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ひるみ発生チェックハンドラ
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fukutsuno( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_FAIL, handler_FukutsunoKokoro },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ワザ出し失敗確定
static void handler_FukutsunoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // ワザ失敗理由=ひるみ, 対象ポケ=自分で発動
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_FAIL_CAUSE) == SV_WAZAFAIL_SHRINK)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  ){
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->header.tokwin_flag = TRUE;
      param->rankType = BPP_AGILITY_RANK;
      param->rankVolume = 1;
      param->fAlmost = FALSE;
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「あついしぼう」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AtuiSibou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_AtuiSibou },
  };

  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 攻撃側の攻撃値決定
static void handler_AtuiSibou( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // ダメージ対象が自分
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  {
    PokeType  type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikaramoti( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Tikaramoti },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_RAIN )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Suisui( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Suisui },   // すばやさ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Youryokuso( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Youryokuso },   // すばやさ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_HAYAASI_AGIRATIO );
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, FALSE );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hayaasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY, handler_Hayaasi },    // すばやさ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tidoriasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Tidoriasi },    // 命中率計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Harikiri( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Harikiri_HitRatio },  // 命中率計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER, handler_Harikiri_AtkPower },  // 攻撃力計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_SP_PRIORITY, BTL_SPPRI_LOW );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Atodasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_SP_PRIORITY,   handler_Atodasi },  // 特殊優先度チェックハンドラ
  };
  *numElems = numHandlersWithHandlerPri( BTL_EVPRI_TOKUSEI_atodasi, NELEMS(HandlerTable) );
  return HandlerTable;
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
    if( work[0] )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SLOWSTART_AGIRATIO );
    }
  }
}
// 攻撃力計算ハンドラ
static void handler_SlowStart_AtkPower( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    if( work[0] )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetDamageType(waza) == WAZADATA_DMG_PHYSIC )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SLOWSTART_ATKRATIO );
      }
    }
  }
}
// メンバー入場ハンドラ（＆とくせい書き換え直後ハンドラ）
static void handler_SlowStart_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    work[0] = TRUE;   // work[0] : スロースタート効果中フラグ
    work[1] = 0;      // work[1] : ターンカウンタ
    {
      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        param->header.tokwin_flag = TRUE;
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_SlowStartON );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
static void handler_SlowStart_Get( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[4] == 0 )
  {
    handler_SlowStart_MemberIn( myHandle, flowWk, pokeID, work );
    work[4] = 1;
  }
}

// ターンチェック終了ハンドラ
static void handler_SlowStart_TurnCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( work[0] ) // work[0] : スロースタート効果中フラグ
    {
      if( work[1] < BTL_CALC_TOK_SLOWSTART_ENABLE_TURN ){
        ++(work[1]);
      }
      if( work[1] >= BTL_CALC_TOK_SLOWSTART_ENABLE_TURN )
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_SlowStartOFF );
          HANDEX_STR_AddArg( &param->str, pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );

        work[0] = FALSE;
      }
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_SlowStart( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_SlowStart_MemberIn  },  // メンバー入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_SlowStart_MemberIn  },  // とくせい書き換え直後ハンドラ
    { BTL_EVENT_CALC_AGILITY,         handler_SlowStart_Agility   },  // すばやさ計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER,       handler_SlowStart_AtkPower  },  // 攻撃力計算ハンドラ
    { BTL_EVENT_TURNCHECK_END,        handler_SlowStart_TurnCheck },  // ターンチェック終了ハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fukugan( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_Fukugan },  // 命中率計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SAND )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SUNAGAKURE_HITRATIO );
    }
  }
}
// 天候リアクションハンドラ
static void handler_Sunagakure_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_guard( myHandle, flowWk, pokeID, work, BTL_WEATHER_SAND );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunagakure( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO,   handler_Sunagakure         }, // 命中率計算ハンドラ
    { BTL_EVENT_WEATHER_REACTION, handler_Sunagakure_Weather }, // 天候リアクションハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SNOW )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_SUNAGAKURE_HITRATIO );
    }
  }
}
// 天候リアクションハンドラ
static void handler_Yukigakure_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_weather_guard( myHandle, flowWk, pokeID, work, BTL_WEATHER_SNOW );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yukigakure( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO,   handler_Yukigakure         }, // 命中率計算ハンドラ
    { BTL_EVENT_WEATHER_REACTION, handler_Yukigakure_Weather }, // 天候リアクションハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
/**
 *  特定の天候ダメージを受けない処理共通
 */
static void common_weather_guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, BtlWeather weather )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE) > 0)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == weather)
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Iromegane( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_Iromegane },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BtlTypeAff  aff = BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEAFF);
    BtlTypeAffAbout affAbout = BTL_CALC_TypeAffAbout( aff );
    if( affAbout == BTL_TYPEAFF_ABOUT_ADVANTAGE )
    {
      // ダメージ75％
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.75) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HardRock( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_HardRock },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sniper( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2, handler_Sniper },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}


//------------------------------------------------------------------------------
/**
 *  とくせい「かそく」
 */
//------------------------------------------------------------------------------
// ターンチェックのハンドラ
static void handler_Kasoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_ACTION_DONE) )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->header.tokwin_flag = TRUE;
        param->rankType = BPP_AGILITY_RANK;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->rankVolume = 1;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
//    param->fAlmost = TRUE;
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kasoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,         handler_Kasoku          },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_TYPEMATCH_FLAG) )
    {
      BTL_EVENTVAR_RewriteValue(BTL_EVAR_RATIO, FX32_CONST(2));
      BTL_Printf("ポケ[%d]の てきおうりょく でタイプ一致補正率２倍に\n", pokeID);
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tekiouryoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TYPEMATCH_RATIO, handler_Tekiouryoku }, // タイプ一致補正率計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Mouka( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Mouka },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Gekiryu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Gekiryu },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sinryoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Sinryoku },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MusinoSirase( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_MusinoSirase },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    u32  borderHP = BTL_CALC_QuotMaxHP_Zero( bpp, 3 );
    u32  hp = BPP_GetValue( bpp, BPP_HP );

    if( hp <= borderHP )
    {
      // 使うのが指定タイプワザなら
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == wazaType )
      {
        // 威力1.5倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.5) );
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
        // 威力1.5倍
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.5) );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Konjou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_Konjou },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「プラス」「マイナス」
 */
//------------------------------------------------------------------------------
// 攻撃威力決定のハンドラ
static void handler_PlusMinus( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 攻撃側が自分で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 場に「プラス」or「マイナス」持ちの味方がいて
    if( checkExistTokuseiFriend(flowWk, pokeID, work, POKETOKUSEI_PURASU)
    ||  checkExistTokuseiFriend(flowWk, pokeID, work, POKETOKUSEI_MAINASU)
    ){
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Plus( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER, handler_PlusMinus }, // 攻撃力決定のハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
/**
 *  場に、指定された「とくせい」を持つ味方が居るか判定（プラス・マイナス共通）
 */
static BOOL checkExistTokuseiFriend( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeTokusei tokuseiID )
{
  BtlPokePos pos = BTL_SVFTOOL_PokeIDtoPokePos( flowWk, pokeID );
  if( pos != BTL_POS_NULL )
  {
    BtlExPos  exPos = EXPOS_MAKE( BTL_EXPOS_FULL_FRIENDS, pos );
    u8* pokeIDAry = (u8*)work;
    u8  cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, exPos, pokeIDAry );
    u8 i;
    for(i=0; i<cnt; ++i)
    {
      if( pokeIDAry[i] != pokeID )
      {
        const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeIDAry[i] );
        if( BPP_GetValue(bpp, BPP_TOKUSEI_EFFECTIVE) == tokuseiID )
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 *  とくせい「フラワーギフト」
 */
//------------------------------------------------------------------------------
/**
 *  フラワーギフト有効ポケモンかどうか判定
 */
static BOOL checkFlowerGiftEnablePokemon( BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_GetMonsNo(bpp) == MONSNO_THERIMU ){
    return TRUE;
  }
  return FALSE;
}
// ポケ入場・とくせい取得後ハンドラ
static void handler_FlowerGift_MemberInComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
  {
    u8 nextForm = (BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE)?  FORMNO_THERIMU_POSI : FORMNO_THERIMU_NEGA;
    common_FlowerGift_FormChange( myHandle, flowWk, pokeID, nextForm, TRUE );
    work[0] = 1;
  }
}
// ポケ入場・とくせい取得後ハンドラ
static void handler_FlowerGift_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
    {
      handler_FlowerGift_MemberInComp( myHandle, flowWk, pokeID, work );
    }
  }
}
/**
 *  フラワーギフト共通：フォルムチェンジ処理
 */
static void common_FlowerGift_FormChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, u8 nextForm, u8 fTokWin )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

  if( nextForm != BPP_GetValue(bpp, BPP_FORM) )
  {
    BTL_HANDEX_PARAM_CHANGE_FORM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_FORM, pokeID );
      param->pokeID = pokeID;
      param->formNo = nextForm;
      param->header.tokwin_flag = fTokWin;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_ChangeForm );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
// 天候変化ハンドラ
static void handler_FlowerGift_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
    {
      u8 nextForm = (BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE)?  FORMNO_THERIMU_POSI : FORMNO_THERIMU_NEGA;
      common_FlowerGift_FormChange( myHandle, flowWk, pokeID, nextForm, TRUE );
    }
  }
}
// いえきハンドラ
static void handler_FlowerGift_TokOff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
    {
      if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
      {
        common_FlowerGift_FormChange( myHandle, flowWk, pokeID, FORMNO_THERIMU_NEGA, FALSE );
      }
    }
  }
}
// エアロック通知ハンドラ
static void handler_FlowerGift_AirLock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
    {
      common_FlowerGift_FormChange( myHandle, flowWk, pokeID, FORMNO_THERIMU_NEGA, FALSE );
    }
  }
}

// とくせい書き換え直前ハンドラ
static void handler_FlowerGift_TokChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
    {
      if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
      &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI_NEXT) != BTL_EVENT_FACTOR_GetSubID(myHandle))
      ){
        common_FlowerGift_FormChange( myHandle, flowWk, pokeID, FORMNO_THERIMU_NEGA, FALSE );
      }
    }
  }
}
// 攻撃威力決定のハンドラ
static void handler_FlowerGift_Power( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
  {
    // 天気「はれ」で
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE )
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
}
// 防御力決定のハンドラ
static void handler_FlowerGift_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( checkFlowerGiftEnablePokemon(flowWk, pokeID) )
  {
    // 天気「はれ」で
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE )
    {
      // 防御側が自分か味方のとき
      u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
      if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) )
      {
        // とくぼう1.5倍
        if( BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_SPECIAL )
        {
          BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FLOWERGIFT_GUARDRATIO );
        }
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FlowerGift( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN_COMP,       handler_FlowerGift_MemberInComp }, // ポケ入場ハンドラ
    { BTL_EVENT_MEMBER_IN,            handler_FlowerGift_MemberIn }, // ポケ入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_FlowerGift_MemberIn }, // とくせい書き換えハンドラ
    { BTL_EVENT_WEATHER_CHANGE_AFTER, handler_FlowerGift_Weather  }, // 天候変化後ハンドラ
    { BTL_EVENT_IEKI_FIXED,           handler_FlowerGift_TokOff   }, // いえき確定ハンドラ
    { BTL_EVENT_NOTIFY_AIRLOCK,       handler_FlowerGift_AirLock  },  // エアロック通知ハンドラ
    { BTL_EVENT_ACTPROC_END,          handler_FlowerGift_Weather  },
    { BTL_EVENT_TURNCHECK_DONE,       handler_FlowerGift_Weather  },
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE,handler_FlowerGift_TokChange}, // とくせい書き換え直前ハンドラ
    { BTL_EVENT_ATTACKER_POWER,       handler_FlowerGift_Power    }, // 攻撃力決定のハンドラ
    { BTL_EVENT_DEFENDER_GUARD,       handler_FlowerGift_Guard    }, // 防御力決定のハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tousousin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Tousousin },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    // 威力60以下なら1.5倍にする
    u32 pow = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_POWER );
    if( pow <= 60 )
    {
//      TAYA_Printf("テクニシャン有効\n");
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, BTL_CALC_TOK_TECKNICIAN_POWRATIO );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Technician( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Technician },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_TetunoKobusi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_TetunoKobusi },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( (WAZADATA_GetParam(waza, WAZAPARAM_DAMAGE_REACTION_RATIO) != 0)
    ||  ( (waza==WAZANO_TOBIGERI) || (waza==WAZANO_TOBIHIZAGERI) )
    ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, BTL_CALC_TOK_SUTEMI_POWRATIO );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sutemi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Sutemi },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      // ダメージタイプが物理の時、防御上昇
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_PHYSIC )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_FUSIGINAUROKO_GDRATIO );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FusiginaUroko( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DEFENDER_GUARD, handler_FusiginaUroko },  ///< 防御能力決定のハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SkillLink( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_COUNT, handler_SkillLink },  // 攻撃回数決定のハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「かいりきばさみ」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KairikiBasami( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_KairikiBasami_Check },
    { BTL_EVENT_RANKEFF_FAILED,     handler_KairikiBasami_Guard },

  };

  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Surudoime( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_Surudoime_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_Surudoime_Guard },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ClearBody( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_ClearBody_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_ClearBody_Guard },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_VOLUME) < 0 )
      {
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
    u32 wazaSerial = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_SERIAL );

    if( (wazaSerial == 0)
    ||  (work[1] != wazaSerial)
    ){
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

      work[1] = wazaSerial;
    }

    work[0] = 0;
  }
}



//------------------------------------------------------------------------------
/**
 *  とくせい「たんじゅん」
 */
//------------------------------------------------------------------------------
// ランク増減値修正ハンドラ
static void handler_Tanjun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    int value = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    value *= 2;
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, value );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tanjun( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKVALUE_CHANGE, handler_Tanjun },  // ランク増減値修正ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE )
    {
      // ポケモン系状態異常＆あくびにはならない
      WazaSick sickID = BTL_EVENTVAR_GetValue( BTL_EVAR_SICKID );
      if( (BTL_CALC_IsBasicSickID(sickID))
      ||  (sickID == WAZASICK_AKUBI)
      ){
        work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ReafGuard( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL, handler_ReafGuard         }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,    handler_AddSickFailCommon },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
// とくせい書き換えハンドラ＆ポケ入場ハンドラ
static void handler_Juunan_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, POKESICK_MAHI );
}
// アクション終了毎ハンドラ
static void handler_Juunan_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, POKESICK_MAHI );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Juunan( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_Juunan_PokeSick   }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Juunan_Wake       }, // とくせい書き換えハンドラ
    { BTL_EVENT_MEMBER_IN,            handler_Juunan_Wake       }, // ポケ入場ハンドラ
    { BTL_EVENT_ACTPROC_END,          handler_Juunan_ActEnd     }, // アクション終了毎ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
// とくせい書き換えハンドラ＆ポケ入場ハンドラ
static void handler_Fumin_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, POKESICK_NEMURI );
}
// アクション終了毎ハンドラ
static void handler_Fumin_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, POKESICK_NEMURI );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fumin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_Fumin_PokeSick    }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Fumin_Wake        }, // とくせい書き換えハンドラ
    { BTL_EVENT_MEMBER_IN,            handler_Fumin_Wake        }, // ポケ入場ハンドラ
    { BTL_EVENT_ACTPROC_END,          handler_Fumin_ActEnd      }, // アクション終了毎ハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マグマのよろい」
 */
//------------------------------------------------------------------------------
// ポケモン系状態異常処理ハンドラ
static void handler_MagumaNoYoroi_PokeSick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_GuardWazaSick( flowWk, pokeID, POKESICK_KOORI );
}
// とくせい書き換えハンドラ＆メンバー入場ハンドラ
static void handler_MagumaNoYoroi_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, POKESICK_KOORI );
}
// アクション終了毎ハンドラ
static void handler_MagumaNoYoroi_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, POKESICK_KOORI );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagumaNoYoroi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_MagumaNoYoroi_PokeSick }, // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon      },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_MagumaNoYoroi_Wake     }, // とくせい書き換えハンドラ
    { BTL_EVENT_MEMBER_IN,             handler_MagumaNoYoroi_Wake     }, // メンバー入場ハンドラ
    { BTL_EVENT_ACTPROC_END,           handler_MagumaNoYoroi_ActEnd   }, // アクション終了毎ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
// とくせい書き換えハンドラ＆メンバー入場ハンドラ
static void handler_Meneki_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, POKESICK_DOKU );
}
// アクション終了毎ハンドラ
static void handler_Meneki_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, POKESICK_DOKU );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Meneki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_Meneki_PokeSick   },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Meneki_Wake       },  // とくせい書き換えハンドラ
    { BTL_EVENT_MEMBER_IN,             handler_Meneki_Wake       },  // メンバー入場ハンドラ
    { BTL_EVENT_ACTPROC_END,           handler_Meneki_ActEnd     },  // アクション終了毎ハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
// とくせい書き換えハンドラ＆メンバー入場ハンドラ
static void handler_MizuNoBale_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, WAZASICK_YAKEDO );
}
// アクション終了毎ハンドラ
static void handler_MizuNoBale_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, WAZASICK_YAKEDO );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MizuNoBale( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_MizuNoBale_PokeSick },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon   },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_MizuNoBale_Wake     },  // とくせい書き換えハンドラ
    { BTL_EVENT_MEMBER_IN,            handler_MizuNoBale_Wake     },  // メンバー入場ハンドラ
    { BTL_EVENT_ACTPROC_END,          handler_MizuNoBale_ActEnd   },  // アクション終了毎ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static void handler_MyPace_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, WAZASICK_KONRAN );
}
// アクション終了毎ハンドラ
static void handler_MyPace_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, WAZASICK_KONRAN );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MyPace( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,     handler_MyPace_PokeSick   },  // ポケモン系状態異常処理ハンドラ
    { BTL_EVENT_ADDSICK_FAILED,        handler_AddSickFailCommon },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_MyPace_Wake       },  // とくせい書き換えハンドラ
    { BTL_EVENT_ACTPROC_END,           handler_MyPace_ActEnd     },  // アクション終了毎ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どんかん」
 */
//------------------------------------------------------------------------------
// ワザ系状態異常ハンドラ
static void handler_Donkan( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_GuardWazaSick( flowWk, pokeID, WAZASICK_MEROMERO );
}
// とくせい書き換えハンドラ
static void handler_Donkan_Wake( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSick( flowWk, pokeID, WAZASICK_MEROMERO );
}
// アクション終了毎ハンドラ
static void handler_Donkan_ActEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_TokuseiWake_CureSickCore( flowWk, pokeID, WAZASICK_MEROMERO );
}
// ワザ無効化チェックハンドラ
static void handler_Donkan_NoEffCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // ワザ「ゆうわく」が無効
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( waza == WAZANO_YUUWAKU )
    {
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
            HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
            HANDEX_STR_AddArg( &param->str, pokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Donkan( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADDSICK_CHECKFAIL,    handler_Donkan            },  // 状態異常失敗チェックハンドラ
    { BTL_EVENT_ADDSICK_FAILED,       handler_AddSickFailCommon },
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Donkan_Wake       },  // とくせい書き換えハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,    handler_Donkan_NoEffCheck },  // ワザ無効化チェックハンドラ
    { BTL_EVENT_ACTPROC_END,          handler_Donkan_ActEnd     },  // アクション終了毎ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( guardSick == attackSick ){
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

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    work[0] = FALSE;
  }
}
/**
 * 「とくせい」が有効になった時点で特定の状態異常ならそれを治す処理 共通
 *
 *  スキルスワップ・なりきり等で「とくせい」が書き換わった直後
 *  入場した直後
 *  アクション終了毎（かたやぶりなどで一時的に無効化されたものが復帰する）
 */
static void common_TokuseiWake_CureSick( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick )
{
  if( BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID ) == pokeID )
  {
    common_TokuseiWake_CureSickCore( flowWk, pokeID, sick );
  }
}
static void common_TokuseiWake_CureSickCore( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaSick sick )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_CheckSick(bpp, sick) )
  {
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
        param->sickCode = sick;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Amefurasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_Amefurasi }, // 自分が出場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Amefurasi }, // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hideri( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Hideri },  // 自分が出場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Hideri },  // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunaokosi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,              handler_Sunaokosi }, // 自分が出場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,   handler_Sunaokosi }, // // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yukifurasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Yukifurasi },  // 自分が出場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Yukifurasi },  // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「エアロック」&「ノーてんき」
 */
//------------------------------------------------------------------------------
// 自分が出場ハンドラ
#if 1
static void handler_AirLock_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
//    if( BTL_SVFTOOL_GetWeather(flowWk) != BTL_WEATHER_NONE )
    TAYA_Printf("エアロック持ち入場\n");
    {
      BTL_HANDEX_PARAM_CHANGE_WEATHER* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_WEATHER, pokeID );

        param->header.tokwin_flag = TRUE;
        param->weather = BTL_WEATHER_NONE;
        param->fAirLock = TRUE;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_STD, BTL_STRID_STD_AirLock );

      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
#endif
// 天候変化ハンドラ
static void handler_AirLock_ChangeWeather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AirLock( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,      handler_AirLock_MemberIn       },
    { BTL_EVENT_WEATHER_CHECK,  handler_AirLock_ChangeWeather  },  // 天候チェックハンドラ
//    { BTL_EVENT_WEATHER_CHANGE, handler_AirLock_ChangeWeather },  // 天候変化ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IcoBody( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION, handler_IceBody },   // ターンチェック開始ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_AmeukeZara( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION, handler_AmeukeZara },  // 天候ダメージ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

/**
 *  天候に応じてHP回復するとくせいの共通処理
 */
static void common_weather_recover( BTL_SVFLOW_WORK* flowWk, u8 pokeID, BtlWeather weather )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_WEATHER) == weather)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  ){
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
      param->header.tokwin_flag = TRUE;
      param->pokeID = pokeID;
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 16 );
    BTL_SVF_HANDEX_Pop( flowWk, param );

    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
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
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
          param->pokeID = pokeID;
          param->damage = damage;
      BTL_SVF_HANDEX_Pop( flowWk, param );
      }
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE )
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SunPower( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION, handler_SunPower_Weather },  // 天候ダメージ計算ハンドラ
    { BTL_EVENT_ATTACKER_POWER,  handler_SunPower_AtkPower },  // 攻撃力決定ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「りんぷん」
 */
//------------------------------------------------------------------------------
// 追加効果（状態異常）ハンドラ
static void handler_Rinpun_Sick( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF))
  &&  (pokeID != BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK))
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_SICKID) != WAZASICK_BIND)
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
// 追加効果（ランク効果）ハンドラ
static void handler_Rinpun_Rank( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
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
// ダメージ反応直前ハンドラ
static void handler_Rinpun_Guard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_RINPUNGUARD_FLG, TRUE );
  }
}
static void handler_Rinpun_GuardHitEnd( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (HandCommon_CheckTargetPokeID(pokeID))
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_TARGET_POKECNT) == 1)
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_RINPUNGUARD_FLG, TRUE );
  }
}


static  const BtlEventHandlerTable*  HAND_TOK_ADD_Rinpun( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADD_SICK,               handler_Rinpun_Sick   }, // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET,        handler_Rinpun_Rank   }, // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_SHRINK_CHECK,           handler_Rinpun_Shrink }, // ひるみチェックハンドラ
    { BTL_EVENT_WAZA_DMG_REACTION_PREV, handler_Rinpun_Guard  }, // ダメージ反応直前
    { BTL_EVENT_DAMAGEPROC_END_HIT_PREV,handler_Rinpun_GuardHitEnd  }, // ダメージ反応直前
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_TennoMegumi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ADD_SICK,         handler_TennoMegumi },  // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET,  handler_TennoMegumi },  // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_WAZA_SHRINK_PER,  handler_TennoMegumi },  // ひるみチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「うるおいボディ」
 */
//------------------------------------------------------------------------------
// 状態異常ダメージハンドラ
static void handler_UruoiBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_RAIN )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
      {
        BTL_HANDEX_PARAM_CURE_SICK* param;

        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
          param->sickCode = WAZASICK_EX_POKEFULL;
          param->pokeID[0] = pokeID;
          param->poke_cnt = 1;
          param->header.autoRemoveFlag = TRUE;
        BTL_SVF_HANDEX_Pop( flowWk, param );

        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_UruoiBody( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,      handler_UruoiBody },  // ターンチェック開始
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「だっぴ」
 */
//------------------------------------------------------------------------------
// ターンチェック開始ハンドラ
static void handler_Dappi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetPokeSick(bpp) != POKESICK_NULL )
    {
      if( Tokusei_IsExePer(flowWk, 33) )
      {
        BTL_HANDEX_PARAM_CURE_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CURE_SICK, pokeID );
          param->header.tokwin_flag = TRUE;
          param->header.autoRemoveFlag = TRUE;
          param->sickCode = WAZASICK_EX_POKEFULL;
          param->pokeID[0] = pokeID;
          param->poke_cnt = 1;
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dappi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,      handler_Dappi },  // ターンチェック開始
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      BTL_HANDEX_PARAM_RECOVER_HP* param;

      BTL_EVENTVAR_RewriteValue( BTL_EVAR_DAMAGE, 0 );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
        param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
        param->pokeID = pokeID;
        param->header.tokwin_flag = TRUE;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_PoisonHeal( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SICK_DAMAGE,      handler_PoisonHeal }, // 状態異常ダメージハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KabutoArmor( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CRITICAL_CHECK,     handler_KabutoArmor },  // クリティカルチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「きょううん」
 */
//------------------------------------------------------------------------------
// クリティカルチェックダメージハンドラ
static void handler_Kyouun( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( pokeID == BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) )
  {
    u8 rank = BTL_EVENTVAR_GetValue( BTL_EVAR_CRITICAL_RANK ) + 1;
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_CRITICAL_RANK, rank );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kyouun( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CRITICAL_CHECK,     handler_Kyouun }, // クリティカルチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「いかりのつぼ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_IkarinoTubo( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    // クリティカルだったら攻撃マックスまでアップ
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_CRITICAL_FLAG) )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      int rankupVolume = BPP_RankEffectUpLimit( bpp, BPP_ATTACK_RANK );
      if( rankupVolume > 0 )
      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->rankType = WAZA_RANKEFF_ATTACK;
          param->rankVolume = BPP_RankEffectUpLimit( bpp, BPP_ATTACK_RANK );
          param->fAlmost = TRUE;
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->fStdMsgDisable = TRUE;
          param->header.tokwin_flag = TRUE;
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_IkarinoTubo );
          HANDEX_STR_AddArg( &param->exStr, pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IkarinoTubo( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_IkarinoTubo },  // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DokunoToge( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_DokunoToge }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
  common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_SEIDENKI_PER );
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Seidenki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Seidenki }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HonoNoKarada( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_HonoNoKarada }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「メロメロボディ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_MeromeroBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  &&  (!BTL_SVFTOOL_CheckShowDown(flowWk))
  ){
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MeromeroBody( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_MeromeroBody }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ほうし」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Housi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    PokeSick sick;
    BPP_SICK_CONT cont;

    {
      u32 rand = BTL_CALC_GetRand( 30 );

      if( rand > 20 ){
        sick = WAZASICK_DOKU;
      }else if( rand > 10 ){
        sick = WAZASICK_MAHI;
      }else{
        sick = WAZASICK_NEMURI;
      }
      cont = BTL_CALC_MakeDefaultPokeSickCont( sick );
      common_touchAddSick( flowWk, pokeID, sick, cont, BTL_CALC_TOK_HOUSI_PER );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Housi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Housi },  // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      if( Tokusei_IsExePer(flowWk, per) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
          param->header.tokwin_flag = TRUE;
          param->sickID = sick;
          param->sickCont = sickCont;
          param->fAlmost = FALSE;
          param->pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「さめはだ」「てつのとげ」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Samehada( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      u8 attackerPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, attackerPokeID );

      if( !BPP_IsDead(bpp) )
      {
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
            param->pokeID = attackerPokeID;
            param->damage = BTL_CALC_QuotMaxHP( bpp, 8 );
            HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Samehada );
            HANDEX_STR_AddArg( &param->exStr, attackerPokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Samehada( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Samehada }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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

        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
            param->pokeID = attackerPokeID;
            param->damage = BTL_CALC_QuotMaxHP( bpp, 4 );
            param->fAvoidHidePoke = TRUE;
            HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_SimpleDamage );
            HANDEX_STR_AddArg( &param->exStr, attackerPokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yuubaku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Yuubaku },  // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「へんしょく」
 */
//------------------------------------------------------------------------------
// ダメージ直後ハンドラ
static void handler_Hensyoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (HandCommon_CheckTargetPokeID(pokeID) )
  &&  (!BTL_SVFTOOL_CheckShowDown(flowWk))
  ){
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( !BPP_IsDead(bpp) )
    {
      PokeType wazaType = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );

      if( (wazaType != POKETYPE_NULL) && (!BPP_IsMatchType(bpp, wazaType)) )
      {
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_CHANGE_TYPE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TYPE, pokeID );
            param->next_type = PokeTypePair_MakePure( wazaType );
            param->pokeID = pokeID;
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hensyoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DAMAGEPROC_END_HIT_REAL,     handler_Hensyoku }, // ダメージ直後ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      if( (attackPokeID != BTL_POKEID_NULL) && (attackPokeID != pokeID) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param;
        const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
        BPP_SICK_CONT cont;

        cont.raw = BTL_EVENTVAR_GetValue( BTL_EVAR_SICK_CONT );

        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
          param->pokeID = attackPokeID;
          param->sickID = sick;
          // もうどくは継続パラメータをそのままにする必要がある
          if( (sick == POKESICK_DOKU) && (BPP_SICKCONT_IsMoudokuCont(cont)) ){
            param->sickCont = cont;
          }else{
            BTL_CALC_MakeDefaultWazaSickCont( sick, bpp, &param->sickCont );
          }
          param->fAlmost = TRUE;
        BTL_SVF_HANDEX_Pop( flowWk, param );

        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Syncro( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_POKESICK_FIXED,      handler_Syncro }, // ポケモン系状態異常ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Isiatama( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_KICKBACK,      handler_Isiatama }, // 反動計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NormalSkin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_PARAM,   handler_NormalSkin }, // ワザタイプ決定ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「トレース」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Trace( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN, handler_Trace },     // 入場ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 入場ハンドラ
static void handler_Trace( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 inPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

  u8 targetPokeID = BTL_POKEID_NULL;
  PokeTokusei nextTok = POKETOKUSEI_NULL;


  // 自分が入場したタイミングは相手を全部見てランダム
  if( inPokeID == pokeID )
  {
    u8  allPokeID[ BTL_POSIDX_MAX ];
    u8  checkPokeID[ BTL_POSIDX_MAX ];
    const BTL_POKEPARAM* bpp;
    u8  pokeCnt, targetCnt, i;
    PokeTokusei tok;

    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
    BtlExPos   exPos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );

    pokeCnt = BTL_SVFTOOL_ExpandPokeID( flowWk, exPos, allPokeID );
    targetCnt = 0;
    for(i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, allPokeID[i] );
      tok = BPP_GetValue( bpp, BPP_TOKUSEI );
      if( !BTL_TABLES_CheckTraceFailTokusei(tok) )
      {
        checkPokeID[ targetCnt++ ] = allPokeID[i];
      }
    }

    // トレースできる対象がいればトレースする
    if( targetCnt )
    {
      u8 idx = (targetCnt==1)? 0 : BTL_CALC_GetRand( targetCnt );
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, checkPokeID[idx] );
      nextTok = BPP_GetValue( bpp, BPP_TOKUSEI );
      targetPokeID = BPP_GetID( bpp );
    }
    // いなければ待機状態へ
    else{
      work[0] = 1;
    }
  }
  // 相手が入場したタイミングに待機状態なら反応
  else if( (!BTL_MAINUTIL_IsFriendPokeID(inPokeID, pokeID))
  &&        (work[0] == 1)
  ){
    BtlPokePos inPokePos = BTL_SVFTOOL_PokeIDtoPokePos( flowWk, inPokeID );
    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );

    if( (inPokePos != BTL_POS_NULL)
    &&  BTL_MAINUTIL_CheckTripleHitArea(myPos, inPokePos)
    ){
      const BTL_POKEPARAM* inPoke = BTL_SVFTOOL_GetPokeParam( flowWk, inPokeID );
      PokeTokusei tok = BPP_GetValue( inPoke, BPP_TOKUSEI );

      if( !BTL_TABLES_CheckTraceFailTokusei(tok) )
      {
        nextTok = tok;
        targetPokeID = inPokeID;
      }
    }
  }

  if( (nextTok != POKETOKUSEI_NULL) && (pokeID != BTL_POKEID_NULL) )
  {
    BTL_HANDEX_PARAM_CHANGE_TOKUSEI* param;
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TOKUSEI, pokeID );
      param->pokeID = pokeID;
      param->tokuseiID = nextTok;
      param->fSkipMemberInEvent = TRUE;
      param->header.tokwin_flag = TRUE;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Trace );
      HANDEX_STR_AddArg( &param->exStr, targetPokeID );
      HANDEX_STR_AddArg( &param->exStr, param->tokuseiID );
    BTL_SVF_HANDEX_Pop( flowWk, param );
    BTL_N_PrintfEx( PRINT_CHANNEL_PRESSURE, DBGSTR_HANDTOK_TRACE_Add, pokeID );
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
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SizenKaifuku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_OUT_FIXED, handler_SizenKaifuku },   // メンバー退出ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( pokeCnt )
    {
      const BTL_POKEPARAM* bpp;
      u16 sumDef = 0;
      u16 sumSpDef = 0;
      u8  valueID;
      u8 i;
      for(i=0; i<pokeCnt; ++i){
        bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
        sumDef += BPP_GetValue( bpp, BPP_DEFENCE );
        sumSpDef += BPP_GetValue( bpp, BPP_SP_DEFENCE );
      }

      // 相手のぼうぎょ・とくぼうを比較して、弱い方に対応した自分のこうげき・とくこうをアップ
      //（同じなら とくこう をアップ
      if( sumDef >= sumSpDef ){
        valueID = BPP_SP_ATTACK_RANK;
      }else{
        valueID = BPP_ATTACK_RANK;
      }

      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->header.tokwin_flag = TRUE;
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->rankType = valueID;
          param->rankVolume = 1;
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DownLoad( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Download },    // メンバー入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Download },    // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    typedef struct {
      u8      pokeID;
      WazaID  wazaID;
    }MAX_PRIORITY_PARAM;

    const BTL_POKEPARAM* bpp;
    MAX_PRIORITY_PARAM*  maxPriParam;
    WazaID waza;
    u8 ePokeID[BTL_POSIDX_MAX];
    u8 pokeCnt = BTL_SVFTOOL_GetAllOpponentFrontPokeID( flowWk, pokeID, ePokeID );
    u8 wazaCnt, i, w;
    u8 pri, maxPri, maxPriCnt;

    maxPri = 0;
    maxPriCnt = 0;
    maxPriParam = BTL_SVFTOOL_GetTmpWork( flowWk, sizeof(MAX_PRIORITY_PARAM) * BTL_POSIDX_MAX );

    for(i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
      wazaCnt = BPP_WAZA_GetCount( bpp );
      for(w=0; w<wazaCnt; ++w)
      {
        waza = BPP_WAZA_GetID( bpp, w );
        // ダメージワザ
        if( WAZADATA_GetDamageType(waza) != WAZADATA_DMG_NONE )
        {
          pri = WAZADATA_GetPower( waza );
          if( pri == 1 )
          {
            if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_ICHIGEKI ){
              pri = 150;
            }else {

              switch( waza ){
              case WAZANO_KAUNTAA:
              case WAZANO_MIRAAKOOTO:
              case WAZANO_METARUBAASUTO:
                pri = 120;
                break;
              default:
                pri = 80;
              }
            }
          }
        }
        // 非ダメージワザ
        else
        {
          pri = 1;
        }

        if( pri >= maxPri )
        {
          if( pri > maxPri ){
            maxPri = pri;
            maxPriCnt = 0;
          }
          maxPriParam[ maxPriCnt ].pokeID = BPP_GetID( bpp );
          maxPriParam[ maxPriCnt ].wazaID = waza;
          ++maxPriCnt;
        }
      }
    }

    if( maxPriCnt )
    {
      i = BTL_CALC_GetRand( maxPriCnt );

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_YotimuExe );
          HANDEX_STR_AddArg( &param->str, maxPriParam[i].pokeID );
          HANDEX_STR_AddArg( &param->str, maxPriParam[i].wazaID );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yotimu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_Yotimu }, // 入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Yotimu }, // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      if( check_kikenyoti_poke(bppUser, bppEnemy) )
      {
        break;
      }
    }
    if( i != pokeCnt )
    {
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Kikenyoti );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
/**
 *  きけんよち該当ポケモンかチェック
 */
static BOOL check_kikenyoti_poke( const BTL_POKEPARAM* bppUser, const BTL_POKEPARAM* bppEnemy )
{
  WazaID  waza;
  PokeTypePair user_type;
  u8 waza_cnt, i;

  user_type = BPP_GetPokeType( bppUser );
  waza_cnt = BPP_WAZA_GetCount( bppEnemy );
  for(i=0; i<waza_cnt; ++i)
  {
    // 一撃必殺ワザはキケン
    waza = BPP_WAZA_GetID( bppEnemy, i );
    if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_ICHIGEKI ){ return TRUE; }

    // 自分に相性バツグンな奴もキケン
    if( WAZADATA_IsDamage(waza) )
    {
      PokeType  waza_type = WAZADATA_GetType( waza );
      if( BTL_CALC_TypeAffPair(waza_type, user_type) > BTL_TYPEAFF_100 ){ return TRUE; }
    }

  }
  return FALSE;
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_KikenYoti( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_KikenYoti },  // 入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_KikenYoti },  // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      i = BTL_CALC_GetRand( itemCnt );
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Omitoosi );
          HANDEX_STR_AddArg( &param->str, pokeID );
          HANDEX_STR_AddArg( &param->str, itemID[i] );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Omitoosi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_Omitoosi }, // 入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Omitoosi }, // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「がんじょう」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Ganjou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ICHIGEKI_CHECK, handler_Ganjou_Ichigeki   },   // 一撃チェックハンドラ
    { BTL_EVENT_KORAERU_CHECK,  handler_Ganjou_KoraeCheck },   // こらえるチェックハンドラ
    { BTL_EVENT_KORAERU_EXE,    handler_Ganjou_KoraeExe   },   // こらえる発動ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 一撃チェックハンドラ
static void handler_Ganjou_Ichigeki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_FLAG, TRUE) )
    {
      BTL_HANDEX_PARAM_MESSAGE* param;

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
// こらえるチェックハンドラ
static void handler_Ganjou_KoraeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_IsHPFull(bpp) )
    {
      work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_KORAERU_CAUSE, BPP_KORAE_TOKUSEI_DEFENDER );
    }
    else{
      work[0] = 0;
    }
  }
}
// こらえる発動ハンドラ
static void handler_Ganjou_KoraeExe( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  ( work[0] )
  ){
    BTL_HANDEX_PARAM_MESSAGE* param;

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Koraeru_Exe );
      HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    work[0] = 0;
  }
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tennen( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RANK,       handler_Tennen_hitRank }, // 命中率・回避率チェックハンドラ
    { BTL_EVENT_ATTACKER_POWER_PREV, handler_Tennen_AtkRank }, // 攻撃ランクチェックハンドラ
    { BTL_EVENT_DEFENDER_GUARD_PREV, handler_Tennen_DefRank }, // 防御ランクチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE ) == POKETYPE_HONOO )
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tainetu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,   handler_Tainetsu_WazaPow },   // ワザ威力計算ハンドラ
    { BTL_EVENT_SICK_DAMAGE,  handler_Tainetsu_SickDmg },   // 状態異常ダメージ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}




/**
 *  ダメージ回復化処理共通：回復化チェック
 */
static BOOL common_DmgToRecover_Check( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType matchType )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == matchType)
  ){
    work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
    return work[0];
  }
  return FALSE;
}
/**
 *  特定タイプワザを無効化するチェック処理共通
 */
static BOOL common_TypeNoEffect( BTL_SVFLOW_WORK* flowWk, u8 pokeID, PokeType wazaType )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) != pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == wazaType)
  ){
    return BTL_EVENTVAR_RewriteValue( BTL_EVAR_NOEFFECT_FLAG, TRUE );
  }
  return FALSE;
}
/**
 *  特定タイプワザを無効化->HP回復の共通処理
 */
static void common_TypeRecoverHP( BTL_SVFLOW_WORK* flowWk, u8 pokeID, u8 denomHP )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

  if( !BPP_IsHPFull(bpp) )
  {
    BTL_HANDEX_PARAM_RECOVER_HP*  param = BTL_SVF_HANDEX_Push( flowWk ,BTL_HANDEX_RECOVER_HP, pokeID );
      param->pokeID = pokeID;
      param->recoverHP = BTL_CALC_QuotMaxHP( bpp, denomHP );
      param->header.tokwin_flag = TRUE;
      HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_HP_Recover );
      HANDEX_STR_AddArg( &param->exStr, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
  else
  {
    BTL_HANDEX_PARAM_MESSAGE* msg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
      msg_param->header.tokwin_flag = TRUE;
    BTL_SVF_HANDEX_Pop( flowWk, msg_param );
  }
}
/**
 *  特定タイプワザを無効化->ランクアップ効果の共通処理
 */
static void common_TypeNoEffect_Rankup( BTL_SVFLOW_WORK* flowWk, u8 pokeID, WazaRankEffect rankType, u8 volume )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_IsRankEffectValid(bpp, rankType, volume) )
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->fAlmost = TRUE;
      param->rankType = rankType;
      param->rankVolume = volume;
      param->header.tokwin_flag = TRUE;
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
  else
  {
    BTL_HANDEX_PARAM_MESSAGE* msg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &msg_param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &msg_param->str, pokeID );
      msg_param->header.tokwin_flag = TRUE;
    BTL_SVF_HANDEX_Pop( flowWk, msg_param );
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
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
          param->pokeID = pokeID;
          param->damage = BTL_CALC_QuotMaxHP( bpp, 8 );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    // 雨の時、MAXHPの 1/8 回復
    }
    else if( weather == BTL_WEATHER_RAIN )
    {
      BTL_HANDEX_PARAM_RECOVER_HP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RECOVER_HP, pokeID );
        param->header.tokwin_flag = TRUE;
        param->pokeID = pokeID;
        param->recoverHP = BTL_CALC_QuotMaxHP( bpp, 8 );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
// ダメージワザ回復化チェックハンドラ
static void handler_Kansouhada_DmgRecover( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_MIZU );
}
// ワザ威力計算ハンドラ
static void handler_Kansouhada_WazaPow( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側の時、ほのおワザの威力を1.25倍
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_HONOO )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.25) );
    }
  }
}



// 無効化チェックハンドラ
static void handler_Kansouhada_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect( flowWk, pokeID, POKETYPE_MIZU ) )
  {
    common_TypeRecoverHP( flowWk, pokeID, 4 );
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kansouhada( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION,      handler_Kansouhada_Weather    },  // 天候ダメージ計算ハンドラ
    { BTL_EVENT_WAZA_POWER,            handler_Kansouhada_WazaPow    },  // ワザ威力計算ハンドラ

//    { BTL_EVENT_DMG_TO_RECOVER_CHECK,  handler_Kansouhada_DmgRecover    },  // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,     handler_Kansouhada_Check  },  // 無効化チェックハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちょすい」
 */
//------------------------------------------------------------------------------
// ダメージワザ回復化チェックハンドラ
static void handler_Tyosui_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect( flowWk, pokeID, POKETYPE_MIZU ) )
  {
    common_TypeRecoverHP( flowWk, pokeID, 4 );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tyosui( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
//    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Tyosui_Check }, // ダメージワザ回復チェックハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,      handler_Tyosui_Check },   // 無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちくでん」
 */
//------------------------------------------------------------------------------
// ワザ無効チェックレベル３
static void handler_Tikuden_CheckEx( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect( flowWk, pokeID, POKETYPE_DENKI ) )
  {
    common_TypeRecoverHP( flowWk, pokeID, 4 );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikuden( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L3,      handler_Tikuden_CheckEx },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「でんきエンジン」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_DenkiEngine( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
//    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_DenkiEngine_Check   },  // ダメージワザ回復チェックハンドラ
//    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_DenkiEngine_Fix     },  // ダメージワザ回復化決定ハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,      handler_DenkiEngine_CheckEx },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

// ダメージワザ回復化チェックハンドラ
static void handler_DenkiEngine_Check( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  common_DmgToRecover_Check( flowWk, pokeID, work, POKETYPE_DENKI );
}
// ダメージワザ回復化決定ハンドラ
static void handler_DenkiEngine_Fix( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
      param->poke_cnt = 1;
      param->pokeID[0] = pokeID;
      param->fAlmost = TRUE;
      param->rankType = BPP_AGILITY_RANK;
      param->rankVolume = 1;
      param->header.tokwin_flag = TRUE;
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
// ワザ無効チェックレベル２
static void handler_DenkiEngine_CheckEx( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect( flowWk, pokeID, POKETYPE_DENKI ) )
  {
    common_TypeNoEffect_Rankup( flowWk, pokeID, BPP_AGILITY_RANK, 1 );
  }
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
      param->pokeID = atkID;
      param->sickID = WAZASICK_MUSTHIT;;
      param->sickCont = BPP_SICKCONT_MakeTurn( 1 );
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NoGuard( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXE_START,        handler_NoGuard  },  // タイプによる無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kimottama( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_AFFINITY,        handler_Kimottama },  // タイプによる無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
            HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
            HANDEX_STR_AddArg( &param->str, pokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bouon( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L3,        handler_Bouon },  // 無効化チェックLv1ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_GEN_FLAG) == FALSE )
    {
      work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, TRUE );
    }
  }
}
// 飛行フラグによる地面ワザ無効化ハンドラ
static void handler_Fuyuu_Disp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (work[0])
  ){
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      param->header.tokwin_flag = TRUE;
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
      HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );
    work[0] = 0;
  }
}
// ターンチェック開始ハンドラ
static void handler_Fuyuu_TurnCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    work[0] = 0;
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Fuyuu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_FLYING,          handler_Fuyuu      },  // 飛行フラグチェックハンドラ
    { BTL_EVENT_WAZA_NOEFF_BY_FLYING,  handler_Fuyuu_Disp },  // 飛行フラグによる地面ワザ無効化ハンドラ
    { BTL_EVENT_TURNCHECK_BEGIN,       handler_Fuyuu_TurnCheck },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) != pokeID)
  ){
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

    // 「わるあがき」以外のダメージワザの場合
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_IsDamage(waza) && (waza != WAZANO_WARUAGAKI) )
    {
      // 効果バツグン以外は無効
      PokeType waza_type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
      PokeTypePair myType = BPP_GetPokeType( bpp );
      BtlTypeAff aff = BTL_CALC_TypeAffPair( waza_type, myType );
      if( aff <= BTL_TYPEAFF_100 )
      {
        if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
        {
          BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
          {
            BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
              HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
              HANDEX_STR_AddArg( &param->str, pokeID );
            BTL_SVF_HANDEX_Pop( flowWk, param );
          }
          BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
        }
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FusiginaMamori( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L3,        handler_FusiginaMamori }, // 無効化チェックLv2ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    if( work[0] )
    {
      work[1] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_TOKUSEI );
      work[0] = 0;
    }
    else{
      work[0] = 1;
    }
  }
}
// とくせい書き換え直後ハンドラ
static void handler_Namake_Get( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_TURNFLAG_Get(bpp, BPP_TURNFLG_ACTION_START) ){
      work[0] = 1;
    }
  }
}
// ワザだし失敗表示ハンドラ
static void handler_Nameke_Failed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( work[1] )
    {
      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        param->header.tokwin_flag = TRUE;
        HANDEX_STR_Setup( &(param->str), BTL_STRTYPE_SET, BTL_STRID_SET_Namake );
        HANDEX_STR_AddArg( &(param->str), pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );

      work[1] = 0;
    }
  }
}
// アクション終了ハンドラ
static void handler_Nameke_EndAct( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 反動ターンになまけフラグが残っているのはクリア
    // （アイテムターンも残ってるハズだが、シューターとの組み合わせが強いので何もしない）
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_ACTION) == BTL_ACTION_SKIP )
    {
      work[0] = 0;
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Namake( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_CHECK_1ST,   handler_Namake        }, // ワザ出し成否チェックハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,     handler_Namake_Get    }, // とくせい書き換え直後ハンドラ
    { BTL_EVENT_WAZA_EXECUTE_FAIL,        handler_Nameke_Failed }, // ワザ出し失敗表示ハンドラ
    { BTL_EVENT_ACTPROC_END,              handler_Nameke_EndAct }, // アクション終了ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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

  work[0] = 0;

  if( (waza == WAZANO_DAIBAKUHATU)
  ||  (waza == WAZANO_ZIBAKU)
  ){
    SV_WazaFailCause cause = BTL_EVENTVAR_GetValue( BTL_EVAR_FAIL_CAUSE );
    if( cause == SV_WAZAFAIL_NULL )
    {
      work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_TOKUSEI );
      work[1] = waza;
    }
  }
}
static void handler_Simerike_Effective( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    u8  targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      param->header.tokwin_flag = TRUE;
      HANDEX_STR_Setup( &(param->str), BTL_STRTYPE_SET, BTL_STRID_SET_WazaCantUse );
      HANDEX_STR_AddArg( &(param->str), targetPokeID );
      HANDEX_STR_AddArg( &(param->str), work[1] );
    BTL_SVF_HANDEX_Pop( flowWk, param );

    work[0] = 0;
  }
}
// ワザ処理開始ハンドラ
static void handler_Simerike_StartSeq( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Simerike_SkipCheck );
}
// ワザ処理終了ハンドラ
static void handler_Simerike_EndSeq( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
}
static void handler_Simerike_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID ){
    BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
  }
}

// スキップチェックハンドラ
static BOOL handler_Simerike_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  // とくせい「しめりけ」だけ無効
  if( factorType == BTL_EVENT_FACTOR_TOKUSEI )
  {
    if( subID == POKETOKUSEI_YUUBAKU ){
      return TRUE;
    }
  }
  return FALSE;
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Simerike( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_CHECK_2ND,   handler_Simerike            },  // ワザ出し成否チェックハンドラ
    { BTL_EVENT_WAZA_EXECUTE_FAIL,        handler_Simerike_Effective  },
    { BTL_EVENT_WAZASEQ_START,            handler_Simerike_StartSeq   },  // ワザ処理開始ハンドラ
    { BTL_EVENT_WAZASEQ_END,              handler_Simerike_EndSeq     },  // ワザ処理終了ハンドラ
    { BTL_EVENT_IEKI_FIXED,               handler_Simerike_Ieki       },  // いえき確定

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_MORAIBI) == FALSE )
    {
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      {
        BTL_HANDEX_PARAM_SET_CONTFLAG* flg_param;
        BTL_HANDEX_PARAM_MESSAGE* param;

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MoraibiExe );
          HANDEX_STR_AddArg( &param->str, pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );

        flg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SET_CONTFLAG, pokeID );
          flg_param->pokeID = pokeID;
          flg_param->flag = BPP_CONTFLG_MORAIBI;
        BTL_SVF_HANDEX_Pop( flowWk, flg_param );
      }
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
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
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_MORAIBI) )
    {
      // 炎ワザの威力1.5倍
      if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_HONOO )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, BTL_CALC_TOK_MORAIBI_POWRATIO );
      }
    }
  }
}
// とくせい書き換え直前ハンドラ
static void handler_Moraibi_Remove( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 「もらいび」状態の解除

    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_MORAIBI) )
    {
      BTL_HANDEX_PARAM_RESET_CONTFLAG* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RESET_CONTFLAG, pokeID );
        param->pokeID = pokeID;
        param->flag = BPP_CONTFLG_MORAIBI;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
// 無効化チェックハンドラ
static void handler_Moraibi_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect( flowWk, pokeID, POKETYPE_HONOO ) )
  {
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      if( BPP_CONTFLAG_Get(bpp, BPP_CONTFLG_MORAIBI) == FALSE )
      {
        BTL_HANDEX_PARAM_MESSAGE* param;
        BTL_HANDEX_PARAM_SET_CONTFLAG* flg_param;

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_MoraibiExe );
          HANDEX_STR_AddArg( &param->str, pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );

        flg_param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SET_CONTFLAG, pokeID );
          flg_param->pokeID = pokeID;
          flg_param->flag = BPP_CONTFLG_MORAIBI;
        BTL_SVF_HANDEX_Pop( flowWk, flg_param );
      }
      else
      {
        BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
          HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
          HANDEX_STR_AddArg( &param->str, pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Moraibi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
//    { BTL_EVENT_DMG_TO_RECOVER_CHECK,  handler_Moraibi_NoEffect      }, // ダメージワザ回復化チェックハンドラ
//    { BTL_EVENT_DMG_TO_RECOVER_FIX,    handler_Moraibi_DmgRecoverFix }, // ダメージ回復化決定ハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,     handler_Moraibi_CheckNoEffect }, // 無効化チェックハンドラ

    { BTL_EVENT_ATTACKER_POWER,        handler_Moraibi_AtkPower      }, // 攻撃力決定ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Moraibi_Remove        }, // とくせい書き換え直前ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ナイトメア」
 */
//------------------------------------------------------------------------------
// ターンチェック最終ハンドラ
static void handler_Nightmare( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp;
    u8 ePokeID[ BTL_POSIDX_MAX ];
    u8 pokeCnt, i;

    u8 exe_flag = FALSE;
    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
    BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );

    pokeCnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, ePokeID );

    for(i=0; i<pokeCnt; ++i)
    {
      bpp = BTL_SVFTOOL_GetPokeParam( flowWk, ePokeID[i] );
      if( BPP_CheckSick(bpp, WAZASICK_NEMURI) )
      {
        BTL_HANDEX_PARAM_DAMAGE* param;

        if( exe_flag == FALSE ){
          exe_flag = TRUE;
          BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        }

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
          param->pokeID = ePokeID[i];
          param->damage = BTL_CALC_QuotMaxHP( bpp, 8 );
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_NightMare );
          HANDEX_STR_AddArg( &param->exStr, ePokeID[i] );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
    if( exe_flag ){
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nightmare( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,        handler_Nightmare },  // ターンチェック最終ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「にげあし」
 */
//------------------------------------------------------------------------------
// にげるチェックハンドラ
static void handler_Nigeasi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  HandCommon_NigeruCalcSkip( myHandle, flowWk, pokeID, work );
}
// にげ確定メッセージハンドラ
static void handler_Nigeasi_Msg( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( HandCommon_CheckNigeruExMsg(myHandle, flowWk, pokeID) )
  {
    BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      param->header.tokwin_flag = TRUE;
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_EscapeSuccess );
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nigeasi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SKIP_NIGERU_CALC,     handler_Nigeasi       }, // にげるチェックハンドラ
    { BTL_EVENT_NIGERU_EXMSG,         handler_Nigeasi_Msg   }, // 逃げ特殊メッセージハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    u16 tokID = BTL_EVENT_FACTOR_GetSubID( myHandle );
    u16 strID;

    switch( tokID ){
    case POKETOKUSEI_TAABOBUREIZU:  strID = BTL_STRID_SET_TurboBraze; break;
    case POKETOKUSEI_TERABORUTEEZI: strID = BTL_STRID_SET_TeraVoltage; break;
    default:  strID = BTL_STRID_SET_Katayaburi; break;
    }

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, strID );
      HANDEX_STR_AddArg( &param->str, pokeID );
    BTL_SVF_HANDEX_Pop( flowWk, param );

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// スキップチェックハンドラ
static BOOL handler_Katayaburi_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( factorType == BTL_EVENT_FACTOR_TOKUSEI )
  {
    if( BTL_TABLES_IsMatchKatayaburiTarget(subID) )
    {
      return TRUE;
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
    if( work[0] == 0 ){
      BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Katayaburi_SkipCheck );
      work[0] = 1;
    }
  }
}
// ワザ処理終了ハンドラ
static void handler_Katayaburi_End( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側ならスキップチェックハンドラをデタッチする
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    if( work[0] == 1 ){
      BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
      work[0] = 0;
    }
  }
}
// いえき確定
static void handler_Katayaburi_Ieki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // スキップチェックハンドラをデタッチ
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID ){
    if( work[0] == 1 ){
      BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
      work[0] = 0;
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Katayaburi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Katayaburi_MemberIn },  // 入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Katayaburi_MemberIn },  // とくせい書き換えハンドラ
    { BTL_EVENT_WAZASEQ_START,        handler_Katayaburi_Start    },  // ワザ処理開始ハンドラ
    { BTL_EVENT_WAZASEQ_END,          handler_Katayaburi_End      },  // ワザ処理終了ハンドラ
    { BTL_EVENT_IEKI_FIXED,           handler_Katayaburi_Ieki     },  // いえき確定
  };

  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「てんきや」
 */
//------------------------------------------------------------------------------
// ポケ入場完了ハンドラ
static void handler_Tenkiya_MemberInComp( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  BtlWeather weather = BTL_SVFTOOL_GetWeather( flowWk );
  common_TenkiFormChange( flowWk, pokeID, weather );
  work[0] = 1;
}
// ポケ入場ハンドラ
static void handler_Tenkiya_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
    {
      handler_Tenkiya_MemberInComp( myHandle, flowWk, pokeID, work );
    }
  }
}
// 天候変化ハンドラ
static void handler_Tenkiya_Weather( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    BtlWeather weather = BTL_SVFTOOL_GetWeather( flowWk );
    common_TenkiFormChange( flowWk, pokeID, weather );
  }
}
// エアロック通知ハンドラ
static void handler_Tenkiya_AirLock( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    common_Tenkiya_Off( myHandle, flowWk, pokeID );
  }
}

// とくせい書き換え直前ハンドラ
static void handler_Tenkiya_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 「てんきや」以外のとくせいに書き換わる時、フォルムをノーマルに戻す
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI_NEXT) != BTL_EVENT_FACTOR_GetSubID(myHandle) )
    {
      common_Tenkiya_Off( myHandle, flowWk, pokeID );
    }
  }
}
// いえき確定
static void handler_Tenkiya_TokOff( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    common_Tenkiya_Off( myHandle, flowWk, pokeID );
  }
}
static void common_Tenkiya_Off( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID )
{
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetMonsNo(bpp) == MONSNO_POWARUN )
    {
      BTL_HANDEX_PARAM_CHANGE_FORM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_FORM, pokeID );
        param->pokeID = pokeID;
        param->formNo = FORMNO_POWARUN_NORMAL;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_ChangeForm );
        HANDEX_STR_AddArg( &param->exStr, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
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
      form_next = FORMNO_POWARUN_NORMAL;
    }

    if( form_next != form_now )
    {
      BTL_HANDEX_PARAM_CHANGE_FORM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_FORM, pokeID );
        param->header.tokwin_flag = TRUE;
        param->pokeID = pokeID;
        param->formNo = form_next;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_ChangeForm );
        HANDEX_STR_AddArg( &param->exStr, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tenkiya( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN_COMP,        handler_Tenkiya_MemberInComp  },  // ポケ入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Tenkiya_MemberIn  },  // とくせい書き換え直後ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Tenkiya_ChangeTok },  // とくせい書き換え直前ハンドラ
    { BTL_EVENT_IEKI_FIXED,            handler_Tenkiya_TokOff    },  // いえき確定ハンドラ
    { BTL_EVENT_NOTIFY_AIRLOCK,        handler_Tenkiya_AirLock   },  // エアロック通知ハンドラ
    { BTL_EVENT_ACTPROC_END,           handler_Tenkiya_Weather   },
    { BTL_EVENT_TURNCHECK_DONE,        handler_Tenkiya_Weather   },
    { BTL_EVENT_WEATHER_CHANGE_AFTER,  handler_Tenkiya_Weather   },  // 天候変化後ハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「よびみず」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yobimizu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TEMPT_TARGET,         handler_Yobimizu               }, // ワザターゲット決定ハンドラ
    { BTL_EVENT_WAZA_EXE_START,       handler_Hiraisin_WazaExeStart  }, // ワザ実行開始ハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,    handler_Yobimizu_CheckNoEffect }, // 無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ワザターゲット引き寄せハンドラ
static void handler_Yobimizu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_WazaTargetChangeToMe( flowWk, pokeID, work, POKETYPE_MIZU );
}
// 無効化チェックハンドラ
static void handler_Yobimizu_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect(flowWk, pokeID, POKETYPE_MIZU) )
  {
    common_TypeNoEffect_Rankup( flowWk, pokeID, BPP_SP_ATTACK_RANK, 1 );
  }
}

//------------------------------------------------------------------------------
/**
 *  とくせい「ひらいしん」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hiraisin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TEMPT_TARGET,         handler_Hiraisin               }, // ワザターゲット決定ハンドラ
    { BTL_EVENT_WAZA_EXE_START,       handler_Hiraisin_WazaExeStart  }, // ワザ実行開始ハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,    handler_Hiraisin_CheckNoEffect }, // 無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

// ワザターゲット決定ハンドラ
static void handler_Hiraisin( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  work[0] = common_WazaTargetChangeToMe( flowWk, pokeID, work, POKETYPE_DENKI );
}
// ワザ実行開始ハンドラ
static void handler_Hiraisin_WazaExeStart( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( work[0] )
  {
    if( HandCommon_CheckTargetPokeID(pokeID) )
    {
      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        param->header.tokwin_flag = TRUE;
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_WazaRecv );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
    work[0] = 0;
  }
}
// 無効化チェックハンドラ
static void handler_Hiraisin_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect(flowWk, pokeID, POKETYPE_DENKI) )
  {
    common_TypeNoEffect_Rankup( flowWk, pokeID, BPP_SP_ATTACK_RANK, 1 );
  }
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
static BOOL common_WazaTargetChangeToMe( BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work, PokeType wazaType )
{
  u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );

  if( atkPokeID != pokeID )
  {
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == wazaType )
    {
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( !BTL_SVFTOOL_CheckFarPoke(flowWk, atkPokeID, pokeID, waza) )
      {
        if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_POKEID_DEF, pokeID) )
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
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
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_FAIL_FLAG, TRUE) )
    {
      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        param->header.tokwin_flag = TRUE;
        HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Kyuuban );
        HANDEX_STR_AddArg( &param->str, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kyuuban( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CHECK_PUSHOUT,    handler_Kyuuban   },  // ふきとばしチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_DAMAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_DAMAGE, pokeID );
        param->pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
        param->damage = damage;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_HedoroEki );
        HANDEX_STR_AddArg( &param->exStr, param->pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// 死亡通知ハンドラ
static void handler_HedoroEki_Dead( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENT_FACTOR_ConvertForIsolate( myHandle );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HedoroEki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_DRAIN_END,    handler_HedoroEki      },  // ドレイン量計算ハンドラ
    { BTL_EVENT_NOTIFY_DEAD,       handler_HedoroEki_Dead },  // 死亡通知ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ぶきよう」
 */
//------------------------------------------------------------------------------
static BOOL handler_Bukiyou_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
  if( BPP_CheckSick(bpp, WAZASICK_IEKI) ){
    return FALSE;
  }

  if( factorType == BTL_EVENT_FACTOR_ITEM )
  {
    if( BTL_EVENT_FACTOR_GetPokeID(myHandle) == pokeID )
    {
      // これらのアイテムは無効化しない
      static const u16 IgnoreItems[] = {
        ITEM_KYOUSEIGIPUSU, ITEM_GAKUSYUUSOUTI, ITEM_OMAMORIKOBAN, ITEM_KIYOMENOOHUDA,
        ITEM_KAWARAZUNOISI, ITEM_SIAWASETAMAGO, ITEM_PAWAARISUTO,  ITEM_PAWAABERUTO,
        ITEM_PAWAARENZU,    ITEM_PAWAABANDO,    ITEM_PAWAAANKURU,  ITEM_PAWAAUEITO,
      };
      u32 i;
      for(i=0; i<NELEMS(IgnoreItems); ++i)
      {
        if( IgnoreItems[i] == subID ){
          return FALSE;
        }
      }

      return TRUE;
    }
  }

  return FALSE;
}
// メンバー入場ハンドラ
static void handler_Bukiyou_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( work[0] == 0 ){
      BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Bukiyou_SkipCheck );
      work[0] = 1;
    }
  }
}
// とくせい書き換え直前ハンドラ
static void handler_Bukiyou_PreChange( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
  }
}
// いえき確定ハンドラ
static void handler_Bukiyou_IekiFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
//  BTL_EVENT_FACTOR_DettachSkipCheckHandler( myHandle );
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetItem(bpp) != ITEM_DUMMY_DATA )
    {
      BTL_HANDEX_PARAM_CHECK_ITEM_EQUIP* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHECK_ITEM_EQUIP, pokeID );
        param->pokeID = pokeID;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}

// ワザ出し成否チェックハンドラ
static void handler_Bukiyou_ExeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );

    work[2] = 0;

    if( (waza == WAZANO_SIZENNOMEGUMI)
    &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
    ){
      work[2] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_CAUSE, SV_WAZAFAIL_TOKUSEI );
    }
  }
}
// ワザ出し失敗確定ハンドラ
static void handler_Bukiyou_ExeFail( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( work[2] )
    {
      u8  targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

      BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
        param->header.tokwin_flag = TRUE;
        HANDEX_STR_Setup( &(param->str), BTL_STRTYPE_SET, BTL_STRID_SET_WazaCantUse );
        HANDEX_STR_AddArg( &(param->str), pokeID );
        HANDEX_STR_AddArg( &(param->str), BTL_EVENTVAR_GetValue(BTL_EVAR_WAZAID) );
      BTL_SVF_HANDEX_Pop( flowWk, param );

      work[2] = 0;
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bukiyou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,                handler_Bukiyou_MemberIn  }, // メンバー入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,     handler_Bukiyou_MemberIn  }, // とくせい書き換えハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE,    handler_Bukiyou_PreChange }, // とくせい書き換え直前ハンドラ
    { BTL_EVENT_IEKI_FIXED,               handler_Bukiyou_IekiFixed }, // いえき確定ハンドラ
    { BTL_EVENT_WAZA_EXECUTE_CHECK_2ND,   handler_Bukiyou_ExeCheck  }, // ワザ出し成否チェックハンドラ
    { BTL_EVENT_WAZA_EXECUTE_FAIL,        handler_Bukiyou_ExeFail   }, // ワザ出し失敗確定ハンドラ
  };

  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
      if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
      {
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
            HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_NoEffect );
            HANDEX_STR_AddArg( &param->str, pokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
        BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
      }
    }
  }
}
static void handler_Nenchaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_ITEM) == ITEM_DUMMY_DATA)
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Nenchaku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L3,    handler_Nenchaku_NoEff },  // 無効化チェックLv2ハンドラ
    { BTL_EVENT_ITEMSET_CHECK,        handler_Nenchaku   },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「プレッシャー」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Pressure( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,            handler_Pressure_MemberIN },  // メンバー入場ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_AFTER, handler_Pressure_MemberIN },  // とくせい書き換えハンドラ
    { BTL_EVENT_DECREMENT_PP,         handler_Pressure          },  // PP消費チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// メンバー入場ハンドラ
static void handler_Pressure_MemberIN( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
       BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
         HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_Pressure );
         HANDEX_STR_AddArg( &param->str, pokeID );
       BTL_SVF_HANDEX_Pop( flowWk, param );
       BTL_N_PrintfEx( PRINT_CHANNEL_PRESSURE, DBGSTR_HANDTOK_PressureIn, pokeID, myHandle );
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
  }
}
// PP消費チェックハンドラ
static void handler_Pressure( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );

  if( !BTL_MAINUTIL_IsFriendPokeID(atkPokeID, pokeID) )
  {
    BOOL fEnable = FALSE;

    if( HandCommon_CheckTargetPokeID(pokeID) ){
      fEnable = TRUE;
    }
    else{

      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetCategory(waza) == WAZADATA_CATEGORY_FIELD_EFFECT ){
        fEnable = TRUE;
      }
      else if( BTL_TABLES_IsPressureEffectiveWaza(waza) ){
        fEnable = TRUE;
      }
    }

    if( fEnable ){
      u8 vol = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME ) + 1;
      BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, vol );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マジックガード」
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagicGuard( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_SIMPLE_DAMAGE_ENABLE,  handler_MagicGuard },  // ワザ以外ダメージチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ワザ以外ダメージチェックハンドラ
static void handler_MagicGuard( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_GEN_FLAG, FALSE );
  }
}
//------------------------------------------------------------------------------
/**
 * とくせい「あくしゅう」
 *
 * ひるみ確率０の打撃ワザを、確率10％に押し上げる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Akusyuu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_SHRINK_PER,  handler_Akusyuu }, // ひるみ確率チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ひるみ確率チェックハンドラ
static void handler_Akusyuu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // ひるみ確率０なら、確率10％に
    u8 per = BTL_EVENTVAR_GetValue( BTL_EVAR_ADD_PER );
    if( per == 0 )
    {
      #ifdef PM_DEBUG
      if( BTL_SVFTOOL_GetDebugFlag(flowWk, BTL_DEBUGFLAG_MUST_TOKUSEI) ){
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_ADD_PER, 100 );
        return;
      }
      #endif

      BTL_EVENTVAR_RewriteValue( BTL_EVAR_ADD_PER, 10 );
    }
  }
}
//------------------------------------------------------------------------------
/**
 * とくせい「かげふみ」
 *
 * 相手が逃げるのを封じる
 */
//------------------------------------------------------------------------------
static const BtlEventHandlerTable*  HAND_TOK_ADD_Kagefumi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NIGERU_FORBID,  handler_Kagefumi }, // 逃げる禁止チェック
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 逃げる禁止チェックハンドラ
static void handler_Kagefumi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 escPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

  if( !BTL_MAINUTIL_IsFriendPokeID(escPokeID, pokeID) )
  {
    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
    BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );
    u8 pokeIDAry[ BTL_POSIDX_MAX ];
    u8 cnt, i;

    // 相手に１体でも「かげふみ」がいるなら無効
    cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, pokeIDAry );
    for(i=0; i<cnt; ++i)
    {
      const BTL_POKEPARAM* bppEnemy = BTL_SVFTOOL_GetPokeParam( flowWk, pokeIDAry[i] );
      if( BPP_GetValue(bppEnemy, BPP_TOKUSEI_EFFECTIVE) == POKETOKUSEI_KAGEFUMI ){
        return;
      }
    }

    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
//------------------------------------------------------------------------------
/**
 * とくせい「ありじごく」
 *
 * 相手が逃げるのを封じる（ふゆうしているポケモンは除外）
 */
//------------------------------------------------------------------------------
static const BtlEventHandlerTable*  HAND_TOK_ADD_Arijigoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NIGERU_FORBID,  handler_Arijigoku }, // 逃げる禁止チェック
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 逃げる禁止チェックハンドラ
static void handler_Arijigoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 escPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

  if( !BTL_MAINUTIL_IsFriendPokeID(escPokeID, pokeID) )
  {
    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
    BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );
    u8 pokeIDAry[ BTL_POSIDX_MAX ];
    u8 cnt, i;

    // 相手に１体でもふゆう状態でないポケがいれば有効
    cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, pokeIDAry );
    for(i=0; i<cnt; ++i)
    {
      if( !BTL_SVFTOOL_IsFloatingPoke(flowWk, pokeIDAry[i]) )
      {
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
        break;;
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 * とくせい「じりょく」
 *
 * 相手が逃げるのを封じる（ふゆうしているポケモンは除外）
 */
//------------------------------------------------------------------------------
static const BtlEventHandlerTable*  HAND_TOK_ADD_Jiryoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NIGERU_FORBID,  handler_Jiryoku }, // 逃げる禁止チェック
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// 逃げる禁止チェックハンドラ
static void handler_Jiryoku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 escPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID );

  if( !BTL_MAINUTIL_IsFriendPokeID(escPokeID, pokeID) )
  {
    BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
    BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_ENEMY, myPos );
    u8 pokeIDAry[ BTL_POSIDX_MAX ];
    u8 cnt, i;


    // 相手に１体でも「はがね」タイプがいたら有効
    cnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, pokeIDAry );
    for(i=0; i<cnt; ++i)
    {
      const BTL_POKEPARAM* bppEnemy = BTL_SVFTOOL_GetPokeParam( flowWk, pokeIDAry[i] );
      if( BPP_IsMatchType(bppEnemy, POKETYPE_HAGANE) )
      {
        BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
      }
    }
  }
}




//------------------------------------------------------------------------------
/**
 * とくせい「かるわざ」
 *
 * 持っている道具がなくなると、身軽になり素早さの元の値を２倍にする。
 * 道具をまた持つと下がる。
 * 最初から道具がない場合は発動しない。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Karuwaza( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ITEMSET_DECIDE,  handler_Karuwaza_BeforeItemSet  }, // アイテム書き換え直前
    { BTL_EVENT_CALC_AGILITY,    handler_Karuwaza_Agility        }, // すばやさ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// アイテム書き換え直前ハンドラ
static void handler_Karuwaza_BeforeItemSet( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_ITEM) == ITEM_DUMMY_DATA)
  ){
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetItem(bpp) != ITEM_DUMMY_DATA )
    {
      work[0] = 1;    // アイテム消費したらフラグON
    }
  }
}
// すばやさ計算ハンドラ
static void handler_Karuwaza_Agility( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // フラグON でアイテム所持していない状態ならすばやさ２倍
    if( work[0] == 1 )
    {
      const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
      if( BPP_GetItem(bpp) == ITEM_DUMMY_DATA )
      {
        BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * とくせい「ものひろい」
 *
 * 戦闘中、相手が使い切ったものをひろってくる。（ターンエンドに）
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Monohiroi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,    handler_Monohiroi },  // ターンチェック開始ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ターンチェック終了ハンドラ
static void handler_Monohiroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bppMe = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetItem(bppMe) == ITEM_DUMMY_DATA )
    {
      typedef struct {
        u8 pokeID[ BTL_POS_MAX ];
        u8 consumedPokeID[ BTL_POS_MAX ];
        u8 pokeCnt;
        u8 consumedPokeCnt;
      }MONOHIROI_WORK;

      BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
      BtlExPos   expos = EXPOS_MAKE( BTL_EXPOS_AREA_OTHERS, myPos );

      MONOHIROI_WORK* mwk = BTL_SVFTOOL_GetTmpWork( flowWk, sizeof(MONOHIROI_WORK) );
      const BTL_POKEPARAM* bpp;
      u8 i, cnt;

      mwk->pokeCnt = BTL_SVFTOOL_ExpandPokeID( flowWk, expos, mwk->pokeID );
      mwk->consumedPokeCnt = 0;

      // 今ターン、アイテムを消費したポケモンを記録
      cnt = 0;
      for(i=0; i<mwk->pokeCnt; ++i)
      {
        bpp = BTL_SVFTOOL_GetPokeParam( flowWk, mwk->pokeID[i] );
        if( (BPP_TURNFLAG_Get(bpp, BPP_TURNFLG_ITEM_CONSUMED))
        &&  (BPP_GetConsumedItem(bpp) != ITEM_DUMMY_DATA)
        ){
          mwk->consumedPokeID[cnt++] = BPP_GetID( bpp );
        }
      }

      if( cnt )
      {
        u8 rndIdx = BTL_CALC_GetRand( cnt );
        u8 targetPokeID = mwk->consumedPokeID[rndIdx];
        u16 itemID;

        bpp = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );
        itemID = BPP_GetConsumedItem( bpp );

        if( itemID != ITEM_DUMMY_DATA )
        {
          BTL_HANDEX_PARAM_SET_ITEM* setParam;

          setParam = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SET_ITEM, pokeID );
            setParam->header.tokwin_flag = TRUE;
            setParam->pokeID = pokeID;
            setParam->itemID = itemID;
            setParam->fClearConsumeOtherPoke = TRUE;
            setParam->clearConsumePokeID = targetPokeID;
            HANDEX_STR_Setup( &setParam->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Monohiroi );
            HANDEX_STR_AddArg( &setParam->exStr, pokeID );
            HANDEX_STR_AddArg( &setParam->exStr, itemID );
          BTL_SVF_HANDEX_Pop( flowWk, setParam );
        }
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * とくせい「わるいてぐせ」
 *
 * 打撃攻撃（接触フラグONのダメージワザ）を受けた際、
 * 自分の道具に空きがあれば相手の持ち物を奪い取る。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_WaruiTeguse( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DAMAGEPROC_END,        handler_WaruiTeguse },  // ダメージワザ処理終了処理ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ダメージワザ処理終了処理ハンドラ
static void handler_WaruiTeguse( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( HandCommon_CheckTargetPokeID(pokeID) )
  {
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    if( !HandCommon_CheckCantStealPoke(flowWk, pokeID, targetPokeID) )
    {
      WazaID  waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
      {
        const BTL_POKEPARAM* me = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
        const BTL_POKEPARAM* attacker = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );

        if( (BPP_GetItem(me) == ITEM_DUMMY_DATA)
        &&  (BPP_GetItem(attacker) != ITEM_DUMMY_DATA)
        ){
          BTL_HANDEX_PARAM_SWAP_ITEM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SWAP_ITEM, pokeID );
            param->header.tokwin_flag = TRUE;
            param->pokeID = targetPokeID;
            HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_WaruiTeguse );
            HANDEX_STR_AddArg( &param->exStr, targetPokeID );
            HANDEX_STR_AddArg( &param->exStr, BPP_GetItem(attacker) );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 * とくせい「のろわれボディ」
 *
 * ３０％で受けた技をかなしばり状態にする。
 */
//------------------------------------------------------------------------------
// ダメージ反応ハンドラ
static void handler_NorowareBody( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  &&  (!BTL_SVFTOOL_CheckShowDown(flowWk))
  ){
    u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    const BTL_POKEPARAM* target = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );

    if( !BPP_CheckSick(target, WAZASICK_KANASIBARI) )
    {
      WazaID  prevWaza = BTL_EVENTVAR_GetValue( BTL_EVAR_ORG_WAZAID );
      if( (prevWaza != WAZANO_NULL)
      &&  (prevWaza != WAZANO_WARUAGAKI)
      &&  (!BTL_TABLES_IsDelayAttackWaza(prevWaza))
      ){
        if( Tokusei_IsExePer(flowWk, 30) )
        {
          BTL_HANDEX_PARAM_ADD_SICK* param;

          // 現ターンに行動済みのポケモンには効果ターン数を+1する
          u8 turns = BTL_KANASIBARI_EFFECTIVE_TURN;
          if( BPP_TURNFLAG_Get(target, BPP_TURNFLG_ACTION_DONE) ){
            ++turns;
          }

          param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
            param->sickID = WAZASICK_KANASIBARI;
            param->sickCont = BPP_SICKCONT_MakeTurnParam( turns, prevWaza );
            param->pokeID = targetPokeID;
            param->header.tokwin_flag = TRUE;
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_NorowareBody( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,    handler_NorowareBody },  // ダメージ反応ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「くだけるよろい」
 *
 * 物理攻撃を受けると防御が１段階下がり、素早さが１段階上がる。
 */
//------------------------------------------------------------------------------
static void handler_KudakeruYoroi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE_TYPE) == WAZADATA_DMG_PHYSIC)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    BOOL fEffective = FALSE;

    if( BPP_IsRankEffectValid(bpp, BPP_DEFENCE_RANK, -1)
    ||  BPP_IsRankEffectValid(bpp, BPP_AGILITY_RANK,  1)
    ){
      fEffective = TRUE;
    }

    if( fEffective && (!BPP_IsDead(bpp)) )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param;

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->fAlmost = TRUE;
          param->rankType = BPP_DEFENCE_RANK;
          param->rankVolume = -1;
        BTL_SVF_HANDEX_Pop( flowWk, param );

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->fAlmost = TRUE;
          param->rankType = BPP_AGILITY_RANK;
          param->rankVolume = 1;
        BTL_SVF_HANDEX_Pop( flowWk, param );

      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_KudakeruYoroi( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,    handler_KudakeruYoroi },  // ダメージ反応ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ちからずく」
 *
 * 追加効果のある技の追加効果を出すことができないが、かわりに威力が1.3倍になる。
 * 追加効果が自分自身の能力を下げるワザの場合には、このとくせい自体が無効
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
// ひるみ追加チェックハンドラ
static void handler_Tikarazuku_ShrinkCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Tikarazuku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Tikarazuku_WazaPow   }, // ワザ威力計算ハンドラ
    { BTL_EVENT_ADD_SICK,        handler_Tikarazuku_CheckFail }, // 追加効果（状態異常）チェックハンドラ
    { BTL_EVENT_ADD_RANK_TARGET, handler_Tikarazuku_CheckFail }, // 追加効果（ランク効果）チェックハンドラ
    { BTL_EVENT_WAZA_SHRINK_PER, handler_Tikarazuku_ShrinkCheck },  // 追加効果（ひるみ）チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

/**
 *  「ちからずく」が効くワザかどうか判定
 */
static BOOL IsTikarazukuEffecive( WazaID waza )
{
  WazaCategory category = WAZADATA_GetCategory( waza );

  // ひるみ確率のあるワザは
  if( WAZADATA_GetParam(waza, WAZAPARAM_SHRINK_PER) != 0 ){
    return TRUE;
  }

  switch( category ){
  // 殴った相手に効果が発動するタイプは基本的に有効
  case WAZADATA_CATEGORY_DAMAGE_SICK:
    {
      // バインド系のみ無効
      WazaSick sickID = WAZADATA_GetParam( waza, WAZAPARAM_SICK );
      if( sickID == WAZASICK_BIND ){
        return FALSE;
      }
    }
    /* fallthru */
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
 *
 * 相手から能力を下げられる技や特性を受けた時、攻撃力が２段階上がる。
 */
//------------------------------------------------------------------------------
static void handler_Makenki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    if( !BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) )
    {
      int volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
      if( volume < 0 )
      {
        BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );

          param->header.tokwin_flag = TRUE;
          param->rankType = BPP_ATTACK_RANK;
          param->rankVolume = 2;
          param->fAlmost = TRUE;
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;

        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Makenki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_FIXED,    handler_Makenki },  // ランク効果確定ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「よわき」
 *
 * ＨＰが半分以下になると、攻撃、特攻が半減する。
 */
//------------------------------------------------------------------------------
// 攻撃側能力の計算ハンドラ
static void handler_Yowaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    u32 hp = BPP_GetValue( bpp, BPP_HP );
    u32 hp_half = BTL_CALC_QuotMaxHP_Zero( bpp, 2 );
    if( hp <= hp_half )
    {
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5f) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Yowaki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_ATTACKER_POWER,    handler_Yowaki },  // 攻撃側能力の計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マルチスケイル」
 *
 * ＨＰが満タンの時に受けるダメージを半減させる。
 */
//------------------------------------------------------------------------------
// ダメージ計算最終チェックハンドラ
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MultiScale( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,    handler_MultiScale },  // ダメージ計算最終チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「フレンドガード」
 *  （旧名 なかまいしき）
 *
 * 自分以外の見方へのダメージを1/2にする。
 */
//------------------------------------------------------------------------------
// ダメージ計算最終チェックハンドラ
static void handler_NakamaIsiki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  u8 defPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );

  // 自分以外の味方へのダメージ半減
  if( (defPokeID != pokeID)
  &&  BTL_MAINUTIL_IsFriendPokeID(pokeID, defPokeID)
  ){
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.75f) );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_FriendGuard( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_PROC2,    handler_NakamaIsiki },  // ダメージ計算最終チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「モラテラピー」
 *
 * 自分以外の味方のポケモン系状態異常が毎ターン30％の確率で回復する。
 */
//------------------------------------------------------------------------------
// ターンチェックハンドラ
static void handler_Moraterapi( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
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
            param->header.autoRemoveFlag = TRUE;
            param->poke_cnt = 1;
            param->pokeID[0] = targetID[i];
            param->sickCode = WAZASICK_EX_POKEFULL;
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_IyasiNoKokoro( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_BEGIN,    handler_Moraterapi },  // ターンチェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どくぼうそう」
 *
 * 毒状態の時、物理攻撃の威力が倍になる。
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.5) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dokubousou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Dokubousou   }, // ワザ威力計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ねつぼうそう」
 *
 * やけど状態の時、特殊攻撃の威力が倍になる。
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
      BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.5) );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Netubousou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,      handler_Netubousou   }, // ワザ威力計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「テレパシー」
 *
 *  味方の攻撃を受けない。
 */
//------------------------------------------------------------------------------
// 無効化チェックLv2ハンドラ
static void handler_AunNoIki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  {
    // 攻撃側が味方ポケモンで
    u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
    if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID) && (pokeID != atkPokeID ) )
    {
      // ダメージワザなら無効化
      WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
      if( WAZADATA_IsDamage(waza) )
      {
        if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_NOEFFECT_FLAG, TRUE) )
        {
          BTL_HANDEX_PARAM_MESSAGE* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
            param->header.tokwin_flag = TRUE;
            HANDEX_STR_Setup( &param->str, BTL_STRTYPE_SET, BTL_STRID_SET_AunNoIki );
            HANDEX_STR_AddArg( &param->str, pokeID );
          BTL_SVF_HANDEX_Pop( flowWk, param );
        }
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Telepassy( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_NOEFFECT_CHECK_L3,        handler_AunNoIki }, // 無効化チェックLv2ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ムラっけ」
 *
 * 毎ターンエンドにどれかひとつの能力が一段階上昇し、ひとつの能力が一段階下降する。
 */
//------------------------------------------------------------------------------
// ターンチェック最終ハンドラ
static void handler_Murakke( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* sys_work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    enum {
      PATTERN_MAX = ((WAZA_RANKEFF_NUMS * WAZA_RANKEFF_NUMS) - WAZA_RANKEFF_NUMS) * 2,
    };

    // 重複しない「上昇・下降」の組み合わせを列挙するための計算用ワーク
    // ※必ず、異なる２つの能力ランクが「上昇」「下降」するようにする。
    // （全能力ランクが上がりきっている（下がりきっている）場合は除く）
    typedef struct {

      u16 patterns[ PATTERN_MAX ];
      u8  upEffects[ WAZA_RANKEFF_NUMS ];
      u8  downEffects[ WAZA_RANKEFF_NUMS ];

    }CALC_WORK;


    CALC_WORK* work = BTL_SVFTOOL_GetTmpWork( flowWk, sizeof(CALC_WORK) );
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    u8 upEffect, downEffect, upCnt, downCnt, ptnCnt;
    u32 i;

  //  BTL_Printf("pokeID=%d\n, bpp=%p\n", pokeID, bpp);

    // 上がりうる能力、下がりうる能力を列挙、カウントする
    upCnt = downCnt = 0;
    for(i=0; i<WAZA_RANKEFF_NUMS; ++i)
    {
      if( BPP_IsRankEffectValid(bpp, WAZA_RANKEFF_ORIGIN+i, 1) ){
        work->upEffects[ upCnt++ ] = WAZA_RANKEFF_ORIGIN+i;
      }
      if( BPP_IsRankEffectValid(bpp, WAZA_RANKEFF_ORIGIN+i, -1) ){
        work->downEffects[ downCnt++ ] = WAZA_RANKEFF_ORIGIN+i;
      }
    }

    // 上がりうる（下がりうる）能力が０の場合、ランダムで１つだけ上がる（下がる）
    upEffect = downEffect = WAZA_RANKEFF_NULL;
    if( (upCnt == 0) && (downCnt != 0) ){
      i = BTL_CALC_GetRand( downCnt );
      downEffect = work->downEffects[ i ];
    }
    else if( (upCnt != 0) && (downCnt == 0) ){
      i = BTL_CALC_GetRand( upCnt );
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
      // 組み合わせの中から１つをランダム決定
      i = BTL_CALC_GetRand( ptnCnt );
      upEffect = (work->patterns[i] >> 8) & 0xff;
      downEffect = work->patterns[i] & 0xff;
    }

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param;

      if( upEffect != BPP_VALUE_NULL )
      {
        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->rankType = upEffect;
          param->rankVolume = 2;
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
      if( downEffect != BPP_VALUE_NULL )
      {
        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
          param->poke_cnt = 1;
          param->pokeID[0] = pokeID;
          param->rankType = downEffect;
          param->rankVolume = -1;
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

  } // if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Murakke( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,        handler_Murakke },  // ターンチェック最終ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ぼうじん」
 *
 * 天候によるダメージを受けない。
 */
//------------------------------------------------------------------------------
// 天候ダメージ計算ハンドラ
static void handler_Boujin_CalcDamage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_DAMAGE) > 0)
  ){
    BTL_EVENTVAR_RewriteValue( BTL_EVAR_FAIL_FLAG, TRUE );
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Boujin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEATHER_REACTION,    handler_Boujin_CalcDamage   }, // 天候ダメージ計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「どくしゅ」
 *
 * 打撃技を相手に当てると３割の確率で相手を毒状態にする。
 */
//------------------------------------------------------------------------------
// ダメージ反応ハンドラ
static void handler_Dokusyu( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_RINPUNGUARD_FLG) == FALSE)
  ){
    // 接触ワザなら
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      // ３割の確率で相手をどくにする
      if( Tokusei_IsExePer(flowWk, 30) )
      {
        BTL_HANDEX_PARAM_ADD_SICK* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_ADD_SICK, pokeID );
          param->header.tokwin_flag = TRUE;
          param->pokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_DEF );
          param->sickID = WAZASICK_DOKU;
          param->sickCont = BTL_CALC_MakeDefaultPokeSickCont( WAZASICK_DOKU );
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Dokusyu );
          HANDEX_STR_AddArg( &param->exStr, param->pokeID );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Dokusyu( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,        handler_Dokusyu }, // ダメージ反応ハンドラ
  };
  *numElems = numHandlersWithHandlerPri( BTL_EVPRI_TOKUSEI_dokusyu, NELEMS(HandlerTable) );
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「さいせいりょく」
 *
 * 控えに下げるとＨＰが1/3回復する。
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
      BTL_HANDEX_PARAM_SHIFT_HP* param;
      u32 recoverHP = BTL_CALC_QuotMaxHP( bpp, 3 );
      u32 recoverLimit = BPP_GetValue(bpp, BPP_MAX_HP) - BPP_GetValue(bpp, BPP_HP);
      if( recoverHP > recoverLimit ){
        recoverHP = recoverLimit;
      }

      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SHIFT_HP, pokeID );
        param->pokeID[0] = pokeID;
        param->volume[0] = recoverHP;
        param->poke_cnt = 1;
        param->fEffectDisable = TRUE;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SaiseiRyoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_OUT_FIXED, handler_SaiseiRyoku },   // メンバー退場確定ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「はとむね」
 *
 * 相手に防御力を下げられない。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Hatomune( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKEFF_LAST_CHECK, handler_Hatomune_Check }, // ランクダウン成功チェック
    { BTL_EVENT_RANKEFF_FAILED,     handler_Hatomune_Guard },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
 *
 * 天候がすなあらし状態の時、素早さ２倍になる＆すなあらしの天候ダメージを受けない
 *
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sunakaki( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_CALC_AGILITY,     handler_Sunakaki           },    /// すばやさ計算ハンドラ
    { BTL_EVENT_WEATHER_REACTION, handler_Sunagakure_Weather }, // 天候リアクションハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

// すばやさ計算ハンドラ
static void handler_Sunakaki( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    // 天候がすなあらしなら素早さ２倍
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SAND ){
      BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ミラクルスキン」
 *
 * 非ダメージ技に対する回避率が高い。50％回避。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_MilacreSkin( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_MilacreSkin   },    /// ワザ命中率取得ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
 *  とくせい「アナライズ」
 *
 * ターン中、一番最後に技を出すと技の威力が1.3倍になる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Analyze( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER, handler_Sinuti   },    // ワザ威力計算ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ワザ威力計算ハンドラ
static void handler_Sinuti( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 順番最後なら威力増加
    if( HandCommon_IsPokeOrderLast(flowWk, pokeID) )
    {
        BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.3) );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「すなのちから」
 *
 * 天候がすなあらしの時、地面、岩、鋼ワザの威力が1.3倍になる
 * ＆すなあらしの天候ダメージを受けない
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SunanoTikara( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_POWER,       handler_SunanoTikara       }, // ワザ威力計算ハンドラ
    { BTL_EVENT_WEATHER_REACTION, handler_Sunagakure_Weather }, // 天候リアクションハンドラ

  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ワザ威力計算ハンドラ
static void handler_SunanoTikara( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が攻撃側で
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_ATK) == pokeID )
  {
    // 天候すなあらしなら威力増加
    if( BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SAND )
    {
      PokeType type = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZA_TYPE );
      if( (type == POKETYPE_JIMEN)
      ||  (type == POKETYPE_IWA)
      ||  (type == POKETYPE_HAGANE)
      ){
        BTL_EVENTVAR_MulValue( BTL_EVAR_WAZA_POWER_RATIO, FX32_CONST(1.3) );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ダルマモード」
 *
 * 【ヒヒダルマの場合のみ有効】
 *  ターンチェック終了時、HPが半分以下ならフォルム１に、HPが半分超ならデフォルトフォルムに変化する
 */
//------------------------------------------------------------------------------
// ターンチェック終了ハンドラ
static void handler_DarumaMode( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

  if( BPP_GetMonsNo(bpp) == MONSNO_HIHIDARUMA )
  {
    u32 hp, hp_border;
    u8  form_next;

    hp = BPP_GetValue( bpp, BPP_HP );
    hp_border = BTL_CALC_QuotMaxHP_Zero( bpp, 2 );

    form_next = (hp <= hp_border)? FORMNO_HIHIDARUMA_MEDITATION : FORMNO_HIHIDARUMA_ACTION;
    if( BPP_GetValue(bpp, BPP_FORM) != form_next )
    {
      BTL_HANDEX_PARAM_CHANGE_FORM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_FORM, pokeID );
        param->header.tokwin_flag = TRUE;
        param->pokeID = pokeID;
        param->formNo = form_next;
        {
          u16 strID = (form_next == FORMNO_HIHIDARUMA_MEDITATION)? BTL_STRID_STD_DarumaOn : BTL_STRID_STD_DarumaOff;
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_STD, strID );
        }
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
// いえき確定
static void handler_DarumaMode_IekiFixed( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );
    if( BPP_GetMonsNo(bpp) == MONSNO_HIHIDARUMA )
    {
      BTL_HANDEX_PARAM_CHANGE_FORM* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_FORM, pokeID );
        param->pokeID = pokeID;
        param->formNo = FORMNO_HIHIDARUMA_ACTION;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_STD, BTL_STRID_STD_DarumaOff );
        HANDEX_STR_AddArg( &param->exStr, pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}


static  const BtlEventHandlerTable*  HAND_TOK_ADD_DarumaMode( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,            handler_DarumaMode            },    // ターンチェック終了ハンドラ
    { BTL_EVENT_IEKI_FIXED,               handler_DarumaMode_IekiFixed  },
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE,    handler_DarumaMode_IekiFixed  },
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}




//------------------------------------------------------------------------------
/**
 *  とくせい「すりぬけ」
 *
 * リフレクター、ひかりのかべ、しんぴのまもり、しろいきりの効果を受けずに攻撃できる。
 */
//------------------------------------------------------------------------------
// 「すりぬけ」スキップチェックハンドラ
static BOOL handler_Surinuke_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( factorType == BTL_EVENT_FACTOR_SIDE )
  {
    if( (subID == BTL_SIDEEFF_REFRECTOR)        // リフレクター無効
    ||  (subID == BTL_SIDEEFF_HIKARINOKABE)     // ひかりのかべ無効
    ||  (subID == BTL_SIDEEFF_SINPINOMAMORI)    // しんぴのまもり無効
    ||  (subID == BTL_SIDEEFF_SIROIKIRI)        // しろいきり無効
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Surinuke( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZASEQ_START,      handler_Surinuke_Start },  // ワザ処理開始
    { BTL_EVENT_WAZASEQ_END,        handler_Surinuke_End   },  // ワザ処理終了
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「じしんかじょう」
 *
 * 相手を倒すと攻撃力が１段階上がる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_JisinKajou( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_DAMAGEPROC_END_HIT_REAL, handler_JisinKajou   },    // ワザダメージプロセス最終ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
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
          param->rankType = BPP_ATTACK_RANK;
          param->rankVolume = 1;
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「せいぎのこころ」
 *
 * 悪タイプのダメージ技を受けると攻撃力が１段階上がる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_SeiginoKokoro( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_SeiginoKokoro   },    // ダメージ反応ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ダメージ反応ハンドラ
static void handler_SeiginoKokoro( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
    // 受けたワザタイプが悪なら攻撃ランクアップ
    if( BTL_EVENTVAR_GetValue(BTL_EVAR_WAZA_TYPE) == POKETYPE_AKU )
    {
      BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
        param->header.tokwin_flag = TRUE;
        param->poke_cnt = 1;
        param->pokeID[0] = pokeID;
        param->rankType = BPP_ATTACK_RANK;
        param->rankVolume = 1;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「びびり」
 *
 * ゴースト、悪、虫タイプの技を受けると素早さが１段階上がる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Bibiri( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_Bibiri   },    // ダメージ反応ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ダメージ反応ハンドラ
static void handler_Bibiri( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
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
        param->rankType = BPP_AGILITY_RANK;
        param->rankVolume = 1;
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ミイラ」
 *
 * 打撃技を受けた相手の特性を「ミイラ」にしてしまう。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Miira( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION, handler_Miira   },    // ダメージ反応ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ダメージ反応ハンドラ
static void handler_Miira( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側で
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID )
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  &&  (!BTL_SVFTOOL_CheckShowDown(flowWk))
  ){
    // 食らったのが接触ワザで
    WazaID waza = BTL_EVENTVAR_GetValue( BTL_EVAR_WAZAID );
    if( WAZADATA_GetFlag(waza, WAZAFLAG_Touch) )
    {
      // 相手のとくせいが「ミイラ」以外なら「ミイラ」にする
      u8 targetPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
      const BTL_POKEPARAM* bppTarget = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID );
      if( BPP_GetValue(bppTarget, BPP_TOKUSEI) != POKETOKUSEI_MIIRA )
      {
        BTL_HANDEX_PARAM_CHANGE_TOKUSEI* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHANGE_TOKUSEI, pokeID );
          param->tokuseiID = POKETOKUSEI_MIIRA;
          param->pokeID = targetPokeID;
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Miira );
          HANDEX_STR_AddArg( &param->exStr, param->pokeID );
          if( !BTL_MAINUTIL_IsFriendPokeID(pokeID, targetPokeID) ){
            param->header.tokwin_flag = TRUE;
          }
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「そうしょく」
 *
 * 草タイプの技を受けるとダメージを受けずに特攻が１段階上がる。
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
  BTL_HANDEX_PARAM_RANK_EFFECT* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_RANK_EFFECT, pokeID );
    param->poke_cnt = 1;
    param->pokeID[0] = pokeID;
    param->rankType = BPP_ATTACK_RANK;
    param->rankVolume = 1;
  BTL_SVF_HANDEX_Pop( flowWk, param );
}
// 無効化チェックハンドラ
static void handler_Sousyoku_CheckNoEffect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( common_TypeNoEffect(flowWk, pokeID, POKETYPE_KUSA) )
  {
    common_TypeNoEffect_Rankup( flowWk, pokeID, BPP_ATTACK_RANK, 1 );
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Sousyoku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
//    { BTL_EVENT_DMG_TO_RECOVER_CHECK,   handler_Sousyoku_Check }, // ダメージワザ回復チェックハンドラ
//    { BTL_EVENT_DMG_TO_RECOVER_FIX,     handler_Sousyoku_Fix   }, // ダメージワザ回復化決定ハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,     handler_Sousyoku_CheckNoEffect }, // 無効化チェックハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「いたずらごころ」
 *
 * 自分が非ダメージワザを出す際、ワザ先制プライオリティを+1できる。（同ワザ同士なら先制できる）
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
static  const BtlEventHandlerTable*  HAND_TOK_ADD_ItazuraGokoro( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_GET_WAZA_PRI,   handler_ItazuraGokoro }, // ワザプライオリティ取得ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「マジックミラー」
 *
 * ワザ“マジックコート”で跳ね返せる技を跳ね返す。
 */
//------------------------------------------------------------------------------
// ワザ出し成否チェックハンドラ
static void handler_MagicMirror_ExeCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  HandCommon_MagicCoat_CheckSideEffWaza( myHandle, flowWk, pokeID, work );
}
// 無効化チェックハンドラ
static void handler_MagicMirror_Wait( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  HandCommon_MagicCoat_Wait( myHandle, flowWk, pokeID, work );
}
// わざ跳ね返し確定ハンドラ
static void handler_MagicMirror_Reflect( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    if( BTL_EVENTVAR_RewriteValue(BTL_EVAR_GEN_FLAG, TRUE) )
    {
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
      HandCommon_MagicCoat_Reaction( myHandle, flowWk, pokeID, work );
      BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_MagicMirror( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_EXECUTE_CHECK_2ND,  handler_MagicMirror_ExeCheck  }, // ワザ出し成否チェックハンドラ
    { BTL_EVENT_NOEFFECT_CHECK_L3,       handler_MagicMirror_Wait      }, // わざ乗っ取り判定ハンドラ
    { BTL_EVENT_WAZASEQ_REFRECT,         handler_MagicMirror_Reflect   }, // わざ跳ね返し確定
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しゅうかく」
 *
 * きのみを使ってもターンエンドに復活する。
 */
//------------------------------------------------------------------------------
// ターンチェック終了ハンドラ
static void handler_Syuukaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    const BTL_POKEPARAM* bpp = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

    u16 usedItem = BPP_GetConsumedItem( bpp );
    if( (usedItem != ITEM_DUMMY_DATA)
    &&  (ITEM_CheckNuts(usedItem))
    &&  (BPP_GetItem(bpp) == ITEM_DUMMY_DATA)
    ){
      if( (BTL_SVFTOOL_GetWeather(flowWk) == BTL_WEATHER_SHINE)
      ||  (Tokusei_IsExePer(flowWk, 50))
      ){
        BTL_HANDEX_PARAM_SET_ITEM* param;

        param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_SET_ITEM, pokeID );
          param->header.tokwin_flag = TRUE;
          param->itemID = usedItem;
          param->pokeID = pokeID;
          param->fClearConsume = TRUE;
          HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Syuukaku );
          HANDEX_STR_AddArg( &param->exStr, pokeID );
          HANDEX_STR_AddArg( &param->exStr, usedItem );
        BTL_SVF_HANDEX_Pop( flowWk, param );
      }
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Syuukaku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_TURNCHECK_END,        handler_Syuukaku },  // ターンチェック終了ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「ヘヴィメタル」
 *
 *  体重２倍にして計算
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_HeavyMetal( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEIGHT_RATIO,        handler_HeavyMetal },  // ランク増減値修正ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
static void handler_HeavyMetal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(2) );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「ライトメタル」
 *
 *  体重1/2にして計算
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_LightMetal( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WEIGHT_RATIO,        handler_LightMetal },  // ランク増減値修正ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
static void handler_LightMetal( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(0.5) );
  }
}

//------------------------------------------------------------------------------
/**
 *  とくせい「あまのじゃく」
 *
 *  能力ランク効果が逆転して働く。（ex. "いかく"を受けると攻撃力が上がる）
 */
//------------------------------------------------------------------------------
// ランク増減値修正ハンドラ
static void handler_Amanojaku( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    int volume = BTL_EVENTVAR_GetValue( BTL_EVAR_VOLUME );
    {
      volume *= -1;
      work[0] = BTL_EVENTVAR_RewriteValue( BTL_EVAR_VOLUME, volume );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Amanojaku( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_RANKVALUE_CHANGE,        handler_Amanojaku },  // ランク増減値修正ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}

//------------------------------------------------------------------------------
/**
 *  とくせい「きんちょうかん」
 *
 * 相手が緊張してきのみが食べられない。
 */
//------------------------------------------------------------------------------
// メンバー入場
static void handler_Kinchoukan_MemberIn( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 「きんちょうかん」持ちであることを告知
  if( BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID) == pokeID )
  {
    BTL_HANDEX_PARAM_MESSAGE* param;
    BtlSide side = BTL_MAINUTIL_GetOpponentSide( BTL_MAINUTIL_PokeIDtoSide(pokeID) );

    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_IN, pokeID );
    param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_MESSAGE, pokeID );
      HANDEX_STR_Setup( &param->str, BTL_STRTYPE_STD, BTL_STRID_STD_Kinchoukan );
      HANDEX_STR_AddArg( &param->str, side );
    BTL_SVF_HANDEX_Pop( flowWk, param );
    BTL_SVF_HANDEX_PushRun( flowWk, BTL_HANDEX_TOKWIN_OUT, pokeID );

    BTL_EVENT_FACTOR_AttachSkipCheckHandler( myHandle, handler_Kinchoukan_SkipCheck );

  }
}
// スキップチェックハンドラ
static BOOL handler_Kinchoukan_SkipCheck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, BtlEventFactorType factorType, BtlEventType eventType, u16 subID, u8 pokeID )
{
  if( BTL_EVENT_FACTOR_GetWorkValue(myHandle, 0) == 0 )
  {
    if( factorType == BTL_EVENT_FACTOR_ITEM )
    {
      u8 my_pokeID = BTL_EVENT_FACTOR_GetPokeID( myHandle );
      if( !BTL_MAINUTIL_IsFriendPokeID(my_pokeID, pokeID) )
      {
        if( ITEM_CheckNuts(subID) )
        {
          return TRUE;
        }
      }
    }
  }
  return FALSE;
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
// とくせい変更直前ハンドラ
static void handler_Kinchoukan_ChangeTok( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI_NEXT) != BTL_EVENT_FACTOR_GetSubID(myHandle))
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

  BtlPokePos myPos = BTL_SVFTOOL_GetExistFrontPokePos( flowWk, pokeID );
  BtlExPos   exPos = EXPOS_MAKE( BTL_EXPOS_FULL_ENEMY, myPos );

  // work[0] に値を入れて装備アイテム使用チェック機能をオフ
  work[0] = 1;

  targetCnt = BTL_SVFTOOL_ExpandPokeID( flowWk, exPos, targetPokeID );
  for(i=0; i<targetCnt; ++i)
  {
    bpp = BTL_SVFTOOL_GetPokeParam( flowWk, targetPokeID[i] );
    if( !BPP_IsDead(bpp) )
    {
      BTL_HANDEX_PARAM_CHECK_ITEM_EQUIP* param;
      param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_CHECK_ITEM_EQUIP, pokeID );
        param->pokeID = targetPokeID[ i ];
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}

static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kinchoukan( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,             handler_Kinchoukan_MemberIn       }, // メンバー入場ハンドラ
//    { BTL_EVENT_CHECK_ITEMEQUIP_FAIL,  handler_Kinchoukan_CheckItemEquip }, // 装備アイテム使用チェックハンドラ
//    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,  handler_Kinchoukan_MemberIn       }, // とくせい書き換えハンドラ
//    { BTL_EVENT_MEMBER_OUT_FIXED,      handler_Kinchoukan_MemberOutFixed }, // メンバー退場確定ハンドラ
//    { BTL_EVENT_IEKI_FIXED,            handler_Kinchoukan_Ieki           }, // いえき確定ハンドラ
//    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Kinchoukan_ChangeTok      }, // とくせい変換直前
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「へんしん」
 *
 * 場に出た時に正面の相手に変身する。目の前に対象がいない場合はへんしんしない。
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
    const BTL_POKEPARAM* self = BTL_SVFTOOL_GetPokeParam( flowWk, pokeID );

    if( (!BPP_IsDead(self)) && (!BPP_IsDead(target)) )
    {
      BTL_HANDEX_PARAM_HENSIN* param = BTL_SVF_HANDEX_Push( flowWk, BTL_HANDEX_HENSIN, pokeID );
        param->header.tokwin_flag = TRUE;
        param->pokeID = tgtPokeID;
        HANDEX_STR_Setup( &param->exStr, BTL_STRTYPE_SET, BTL_STRID_SET_Hensin );
        HANDEX_STR_AddArg( &param->exStr, pokeID );
        HANDEX_STR_AddArg( &param->exStr, param->pokeID );
      BTL_SVF_HANDEX_Pop( flowWk, param );
    }
  }
}
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Kawarimono( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_MEMBER_IN,              handler_Hensin }, // 入場ハンドラ
//    { BTL_EVENT_CHANGE_TOKUSEI_AFTER,   handler_Hensin }, // とくせい書き換えハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
//------------------------------------------------------------------------------
/**
 *  とくせい「イリュージョン」
 *
 * リストの並び順で後ろのポケモンと同じ姿・名前で戦闘に出せる。ダメージを受けると元に戻る。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_Illusion( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_DMG_REACTION,     handler_Illusion_Damage    },   // ダメージ反応ハンドラ
    { BTL_EVENT_IEKI_FIXED,            handler_Illusion_Ieki      },   // いえき確定ハンドラ
    { BTL_EVENT_CHANGE_TOKUSEI_BEFORE, handler_Illusion_ChangeTok },   // とくせい変換直前
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
// ダメージ反応ハンドラ
static void handler_Illusion_Damage( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分が防御側ならイリュージョン解除
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_POKEID_DEF) == pokeID)
  &&  (BTL_EVENTVAR_GetValue(BTL_EVAR_MIGAWARI_FLAG) == FALSE)
  ){
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
  if( (BTL_EVENTVAR_GetValue(BTL_EVAR_TOKUSEI_NEXT) != BTL_EVENT_FACTOR_GetSubID(myHandle))
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
    BTL_SVF_HANDEX_Pop( flowWk, param );
  }
}
//------------------------------------------------------------------------------
/**
 *  とくせい「しょうりのほし」
 *
 * 自分を含め、味方側の命中率が1.1倍になる。
 */
//------------------------------------------------------------------------------
static  const BtlEventHandlerTable*  HAND_TOK_ADD_GoodLuck( u32* numElems )
{
  static const BtlEventHandlerTable HandlerTable[] = {
    { BTL_EVENT_WAZA_HIT_RATIO, handler_GoodLuck   },       /// ワザ命中率取得ハンドラ
  };
  *numElems = NELEMS(HandlerTable);
  return HandlerTable;
}
/// ワザ命中率取得ハンドラ
static void handler_GoodLuck( BTL_EVENT_FACTOR* myHandle, BTL_SVFLOW_WORK* flowWk, u8 pokeID, int* work )
{
  // 自分の味方が攻撃する場合 命中率 x1.1
  u8 atkPokeID = BTL_EVENTVAR_GetValue( BTL_EVAR_POKEID_ATK );
  if( BTL_MAINUTIL_IsFriendPokeID(pokeID, atkPokeID ) )
  {
    BTL_EVENTVAR_MulValue( BTL_EVAR_RATIO, FX32_CONST(1.1) );
  }
}

