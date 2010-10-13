//======================================================================
/**
 * @file  def_table_macro.h
 * @brief スクリプトコマンド定義用マクロ
 * @date  2009.09.27
 * @author  tamada GAME FREAK inc.
 *
 * @note
 */
//======================================================================
//======================================================================
//  macro
//======================================================================
#define INIT_CMD  const VMCMD_FUNC ScriptCmdTbl[] = {
#define INIT_OVERLAY_CMD(symname)  const VMCMD_FUNC ScriptCmdTbl_##symname[] = {
#define DEF_CMD( symfunc, symname )   symfunc,
#define NO_CMD()                        NULL,
#define END_CMD   (const VMCMD_FUNC)0xFFFFFFFF }; //番兵を入れて要素数を取れるようにしておく
