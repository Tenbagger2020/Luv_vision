/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DefinedElementSys.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.10.04	M.Iwasaki	K34004(ファーストボード対応)
 ** 注記：
 ** 説明：システムパラメータ 各要素のコードと名称の紐づけデータベース
 **/

#include "stdafx.h"
#include "DefinedElementSys.h"


CDefinedElementSys::CDefinedElementSys()
{
	/** ****************************************************** **
	 ** PLCタイプ
	 **		接続するシーケンサ情報
	 **		PLCタイプ type
	 **			0x01:三菱 QCPU
	 **			0x11:キーエンス KV
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	plc[0].type = 0x01;
	plc[0].s_text = "三菱 QCPU";

	plc[1].type = 0x11;
	plc[1].s_text = "キーエンス KV";


	/** ****************************************************** **
	 ** 画像入力ボード
	 **		画像入力ボード情報
	 **		インターフェース種別(1桁) interface_code
	 **			0x01:カメラリンク
	 **		メーカーコード(1桁) vender_code
	 **			0x01:AVALDATA
	 **			0x02:FAST
	 **		製品コード(2桁) product_code
	 **			連番
	 **		モデルコード(4桁) model_code
	 **			ボード毎に一意の番号(16進16ビット)
	 **			model_code = (interface_code:1桁)(vender_code:1桁)(product_code:2桁)
	 **			0x1101:APX-3324A
	 **			0x1102:APX-3323
	 **			0x1201:FVC08CLB
	 **		テキスト s_text
	 **			製品型式
	 **
	 ** ****************************************************** **/
	grab_model[0].interface_code = 0x01;
	grab_model[0].vender_code = 0x01;
	grab_model[0].product_code = 0x01;
	grab_model[0].model_code = 0x1101;
	grab_model[0].s_text = "APX-3324A";

	grab_model[1].interface_code = 0x01;
	grab_model[1].vender_code = 0x01;
	grab_model[1].product_code = 0x02;
	grab_model[1].model_code = 0x1102;
	grab_model[1].s_text = "APX-3323";

	grab_model[2].interface_code = 0x01;
	grab_model[2].vender_code = 0x02;
	grab_model[2].product_code = 0x01;
	grab_model[2].model_code = 0x1201;
	grab_model[2].s_text = "FVC08CLB";
}


CDefinedElementSys::~CDefinedElementSys()
{
}
