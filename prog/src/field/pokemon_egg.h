//////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief  ポケモンタマゴ
 * @file   pokemon_egg.h
 * @author obata
 * @date   2010.04.21
 */
////////////////////////////////////////////////////////////////////////////////////////// 
#pragma once

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
extern void POKEMON_EGG_Create( 
    HEAPID heap_id, const MYSTATUS* mystatus, int memo, 
    const POKEMON_PARAM* poke1, const POKEMON_PARAM* poke2, POKEMON_PARAM* egg );

//---------------------------------------------------------------------------------------- 
/**
 * @brief タマゴを孵化させる
 *
 * @param egg     孵化させるタマゴ
 * @param owner   タマゴの所持者
 * @param heap_id 使用するヒープID
 */
//---------------------------------------------------------------------------------------- 
extern void POKEMON_EGG_Birth( POKEMON_PARAM* egg, const MYSTATUS* owner, HEAPID heap_id );


#ifdef PM_DEBUG
//---------------------------------------------------------------------------------------- 
/**
 * @brief デバッグ用タマゴを作成する
 *
 * @param gameData
 * @param heapID
 *
 * @return 作成したタマゴ
 */
//---------------------------------------------------------------------------------------- 
extern POKEMON_PARAM* DEBUG_POKEMON_EGG_CreatePlainEgg( GAMEDATA* gameData, HEAPID heap_id ); // 産まれたて
extern POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateQuickEgg( GAMEDATA* gameData, HEAPID heap_id ); // もうすぐ孵化
extern POKEMON_PARAM* DEBUG_POKEMON_EGG_CreateIllegalEgg( GAMEDATA* gameData, HEAPID heap_id ); // だめタマゴ
#endif // PM_DEBUG
