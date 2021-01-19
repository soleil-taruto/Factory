Factory
=======

Cで書いたツールとライブラリの寄せ集めです。
必要に応じて必要な機能を書き足し続けてきたものなので雑然としています。
実行形式を含むアーカイブは以下の場所から落とせます。

http://stackprobe.dip.jp/_kit/Factory


開発環境
--------

Windows 10 (Pro | Home)
Visual C++ 2010 Express | Visual Studio Community 2019

システムドライブ = C


フォルダ構成
------------

一部のプログラムが以下のパス名を一時ディレクトリとして使用します。

	C:\1, C:\2, C:\3, ... C:\999


定番のフォルダ (自分用メモ)

	---> _memo_Hierarchy.txt


ビルド手順
----------

1. Windows 10 Pro または Windows 10 Home 環境を用意する。

	1.1. アップデートを全て適用する。

2. Visual C++ 2010 Express または Visual Studio Community 2019 をインストールする。

	Visual Studio Community 2019 の場合「ワークロード」で "C++ によるデスクトップ開発" を選択する。

	2.1. アップデートを全て適用する。

3. 全てのファイルをローカルに展開する。

	このファイルが C:\Factory\Readme.txt となるように配置する。

4. コンソールを開く。

	4.1. OpenConsole.bat を実行する。

5. 以下のコマンドを実行する。-- 全てのソースをコンパイル・リンクする。

	> ff
	> cd build
	> cd _cx
	> rebuild
	> ff
	> cx **

6. 以下のコマンドを実行する。-- 改行コードをCR-LFにする。

	> fcrlf

7. 以下のコマンドを実行する。-- 一時ファイルを削除する。

	> zz


補足
----

/J コンパイラオプションにより char は unsigned char になります。

習慣で unsigned を多用しています。

型の想定

	---> _memo_Types.txt

