:: this batch script assumes that you have the basic GNU tools (installed with RTools)
@ECHO off
reg query HKEY_CURRENT_USER\Environment /v path | sed -n 's/.*REG_SZ[ ]*//p' > temp.txt
set /p VAR=<temp.txt
SETX Path %~dp0lib\tbb;%VAR%
rm temp.txt
