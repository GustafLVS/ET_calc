@echo off
title ET-Rechner Build
echo ============================================
echo  ET-Rechner - Qt Build Script
echo ============================================
echo.

REM Find qmake — adjust path if your Qt installation is elsewhere
SET QT_PATH=
FOR %%d IN (
    "C:\Qt\6.7.0\msvc2019_64\bin"
    "C:\Qt\6.6.0\msvc2019_64\bin"
    "C:\Qt\6.5.3\msvc2019_64\bin"
    "C:\Qt\6.5.0\msvc2019_64\bin"
    "C:\Qt\6.4.0\msvc2019_64\bin"
    "C:\Qt\6.7.0\mingw_64\bin"
    "C:\Qt\6.6.0\mingw_64\bin"
    "C:\Qt\6.5.3\mingw_64\bin"
    "C:\Qt\6.7.0\msvc2022_64\bin"
    "C:\Qt\6.8.0\msvc2022_64\bin"
    "C:\Qt\6.8.0\mingw_64\bin"
) DO (
    IF EXIST "%%~d\qmake.exe" (
        SET QT_PATH=%%~d
        goto :found
    )
)

echo ERROR: Qt not found! Please edit build.bat and set QT_PATH manually.
echo Common Qt paths:
echo   C:\Qt\6.x.x\msvc2022_64\bin
echo   C:\Qt\6.x.x\mingw_64\bin
echo.
echo You can also use Qt Creator to open ET_calc.pro directly.
pause
exit /b 1

:found
echo Found Qt at: %QT_PATH%
SET PATH=%QT_PATH%;%PATH%

echo.
echo [1/3] Running qmake...
qmake ET_calc.pro -spec win32-msvc CONFIG+=release
IF %ERRORLEVEL% NEQ 0 (
    echo qmake failed. Trying MinGW...
    qmake ET_calc.pro CONFIG+=release
    IF %ERRORLEVEL% NEQ 0 (
        echo qmake failed! Check Qt installation.
        pause
        exit /b 1
    )
)

echo [2/3] Building with nmake / mingw32-make...
WHERE nmake >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    nmake /nologo
) ELSE (
    mingw32-make -j4
)

IF %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo [3/3] Deploy Qt DLLs (windeployqt)...
IF EXIST "release\ET_calc.exe" (
    windeployqt release\ET_calc.exe
    echo.
    echo Build complete! EXE: release\ET_calc.exe
) ELSE IF EXIST "ET_calc.exe" (
    windeployqt ET_calc.exe
    echo Build complete! EXE: ET_calc.exe
)

echo.
echo ============================================
echo  Build erfolgreich!
echo ============================================
pause
