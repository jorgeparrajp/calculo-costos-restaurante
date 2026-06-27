@echo off
cd /d "%~dp0"
C:\msys64\ucrt64\bin\gcc.exe main.c restaurante.c -o restaurante.exe
if errorlevel 1 (
    echo.
    echo Error al compilar.
    pause
    exit /b 1
)
echo.
echo Compilado correctamente.
echo.
restaurante.exe
pause

