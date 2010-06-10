#include "message_dl.naix"
#define MB_ERROR_MSG_FILE (NARC_message_dl_net_err_dat)
#define MB_BACKUP_ERROR_MSG_FILE (NARC_message_dl_backup_err_dat)

#include "../prog/src/system/net_err.c"
#include "../prog/src/system/save_error_warning.c"

//net_errの名前のままだと、narcファイルを*.oファイルとしてROMに
//置くため、net_err.oが2個存在してしまうという状況になる。
//なので、ソースでソースをincludeしてファイル名を変えることで対処している・・・