/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcUtil.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	オリジナル記号文字列処理追加、言語切替修正
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2024.07.16	M.Iwasaki	10.18.4.6	ブロブ機能拡張
 **			2026.03.26	M.Iwasaki	10.19.1.2	FTP機能追加
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：ユーティリティ系演算プログラム
 **
 **/

#include "stdafx.h"
#include "CalcUtil.h"

#include "DlgSystemMain.h"

#include "Commands.h"			//コマンドパラメータ
#include "CurrentStatus.h"		//現状共通データ
#include "CalcPLC.h"			//PLCデータ取得
#include "ProcFileAccess.h"		//ファイルアクセス処理

#include "AccessParamScrp.h"	//機種毎スコープパラメータファイルアクセス
#include "AccessParamTemp.h"	//機種毎テンプレートパラメータファイルアクセス
#include "AccessParamMes.h"		//機種毎検査パラメータファイルアクセス
#include "AccessParamWork.h"	//機種パラメータファイルアクセス
#include "Templates.h"			//システムテンプレートパラメータ
#include "ScopeParam.h"			//機種毎スコープパラメータ
#include "WorkParam.h"			//機種パラメータ
#include "OutResult.h"			//結果出力パラメータ

#include "FtpCommunication.h"	//FTP通信

#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

//DLL
#include "AlignBarcode.h"	//AlignBarcode DLL Header

extern CCommands		cmd_param[256];		//コマンドパラメータ
extern CCurrentStatus	current;			//共有情報
extern CCalcPLC			calc_plc;			//PLCデータ取得

extern CAccessParamScrp	access_scp;			//機種毎スコープパラメータファイルアクセス
extern CAccessParamTemp	access_tmp;			//機種毎テンプレートパラメータファイルアクセス
extern CAccessParamMes	access_mes;			//機種毎検査パラメータファイルアクセス
extern CAccessParamWork	access_work;		//機種パラメータファイルアクセス
extern CTemplates		sys_template[256];	//システムテンプレートパラメータ
extern CScopeParam		scope_param[16];	//機種毎スコープパラメータ
extern COutResult		sys_outrslt;		//結果出力パラメータ

extern CFtpCommunication	ftp_com;		//FTP通信

extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

//DLL
extern AlignBarcode::Param::CBarcodeParam bcr_param[256];	//機種毎バーコードリーダーパラメータ

extern char* APP_ROOT;

CCalcUtil::CCalcUtil()
{
}


CCalcUtil::~CCalcUtil()
{
}


/** ****************************************************** **
 ** ユーティリティメイン
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcMain(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	int rtn_result = 1;

	byte cmd = dlg->run_cmd;	//実行コマンド

	byte base_method = cmd_param[cmd].base_method;	//機能種別

	switch (base_method) {

	case 0x12:	//機種切替
		//演算処理
		if (CalcExec_12(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x13:	//カメラ切替
		//演算処理
		if (CalcExec_13(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x16:	//ファイルアクセス
		//演算処理
		if (CalcExec_16(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x17:	//マニュアルサーチ有効
		//演算処理
		if (CalcExec_17(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x41:	//外部オフセット設定
		//演算処理
		if (CalcExec_41(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x51:	//オリジナル記号文字列
		//演算処理
		if (CalcExec_51(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x52:	//FTPFileDownload
		//演算処理
		if (CalcExec_52(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	case 0x53:	//FTPFileRemove
		//演算処理
		if (CalcExec_53(cmd, base_method, &dlg->thread_rs.calc_err) != 0) goto END;
		break;
	default:
		goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 機種切替の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_12(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	CWorkParam work_param;	//機種パラメータ

	//機種番号(w_data[0])の取得
	WORD data0;
	calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 0, &data0);
	int number = (int)data0;

	if (!(number >= 1 && number <= 999)) {
		//コマンドログ
		outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "機種指定失敗:", "Model specification failure:") + Tips0.IntToString(number));
		*calc_err = 0x4121;		//機種切替 機種指定失敗
		goto JUMP;
	}

	//切替Work.iniの読込
	if (access_work.CheckIniFileParam(number) == 0) {
		access_work.ReadIniFileParam(number, &work_param);
	}

	//切替機種の登録確認
	if (!work_param.regi.enable) {
		//コマンドログ
		outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "未作成機種選択:", "Uncreated model selection:") + Tips0.IntToString(number));
		*calc_err = 0x4122;		//機種切替 未作成機種選択
		goto JUMP;
	}

	//機種毎スコープパラメータの全初期化
	access_scp.InitParams();
	//機種毎スコープパラメータの読み込み
	for (int i = 0; i < 16; i++) {
		if (access_scp.CheckIniFileParam(number, i) == 0) access_scp.ReadIniFileParam(number, i);

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

	current.work_number = number;	//現在機種の更新

JUMP:
	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "機種切替完了:機種No", "Model switching completed:Model No") + Tips0.IntToString(number, 3));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "機種切替失敗:機種No", "Model switching failure:Model No") + Tips0.IntToString(number, 3));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** カメラ切替の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_13(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//カメラ番号(w_data[0])の取得
	WORD data0;
	calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 0, &data0);
	int change_cam = (int)data0;

	current.main_view_cam_num = change_cam;

	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "カメラ切替完了", "Camera switching completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "カメラ切替失敗", "Camera switching failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** ファイルアクセスの演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 **　WORD* calc_err		演算エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_16(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	//参照ディレクトリパス
	char access_path[MAX_PATH] = { '\0' };			//参照絶対パス
	static char relative_dir[MAX_PATH] = { '\0' };	//参照相対パス
	char read_dir[40] = { '\0' };		//読込ディレクトリ
	char read_file_name[30] = { '\0' };	//読込ファイル名

	//コマンドの取得
	WORD cmd_head[2];
	calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 0, &cmd_head[0]);
	calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 1, &cmd_head[1]);

	//コマンド"CD"の場合
	if (cmd_head[0] == 0x4443) {
		outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "CD実行", "CD execution"));	//コマンドログ

		int rtn_cd = ReadDirectory(cmd ,read_dir);
		//成功
		if (rtn_cd == 0) {
			strcat_s(relative_dir, sizeof(relative_dir), read_dir);
			
			outlog.LogOutListCmd(1, 2, (CString)relative_dir);
		}
		//失敗
		else {
			//コマンドログ
			outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "CD失敗", "CD failure"));
			*calc_err = 0x4161;		//ファイルアクセス CD失敗
		}
	}
	//コマンド"RS"の場合
	else if (cmd_head[0] == 0x5352) {
		relative_dir[0] = '\0';

		//コマンドログ
		outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "RS実行", "RS execution"));
	}
	//コマンド"CD"以外
	else {
		//ファイルアクセス
		CProcFileAccess proc_access;

		//デバイス文字の取得
		WORD dev_head;
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 2, &dev_head);
		char data_c[5] = { '\0' };
		char asc_array[3] = { '\0' };
		Tips0.HexToChar(data_c, sizeof(data_c), dev_head, 4);	//WORDをchar文字列に変換
		asc_array[0] = data_c[2];
		asc_array[1] = data_c[3];
		char dev_char = Tips0.CharAsciiToChar(asc_array);

		//デバイス先頭アドレスの取得
		WORD dev_adrs;
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 4, &dev_adrs);

		//デバイス数の取得
		WORD dev_num;
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 5, &dev_num);

		//絶対パスの作成
		strcat_s(access_path, sizeof(access_path), (CString)APP_ROOT + "\\PLCDATA");
		strcat_s(access_path, sizeof(access_path), relative_dir);
		strcat_s(access_path, sizeof(access_path), "\\");

		//コマンド"LOAD"の確認
		if (cmd_head[0] == 0x4F4C && cmd_head[1] == 0x4441) {
			//コマンドログ
			outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "LOAD実行", "LOAD execution"));

			//ファイル名の取得
			if (ReadFileName(cmd, read_file_name) == 0) {
				strcat_s(access_path, sizeof(access_path), read_file_name);		//パス完成

				outlog.LogOutListCmd(1, 2, (CString)access_path);
				CString str_device = (CString)dev_char + Tips0.IntToString((int)dev_adrs) + "(" + Tips0.HexToString(dev_adrs) + "h)、" +
					Tips0.IntToString((int)dev_num) + "device";
				outlog.LogOutListCmd(1, 2, str_device);

				int rtn_load = proc_access.DataFileLoad(dev_char, (int)dev_adrs, (int)dev_num, access_path);
				if (rtn_load != 0) {
					//コマンドログ
					if (rtn_load == 1) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "ファイル無し", "No file"));
					if (rtn_load == 2) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "データ無し", "No data"));
					if (rtn_load == 3) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "PLC書込失敗", "PLC write failure"));
					*calc_err = 0x4162;		//ファイルアクセス LOAD失敗
				}
			}
			else {
				outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "ファイル名取得失敗", "File name acquisition failure"));
				*calc_err = 0x4162;		//ファイルアクセス LOAD失敗
			}
		}
		//コマンド"SAVE"の確認
		else if (cmd_head[0] == 0x4153 && cmd_head[1] == 0x4556) {
			//コマンドログ
			outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "SAVE実行", "SAVE execution"));

			//ファイル名の取得
			if (ReadFileName(cmd, read_file_name) == 0) {
				strcat_s(access_path, sizeof(access_path), read_file_name);		//パス完成

				outlog.LogOutListCmd(1, 2, (CString)access_path);
				CString str_device = (CString)dev_char + Tips0.IntToString((int)dev_adrs) + "(" + Tips0.HexToString(dev_adrs) + "h)、" +
					Tips0.IntToString((int)dev_num) + "device";
				outlog.LogOutListCmd(1, 2, str_device);

				int rtn_save = proc_access.DataFileSave(dev_char, (int)dev_adrs, (int)dev_num, access_path);
				if (rtn_save != 0) {
					//コマンドログ
					if (rtn_save == 1) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "ディレクトリ作成失敗", "Directory creation failure"));
					if (rtn_save == 2) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "ファイル作成失敗", "File creation failure"));
					if (rtn_save == 3) outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "PLC読込失敗", "PLC read failure"));
					*calc_err = 0x4163;		//ファイルアクセス SAVE失敗
				}
			}
			else {
				outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "ファイル名取得失敗", "File name acquisition failure"));
				*calc_err = 0x4163;		//ファイルアクセス LOAD失敗
			}
		}
		//コマンド異常
		else {
			char str0[5] = { '\0' };
			Tips0.HexToChar(str0, sizeof(str0), cmd_head[0], 4);
			char str1[5] = { '\0' };
			Tips0.HexToChar(str1, sizeof(str1), cmd_head[1], 4);
			char c_cmd[5] = { '\0' };
			char tmp[3] = { '\0' };
			tmp[0] = str0[2];
			tmp[1] = str0[3];
			c_cmd[0] = Tips0.CharAsciiToChar(tmp);
			tmp[0] = str0[0];
			tmp[1] = str0[1];
			c_cmd[1] = Tips0.CharAsciiToChar(tmp);
			tmp[0] = str1[2];
			tmp[1] = str1[3];
			c_cmd[2] = Tips0.CharAsciiToChar(tmp);
			tmp[0] = str1[0];
			tmp[1] = str1[1];
			c_cmd[3] = Tips0.CharAsciiToChar(tmp);

			//コマンドログ
			outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "コマンド未定義:", "Command undefined:") + (CString)c_cmd);
			*calc_err = 0x4169;		//ファイルアクセス コマンド未定義
		}
	}

	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ファイルアクセス完了", "File access completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ファイルアクセス失敗", "File access failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** ディレクトリパスの読込
 ** 引数：
 **　byte cmd	実行コマンド
 **　char* dir	読込パス
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		読込パスは39文字までとする
 ** ****************************************************** **/
int CCalcUtil::ReadDirectory(byte cmd, char* dir)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//文字列の読込
	WORD data_w;
	char data_c[5] = { '\0' };
	char asc_array[3] = { '\0' };
	int dir_num = 0;
	for (int i = 2; i < 20; i++) {
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, i, &data_w);
		Tips0.HexToChar(data_c, sizeof(data_c), data_w, 4);	//WORDをchar文字列に変換
		for (int j = 0; j < 4; j++) {
			//ascii文字配列にセット
			if (j == 0) asc_array[0] = data_c[2];
			if (j == 1) asc_array[1] = data_c[3];
			if (j == 2) asc_array[0] = data_c[0];
			if (j == 3) asc_array[1] = data_c[1];
			if (j == 1 || j == 3) {
				if (dir_num > 38) goto END;
				dir[dir_num] = Tips0.CharAsciiToChar(asc_array);
				if (dir[dir_num] == '\0') goto NULL_JUNP;	//NULL文字(0x00)になったら終了
				dir_num++;
			}
		}
	}
NULL_JUNP:

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 参照ファイル名の読込
 ** 引数：
 **　byte cmd	実行コマンド
 **　char* name	読込ファイル名
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		読込ファイル名は28文字までとする
 ** ****************************************************** **/
int CCalcUtil::ReadFileName(byte cmd, char* name)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//文字列の読込
	WORD data_w;
	char data_c[5] = { '\0' };
	char asc_array[3] = { '\0' };
	int name_num = 0;
	for (int i = 6; i < 20; i++) {
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, i, &data_w);
		Tips0.HexToChar(data_c, sizeof(data_c), data_w, 4);	//WORDをchar文字列に変換
		for (int j = 0; j < 4; j++) {
			//ascii文字配列にセット
			if (j == 0) asc_array[0] = data_c[2];
			if (j == 1) asc_array[1] = data_c[3];
			if (j == 2) asc_array[0] = data_c[0];
			if (j == 3) asc_array[1] = data_c[1];
			if (j == 1 || j == 3) {
				if (name_num > 27) goto END;
				name[name_num] = Tips0.CharAsciiToChar(asc_array);
				if (name[name_num] == '\0') goto NULL_JUNP;	//NULL文字(0x00)になったら終了
				name_num++;
			}
		}
	}
NULL_JUNP:

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** マニュアルアライメント有効化の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_17(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//演算は無し
	
	//コマンドログ
	outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "マニュアルサーチ有効", "Manual search enabled"));

	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "マニュアルサーチ有効完了", "Manual search enabled"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "マニュアルサーチ有効失敗", "Manual search enabled failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 外部オフセット設定の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_41(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//コマンドのテンプレート、スコープ、視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int scope = sys_template[template_number].scope;

	//外部オフセットの取得
	double offset_x;	//オフセットX(mm)
	double offset_y;	//オフセットY(mm)
	double offset_t;	//オフセットT(deg)
	if (calc_plc.GetExtOffsetXYT(cmd, &offset_x, &offset_y, &offset_t) != 0) {
		//コマンドログ
		outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "データセット異常", "Data set error"));
		*calc_err = 0x4411;		//外部オフセット設定 データセット失敗
		goto JUMP;
	}

	scope_param[scope].ext_offset_x = offset_x;
	scope_param[scope].ext_offset_y = offset_y;
	scope_param[scope].ext_offset_t = offset_t;

JUMP:
	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "外部オフセット設定完了", "External offset setting completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "外部オフセット設定失敗", "External offset setting failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** オリジナル記号文字列の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_51(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//読込データ
	char char_string[40] = { '\0' };		//読込文字列

	//識別文字の取得
	WORD symbol_word;
	char symbol_characters[3] = { '\0' };
	calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, 0, &symbol_word);
	Tips0.conv3.val_w = symbol_word;
	symbol_characters[0] = Tips0.conv3.val_b[0];
	symbol_characters[1] = Tips0.conv3.val_b[1];

	//オリジナル記号文字列の取得
	for (int i = 0; i < 10; i++) {
		if (Tips0.CompCharacters(symbol_characters, sys_outrslt.org_format[i].symbol) == 0) {

			int rtn_cd = ReadCharacters(cmd, char_string);
			//成功
			if (rtn_cd == 0) {
				strcpy_s(sys_outrslt.org_format[i].now_val, 128, char_string);

				CString symbol = (CString)sys_outrslt.org_format[i].symbol;
				CString report = "(" + symbol + "):" + (CString)char_string;
				outlog.LogOutListCmd(0, 2, report);
				outlog.LogOutFile(1, "オリジナル記号" + report);
			}
			//失敗
			else {
				//コマンドログ
				outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "文字列セット失敗(文字列異常)", "Read failure(characters)"));
				*calc_err = 0x4511;		//文字列セット失敗(文字列異常)
			}
			break;
		}

		//識別文字見つからない
		if (i == 9) {
			//コマンドログ
			outlog.LogOutListCmd(1, 2, Tips0.SelStr(language.jpn, "文字列セット失敗(識別文字異常)", "Read failure(symbol)"));
			*calc_err = 0x4512;		//文字列セット失敗(識別文字異常)
		}
	}

JUMP:
	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "オリジナル記号文字列完了", "Original characters setting completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "オリジナル記号文字列失敗", "Original characters setting failure"));		//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 文字列の読込
 ** 引数：
 **　byte cmd	実行コマンド
 **　char* char_string	設定文字列
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		読込文字列は39文字までとする
 ** ****************************************************** **/
int CCalcUtil::ReadCharacters(byte cmd, char* char_string)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//文字列の読込
	WORD data_w;
	char data_c[5] = { '\0' };
	char asc_array[3] = { '\0' };
	int char_num = 0;
	for (int i = 1; i < 20; i++) {
		calc_plc.GetWORDData(&cmd_param[cmd].plc_read.read_data, i, &data_w);
		Tips0.HexToChar(data_c, sizeof(data_c), data_w, 4);	//WORDをchar文字列に変換
		for (int j = 0; j < 4; j++) {
			//ascii文字配列にセット
			if (j == 0) asc_array[0] = data_c[2];
			if (j == 1) asc_array[1] = data_c[3];
			if (j == 2) asc_array[0] = data_c[0];
			if (j == 3) asc_array[1] = data_c[1];
			if (j == 1 || j == 3) {
				if (char_num > 38) goto END;
				char_string[char_num] = Tips0.CharAsciiToChar(asc_array);
				if (char_string[char_num] == '\0') goto NULL_JUNP;	//NULL文字(0x00)になったら終了
				char_num++;
			}
		}
	}
NULL_JUNP:

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** FTPファイルダウンロードの演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_52(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//FTPファイルダウンロード
	CString login_user = cmd_param[cmd].ftp_login.login_user;		//ログインユーザー
	CString login_pw = cmd_param[cmd].ftp_login.login_pw;			//ログインパスワード
	CString ip_address = cmd_param[cmd].ftp_login.ip_address;		//IPアドレス
	CString sv_directory = cmd_param[cmd].ftp_dlgot.sv_directory;	//転送元ディレクトリ
	CString download_path = cmd_param[cmd].ftp_dlgot.download_path;	//転送先パス
	CString file_name = cmd_param[cmd].ftp_dlgot.file_name;			//指定ファイル名
	int rslt_GOT = ftp_com.DownloadFTPCom(login_user, login_pw, ip_address, sv_directory, download_path, file_name, TRUE);
	if (rslt_GOT == 1) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ログイン失敗", "login failure"));	//コマンドログ
		*calc_err = 0x4611;		//FTPファイルダウンロード ログイン失敗
		goto JUMP;
	}
	if (rslt_GOT == 2) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ディレクトリ移動失敗", "directory change failure"));	//コマンドログ
		*calc_err = 0x4612;		//FTPファイルダウンロード ディレクトリ移動失敗
		goto JUMP;
	}
	if (rslt_GOT == 3) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ダウンロード失敗", "download failure"));	//コマンドログ
		*calc_err = 0x4613;		//FTPファイルダウンロード ダウンロード失敗
		goto JUMP;
	}

JUMP:
	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "FTPファイルダウンロード完了", "FTP File Download completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "FTPファイルダウンロード失敗", "FTP File Download failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}


/** ****************************************************** **
 ** FTPファイル削除の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcUtil::CalcExec_53(byte cmd, byte base_method, WORD* calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//FTPファイル削除
	CString login_user = cmd_param[cmd].ftp_login.login_user;		//ログインユーザー
	CString login_pw = cmd_param[cmd].ftp_login.login_pw;			//ログインパスワード
	CString ip_address = cmd_param[cmd].ftp_login.ip_address;		//IPアドレス
	CString sv_directory = cmd_param[cmd].ftp_dlgot.sv_directory;	//転送元ディレクトリ
	CString file_name = cmd_param[cmd].ftp_dlgot.file_name;			//指定ファイル名
	int rslt_GOT = ftp_com.RemoveFTPCom(login_user, login_pw, ip_address, sv_directory, file_name, TRUE);
	if (rslt_GOT == 1) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ログイン失敗", "login failure"));	//コマンドログ
		*calc_err = 0x4611;		//FTPファイル削除 ログイン失敗
		goto JUMP;
	}
	if (rslt_GOT == 2) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "ディレクトリ移動失敗", "directory change failure"));	//コマンドログ
		*calc_err = 0x4612;		//FTPファイル削除 ディレクトリ移動失敗
		goto JUMP;
	}
	if (rslt_GOT == 3) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "削除失敗", "Remove failure"));	//コマンドログ
		*calc_err = 0x4613;		//FTPファイル削除 削除失敗
		goto JUMP;
	}

JUMP:
	//
	//演算結果のデータ出力
	//
	calc_plc.UtilCalcDataOut(cmd, &cmd_param[cmd].plc_write.write_data, base_method, *calc_err);

	rtn_result = 0;

	if (*calc_err == 0x0000) {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "FTPファイル削除完了", "FTP File Remove completed"));	//コマンドログ 演算完了
	}
	else {
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "FTPファイル削除失敗", "FTP File Remove failure"));	//コマンドログ 演算失敗
	}

	return rtn_result;
}
