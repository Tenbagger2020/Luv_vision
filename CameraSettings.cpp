/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CameraSettings.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2023.10.30	M.Iwasaki	10.14.2.0	東芝テリーUSBカメラ対応
 ** 注記：
 ** 説明：カメラパラメータ
 **		カメラデバイスのパラメータクラス
 **/

#include "stdafx.h"
#include "CameraSettings.h"


CCameraSettings::CCameraSettings()
{
	//ハードウェアパラメータ
	device.vender_code = 0x00;	//0x00:未定義
	device.model_code = 0x0000;	//0x0000:未定義
	device.grab_width = 0;		//0:未設定
	device.grab_height = 0;		//0:未設定
	device.inifile_path = "";	//未定義
	device.color = 0x00;		//0x00:白黒
	device.nick_name = "(NoName)";	//名称
	device.serial_number = "";		//シリアル番号

	//カメラパラメータ
	regi_setting = 0;	//0:初期値を使用

	//カメラ座標
	coordinate_type = 0;			//0:構成1
}


CCameraSettings::~CCameraSettings()
{
}
