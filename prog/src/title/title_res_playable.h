//======================================================================
/**
 * @file  title_res_playable.h
 * @brief タイトルのバージョン違い吸収用
 * @author  ariizumi --> tamada
 * @since  10.05.15
 * @date  2010.06.24
 *
 * モジュール名：TITLE_RES
 */
//======================================================================
#pragma once

#include "pm_version.h"

#define TITLE_RES_ARCID get_TITLE_RES_ARCID()
static inline int get_TITLE_RES_ARCID(void)
{
  if (GET_VERSION() == VERSION_BLACK) {
    return ARCID_TITLE_B;
  } else {
    return ARCID_TITLE_W;
  }
}


#include "title_black.naix"
#include "title_white.naix"

#define TITLE_RES_LOGO_NCLR get_TITLE_RES_LOGO_NCLR()
static inline int get_TITLE_RES_LOGO_NCLR (void){
  if (GET_VERSION() == VERSION_BLACK){
    return NARC_title_black_blk_logo_NCLR;
  } else {
    return NARC_title_white_wht_logo_NCLR;
  }
}
#define TITLE_RES_LOGO_NCGR get_TITLE_RES_LOGO_NCGR()
static inline int get_TITLE_RES_LOGO_NCGR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_logo_lz_NCGR;
  } else {
    return NARC_title_black_blk_logo_lz_NCGR;
  }
}
#define TITLE_RES_LOGO_NSCR get_TITLE_RES_LOGO_NSCR()
static inline int get_TITLE_RES_LOGO_NSCR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_logo_lz_NSCR;
  } else {
    return NARC_title_white_wht_logo_lz_NSCR;
  }
}
#define TITLE_RES_BACK_NSCR get_TITLE_RES_BACK_NSCR()
static inline int get_TITLE_RES_BACK_NSCR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_base_lz_NSCR;
  } else {
    return NARC_title_white_wht_base_lz_NSCR;
  }
}

// 「PUSH START BUTTON」（セルアクター）
#define TITLE_RES_PUSH_NCLR get_TITLE_RES_PUSH_NCLR()
static inline int get_TITLE_RES_PUSH_NCLR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_obj_NCLR;
  } else {
    return NARC_title_white_wht_obj_NCLR;
  }
}
#define TITLE_RES_PUSH_NCGR get_TITLE_RES_PUSH_NCGR()
static inline int get_TITLE_RES_PUSH_NCGR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_obj_lz_NCGR;
  } else {
    return NARC_title_white_wht_obj_lz_NCGR;
  }
}
#define TITLE_RES_PUSH_NCER get_TITLE_RES_PUSH_NCER()
static inline int get_TITLE_RES_PUSH_NCER (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_obj_NCER;
  } else {
    return NARC_title_white_wht_obj_NCER;
  }
}
#define TITLE_RES_PUSH_NANR get_TITLE_RES_PUSH_NANR()
static inline int get_TITLE_RES_PUSH_NANR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_obj_NANR;
  } else {
    return NARC_title_white_wht_obj_NANR;
  }
}

// 「Developed by GameFreak」(BG)
#define TITLE_RES_DEV_NCLR get_TITLE_RES_DEV_NCLR()
static inline int get_TITLE_RES_DEV_NCLR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_develop_NCLR;
  } else {
    return NARC_title_white_wht_develop_NCLR;
  }
}
#define TITLE_RES_DEV_NCGR get_TITLE_RES_DEV_NCGR()
static inline int get_TITLE_RES_DEV_NCGR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_develop_lz_NCGR;
  } else {
    return NARC_title_white_wht_develop_lz_NCGR;
  }
}
#define TITLE_RES_DEV_NSCR get_TITLE_RES_DEV_NSCR()
static inline int get_TITLE_RES_DEV_NSCR (void){
  if (GET_VERSION() == VERSION_BLACK) {
    return NARC_title_black_blk_develop_lz_NSCR;
  } else {
    return NARC_title_white_wht_develop_lz_NSCR;
  }
}

