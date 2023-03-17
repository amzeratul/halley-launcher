@echo off

taskkill /IM halley-launcher.exe /F 2>NUL

sleep 1

robocopy ..\assets ..\..\assets /E /COPYALL /MOVE
robocopy ..\bin ..\..\bin /E /COPYALL /MOVE
