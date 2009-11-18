//======================================================================
/**
 * @file	mb_local_def.h
 * @brief	マルチブート 共通定義
 * @author	ariizumi
 * @data	09/11/13
 *
 * モジュール名：MB_DEF
 */
//======================================================================


#define MB_DEF_GGID (0x444)


#ifndef MULTI_BOOT_MAKE

#include "font/font.naix"
#include "message.naix"
#include "msg/msg_multiboot_parent.h"
#define ARCID_FONT_MB (ARCID_FONT)
#define ARCID_MESSAGE_MB (ARCID_MESSAGE)
#define FILE_MSGID_MB (NARC_message_multiboot_parent_dat)
#define FILE_FONT_LARGE_MB (NARC_font_large_gftr)
#define FILE_FONT_PLT_MB (NARC_font_default_nclr)
#define MSGID_YES_MB (MSG_MB_PAERNT_SYS_01)
#define MSGID_NO_MB (MSG_MB_PAERNT_SYS_02)

#else

#include "font/font_dl.naix"
#include "message_dl.naix"
#include "msg/msg_multiboot_child.h"
#define ARCID_FONT_MB (ARCID_FONT)
#define ARCID_MESSAGE_MB (ARCID_MESSAGE)
#define FILE_MSGID_MB (NARC_message_dl_multiboot_child_dat)
#define FILE_FONT_LARGE_MB (NARC_font_dl_large_gftr)
#define FILE_FONT_PLT_MB (NARC_font_dl_default_nclr)
#define MSGID_YES_MB (MSG_MB_CHILD_SYS_01)
#define MSGID_NO_MB (MSG_MB_CHILD_SYS_02)

#endif //MULTI_BOOT_MAKE