REM @echo off
ECHO "MuseScore build"

SET ARTIFACTS_DIR=build.artifacts
SET INSTALL_DIR=../build.install
SET BUILD_NUMBER=""
SET CRASH_LOG_SERVER_URL=""
SET TARGET_PROCESSOR_BITS=64
SET BUILD_WIN_PORTABLE=OFF

:GETOPTS
IF /I "%1" == "-n" SET BUILD_NUMBER=%2& SHIFT
IF /I "%1" == "-b" SET TARGET_PROCESSOR_BITS=%2& SHIFT
IF /I "%1" == "--crash_log_url" SET CRASH_LOG_SERVER_URL=%2& SHIFT & SHIFT
IF /I "%1" == "--portable" SET BUILD_WIN_PORTABLE=%2& SHIFT
SHIFT
IF NOT "%1" == "" GOTO GETOPTS

IF %BUILD_NUMBER% == "" ( ECHO "error: not set BUILD_NUMBER" & EXIT /b 1)
IF NOT %TARGET_PROCESSOR_BITS% == 64 (
    IF NOT %TARGET_PROCESSOR_BITS% == 32 (
        ECHO "error: not set TARGET_PROCESSOR_BITS, must be 32 or 64, current TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
        EXIT /b 1
    )
)

SET /p BUILD_MODE=<%ARTIFACTS_DIR%\env\build_mode.env
SET "MUSE_APP_BUILD_MODE=dev"
IF %BUILD_MODE% == devel   ( SET "MUSE_APP_BUILD_MODE=dev" ) ELSE (
IF %BUILD_MODE% == nightly ( SET "MUSE_APP_BUILD_MODE=dev" ) ELSE (
IF %BUILD_MODE% == testing ( SET "MUSE_APP_BUILD_MODE=testing" ) ELSE (
IF %BUILD_MODE% == stable  ( SET "MUSE_APP_BUILD_MODE=release" ) ELSE (
    ECHO "error: unknown BUILD_MODE: %BUILD_MODE%"
    EXIT /b 1
))))

ECHO "MUSE_APP_BUILD_MODE: %MUSE_APP_BUILD_MODE%"
ECHO "BUILD_NUMBER: %BUILD_NUMBER%"
ECHO "TARGET_PROCESSOR_BITS: %TARGET_PROCESSOR_BITS%"
ECHO "CRASH_LOG_SERVER_URL: %CRASH_LOG_SERVER_URL%"
ECHO "BUILD_WIN_PORTABLE: %BUILD_WIN_PORTABLE%"

XCOPY "C:\musescore_dependencies" %CD% /E /I /Y
ECHO "Finished copy dependencies"

SET "QT_DIR=C:\Qt\6.2.11"
SET "JACK_DIR=C:\Program Files (x86)\Jack"
SET "PATH=%QT_DIR%\bin;%JACK_DIR%;%PATH%"

SET MUSESCORE_BUILD_CONFIGURATION="app"
IF %BUILD_WIN_PORTABLE% == ON ( 
    SET INSTALL_DIR=../build.install/App/MuseScore
    SET MUSESCORE_BUILD_CONFIGURATION="app-portable"
)

bash ./buildscripts/ci/tools/make_revision_env.sh 
SET /p MUSESCORE_REVISION=<%ARTIFACTS_DIR%\env\build_revision.env

SET MUSESCORE_BUILD_CONFIGURATION=%MUSESCORE_BUILD_CONFIGURATION%
SET MUSE_APP_BUILD_MODE=%MUSE_APP_BUILD_MODE%
SET MUSESCORE_BUILD_NUMBER=%BUILD_NUMBER%
SET MUSESCORE_REVISION=%MUSESCORE_REVISION%
SET MUSESCORE_INSTALL_DIR=%INSTALL_DIR%
SET MUSESCORE_CRASHREPORT_URL="%CRASH_LOG_SERVER_URL%"
SET MUSESCORE_BUILD_VST_MODULE="ON"
SET MUSESCORE_BUILD_WEBSOCKET="ON"

CALL ninja_build.bat -t installrelwithdebinfo || exit \b 1

bash ./buildscripts/ci/tools/make_release_channel_env.sh -c %MUSE_APP_BUILD_MODE%
bash ./buildscripts/ci/tools/make_version_env.sh %BUILD_NUMBER%
bash ./buildscripts/ci/tools/make_branch_env.sh
