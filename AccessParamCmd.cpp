/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamCmd.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.11.18	M.Iwasaki	パラメータ読み/書き不具合修正
 **			2022.09.12	M.Iwasaki	キャリブレーションデータコピー改善
 **			2024.07.16	M.Iwasaki	10.18.4.6	TCP/IPサーバ機能追加
 ** 注記：
 ** 説明：コマンドパラメータの読込と書込
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamCmd.h"

#include "Commands.h"		//コマンドパラメータ

#include "direct.h"

extern CCommands cmd_param[256];			//コマンドパラメータ

extern char* APP_ROOT;

CAccessParamCmd::CAccessParamCmd()
{
}


CAccessParamCmd::~CAccessParamCmd()
{
}

/** ****************************************************** **
 ** Iniファイルディレクトリの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::CheckDir()
{
	// TODO: ここに実装コードを追加します.
	if (_chdir((CString)APP_ROOT) != 0) {
		if (_mkdir((CString)APP_ROOT) != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT) != 0) return 1;
	if (_chdir((CString)APP_ROOT + "\\SYSTEM") != 0) {
		if (_mkdir((CString)APP_ROOT + "\\SYSTEM") != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT + "\\SYSTEM") != 0) return 1;
	if (_chdir((CString)APP_ROOT + "\\SYSTEM\\CMD") != 0) {
		if (_mkdir((CString)APP_ROOT + "\\SYSTEM\\CMD") != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT + "\\SYSTEM\\CMD") != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** Command***.iniファイルの確認
 ** 引数：
 **　int cmd_number　コマンド番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::CheckIniFile(int cmd_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\CMD\\Command" + Tips0.IntToString(cmd_number) + ".ini";
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, filename, "r");

	//ファイルが開ければ結果0
	if (err == 0) rtn_result = 0;

	if (fp) {
		fclose(fp);
	}

	return rtn_result;
}


/** ****************************************************** **
 ** Command***.iniファイルの作成
 ** 引数：
 **　int cmd_number　コマンド番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::CreateIniFile(int cmd_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFile(cmd_number);

	//最後にiniファイルの確認
	if (CheckIniFile(cmd_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Command***.iniファイルの書込
 ** 引数：
 **　int cmd_number　コマンド番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::WriteIniFile(int cmd_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\CMD\\Command" + Tips0.IntToString(cmd_number) + ".ini";

	//機能種別
	buf = Tips0.IntToString((int)cmd_param[cmd_number].base_method);
	ret = WritePrivateProfileStringA("Command", "base_method", buf, filename);

	//テンプレートパラメータ
	buf = Tips0.IntToString(cmd_param[cmd_number].template_number);
	ret = WritePrivateProfileStringA("Command", "template_number", buf, filename);

	//データ読込
	buf = Tips0.IntToString((int)cmd_param[cmd_number].read.enable);
	ret = WritePrivateProfileStringA("ReadData", "enable", buf, filename);
	buf = Tips0.IntToString((int)cmd_param[cmd_number].read.type);
	ret = WritePrivateProfileStringA("ReadData", "type", buf, filename);
	buf = Tips0.IntToString((int)cmd_param[cmd_number].read.calpos_enable);
	ret = WritePrivateProfileStringA("ReadData", "calpos_enable", buf, filename);

	//画像取込
	buf = Tips0.IntToString((int)cmd_param[cmd_number].grab.enable);
	ret = WritePrivateProfileStringA("GrabImage", "enable", buf, filename);

	//探索と演算処理
	buf = Tips0.IntToString((int)cmd_param[cmd_number].search.search_enable);
	ret = WritePrivateProfileStringA("SearchOperarion", "search_enable", buf, filename);
	buf = Tips0.IntToString((int)cmd_param[cmd_number].search.calc_enable);
	ret = WritePrivateProfileStringA("SearchOperarion", "calc_enable", buf, filename);
	buf = Tips0.IntToString((int)cmd_param[cmd_number].search.method);
	ret = WritePrivateProfileStringA("SearchOperarion", "method", buf, filename);

	//データ書込
	buf = Tips0.IntToString((int)cmd_param[cmd_number].write.enable);
	ret = WritePrivateProfileStringA("WriteData", "enable", buf, filename);
	buf = Tips0.IntToString((int)cmd_param[cmd_number].write.type);
	ret = WritePrivateProfileStringA("WriteData", "type", buf, filename);

	//連続実行コマンド
	buf = Tips0.IntToString(cmd_param[cmd_number].chain.ok_chain);
	ret = WritePrivateProfileStringA("CommandChain", "ok_chain", buf, filename);
	buf = Tips0.IntToString(cmd_param[cmd_number].chain.ng_chain);
	ret = WritePrivateProfileStringA("CommandChain", "ng_chain", buf, filename);

	//アライメントパラメータ
	buf = Tips0.IntToString(cmd_param[cmd_number].align.parts);
	ret = WritePrivateProfileStringA("AlignParameter", "parts", buf, filename);
	buf = Tips0.IntToString(cmd_param[cmd_number].align.base_scope);
	ret = WritePrivateProfileStringA("AlignParameter", "base_scope", buf, filename);
	buf = Tips0.IntToString(cmd_param[cmd_number].align.target_scope);
	ret = WritePrivateProfileStringA("AlignParameter", "target_scope", buf, filename);

	//キャリブレーションパラメータ
	buf = Tips0.IntToString(cmd_param[cmd_number].calib.copy_view);
	ret = WritePrivateProfileStringA("CalibParameter", "copy_view", buf, filename);
	buf = Tips0.IntToString(cmd_param[cmd_number].calib.copy_scope);
	ret = WritePrivateProfileStringA("CalibParameter", "copy_scope", buf, filename);
	//データコピー
	char chars_custom[17] = { '\0' };
	for (int i = 0; i < 16; i++) {
		WORD d_scope = 0;
		for (int j = 0; j < 4; j++) {
			d_scope += cmd_param[cmd_number].calib.custom_copy_scope[i].view[3 - j];
			if (j < 3) d_scope = d_scope << 1;
		}
		char c_scope[2] = { '\0' };
		Tips0.HexToChar(c_scope, sizeof(c_scope), d_scope);
		chars_custom[i] = c_scope[0];
	}
	ret = WritePrivateProfileStringA("CalibParameter", "custom_copy", chars_custom, filename);

	//オプション機能
	buf = Tips0.IntToString(cmd_param[cmd_number].option.after_cam_number);
	ret = WritePrivateProfileStringA("Option", "after_cam_number", buf, filename);

	//FTP機能基本データ
	buf = cmd_param[cmd_number].ftp_login.login_user;
	ret = WritePrivateProfileStringA("FTPlogin", "login_user", buf, filename);
	buf = cmd_param[cmd_number].ftp_login.login_pw;
	ret = WritePrivateProfileStringA("FTPlogin", "login_pw", buf, filename);
	buf = cmd_param[cmd_number].ftp_login.ip_address;
	ret = WritePrivateProfileStringA("FTPlogin", "ip_address", buf, filename);

	//FTPファイルダウンロードパラメータ
	buf = cmd_param[cmd_number].ftp_dlgot.sv_directory;
	ret = WritePrivateProfileStringA("FTPGotDlParam", "sv_directory", buf, filename);
	buf = cmd_param[cmd_number].ftp_dlgot.download_path;
	ret = WritePrivateProfileStringA("FTPGotDlParam", "download_path", buf, filename);
	buf = cmd_param[cmd_number].ftp_dlgot.file_name;
	ret = WritePrivateProfileStringA("FTPGotDlParam", "file_name", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** Command***.iniファイルの読込
 ** 引数：
 **　int cmd_number　コマンド番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::ReadIniFile(int cmd_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[32];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\CMD\\Command" + Tips0.IntToString(cmd_number) + ".ini";


	//機能種別
	ret = GetPrivateProfileStringA("Command", "base_method", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].base_method = (byte)atoi(buf);

	//テンプレートパラメータ
	ret = GetPrivateProfileStringA("Command", "template_number", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].template_number = atoi(buf);

	//データ読込
	ret = GetPrivateProfileStringA("ReadData", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].read.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("ReadData", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].read.type = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("ReadData", "calpos_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].read.calpos_enable = (BOOL)atoi(buf);

	//画像取込
	ret = GetPrivateProfileStringA("GrabImage", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].grab.enable = (BOOL)atoi(buf);

	//探索と演算処理
	ret = GetPrivateProfileStringA("SearchOperarion", "search_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].search.search_enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("SearchOperarion", "calc_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].search.calc_enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("SearchOperarion", "method", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].search.method = (WORD)atoi(buf);

	//データ書込
	ret = GetPrivateProfileStringA("WriteData", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].write.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("WriteData", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].write.type = (byte)atoi(buf);

	//連続実行コマンド
	ret = GetPrivateProfileStringA("CommandChain", "ok_chain", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].chain.ok_chain = atoi(buf);
	ret = GetPrivateProfileStringA("CommandChain", "ng_chain", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].chain.ng_chain = atoi(buf);

	//アライメントパラメータ
	ret = GetPrivateProfileStringA("AlignParameter", "parts", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].align.parts = atoi(buf);
	ret = GetPrivateProfileStringA("AlignParameter", "base_scope", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].align.base_scope = atoi(buf);
	ret = GetPrivateProfileStringA("AlignParameter", "target_scope", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].align.target_scope = atoi(buf);

	//キャリブレーションパラメータ
	ret = GetPrivateProfileStringA("CalibParameter", "copy_view", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].calib.copy_view = atoi(buf);
	ret = GetPrivateProfileStringA("CalibParameter", "copy_scope", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].calib.copy_scope = atoi(buf);
	//データコピー
	ret = GetPrivateProfileStringA("CalibParameter", "custom_copy", "", buf, sizeof(buf), filename);
	if (ret != 0) {
		if (strlen(buf) == 16) {
			for (int i = 0; i < 16; i++) {
				char c_scope[2] = { '\0' };
				c_scope[0] = buf[i];
				int d_scope = Tips0.CharToBin(c_scope, 16);
				for (int j = 0; j < 4; j++) {
					cmd_param[cmd_number].calib.custom_copy_scope[i].view[j] = d_scope % 2;
					if (j < 3) d_scope = d_scope >> 1;
				}
			}
		}
	}

	//オプション機能
	ret = GetPrivateProfileStringA("Option", "after_cam_number", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].option.after_cam_number = atoi(buf);

	//FTP機能基本データ
	ret = GetPrivateProfileStringA("FTPlogin", "login_user", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_login.login_user = buf;
	ret = GetPrivateProfileStringA("FTPlogin", "login_pw", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_login.login_pw = buf;
	ret = GetPrivateProfileStringA("FTPlogin", "ip_address", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_login.ip_address = buf;

	//FTPファイルダウンロードパラメータ
	ret = GetPrivateProfileStringA("FTPGotDlParam", "sv_directory", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_dlgot.sv_directory = buf;
	ret = GetPrivateProfileStringA("FTPGotDlParam", "download_path", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_dlgot.download_path = buf;
	ret = GetPrivateProfileStringA("FTPGotDlParam", "file_name", "", buf, sizeof(buf), filename);
	if (ret != 0) cmd_param[cmd_number].ftp_dlgot.file_name = buf;

	return 0;
}


/** ****************************************************** **
 ** Command***.iniファイルの削除
 ** 引数：
 **　int cmd_number　コマンド番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::DeleteIniFile(int cmd_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\CMD\\Command" + Tips0.IntToString(cmd_number) + ".ini";

	BOOL del_result = DeleteFileA(filename);
	if (del_result) rtn_result = 0;

	return rtn_result;
}


/** ****************************************************** **
 ** EthernetTrg.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamCmd::ReadIniFileEthernetTrg()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\SPEC\\EthernetTrg.ini";

	//ファイル有無確認
	int rtn_open = 1;
	{
		FILE* fp;
		errno_t err;

		err = fopen_s(&fp, filename, "r");

		//ファイルが開ければ結果0
		if (err == 0) rtn_open = 0;

		if (fp) {
			fclose(fp);
		}
	}
	if (rtn_open != 0) return 0;	//ファイルなければ終了


	//特殊設定
	CString cmd_name;
	CString data_name;
	for (int cmd_num = 0; cmd_num < 256; cmd_num++) {
		cmd_name = "Command" + Tips0.IntToString(cmd_num);		//例)Command0

		//data設定読出
		for (int data_num = 0; data_num < 4; data_num++) {
			data_name = "data" + Tips0.IntToString(data_num + 1);		//例)data1

			CString str_setting;
			ret = GetPrivateProfileStringA(cmd_name, data_name, "", buf, sizeof(buf), filename);
			if (ret != 0) {
				cmd_param[cmd_num].eTrgData.Assign(data_num, buf, strlen(buf));
			}
		}
	}

	return 0;
}
