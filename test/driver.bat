@echo off
@setlocal

pushd %~dp0
pushd ..\x64\Debug\
echo "TEST START >>>> >>>> >>>> >>>>"
set test_folder=..\..\test

@REM %test_folder%\test.bat
@REM timeout 1

icl.exe -i %test_folder%\arithmetic.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

icl.exe -i %test_folder%\operator.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

icl.exe -i %test_folder%\control.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

icl.exe -i %test_folder%\scope.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

icl.exe -i %test_folder%\declaration.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

icl.exe -i %test_folder%\buildin.ich > %test_folder%\out.bat
call %test_folder%\out.bat
timeout 1

%test_folder%\error_test.bat
timeout 1



echo "TEST FINISH <<<< <<<< <<<< <<<<"

popd
popd

