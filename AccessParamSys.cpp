/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamSys.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.09.02	M.Iwasaki	検出位置十字サイズ追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.06.20	M.Iwasaki	4視野アライメント演算処理パラメータ追加
 **			2023.10.30	M.Iwasaki	10.14.2.0	東芝テリーUSBカメラ対応
 **			2024.07.16	M.Iwasaki	10.18.4.6	テンプレート特殊設定群追加
 ** 注記：
 ** 説明：システムパラメータの読込と書込
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamSys.h"

#include "CurrentStatus.h"		//現状共通データ
#include "SysParamCom.h"		//システムパラメータ 通信設定
#include "SysParamDevice.h"		//システムパラメータ　デバイス設定
#include "CameraSettings.h"		//カメラパラメータ
#include "Scopes.h"				//スコープパラメータ
#include "Templates.h"			//システムテンプレートパラメータ
#include "TemplatesGroup.h"		//テンプレートグループパラメータ
#include "Layer.h"				//システムレイヤパラメータ
#include "SysOption.h"			//システムオプションパラメータ
#include "OutResult.h"			//結果出力パラメータ

#include "direct.h"

extern CCurrentStatus	current;			//共有情報
extern CSysParamCom		com_parameter;		//通信設定パラメータ
extern CSysParamDevice	dev_parameter;		//デバイスパラメータ
extern CCameraSettings	cam_parameter[8];	//カメラパラメータ
extern CScopes			sys_scope[16];		//システムスコープパラメータ
extern CTemplates		sys_template[256];	//システムテンプレートパラメータ
extern CTemplatesGroup	grp_template;		//テンプレートグループパラメータ
extern CLayer			sys_layer[8];		//システムレイヤパラメータ
extern CSysOption		sys_option;			//システムオプションパラメータ
extern COutResult		sys_outrslt;		//結果出力パラメータ

extern char* APP_ROOT;

CAccessParamSys::CAccessParamSys()
{
}


CAccessParamSys::~CAccessParamSys()
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
int CAccessParamSys::CheckDir()
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

	return 0;
}


/** ****************************************************** **
 ** Info.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileInfo()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Info.ini";
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
 ** Info.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileInfo()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFileInfo();

	//最後にiniファイルの確認
	if (CheckIniFileInfo() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Info.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileInfo()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Info.ini";

	//現状情報
	buf = Tips0.IntToString(current.work_number);
	ret = WritePrivateProfileStringA("CurrentInfo", "work_number", buf, filename);

	//メイン画面表示
	buf = Tips0.IntToString(current.baseline_type);
	ret = WritePrivateProfileStringA("MainView", "baseline_type", buf, filename);

	//カメラ情報(1～8)
	CString camera;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) camera = "Camera1";
		if (cam_number == 1) camera = "Camera2";
		if (cam_number == 2) camera = "Camera3";
		if (cam_number == 3) camera = "Camera4";
		if (cam_number == 4) camera = "Camera5";
		if (cam_number == 5) camera = "Camera6";
		if (cam_number == 6) camera = "Camera7";
		if (cam_number == 7) camera = "Camera8";
		buf = Tips0.IntToString((int)current.cam[cam_number].last_cmd.cmd);
		ret = WritePrivateProfileStringA(camera, "last_cmd_no", buf, filename);
		buf = Tips0.IntToString(current.cam[cam_number].last_cmd.template_number);
		ret = WritePrivateProfileStringA(camera, "last_cmd_template", buf, filename);
		buf = Tips0.IntToString(current.cam[cam_number].last_cmd.scope);
		ret = WritePrivateProfileStringA(camera, "last_cmd_scope", buf, filename);
		buf = Tips0.IntToString(current.cam[cam_number].last_cmd.view);
		ret = WritePrivateProfileStringA(camera, "last_cmd_view", buf, filename);
		buf = Tips0.IntToString((int)current.cam[cam_number].last_cmd.search_method);
		ret = WritePrivateProfileStringA(camera, "last_cmd_search_method", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** Info.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileInfo()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Info.ini";

	//現状情報
	ret = GetPrivateProfileStringA("CurrentInfo", "work_number", "", buf, sizeof(buf), filename);
	if (ret != 0) current.work_number = atoi(buf);

	//メイン画面表示
	ret = GetPrivateProfileStringA("MainView", "baseline_type", "", buf, sizeof(buf), filename);
	if (ret != 0) current.baseline_type = atoi(buf);

	//カメラ情報(1～4)
	CString camera;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) camera = "Camera1";
		if (cam_number == 1) camera = "Camera2";
		if (cam_number == 2) camera = "Camera3";
		if (cam_number == 3) camera = "Camera4";
		if (cam_number == 4) camera = "Camera5";
		if (cam_number == 5) camera = "Camera6";
		if (cam_number == 6) camera = "Camera7";
		if (cam_number == 7) camera = "Camera8";
		ret = GetPrivateProfileStringA(camera, "last_cmd_no", "", buf, sizeof(buf), filename);
		if (ret != 0) current.cam[cam_number].last_cmd.cmd = (byte)atoi(buf);
		ret = GetPrivateProfileStringA(camera, "last_cmd_template", "", buf, sizeof(buf), filename);
		if (ret != 0) current.cam[cam_number].last_cmd.template_number = atoi(buf);
		ret = GetPrivateProfileStringA(camera, "last_cmd_scope", "", buf, sizeof(buf), filename);
		if (ret != 0) current.cam[cam_number].last_cmd.scope = atoi(buf);
		ret = GetPrivateProfileStringA(camera, "last_cmd_view", "", buf, sizeof(buf), filename);
		if (ret != 0) current.cam[cam_number].last_cmd.view = atoi(buf);
		ret = GetPrivateProfileStringA(camera, "last_cmd_search_method", "", buf, sizeof(buf), filename);
		if (ret != 0) current.cam[cam_number].last_cmd.search_method = (WORD)atoi(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** Com.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileCom()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Com.ini";
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
 ** Com.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileCom()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFileCom();

	//最後にiniファイルの確認
	if (CheckIniFileCom() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Com.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileCom()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Com.ini";

	//PLCタイプ
	buf = Tips0.IntToString((int)com_parameter.plc.type);
	ret = WritePrivateProfileStringA("PLCParameter", "type", buf, filename);

	//デバイス設定
	buf = Tips0.IntToString(com_parameter.device.read_number);
	ret = WritePrivateProfileStringA("DeviceNo", "read_number", buf, filename);
	buf = Tips0.IntToString(com_parameter.device.write_number);
	ret = WritePrivateProfileStringA("DeviceNo", "write_number", buf, filename);

	//TCP/IP設定
	buf = Tips0.IntToString((int)com_parameter.tcp.enable);
	ret = WritePrivateProfileStringA("TCPIPParameter", "enable", buf, filename);
	buf = com_parameter.tcp.ip_address;
	ret = WritePrivateProfileStringA("TCPIPParameter", "ip_address", buf, filename);
	buf = Tips0.IntToString(com_parameter.tcp.port_number);
	ret = WritePrivateProfileStringA("TCPIPParameter", "port_number", buf, filename);

	//RS232設定
	buf = Tips0.IntToString((int)com_parameter.rs232.enable);
	ret = WritePrivateProfileStringA("RS232Parameter", "enable", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.com_port);
	ret = WritePrivateProfileStringA("RS232Parameter", "com_port", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.boudrate);
	ret = WritePrivateProfileStringA("RS232Parameter", "boudrate", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.data_bit);
	ret = WritePrivateProfileStringA("RS232Parameter", "data_bit", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.parity);
	ret = WritePrivateProfileStringA("RS232Parameter", "parity", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.stop_bit);
	ret = WritePrivateProfileStringA("RS232Parameter", "stop_bit", buf, filename);
	buf = Tips0.IntToString(com_parameter.rs232.flow_control);
	ret = WritePrivateProfileStringA("RS232Parameter", "flow_control", buf, filename);

	//交信設定
	buf = Tips0.IntToString(com_parameter.com.type);
	ret = WritePrivateProfileStringA("Communication", "type", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** Com.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileCom()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Com.ini";

	//PLCタイプ
	ret = GetPrivateProfileStringA("PLCParameter", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.plc.type = (byte)atoi(buf);

	//デバイス設定
	ret = GetPrivateProfileStringA("DeviceNo", "read_number", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.device.read_number = atoi(buf);
	ret = GetPrivateProfileStringA("DeviceNo", "write_number", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.device.write_number = atoi(buf);

	//TCP/IP設定
	ret = GetPrivateProfileStringA("TCPIPParameter", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.tcp.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("TCPIPParameter", "ip_address", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.tcp.ip_address = buf;
	ret = GetPrivateProfileStringA("TCPIPParameter", "port_number", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.tcp.port_number = atoi(buf);

	//RS232設定
	ret = GetPrivateProfileStringA("RS232Parameter", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "com_port", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.com_port = atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "boudrate", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.boudrate = atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "data_bit", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.data_bit = atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "parity", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.parity = atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "stop_bit", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.stop_bit = atoi(buf);
	ret = GetPrivateProfileStringA("RS232Parameter", "flow_control", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.rs232.flow_control = atoi(buf);

	//交信設定
	ret = GetPrivateProfileStringA("Communication", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) com_parameter.com.type = atoi(buf);

	return 0;
}

/** ****************************************************** **
 ** Device.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileDevice()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Device.ini";
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
 ** Device.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileDevice()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//デフォルト設定でファイル作成
	WriteIniFileDevice();

	//最後にiniファイルの確認
	if (CheckIniFileDevice() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Device.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileDevice()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Device.ini";

	//画像入力ボード設定1
	buf = Tips0.IntToString((int)dev_parameter.grab[0].interface_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter", "interface_code", buf, filename);
	buf = Tips0.IntToString((int)dev_parameter.grab[0].vender_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter", "vender_code", buf, filename);
	buf = Tips0.IntToString((int)dev_parameter.grab[0].model_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter", "model_code", buf, filename);

	//画像入力ボード設定2
	buf = Tips0.IntToString((int)dev_parameter.grab[1].interface_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter2", "interface_code", buf, filename);
	buf = Tips0.IntToString((int)dev_parameter.grab[1].vender_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter2", "vender_code", buf, filename);
	buf = Tips0.IntToString((int)dev_parameter.grab[1].model_code);
	ret = WritePrivateProfileStringA("GrabBoardParameter2", "model_code", buf, filename);

	//デジタルI/Oボード
	buf = dev_parameter.dio.name_device_io;
	ret = WritePrivateProfileStringA("DIOParameter", "name_device_io", buf, filename);

	//カメラ設定(1～8)
	CString device_name;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) device_name = "CameraDevice1";
		if (cam_number == 1) device_name = "CameraDevice2";
		if (cam_number == 2) device_name = "CameraDevice3";
		if (cam_number == 3) device_name = "CameraDevice4";
		if (cam_number == 4) device_name = "CameraDevice5";
		if (cam_number == 5) device_name = "CameraDevice6";
		if (cam_number == 6) device_name = "CameraDevice7";
		if (cam_number == 7) device_name = "CameraDevice8";
		buf = Tips0.IntToString((int)cam_parameter[cam_number].device.vender_code);
		ret = WritePrivateProfileStringA(device_name, "vender_code", buf, filename);
		buf = Tips0.IntToString((int)cam_parameter[cam_number].device.model_code);
		ret = WritePrivateProfileStringA(device_name, "model_code", buf, filename);
		buf = Tips0.IntToString(cam_parameter[cam_number].device.grab_width);
		ret = WritePrivateProfileStringA(device_name, "grab_width", buf, filename);
		buf = Tips0.IntToString(cam_parameter[cam_number].device.grab_height);
		ret = WritePrivateProfileStringA(device_name, "grab_height", buf, filename);
		buf = cam_parameter[cam_number].device.inifile_path;
		ret = WritePrivateProfileStringA(device_name, "inifile_path", buf, filename);
		buf = Tips0.IntToString((int)cam_parameter[cam_number].device.color);
		ret = WritePrivateProfileStringA(device_name, "color", buf, filename);
		buf = Tips0.IntToString(cam_parameter[cam_number].coordinate_type);
		ret = WritePrivateProfileStringA(device_name, "coordinate_type", buf, filename);
		buf = cam_parameter[cam_number].device.nick_name;
		ret = WritePrivateProfileStringA(device_name, "nick_name", buf, filename);
		buf = Tips0.IntToString(cam_parameter[cam_number].regi_setting);
		ret = WritePrivateProfileStringA(device_name, "regi_setting", buf, filename);
		buf = cam_parameter[cam_number].device.serial_number;
		ret = WritePrivateProfileStringA(device_name, "serial_number", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** Device.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileDevice()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Device.ini";

	//画像入力ボード設定1
	ret = GetPrivateProfileStringA("GrabBoardParameter", "interface_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[0].interface_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("GrabBoardParameter", "vender_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[0].vender_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("GrabBoardParameter", "model_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[0].model_code = (WORD)atoi(buf);

	//画像入力ボード設定2
	ret = GetPrivateProfileStringA("GrabBoardParameter2", "interface_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[1].interface_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("GrabBoardParameter2", "vender_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[1].vender_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("GrabBoardParameter2", "model_code", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.grab[1].model_code = (WORD)atoi(buf);

	//デジタルI/Oボード
	ret = GetPrivateProfileStringA("DIOParameter", "name_device_io", "", buf, sizeof(buf), filename);
	if (ret != 0) dev_parameter.dio.name_device_io = buf;

	//カメラ設定(1～8)
	CString device_name;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) device_name = "CameraDevice1";
		if (cam_number == 1) device_name = "CameraDevice2";
		if (cam_number == 2) device_name = "CameraDevice3";
		if (cam_number == 3) device_name = "CameraDevice4";
		if (cam_number == 4) device_name = "CameraDevice5";
		if (cam_number == 5) device_name = "CameraDevice6";
		if (cam_number == 6) device_name = "CameraDevice7";
		if (cam_number == 7) device_name = "CameraDevice8";
		ret = GetPrivateProfileStringA(device_name, "vender_code", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.vender_code = (byte)atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "model_code", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.model_code = (WORD)atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "grab_width", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.grab_width = atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "grab_height", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.grab_height = atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "inifile_path", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.inifile_path = buf;
		ret = GetPrivateProfileStringA(device_name, "color", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.color = (byte)atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "coordinate_type", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].coordinate_type = atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "nick_name", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.nick_name = buf;
		ret = GetPrivateProfileStringA(device_name, "regi_setting", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].regi_setting = atoi(buf);
		ret = GetPrivateProfileStringA(device_name, "serial_number", "", buf, sizeof(buf), filename);
		if (ret != 0) cam_parameter[cam_number].device.serial_number = buf;
	}

	return 0;
}


/** ****************************************************** **
 ** Scope.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileScope()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Scope.ini";
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
 ** Scope.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileScope()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//デフォルト設定でファイル作成
	WriteIniFileScope();

	//最後にiniファイルの確認
	if (CheckIniFileScope() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Scope.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileScope()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Scope.ini";

	CString scope_name;
	CString view_name;
	for (int scope_number = 0; scope_number < 16; scope_number++) {
		//スコープ設定
		scope_name = "Scope" + Tips0.IntToString(scope_number + 1);		//例)Scope1
		buf = sys_scope[scope_number].name;
		ret = WritePrivateProfileStringA(scope_name, "name", buf, filename);

		//視野パラメータ(1～4)
		for (int view_number = 0; view_number < 4; view_number++) {
			view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1
			buf = Tips0.IntToString(sys_scope[scope_number].view[view_number].assign_camera);
			ret = WritePrivateProfileStringA(view_name, "assign_camera", buf, filename);
		}
	}

	return 0;
}


/** ****************************************************** **
 ** Scope.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileScope()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Scope.ini";

	CString scope_name;
	CString view_name;
	for (int scope_number = 0; scope_number < 16; scope_number++) {
		//スコープ設定
		scope_name = "Scope" + Tips0.IntToString(scope_number + 1);		//例)Scope1
		ret = GetPrivateProfileStringA(scope_name, "name", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_scope[scope_number].name = buf;

		//視野パラメータ(1～4)
		for (int view_number = 0; view_number < 4; view_number++) {
			view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1
			ret = GetPrivateProfileStringA(view_name, "assign_camera", "", buf, sizeof(buf), filename);
			if (ret != 0) sys_scope[scope_number].view[view_number].assign_camera = atoi(buf);
		}
	}

	return 0;
}


/** ****************************************************** **
 ** Template.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileTemplate()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Template.ini";
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
 ** Template.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileTemplate()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//デフォルト設定でファイル作成
	WriteIniFileTemplate();

	//最後にiniファイルの確認
	if (CheckIniFileTemplate() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Template.iniファイルの書込(全テンプレート)
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileTemplate()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Template.ini";

	//テンプレート設定
	CString template_name;
	for (int template_number = 0; template_number < 256; template_number++) {
		template_name = "Template" + Tips0.IntToString(template_number);		//例)Template0
		buf = sys_template[template_number].name;
		ret = WritePrivateProfileStringA(template_name, "name", buf, filename);
		buf = sys_template[template_number].comment;
		ret = WritePrivateProfileStringA(template_name, "comment", buf, filename);
		buf = Tips0.IntToString(sys_template[template_number].scope);
		ret = WritePrivateProfileStringA(template_name, "scope", buf, filename);
		buf = Tips0.IntToString(sys_template[template_number].view);
		ret = WritePrivateProfileStringA(template_name, "view", buf, filename);
		buf = Tips0.IntToString((int)sys_template[template_number].method_type);
		ret = WritePrivateProfileStringA(template_name, "method_type", buf, filename);
	}

	//グループ設定
	CString group_name;
	for (int group_number = 0; group_number < 4; group_number++) {
		group_name = "Group" + Tips0.IntToString(group_number + 1);		//例)Group1
		buf = Tips0.IntToString(grp_template.group[group_number].first_number);
		ret = WritePrivateProfileStringA(group_name, "first_number", buf, filename);
		buf = Tips0.IntToString(grp_template.group[group_number].end_number);
		ret = WritePrivateProfileStringA(group_name, "end_number", buf, filename);
		buf = grp_template.group[group_number].group_name;
		ret = WritePrivateProfileStringA(group_name, "group_name", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** Template.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileTemplate()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Template.ini";

	//テンプレート設定
	CString template_name;
	for (int template_number = 0; template_number < 256; template_number++) {
		template_name = "Template" + Tips0.IntToString(template_number);		//例)Template0
		ret = GetPrivateProfileStringA(template_name, "name", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].name = buf;
		ret = GetPrivateProfileStringA(template_name, "comment", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].comment = buf;
		ret = GetPrivateProfileStringA(template_name, "scope", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].scope = atoi(buf);
		ret = GetPrivateProfileStringA(template_name, "view", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].view = atoi(buf);
		ret = GetPrivateProfileStringA(template_name, "method_type", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].method_type = (byte)atoi(buf);
	}

	//グループ設定
	CString group_name;
	for (int group_number = 0; group_number < 4; group_number++) {
		group_name = "Group" + Tips0.IntToString(group_number + 1);		//例)Group1
		ret = GetPrivateProfileStringA(group_name, "first_number", "", buf, sizeof(buf), filename);
		if (ret != 0) grp_template.group[group_number].first_number = atoi(buf);
		ret = GetPrivateProfileStringA(group_name, "end_number", "", buf, sizeof(buf), filename);
		if (ret != 0) grp_template.group[group_number].end_number = atoi(buf);
		ret = GetPrivateProfileStringA(group_name, "group_name", "", buf, sizeof(buf), filename);
		if (ret != 0) grp_template.group[group_number].group_name = buf;
	}

	return 0;
}

/** ****************************************************** **
 ** Template.iniファイルの書込(1テンプレート)
 ** 引数：
 **　int template_number　書込テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileTemplate(int template_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Template.ini";

	//テンプレート設定
	CString template_name;
	template_name = "Template" + Tips0.IntToString(template_number);		//例)Template0
	buf = sys_template[template_number].name;
	ret = WritePrivateProfileStringA(template_name, "name", buf, filename);
	buf = sys_template[template_number].comment;
	ret = WritePrivateProfileStringA(template_name, "comment", buf, filename);
	buf = Tips0.IntToString(sys_template[template_number].scope);
	ret = WritePrivateProfileStringA(template_name, "scope", buf, filename);
	buf = Tips0.IntToString(sys_template[template_number].view);
	ret = WritePrivateProfileStringA(template_name, "view", buf, filename);
	buf = Tips0.IntToString((int)sys_template[template_number].method_type);
	ret = WritePrivateProfileStringA(template_name, "method_type", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** Template.iniファイルの書込(グループ)
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileTempGroup()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Template.ini";

	//グループ設定
	CString group_name;
	for (int group_number = 0; group_number < 4; group_number++) {
		group_name = "Group" + Tips0.IntToString(group_number + 1);		//例)Group1
		buf = Tips0.IntToString(grp_template.group[group_number].first_number);
		ret = WritePrivateProfileStringA(group_name, "first_number", buf, filename);
		buf = Tips0.IntToString(grp_template.group[group_number].end_number);
		ret = WritePrivateProfileStringA(group_name, "end_number", buf, filename);
		buf = grp_template.group[group_number].group_name;
		ret = WritePrivateProfileStringA(group_name, "group_name", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** TemplateUniq.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileTempSpec()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\SPEC\\TemplateUniq.ini";

	//特殊設定
	CString template_name;
	for (int template_number = 0; template_number < 256; template_number++) {
		template_name = "Template" + Tips0.IntToString(template_number);		//例)Template0

		//ブロブ抽出結果描画
		ret = GetPrivateProfileStringA(template_name, "blob_draw_cmd_result", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_template[template_number].spec.blob.draw_cmd_result = atoi(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** Layer.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileLayer()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Layer.ini";
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
 ** Layer.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileLayer()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFileLayer();

	//最後にiniファイルの確認
	if (CheckIniFileLayer() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Layer.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileLayer()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Layer.ini";

	CString camera;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) camera = "Camera1";
		if (cam_number == 1) camera = "Camera2";
		if (cam_number == 2) camera = "Camera3";
		if (cam_number == 3) camera = "Camera4";
		if (cam_number == 4) camera = "Camera5";
		if (cam_number == 5) camera = "Camera6";
		if (cam_number == 6) camera = "Camera7";
		if (cam_number == 7) camera = "Camera8";

		//カメラ座標表示
		buf = Tips0.IntToString((int)sys_layer[cam_number].cord_sign.enable);
		ret = WritePrivateProfileStringA(camera + "_cord_sign", "enable", buf, filename);
		buf = Tips0.IntToString(sys_layer[cam_number].cord_sign.position);
		ret = WritePrivateProfileStringA(camera + "_cord_sign", "position", buf, filename);
		Tips0.conv1.val_dw = sys_layer[cam_number].cord_sign.color;
		buf = Tips0.HexToString(Tips0.conv1.val_w[0]);
		ret = WritePrivateProfileStringA(camera + "_cord_sign", "color_lower", buf, filename);
		buf = Tips0.HexToString(Tips0.conv1.val_w[1]);
		ret = WritePrivateProfileStringA(camera + "_cord_sign", "color_upper", buf, filename);

		//カメラ名表示
		buf = Tips0.IntToString((int)sys_layer[cam_number].cam_sign.enable);
		ret = WritePrivateProfileStringA(camera + "_cam_sign", "enable", buf, filename);
		buf = Tips0.IntToString(sys_layer[cam_number].cam_sign.position);
		ret = WritePrivateProfileStringA(camera + "_cam_sign", "position", buf, filename);
		Tips0.conv1.val_dw = sys_layer[cam_number].cam_sign.color;
		buf = Tips0.HexToString(Tips0.conv1.val_w[0]);
		ret = WritePrivateProfileStringA(camera + "_cam_sign", "color_lower", buf, filename);
		buf = Tips0.HexToString(Tips0.conv1.val_w[1]);
		ret = WritePrivateProfileStringA(camera + "_cam_sign", "color_upper", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** Layer.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileLayer()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Layer.ini";

	CString camera;
	for (int cam_number = 0; cam_number < 8; cam_number++) {
		if (cam_number == 0) camera = "Camera1";
		if (cam_number == 1) camera = "Camera2";
		if (cam_number == 2) camera = "Camera3";
		if (cam_number == 3) camera = "Camera4";
		if (cam_number == 4) camera = "Camera5";
		if (cam_number == 5) camera = "Camera6";
		if (cam_number == 6) camera = "Camera7";
		if (cam_number == 7) camera = "Camera8";

		//カメラ座標表示
		ret = GetPrivateProfileStringA(camera + "_cord_sign", "enable", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_layer[cam_number].cord_sign.enable = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(camera + "_cord_sign", "position", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_layer[cam_number].cord_sign.position = atoi(buf);
		ret = GetPrivateProfileStringA(camera + "_cord_sign", "color_lower", "", buf, sizeof(buf), filename);
		if (ret != 0) {
			Tips0.conv1.val_w[0] = (WORD)Tips0.CharToBin(buf, 16);
			ret = GetPrivateProfileStringA(camera + "_cord_sign", "color_upper", "", buf, sizeof(buf), filename);
			if (ret != 0) {
				Tips0.conv1.val_w[1] = (WORD)Tips0.CharToBin(buf, 16);
				sys_layer[cam_number].cord_sign.color = Tips0.conv1.val_dw;
			}
		}

		//カメラ名表示
		ret = GetPrivateProfileStringA(camera + "_cam_sign", "enable", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_layer[cam_number].cam_sign.enable = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(camera + "_cam_sign", "position", "", buf, sizeof(buf), filename);
		if (ret != 0) sys_layer[cam_number].cam_sign.position = atoi(buf);
		ret = GetPrivateProfileStringA(camera + "_cam_sign", "color_lower", "", buf, sizeof(buf), filename);
		if (ret != 0) {
			Tips0.conv1.val_w[0] = (WORD)Tips0.CharToBin(buf, 16);
			ret = GetPrivateProfileStringA(camera + "_cam_sign", "color_upper", "", buf, sizeof(buf), filename);
			if (ret != 0) {
				Tips0.conv1.val_w[1] = (WORD)Tips0.CharToBin(buf, 16);
				sys_layer[cam_number].cam_sign.color = Tips0.conv1.val_dw;
			}
		}
	}

	return 0;
}


/** ****************************************************** **
 ** Option.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileOption()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Option.ini";
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
 ** Option.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileOption()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFileOption();

	//最後にiniファイルの確認
	if (CheckIniFileOption() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Option.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileOption()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Option.ini";

	//
	//システム系
	//

	//Fビット制御方法
	buf = Tips0.IntToString((int)sys_option.bitF_mode);
	ret = WritePrivateProfileStringA("System", "bitF_mode", buf, filename);

	//システム開始タイマ
	buf = Tips0.IntToString(sys_option.start_timer);
	ret = WritePrivateProfileStringA("System", "start_timer", buf, filename);

	//マルチディスプレイ対応(番号)
	buf = Tips0.IntToString(sys_option.multi_display);
	ret = WritePrivateProfileStringA("System", "multi_display", buf, filename);

	//言語
	buf = Tips0.IntToString((int)sys_option.language);
	ret = WritePrivateProfileStringA("System", "language", buf, filename);

	//画像保存フォルダ
	buf = sys_option.image_save_dir;
	ret = WritePrivateProfileStringA("System", "image_save_dir", buf, filename);

	//テンプレート結果保存フォルダ
	buf = sys_option.template_result_dir;
	ret = WritePrivateProfileStringA("System", "template_result_dir", buf, filename);

	//4視野アライメント演算方法
	buf = Tips0.IntToString(sys_option.fourparts_align);
	ret = WritePrivateProfileStringA("System", "fourparts_align", buf, filename);

	//I/OUnit無し
	buf = Tips0.IntToString((int)sys_option.io_unit_less);
	ret = WritePrivateProfileStringA("System", "io_unit_less", buf, filename);

	//ビット先頭デバイスX
	buf = sys_option.x_device_start_number;
	buf.MakeUpper();
	ret = WritePrivateProfileStringA("System", "x_device_start_number", buf, filename);

	//ビット先頭デバイスY
	buf = sys_option.y_device_start_number;
	buf.MakeUpper();
	ret = WritePrivateProfileStringA("System", "y_device_start_number", buf, filename);

	//
	//ユーザー系
	//

	//コマンド実行後の結果表示時間(秒)
	buf = Tips0.IntToString(sys_option.cmd_view_time);
	ret = WritePrivateProfileStringA("User", "cmd_view_time", buf, filename);

	//コマンド画像保存
	buf = Tips0.IntToString((int)sys_option.image_save_mode);
	ret = WritePrivateProfileStringA("User", "image_save_mode", buf, filename);

	//画像保存形式
	buf = Tips0.IntToString((int)sys_option.file_type);
	ret = WritePrivateProfileStringA("User", "file_type", buf, filename);

	//追加モニタ カメラ番号
	buf = Tips0.IntToString(sys_option.add_monitor_cam);
	ret = WritePrivateProfileStringA("User", "add_monitor_cam", buf, filename);

	//検出十字サイズ カメラ1～8
	CString item_name;
	for (int i = 0; i < 8; i++) {
		item_name = "size_point_srch" + Tips0.IntToString(i + 1);		//例)size_point_srch1
		buf = Tips0.IntToString(sys_option.size_point_srch[i]);
		ret = WritePrivateProfileStringA("User", item_name, buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** Option.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileOption()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\Option.ini";

	//
	//システム系
	//

	//Fビット制御方法
	ret = GetPrivateProfileStringA("System", "bitF_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.bitF_mode = (byte)atoi(buf);

	//システム開始タイマ
	ret = GetPrivateProfileStringA("System", "start_timer", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.start_timer = atoi(buf);

	//マルチディスプレイ対応(番号)
	ret = GetPrivateProfileStringA("System", "multi_display", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.multi_display = atoi(buf);

	//言語
	ret = GetPrivateProfileStringA("System", "language", "", buf, sizeof(buf), filename);
	if (ret != 0)sys_option.language = (byte)atoi(buf);

	//画像保存フォルダ
	ret = GetPrivateProfileStringA("System", "image_save_dir","" , buf, sizeof(buf), filename);
	if (ret != 0)sys_option.image_save_dir = buf;

	//テンプレート結果保存フォルダ
	ret = GetPrivateProfileStringA("System", "template_result_dir", "", buf, sizeof(buf), filename);
	if (ret != 0)sys_option.template_result_dir = buf;

	//4視野アライメント演算方法
	ret = GetPrivateProfileStringA("System", "fourparts_align", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.fourparts_align = atoi(buf);

	//I/OUnit無し
	ret = GetPrivateProfileStringA("System", "io_unit_less", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.io_unit_less = (BOOL)atoi(buf);

	//ビット先頭デバイスX
	ret = GetPrivateProfileStringA("System", "x_device_start_number", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.x_device_start_number = buf;

	//ビット先頭デバイスY
	ret = GetPrivateProfileStringA("System", "y_device_start_number", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.y_device_start_number = buf;

	//
	//ユーザー系
	//

	//コマンド実行後の結果表示時間(秒)
	ret = GetPrivateProfileStringA("User", "cmd_view_time", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.cmd_view_time = atoi(buf);

	//コマンド画像保存
	ret = GetPrivateProfileStringA("User", "image_save_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.image_save_mode = (byte)atoi(buf);

	//画像保存形式
	ret = GetPrivateProfileStringA("User", "file_type", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.file_type = (byte)atoi(buf);

	//追加モニタ カメラ番号
	ret = GetPrivateProfileStringA("User", "add_monitor_cam", "", buf, sizeof(buf), filename);
	if (ret != 0) sys_option.add_monitor_cam = atoi(buf);

	//検出十字サイズ カメラ1～8
	CString item_name;
	for (int i = 0; i < 8; i++) {
		item_name = "size_point_srch" + Tips0.IntToString(i + 1);		//例)size_point_srch1
		ret = GetPrivateProfileStringA("User", item_name, "", buf, sizeof(buf), filename);
		if (ret != 0) sys_option.size_point_srch[i] = atoi(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** OutRslt.iniファイルの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CheckIniFileOutRslt()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\OutRslt.ini";
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
 ** OutRslt.iniファイルの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::CreateIniFileOutRslt()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFileOutRslt();

	//最後にiniファイルの確認
	if (CheckIniFileOutRslt() == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** OutRslt.iniファイルの書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::WriteIniFileOutRslt()
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\OutRslt.ini";

	//
	//オリジナル記号
	//

	CString format_name;
	for (int i = 0; i < 10; i++) {
		format_name = "OrgFormat" + Tips0.IntToString(i + 1);		//例)OrgFormat1
		//識別文字
		buf = sys_outrslt.org_format[i].symbol;
		ret = WritePrivateProfileStringA(format_name, "symbol", buf, filename);
		//記号説明
		buf = sys_outrslt.org_format[i].content;
		ret = WritePrivateProfileStringA(format_name, "content", buf, filename);
		//初期文字列
		buf = sys_outrslt.org_format[i].initial_val;
		ret = WritePrivateProfileStringA(format_name, "initial_val", buf, filename);
	}

	//初期文字列を現在設定文字列にコピー
	for (int i = 0; i < 10; i++) {
		strcpy_s(sys_outrslt.org_format[i].now_val, 128, sys_outrslt.org_format[i].initial_val);
	}

	return 0;
}


/** ****************************************************** **
 ** OutRslt.iniファイルの読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamSys::ReadIniFileOutRslt()
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\OutRslt.ini";

	//
	//オリジナル記号
	//

	CString format_name;
	for (int i = 0; i < 10; i++) {
		format_name = "OrgFormat" + Tips0.IntToString(i + 1);		//例)OrgFormat1
		//識別文字
		ret = GetPrivateProfileStringA(format_name, "symbol", "", buf, sizeof(buf), filename);
		if (ret != 0) strcpy_s(sys_outrslt.org_format[i].symbol, 128, buf);
		//記号説明
		ret = GetPrivateProfileStringA(format_name, "content", "", buf, sizeof(buf), filename);
		if (ret != 0) strcpy_s(sys_outrslt.org_format[i].content, 128, buf);
		//初期文字列
		ret = GetPrivateProfileStringA(format_name, "initial_val", "", buf, sizeof(buf), filename);
		if (ret != 0) strcpy_s(sys_outrslt.org_format[i].initial_val, 128, buf);
	}

	//初期文字列を現在設定文字列にコピー
	for (int i = 0; i < 10; i++) {
		strcpy_s(sys_outrslt.org_format[i].now_val, 128, sys_outrslt.org_format[i].initial_val);
	}

	return 0;
}
