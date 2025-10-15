@echo off
NET SESSION >NUL 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO This script requires administrator privileges.
    ECHO Please right-click this file and choose "Run as administrator."
    PAUSE
    EXIT /B
)

ECHO.
ECHO   Available Network Adapters (Connected)
ECHO.

setlocal enabledelayedexpansion
set count=0

for /f "skip=1 tokens=1,2,3,*" %%A in ('netsh interface show interface ^| findstr /I "Connected"') do (
    for /f "tokens=* delims= " %%B in ("%%D") do (
        set /a count+=1
        set "adapter!count!=%%B"
        echo !count! %%B
    )
)

if %count%==0 (
    echo No active network adapters found.
    pause
    exit /b
)

echo.
set /p sel="Select adapter number: "

if "%sel%"=="" (
    echo Invalid selection.
    pause
    exit /b
)

if %sel% GTR %count% (
    echo Invalid selection.
    pause
    exit /b
)

set "adapter=!adapter%sel%!"
echo.
echo You selected: "%adapter%"
echo.

echo Current DNS settings for "!adapter!"
echo.
for /f "tokens=* delims= " %%B in ("!adapter!") do set "adapter_trimmed=%%B"
netsh interface ip show dns name="!adapter_trimmed!"
echo.

echo What would you like to do?
echo 1) Automatic (DHCP)
echo 2) Google DNS (8.8.8.8, 8.8.4.4)
echo 3) Manual (enter your own DNS servers)
echo.
set /p choice="Select DNS configuration option number: "

if "%choice%"=="1" goto SET_DHCP
if "%choice%"=="2" goto SET_GOOGLE
if "%choice%"=="3" goto SET_MANUAL

echo Invalid selection.
pause
exit /b

:SET_DHCP
echo.
echo Setting DNS to Automatic (DHCP)...
netsh interface ip set dns name="%adapter%" source=dhcp
echo DNS set to Automatic (DHCP).
goto SHOW_DNS

:SET_GOOGLE
echo.
echo Setting DNS to Google DNS...
netsh interface ip set dns name="%adapter%" static 8.8.8.8 primary
netsh interface ip add dns name="%adapter%" 8.8.4.4 index=2
echo DNS set to Google DNS (8.8.8.8, 8.8.4.4).
goto SHOW_DNS

:SET_MANUAL
echo.
set /p dns1="Enter primary DNS server: "
set /p dns2="Enter secondary DNS server (or leave blank): "
echo.
echo Setting manual DNS servers...
netsh interface ip set dns name="%adapter%" static %dns1% primary
if not "%dns2%"=="" netsh interface ip add dns name="%adapter%" %dns2% index=2
echo DNS set to Manual (%dns1% %dns2%).
goto SHOW_DNS

:SHOW_DNS
echo.
echo ========================================
echo   Updated DNS settings for "%adapter%"
echo ========================================
netsh interface ip show dns name="%adapter%"
echo.
pause
exit /b
