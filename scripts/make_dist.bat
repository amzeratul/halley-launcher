cd ..
rd /s /q dist
mkdir dist
mkdir dist\bin
mkdir dist\scripts
copy bin\halley-launcher.exe dist\bin\halley-launcher.exe /Y
copy bin\SDL2.dll dist\bin\SDL2.dll /Y
copy scripts\update.bat dist\scripts\update.bat
robocopy assets dist\assets /E
cd dist
7z a -tzip halley-launcher.zip *