//======================================================================
/**
 * @file	debug_flg.h
 * @brief	グローバルに参照できるフラグの管理
 * @author	ariizumi
 * @data	09/10/27
 *
 * モジュール名：DEBUG_FLG
 */
//======================================================================
#pragma once

#define DEBUG_FLG_DEF(flgName,dispName) DEBUG_FLG_ ## flgName , //dispName
enum
{
#include "debug/debug_flg.cdat"
	DEBUG_FLG_MAX,
};
#undef DEBUG_FLG_DEF


//======================================================================
//	define
//======================================================================
#pragma mark [> define

//======================================================================
//	enum
//======================================================================
#pragma mark [> enum


//======================================================================
//	typedef struct
//======================================================================
#pragma mark [> struct

//======================================================================
//	proto
//======================================================================
#pragma mark [> proto

//--------------------------------------------------------------
//	フラグ操作と取得
//--------------------------------------------------------------
extern void DEBUG_FLG_FlgOn( const u16 flgIdx );
extern void DEBUG_FLG_FlgOff( const u16 flgIdx );
extern void DEBUG_FLG_FlgFlip( const u16 flgIdx );
extern const BOOL DEBUG_FLG_GetFlg( const u16 flgIdx );

//--------------------------------------------------------------
//	デバッググループ登録
//--------------------------------------------------------------
extern void DEBUG_FLG_CreateDebugGoupe( const HEAPID heapId );
extern void DEBUG_FLG_DeleteDebugGoupe( void );


