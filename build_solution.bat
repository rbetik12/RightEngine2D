@echo off

:menu
cls
echo Select build config:
echo 1: Debug
echo 2: Release

set /p choice=Enter your choice: 

if "%choice%"=="1" (
    set build_type=Debug
    goto launch_script
) else if "%choice%"=="2" (
    set build_type=Release
    goto launch_script
) else (
    echo Invalid choice. Please select 1 or 2.
    pause
    goto menu
)

:launch_script
python Scripts/build_solution.py %build_type%
pause