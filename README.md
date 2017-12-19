# MQOutlineGen
メタセコで裏面ポリゴンを簡単につくるプラグイン

![outlinegen1](https://raw.githubusercontent.com/devil-tamachan/MQOutlineGen/master/outlinegen1.jpg)

## ダウンロード (Win/MacOSX)：

 - https://github.com/devil-tamachan/MQOutlineGen/releases
 
##  インストール方法：

#### Windows 32/64ビット版インストール方法:

 - 上記リンクからダウンロードしたzipを解凍する
 - メタセコ画面上部のメニュー → ヘルプ → プラグインについて → インストールボタンをクリック。MQOutlineGen.dll または MQOutlineGen64.dllを選択してインストール

#### MacOSX版インストール方法:

 - 上記リンクからダウンロードしたzipを解凍する
 - メタセコ画面上部のメニュー → ヘルプ → プラグインについて → インストールボタンをクリック。MQOutlineGen.pluginを選択してインストール
 
## 使い方：

 - "shadow"という名前のマテリアルを作る
   - 裏面ポリゴンはこの材質が適用されます
 - 裏面ポリゴンを作りたいポリゴンだけ表示する
 - メニュー - オブジェクト - OutlineGen を実行
 - "OutlineGenPlugin" という名前のオブジェクトができる
 
## 使い方２：

 - 元のメッシュをいじったあとまた裏面ポリゴンを作りたい
 - 再度OutlineGenを実行すると、"OutlinGenPlugin"オブジェクトに上書きされます。前回作ったメッシュは破棄されるので注意
 - 前回のを残しておきたい場合は事前にリネームしておいてください
 - ミラーの設定はそのまま受け継がれます

## 注意点：

 - 単ポリ、両面ポリはうまく裏面ポリゴンが作られません
 - ミラー有りとミラーなしの分離機能は今のところありません
 
