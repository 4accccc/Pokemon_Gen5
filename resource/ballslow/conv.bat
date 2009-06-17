

perl bucket_conv.pl



nnsarc -c -i -n ballslow_data -S list.txt
copy *.naix ..\..\prog\arc
copy *.narc ..\..\prog\arc

pause

del *.narc
del *.naix
del *.bin

