#include "../prog/src/test/ohno/fatal_error.c"

//fatal_errorの名前のままだと、narcファイルを*.oファイルとしてROMに
//置くため、fatal_error.oが2個存在してしまうという状況になる。
//なので、ソースでソースをincludeしてファイル名を変えることで対処している・・・