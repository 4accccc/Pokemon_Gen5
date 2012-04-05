//=============================================================================
/**
 *
 *	@file		fes_rom_sub.h
 *	@brie   フェス配信ロム画面 サブ
 *	@author	Miyuki Iwasawa
 *	@data		2012.03.29
 *
 */
//=============================================================================

#pragma once

/*
 *  @brief  フェス配信ロム画面初期描画
 */
extern void FesRom_InitialDraw( FES_ROM_WORK* wk );

/*
 *  @brief  フェス配信ベース画面　入力取得
 */
extern int FesRom_InputCheckBase( FES_ROM_WORK* wk );

/*
 *  @brief  フェス配信ロム画面　メニューバー状態更新
 */
extern void FesRom_ButtonActiveControl( FES_ROM_WORK* wk );

