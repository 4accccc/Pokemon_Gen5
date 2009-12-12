//======================================================================
/**
 * @file  map_replace_def.h
 * @brief
 * @date  2009.12.08
 * @author  tamada GAMEFREAK inc.
 */
//======================================================================
#pragma once

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
  MAPREPLACE_TYPE_SEASON = 0,
  MAPREPLACE_TYPE_VERSION,
  MAPREPLACE_TYPE_SEASON_VERSION,
  MAPREPLACE_TYPE_EVENT01,
  MAPREPLACE_TYPE_EVENT02,
  MAPREPLACE_TYPE_EVENT03,
  MAPREPLACE_TYPE_EVENT04,
  MAPREPLACE_TYPE_EVENT05,
  MAPREPLACE_TYPE_EVENT06,
  MAPREPLACE_TYPE_EVENT07,
  MAPREPLACE_TYPE_EVENT08,
  MAPREPLACE_TYPE_EVENT09,
  MAPREPLACE_TYPE_EVENT10,

  MAPREPLACE_TYPE_MAX,
}MAPREPLACE_TYPE;

//--------------------------------------------------------------
//--------------------------------------------------------------
typedef enum {
  MAPREPLACE_LAYER_MAPBLOCK = 0,
  MAPREPLACE_LAYER_MATRIX,
  MAPREPLACE_LAYER_MAX,
}MAPREPLACE_LAYER;

