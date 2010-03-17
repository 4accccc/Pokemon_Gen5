//============================================================================
/**
 *
 *  @file   pdc_msg.h
 *  @brief  ポケモンドリームキャッチ
 *  @author soga
 *  @data   2010.03.16
 *
 */
//============================================================================
#pragma once

#include "print/printsys.h"

typedef struct _PDC_MSG_WORK PDC_MSG_WORK;

extern  PDC_MSG_WORK* PDC_MSG_Create( GFL_FONT* font, CONFIG* config, GFL_TCBLSYS* tcbl, HEAPID heapID );
extern  void          PDC_MSG_Delete( PDC_MSG_WORK* pmw );
extern  void          PDC_MSG_SetEncountMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp );
extern  void          PDC_MSG_SetDousuruMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp );
extern  void          PDC_MSG_SetThrowMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp );
extern  void          PDC_MSG_SetCaptureMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp );
extern  void          PDC_MSG_SetEscapeMsg( PDC_MSG_WORK* pmw, PDC_SETUP_PARAM* psp );
extern  BOOL          PDC_MSG_Wait( PDC_MSG_WORK* pmw );
extern  void          PDC_MSG_HideWindow( PDC_MSG_WORK* wk );
extern  BOOL          PDC_MSG_WaitHideWindow( PDC_MSG_WORK* wk );
extern  BOOL          PDC_MSG_IsJustDone( PDC_MSG_WORK* wk );
