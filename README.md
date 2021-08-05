# itch
a programming language that is compiled into a windows batch file.

itch は windows バッチファイルを生成するためのシンプルなプログラミング言語です。コンパイラは windows バッチファイルを生成します

型、定数、レキシカルスコープなどの機能を提供しバッチファイルにつきものの複雑なテクニックは自動生成させることで、高速に安全にバッチファイルを作成することが可能です

# Table of Contents
- [getting started](#getting%20started)
    - [compile](##compile)
    - [hello world](##hello%20world)
    - [main](##main)
- [spec](#spec)
    - [variable](##variable)
        - [var](###var)
        - [const](###const)
    - [type](##type)
        - [int](###int)
        - [string](###string)
        - [bool](###bool)
        - [func](###func)
    - [if,else](###if,%20else)
    - [for](###for)
    - [operator](###operator)
    - [escape sequence](##escape%20sequence)
    - [substitution](##substitution)
    - [comment](##comment)
    - [command-literal](##command-literal)
- [built in functions](#built-in-functions)

# getting started
## compile
* コンパイラをダウンロード (TBA) or ビルドする
* `>icl.exe sample/fizzbuzz.ich` ... fizzbuzz.bat が生成される
* `>fizzbuzz.bat` ... fizzbuzz 実行
## hello world
```
func main(): void {
    print("Hello world");
}
```
## main
* main 関数がエントリーポイントとなります
* main から返された値はプロセス（バッチファイル）の戻り値となります

# spec
## variable
* 変数、定数
* 実体は環境変数です、そのため変数名は大文字と小文字が区別されません
### var 
```
var i: int;
var s: string;
```
* `var 変数名: 型;` のように定義します
```
var i: int = 42;
```
* 初期値を指定することが可能です
```
var s: = "hello";
```
* 初期値が与えられる場合には型を省略することが可能です

### const
```
const MESSAGE: string = "thank you";
```
* `const 変数名: 型 = 値` のように定義します
```
// const MESSAGE: string; <- Error
const MESSAGE: string = "thank you"; valid
```
* var と異なり値の指定が必須です
```
const MESSAGE: string = "thank you";
MESSAGE = "bye"; // <- Error
```
* 再代入することは出来ません

## type
### int
* 整数を表す型です
* 利用可能な範囲は -2,147,483,648 ～ 2,147,483,647 (32bit integer)
### string
* 文字列を表す型です
* リテラル文字列は ダブルクォーテーションで囲んで表現します 
* コマンドプロンプトの制約上利用可能な最大の文字列長は8000文字ほどとなっています
### bool
* 真偽値を表す型です
* 真 ... true
* 偽 ... false
## func
* 関数を表す型です
```
func add(x: int, y: int): int;
```
* `func 関数名(仮引数名: 型, ...): 戻り値の型` のように記述します

## function
* 関数です
```
func add(x: int, y: int): int {
    return x + y;
}
add(39, 3); // -> 42
```
## if, else
* if ... if 文です
```
if (condition) {
    print("truthy");
}
```
* if の条件は bool 型のみ指定可能です

* if - else ...
```
if (condition) {
    print("truthy");
} else {
    print("falsy");
}
```
* if, else ともにブロック({})が必要です
```
if (cond) print("NG"); // Error
if (cond) { print("OK"); } // OK
```

## for
* for 文です
```
for (var i = 0; i < 10; i++) {
    print("Thank you");
}
```
* `for(初期化; 継続条件; 更新 { ... }` のように記述します
* for 文はブロックが必須です
```
for(var i = 0; i < 10; i++) print("Thank you"); // Error
for(var i = 0; i < 10; i++) { print("Thank you"); } // OK
```
## operator
* '+' 正の整数を表す単項演算子
* '-' 負数を表す単項演算子

* '+' 数値の加算、文字列の連結
* '-' 数値の減算
* '*' 数値の乗算
* '/' 数値の除算
* '%' 数値の剰余演算
* '=' 代入
* '+=' 数値の加算代入、文字列の連結代入
* '-=' 数値の減算代入
* '*=' 数値の乗算代入
* '/=' 数値の除算代入
* '%=' 数値の剰余演算代入
* '<' 数値の比較演算 小なり
* '>' 数値の比較演算 大なり
* '<=' 数値の比較演算 小なりイコール
* '>=' 数値の比較演算 大なりイコール
* '++' 数値のインクリメント（後置のみ）
* '--' 数値のデクリメント（後置のみ）

* 比較演算はbool値を返します
* 代入演算は値を返しません

## escape sequence
* "         ->  \"
* %         ->  %%
* 改行      ->  \n ... (プログラム中でのみ改行として動作します)
* タブ文字  ->  \t
* \`        -> \\` ... (コマンドリテラル中のみ)
## substitution
* %変数名%
```
var message="Thank you";
print("%message% for your kindness");
```
## comment
* `// ここはコメント`
* ブロックコメントはありません

## command-literal
* プログラム中にそのままコマンド（バッチ）を記述することが可能です
```
print("hello");
`echo world`;
// -> hello
// -> world
```


## keywords
* int, string, bool, void, func, return, for, if, else, break, continue, switch, case
* 連続する2つのアンダースコア からはじまるラベルは自動生成されるラベルと重複するため利用禁止
* ICH_ からはじまる変数は禁止


# built-in-functions
### print(message: string): void
* 標準出力へメッセージを出力します
* プログラム内部で記述されたか改行コード (\n) を認識します 
```
    print("hello world");
    // -> hello world
    print("new\nline");
    // -> new
    // -> line
```

### cprint(message: string, color: int)
* ANSI エスケープシーケンスを利用し指定された色でメッセージを出力します
* color = ICH_BLACK | ICH_RED | ICH_GREEN | ICH_YELLOW | ICH_BLUE | ICH_MAGENTA | ICH_CYAN | ICH_WHITE
```
    cprint("black",    ICH_BLACK);
    cprint("red",      ICH_RED);
    cprint("green",    ICH_GREEN);
    cprint("yellow",   ICH_YELLOW);
    cprint("blue",     ICH_BLUE);
    cprint("magenta",  ICH_MAGENTA);
    cprint("cyan",     ICH_CYAN);
    cprint("white",    ICH_WHITE);
```
* ANSIエスケープシーケンスによるカラー表示に対応した環境でのみ正しく表示されます

### printerr(message: string)
* 標準エラー出力へメッセージを出力します
```
    printerr("ERROR: an error has occurred!");
```

### cprinterr(message: string, color: int)
* 標準エラー出力へ指定された色でメッセージを出力します
* color = ICH_BLACK | ICH_RED | ICH_GREEN | ICH_YELLOW | ICH_BLUE | ICH_MAGENTA | ICH_CYAN | ICH_WHITE
```
    cprinterr("ERROR: an error has occurred!", ICH_RED);
```

### exec(command: string): string
* コマンドを実行し出力を返す
```
    var current_dir = exec("cd");
    print("%current_dir%");
```
* 標準出力へは出力されません。戻り値として出力を受け取ることが出来ます
* 出力が複数行あった場合最後の行が戻り値になります (複数行の結果を利用したい場合は`system` が利用可能です)
* command に " (ダブルクォーテーション), \` (バッククォート) 含む文字列を渡した場合処理は失敗します (`execq`の利用を検討してください)

### execq(command: string, subst: string): string
* コマンドを実行し出力を返す
* あらかじめ command に含まれる subst と一致する部分を " (ダブルクォーテーション) に置換します
```
    var out = execq("echo 'hello'", "'");
    print(out);
    // -> "hello"
```
* 標準出力へは出力されません。戻り値として出力を受け取ることが出来ます
* 出力が複数行あった場合最後の行が戻り値になります (複数行の結果を利用したい場合は`systemq` が利用可能です)
* subst に * (アスタリスク) を含む文字列を渡した場合処理は失敗します
* command に " (ダブルクォーテーション), \` (バッククォート) 含む文字列を渡した場合処理は失敗します

### thru(command: string): int
* コマンドを実行し、コマンドの戻り値 (ERRORLEVEL) を返します
* exec, system と異なり標準出力への出力が行われます
```
    var i = thru("choice /M 'Yes or No'");
    if (i == 1) {
        print("Yes!");
    } else {
        print("No!");
    }
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します

### thruq(command: string, subst: string): int
* コマンドを実行し、コマンドの戻り値 (ERRORLEVEL) を返します
* あらかじめ command に含まれる subst と一致する部分を " (ダブルクォーテーション) に置換
* exec, system と異なり標準出力への出力が行われます
```
    var i = thru("where test.txt");
    if (i == 0) {
        print("Found!");
    } else {
        print("Not Found!");
    }
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します
* subst に * (アスタリスク) 、を含む文字列を渡した場合処理は失敗します

### system(command: string, callback: func(strin): void): void
* コマンドを実行し、出力毎にコールバック関数を呼び出します
```
    system("dir", func(line: string): void {
        print("dir: %line%");
    });
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します

### systemq(command: string, subst: string, callback: func(strin): void): void
* コマンドを実行し、出力毎にコールバック関数を呼び出します
* あらかじめ command に含まれる subst と一致する部分を " (ダブルクォーテーション) に置換します
```
    system("findstr 'foo bar' test.txt", "'", func(line: string): void {
        print("dir: %line%");
    });
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します
* subst に * (アスタリスク) 、を含む文字列を渡した場合処理は失敗します

### start_proc(command: string): void
* プロセスを起動します
* 起動したプロセスの完了をつことなく start_proce は処理を終了します
```
    start_pcoc
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します

### start_procq(command: string, subst: string): void
* プロセスを起動します
* あらかじめ command に含まれる subst と一致する部分を " (ダブルクォーテーション) に置換します
* 起動したプロセスの完了をつことなく start_proce は処理を終了します
```
    start_pcoc
```
* command に " (ダブルクォーテーション) を含む文字列を渡した場合処理は失敗します



### read_file(file_path: string, callback: func(string): void): void
* 指定されたファイルを読み込み、1行毎にコールバック関数を呼び出します
```
    read_file("foo.txt", func(line: string): void {
        print("foo: %line%");
    });
```

### list_files(dir: string, callback: func(string): void): void
* 指定ディレクトリの中にあるファイルの一覧を取得し、ファイル毎にコールバック関数を呼び出します
```
    list_files("my_folder", func(file: string): void {
        print("file: %file%");
    });
```

### list_folders(dir: string, callback: func(string): void): void
* 指定ディレクトリの中にあるフォルダの一覧を取得し、フォルダ毎にコールバック関数を呼び出します
```
    list_folders(".", func(folder: string): void {
        print("current: %folder%");
    });
```

### find_files(query: string, callback: func(string): void): void
* 指定された名前のファイルを探し、1行ごとにコールバック関数を呼び出します
* ワイルドカードが利用可能です
```
    find_files("*.txt", func(file: string): void {
        print("find_file: %file%");
    });
```

### find_folders(query: string, callback: func(string): void): void
* 指定された名前のフォルダを探し、1行ごとにコールバック関数を呼び出します
* ワイルドカードが利用可能です
```
    find_folders("*test*", func(folder: string): void {
        print("find_folder: %folder%");
    });
```

### fully_qualify(name: string): string
* 指定されたファイル/フォルダ名を完全修飾パス名に展開します
```
    var fullpath = fully_qualify(".\test");
    print(fullpath);
    // -> C:\temp\test
```

### to_path(name: string): string
* 指定されたパスからファイル名を取り除いたパス名へ変換します
```
    var path = to_path("C:\\Users\\test\\temp\\sample.txt");
    print(fullpath);
    // -> C:\Users\test\temp
```

### to_file_name(name: string): string
* 指定されたパスからパスを取り除いたファイル名へ変換します
```
    var file = to_file_name("C:\\Users\\test\\temp\\sample.txt");
    print(file);
    // -> sample.txt
```

### to_extension(path: string): string
* 指定されたパスから拡張子のみを取り出します
```
    var e = to_extension("C:\\Users\\test\\temp\\sample.txt");
    print(e);
    // -> .txt
```

### str_replace(src: string, old: string, new: string): string
* 文字列 src に含まれる old を new で置き換えた文字列を返します
```
    var replaced = str_replace("hello name", "name", "world");
    print(replaced);
    // -> hello world
```
* 引数内に " ダブルクォーテーションを含む置換は失敗します
* 引数内に * アスタリスクを含む置換は失敗します
* 引数内に = 等号を含む置換は失敗します

## str_replace_ex(str: string, old: string, new: string): string
* 文字列 src に含まれる old を new で置き換えた文字列を返します
```
    var replaced = str_replace_ex("hello \"world\"", "\"", "'");
    print(replaced);
    // -> hello 'world'
```
* 引数内に " ダブルクォーテーションを含む置換は失敗します



### substr(src: string, offset: int, len: int): string
* 部分文字列 offset を開始位置として、len で指定された長さ分の文字列を返します
* offset にマイナスの値を指定した場合は src の末尾から数えて |offset| 文字目から開始します
* len にマイナスの値を指定した場合は末尾から len 文字分を除いた文字列を取得します
```
    var s1 = substr("abcdefg", 1, 3);
    print(s1); // -> bcd
    var s2 = substr("abcdefg", -5, 4);
    print(s2); // -> cdef
    var s3 = substr("abcdefg", 3, -1);
    print(s3); // -> def
    var s4 = substr("abcdefg", -4, -2);
    print(s4); // -> de
```
* src に " ダブルクォーテーションを含む文字列を指定した場合処理は失敗します

### str_cut(src: string, offset: int, len: int): string
* 部分文字列 offset を開始位置する文字列を返します
* offset にマイナスの値を指定した場合は src の末尾から数えて |offset| 文字目から開始します
```
    var s1 = str_cut("abcdefg", 4);
    print(s1); // -> efg
    var s2 = str_cut("abcdefg", -5);
    print(s2); // -> cdefg
```
* src に " ダブルクォーテーションを含む文字列を指定した場合処理は失敗します

### str_split(src: string, delim: string, callback: func(string): void): void
* 文字列を指定された区切り文字で分割し、要素毎にコールバック関数を呼び出します
```
    str_split("foo,bar,baz", ",", func(str: string): void {
        print(str);
    });
    // -> foo
    // -> bar
    // -> baz
```

### str_len(src: string): int
* 文字列の長さを取得します
```
    var s = "12345";
    var len1 = str_len(s);
    print("length: %len1%"); // -> 5

    var len2 = str_len("foobarbuz");
    print("length: %len2%"); // -> 6
```

### str_empty(src: string): bool
* src が空文字列の場合 true それ以外の場合 false を返します


### str_equals(lhs: string, rhs: string): bool
* 大文字と小文字を区別せず文字列を比較します
* 2つの文字列が同じである場合 true をそうではない場合 false を返します
* == による比較は大文字と小文字を区別しますが、この関数では "FOO" と "foo" は同値と評価します
```
    if (str_equals("FOO", "foo")) {
        print("str_equals: equal");
    }
    if ("FOO" == "foo") {
        print("==: equal");
    } else {
        print("==: not");
    }
```

### env_var(name: string): string
* 環境変数を取得します
```
    var dt = env_var("date");
    print(dt);
    // -> 2021/01/01
    var tm = env_var("time");
    print(tm);
    // -> 12:34:56.78
    var rd = env_var("random");
    print(rd);
    // -> 1234
    var current_dir = env_var("cd");
    print(current_dir);
    // -> C:\Users\test
```

### s_to_i(str: string): int
* 文字列から整数への変換を行います (string to integer)
```
    var s = "123";
    var i = s_to_i(s);
    print("i: %i%");
    // -> i: 123
    s_to_i("0xff"); // 255
```

### i_to_s(num: int): string
* 数値から文字列への変換を行います
```
   print(i_to_s(42));
   // -> 42
```

* error (pause?)
* str_match?
* starts_with
* ends_with
* array, at, len, push, pop