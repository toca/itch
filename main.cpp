
#include <cstdio>
#include <fstream>
#include <filesystem>
#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"
#include "tokens.h"
#include "error.h"

constexpr wchar_t VERSION[] = L"0.1.0";
std::wstring source{};
std::vector<Node*> nodes{};


std::wstring builtin_functions()
{
    std::wstring s = LR"(
// ==== ====  Built-in-Functions  ==== ====

// command ====================================================
func exec(command: string): string {
    `set "command=%command:&=^&%"`;
    `set "command=%command:|=^|%"`;
    `set "command=%command:<=^<%"`;
    `set "command=%command:>=^>%"`;
    `set "command=%command:(=^(%"`;
    `set "command=%command:)=^)%"`;

    var result: string;
    `for /F "usebackq delims=" %%A in (\`%command%\`) do set "result=%%A"`;
    return result;
}

func execq(command: string, subst: string): string {
    `set "command=%command:&=^&%"`;
    `set "command=%command:|=^|%"`;
    `set "command=%command:<=^<%"`;
    `set "command=%command:>=^>%"`;
    `set "command=%command:(=^(%"`;
    `set "command=%command:)=^)%"`;
    `call set "command=%%command:%subst%="%%"`;
    var result: string;
    `for /F "usebackq delims=" %%A in (\`%command%\`) do set "result=%%A"`;
    return result;
}

func thru(command: string): int {
    var result: int;
    `%command%`;
    `set result=%ERRORLEVEL%`;
    return result;
}

func thruq(command: string, subst: string): int {
    var result: int;
    `call set "command=%%command:%subst%="%%"`;
    `%command%`;
    `set result=%ERRORLEVEL%`;
    return result;
}


func system(command: string, callback: func(string): void): void {
    `set "command=%command:&=^&%"`;
    `set "command=%command:|=^|%"`;
    `set "command=%command:<=^<%"`;
    `set "command=%command:>=^>%"`;
    `set "command=%command:(=^(%"`;
    `set "command=%command:)=^)%"`;

    `for /f "usebackq delims=" %%A in (\`%command%\`) do call :%callback% "%%A"`;
}

func systemq(command: string, subst: string, callback: func(string): void): void {
    `set "command=%command:&=^&%"`;
    `set "command=%command:|=^|%"`;
    `set "command=%command:<=^<%"`;
    `set "command=%command:>=^>%"`;
    `set "command=%command:(=^(%"`;
    `set "command=%command:)=^)%"`;
    `call set "command=%%command:%subst%="%%"`;
    `for /f "usebackq delims=" %%A in (\`%command%\`) do call :%callback% "%%A"`;
}


func start_proc(command: string): void {
    `start %command%`;
}

func start_procq(command: string, subst: string): void {
    `call set "command=%%command:%subst%="%%"`;
    `start %command%`;
}

// print ======================================================

)";
    s+=
L"func print(arg: string) : void {\n"
L"    `set arg=%*`;\n"
L"    `set \"arg=%arg:~1,-1%\"`;\n"
L"    `if not defined arg echo. & endlocal & exit /b`;\n"
L"    `set \"trimed=%arg: =%\"`;\n"
L"    `if not defined trimed echo. & endlocal & exit /b`;\n"
L"    `set \"trimed=%arg:\t=%\"`;\n"
L"    `if not defined trimed echo. & endlocal & exit /b`;\n"
L"    `set \"arg=%arg:&=^&%\"`;\n"
L"    `set \"arg=%arg:|=^|%\"`;\n"
L"    `set \"arg=%arg:<=^<%\"`;\n"
L"    `set \"arg=%arg:>=^>%\"`;\n"
L"    `set \"arg=%arg:(=^(%\"`;\n"
L"    `set \"arg=%arg:)=^)%\"`;\n"
L"    `set \"arg=%arg:\034=& echo.%\"`;\n"
L"    `echo %arg%`;\n"
L"}\n"

L"func printerr(arg: string) : void {\n"
L"    `if not defined arg (`;\n"
L"    `    echo. 1>&2`;\n"
L"    `    exit /b`;\n"
L"    `)`;\n"
L"    `set \"arg=%arg:&=^&%\"`;\n"
L"    `set \"arg=%arg:|=^|%\"`;\n"
L"    `set \"arg=%arg:<=^<%\"`;\n"
L"    `set \"arg=%arg:>=^>%\"`;\n"
L"    `set \"arg=%arg:(=^(%\"`;\n"
L"    `set \"arg=%arg:)=^)%\"`;\n"
L"    `set \"arg=%arg:\"\"=\"%\"`;\n"
L"    `set \"arg=%arg:\034=& echo.%\"`;\n"
L"    `echo %arg% 1>&2`;\n"
L"}\n";

    s += LR"(
// const COLORS
const ICH_BLACK   = 30;
const ICH_RED     = 31;
const ICH_GREEN   = 32;
const ICH_YELLOW  = 33;
const ICH_BLUE    = 34;
const ICH_MAGENTA = 35;
const ICH_CYAN    = 36;
const ICH_WHITE   = 37;
)";
    
    s +=
L"func cprint(arg: string, color : int): void {\n"
L"    print(\"\033[%color%m%arg%\033[0m\");\n"
L"}\n"

L"func cprinterr(arg: string, color : int): void {\n"
L"    printerr(\"\033[%color%m%arg%\033[0m\");\n"
L"}\n";
    
    s += LR"(

// string =====================================================

func str_replace(src: string, old: string, new: string): string {
    var result: string;
    `call set "result=%%src:%old%=%new%%%"`;
    return result;
}

func str_replace_ex(str: string, old: string, new: string): string {
    var result: string;
    `if "%old%"=="_" (`;
    `    set "str=-%str%-"`;
    `) else (`;
    `    set "str=_%str%_"`;
    `)`;
    `:__split_ex_loop__`;
    `for /F "tokens=1* delims=%old%" %%A in ("%str%") do (`;
    `    set "cur=%%A"`;
    `    set "str=%%B"`;
    `)`;
    `if defined str (`;
    `    set "result=%result%%cur%%new%"`;
    `    goto __split_ex_loop__`;
    `) else (`;
    `    set "result=%result%%cur%"`;
    `)`;
    `set "result=%result:~1,-1%"`;
    return result;
}

func substr(src: string, offset: int, len: int): string {
    var result: string;
    `call set "result=%%src:~%offset%,%len%%%"`;
    return result;
}

func str_cut(src: string, offset: int): string {
    var result: string;
    `call set "result=%%src:~%offset%%%"`;
    return result;
}

func str_split(str: string, delim: string, callback: func(string): void): void {
    `:__split_loop__`;
    `for /f "tokens=1* delims=%delim%" %%A in ("%str%") do (`;
    `   call :%callback% "%%A"`;
    `   set "str=%%B"`;
    `)`;
    `if defined str goto __split_loop__`;
}

func str_len(str: string): int {
    var len = 0;
    `:__str_len_loop`;
    `if not "%str%"=="" (`;
        `set str=%str:~1%`;
        `set /a len+=1`;
        `goto :__str_len_loop`;
    `)`;
    return len;
}

func str_empty(str: string): bool {
    var result = false;
    `:__str_empty_loop`;
    `if not defined str set "result=0==0" & goto :__str_empty_done`;
    `set "c=%str:~0,1%"`;
    `if not defined c set "result=0==0" & goto :__str_empty_done`;
    `set "str=%str:~1%"`;
    `if "%c%"==" " goto :__str_empty_loop`;
    `if "%c%"=="	" goto :__str_empty_loop`;
    `:__str_empty_done`;
    return result;
}

func str_equals(lhs: string, rhs: string): bool {
    var result = false;
    `if /I "%lhs%"=="%rhs%" SET "result=0==0"`;
    return result;
}


// file, dir ==================================================

func read_file(file: string, callback: func(string): void): void {
    `if exist "%file%\*" (`;
    `    call :printerr "read_file: '%file%' is folder!!"`;
    `    pause`;
    `)`;
    `if not exist "%file%" (`;
    `    call :printerr "read_file: file '%file%' not found!!"`;
    `    pause`;
    `)`;
    `for /f "tokens=1* delims=: eol=" %%A in ('findstr /n "^" %file%') do (`;
        var line: string;
        `set "line=%%B"`;
        callback(line);
    `)`;
}

func list_files(dir: string, callback: func(string): void): void {
    `if not exist "%dir%\*" (`;
    `    call :printerr "list_files: '%file%' not found!!"`;
    `    pause`;
    `)`;
    `for %%A in ("%dir%\*") do (`;
    `    call :%callback% "%%A"`;
    `)`;
}

func list_folders(dir: string, callback: func(string): void): void {
    `if not exist "%dir%\*" (`;
    `    call :printerr "list_folders: '%dir%' not found!!"`;
    `    pause`;
    `)`;
    `for /D %%A in ("%dir%\*") do (`;
    `    call :%callback% "%%A"`;
    `)`;
}

func find_files(dir: string, callback: func(string): void): void {
    `for %%A in ("%dir%") do (`;
    `    call :%callback% "%%A"`;
    `)`;
}

func find_folders(dir: string, callback: func(string): void): void {
    `for /D %%A in ("%dir%") do (`;
    `    call :%callback% "%%A"`;
    `)`;
}



func fully_qualify (path: string): string {
    var result: string;
    `set "result=%~f1"`;
    return result;
}

func to_path(path: string): string {
    var result: string;
    `set "result=%~dp1"`;
    return result;
}

func to_file_name(path: string): string {
    var result: string;
    `set "result=%~nx1"`;
    return result;
}

func to_extension(path: string): string {
    var result: string;
    `set "result=%~x1"`;
    return result;
}

// util
func env_var(name: string): string {
    var result: string;
    `set name=!%name%!`;
    `setlocal enabledelayedexpansion`;
    `set result=%name%`;
    `endlocal & call set result=%result%`;
    return result;
}

func i_to_s(num: int): string {
    var result: string;
    `set result=%num%`;
    return result;
}

func s_to_i(str: string): int {
    var result: int;
    `set /A result=str+0`;
    return result;
}
)";
    return s;
}


std::wstring read_file(const std::wstring& path)
{
    std::wifstream file(path);
    return std::wstring(std::istreambuf_iterator<wchar_t>(file), std::istreambuf_iterator<wchar_t>());
}

enum class Mode {
    HELP,
    VERSION,
    COMPILE,
};

int wmain(int argc, wchar_t* argv[])
{
    try
    {
        Mode mode = Mode::COMPILE;
        if (argc <= 1)
        {
            printerr(L"Error:  input file required");
            mode = Mode::HELP;
        }

        std::wstring in_path{};
        std::wstring out_path{};

        for (int i = 0; i < argc; i++)
        {
            if (wcscmp(argv[i], L"-i") == 0) // input
            {
                if (argc < i + 1)
                {
                    printerr(L"input file path required!!");
                    exit(1);
                }
                in_path = argv[++i];
            }
            if (wcscmp(argv[i], L"-o") == 0) // output
            {
                if (argc < i + 1)
                {
                    printerr(L"output file path required!!");
                    exit(1);
                }
                out_path = argv[++i];
            }
            if (wcscmp(argv[i], L"--version") == 0)
            {
                mode = Mode::VERSION;
            }
            if (wcscmp(argv[i], L"--help") == 0)
            {
                mode = Mode::HELP;
            }
        }

        switch (mode)
        {
        case Mode::HELP:
            wprintf(L"%%%% icl itch lang compiler %%%%\n");
            wprintf(L"Useage: icl.exe [Options] <input_file>\n");
            wprintf(L"Options:\n");
            wprintf(L"  -i <file>    input from <file>\n");
            wprintf(L"  -o <file>    output to <file>\n");
            wprintf(L"  --help       show help\n");
            wprintf(L"  --version    show version\n");
            return 0;
        case Mode::VERSION:
            wprintf(L"icl %s\n", VERSION);
            wprintf(L"Copyright\n");
            return 0;
        case Mode::COMPILE:
            // thru
            break;
        }
 


        // input
        if (in_path.empty())
        {
            in_path = argv[1];
        }
        source = read_file(in_path) + L"\n";
        source += builtin_functions();
        
        // output
        if (out_path.empty())
        {
            std::filesystem::path path(in_path);
            out_path = path.stem().wstring() + L".bat";
        }
        FILE* file;
        _wfreopen_s(&file, out_path.c_str(), L"w", stdout);


        // tokenize
        Tokenizer tokenizer(source, in_path);
        Tokens tokens = tokenizer.tokenize();
        //tokens.dump();

        // parse
        Parser parser(tokens, in_path, source);
        auto func = parser.parse();
        ////node_dump();

        // generate
        codegen(func);

        if (file) fclose(file);
        return 0;
    }
    catch (std::exception& err)
    {
        fprintf(stderr, "%s", err.what());
        return 1;
    }
}
