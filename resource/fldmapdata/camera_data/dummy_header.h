//#include <nitro.h>
//----------------------------------------------------------------------------
//NitroSDK/include/nitro/types.h‚æ‚èƒRƒsƒy
//----------------------------------------------------------------------------
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;
typedef signed long s32;


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//NitroSDK/include/nitro/fx/fx.h
typedef s32 fx32;

//----------------------------------------------------------------------------
//  VecFx32: 3D Vector of fx32
//----------------------------------------------------------------------------
typedef struct
{
    fx32    x;
    fx32    y;
    fx32    z;
}
VecFx32;

#define FX32_ONE                         ((fx32) 0x00001000L)     // 1.000

#include "fieldcameraformat.h"
