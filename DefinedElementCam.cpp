/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DefinedElementCam.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.08.27	M.Iwasaki	東芝テリーBC505LMG対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.10.04	M.Iwasaki	K34004(ファーストボード対応)
 **			2023.10.30	M.Iwasaki	10.14.2.0	東芝テリーUSBカメラ対応
 **			2024.05.17	M.Iwasaki	10.17.2.0	オムロンセンテックカメラ対応
 **			2026.06.05	M.Iwasaki	10.21.1.0	赤外カメラ対応
 ** 注記：
 ** 説明：カメラパラメータ 各要素のコードと名称の紐づけ
 **/

#include "stdafx.h"
#include "DefinedElementCam.h"

CDefinedElementCam::CDefinedElementCam()
{
	/** ****************************************************** **
	 ** カメラベンダー
	 **		カメラメーカー情報
	 **		メーカー vender_code
	 **			0x00:なし
	 **			0x01:東芝テリー
	 **			0x02:CIS
	 **			0x03:VIEWORKS
	 **			0x04:OmronSentech
	 **			0x05:AVALDATA
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	vender[0].vender_code = 0x00;
	vender[0].s_text = "なし";

	vender[1].vender_code = 0x01;
	vender[1].s_text = "東芝テリー";

	vender[2].vender_code = 0x02;
	vender[2].s_text = "CIS";

	vender[3].vender_code = 0x03;
	vender[3].s_text = "VIEWORKS";

	vender[4].vender_code = 0x04;
	vender[4].s_text = "OmronSentech";

	vender[5].vender_code = 0x05;
	vender[5].s_text = "AVALDATA";


	/** ****************************************************** **
	 ** カメラデバイス
	 **		カメラデバイス情報
	 **		インターフェース種別(1桁) interface_code
	 **			0x01:カメラリンク
	 **			0x02:USB
	 **		メーカーコード(1桁) vender_code
	 **			0x01:東芝テリー
	 **			0x02:CIS
	 **			0x03:VIEWORKS
	 **			0x04:OmronSentech
	 **			0x05:AVALDATA
	 **		製品コード(2桁) product_code
	 **			連番
	 **		モデルコード(4桁) model_code
	 **			カメラ毎に一意の番号(16進16ビット)
	 **			model_code = (interface_code:1桁)(vender_code:1桁)(product_code:2桁)
	 **			0x1101:CSCV90BC3
	 **			0x1111:BC040M
	 **			0x1112:BC160M
	 **			0x1113:BC302LMG
	 **			0x1114:BC505LMG
	 **			0x1201:CIS CAM1
	 **			0x1202:CIS CAM2
	 **			0x1301:VC-25MC-M30D0
	 **			0x1411:STC-SPB43PCL
	 **			0x1412:STC-SPB163PCL
	 **			0x1413:STC-SPB312PCL
	 **			0x1414:STC-SPB500PCL
	 **			0x1501:ABA-003VIR
	 **			0x1502:ABA-013VIR
	 **			0x2111:BU040MG
	 **			0x2112:BU160MG
	 **			0x2113:BU302MG
	 **			0x2114:BC505LMG
	 **			0x2411:STC-MBS43U3V
	 **			0x2412:STC-MBS163U3V
	 **			0x2413:STC-MBS322U3V
	 **			0x2414:STC-MBS510U3V
	 **		テキスト(製品の型式) s_text
	 **			製品型式
	 **		画像サイズX grab_width
	 **			pixel
	 **		画像サイズY grab_height
	 **			pixel
	 **		カメラIniファイルパス inifile_path
	 **			カメラIniファイルのパス
	 **
	 ** ****************************************************** **/
	device[0].interface_code = 0x01;
	device[0].vender_code = 0x01;
	device[0].product_code = 0x01;
	device[0].model_code = 0x1101;
	device[0].s_text = "CSCV90BC3";
	device[0].grab_width = 640;
	device[0].grab_height = 480;
	device[0].inifile_path = "\\CamIniFile\\CSCV90BC3.ini";
	device[0].xmlfile_path = "";

	device[1].interface_code = 0x01;
	device[1].vender_code = 0x01;
	device[1].product_code = 0x11;
	device[1].model_code = 0x1111;
	device[1].s_text = "BC040M";
	device[1].grab_width = 720;
	device[1].grab_height = 540;
	device[1].inifile_path = "\\CamIniFile\\BC040M(1TAP).ini";
	device[1].xmlfile_path = "\\CamIniFile\\fvc08\\FVC08_BC040M_1TAP_50MHz.xml";

	device[2].interface_code = 0x01;
	device[2].vender_code = 0x01;
	device[2].product_code = 0x12;
	device[2].model_code = 0x1112;
	device[2].s_text = "BC160M";
	device[2].grab_width = 1440;
	device[2].grab_height = 1080;
	device[2].inifile_path = "\\CamIniFile\\BC160M(1TAP).ini";
	device[2].xmlfile_path = "\\CamIniFile\\fvc08\\FVC08_BC160M_1TAP_50MHz.xml";

	device[3].interface_code = 0x01;
	device[3].vender_code = 0x01;
	device[3].product_code = 0x13;
	device[3].model_code = 0x1113;
	device[3].s_text = "BC302LMG";
	device[3].grab_width = 2048;
	device[3].grab_height = 1536;
	device[3].inifile_path = "\\CamIniFile\\BC302LMG(2TAP).ini";
	device[3].xmlfile_path = "\\CamIniFile\\fvc08\\FVC08_BC302LMG_2TAP_83MHz.xml";

	device[4].interface_code = 0x01;
	device[4].vender_code = 0x02;
	device[4].product_code = 0x01;
	device[4].model_code = 0x1201;
	device[4].s_text = "CIS CAM1";
	device[4].grab_width = 640;
	device[4].grab_height = 480;
	device[4].inifile_path = "\\CamIniFile\\test1.ini";
	device[4].xmlfile_path = "";

	device[5].interface_code = 0x01;
	device[5].vender_code = 0x02;
	device[5].product_code = 0x02;
	device[5].model_code = 0x1202;
	device[5].s_text = "CIS CAM2";
	device[5].grab_width = 1280;
	device[5].grab_height = 1024;
	device[5].inifile_path = "\\CamIniFile\\test2.ini";
	device[5].xmlfile_path = "";

	device[6].interface_code = 0x01;
	device[6].vender_code = 0x01;
	device[6].product_code = 0x14;
	device[6].model_code = 0x1114;
	device[6].s_text = "BC505LMG";
	device[6].grab_width = 2448;
	device[6].grab_height = 2048;
	device[6].inifile_path = "\\CamIniFile\\BC505LM(C)G(2TAP).ini";
	device[6].xmlfile_path = "\\CamIniFile\\fvc08\\FVC08_BC505LMG_2TAP.xml";

	device[7].interface_code = 0x01;
	device[7].vender_code = 0x03;
	device[7].product_code = 0x01;
	device[7].model_code = 0x1301;
	device[7].s_text = "VC-25MC-M30D0";
	device[7].grab_width = 5120;
	device[7].grab_height = 5120;
	device[7].inifile_path = "\\CamIniFile\\VC-25MC-M(C)30D0(8tap).ini";
	device[7].xmlfile_path = "";

	device[8].interface_code = 0x02;
	device[8].vender_code = 0x01;
	device[8].product_code = 0x11;
	device[8].model_code = 0x2111;
	device[8].s_text = "BU040MG";
	device[8].grab_width = 720;
	device[8].grab_height = 540;
	device[8].inifile_path = "";
	device[8].xmlfile_path = "";

	device[9].interface_code = 0x02;
	device[9].vender_code = 0x01;
	device[9].product_code = 0x12;
	device[9].model_code = 0x2112;
	device[9].s_text = "BU160MG";
	device[9].grab_width = 1440;
	device[9].grab_height = 1080;
	device[9].inifile_path = "";
	device[9].xmlfile_path = "";

	device[10].interface_code = 0x02;
	device[10].vender_code = 0x01;
	device[10].product_code = 0x13;
	device[10].model_code = 0x2113;
	device[10].s_text = "BU302MG";
	device[10].grab_width = 2048;
	device[10].grab_height = 1536;
	device[10].inifile_path = "";
	device[10].xmlfile_path = "";

	device[11].interface_code = 0x02;
	device[11].vender_code = 0x01;
	device[11].product_code = 0x14;
	device[11].model_code = 0x2114;
	device[11].s_text = "BU505MG";
	device[11].grab_width = 2448;
	device[11].grab_height = 2048;
	device[11].inifile_path = "";
	device[11].xmlfile_path = "";

	device[12].interface_code = 0x01;
	device[12].vender_code = 0x04;
	device[12].product_code = 0x11;
	device[12].model_code = 0x1411;
	device[12].s_text = "STC-SPB43PCL";
	device[12].grab_width = 720;
	device[12].grab_height = 540;
	device[12].inifile_path = "\\CamIniFile\\STC-SPB(C)43PCL(2TAP).ini";
	device[12].xmlfile_path = "";

	device[13].interface_code = 0x01;
	device[13].vender_code = 0x04;
	device[13].product_code = 0x12;
	device[13].model_code = 0x1412;
	device[13].s_text = "STC-SPB163PCL";
	device[13].grab_width = 1440;
	device[13].grab_height = 1080;
	device[13].inifile_path = "\\CamIniFile\\STC-SPB(C)163PCL(2TAP).ini";
	device[13].xmlfile_path = "";

	device[14].interface_code = 0x01;
	device[14].vender_code = 0x04;
	device[14].product_code = 0x13;
	device[14].model_code = 0x1413;
	device[14].s_text = "STC-SPB312PCL";
	device[14].grab_width = 2048;
	device[14].grab_height = 1536;
	device[14].inifile_path = "\\CamIniFile\\STC-SPB(C)312PCL(2TAP).ini";
	device[14].xmlfile_path = "";

	device[15].interface_code = 0x01;
	device[15].vender_code = 0x04;
	device[15].product_code = 0x14;
	device[15].model_code = 0x1414;
	device[15].s_text = "STC-SPB500PCL";
	device[15].grab_width = 2448;
	device[15].grab_height = 2048;
	device[15].inifile_path = "\\CamIniFile\\STC-SPB(C)500PCL(2TAP).ini";
	device[15].xmlfile_path = "";

	device[16].interface_code = 0x02;
	device[16].vender_code = 0x04;
	device[16].product_code = 0x11;
	device[16].model_code = 0x2411;
	device[16].s_text = "STC-MBS43U3V";
	device[16].grab_width = 720;
	device[16].grab_height = 540;
	device[16].inifile_path = "";
	device[16].xmlfile_path = "";

	device[17].interface_code = 0x02;
	device[17].vender_code = 0x04;
	device[17].product_code = 0x12;
	device[17].model_code = 0x2412;
	device[17].s_text = "STC-MBS163U3V";
	device[17].grab_width = 1440;
	device[17].grab_height = 1080;
	device[17].inifile_path = "";
	device[17].xmlfile_path = "";

	device[18].interface_code = 0x02;
	device[18].vender_code = 0x04;
	device[18].product_code = 0x13;
	device[18].model_code = 0x2413;
	device[18].s_text = "STC-MBS322U3V";
	device[18].grab_width = 2048;
	device[18].grab_height = 1536;
	device[18].inifile_path = "";
	device[18].xmlfile_path = "";

	device[19].interface_code = 0x02;
	device[19].vender_code = 0x04;
	device[19].product_code = 0x14;
	device[19].model_code = 0x2414;
	device[19].s_text = "STC-MBS510U3V";
	device[19].grab_width = 2448;
	device[19].grab_height = 2048;
	device[19].inifile_path = "";
	device[19].xmlfile_path = "";

	device[20].interface_code = 0x01;
	device[20].vender_code = 0x05;
	device[20].product_code = 0x01;
	device[20].model_code = 0x1501;
	device[20].s_text = "ABA-003VIR";
	device[20].grab_width = 624;
	device[20].grab_height = 512;
	device[20].inifile_path = "\\CamIniFile\\ABA-003VIR-CL(3TAP).ini";
	device[20].xmlfile_path = "";

	device[21].interface_code = 0x01;
	device[21].vender_code = 0x05;
	device[21].product_code = 0x02;
	device[21].model_code = 0x1502;
	device[21].s_text = "ABA-013VIR";
	device[21].grab_width = 1248;
	device[21].grab_height = 1024;
	device[21].inifile_path = "\\CamIniFile\\ABA-013VIR-CL(3TAP).ini";
	device[21].xmlfile_path = "";
}


CDefinedElementCam::~CDefinedElementCam()
{
}
