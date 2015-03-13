REM: To generate the python libs for MinGW, the libraries from the lib windows folder were used. You need gendef to generate the def files for MinGW
REM: gendef can be found here http://sourceforge.net/projects/mingw/files/MinGW/Extension/gendef/

cd c:\blenderdev\lib\mingw32\python\lib
gendef.exe python34.dll
gendef.exe python34_d.dll
c:\MinGW\bin\dlltool.exe --dllname python34_d.dll --input-def python34_d.def --output-lib python34mw_d.lib
c:\MinGW\bin\dlltool.exe --dllname python34.dll --input-def python34.def --output-lib python34mw.lib
@ECHO OFF
SET USRINPUT=

