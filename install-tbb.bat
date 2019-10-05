:: this batch script assumes that you have the basic GNU tools (installed with RTools)
@ECHO off
reg query HKEY_CURRENT_USER\Environment /v path | grep -o "REG_SZ.*" | cut -d" " -f5 > temp.txt
set /p VAR=<temp.txt
SETX Path %~dp0\lib\tbb;%VAR%
rm temp.txt
