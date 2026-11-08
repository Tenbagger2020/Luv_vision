/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgWorkChangeMain.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2024.07.16	M.Iwasaki	10.18.4.6	ブロブ機能拡張
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：機種切替メインダイアログ
 **		画像処理の機種切替を行う
 **/
 
// DlgWorkChangeMain.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgWorkChangeMain.h"
#include "afxdialogex.h"

#include "ProcFileOpe.h"
#include "DlgWorkChgComment.h"
#include "DlgWorkChgCopyAll.h"
#include "DlgWorkChgCopyUniq.h"

#include "DlgSystemMain.h"
#include "PLCCommunication.h"	//シリアル通信メインクラス

//DLL
#include "AlignBarcode.h"	//AlignBarcode DLL Header

extern CAccessParamSys	access_sys;			//システムIniファイルアクセス
extern CAccessParamScrp	access_scp;			//機種毎スコープパラメータファイルアクセス
extern CAccessParamTemp	access_tmp;			//機種毎テンプレートパラメータファイルアクセス
extern CAccessParamMes	access_mes;			//機種毎検査パラメータファイルアクセス
extern CAccessParamLyer	access_lyr;			//機種毎個別レイヤパラメータファイルアクセス
extern CAccessParamWork	access_work;		//機種パラメータファイルアクセス
extern CTemplates		sys_template[256];	//システムテンプレートパラメータ
extern CProcFileOpe		proc_file;			//ファイル処理クラス
extern CCurrentStatus	current;			//共有情報
extern COutputLog		outlog;				//ログ出力

extern CPLCCommunication	plc_com;	//シリアル通信
extern CSysParamCom		com_parameter;	//通信設定パラメータ
extern CLanguage			language;			//言語設定用

//DLL
extern AlignBarcode::Param::CBarcodeParam bcr_param[256];	//機種毎バーコードリーダーパラメータ

extern char* APP_ROOT;

// CDlgWorkChangeMain ダイアログ

IMPLEMENT_DYNAMIC(CDlgWorkChangeMain, CDialogEx)

CDlgWorkChangeMain::CDlgWorkChangeMain(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgWorkChangeMain, pParent)
{

}

CDlgWorkChangeMain::~CDlgWorkChangeMain()
{
}

void CDlgWorkChangeMain::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WORK_LIST1, ctrl_work_list);
	DDX_Control(pDX, IDC_STATIC_WORK_NO, disp_work_no);
	DDX_Control(pDX, IDC_STATIC_SELECT_MODEL, static_select_model);
	DDX_Control(pDX, IDC_BUTTON3, btn_switch);
	DDX_Control(pDX, IDC_BUTTON4, btn_edit_comment);
	DDX_Control(pDX, IDC_STATIC_CAN_BE_COPY, static_can_be_copy);
	DDX_Control(pDX, IDC_STATIC_WORK_COPY, static_copy);
	DDX_Control(pDX, IDC_STATIC_CREATE_DELETE, static_create_delete);
	DDX_Control(pDX, IDC_BUTTON5, btn_all_data);
	DDX_Control(pDX, IDC_BUTTON6, btn_template_individual);
	DDX_Control(pDX, IDC_BUTTON7, btn_create_new);
	DDX_Control(pDX, IDC_BUTTON8, btn_delete);
	DDX_Control(pDX, IDOK, btn_exit);
}


BEGIN_MESSAGE_MAP(CDlgWorkChangeMain, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgWorkChangeMain::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgWorkChangeMain::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgWorkChangeMain::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgWorkChangeMain::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgWorkChangeMain::OnBnClickedButton4)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WORK_LIST1, &CDlgWorkChangeMain::OnNMCustomdrawWorkList1)
	ON_BN_CLICKED(IDC_BUTTON7, &CDlgWorkChangeMain::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgWorkChangeMain::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgWorkChangeMain::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON8, &CDlgWorkChangeMain::OnBnClickedButton8)
END_MESSAGE_MAP()


// CDlgWorkChangeMain メッセージ ハンドラー


BOOL CDlgWorkChangeMain::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgWorkChangeMain in ***");

	dlg_x = 320;
	dlg_y = 160;
	dlg_width = 480;
	dlg_height = 576;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


/** ****************************************************** **
 ** ダイアログ操作準備
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	page = 0;	//ページ初期化

	//現在機種No表示
	font_work_no.CreatePointFont(128, "ＭＳ Ｐゴシック");
	disp_work_no.SetFont(&font_work_no);
	UpdateWorkNo();

	//機種選択リストの準備
	ctrl_work_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);				//Noカラム追加
	ctrl_work_list.InsertColumn(1, Tips0.SelChar(language.jpn, "機種コメント", "Comment"), LVCFMT_LEFT, 200, -1);	//機種コメントカラム追加
	ctrl_work_list.InsertColumn(2, Tips0.SelChar(language.jpn, "作成日", "Created Date"), LVCFMT_LEFT, 110, -1);	//作成日カラム追加

	//機種選択リスト内の初期処理
	//空文字でNo項目を100個作成する
	char p_number[4] = { '\0' };
	char p_comment[128] = { '\0' };
	char p_date[24] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	for (int i = 0; i < 100; i++) {
		//No表示
		item_number.pszText = p_number;
		item_number.iItem = i;
		ctrl_work_list.InsertItem(&item_number);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	ctrl_work_list.SetExtendedStyle(ctrl_work_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LoadParameter();	//パラメータの展開
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1001));
		disp_work_no.SetWindowTextA(language.GetCharacterString(1002));				//現在機種No.表示
		static_select_model.SetWindowTextA(language.GetCharacterString(1005));		//切替機種を選択します
		btn_switch.SetWindowTextA(language.GetCharacterString(1006));				//切替実行
		btn_edit_comment.SetWindowTextA(language.GetCharacterString(1007));			//コメント編集
		static_can_be_copy.SetWindowTextA(language.GetCharacterString(1008));		//機種コピー、新規作成と削除を行います
		static_copy.SetWindowTextA(language.GetCharacterString(1009));				//機種コピー
		static_create_delete.SetWindowTextA(language.GetCharacterString(1010));		//作成と削除
		btn_all_data.SetWindowTextA(language.GetCharacterString(1011));				//一括
		btn_template_individual.SetWindowTextA(language.GetCharacterString(1012));	//テンプレート個別
		btn_create_new.SetWindowTextA(language.GetCharacterString(1013));			//新規作成
		btn_delete.SetWindowTextA(language.GetCharacterString(1014));				//削除
		btn_exit.SetWindowTextA(language.GetCharacterString(1015));					//戻る
	}
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::LoadParameter()
{
	// TODO: ここに実装コードを追加します.
	int start_number = page * 100;

	//機種選択リスト内の全アイテムの削除はしない
	//DeleteAllItems()→InsertItem()→SetItem()→SetItem()としても問題ないが
	//再描画時にチカチカするので、リスト更新ではDeleteAllItems()はせず全ての項目をSetItem()で上書きする
	//ctrl_work_list.DeleteAllItems();

	//Work.ini内の機種コメントと作成日時の展開
	char p_number[4] = { '\0' };
	char p_comment[128] = { '\0' };
	char p_date[24] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	LVITEM item_comment;
	item_comment.mask = LVIF_TEXT;
	item_comment.iSubItem = 1;
	LVITEM item_date;
	item_date.mask = LVIF_TEXT;
	item_date.iSubItem = 2;
	for (int i = 0; i < 100; i++) {
		//Work.iniの読込
		if (access_work.CheckIniFileParam(start_number + i) == 0) {
			access_work.ReadIniFileParam(start_number + i, &work_list[i]);
		}
		else {
			work_list[i].regi.enable = FALSE;
			work_list[i].regi.comment = "";
			work_list[i].regi.create_date = "";
		}

		//No表示
		Tips0.IntToChar(p_number, sizeof(p_number), start_number + i, 3);
		item_number.pszText = p_number;
		item_number.iItem = i;
		ctrl_work_list.SetItem(&item_number);
		//機種コメント表示
		strcpy_s(p_comment, sizeof(p_comment), work_list[i].regi.comment);
		item_comment.pszText = p_comment;
		item_comment.iItem = i;
		ctrl_work_list.SetItem(&item_comment);
		//作成日表示
		strcpy_s(p_date, sizeof(p_date), work_list[i].regi.create_date);
		item_date.pszText = p_date;
		item_date.iItem = i;
		ctrl_work_list.SetItem(&item_date);
	}
}


/** ****************************************************** **
 ** 機種切替実行
 ** 引数：
 **　int number		切替機種No(1～999)
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChangeMain::ChangeParam(int number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (!(number >= 1 && number <= 999)) goto END;

	//機種毎スコープパラメータの全初期化
	access_scp.InitParams();
	//機種毎スコープパラメータの読み込み
	for (int i = 0; i < 16; i++) {
		if (access_scp.CheckIniFileParam(number,i) == 0) access_scp.ReadIniFileParam(number, i);

		//画素サイズ固定化
		access_scp.ReadIniFilePixel(i);

		//経時変化読み込み
		if (access_scp.CheckVariationParam(number, i) == 0) access_scp.ReadVariationParam(number, i);
	}

	//機種テンプレートの全初期化
	access_tmp.InitParams();
	//機種テンプレートの読み込み
	for (int i = 0; i < 256; i++) {
		//未定義でなければ
		if (sys_template[i].method_type != 0x00) {
			if (access_tmp.CheckIniFileParam(number, i) == 0) access_tmp.ReadIniFileParam(number, i);
		}
	}

	//機種検査パラメータの全初期化
	access_mes.InitParams();
	//機種検査パラメータの読み込み
	for (int i = 0; i < 256; i++) {
		//計測モードを確認
		if (sys_template[i].method_type == 0x02) {
			//TMeasComm.iniファイルの読込
			if (access_mes.CheckIniFilePrmCom(number, i) == 0) access_mes.ReadIniFilePrmCom(number, i);

			//シート0～7を確認
			for (int j = 0; j < 8; j++) {
				//TMeasSh*.iniの読込
				if (access_mes.CheckIniFilePrmSht(number, i, j) == 0) access_mes.ReadIniFilePrmSht(number, i, j);
				//TPlcSh*.iniの読込
				if (access_mes.CheckIniFilePrmPlc(number, i, j) == 0) access_mes.ReadIniFilePrmPlc(number, i, j);
			}
		}
	}

	//機種バーコードパラメータの全初期化
	for (int i = 0; i < 256; i++) {
		bcr_param[i].InitialParam();
	}
	//機種バーコードパラメータの読み込み
	for (int i = 0; i < 256; i++) {
		//バーコードを確認
		if (sys_template[i].method_type == 0x03) {
			//Barcode.iniファイルの読込
			AlignBarcode::Param::CAccessParamBarcode access_bcr;
			if (access_bcr.CheckIniFilePrmBarcode(number, i) == 0) {
				access_bcr.ReadIniFilePrmBarcode(number, i, &bcr_param[i]);
			}
		}
	}

	//機種毎レイヤパラメータの全初期化
	access_lyr.InitParams();
	//機種毎レイヤパラメータの読み込み
	for (int i = 0; i < 4; i++) {
		if (access_lyr.CheckIniFileParam(number, i) == 0) access_lyr.ReadIniFileParam(number, i);
	}

	current.work_number = number;	//現在機種の更新
	UpdateWorkNo();		//表示更新

	//情報ファイル(Info.ini)の書込
	access_sys.WriteIniFileInfo();

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 機種作成実行
 ** 引数：
 **　int number		作成機種No(1～999)
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChangeMain::CreateWork(int number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (!(number >= 1 && number <= 999)) goto END;

	//機種削除実行(失敗しても無視)
	DeleteWork(number);

	//機種作成実行
	if (access_work.CreateIniFileParam(number) != 0) goto END;

	LoadParameter();	//パラメータの展開

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 機種削除実行
 ** 引数：
 **　int number		削除機種No(1～999)
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChangeMain::DeleteWork(int number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char path[MAX_PATH] = { '\0' };
	CString s_work;

	if (!(number >= 1 && number <= 999)) goto END;

	//機種削除実行
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_work = Tips0.IntToString(number, 3);
	strcat_s(path, s_work);

	if (proc_file.DeleteFileAllEx(path, TRUE) == 0) rtn_result = 0;

	//リスト更新
	LoadParameter();	//パラメータの展開

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 現在機種No表示更新
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::UpdateWorkNo()
{
	// TODO: ここに実装コードを追加します.
	disp_work_no.SetWindowTextA(Tips0.SelStr(language.jpn, "現在機種 No", "Current Work No") + Tips0.IntToString(current.work_number, 3));
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
}


/** ****************************************************** **
 ** PLCデータの[18]ワードに機種番号を書き込む
 ** 引数：
 **　なし
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChangeMain::UpdateWorkNoOut()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//書き込みデータ
	WORD* write_data;
	write_data = new WORD[1];	//データ数 1

	//機種番号のみ返す
	write_data[0] = current.work_number;

	//データの書き込み
	char dev_char = 'D';	//デバイス'D'のみ対応
	int write_adrs = com_parameter.device.write_number + 18;	//書込先頭アドレス
	if (plc_com.WriteDataWordEx(write_data, 1, dev_char, write_adrs) == 0) rtn_result = 0;

	plc_com.CloseTCPIP();	//TCPIPクローズ処理

	delete[] write_data;

	return rtn_result;
}


/** ****************************************************** **
 ** ページ切替(<<)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	page = page - 1;
	if (page < 0) page = 9;

	LoadParameter();	//パラメータの展開
}


/** ****************************************************** **
 ** ページ切替(>>)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	page = page + 1;
	if (page > 9) page = 0;

	LoadParameter();	//パラメータの展開
}


/** ****************************************************** **
 ** 切替実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//選択行の取得
	int select_row = ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_row < 0 || select_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "切替機種を選択してください", "Please select the switching model");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;	//選択行の確認
	}

	//切替機種No
	int select_work = page * 100 + select_row;

	//機種No.0は切替しない
	if (select_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種切替失敗", "Model switching failure");
		msg2 = Tips0.SelStr(language.jpn, "機種No000は選択できません", "Model No000 cannot be selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	if (work_list[select_row].regi.enable) {
		msg1 = Tips0.SelStr(language.jpn, "機種切替実行", "Model switching execution");
		msg2 = Tips0.SelStr(language.jpn, "機種No", "Switch to Model No") +
			Tips0.IntToString(select_work, 3) +
			Tips0.SelStr(language.jpn, "に切り替えます。よろしいですか？", ". Is it OK?");

		int judge = MessageBoxA(msg1 + "\n" + msg2, title, MB_OKCANCEL | MB_ICONQUESTION);
		if (judge == IDOK) {
			//機種切替実行
			if (ChangeParam(select_work) == 0) {
				//メイン画面の機種情報更新要求
				CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));
				dlg->PostMessageA(WM_APP_COMMAND_WORKNO_UPDATE);

				UpdateWorkNoOut();	//PLCデータの[18]ワードに機種番号を書き込む

				//成功メッセージ
				msg1 = Tips0.SelStr(language.jpn, "機種切替実行", "Model switching execution");
				msg2 = Tips0.SelStr(language.jpn, "機種切替に成功しました。", "The model was successfully switched");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
				outlog.LogOutFile(1, "機種切替成功 No" + Tips0.IntToString(select_work, 3));
			}
			else {
				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "機種切替実行", "Model switching execution");
				msg2 = Tips0.SelStr(language.jpn, "機種切替に失敗しました。", "Model switching has failed");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "機種切替失敗 No" + Tips0.IntToString(select_work, 3));
			}
		}
	}
	else {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種切替失敗", "Model switching failure");
		msg2 = Tips0.SelStr(language.jpn, "作成前の機種が選択されました", "Model before creation was selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
	}
}


/** ****************************************************** **
 ** コメント編集ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//選択行の取得
	int select_row = ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_row < 0 || select_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "編集機種を選択してください", "Please select the editing model");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;	//選択行の確認
	}

	//編集機種No
	int select_work = page * 100 + select_row;

	//機種No.0は編集しない
	if (select_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "編集失敗", "Edit failed");
		msg2 = Tips0.SelStr(language.jpn, "機種No000はコメント編集できません", "You cannot edit comments for model No000");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	if (work_list[select_row].regi.enable) {
		//コメント編集ダイアログ
		CDlgWorkChgComment dlg_comment;
		dlg_comment.SetNumber(select_work);
		dlg_comment.SetComment(work_list[select_row].regi.comment);
		dlg_comment.DoModal();
		work_list[select_row].regi.comment = dlg_comment.GetComment();	//変更後コメントの取得

		//機種コメントのパラメータ更新
		access_work.WriteIniFileParam(select_work, &work_list[select_row]);

		LoadParameter();	//パラメータの展開

		if (select_work == current.work_number) {
			//メイン画面の機種情報更新要求
			CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));
			dlg->PostMessageA(WM_APP_COMMAND_WORKNO_UPDATE);
		}
	}
	else {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "編集失敗", "Edit failed");
		msg2 = Tips0.SelStr(language.jpn, "作成前の機種が選択されました", "Model before creation was selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
	}
}


/** ****************************************************** **
 ** 機種コピー 一括ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//一括コピーダイアログ
	CDlgWorkChgCopyAll dlg_copy_all;
	dlg_copy_all.DoModal();

	//現在機種の再読み込み処理
	if (dlg_copy_all.GetReloadAns()) {
		if (ChangeParam(current.work_number) == 0) {
			//成功メッセージ
			msg1 = Tips0.SelStr(language.jpn, "現在機種を再読込しました。", "The current model has been reloaded");
			MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
			outlog.LogOutFile(1, "現在機種再読込成功 No" + Tips0.IntToString(current.work_number, 3));
		}
		else {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "現在機種の再読込に失敗しました。", "Failed to reload the current model");
			MessageBoxA(msg1, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "現在機種再読込失敗 No" + Tips0.IntToString(current.work_number, 3));
		}
	}

	LoadParameter();	//パラメータの展開
}


/** ****************************************************** **
 ** 機種コピー テンプレート個別ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//テンプレート個別コピーダイアログ
	CDlgWorkChgCopyUniq dlg_copy_uniq;
	dlg_copy_uniq.DoModal();

	//現在機種の再読み込み処理
	if (dlg_copy_uniq.GetReloadAns()) {
		if (ChangeParam(current.work_number) == 0) {
			//成功メッセージ
			msg1 = Tips0.SelStr(language.jpn, "現在機種を再読込しました。", "The current model has been reloaded");
			MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
			outlog.LogOutFile(1, "現在機種再読込成功 No" + Tips0.IntToString(current.work_number, 3));
		}
		else {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "現在機種の再読込に失敗しました。", "Failed to reload the current model");
			MessageBoxA(msg1, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "現在機種再読込失敗 No" + Tips0.IntToString(current.work_number, 3));
		}
	}

	LoadParameter();	//パラメータの展開
}


/** ****************************************************** **
 ** 新規作成ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton7()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//選択行の取得
	int select_row = ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_row < 0 || select_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "作成機種を選択してください", "Please select the model to create");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;	//選択行の確認
	}

	//作成機種No
	int create_work = page * 100 + select_row;

	//機種No.0は作成しない
	if (create_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "新規作成失敗", "New creation failure");
		msg2 = Tips0.SelStr(language.jpn, "機種No000は作成できません", "Model No000 cannot be created");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	msg1 = Tips0.SelStr(language.jpn, "新規作成実行", "New creation execution");
	msg2 = Tips0.SelStr(language.jpn, "機種No", "Create model No") +
		Tips0.IntToString(create_work, 3) +
		Tips0.SelStr(language.jpn, "を作成します。よろしいですか？", ". Is it OK?");
	if (work_list[select_row].regi.enable) {
		msg2 = Tips0.SelStr(language.jpn, "機種No", "Recreate model No") +
			Tips0.IntToString(create_work, 3) +
			Tips0.SelStr(language.jpn, "を再作成します。よろしいですか？", ". Is it OK?");

		if (create_work == current.work_number) {
			msg2 = Tips0.SelStr(language.jpn, "機種No", "Recreate model No") +
				Tips0.IntToString(create_work, 3) +
				Tips0.SelStr(language.jpn, "(現在機種)を再作成します。よろしいですか？", "(current model). Is it OK?");
		}
	}

	int judge = MessageBoxA(msg1 + "\n" + msg2, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		//作成実行
		int res_create = 1;
		if (CreateWork(create_work) == 0) {
			//現在機種を新規作成した場合
			if (create_work == current.work_number) {
				//機種切替実行
				if (ChangeParam(create_work) == 0) res_create = 0;
			}
			//他機種を新規作成した場合
			else {
				res_create = 0;
			}
		}

		if (res_create == 0) {
			//成功メッセージ
			msg1 = Tips0.SelStr(language.jpn, "新規作成実行", "New creation execution");
			msg2 = Tips0.SelStr(language.jpn, "機種作成に成功しました。", "The model was created successfully");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
			outlog.LogOutFile(1, "機種作成成功 No" + Tips0.IntToString(create_work, 3));
		}
		else {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "新規作成実行", "New creation execution");
			msg2 = Tips0.SelStr(language.jpn, "機種作成に失敗しました。", "Failed to create the model");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "機種作成失敗 No" + Tips0.IntToString(create_work, 3));
		}
	}
}


/** ****************************************************** **
 ** 削除ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedButton8()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//選択行の取得
	int select_row = ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_row < 0 || select_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "削除機種を選択してください", "Please select the model to be deleted");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;	//選択行の確認
	}

	//削除機種No
	int delete_work = page * 100 + select_row;

	//現在機種は削除しない
	if (delete_work == current.work_number) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "削除失敗", "Deletion failure");
		msg2 = Tips0.SelStr(language.jpn, "現在機種は削除できません", "Currently model cannot be deleted");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	//機種No.0は削除しない
	if (delete_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "削除失敗", "Deletion failure");
		msg2 = Tips0.SelStr(language.jpn, "機種No000は削除できません", "Model No000 cannot be deleted");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	if (work_list[select_row].regi.enable) {
		msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
		msg2 = Tips0.SelStr(language.jpn, "機種No", "Delete model No") +
			Tips0.IntToString(delete_work, 3) +
			Tips0.SelStr(language.jpn, "を削除します。よろしいですか？", ". Is it OK?");

		int judge = MessageBoxA(msg1 + "\n" + msg2, title, MB_OKCANCEL | MB_ICONQUESTION);
		if (judge == IDOK) {
			//削除実行
			if (DeleteWork(delete_work) == 0) {
				//成功メッセージ
				msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
				msg2 = Tips0.SelStr(language.jpn, "機種削除に成功しました。", "The model was successfully deleted");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
				outlog.LogOutFile(1, "機種削除成功 No" + Tips0.IntToString(delete_work, 3));
			}
			else {
				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
				msg2 = Tips0.SelStr(language.jpn, "機種削除に失敗しました。", "Failed to delete the model");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "機種削除失敗 No" + Tips0.IntToString(delete_work, 3));
			}
		}
	}
	else {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "削除失敗", "Deletion failure");
		msg2 = Tips0.SelStr(language.jpn, "未作成の機種が選択されました", "Uncreated model has been selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
	}	
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgWorkChangeMain out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** 機種選択リスト 選択背景の色変更
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChangeMain::OnNMCustomdrawWorkList1(NMHDR *pNMHDR, LRESULT *pResult)
 {
	 //LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	 LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	 // TODO: ここにコントロール通知ハンドラー コードを追加します。
	 *pResult = 0;

	 if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
		 *pResult = CDRF_NOTIFYITEMDRAW;
	 }
	 else if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {

		 if (pNMCD->nmcd.uItemState & CDIS_SELECTED) {
			 pNMCD->clrTextBk = RGB(181, 218, 255);		//薄青
			 pNMCD->nmcd.uItemState &= ~CDIS_SELECTED;
		 }
		 else {
			 pNMCD->clrTextBk = RGB(255, 255, 255);	//白
		 }
		 *pResult = CDRF_NEWFONT;
	 }
 }


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgWorkChangeMain::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	if (WM_KEYDOWN == pMsg->message) {
		switch (pMsg->wParam) {
		case VK_RETURN:		//Enterキーで画面を閉じない
			return FALSE;
		case VK_ESCAPE:		//ESCキーで画面を閉じない
			return FALSE;
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
