@echo off

cmake --build build/release --config Release --parallel %NUMBER_OF_PROCESSORS% -- /p:CL_MPcount=%NUMBER_OF_PROCESSORS%
@REM cmake --build build/release --config Debug --parallel %NUMBER_OF_PROCESSORS% -- /p:CL_MPcount=%NUMBER_OF_PROCESSORS%

if %ERRORLEVEL% EQU 0 cd bin & .\app_name_here