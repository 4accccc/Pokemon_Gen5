REM 引数0を成功・それ以外を失敗とみなす
if %1 == 0 goto _OK:

:_NG
"C:\Program Files\Ipmsg\ipmsg.exe" /MSG W00159 "メイク失敗！"
goto _END:

:_OK
"C:\Program Files\Ipmsg\ipmsg.exe" /MSG W00159 "メイク成功！"
goto _END:

:_END
