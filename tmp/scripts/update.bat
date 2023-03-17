@echo off

taskkill /IM halley-launcher.exe /F 2>NUL

sleep 1

robocopy ..\assets ..\..\assets /E /MOVE /IS
robocopy ..\bin ..\..\bin /E /MOVE /IS
