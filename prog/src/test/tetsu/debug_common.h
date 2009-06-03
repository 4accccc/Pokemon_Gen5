#pragma once

#include "message.naix"
#include "font/font.naix"
#include "print/printsys.h"
#include "print/str_tool.h"
#include "test/camera_adjust_view.h"

#include "msg/msg_d_tetsu.h"

typedef struct _DWS_SYS DWS_SYS;

extern DWS_SYS*	DWS_SYS_Setup(HEAPID heapID);
extern void			DWS_SYS_Framework(DWS_SYS* dws);
extern void			DWS_SYS_Delete(DWS_SYS* dws);

extern GFL_FONT*					DWS_GetFontHandle(DWS_SYS* dws);
extern GFL_MSGDATA*				DWS_GetMsgManager(DWS_SYS* dws);
extern GFL_G3D_CAMERA*		DWS_GetG3Dcamera(DWS_SYS* dws);
extern GFL_G3D_LIGHTSET*	DWS_GetG3Dlight(DWS_SYS* dws);
extern GFL_CAMADJUST*			DWS_GetCamAdjust(DWS_SYS* dws);
extern void								DWS_SetG3DcamTarget(DWS_SYS* dws, VecFx32* t);
extern void								DWS_CamAdjustOn(DWS_SYS* dws);
extern void								DWS_CamAdjustOff(DWS_SYS* dws);

extern void								DWS_DrawLocalOriginPlane(DWS_SYS* dws, GXRgb color);

