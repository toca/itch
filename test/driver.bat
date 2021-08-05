@echo off
@setlocal

pushd %~dp0
pushd ..\x64\Debug\
echo "TEST START >>>> >>>> >>>> >>>>"
set test_folder=..\..\test

@REM %test_folder%\test.bat
@REM timeout 1

@REM icl.exe -i %test_folder%\arithmetic.ich -o %test_folder%\out.bat
@REM call %test_folder%\out.bat
@REM timeout 1

icl.exe -i %test_folder%\operator.ich -o %test_folder%\out.bat
call %test_folder%\out.bat
@REM timeout 1

@REM icl.exe -i %test_folder%\control.ich -o %test_folder%\out.bat
@REM call %test_folder%\out.bat
@REM timeout 1

@REM icl.exe -i %test_folder%\scope.ich -o %test_folder%\out.bat
@REM call %test_folder%\out.bat
@REM timeout 1

@REM icl.exe -i %test_folder%\declaration.ich -o %test_folder%\out.bat
@REM call %test_folder%\out.bat
@REM timeout 1

@REM icl.exe -i %test_folder%\buildin.ich -o %test_folder%\out.bat
@REM call %test_folder%\out.bat
@REM timeout 1

@REM %test_folder%\error_test.bat
@REM timeout 1



echo "TEST FINISH <<<< <<<< <<<< <<<<"

popd
popd

