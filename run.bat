@echo off

cmake --build build/release --config Release --parallel %NUMBER_OF_PROCESSORS% -- /p:CL_MPcount=3

if %ERRORLEVEL% EQU 0 cd bin & .\app_name_here