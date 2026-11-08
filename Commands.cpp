/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：Commands.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.09.12	M.Iwasaki	キャリブレーションデータコピー改善
 **			2024.07.16	M.Iwasaki	10.18.4.6	TCP/IPサーバ機能追加
 **			2026.03.26	M.Iwasaki	10.19.1.2	FTP機能追加
 ** 注記：
 ** 説明：コマンドパラメータ
 **		コマンドのパラメータクラス
 **		入力テンプレート、実行内容で構成される
 **		データ読込：PLCからシリアルデータを読み込む
 **		画像取込：カメラから画像を取り込む
 **		演算処理：中点一致などの指定された機能を実行する
 **		データ書込：PLCへシリアルデータを書き込む
 **		他、マニュアルサーチ有効、ファイルアクセス、カメラ切替、機種切替などがある
 **/

#include "stdafx.h"
#include "Commands.h"


CCommands::CCommands()
{
	Init();		//パラメータ初期化
}


CCommands::~CCommands()
{
}


/** ****************************************************** **
 ** パラメータ初期化
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCommands::Init()
{
	// TODO: ここに実装コードを追加します.

	//機能種別
	base_method = 0x00;		//0x00:未定義

	//テンプレートパラメータ
	template_number = -1;	//-1:未定義

	//データ読込
	read.enable = FALSE;		//無し設定
	read.type = 0x00;			//0x00:未定義
	read.calpos_enable = FALSE;	//無し設定

	//画像取込
	grab.enable = FALSE;	//無し設定

	//探索と演算処理
	search.search_enable = FALSE;	//探索無し設定
	search.calc_enable = FALSE;		//演算無し設定
	search.method = 0x0000;			//0x0000:未定義

	//データ書込
	write.enable = FALSE;	//無し設定
	write.type = 0x00;		//0x00:データ無し(ゼロリセット)

	//連続実行コマンド
	chain.ok_chain = -1;	//-1:実行なし
	chain.ng_chain = -1;	//-1:実行なし

	//アライメントパラメータ
	align.parts = 0;			//0:未定義
	align.base_scope = -1;		//-1:未定義
	align.target_scope = -1;	//-1:未定義

	//キャリブレーションパラメータ
	calib.copy_view = -1;	//-1:なし
	calib.copy_scope = -1;	//-1:なし
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			calib.custom_copy_scope[i].view[j] = FALSE;
		}
	}

	//オプション機能
	option.after_cam_number = -1;	//-1:無し

	//FTP機能基本パラメータ
	ftp_login.login_user = "user";
	ftp_login.login_pw = "password";
	ftp_login.ip_address = "0.0.0.0";

	//FTPファイルダウントーロパラメータ
	ftp_dlgot.sv_directory = ".\\";
	ftp_dlgot.download_path = "C:\\";
	ftp_dlgot.file_name = "*.CSV";
}
