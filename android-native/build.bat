@echo off
if not exist .\build mkdir .\build

set APK=native.apk

if "%ANDROID_NDK%" equ "" set ANDROID_NDK=C:\Users\Pavel\AppData\Local\Android\Sdk\ndk-bundle
if "%ANDROID_SDK%" equ "" set ANDROID_SDK=C:\Users\Pavel\AppData\Local\Android\Sdk
if "%JAVA_JDK%" equ "" set JAVA_JDK=C:\dev\jdk-20.0.2
if "%BUILD_TOOLS%" equ "" set BUILD_TOOLS=34.0.0
if "%PLATFORM%" equ "" set PLATFORM=android-33

call "%ANDROID_NDK%\ndk-build.cmd" -j4 NDK_LIBS_OUT=jniLibs\lib || exit /b 1
if ERRORLEVEL 1 exit /b 1

"%ANDROID_SDK%\build-tools\%BUILD_TOOLS%\aapt.exe" package -f -M AndroidManifest.xml -I "%ANDROID_SDK%\platforms\%PLATFORM%\android.jar" -A ..\assets -F build\%APK%.build jniLibs
if ERRORLEVEL 1 exit /b 1

if not exist .keystore (
  "%JAVA_JDK%\bin\keytool.exe" -genkey -dname "CN=Android Debug, O=Android, C=US" -keystore .keystore -alias androiddebugkey -storepass android -keypass android -keyalg RSA -validity 30000
  if ERRORLEVEL 1 exit /b 1
)

"%ANDROID_SDK%\build-tools\%BUILD_TOOLS%\zipalign.exe" -f 4 build\%APK%.build build\%APK%
if ERRORLEVEL 1 exit /b 1

"%ANDROID_SDK%\build-tools\%BUILD_TOOLS%\apksigner.bat" sign --ks .keystore --ks-key-alias androiddebugkey --ks-pass pass:android build\%APK%
if ERRORLEVEL 1 exit /b 1
