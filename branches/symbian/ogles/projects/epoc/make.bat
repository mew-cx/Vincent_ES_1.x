@echo off
set pkg1=
set pkg2=

if "%1"=="" goto help
if "%2"=="" goto help
if "%3"=="" goto help

if "%1"=="s60" goto build_s60
if "%1"=="uiq" goto build_uiq

:help
echo Usage: make.bat ui platform build
echo ui:
echo   s60 uiq
echo platform:
echo   wins armi
echo build:
echo   udeb urel
goto end

:build_s60
echo Building for S60 target...
copy /y tests60.mmp test.mmp
copy /y ..\..\test\epoc\tests60.rss ..\..\test\epoc\test.rss
set pkg1=gles_cl60.pkg
set pkg2=test60.pkg
goto do_build

:build_uiq
echo Builing for UIQ target...
copy /y testuiq.mmp test.mmp
copy /y ..\..\test\epoc\testuiq.rss ..\..\test\epoc\test.rss
set pkg1=gles_cl.pkg
set pkg2=test.pkg
goto do_build

:do_build
call bldmake bldfiles
call abld export
call abld build %2 %3
call abld test build %2 %3

if "%2"=="wins" goto end
call makesis %pkg1%
call makesis %pkg2%

set pkg1=
set pkg2=
:end