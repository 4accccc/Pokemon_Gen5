

ruby ../../tools/binlinker.rb map_a1.nsbmd map_a1.nsbtx NONE map_a1.bin map_a1.3dmd map_a1.3dppack
pause
nnsarc -c -l -n -i test3dp -S data.list
pause
ruby ../../tools/naixcutstamp.rb test3dp.naix
pause
