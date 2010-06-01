REM 世界全体CSVデータ作成中・・・
wificonv 3 world.txt place_pos_wrd.dat 
REM アルゼンチンCSVデータ作成中・・・
wificonv 2 ARG.txt place_pos_ARG.dat 
REM オーストラリアCSVデータ作成中・・・
wificonv 2 AUS.txt place_pos_AUS.dat 
REM ブラジルCSVデータ作成中・・・
wificonv 2 BRA.txt place_pos_BRA.dat 
REM カナダCSVデータ作成中・・・
wificonv 2 CAN.txt place_pos_CAN.dat 
REM 中国CSVデータ作成中・・・
wificonv 2 CHN.txt place_pos_CHN.dat 
REM フィンランドCSVデータ作成中・・・
wificonv 2 FIN.txt place_pos_FIN.dat 
REM フランスCSVデータ作成中・・・
wificonv 2 FRA.txt place_pos_FRA.dat 
REM ドイツCSVデータ作成中・・・
wificonv 2 DEU.txt place_pos_DEU.dat 
REM インドCSVデータ作成中・・・
wificonv 2 IND.txt place_pos_IND.dat 
REM イタリアCSVデータ作成中・・・
wificonv 2 ITA.txt place_pos_ITA.dat 
REM 日本CSVデータ作成中・・・
wificonv 2 JPN.txt place_pos_JPN.dat 
REM ノルウェーCSVデータ作成中・・・
wificonv 2 NOR.txt place_pos_NOR.dat 
REM ポーランドCSVデータ作成中・・・
wificonv 2 POL.txt place_pos_POL.dat 
REM ロシアCSVデータ作成中・・・
wificonv 2 RUS.txt place_pos_RUS.dat 
REM スペインCSVデータ作成中・・・
wificonv 2 ESP.txt place_pos_ESP.dat 
REM スウェーデンCSVデータ作成中・・・
wificonv 2 SWE.txt place_pos_SWE.dat 
REM イギリスCSVデータ作成中・・・
wificonv 2 GBR.txt place_pos_GBR.dat 
REM アメリカCSVデータ作成中・・・
wificonv 2 USA.txt place_pos_USA.dat 

ruby wifiearth_gmm.rb world.txt wifi_place_msg_world.gmm
ruby wifiearth_gmm.rb ARG.txt wifi_place_msg_ARG.gmm
ruby wifiearth_gmm.rb AUS.txt wifi_place_msg_AUS.gmm
ruby wifiearth_gmm.rb BRA.txt wifi_place_msg_BRA.gmm
ruby wifiearth_gmm.rb CAN.txt wifi_place_msg_CAN.gmm
ruby wifiearth_gmm.rb CHN.txt wifi_place_msg_CHN.gmm
ruby wifiearth_gmm.rb DEU.txt wifi_place_msg_DEU.gmm
ruby wifiearth_gmm.rb ESP.txt wifi_place_msg_ESP.gmm
ruby wifiearth_gmm.rb FIN.txt wifi_place_msg_FIN.gmm
ruby wifiearth_gmm.rb FRA.txt wifi_place_msg_FRA.gmm
ruby wifiearth_gmm.rb GBR.txt wifi_place_msg_GBR.gmm
ruby wifiearth_gmm.rb IND.txt wifi_place_msg_IND.gmm
ruby wifiearth_gmm.rb ITA.txt wifi_place_msg_ITA.gmm
ruby wifiearth_gmm.rb JPN.txt wifi_place_msg_JPN.gmm
ruby wifiearth_gmm.rb NOR.txt wifi_place_msg_NOR.gmm
ruby wifiearth_gmm.rb POL.txt wifi_place_msg_POL.gmm
ruby wifiearth_gmm.rb RUS.txt wifi_place_msg_RUS.gmm
ruby wifiearth_gmm.rb SWE.txt wifi_place_msg_SWE.gmm
ruby wifiearth_gmm.rb USA.txt wifi_place_msg_USA.gmm

ruby wifiearth_cdat.rb aiueo.txt sort_list.cdat
