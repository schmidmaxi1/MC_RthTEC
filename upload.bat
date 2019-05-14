mode com7 BAUD=115200 DATA=8 PARITY=N dtr=on
avrdude -Cavrdude.conf -v -patmega2560 -cwiring -PCOM7 -b115200 -D -Uflash:w:"Debug/MC_RthRack_V3_0.hex":i

if %ERRORLEVEL%==0 goto fertig
Pause
:fertig
mode com7 dtr=off
exit