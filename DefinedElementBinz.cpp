/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DefinedElementBinz.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：2値化パラメータ 各要素のコードと名称の紐づけデータベース
 **/

#include "stdafx.h"
#include "DefinedElementBinz.h"


CDefinedElementBinz::CDefinedElementBinz()
{
	/** ****************************************************** **
	 ** 基本種別
	 **		2値化機能のタイプ
	 **		種別コード(2桁) base_code
	 **			0x01:固定閾値
	 **			0x02:動的閾値
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	base[0].base_code = 0x01;
	base[0].s_text = "固定閾値";
	base[0].e_text = "Fixed";

	base[1].base_code = 0x02;
	base[1].s_text = "動的閾値";
	base[1].e_text = "Dynamic";


	/** ****************************************************** **
	 ** 2値化手法
	 **		固定/動的閾値2値化手法
	 **		種別コード(2桁) base_code
	 **			0x01:固定閾値
	 **			0x02:動的閾値
	 **		手法コード(4桁) type
	 **			type = (base_code:2桁)(連番:2桁)
	 **			0x0101:単一閾値指定
	 **			0x0102:バンド閾値指定
	 **			0x0201:閾値判別分析法
	 **			0x0202:微分ヒストグラム法
	 **			0x0203:ラプラシアンヒストグラム法
	 **			0x0204:Niblack法
	 **			0x0205:Pタイル法
	 **			0x0206:Sauvola法
	 **			0x0207:局所領域平均値
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	binz_method[0].base_code = 0x01;
	binz_method[0].type = 0x0101;
	binz_method[0].s_text = "単一閾値指定";
	binz_method[0].e_text = "Single";

	binz_method[1].base_code = 0x01;
	binz_method[1].type = 0x0102;
	binz_method[1].s_text = "バンド閾値指定";
	binz_method[1].e_text = "BandThreshold";

	binz_method[2].base_code = 0x02;
	binz_method[2].type = 0x0201;
	binz_method[2].s_text = "閾値判別分析法";
	binz_method[2].e_text = "Discrimination";

	binz_method[3].base_code = 0x02;
	binz_method[3].type = 0x0202;
	binz_method[3].s_text = "微分ヒストグラム法";
	binz_method[3].e_text = "Gradient";

	binz_method[4].base_code = 0x02;
	binz_method[4].type = 0x0203;
	binz_method[4].s_text = "ラプラシアンヒストグラム法";
	binz_method[4].e_text = "Laplacian";

	binz_method[5].base_code = 0x02;
	binz_method[5].type = 0x0204;
	binz_method[5].s_text = "Niblack法";
	binz_method[5].e_text = "Niblack";

	binz_method[6].base_code = 0x02;
	binz_method[6].type = 0x0205;
	binz_method[6].s_text = "Pタイル法";
	binz_method[6].e_text = "PTile";

	binz_method[7].base_code = 0x02;
	binz_method[7].type = 0x0206;
	binz_method[7].s_text = "Sauvola法";
	binz_method[7].e_text = "Sauvola";

	binz_method[8].base_code = 0x02;
	binz_method[8].type = 0x0207;
	binz_method[8].s_text = "局所領域平均値";
	binz_method[8].e_text = "ShiftAverage";
}


CDefinedElementBinz::~CDefinedElementBinz()
{
}
