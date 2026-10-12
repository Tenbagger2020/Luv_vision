# Luv_Vision_AlignSystem で使っている C++ / MFC の技術

- 対象:Ver10.21.1.3 のプロジェクト内 .cpp / .h(外部 DLL のヘッダは除く)
- 「件数」は `ファイル数 / 出現数`。コードを grep した数で、目安
- 記載方針:コードで確認できた使い方だけを書く

---

## 1. 全体像

| 区分 | 中身 |
|---|---|
| 言語 | C++(書き方は C に近い。11章) |
| アプリ形態 | MFC のダイアログベースアプリ(`CWinApp` + `CDialogEx`) |
| 文字列 | MFC の `CString` と C の `char[]` が混在。API は ANSI 版(`MessageBoxA`、`GetWindowTextA` など) |
| 画像処理 | WIL(`FVCL` 名前空間)。画面表示は WIL の `CEvView` |
| 並行処理 | MFC のワーカースレッド(`AfxBeginThread`)+ C++11 の `std::mutex` |
| 外部 I/O | Win32 API(Winsock、ini ファイル、ファイル操作)、CONTEC CDIO、各カメラ SDK |
| 設定保存 | ini ファイル(`GetPrivateProfileString` / `WritePrivateProfileString`) |

---

## 2. クラスと構造

| 技術 | 件数 | このコードでの使い方 | 例 |
|---|---|---|---|
| クラス | 191 ファイル | 1ファイル1クラス。画面・処理・計算・設定データごとに分ける | `CCalcAlign`、`CProcSearch` |
| 継承 | — | 自作クラス同士の継承はない。MFC / WIL のクラスを継承するだけ | `CDialogEx` を継承 99、`CWinApp` 1、`CEvView` 1(`CPatternView`) |
| 仮想関数 | 101 / 467 | ほぼ MFC の仮想関数の上書き(`OnInitDialog`、`PreTranslateMessage`、`DoDataExchange`、`DefWindowProc`)。`override` キーワードは使っていない | 全ダイアログ |
| static メンバ関数 | 33 / 238 | スレッドの入口にするため、`UINT 関数(LPVOID)` の形で static にする | `CProcDispatchMain::Dispatch`、`CProcSearch::GoSearch`、`CProcDataRead::DataRead` |
| クラス内の構造体 | 51 / 176 | 設定項目をグループごとにまとめる。構造体の配列も使う | `CCommands::read`、`CScopeParam::view[4]`、`CTempParam::additional` |
| 共用体(union) | 1 / 3 | 2ワード ⇔ 32bit 整数の変換に使う | `CToolTips0::conv1`(`val_w[2]` と `val_i`) |
| 関数の多重定義 | — | 引数の違う同名関数 | `CToolTips0::IntToString`(2通り)、`DblToString`(3通り) |
| 既定引数 | — | 呼び出しを短くする | `CDlg…(CWnd* pParent = nullptr)` |
| `#pragma once` | 195 | ヘッダの二重読み込み防止 | 全ヘッダ |
| `#define` 定数 | 19 / 1079 | ビット番号、状態値、ユーザーメッセージ番号など | `OUT_RDY`、`CMD_RDY`、`WM_APP_COMMAND_RESULT_SUCCESS` |

**使っていないもの**:自作テンプレート、演算子の多重定義、ラムダ式、`auto`、`override`、スマートポインタ(`unique_ptr` / `shared_ptr`)、STL コンテナ(`vector` / `map`)

---

## 3. データの共有

| 技術 | 件数 | 使い方 | 例 |
|---|---|---|---|
| グローバル配列 + `extern` | 141 / 648 | 設定・結果はすべてグローバル配列。使うファイルで `extern` 宣言する | `extern CCommands cmd_param[256];`、`scope_param[16]`、`template_param[256]` |
| グローバルなクラスのインスタンス | — | 共通処理もグローバル変数1つで共有 | `calc_conv`、`calc_plc`、`outlog`、`dio`、`plc_com` |
| 番号(添字)でつなぐ | — | コマンド → テンプレート → スコープ×視野、をすべて番号で参照 | `cmd_param[cmd].template_number` |
| ポインタで別名 | — | 長い配列参照を短く書く | `CTempParam* temp_param = &template_param[n];` |

---

## 4. 関数の書き方

| 技術 | 件数 | 使い方 | 例 |
|---|---|---|---|
| 出力引数(ポインタ) | 多数 | 戻り値は成否(0 / 1)、結果はポインタ引数で返す | `GetMarkGS2(temp_param, &mark_x, &mark_y)`、`GetAxisPosXY(n, &x, &y)` |
| `goto` で出口を1つに | 51 / 700 | 失敗したら関数末尾のラベルへ飛ぶ | `goto END;` … `END: return rtn_result;` |
| エラーコード | — | `WORD` の 16進コードを返す・セットする | `CheckParameter()` が 0x0101 など、`thread_rs.calc_err = 0x9411` |
| switch の流れ落ち | — | `break` を書かずに下の case も実行する | `DispatchCalib` のキャリブ初期化 |
| C 形式のキャスト | 24 / 125 以上 | 数値の型変換 | `(double)data1 / 10000`、`(WORD)dlg->run_cmd` |
| C++ のキャスト | `dynamic_cast` 63、`reinterpret_cast` 79、`static_cast` 40 | スレッド関数で `LPVOID` をダイアログに戻す | `dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam))` |

---

## 5. メモリ管理

| 技術 | 件数 | 使い方 | 例 |
|---|---|---|---|
| 配列の `new[]` / `delete[]` | 38 / 40 | 一時的な配列 | `new ResultArray[count]`、`new int[vert_size]` |
| ダイアログの `new` | 40 | タブの中身(子ダイアログ)を動的に作る | `dlg_TabTilt = new CTabSearchCalcTiltMain(&tab);` |
| スタック上のダイアログ | — | モーダル画面はローカル変数で作って `DoModal` | `CDlgCmdSettingAlign dlg_align; dlg_align.DoModal();` |
| `nullptr` | 199 | MFC ウィザードが生成したコンストラクタの既定引数が中心 | `CWnd* pParent = nullptr` |

- スマートポインタは使っていない。途中の `return` で `delete[]` を通らない箇所がある(ARCHITECTURE.md の ISSUE-13)

---

## 6. 例外

| 技術 | 件数 | 使い方 |
|---|---|---|
| `try` / `catch` | 9 / 28 | WIL の画像読み込みなど一部のみ。`catch (FVCL::CFveException &e)` |

- 基本のエラー処理は例外ではなく、戻り値とエラーコード

---

## 7. MFC:アプリとダイアログ

| 技術 | 件数 | 使い方 | 例 |
|---|---|---|---|
| `CWinApp::InitInstance` | 1 | 起動処理。ダイアログを `DoModal` で開き、閉じたら終了 | `CLuvVisionApp` |
| `CDialogEx` | 99 クラス | すべての画面 | `CDlgSystemMain`、`CDlgCmdSettingAlign` |
| `IMPLEMENT_DYNAMIC` / `DECLARE_DYNAMIC` | 98 | MFC の実行時型情報 | 各ダイアログ |
| `OnInitDialog` | 198 ファイル | 画面を開いたときの初期化 | `ControlInitialization()` → `LoadParameter()` |
| `DoModal`(モーダル) | 23 / 71 | 設定画面を開いて、閉じるまで待つ | メニュー → 各設定画面 |
| `Create`(モードレス) | 14 / 41 | タブの中に子ダイアログを埋め込む | `dlg_TabMain->Create(IDD_…, &tab_setting_param)` |
| `CDialogEx::OnOK` / `OnCancel` | 56 / 91 | OK・キャンセルで閉じる | `OnBnClickedOk` の最後 |
| `PreTranslateMessage` | 194 / 291 | Enter / Esc で画面が閉じないようにする | 各ダイアログ共通の書き方 |
| `SetWindowPos` | 61 / 63 | 画面の位置・大きさを固定値で決める | `this->SetWindowPos(&wndTop, 400, 400, 640, 528, …)` |

---

## 8. MFC:コントロールとメッセージ

**コントロールと変数の結びつけ**

| 技術 | 件数 | 使い方 |
|---|---|---|
| `DDX_Control` | 98 / 1306 | 画面の部品とメンバ変数(`CComboBox` など)を結びつける。主流 |
| `DDX_Text` + `UpdateData` | 10 / 38、12 / 31 | 一部の画面で、値を直接メンバ変数とやり取り |

**使っているコントロール**(件数はファイル数 / 出現数)

| クラス | 件数 | 主な使い方 |
|---|---|---|
| `CButton` | 59 / 215 | ボタン、チェックボックス(`GetCheck` / `SetCheck`) |
| `CStatic` | 63 / 272 | 表示ラベル |
| `CEdit` | 46 / 139 | 数値・文字の入力(`GetWindowTextA` → `atoi` / `atof`) |
| `CComboBox` | 44 / 116 | 選択肢(`AddString`、`SetCurSel`、`GetCurSel`) |
| `CListBox` | 17 / 45 | ログ表示 |
| `CSliderCtrl` | 13 / 27 | 閾値などの調整 |
| `CTabCtrl` | 13 / 14 | タブ切替(`TCN_SELCHANGE` で子ダイアログを表示切替) |
| `CListCtrl` | 8 / 9 | 一覧表示 |
| `CProgressCtrl` | 1 | 処理経過 |

**メッセージマップ**

| マクロ | 件数 | 使い方 |
|---|---|---|
| `BEGIN_MESSAGE_MAP` | 101 | ダイアログごとのイベント対応表 |
| `ON_BN_CLICKED` | 69 / 320 | ボタン・チェックボックスのクリック |
| `ON_CBN_SELCHANGE` | 12 / 30 | コンボボックスの選択変更 |
| `ON_NOTIFY` | 18 / 20 | タブ切替など通知 |
| `ON_MESSAGE` | 2 / 20 | 自作メッセージ(`WM_APP + n`)の受け取り |
| `ON_WM_TIMER` | 3 | タイマー |
| `DefWindowProc` の上書き | 10 / 15 | DIO ボードからの通知(`DIOM_TRIGGER`、`DIOM_INTERRUPT`)や Ethernet トリガを受け取る |

**画面の見た目**:`OnCtlColor`(14)+ `CBrush` で背景色・文字色を変える。`CFont` でフォント変更

---

## 9. MFC / Win32:スレッドと同期

| 技術 | 件数 | 使い方 | 例 |
|---|---|---|---|
| `AfxBeginThread` | 2 / 17 | ワーカースレッドを起動。`CREATE_SUSPENDED` で作り、`m_bAutoDelete = FALSE` にしてから `ResumeThread` | `RunThreadGrab`、`RunThreadSrch` |
| `CWinThread*` | 9 | スレッドのハンドルを保持 | `dlg->thread_grab` |
| `WaitForSingleObject` | 2 / 7 | タイムアウト付きでスレッド終了を待つ | 取込 1000ms、探索 3000ms |
| `PostMessage` | 8 / 46 | ワーカースレッドから画面スレッドへ処理を頼む(待たない) | `WM_APP_COMMAND_RESULT_SUCCESS`、`WM_APP_COMMAND_LATCH_ON` |
| `SendMessage` | 32 / 139 | 同上(処理が終わるまで待つ)。I/O ユニットなしの LATCH など | `dlg->SendMessageA(WM_APP_COMMAND_LATCH_ON, …)` |
| `std::mutex` + `std::lock_guard` | 5 / 31、4 / 17 | C++11 の排他。PLC 通信とトリガ処理 | `comm_mutex`、`runtrigger_mutex` |
| `std::this_thread::sleep_for` | 4 / 8 | 待ち(10ms など) | `CheckTrgFall` |
| `SetTimer` / `KillTimer` | 4 / 10 | 周期処理、時間監視。コールバック関数版も使う | `TIM_CMD_LIMIT`(10秒)、`TIM_IO_UPDATE` |
| `CALLBACK` 関数 | 9 / 24 | タイマーやカメラ SDK のコールバック | `CallbackCmdTimeLimit` |
| `CreateMutex` | 1 | 2重起動の検出 | `Luv_Vision.cpp` |

**スレッドの構成**

```
画面スレッド(CDlgSystemMain)
 └ コマンドスレッド(Dispatch)
     ├ 画像取込スレッド(GoLoad)
     ├ データ読込スレッド(DataRead)
     ├ 探索スレッド(GoSearch)
     ├ ビット8 OFF 監視スレッド(CheckTrgFall)
     ├ LATCH 書込スレッド(WriteLatch)
     └ データ書込スレッド(DataWrite)
Ethernet トリガ受信スレッド(EthernetTrgProc)※I/O ユニットなし時
```

- スレッド間のやり取りは、ダイアログのメンバ構造体 `thread_rq`(要求)と `thread_rs`(結果)

---

## 10. Win32 API とライブラリ

| 分野 | 技術 | 件数 | 使い方 |
|---|---|---|---|
| ini ファイル | `GetPrivateProfileString(A)` / `GetPrivateProfileInt(A)` / `WritePrivateProfileString(A)` | 404 / 387 | 設定の読み書きすべて |
| ファイル | `fopen_s`、`_chdir` / `_mkdir`、`FindFirstFile` | 27、112、14 | ファイル存在確認、フォルダ作成、古いファイル削除 |
| 文字列(C) | `strcpy_s` / `strcat_s`、`strtok_s`、`atoi` / `atof` | 308 | パス作成、入力値変換 |
| 文字列(MFC) | `CString`(`Mid`、`Format`、`Append`、`+` 連結) | 139 / 1415 | 表示文字列、ログ |
| ソケット | Winsock(`WSAStartup`、`socket`、`connect` / `bind` / `listen` / `accept`、`send` / `recv`) | — | PLC との MC プロトコル(クライアント)、Ethernet トリガ(サーバー) |
| FTP | `CInternetSession` / `CFtpConnection` / `CFtpFileFind`(MFC) | — | FTP ダウンロード・削除(`CFtpMFC`) |
| ファイル選択 | `OPENFILENAME` / `GetOpenFileName` | 14 | BMP の選択 |
| メッセージ表示 | `MessageBoxA` / `AfxMessageBox` | 319 / 6 | 確認・エラー表示 |
| 時間 | `timeGetTime`(`mmsystem.h`) | 5 | コマンド処理時間の計測 |
| システム | `ExitWindowsEx`、`EnumDisplayMonitors`、`ShellExecute` | — | PC シャットダウン、マルチモニタ、外部プログラム |
| DIO | CONTEC CDIO(`CDioCtec1616` 経由) | 132 | ビット入出力、トリガ通知 |
| 画像処理 | WIL `FVCL`:`Data::CFvImage`(391)、`GS2`(グレイサーチ)、`FPM2`(形状サーチ)、`Edge`、`Blob`、`Operation`、`Region`、`File` | — | 画像・探索・前処理 |
| 画像表示 | WIL `CEvView`(378)、`CreateOverlay` + `FVCL::GDI` 図形(382) | — | 画像表示と十字・枠の重ね描き |

---

## 11. このコードの書き方の特徴

| 特徴 | 中身 |
|---|---|
| C 寄りの C++ | クラスは「関数のまとまり」として使い、継承・テンプレート・STL はほぼ使わない |
| 固定長配列 | 動的なコンテナを使わず、上限(コマンド 256、スコープ 16、視野 4、カメラ 8)を配列サイズで決めている |
| 戻り値は成否 | 0:成功 / 1:失敗。詳細は 16進のエラーコード |
| `goto END` | 失敗時の出口を1つにまとめる |
| 番号でつなぐ | オブジェクト同士を参照ではなく番号で結ぶ |
| 同じ処理のコピー | マーク位置の取り出し(グレイ / 形状 / エッジ / 2値化重心)などが複数ファイルに同じ形で書かれている |
| ヘッダの説明コメント | ファイル先頭に作成者・履歴・説明、関数ごとに引数・戻り値・特記事項を書く統一書式 |
| 文字コード混在 | Shift-JIS / UTF-16 / UTF-8(BOM付き)のファイルが混在 |

---

## 12. 読む・直すときの注意

- **スレッドから画面を直接触らない**:画面更新は `PostMessage` / `SendMessage` で画面スレッドに頼む形になっている。ワーカースレッドで `MessageBox` を出すと、そのスレッドが止まり、待っている側がタイムアウトする
- **グローバル配列は複数スレッドから触られる**:`scope_param` などに排他はない。PLC 通信だけが `comm_mutex` で守られている
- **`delete[]` の前の `return`**:途中で抜けるとメモリが解放されない。新しく書くなら `std::vector` などで自動解放にすると安全
- **ANSI API**:`…A` 版の関数と `char[]` を使っている。日本語のパスや文字は Shift-JIS 前提
- **ヘッダ変更の影響範囲**:グローバル配列のクラス(`CCommands`、`CScopeParam` など)を変えると、`extern` している全ファイルに影響する

---

## 更新履歴

| 日付 | 内容 |
|---|---|
| 2026-10-12 | 初版(Ver10.21.1.3 のソースから集計) |
