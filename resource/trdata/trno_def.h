//============================================================================================
/**
 * @file	trno_def.h
 * @bfief	トレーナーID定義ファイル
 * @author	TrainerDataConverter
 * @date	2009-09-14
 * @author	このファイルは、コンバータが吐き出したファイルです
*/
//============================================================================================

#ifndef __TRNO_DEF_H_
#define __TRNO_DEF_H_

#define	TRID_NULL   ( 0 )
#define	TRID_TANPAN_01   ( 1 ) 
#define	TRID_MINI_01   ( 2 ) 
#define	TRID_SCHOOLB_01   ( 3 ) 
#define	TRID_SCHOOLG_01   ( 4 ) 
#define	TRID_TENNIS_01   ( 5 ) 
#define	TRID_FOOTBALL_01   ( 6 ) 

#ifndef __ASM_NO_DEF_  //これ以降はアセンブラでは無視
typedef  int TrainerID;
#endif __ASM_NO_DEF_

#endif __TRNO_DEF_H_
