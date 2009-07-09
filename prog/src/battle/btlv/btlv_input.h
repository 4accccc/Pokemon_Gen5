
//============================================================================================
/**
 * @file	btlv_input.h
 * @brief	êÌì¨â∫âÊñ 
 * @author	soga
 * @date	2008.06.30
 */
//============================================================================================

#pragma once

typedef struct _BTLV_INPUT_WORK BTLV_INPUT_WORK;

typedef enum
{ 
  BTLV_INPUT_TYPE_SINGLE = 0,
  BTLV_INPUT_TYPE_DOUBLE,
  BTLV_INPUT_TYPE_TRIPLE,
  BTLV_INPUT_TYPE_ROTATION,
}BTLV_INPUT_TYPE;

typedef enum
{ 
  BTLV_INPUT_SCRTYPE_STANDBY = 0,
  BTLV_INPUT_SCRTYPE_COMMAND,
  BTLV_INPUT_SCRTYPE_WAZA,
  BTLV_INPUT_SCRTYPE_DIR,
  BTLV_INPUT_SCRTYPE_YES_NO,
  BTLV_INPUT_SCRTYPE_ROTATE,
}BTLV_INPUT_SCRTYPE;

extern  BTLV_INPUT_WORK*  BTLV_INPUT_Init( BTLV_INPUT_TYPE type, GFL_FONT* font, HEAPID heapID );
extern	void			        BTLV_INPUT_Exit( BTLV_INPUT_WORK* biw );
extern	void			        BTLV_INPUT_Main( BTLV_INPUT_WORK* biw );
extern  void              BTLV_INPUT_SetFrame( void );
extern  void              BTLV_INPUT_FreeFrame( void );
extern  void              BTLV_INPUT_CreateScreen( BTLV_INPUT_WORK* biw, BTLV_INPUT_SCRTYPE type );

