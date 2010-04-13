//======================================================================
/**
 * @file  debug_speed_check.h
 * @author  tamada GAMEFREAK inc.
 * @date  2010.04.09
 * @brief デバッグ用の時間計測ツール
 */
//======================================================================
#pragma once

//--------------------------------------------------------------
//  PM_DEBUGが無効な時はDEBUG_SPEED_CHECK_ENABLEも無効にする
//--------------------------------------------------------------
#if !defined(PM_DEBUG) && defined(DEBUG_SPEED_CHECK_ENABLE)
#undef  DEBUG_SPEED_CHECK_ENABLE
#endif

//--------------------------------------------------------------
//--------------------------------------------------------------
#ifdef DEBUG_SPEED_CHECK_ENABLE

#ifndef DEBUG_SPEED_CHECK_MAX
#define DEBUG_SPEED_CHECK_MAX 20
#endif

typedef struct {
  u64 checks[DEBUG_SPEED_CHECK_MAX];
  const char * mark_str[DEBUG_SPEED_CHECK_MAX];
  u32 check_count;
  OSTick _start_tick;
  OSTick _end_tick;
}DEBUG_SPEED_WORK;

static DEBUG_SPEED_WORK debugSpeedWork;

static void SET_CHECK(const char * str)
{
  if (debugSpeedWork.check_count >= DEBUG_SPEED_CHECK_MAX) return;
  debugSpeedWork.checks[debugSpeedWork.check_count] = OS_GetTick();
  debugSpeedWork.mark_str[debugSpeedWork.check_count] = str;
  debugSpeedWork.check_count ++;
}

static void START_CHECK( void )
{
  debugSpeedWork.check_count = 0;
  debugSpeedWork._start_tick = OS_GetTick(); 
  SET_CHECK("INIT_CHECK");
}

static void INIT_CHECK( void )
{
  GFL_STD_MemClear( &debugSpeedWork, sizeof(DEBUG_SPEED_WORK) );
  START_CHECK();
}

static void TAIL_CHECK( OSTick * _end_tick )
{
  *_end_tick = OS_GetTick() - debugSpeedWork._start_tick;
}

static void PUT_CHECK( void )
{
  int i;
  u64 value;
  for (i = 1; i < debugSpeedWork.check_count; i++) {
    OS_TPrintf("%8ld:", debugSpeedWork.checks[i] );
    value = debugSpeedWork.checks[i] - debugSpeedWork.checks[i-1];
    OS_TPrintf("%8ld", value);
    OS_TPrintf(" %s\n", debugSpeedWork.mark_str[i-1]);
  }
}

#else

#define INIT_CHECK()  /* DO NOTHING */
#define START_CHECK() /* DO NOTHING */
#define SET_CHECK(word)   /* DO NOTHING */
#define TAIL_CHECK(valiable)  {*(valiable) = 0;}/* DO NOTHING */
#define PUT_CHECK()   /* DO NOTHING */

#endif
 
