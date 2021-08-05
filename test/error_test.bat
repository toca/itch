@echo off
@setlocal
@REM cd %~dp0
echo ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ==== ====
set num_err=0

@REM func syntax
call :fail "func main(): int {{ return 42; }"
call :fail "func (): int { return 42; }"
call :fail "func main() int { return 42; }"
call :fail "func main() : { return 42; }"
call :fail "func main(i): int {return 42;}"
call :fail "func main(i: ): int{ return 42; }"
call :fail "func main(i: int, b:): int { return 42; }"

@REM declaration
call :fail "func main(): int { var 1i: int; return 0; }"
call :fail "func main(): int { var @i: int; return 0; }"
call :fail "func main(): int { var ?i: int; return 0; }"
call :fail "func main(): int { var #i: int; return 0; }"
call :fail "func main(): int { var i: int; var I: int; return 0; }"
call :fail "func main(): int { var s: string; var S: string; return 0; }"


@REM assign
call :fail "func main(): int { var b: bool = 1; return 0; }"
call :fail "func main(): int { var b: bool ="foo"; return 0; }"
call :fail "func main(): int { var s: string = 1; return 0; }"
call :fail "func main(): int { var s: string = true; return 0; }"
call :fail "func main(): int { var i: int = true; return 0; }"
call :fail "func main(): int { var i: int = "foo"; return 0; }"


@REM if condition
call :fail "func main(): int { if (1) { return 3; } return 5; }"
call :fail "func main(): int { if ("hello") { return 3; } return 5; }"
call :fail "func main(): int { if (!1) { return 3; } return 5; }"
@REM if syntax
call :fail "func main(): int { if (true) return 3; return 5; }"
call :fail "func main(): int { if (true) { return 3; } else return 5; }"

@REM op - (unary)
call :fail "func main(): int { var s: string = -"foo"; return 42; }"
call :fail "func main(): int { var b: bool = -true; return 42; }"
@REM op !
call :fail "func main(): int { var i: int = !10; return 42; }"
call :fail "func main(): int { var s: string = !"foo"; return 42; }
@REM op +
call :fail "func main(): int { var s: string = "foo" + "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true + false; return 42; }"
@REM op -
call :fail "func main(): int { var s: string = "foo" - "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true - false; return 42; }"
@REM op *
call :fail "func main(): int { var s: string = "foo" * "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true * false; return 42; }"
@REM op /
call :fail "func main(): int { var s: string = "foo" / "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true / false; return 42; }"
@REM op %
call :fail "func main(): int { var s: string = 1 % "bar"; return 42; }"
call :fail "func main(): int { var b: bool = 2 % false; return 42; }"
@REM op =
call :fail "func main(): int { "foo" = "foo"; return 42; }"
@REM op +=
call :fail "func main(): int { var s: string = "foo"; s += "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true; b += false; return 42; }"
@REM op -=
call :fail "func main(): int { var s: string = "foo"; s -= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true; b -= false; return 42; }"
@REM op *=
call :fail "func main(): int { var s: string = "foo"; s *= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true; b *= false; return 42; }"
@REM op /=
call :fail "func main(): int { var s: string = "foo"; s /= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true; b /= false; return 42; }"
@REM op %=
call :fail "func main(): int { var s: string = "foo"; s %%%%= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true; b %%%%= false; return 42; }"

@REM op <
call :fail "func main(): int { var s: string = "foo" < "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true < false; return 42; }"
@REM op <=
call :fail "func main(): int { var s: string = "foo" <= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true <= false; return 42; }"
@REM >
call :fail "func main(): int { var s: string = "foo" > "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true > false; return 42; }"
@REM >=
call :fail "func main(): int { var s: string = "foo" >= "bar"; return 42; }"
call :fail "func main(): int { var b: bool = true >= false; return 42; }"
@REM ==
call :fail "func main(): int { var s: string = "foo" == 1; return 42; }"
call :fail "func main(): int { var b: bool = false == "foo"; return 42; }"
@REM !=
call :fail "func main(): int { var s: string = true != 1; return 42; }"
call :fail "func main(): int { var b: bool = "bar" == true; return 42; }"

@REM function
call :fail "func f(): void { return 1; } func main(): int { return 42; }"
call :fail "func f(): int { return "foo"; } func main(): int { return 42; }"
call :fail "func f(): void {} func F(): void {} func main(): int {return 42; }"
call :fail "var f: int; func f(): void {} func main(): int { return 42; }"
@REM func-call
call :fail "func f(): void { } func main(): int { f(1); return 42; }"
call :fail "func f(i: int): void {} func main(): int { f(); return 42; }"
call :fail "func f(i: int): void {} func main(): int { f(true); return 42; }"
call :fail "func f(): int {return 42;} func main(): int { var s: string = f(); return 42; }"
call :fail "func f(): void {} func main(): int { var i: int = f(); return 42; }"
@REM return 
call :fail "func f(): int { if(true) { return 1; } } func main(): int { return 42; }"
call :fail "func f(): int { if(true) { } else { return 2; } } func main(): int { return 42; }"
call :fail "func f(): int { if(true) { if (true) { return 1; } } else { return 2; } } func main(): int { return 42; }"

@REM const
call :fail "func main(): int { const i = 42; i++; return i; }"


if %num_err%==0 (
    echo [32m+---- ---- ---- ----+[0m
    echo [32m^|        OK         ^|[0m
    echo [32m+---- ---- ---- ----+[0m
) else (
    echo [31m+---- ---- ---- ----+[0m
    echo [31m^|        NG! [%num_err%]    ^|[0m
    echo [31m+---- ---- ---- ----+[0m
)
endlocal
exit /b

:fail
    set "arg=%~1"
    set "arg=%arg:&=^&%"
    set "arg=%arg:|=^|%"
    set "arg=%arg:<=^<%"
    set "arg=%arg:>=^>%"
    set "arg=%arg:(=^(%"
    set "arg=%arg:)=^)%"

    echo "%arg%"
    echo %arg%> in.ich
    call cmd /c icl.exe -i in.ich > out.bat
    if 0==%ERRORLEVEL% (
        set /a num_err+=1
        echo [31m"unexpected success: %arg%"[0m
    )
exit /b