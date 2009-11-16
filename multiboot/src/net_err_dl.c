#include "../prog/src/system/net_err.c"

//net_errの名前のままだと、narcファイルを*.oファイルとしてROMに
//置くため、net_err.oが2個存在してしまうという状況になる。
//なので、ソースでソースをincludeしてファイル名を変えることで対処している・・・