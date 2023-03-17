sleep 1 2>NUL

taskkill /IM halley-launcher.exe /F 2>NUL

sleep 1 2>NUL

robocopy ..\assets ..\..\assets /E /MOVE
robocopy ..\bin ..\..\bin /E /MOVE

cd ..\..\bin
start halley-launcher.exe
