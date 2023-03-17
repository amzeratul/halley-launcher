@echo off

sleep 1 2>NUL

taskkill /IM halley-launcher.exe /F 2>NUL

sleep 1 2>NUL

robocopy ..\assets ..\..\assets /E /COPYALL /MOVE
robocopy ..\bin ..\..\bin /E /COPYALL /MOVE

cd ..\..\bin
start halley-launcher.exe
