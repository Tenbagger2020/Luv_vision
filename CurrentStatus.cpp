/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CurrentStatus.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：現状共通データ
 **		共有する現状データクラス
 **/

#include "stdafx.h"
#include "CurrentStatus.h"


CCurrentStatus::CCurrentStatus()
{
	work_number = 0;		//現在機種No

	//デジタルIOボード
	dio_initial_standby = FALSE;	//イニシャル処理完了フラグ
	dio_ready = FALSE;				//正常フラグ

	//画像入力ボード
	grab_initial_standby = FALSE;	//イニシャル処理完了フラグ
	grab_ready = FALSE;				//正常フラグ

	//メイン画面表示
	main_view_cam_num = 1;		//メイン画面表示カメラ番号(1～8)
	main_view_cam_mem = 0;		//メイン画面表示カメラ記憶(0:リセット、1～8:記憶番号)
	run_main_view_cmd = FALSE;	//メイン画面表示 コマンド実行中
	baseline_type = 2;			//基準線描画タイプ 0:なし、1:画像中心、2:キャリブレーション中心

	//カメラ情報
	for (int i = 0; i < 8; i++) {
		//最終実行コマンド
		cam[i].last_cmd.cmd = 0x00;
		cam[i].last_cmd.template_number = 0;
		cam[i].last_cmd.scope = 0;
		cam[i].last_cmd.view = 0;
		cam[i].last_cmd.search_method = 0x0000;
	}

	//終了処理
	pc_shutdown = FALSE;
}


CCurrentStatus::~CCurrentStatus()
{
}
