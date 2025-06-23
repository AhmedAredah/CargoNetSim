@echo off
:: CargoNetSim Installer Build Script for Windows
:: This script automates the process of building the CargoNetSim installer

echo ===============================================
echo CargoNetSim Installer Build Script
echo ===============================================

:: Check if we're in the right directory
if not exist "..\..\CMakeLists.txt" (
    echo ERROR: This script must be run from the src/installer directory
    echo Current directory: %CD%
    pause
    exit /b 1
)

:: Set build directory
set BUILD_DIR=..\..\build
set INSTALL_DIR=%BUILD_DIR%\install

echo Step 1: Creating build directory...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

echo Step 2: Configuring CMake with installer enabled...
cd "%BUILD_DIR%"
cmake .. -DCARGONET_BUILD_INSTALLER=ON -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo Step 3: Building CargoNetSim...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo Step 4: Installing to staging directory...
cmake --build . --target install --config Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Install failed
    pause
    exit /b 1
)

echo Step 5: Generating installer package...
cpack -G IFW -C Release
if %ERRORLEVEL% neq 0 (
    echo ERROR: Installer generation failed
    pause
    exit /b 1
)

echo ===============================================
echo SUCCESS: Installer generated successfully!
echo ===============================================
echo Installer location: %BUILD_DIR%\CargoNetSimInstaller-1.0.0-Windows.exe
echo.
echo You can now distribute the installer to end users.
echo.
pause