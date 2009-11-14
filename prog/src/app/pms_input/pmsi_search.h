//============================================================================
/**
 *
 *	@file		pmsi_search.h
 *	@brief
 *	@author		hosaka genya
 *	@data		2009.11.14
 *
 */
//============================================================================
#pragma once

typedef struct _PMS_INPUT_SEARCH PMS_INPUT_SEARCH;

//=============================================================================
/**
 *								íËêîíËã`
 */
//=============================================================================

//=============================================================================
/**
 *								ç\ë¢ëÃíËã`
 */
//=============================================================================

//=============================================================================
/**
 *								EXTERNêÈåæ
 */
//=============================================================================
extern PMS_INPUT_SEARCH* PMSI_SEARCH_Create( const PMS_INPUT_WORK* mwk, const PMS_INPUT_DATA* dwk, HEAPID heap_id );
extern void PMSI_SEARCH_Delete( PMS_INPUT_SEARCH* wk );
extern void PMSI_SEARCH_AddWord( PMS_INPUT_SEARCH* wk, u16 word_code );
extern BOOL PMSI_SEARCH_DelWord( PMS_INPUT_SEARCH* wk );
extern BOOL PMSI_SEARCH_Start( PMS_INPUT_SEARCH*wk );
extern u16 PMSI_SEARCH_GetResultCount( PMS_INPUT_SEARCH* wk );
extern STRBUF* PMSI_SEARCH_CreateResultString( PMS_INPUT_SEARCH* wk, u8 result_idx );


