/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DefinedElementCmd.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2022.05.19	M.Iwasaki	結果出力機能追加
 **			2026.03.26	M.Iwasaki	10.19.1.2	FTP機能追加
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：コマンドパラメータ 各要素のコードと名称の紐づけデータベース
 **/

#include "stdafx.h"
#include "DefinedElementCmd.h"


CDefinedElementCmd::CDefinedElementCmd()
{

	/** ****************************************************** **
	 ** 機能種別
	 **		基本的な機能種別
	 **		機能種別コード code
	 **			0x01:アライメント
	 **			0x02:計測モード
	 **			0x03:キャリブレーション
	 **			0x04:バーコードリーダー
	 **			0x12:機種切替
	 **			0x13:カメラ切替
	 **			0x16:ファイルアクセス
	 **			0x17:マニュアルサーチ有効
	 **			0x41:外部オフセット設定
	 **			0x51:文字列設定
	 **			0x52:FTPFileDownload
	 **			0x53:FTPFileRemove
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	base[0].code = 0x01;
	base[0].s_text = "アライメント";

	base[1].code = 0x02;
	base[1].s_text = "計測モード";

	base[2].code = 0x03;
	base[2].s_text = "キャリブレーション";

	base[3].code = 0x12;
	base[3].s_text = "機種切替";

	base[4].code = 0x13;
	base[4].s_text = "カメラ切替";

	base[5].code = 0x16;
	base[5].s_text = "ファイルアクセス";

	base[6].code = 0x17;
	base[6].s_text = "マニュアルサーチ有効";

	base[7].code = 0x41;
	base[7].s_text = "外部オフセット設定";

	base[8].code = 0x04;
	base[8].s_text = "バーコードリーダー";

	base[9].code = 0x51;
	base[9].s_text = "オリジナル記号文字列";

	base[10].code = 0x52;
	base[10].s_text = "FTPFileDownload";

	base[11].code = 0x53;
	base[11].s_text = "FTPFileRemove";


	/** ****************************************************** **
	 ** 探索機能
	 **		探索機能に紐づく演算機能
	 **		アライメントの他、計測モードやキャリブレーションなど
	 **		機能種別コード(2桁) base_method
	 **			0x01:アライメント
	 **			0x02:計測モード
	 **			0x03:キャリブレーション
	 **			0x04:バーコードリーダー
	 **		機能コード(4桁) method
	 **			method = (base_method:2桁)(連番:2桁)
	 **			0x0101:中点一致
	 **			0x0102:端点一致 視野1
	 **			0x0103:端点一致 視野2
	 **			0x0104:端点一致 視野3
	 **			0x0105:端点一致 視野4
	 **			0x0201:計測モード
	 **			0x0306:画素CALポジション1
	 **			0x0307:画素CALポジション2
	 **			0x0308:回転中心CALポジション1
	 **			0x0309:回転中心CALポジション2
	 **			0x0310:経時変化基準指定
	 **			0x0311:経時変化補正算出
	 **			0x0401:バーコードリーダー
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	search[0].base_method = 0x01;
	search[0].method = 0x0101;
	search[0].s_text = "中点一致";

	search[1].base_method = 0x01;
	search[1].method = 0x0102;
	search[1].s_text = "端点一致 視野1";

	search[2].base_method = 0x01;
	search[2].method = 0x0103;
	search[2].s_text = "端点一致 視野2";

	search[3].base_method = 0x02;
	search[3].method = 0x0201;
	search[3].s_text = "計測モード";

	search[4].base_method = 0x03;
	search[4].method = 0x0306;
	search[4].s_text = "画素CALポジション1";

	search[5].base_method = 0x03;
	search[5].method = 0x0307;
	search[5].s_text = "画素CALポジション2";

	search[6].base_method = 0x03;
	search[6].method = 0x0308;
	search[6].s_text = "回転中心CALポジション1";

	search[7].base_method = 0x03;
	search[7].method = 0x0309;
	search[7].s_text = "回転中心CALポジション2";

	search[8].base_method = 0x04;
	search[8].method = 0x0401;
	search[8].s_text = "バーコードリーダー";

	search[9].base_method = 0x01;
	search[9].method = 0x0104;
	search[9].s_text = "端点一致 視野3";

	search[10].base_method = 0x01;
	search[10].method = 0x0105;
	search[10].s_text = "端点一致 視野4";

	search[11].base_method = 0x03;
	search[11].method = 0x0310;
	search[11].s_text = "経時変化基準指定";

	search[12].base_method = 0x03;
	search[12].method = 0x0311;
	search[12].s_text = "経時変化補正算出";

	search[13].base_method = 0x03;
	search[13].method = 0x0312;
	search[13].s_text = "経時変化補正リセット";
}


CDefinedElementCmd::~CDefinedElementCmd()
{
}
