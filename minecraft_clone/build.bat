@echo off
echo ============================================
echo   Build Minecraft Clone -> .exe
echo ============================================
echo.

echo [1/3] Installation des dependances...
pip install -r requirements.txt
if errorlevel 1 (
    echo ERREUR : pip a echoue.
    pause
    exit /b 1
)

echo.
echo [2/3] Compilation avec PyInstaller...
pyinstaller --onefile --windowed --name "MinecraftClone" main.py
if errorlevel 1 (
    echo ERREUR : PyInstaller a echoue.
    pause
    exit /b 1
)

echo.
echo [3/3] Termine !
echo L'executable se trouve dans : dist\MinecraftClone.exe
echo.
pause
