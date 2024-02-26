@echo off
set APK=native.apk
if "%BUILD_TOOLS%" equ "" set BUILD_TOOLS=34.0.0

"%ANDROID_SDK%\platform-tools\adb.exe" install --no-incremental -r build\%APK%
if ERRORLEVEL 1 exit /b 1

for /f "tokens=1-2" %%a in ('"%ANDROID_SDK%\build-tools\%BUILD_TOOLS%\aapt.exe" dump badging build\%APK%') do (
  if "%%a" equ "package:" (
    for /f "tokens=2 delims='" %%v in ("%%b") do (
      set PACKAGE=%%v
    )
  ) else if "%%a" equ "launchable-activity:" (
    for /f "tokens=2 delims='" %%v in ("%%b") do (
      set ACTIVITY=%%v
    )
  )
)

"%ANDROID_SDK%\platform-tools\adb.exe" shell am start -n %PACKAGE%/%ACTIVITY%
if ERRORLEVEL 1 exit /b 1