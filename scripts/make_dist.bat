cd ..
rd /s /q halley-launcher
mkdir halley-launcher
mkdir halley-launcher\bin
copy bin\halley-launcher.exe halley-launcher\bin\halley-launcher.exe /Y
copy bin\SDL2.dll halley-launcher\bin\SDL2.dll /Y
robocopy assets halley-launcher\assets /E