//======================================================================
/**
 * @file	battle_championship.h
 * @brief	大会メニュー
 * @author	ariizumi
 * @data	09/10/08
 *
 * モジュール名：BATTLE_CHAMPINONSHIP
 */
//======================================================================
#pragma once

//======================================================================
//	define
//======================================================================
#pragma mark [> define
//ちょっと乱暴ですが、この値を引数のvoid*に与えてください
typedef void* BATTLE_CHAMPIONSHIP_MODE;
#define BATTLE_CHAMPIONSHIP_MODE_MAIN_MENU  ((BATTLE_CHAMPIONSHIP_MODE)0)    //初期メニュー
#define BATTLE_CHAMPIONSHIP_MODE_WIFI_MENU  ((BATTLE_CHAMPIONSHIP_MODE)1)    //WIFI大会メニュー

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto
extern const GFL_PROC_DATA BATTLE_CHAMPIONSHIP_ProcData;

//--------------------------------------------------------------
//	
//--------------------------------------------------------------
FS_EXTERN_OVERLAY( battle_championship );
