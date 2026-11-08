/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcAlign.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.06.20	M.Iwasaki	4視野アライメント演算処理の作成
 **			2022.09.12	M.Iwasaki	デバックモードログ出力追加
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：アライメント演算プログラム
 **
 **/

#include "stdafx.h"
#include "CalcAlign.h"

#include "DlgSystemMain.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Commands.h"			//コマンドパラメータ
#include "TempParam.h"			//機種毎テンプレートパラメータ
#include "SysOption.h"			//システムオプションパラメータ

#include "CalcMark.h"			//サーチ結果取得
#include "CalcConv.h"			//座標変換
#include "CalcPLC.h"			//PLCデータ取得

#include "Statistics.h"			//統計データ
#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ
extern CSysOption		sys_option;				//システムオプションパラメータ

extern CCalcMark	calc_mark;		//サーチ結果取得
extern CCalcConv	calc_conv;		//座標変換
extern CCalcPLC		calc_plc;		//PLCデータ取得

extern CStatistics	data_statis;	//統計データ
extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

CCalcAlign::CCalcAlign()
{
}


CCalcAlign::~CCalcAlign()
{
}


/** ****************************************************** **
 ** アライメントメイン
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::CalcMain(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	int rtn_result = 1;

	byte cmd = dlg->run_cmd;	//実行コマンド
	byte base_method = cmd_param[cmd].base_method;	//機能種別

	//対象視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo

	//対象スコープ
	int base_scope = cmd_param[cmd].align.base_scope;

	//目標スコープ
	int target_scope = cmd_param[cmd].align.target_scope;

	//中点一致実施
	if (cmd_param[cmd].search.method == 0x0101) {
		//アライメントのデータセット(対象スコープ)
		if (CalcDataSet(cmd, base_scope) != 0) {
			dlg->thread_rs.calc_err = 0x9411;	//エラーコード
			goto END;
		}
		//アライメントのデータセット(目標スコープ)
		//キャリブレーション基準(16,17)の場合は行わない
		if (target_scope != 16 && target_scope != 17) {
			if (CalcDataSet(cmd, target_scope) != 0) {
				dlg->thread_rs.calc_err = 0x9411;	//エラーコード
				goto END;
			}
		}
		//中点一致の演算実行
		if (MidPointCalcExec(cmd, base_method) != 0) {
			dlg->thread_rs.calc_err = 0x9412;	//エラーコード
			goto END;
		}
	}
	//端点一致実施
	else if (cmd_param[cmd].search.method == 0x0102 || cmd_param[cmd].search.method == 0x0103
		|| cmd_param[cmd].search.method == 0x0104 || cmd_param[cmd].search.method == 0x0105) {
		//アライメントのデータセット(対象スコープ)
		if (CalcDataSet(cmd, base_scope) != 0) {
			dlg->thread_rs.calc_err = 0x9411;	//エラーコード
			goto END;
		}
		//アライメントのデータセット(目標スコープ)
		//キャリブレーション基準(16,17)の場合は行わない
		if (target_scope != 16 && target_scope != 17) {
			if (CalcDataSet(cmd, target_scope) != 0) {
				dlg->thread_rs.calc_err = 0x9411;	//エラーコード
				goto END;
			}
		}
		//端点一致の演算実行
		if (EndPointCalcExec(cmd, base_method) != 0) {
			dlg->thread_rs.calc_err = 0x9413;	//エラーコード
			goto END;
		}
	}
	//異常系(想定外コマンド)
	else {
		dlg->thread_rs.calc_err = 0x9419;	//エラーコード
		goto END;
	}

	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "アライメント演算完了", "Alignment calculation completed"));	//コマンドログ 演算完了

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** アライメントのデータセット
 ** 引数：
 **　byte cmd	実行コマンド
 **　int scope	読み込むスコープ(0～15)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::CalcDataSet(byte cmd, int scope)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	int parts = cmd_param[cmd].align.parts;	//アライメント視野数
	if (parts < 1 || parts > 4) goto END;

	if (scope < 0 || scope > 15) goto END;	//スコープは0～15

	for (int i = 0; i < parts; i++) {
		//視野数1の場合は対象視野のデータを使用する
		if (parts == 1) {
			WORD search_method = cmd_param[cmd].search.method;		//探索機能
			//端点一致 視野1
			if (search_method == 0x0102) {
				i = 0;	//視野1を使用
			}
			else if (search_method == 0x0103) {
				i = 1;	//視野2を使用
			}
			else if (search_method == 0x0104) {
				i = 2;	//視野3を使用
			}
			else if (search_method == 0x0105) {
				i = 3;	//視野4を使用
			}
			else {
				return 1;
			}
		}

		//対象スコープのテンプレート番号を取得
		int template_number = scope_param[scope].view[i].align.template_number;
		CTempParam* temp_param = &template_param[template_number];

		//データ変換
		if (CalcDataConv(template_number, &scope_param[scope].view[i]) != 0) goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** データ変換
 ** 引数：
 **　int template_number	テンプレート番号
 **　CScopeParam::ViewParameter* view	視野パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::CalcDataConv(int template_number, CScopeParam::ViewParameter* view)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//対象スコープのテンプレート番号を取得
	CTempParam* temp_param = &template_param[template_number];

	//検出位置
	double mark_x;	//検出位置X
	double mark_y;	//検出位置Y
	//サーチ成功
	if (!temp_param->manu.do_point) {
		byte add_type = temp_param->additional.add_type;
		//基準点サーチ
		if (add_type == 0x00) {
			//グレイサーチの場合
			if (temp_param->basic.matching == 0x00) {
				if (calc_mark.GetMarkGS2(temp_param, &mark_x, &mark_y) != 0) goto END;
			}
			//形状サーチの場合
			else if (temp_param->basic.matching == 0x01) {
				if (calc_mark.GetMarkFPM2(temp_param, &mark_x, &mark_y) != 0) goto END;
			}
			else {
				goto END;
			}
		}
		//エッジサーチ
		else if (add_type == 0x01) {
			mark_x = temp_param->add_edge.rslt_cross.x;
			mark_y = temp_param->add_edge.rslt_cross.y;
		}
		//2値化重心サーチ
		else if (add_type == 0x02) {
			mark_x = temp_param->add_blob.rslt_center.x;
			mark_y = temp_param->add_blob.rslt_center.y;
		}
		else {
			goto END;
		}
	}
	//マニュアルサーチ実施
	else {
		mark_x = temp_param->manu.manual_srch.x;
		mark_y = temp_param->manu.manual_srch.y;
	}

	//経時変化補正実施
	mark_x = mark_x - view->move.displacement_x;
	mark_y = mark_y - view->move.displacement_y;

	//スコープデータへセット
	view->align.mark_x = mark_x;
	view->align.mark_y = mark_y;

	//ポジション座標変換
	double crdnt_x;
	double crdnt_y;
	double cal_base_x = view->calib.pixel.cal_base_x;
	double cal_base_y = view->calib.pixel.cal_base_y;
	int coordinate_type = view->coordinate_type;
	if (calc_conv.ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x, mark_y, &crdnt_x, &crdnt_y) != 0) goto END;
	view->align.crdnt_x = crdnt_x;
	view->align.crdnt_y = crdnt_y;

	//軸位置
	double axis_x;		//軸位置X
	double axis_y;		//軸位置Y
	byte read_type = temp_param->plc.read_type;
	switch (read_type) {
		//視野位置(X,Y)
	case 0x01:
		if (calc_plc.GetAxisPosXY(template_number, &axis_x, &axis_y) != 0) goto END;
		break;
		//外部オフセット
	case 0x03:
		axis_x = view->calib.pixel.axis_base_x;
		axis_y = view->calib.pixel.axis_base_y;
		break;
		//キャリブレーション基準位置を使用する
	case 0xFF:
		axis_x = view->calib.pixel.axis_base_x;
		axis_y = view->calib.pixel.axis_base_y;
		break;
	default:
		goto END;
	}
	view->align.axis_x = axis_x;
	view->align.axis_y = axis_y;

	//メカ座標変換
	double mech_x;
	double mech_y;
	double pixel_size;
	double crdnt_radian;
	pixel_size = view->calib.pixel_size;
	crdnt_radian = view->calib.crdnt_radian;
	if (calc_conv.ConvMechCoord(crdnt_x, crdnt_y, axis_x, axis_y, pixel_size, crdnt_radian, &mech_x, &mech_y) != 0) goto END;
	view->align.mech_x = mech_x;
	view->align.mech_y = mech_y;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 中点一致の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		・対象視野は視野1～4
 **		・視野数は1～4
 ** ****************************************************** **/
int CCalcAlign::MidPointCalcExec(byte cmd, byte base_method)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	int template_number = cmd_param[cmd].template_number;	//テンプレートNo

	//対象視野の確認
	//アライメント視野数1の場合に使用する
	int view = sys_template[template_number].view;			//対象視野
	if (view < 0 || view > 4) goto END;

	//対象スコープの確認
	int base_scope = cmd_param[cmd].align.base_scope;
	if (base_scope < 0 || base_scope > 15) goto END;
	CScopeParam* base_param = &scope_param[base_scope];

	//目標スコープの確認
	int target_scope = cmd_param[cmd].align.target_scope;
	if (target_scope < 0 || target_scope > 17) goto END;
	CScopeParam* target_param = NULL;
	if (target_scope >= 0 && target_scope <= 15) target_param = &scope_param[target_scope];
	if (base_scope == target_scope) goto END;

	//アライメント視野数の確認
	int parts = cmd_param[cmd].align.parts;		//アライメント視野数
	if (parts < 1 || parts > 4) goto END;

	//キャリブレーション完了の確認はDispatchでやっているのでここでは省略

	//データ読み込み
	byte read_type = cmd_param[cmd].read.type;

	//探索機能
	WORD method = cmd_param[cmd].search.method;

	//目標視野座標
	double target_mech_x[4] = {};	//0で初期化
	double target_mech_y[4] = {};

	//対象視野座標
	double base_mech_x[4] = {};		//0で初期化
	double base_mech_y[4] = {};

	//目標スコープに合わせ込む
	if (target_scope >= 0 && target_scope <= 15) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			target_mech_x[0] = target_param->view[0].align.mech_x;
			target_mech_y[0] = target_param->view[0].align.mech_y;
			target_mech_x[1] = target_param->view[1].align.mech_x;
			target_mech_y[1] = target_param->view[1].align.mech_y;
			target_mech_x[2] = target_param->view[2].align.mech_x;
			target_mech_y[2] = target_param->view[2].align.mech_y;
			target_mech_x[3] = target_param->view[3].align.mech_x;
			target_mech_y[3] = target_param->view[3].align.mech_y;
			break;
			//視野数3
		case 3:
			target_mech_x[0] = target_param->view[0].align.mech_x;
			target_mech_y[0] = target_param->view[0].align.mech_y;
			target_mech_x[1] = target_param->view[1].align.mech_x;
			target_mech_y[1] = target_param->view[1].align.mech_y;
			target_mech_x[2] = target_param->view[2].align.mech_x;
			target_mech_y[2] = target_param->view[2].align.mech_y;
			break;
			//視野数2,視野数1
		case 2:
		case 1:
			target_mech_x[0] = target_param->view[0].align.mech_x;
			target_mech_y[0] = target_param->view[0].align.mech_y;
			target_mech_x[1] = target_param->view[1].align.mech_x;
			target_mech_y[1] = target_param->view[1].align.mech_y;
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			base_mech_x[3] = base_param->view[3].align.mech_x;
			base_mech_y[3] = base_param->view[3].align.mech_y;
			break;
			//視野数3
		case 3:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			break;
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[view].align.mech_x;
			base_mech_y[0] = base_param->view[view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			break;
		}
	}

	//キャリブレーション基準(移動)に合わせ込む
	if (target_scope == 16) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			target_mech_x[0] = base_param->view[0].align.axis_x;
			target_mech_y[0] = base_param->view[0].align.axis_y;
			target_mech_x[1] = base_param->view[1].align.axis_x;
			target_mech_y[1] = base_param->view[1].align.axis_y;
			target_mech_x[2] = base_param->view[2].align.axis_x;
			target_mech_y[2] = base_param->view[2].align.axis_y;
			target_mech_x[3] = base_param->view[3].align.axis_x;
			target_mech_y[3] = base_param->view[3].align.axis_y;

			//目標のメカ座標がすべて同じ場合は、補正量の計算でエラーになる
			//よって目標のメカ座標を若干ズラす必要がある
			//補正量の計算には問題無いっぽい
			if (((target_mech_x[0] == target_mech_x[1]) && (target_mech_x[2] == target_mech_x[3]) && (target_mech_x[1] == target_mech_x[2])) &&
				((target_mech_y[0] == target_mech_y[1]) && (target_mech_y[2] == target_mech_y[3]) && (target_mech_y[1] == target_mech_y[2]))) {

				//この条件の意図がわからない...
				//　これは使用しない
				//if (base_param->view[0].calib.pixel.axis_y[0] == base_param->view[1].calib.pixel.axis_y[1]) {
				//	double data_x = base_param->view[0].calib.pixel.axis_x[0];
				//	target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x - fabs(data_x);
				//	target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y - fabs(data_x);
				//	target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x + fabs(data_x);
				//	target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y - fabs(data_x);
				//	target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x - fabs(data_x);
				//	target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y + fabs(data_x);
				//	target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x + fabs(data_x);
				//	target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y + fabs(data_x);
				//}
				//else {
				//	double data_y = base_param->view[0].calib.pixel.axis_y[0];
				//	target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x - fabs(data_y);
				//	target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y - fabs(data_y);
				//	target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x + fabs(data_y);
				//	target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y - fabs(data_y);
				//	target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x - fabs(data_y);
				//	target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y + fabs(data_y);
				//	target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x + fabs(data_y);
				//	target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y + fabs(data_y);
				//}

				//とりあえず0.1mmを入れる
				target_mech_x[0] = base_param->view[0].align.axis_x - 0.1;
				target_mech_y[0] = base_param->view[0].align.axis_y - 0.1;
				target_mech_x[1] = base_param->view[1].align.axis_x + 0.1;
				target_mech_y[1] = base_param->view[1].align.axis_y - 0.1;
				target_mech_x[2] = base_param->view[2].align.axis_x + 0.1;
				target_mech_y[2] = base_param->view[2].align.axis_y + 0.1;
				target_mech_x[3] = base_param->view[3].align.axis_x - 0.1;
				target_mech_y[3] = base_param->view[3].align.axis_y + 0.1;
			}
			break;
			//視野数3
		case 3:
			target_mech_x[0] = base_param->view[0].align.axis_x;
			target_mech_y[0] = base_param->view[0].align.axis_y;
			target_mech_x[1] = base_param->view[1].align.axis_x;
			target_mech_y[1] = base_param->view[1].align.axis_y;
			target_mech_x[2] = base_param->view[2].align.axis_x;
			target_mech_y[2] = base_param->view[2].align.axis_y;
			break;
			//視野数2
		case 2:
			target_mech_x[0] = base_param->view[0].align.axis_x;
			target_mech_y[0] = base_param->view[0].align.axis_y;
			target_mech_x[1] = base_param->view[1].align.axis_x;
			target_mech_y[1] = base_param->view[1].align.axis_y;
			break;
			//視野数1
		case 1:
			target_mech_x[0] = base_param->view[0].align.axis_x;
			target_mech_y[0] = base_param->view[0].align.axis_y;
			target_mech_x[1] = base_param->view[1].align.axis_x;
			target_mech_y[1] = base_param->view[1].align.axis_y;
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			base_mech_x[3] = base_param->view[3].align.mech_x;
			base_mech_y[3] = base_param->view[3].align.mech_y;
			break;
			//視野数3
		case 3:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			break;
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[view].align.mech_x;
			base_mech_y[0] = base_param->view[view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			break;
		}
	}

	//キャリブレーション基準(固定)に合わせ込む
	if (target_scope == 17) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
			target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
			target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
			target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;
			target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x;
			target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y;
			target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x;
			target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y;


			//目標のメカ座標がすべて同じ場合は、補正量の計算でエラーになる
			//よって目標のメカ座標を若干ズラす必要がある
			//補正量の計算には問題無いっぽい
			if (((target_mech_x[0] == target_mech_x[1]) && (target_mech_x[2] == target_mech_x[3]) && (target_mech_x[1] == target_mech_x[2])) &&
				((target_mech_y[0] == target_mech_y[1]) && (target_mech_y[2] == target_mech_y[3]) && (target_mech_y[1] == target_mech_y[2]))) {

				//この条件の意図がわからない...
				//　これは使用しない
				//if (base_param->view[0].calib.pixel.axis_y[0] == base_param->view[1].calib.pixel.axis_y[1]) {
				//	double data_x = base_param->view[0].calib.pixel.axis_x[0];
				//	target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x - fabs(data_x);
				//	target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y - fabs(data_x);
				//	target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x + fabs(data_x);
				//	target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y - fabs(data_x);
				//	target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x - fabs(data_x);
				//	target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y + fabs(data_x);
				//	target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x + fabs(data_x);
				//	target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y + fabs(data_x);
				//}
				//else {
				//	double data_y = base_param->view[0].calib.pixel.axis_y[0];
				//	target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x - fabs(data_y);
				//	target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y - fabs(data_y);
				//	target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x + fabs(data_y);
				//	target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y - fabs(data_y);
				//	target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x - fabs(data_y);
				//	target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y + fabs(data_y);
				//	target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x + fabs(data_y);
				//	target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y + fabs(data_y);
				//}

				//とりあえず0.1mmを入れる
				target_mech_x[0] = base_param->view[0].align.axis_x - 0.1;
				target_mech_y[0] = base_param->view[0].align.axis_y - 0.1;
				target_mech_x[1] = base_param->view[1].align.axis_x + 0.1;
				target_mech_y[1] = base_param->view[1].align.axis_y - 0.1;
				target_mech_x[2] = base_param->view[2].align.axis_x + 0.1;
				target_mech_y[2] = base_param->view[2].align.axis_y + 0.1;
				target_mech_x[3] = base_param->view[3].align.axis_x - 0.1;
				target_mech_y[3] = base_param->view[3].align.axis_y + 0.1;
			}
			break;
			//視野数3
		case 3:
			target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
			target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
			target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
			target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;
			target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x;
			target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y;
			break;
			//視野数2
		case 2:
			target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
			target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
			target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
			target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;

			//キャリ基準位置が同じ場合　※この処理の意図がわからない..
			if ((target_mech_x[0] == target_mech_x[1]) && (target_mech_y[0] == target_mech_y[1])) {
				//中点一致の場合
				target_mech_x[0] = base_param->view[0].calib.pixel.axis_x[0];
				target_mech_y[0] = base_param->view[0].calib.pixel.axis_y[0];
				target_mech_x[1] = base_param->view[1].calib.pixel.axis_x[1];
				target_mech_y[1] = base_param->view[1].calib.pixel.axis_y[1];
			}
			break;
			//視野数1
		case 1:
			target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
			target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
			target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
			target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数4
		case 4:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			base_mech_x[3] = base_param->view[3].align.mech_x;
			base_mech_y[3] = base_param->view[3].align.mech_y;
			break;
			//視野数3
		case 3:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			base_mech_x[2] = base_param->view[2].align.mech_x;
			base_mech_y[2] = base_param->view[2].align.mech_y;
			break;
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[view].align.mech_x;
			base_mech_y[0] = base_param->view[view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			break;
		}
	}

	//
	//補正演算実行
	//
	double base_rotation_center_x = base_param->conf_rotcent_x;
	double base_rotation_center_y = base_param->conf_rotcent_y;
	BOOL theta_complete = base_param->calib_theta_complete;
	double offset_x;	//外部オフセットX
	double offset_y;	//外部オフセットY
	double offset_t;	//外部オフセットT(radian)
	if (target_scope >= 0 && target_scope <= 15) {
		offset_x = target_param->ext_offset_x;
		offset_y = target_param->ext_offset_y;
		offset_t = Tips0.DegToRadian(target_param->ext_offset_t);
	}
	else {
		offset_x = base_param->ext_offset_x;
		offset_y = base_param->ext_offset_y;
		offset_t = Tips0.DegToRadian(base_param->ext_offset_t);
	}
	double theta_x;			//補正結果 θX
	double theta_y;			//補正結果 θY
	double delta_t_rad;		//補正結果 ⊿T(radian)
	double delta_x;			//補正結果 ⊿X
	double delta_y;			//補正結果 ⊿Y
	int result_align = 1;
	if (parts <= 2) {
		//2視野補正演算実行
		result_align = TwoPartsAlignment(parts,
										method,
										target_mech_x, target_mech_y,
										base_mech_x, base_mech_y,
										base_rotation_center_x, base_rotation_center_y, theta_complete,
										offset_x, offset_y, offset_t,
										&theta_x, &theta_y, &delta_t_rad, &delta_x, &delta_y);
	}
	else {
		//4視野補正演算実行
		if (sys_option.fourparts_align == 1) {
			//旧タイプ演算処理
			result_align = FourPartsAlignment(parts,
											method,
											target_mech_x, target_mech_y,
											base_mech_x, base_mech_y,
											base_rotation_center_x, base_rotation_center_y, theta_complete,
											offset_x, offset_y, offset_t,
											&theta_x, &theta_y, &delta_t_rad, &delta_x, &delta_y);
		}
		else {
			//新タイプ演算処理
			result_align = FourPartsAlignmentRev2(parts,
												method,
												target_mech_x, target_mech_y,
												base_mech_x, base_mech_y,
												base_rotation_center_x, base_rotation_center_y, theta_complete,
												offset_x, offset_y, offset_t,
												&theta_x, &theta_y, &delta_t_rad, &delta_x, &delta_y);
		}
	}
	if (result_align != 0) goto END;

	//
	//マーク間距離
	//
	double base_mark_width_array[4] = {};	//対象スコープマーク間距離 0で初期化
	double target_mark_width_array[4] = {};	//目標スコープマーク間距離 0で初期化
	CalcMarkWidth(base_mech_x, base_mech_y, target_mech_x, target_mech_y, base_mark_width_array, target_mark_width_array);

	//統計データセット
	if (SetStatistics(base_scope, theta_x, theta_y, delta_t_rad, delta_x, delta_y) != 0) goto END;

	//
	//中点一致の演算結果のデータセット
	//
	int scope = sys_template[template_number].scope;
	SetResult(cmd, scope, base_method, theta_x, theta_y, delta_t_rad, delta_x, delta_y, base_mark_width_array, target_mark_width_array);

	//コマンドログ出力
	CmdLogOut(cmd, theta_x, theta_y, delta_t_rad, delta_x, delta_y, base_mark_width_array, target_mark_width_array);

	//デバックモードログ出力
	DebugLogOut(target_mech_x, target_mech_y, base_mech_x, base_mech_y, parts);

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 端点一致の演算実行
 ** 引数：
 **　byte cmd			実行コマンド
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		・対象視野は視野1,視野2のみ
 **		・視野数は1,2のみ
 ** ****************************************************** **/
int CCalcAlign::EndPointCalcExec(byte cmd, byte base_method)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	int template_number = cmd_param[cmd].template_number;	//テンプレートNo

	//対象視野の確認
	//アライメント視野数1の場合に使用する
	//テンプレートの視野と機能"端点一致 視野*"は一致する
	int one_view = sys_template[template_number].view;		//対象視野
	if (one_view < 0 || one_view > 3) goto END;

	//対象スコープの確認
	int base_scope = cmd_param[cmd].align.base_scope;
	if (base_scope < 0 || base_scope > 15) goto END;
	CScopeParam* base_param = &scope_param[base_scope];

	//目標スコープの確認
	int target_scope = cmd_param[cmd].align.target_scope;	//目標スコープ
	if (target_scope < 0 || target_scope > 17) goto END;
	CScopeParam* target_param = NULL;
	if (target_scope >= 0 && target_scope <= 15) target_param = &scope_param[target_scope];
	if (base_scope == target_scope) goto END;

	//アライメント視野数の確認
	int parts = cmd_param[cmd].align.parts;		//アライメント視野数
	if (parts < 1 || parts > 2) goto END;

	//キャリブレーション完了の確認はDispatchでやっているのでここでは省略

	//データ読み込み
	byte read_type = cmd_param[cmd].read.type;

	//探索機能
	WORD method = cmd_param[cmd].search.method;

	//目標視野座標
	double target_mech_x[4] = {};	//0で初期化
	double target_mech_y[4] = {};

	//対象視野座標
	double base_mech_x[4] = {};		//0で初期化
	double base_mech_y[4] = {};

	//目標スコープに合わせ込む
	if (target_scope >= 0 && target_scope <= 15) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			target_mech_x[0] = target_param->view[0].align.mech_x;
			target_mech_y[0] = target_param->view[0].align.mech_y;
			target_mech_x[1] = target_param->view[1].align.mech_x;
			target_mech_y[1] = target_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			//端点一致 視野1の場合
			if (method == 0x0102) {
				target_mech_x[0] = target_param->view[0].align.mech_x;
				target_mech_y[0] = target_param->view[0].align.mech_y;
			}
			//端点一致 視野2の場合
			if (method == 0x0103) {
				target_mech_x[1] = target_param->view[1].align.mech_x;
				target_mech_y[1] = target_param->view[1].align.mech_y;
			}
			//端点一致 視野3の場合
			if (method == 0x0104) {
				target_mech_x[2] = target_param->view[2].align.mech_x;
				target_mech_y[2] = target_param->view[2].align.mech_y;
			}
			//端点一致 視野4の場合
			if (method == 0x0105) {
				target_mech_x[3] = target_param->view[3].align.mech_x;
				target_mech_y[3] = target_param->view[3].align.mech_y;
			}
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[one_view].align.mech_x;
			base_mech_y[0] = base_param->view[one_view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			base_mech_x[2] = base_mech_x[0];
			base_mech_y[2] = base_mech_y[0];
			base_mech_x[3] = base_mech_x[0];
			base_mech_y[3] = base_mech_y[0];
			break;
		}
	}

	//キャリブレーション基準(移動)に合わせ込む
	if (target_scope == 16) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			target_mech_x[0] = base_param->view[0].align.axis_x;
			target_mech_y[0] = base_param->view[0].align.axis_y;
			target_mech_x[1] = base_param->view[1].align.axis_x;
			target_mech_y[1] = base_param->view[1].align.axis_y;
			break;
			//視野数1
		case 1:
			//端点一致 視野1の場合
			if (method == 0x0102) {
				target_mech_x[0] = base_param->view[0].align.axis_x;
				target_mech_y[0] = base_param->view[0].align.axis_y;
			}
			//端点一致 視野2の場合
			if (method == 0x0103) {
				target_mech_x[1] = base_param->view[1].align.axis_x;
				target_mech_y[1] = base_param->view[1].align.axis_y;
			}
			//端点一致 視野3の場合
			if (method == 0x0104) {
				target_mech_x[2] = base_param->view[2].align.axis_x;
				target_mech_y[2] = base_param->view[2].align.axis_y;
			}
			//端点一致 視野4の場合
			if (method == 0x0105) {
				target_mech_x[3] = base_param->view[3].align.axis_x;
				target_mech_y[3] = base_param->view[3].align.axis_y;
			}
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[one_view].align.mech_x;
			base_mech_y[0] = base_param->view[one_view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			base_mech_x[2] = base_mech_x[0];
			base_mech_y[2] = base_mech_y[0];
			base_mech_x[3] = base_mech_x[0];
			base_mech_y[3] = base_mech_y[0];
			break;
		}
	}

	//キャリブレーション基準(固定)に合わせ込む
	if (target_scope == 17) {
		//
		//目標視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
			target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
			target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
			target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;

			//キャリ基準位置が同じ場合　※この処理の意図がわからない..
			if ((target_mech_x[0] == target_mech_x[1]) && (target_mech_y[0] == target_mech_y[1])) {
				//端点一致 視野1の場合
				if (method == 0x0102) {
					target_mech_x[1] = base_param->view[1].calib.pixel.axis_x[1];
					target_mech_y[1] = base_param->view[1].calib.pixel.axis_y[1];
				}
				//端点一致 視野2の場合
				if (method == 0x0103) {
					target_mech_x[0] = base_param->view[0].calib.pixel.axis_x[0];
					target_mech_y[0] = base_param->view[0].calib.pixel.axis_y[0];
				}
			}
			break;
			//視野数1
		case 1:
			//キャリ基準固定の場合
			if (read_type == 0x02) {
				//端点一致 視野1の場合
				if (method == 0x0102) {
					target_mech_x[0] = base_param->view[0].calib.pixel.axis_base_x;
					target_mech_y[0] = base_param->view[0].calib.pixel.axis_base_y;
				}
				//端点一致 視野2の場合
				if (method == 0x0103) {
					target_mech_x[1] = base_param->view[1].calib.pixel.axis_base_x;
					target_mech_y[1] = base_param->view[1].calib.pixel.axis_base_y;
				}
				//端点一致 視野3の場合
				if (method == 0x0104) {
					target_mech_x[2] = base_param->view[2].calib.pixel.axis_base_x;
					target_mech_y[2] = base_param->view[2].calib.pixel.axis_base_y;
				}
				//端点一致 視野4の場合
				if (method == 0x0105) {
					target_mech_x[3] = base_param->view[3].calib.pixel.axis_base_x;
					target_mech_y[3] = base_param->view[3].calib.pixel.axis_base_y;
				}
			}
			break;
		}

		//
		//対象視野座標
		//
		switch (parts) {
			//視野数2
		case 2:
			base_mech_x[0] = base_param->view[0].align.mech_x;
			base_mech_y[0] = base_param->view[0].align.mech_y;
			base_mech_x[1] = base_param->view[1].align.mech_x;
			base_mech_y[1] = base_param->view[1].align.mech_y;
			break;
			//視野数1
		case 1:
			base_mech_x[0] = base_param->view[one_view].align.mech_x;
			base_mech_y[0] = base_param->view[one_view].align.mech_y;
			base_mech_x[1] = base_mech_x[0];
			base_mech_y[1] = base_mech_y[0];
			base_mech_x[2] = base_mech_x[0];
			base_mech_y[2] = base_mech_y[0];
			base_mech_x[3] = base_mech_x[0];
			base_mech_y[3] = base_mech_y[0];
			break;
		}
	}

	//
	//2視野補正演算実行
	//
	double base_rotation_center_x = base_param->conf_rotcent_x;
	double base_rotation_center_y = base_param->conf_rotcent_y;
	BOOL theta_complete = base_param->calib_theta_complete;
	double offset_x;	//外部オフセットX
	double offset_y;	//外部オフセットY
	double offset_t;	//外部オフセットT(radian)
	if (target_scope >= 0 && target_scope <= 15) {
		offset_x = target_param->ext_offset_x;
		offset_y = target_param->ext_offset_y;
		offset_t = Tips0.DegToRadian(target_param->ext_offset_t);
	}
	else {
		offset_x = base_param->ext_offset_x;
		offset_y = base_param->ext_offset_y;
		offset_t = Tips0.DegToRadian(base_param->ext_offset_t);
	}
	double theta_x;				//補正結果 θX
	double theta_y;				//補正結果 θY
	double delta_t_rad;			//補正結果 ⊿T(radian)
	double delta_x;				//補正結果 ⊿X
	double delta_y;				//補正結果 ⊿Y
	int result_align = 1;
	//アライメント視野数1の場合
	if (parts == 1) {
		result_align = OnePartsAlignment(parts,
										method,
										target_mech_x, target_mech_y,
										base_mech_x, base_mech_y,
										offset_x, offset_y, offset_t,
										&theta_x, &theta_y, &delta_t_rad, &delta_x, &delta_y);
	}
	//アライメント視野数2の場合
	if (parts == 2) {
		result_align = TwoPartsAlignment(parts,
										method,
										target_mech_x, target_mech_y,
										base_mech_x, base_mech_y,
										base_rotation_center_x, base_rotation_center_y, theta_complete,
										offset_x, offset_y, offset_t,
										&theta_x, &theta_y, &delta_t_rad, &delta_x, &delta_y);
	}
	if (result_align != 0) goto END;

	//
	//マーク間距離
	//
	double base_mark_width_array[4] = {};	//対象スコープマーク間距離 0で初期化
	double target_mark_width_array[4] = {};	//目標スコープマーク間距離 0で初期化
	CalcMarkWidth(base_mech_x, base_mech_y, target_mech_x, target_mech_y, base_mark_width_array, target_mark_width_array);

	//統計データセット
	if (SetStatistics(base_scope, theta_x, theta_y, delta_t_rad, delta_x, delta_y) != 0) goto END;

	//
	//端点一致の演算結果のデータセット
	//
	int scope = sys_template[template_number].scope;
	SetResult(cmd, scope, base_method, theta_x, theta_y, delta_t_rad, delta_x, delta_y, base_mark_width_array, target_mark_width_array);

	//コマンドログ出力
	CmdLogOut(cmd, theta_x, theta_y, delta_t_rad, delta_x, delta_y, base_mark_width_array, target_mark_width_array);

	//デバックモードログ出力
	DebugLogOut(target_mech_x, target_mech_y, base_mech_x, base_mech_y, parts);

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 3～4視野対応アライメントの演算実行
 ** 引数：
 **　int parts					アライメント視野数
 **　WORD method				探索機能
 **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 **　double rotation_center_x	回転中心X
 **　double rotation_center_y	回転中心Y
 **　BOOL theta_complete		回転中心キャリブレーション完了フラグ
 **　double offset_x			オフセットX
 **　double offset_y			オフセットY
 **　double offset_t			オフセットT(radian)
 **　double* theta_x			補正量θX
 **　double* theta_y			補正量θY
 **　double* delta_t_rad		補正量⊿T(radian)
 **　double* delta_x			補正量⊿X
 **　double* delta_y			補正量⊿Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		アライメント視野数は3もしくは4
 **		対象とする探索機能は
 **		0x0101:中点一致
 **		旧システムでは端点一致の演算も記述されているが、機能させていない(本システムでも対応しない)
 ** ****************************************************** **/
int CCalcAlign::FourPartsAlignment(int parts,
								WORD method,
								double* target_mech_x, double* target_mech_y,
								double* base_mech_x, double* base_mech_y,
								double rotation_center_x, double rotation_center_y, BOOL theta_complete,
								double offset_x, double offset_y, double offset_t,
								double* theta_x, double* theta_y, double* delta_t_rad, double* delta_x, double* delta_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//アライメント視野数の確認
	if (parts < 3 || parts > 4) goto END;

	//探索機能の確認
	if (method != 0x0101) goto END;

	outlog.LogOutFile(1, "4点アライメント 旧タイプ演算使用");

	double bufX;
	double bufY;
	double bufT;

	offset_x = offset_x * -1;
	offset_y = offset_y * -1;
	offset_t = offset_t * -1;

	//目標側傾き
	double target_theta = 0;
	if ((target_mech_x[1] - target_mech_x[0]) == 0) {
		target_theta = 0;
	}
	else {
		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
	}

	//ｵﾌｾｯﾄ加算
	double result_offx = offset_x * cos(target_theta) - offset_y * sin(target_theta);
	double result_offy = offset_y * cos(target_theta) + offset_x * sin(target_theta);

	double tartget_xs[4];
	double tartget_ys[4];
	tartget_xs[0] = target_mech_x[0], tartget_ys[0] = target_mech_y[0];
	tartget_xs[1] = target_mech_x[1], tartget_ys[1] = target_mech_y[1];
	tartget_xs[2] = target_mech_x[2], tartget_ys[2] = target_mech_y[2];
	tartget_xs[3] = target_mech_x[3], tartget_ys[3] = target_mech_y[3];

	double base_xm[4];
	double base_ym[4];
	base_xm[0] = base_mech_x[0], base_ym[0] = base_mech_y[0];
	base_xm[1] = base_mech_x[1], base_ym[1] = base_mech_y[1];
	base_xm[2] = base_mech_x[2], base_ym[2] = base_mech_y[2];
	base_xm[3] = base_mech_x[3], base_ym[3] = base_mech_y[3];

	//対象側対角中点座標
	double base_center_x = 0;
	double base_center_y = 0;
	if (GetPartsCenter(parts, &base_center_x, &base_center_y, base_xm, base_ym) != 0) goto END;
	//目標側対角中点座標
	double target_center_x = 0;
	double target_center_y = 0;
	if (GetPartsCenter(parts, &target_center_x, &target_center_y, tartget_xs, tartget_ys) != 0) goto END;

	bufX = 0;
	bufY = 0;
	bufT = 0;
	if (Align4(parts, &bufT, &bufX, &bufY, tartget_xs, tartget_ys, base_xm, base_ym) != 0) goto END;

	*delta_t_rad = (bufT + offset_t)*-1;
	//*delta_x = ((target_center_x - base_center_x) + result_offx);
	//*delta_y = ((target_center_y - base_center_y) + result_offy);
	//上記⊿X,⊿Yへの外部オフセットの計算がおかしい
	//　・オフセットθが含まれていない
	//　・オフセットX,Y加算の符号が間違っている
	//よって、オフセット分の計算を修正する
	double offset_target_theta = target_theta + (offset_t * -1);
	double delta_off_x = (offset_x * -1) * cos(offset_target_theta) - (offset_y * -1) * sin(offset_target_theta);
	double delta_off_y = (offset_y * -1) * cos(offset_target_theta) + (offset_x * -1) * sin(offset_target_theta);
	*delta_x = ((target_center_x - base_center_x) + delta_off_x);
	*delta_y = ((target_center_y - base_center_y) + delta_off_y);

	//θ成分含むXY(θX,θY)の算出
	if (theta_complete) {
		bufT = bufT * -1;

		for (int i = 0; i < 4; i++) {
			double xbuf = 0;
			double ybuf = 0;

			xbuf = cos(bufT)*(base_xm[i] - rotation_center_x) - sin(bufT)*(base_ym[i] - rotation_center_y) + rotation_center_x;
			ybuf = cos(bufT)*(base_ym[i] - rotation_center_y) + sin(bufT)*(base_xm[i] - rotation_center_x) + rotation_center_y;

			base_xm[i] = xbuf;
			base_ym[i] = ybuf;
		}

		bufX = 0;
		bufY = 0;
		bufT = 0;
		if (Align4(parts, &bufT, &bufX, &bufY, tartget_xs, tartget_ys, base_xm, base_ym) != 0) goto END;

		for (int i = 0; i < 4; i++) {
			base_xm[i] = base_xm[i] - bufX;
			base_ym[i] = base_ym[i] - bufY;
		}

		//中点一致の場合
		if (method == 0x0101) {
			//対象側対角中点座標
			if (GetPartsCenter(parts, &base_center_x, &base_center_y, base_xm, base_ym) != 0) goto END;

			double offt_add_x = 0;
			double offt_add_y = 0;
			if (offset_t != 0) {
				offt_add_x = cos(offset_t) * (base_center_x - rotation_center_x) - sin(offset_t) * (base_center_y - rotation_center_y) + rotation_center_x;
				offt_add_y = cos(offset_t) * (base_center_y - rotation_center_y) + sin(offset_t) * (base_center_x - rotation_center_x) + rotation_center_y;
				offt_add_x = offt_add_x - base_center_x;
				offt_add_y = offt_add_y - base_center_y;
			}

			//中点一致
			*theta_x = bufX + result_offx + offt_add_x;
			*theta_y = bufY + result_offy + offt_add_y;
		}

		//4点アライメントでの端点一致処理は対応しない
		/************************************* ここから
		else if (Method == 2)
		{	//視野１
			SiyaX = xm[0] - xs[0];
			SiyaY = ym[0] - ys[0];
			CX = xm[0] - SiyaX;
			CY = ym[0] - SiyaY;

			if (OffT != 0)
			{
				RzX = cos(OffT)*(CX - RotX) - sin(OffT)*(CY - RotY) + RotX;
				RzY = cos(OffT)*(CY - RotY) + sin(OffT)*(CX - RotX) + RotY;
				RzX = RzX - CX;
				RzY = RzY - CY;
			}

			dTX = bufX + SiyaX + OffX + RzX;
			dTY = bufY + SiyaY + OffY + RzY;
		}
		else if (Method == 3)
		{
			//視野２
			SiyaX = xm[1] - xs[1];
			SiyaY = ym[1] - ys[1];
			CX = xm[1] - SiyaX;
			CY = ym[1] - SiyaY;

			if (OffT != 0)
			{
				RzX = cos(OffT)*(CX - RotX) - sin(OffT)*(CY - RotY) + RotX;
				RzY = cos(OffT)*(CY - RotY) + sin(OffT)*(CX - RotX) + RotY;
				RzX = RzX - CX;
				RzY = RzY - CY;
			}

			dTX = bufX + SiyaX + OffX + RzX;
			dTY = bufY + SiyaY + OffY + RzY;
		}
		else if (Method == 4)
		{
			//視野3
			SiyaX = xm[2] - xs[2];
			SiyaY = ym[2] - ys[2];
			CX = xm[2] - SiyaX;
			CY = ym[2] - SiyaY;
			if (OffT != 0)
			{
				RzX = cos(OffT)*(CX - RotX) - sin(OffT)*(CY - RotY) + RotX;
				RzY = cos(OffT)*(CY - RotY) + sin(OffT)*(CX - RotX) + RotY;
				RzX = RzX - CX;
				RzY = RzY - CY;
			}

			dTX = bufX + SiyaX + OffX + RzX;
			dTY = bufY + SiyaY + OffY + RzY;
		}
		else if (Method == 5)
		{
			//視野4
			SiyaX = xm[3] - xs[3];
			SiyaY = ym[3] - ys[3];
			CX = xm[3] - SiyaX;
			CY = ym[3] - SiyaY;

			if (OffT != 0)
			{
				RzX = cos(OffT)*(CX - RotX) - sin(OffT)*(CY - RotY) + RotX;
				RzY = cos(OffT)*(CY - RotY) + sin(OffT)*(CX - RotX) + RotY;
				RzX = RzX - CX;
				RzY = RzY - CY;
			}

			dTX = bufX + SiyaX + OffX + RzX;
			dTY = bufY + SiyaY + OffY + RzY;
		}
		ここまで ****************************************/

		*theta_x = (*theta_x) * -1;
		*theta_y = (*theta_y) * -1;
	}
	//回転中心キャリブレーションが完了していない場合(回転中心キャリブレーションをしていない場合)
	else {
		*theta_x = 0;
		*theta_y = 0;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 3～4視野対応アライメントの演算実行(新タイプ)
 ** 引数：
 **　int parts					アライメント視野数
 **　WORD method				探索機能
 **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 **　double rotation_center_x	回転中心X
 **　double rotation_center_y	回転中心Y
 **　BOOL theta_complete		回転中心キャリブレーション完了フラグ
 **　double offset_x			オフセットX
 **　double offset_y			オフセットY
 **　double offset_t			オフセットT(radian)
 **　double* theta_x			補正量θX
 **　double* theta_y			補正量θY
 **　double* delta_t_rad		補正量⊿T(radian)
 **　double* delta_x			補正量⊿X
 **　double* delta_y			補正量⊿Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		アライメント視野数は4
 **		対象とする探索機能は 0x0101:中点一致 のみ
 **		旧タイプ不具合により新規作成
 **		対象スコープと目標スコープの各視野1/視野2検出位置から⊿Tを算出、θ補正後の両スコープ重心同士の補正量X/YをTX/TYとする
 ** ****************************************************** **/
int CCalcAlign::FourPartsAlignmentRev2(int parts,
									WORD method,
									double* target_mech_x, double* target_mech_y,
									double* base_mech_x, double* base_mech_y,
									double rotation_center_x, double rotation_center_y, BOOL theta_complete,
									double offset_x, double offset_y, double offset_t,
									double* theta_x, double* theta_y, double* delta_t_rad, double* delta_x, double* delta_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//アライメント視野数の確認
	if (parts < 3 || parts > 4) goto END;

	//探索機能の確認
	if (method != 0x0101) goto END;

	///////////////////////////////////////////////////////////////////////////////
	//⊿T算出
	//対象スコープの視野1,2の傾き、目標スコープの視野1,2の傾きの算出
	//算出方法は2視野アライメントの方法をそのまま使用する

	double pi = acos(double(-1));

	//対象側傾き
	double base_theta = 0;
	if ((base_mech_x[1] - base_mech_x[0]) == 0) {
		//base_theta = pi / 2;
		if ((base_mech_y[1] - base_mech_y[0]) == 0) {
			base_theta = 0;
		}
		else if ((base_mech_y[1] - base_mech_y[0]) > 0) {
			base_theta = -(pi / 2);
		}
		else if ((base_mech_y[1] - base_mech_y[0]) < 0) {
			base_theta = pi / 2;
		}
	}
	else if ((base_mech_x[1] - base_mech_x[0]) > 0) {
		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
	}
	else if ((base_mech_x[1] - base_mech_x[0]) < 0) {
		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
		base_theta = pi + base_theta;
	}

	//目標側傾き
	double target_theta = 0;
	if ((target_mech_x[1] - target_mech_x[0]) == 0) {
		//target_theta = pi / 2;
		if ((target_mech_y[1] - target_mech_y[0]) == 0) {
			target_theta = 0;
		}
		else if ((target_mech_y[1] - target_mech_y[0]) > 0) {
			target_theta = -(pi / 2);
		}
		else if ((target_mech_y[1] - target_mech_y[0]) < 0) {
			target_theta = pi / 2;
		}
	}
	else if ((target_mech_x[1] - target_mech_x[0]) > 0) {
		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
	}
	else if ((target_mech_x[1] - target_mech_x[0]) < 0) {
		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
		target_theta = pi + target_theta;
	}

	//補正値Δθ
	double dan = target_theta - base_theta;
	double offset_target_theta = target_theta + offset_t;
	*delta_t_rad = dan + offset_t;
	//⊿T算出ここまで
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	//⊿X⊿Y算出
	//算出方法は旧タイプの方法をそのまま使用する
	//WILライブラリ内に4点の重心を算出できる関数があるので、それを利用しても良い

	double tartget_xs[4];
	double tartget_ys[4];
	tartget_xs[0] = target_mech_x[0], tartget_ys[0] = target_mech_y[0];
	tartget_xs[1] = target_mech_x[1], tartget_ys[1] = target_mech_y[1];
	tartget_xs[2] = target_mech_x[2], tartget_ys[2] = target_mech_y[2];
	tartget_xs[3] = target_mech_x[3], tartget_ys[3] = target_mech_y[3];

	double base_xm[4];
	double base_ym[4];
	base_xm[0] = base_mech_x[0], base_ym[0] = base_mech_y[0];
	base_xm[1] = base_mech_x[1], base_ym[1] = base_mech_y[1];
	base_xm[2] = base_mech_x[2], base_ym[2] = base_mech_y[2];
	base_xm[3] = base_mech_x[3], base_ym[3] = base_mech_y[3];

	//対象側対角中点座標
	double base_center_x = 0;
	double base_center_y = 0;
	if (GetPartsCenter(parts, &base_center_x, &base_center_y, base_xm, base_ym) != 0) goto END;
	//目標側対角中点座標
	double target_center_x = 0;
	double target_center_y = 0;
	if (GetPartsCenter(parts, &target_center_x, &target_center_y, tartget_xs, tartget_ys) != 0) goto END;
	//目標側にｵﾌｾｯﾄ加算
	target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
	target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);

	//補正値Δｘ、補正値Δｙ
	*delta_x = target_center_x - base_center_x;
	*delta_y = target_center_y - base_center_y;
	//⊿X⊿Y算出ここまで
	///////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	//θ成分含むXY(θX,θY)の算出

	if (theta_complete) {
		//θ回転後の対象マークセンター座標
		double theta_aligned_center_x = (base_center_x - rotation_center_x) * cos(*delta_t_rad) - (base_center_y - rotation_center_y) * sin(*delta_t_rad) + rotation_center_x;
		double theta_aligned_center_y = (base_center_x - rotation_center_x) * sin(*delta_t_rad) + (base_center_y - rotation_center_y) * cos(*delta_t_rad) + rotation_center_y;

		*theta_x = target_center_x - theta_aligned_center_x;
		*theta_y = target_center_y - theta_aligned_center_y;
	}
	//回転中心キャリブレーションが完了していない場合(回転中心キャリブレーションをしていない場合)
	else {
		*theta_x = 0;
		*theta_y = 0;
	}
	//TXTY算出ここまで
	///////////////////////////////////////////////////////////////////////////////

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 2視野対応アライメントの演算実行
 ** 引数：
 **　int parts					アライメント視野数
 **　WORD method				探索機能
 **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 **　double rotation_center_x	回転中心X
 **　double rotation_center_y	回転中心Y
 **　BOOL theta_complete		回転中心キャリブレーション完了フラグ
 **　double offset_x			オフセットX
 **　double offset_y			オフセットY
 **　double offset_t			オフセットT(radian)
 **　double* theta_x			補正量θX
 **　double* theta_y			補正量θY
 **　double* delta_t_rad		補正量⊿T(radian)
 **　double* delta_x			補正量⊿X
 **　double* delta_y			補正量⊿Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		アライメント視野数は2
 **		対象とする探索機能は
 **		0x0101:中点一致
 **		0x0102:端点一致 視野1
 **		0x0103:端点一致 視野2
 ** ****************************************************** **/
int CCalcAlign::TwoPartsAlignment(int parts,
								WORD method,
								double* target_mech_x, double* target_mech_y,
								double* base_mech_x, double* base_mech_y,
								double rotation_center_x, double rotation_center_y, BOOL theta_complete,
								double offset_x, double offset_y, double offset_t,
								double* theta_x, double* theta_y, double* delta_t_rad, double* delta_x, double* delta_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//アライメント視野数の確認
	if (parts != 2) goto END;

	//探索機能の確認
	if (method != 0x0101 && method != 0x0102 && method != 0x0103) goto END;

	double pi = acos(double(-1));

	//対象側傾き
	double base_theta = 0;
	//FVXでは視野1,2でX方向の変化がない場合、90度足していた
	//　WILでは下記に変更
	//　　・Y方向も変化がない場合 → 0
	//　　・Y方向視野1より視野2のY位置が大きい場合 → -(pi / 2) -90度
	//　　・Y方向視野1より視野2のY位置が小さい場合 → +(pi / 2) +90度
	//また視野1より視野2のX位置が小さい場合、180度足していた
	//　→ WILでは検討要
	if ((base_mech_x[1] - base_mech_x[0]) == 0) {
		//base_theta = pi / 2;
		if ((base_mech_y[1] - base_mech_y[0]) == 0) {
			base_theta = 0;
		}
		else if ((base_mech_y[1] - base_mech_y[0]) > 0) {
			base_theta = -(pi / 2);
		}
		else if ((base_mech_y[1] - base_mech_y[0]) < 0) {
			base_theta = pi / 2;
		}
	}
	else if ((base_mech_x[1] - base_mech_x[0]) > 0) {
		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
	}
	else if ((base_mech_x[1] - base_mech_x[0]) < 0) {
		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
		base_theta = pi + base_theta;
	}

	//目標側傾き
	double target_theta = 0;
	//FVXでは視野1,2でX方向の変化がない場合、90度足していた
	//　WILでは下記に変更
	//　　・Y方向も変化がない場合 → 0
	//　　・Y方向視野1より視野2のY位置が大きい場合 → -(pi / 2) -90度
	//　　・Y方向視野1より視野2のY位置が小さい場合 → +(pi / 2) +90度
	//また視野1より視野2のX位置が小さい場合、180度足していた
	//　→ WILでは検討要
	if ((target_mech_x[1] - target_mech_x[0]) == 0) {
		//target_theta = pi / 2;
		if ((target_mech_y[1] - target_mech_y[0]) == 0) {
			target_theta = 0;
		}
		else if ((target_mech_y[1] - target_mech_y[0]) > 0) {
			target_theta = -(pi / 2);
		}
		else if ((target_mech_y[1] - target_mech_y[0]) < 0) {
			target_theta = pi / 2;
		}
	}
	else if ((target_mech_x[1] - target_mech_x[0]) > 0) {
		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
	}
	else if ((target_mech_x[1] - target_mech_x[0]) < 0) {
		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
		target_theta = pi + target_theta;
	}

	//補正値Δθ
	double dan = target_theta - base_theta;
	double offset_target_theta = target_theta + offset_t;
	//dT = dan*-1;
	*delta_t_rad = dan + offset_t;

	//対象側センター座標
	double base_center_x = 0;
	double base_center_y = 0;
	switch (method) {
		//中点一致
	case 0x0101:
		base_center_x = (base_mech_x[0] + base_mech_x[1]) / 2;
		base_center_y = (base_mech_y[0] + base_mech_y[1]) / 2;
		break;
		//端点一致 視野1
	case 0x0102:
		base_center_x = base_mech_x[0];
		base_center_y = base_mech_y[0];
		break;
		//端点一致 視野2
	case 0x0103:
		base_center_x = base_mech_x[1];
		base_center_y = base_mech_y[1];
		break;
	}

	//目標側センター座標
	//どこに合わせこむかにより場合分け
	double target_center_x = 0;
	double target_center_y = 0;
	switch (method) {
		//中点一致
	case 0x0101:
		target_center_x = (target_mech_x[0] + target_mech_x[1]) / 2;
		target_center_y = (target_mech_y[0] + target_mech_y[1]) / 2;
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
		//端点一致 視野1
	case 0x0102:
		target_center_x = target_mech_x[0];
		target_center_y = target_mech_y[0];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
		//端点一致 視野2
	case 0x0103:
		target_center_x = target_mech_x[1];
		target_center_y = target_mech_y[1];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
	}

	//θ成分含むXY(θX,θY)の算出
	if (theta_complete) {
		//θ回転後の対象マークセンター座標
		double theta_aligned_center_x = (base_center_x - rotation_center_x) * cos(*delta_t_rad) - (base_center_y - rotation_center_y) * sin(*delta_t_rad) + rotation_center_x;
		double theta_aligned_center_y = (base_center_x - rotation_center_x) * sin(*delta_t_rad) + (base_center_y - rotation_center_y) * cos(*delta_t_rad) + rotation_center_y;

		//デバック 20200403
		//　演算方法を替え算出してみたが結果は同じだったので、既存の算出方法のままとする
		//double sub_x = rotation_center_x - base_center_x;
		//double sub_y = rotation_center_y - base_center_y;
		//double L = sqrt(pow(sub_x, 2.0) + pow(sub_y, 2.0));

		//double theta_base_rad = acos(sub_x / L);
		//double theta_base_rad2 = atan(sub_y / sub_x);
		//double theta_algn_rad = theta_base_rad + *delta_t_rad;

		//double debug_align_x = L * cos(theta_algn_rad);
		//double debug_align_y = L * sin(theta_algn_rad);
		//double debug_theta_aligned_center_x = rotation_center_x - debug_align_x;
		//double debug_theta_aligned_center_y = rotation_center_y - debug_align_y;
		//デバック ここまで


		//最終アライメントセンター位置座標   
		//RESULT_DATA.mXac=nx;
		//RESULT_DATA.mYac=ny;

		//アライメント視野数2の場合
		if (parts == 2) {
			//符号を変える
			*theta_x = target_center_x - theta_aligned_center_x;
			*theta_y = target_center_y - theta_aligned_center_y;
		}
	}
	//回転中心キャリブレーションが完了していない場合(回転中心キャリブレーションをしていない場合)
	else {
		*theta_x = 0;
		*theta_y = 0;
	}

	//補正値Δｘ、補正値Δｙ
	*delta_x = target_center_x - base_center_x;
	*delta_y = target_center_y - base_center_y;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 1視野対応アライメントの演算実行
 ** 引数：
 **　int parts					アライメント視野数
 **　WORD method				探索機能
 **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 **　double offset_x			オフセットX
 **　double offset_y			オフセットY
 **　double offset_t			オフセットT(radian)
 **　double* theta_x			補正量θX
 **　double* theta_y			補正量θY
 **　double* delta_t_rad		補正量⊿T(radian)
 **　double* delta_x			補正量⊿X
 **　double* delta_y			補正量⊿Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		アライメント視野数は1
 **		対象とする探索機能は
 **		0x0102:端点一致 視野1
 **		0x0103:端点一致 視野2
 **		0x0104:端点一致 視野3
 **		0x0105:端点一致 視野4
 ** ****************************************************** **/
int CCalcAlign::OnePartsAlignment(int parts,
	WORD method,
	double* target_mech_x, double* target_mech_y,
	double* base_mech_x, double* base_mech_y,
	double offset_x, double offset_y, double offset_t,
	double* theta_x, double* theta_y, double* delta_t_rad, double* delta_x, double* delta_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//アライメント視野数の確認
	if (parts != 1) goto END;

	//探索機能の確認
	if (method != 0x0102 && method != 0x0103 && method != 0x0104 && method != 0x0105) goto END;

	//対象側傾き
	double base_theta = 0;

	//目標側傾き
	double target_theta = 0;

	//補正値Δθ
	double dan = target_theta - base_theta;
	double offset_target_theta = target_theta + offset_t;
	//dT = dan*-1;
	*delta_t_rad = dan + offset_t;

	//対象側センター座標
	double base_center_x = 0;
	double base_center_y = 0;
	switch (method) {
		//端点一致 視野1
	case 0x0102:
		base_center_x = base_mech_x[0];
		base_center_y = base_mech_y[0];
		break;
		//端点一致 視野2
	case 0x0103:
		base_center_x = base_mech_x[1];
		base_center_y = base_mech_y[1];
		break;
		//端点一致 視野3
	case 0x0104:
		base_center_x = base_mech_x[2];
		base_center_y = base_mech_y[2];
		break;
		//端点一致 視野4
	case 0x0105:
		base_center_x = base_mech_x[3];
		base_center_y = base_mech_y[3];
		break;
	}

	//目標側センター座標
	//どこに合わせこむかにより場合分け
	double target_center_x = 0;
	double target_center_y = 0;
	switch (method) {
		//端点一致 視野1
	case 0x0102:
		target_center_x = target_mech_x[0];
		target_center_y = target_mech_y[0];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
		//端点一致 視野2
	case 0x0103:
		target_center_x = target_mech_x[1];
		target_center_y = target_mech_y[1];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
		//端点一致 視野3
	case 0x0104:
		target_center_x = target_mech_x[2];
		target_center_y = target_mech_y[2];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
		//端点一致 視野4
	case 0x0105:
		target_center_x = target_mech_x[3];
		target_center_y = target_mech_y[3];
		//ｵﾌｾｯﾄ加算
		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
		break;
	}

	//補正量θX／θY、補正量Δθ
	//*theta_x = target_center_x - base_center_x;	//視野数1の場合は⊿Xを代入(旧システム仕様)
	//*theta_y = target_center_y - base_center_y;	//視野数1の場合は⊿Yを代入(旧システム仕様)
	*theta_x = 0;
	*theta_y = 0;
	*delta_t_rad = 0;

	//補正値Δｘ、補正値Δｙ
	*delta_x = target_center_x - base_center_x;
	*delta_y = target_center_y - base_center_y;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 対角中点座標を求める
 ** 引数：
 **　int parts				アライメント視野数
 **　double* center_x		中心座標X
 **　double* center_y		中心座標Y
 **　const double* mech_x	視野座標X(目標視野座標X配列の先頭アドレス)
 **　const double* mech_y	視野座標Y(目標視野座標X配列の先頭アドレス)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::GetPartsCenter(int parts, double* center_x, double* center_y, const double* mech_x, const double* mech_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//アライメント視野数の確認
	if (parts < 3 || parts > 4) goto END;

	double a1 = 0, b1 = 0, a2 = 0, b2 = 0;

	double mech_cent_x[4];	//マーク間センターX
	double mech_cent_y[4];	//マーク間センターY

	//アライメント視野数4
	if (parts == 4) {

		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
		mech_cent_x[1] = (mech_x[1] + mech_x[2]) / 2;
		mech_cent_x[2] = (mech_x[2] + mech_x[3]) / 2;
		mech_cent_x[3] = (mech_x[3] + mech_x[0]) / 2;

		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
		mech_cent_y[1] = (mech_y[1] + mech_y[2]) / 2;
		mech_cent_y[2] = (mech_y[2] + mech_y[3]) / 2;
		mech_cent_y[3] = (mech_y[3] + mech_y[0]) / 2;

		if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0 && mech_cent_x[0] != 0 && mech_cent_x[1] != 0) {

			b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
			a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];

			b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
			a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];

			//交点
			*center_y = (a1 * b2 - a2 * b1) / (a1 - a2);
			*center_x = (*center_y - b1) / a1;
		}
		else {
			if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0) {

				b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
				a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];

				//交点
				*center_x = mech_cent_x[0];
				*center_y = a2 * (*center_x) + b2;
			}
			else if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {

				b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
				a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];

				//交点
				*center_x = mech_cent_x[1];
				*center_y = a1 * (*center_x) + b1;
			}
			else if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {

				*center_x = mech_cent_x[0];
				*center_y = (mech_y[0] + mech_y[3]) / 2;
			}
		}
	}
	//アライメント視野数3
	else if (parts == 3) {
		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
		mech_cent_x[1] = (mech_x[0] + mech_x[2]) / 2;
		mech_cent_y[1] = (mech_y[0] + mech_y[2]) / 2;

		if ((mech_x[1] - mech_cent_x[1]) != 0 && (mech_x[2] - mech_cent_x[0]) != 0 && mech_x[1] != 0 && mech_x[2] != 0) {

			b1 = (mech_x[1] * mech_cent_y[1] - mech_cent_x[1] * mech_y[1]) / (mech_x[1] - mech_cent_x[1]);
			a1 = (mech_y[1] - b1) / mech_x[1];

			b2 = (mech_x[2] * mech_cent_y[0] - mech_cent_x[0] * mech_y[2]) / (mech_x[2] - mech_cent_x[0]);
			a2 = (mech_y[2] - b2) / mech_x[2];

			//交点
			*center_y = (a1*b2 - a2 * b1) / (a1 - a2);
			*center_x = (*center_y - b1) / a1;
		}
		else {
			goto END;
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 伸縮率、オフセット、角度を求める
 ** 引数：
 **　int nn				測定点の数
 **　double* thTemp		オフセットT
 **　double* xoTemp		オフセットX
 **　double* yoTemp		オフセットY
 **　const double* tartget_xs	目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　const double* tartget_ys	目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　const double* base_xm		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　const double* base_ym		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::Align4(int nn,
					double* thTemp, double* xoTemp, double* yoTemp,
					const double* tartget_xs, const double* tartget_ys,
					const double* base_xm, const double* base_ym)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	int i, in;
	int jCor[10];
	double th, xo, yo, dth, dxo, dyo, ddx, ddy;

	th = *thTemp;
	xo = *xoTemp;
	yo = *yoTemp;
	dth = dxo = dyo = 0.0;

	for (i = 0; i < nn; i++) {
		jCor[i] = 1;
	}
	for (i = nn; i < 10; i++) {
		jCor[i] = 0;
	}

	for (i = 0; i < 10; i++) {
		th -= dth;
		xo -= dxo;
		yo -= dyo;
		if (AlignmentStep(nn, jCor, th, xo, yo, &dth, &dxo, &dyo, tartget_xs, tartget_ys, base_xm, base_ym) != 0) goto END;
	}

	*thTemp = th;
	*xoTemp = xo;
	*yoTemp = yo;

	//※以下の処理不明 旧システムではblGapX,blGapYの初期化コード無し
	//	旧システムでも機能していないので実行しない
	//double blGapX = 0;	//Xズレ
	//double blGapY = 0;	//Yズレ
	//for (in = 0, i = 0; i < nn; i++) {
	//	if (jCor[i] == 1) {
	//		ddx = base_xm[i] - tartget_xs[i] * cos(th) + tartget_ys[i] * sin(th) - xo;
	//		ddy = base_ym[i] - tartget_xs[i] * sin(th) - tartget_ys[i] * cos(th) - yo;
	//		if (sqrt(ddx * ddx + ddy * ddy) < (blGapX + blGapY) / 2.0) {
	//			++in;
	//		}
	//		else jCor[i] = 0;
	//	}
	//	else jCor[i] = 0;
	//}
	//if (in < nn) goto END;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** パラメータの修正量を求める
 ** 引数：
 **　int nn				測定点の数
 **　int *jCor			測定値が存在すれば、1
 **　double th			オフセットT
 **　double xo			オフセットX
 **　double yo			オフセットY
 **　double* dth		θの次回のステップ幅
 **　double* dxo		xオフセットの次回のステップ幅
 **　double* dyo		yオフセットの次回のステップ幅
 **　const double* tartget_xs	目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　const double* tartget_ys	目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　const double* base_xm		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　const double* base_ym		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::AlignmentStep(int nn,
							int *jCor,
							double th, double xo, double yo,
							double* dth, double* dxo, double* dyo,
							const double* tartget_xs, const double* tartget_ys,
							const double* base_xm, const double* base_ym)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	/**********************************************************************************************************************************/
	/*    xdi =  xsi*cosθ - ysi*sinθ +  a        ---------- (1)                                                                     */
	/*    ydi =  xsi*sinθ + ysi*cosθ +  b        ---------- (2)                                                                     */
	/*	δxi  =  xmi - xsi*cosθ + ysi*sinθ - a   ---------- (3)                                                                     */
	/*	δyi  =  ymi - xsi*sinθ - ysi*cosθ - b   ---------- (4)                                                                     */
	/*   S  = Σ(δxi*δxi  + δyi*δyi)                                                                                              */
	/*   ∂S /∂(θ1) = 0   ----------  (9)                                                                                           */
	/*   ∂S /∂(a1)  = 0   ----------  (10)                                                                                          */
	/*   ∂S /∂(b1)  = 0   ----------  (11)                                                                                          */
	/*                                                                                                                                */
	/*        A   B   C       εθ    =    P                                                                                          */
	/*        B   D   E       εxo    =    Q                                                                                          */
	/*        C   E   F       εyo    =    R        ----------  ( 12 )                                                                */
	/*                                                                                                                                */
	/*  A = Σ(xsi*xsi+ysi*ysi), B = Σ(-xsi*sinθ-ysi*cosθ), C = Σ(xsi*cosθ-*ysi*sinθ)                                           */
	/*  D = Σ(1.0), E = Σ(0.0), F = Σ(1.0)                                                                                         */
	/*  P = Σ(δxi*(xsi*sinθ+ysi*cosθ)+δyi*(-xsi*cosθ+ysi*sinθ)), Q = Σ(-δxi),  R = Σ(-δyi)                                 */
	/**********************************************************************************************************************************/

	int i, j;
	double A, B, C, D, E, F, P, Q, R, Fxi, Fyi;

	double p[20][20];
	double r[20];
	double s[20];

	A = B = C = D = E = F = P = Q = R = 0.0;
	for (i = 0; i < nn; i++) {
		if (jCor[i] != 1) continue;
		A += tartget_xs[i] * tartget_xs[i] + tartget_ys[i] * tartget_ys[i];
		B += -tartget_xs[i] * sin(th) - tartget_ys[i] * cos(th);
		C += tartget_xs[i] * cos(th) - tartget_ys[i] * sin(th);
		D += 1.0;
		E += 0.0;
		F += 1.0;

		Fxi = base_xm[i] - tartget_xs[i] * cos(th) + tartget_ys[i] * sin(th) - xo; /* Fxi = δxi */
		Fyi = base_ym[i] - tartget_xs[i] * sin(th) - tartget_ys[i] * cos(th) - yo; /* Fyi = δyi */
		P += Fxi * (tartget_xs[i] * sin(th) + tartget_ys[i] * cos(th)) + Fyi * (-tartget_xs[i] * cos(th) + tartget_ys[i] * sin(th));
		Q += -Fxi;
		R += -Fyi;
	}

	for (i = 0; i < 3; i++) {
		r[i] = 0.0;
		for (j = 0; j < 3; j++) {
			p[i][j] = 0.0;
		}
	}

	p[0][0] = A;
	p[0][1] = p[1][0] = B;
	p[0][2] = p[2][0] = C;
	p[1][1] = D;
	p[1][2] = p[2][1] = E;
	p[2][2] = F;

	r[0] = P;
	r[1] = Q;
	r[2] = R;
	if (matrix_s(3, p, r, s) != 0) goto END;

	*dth = s[0];
	*dxo = s[1];
	*dyo = s[2];

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** nn次の行列の逆行列を求めてステップ幅を計算する
 ** 引数：
 **　int nn
 **　double (*p)[20]
 **　double* r
 **　double* s
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		nn <= 20
 ** ****************************************************** **/
int CCalcAlign::matrix_s(int nn, double (*p)[20], double* r, double* s)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	int i, j, k, in, n, ik;
	double	pp, pp1, pp2;

	double q[20][20];
	double o[20][20];

	/*初期化 */
	for (i = 0; i < 10; i++) {
		for (j = 0; j < 10; j++) {
			q[i][j] = 0.0;
		}
	}
	for (i = 0; i < 10; i++) {
		q[i][i] = 1.0;
	}

	/*  */
	for (i = 0; i < nn; i++) {
		for (j = 0; j < nn; j++) {
			o[i][j] = p[i][j];
		}
	}
	for (k = 0; k < nn; k++) {
		if (fabs(p[k][k]) < 1.0E-12) {
			in = 0;
			for (n = k + 1; n < nn; n++) {
				if (fabs(p[n][k]) > 1.0E-12) {
					in = 1;
					ik = n;
					break;
				}
			}
			if (in == 0) {
#if _DEBUG
				printf("This matrix has not a inverse matrix\n");
#endif
				goto END;
			}
			for (i = 0; i < nn; i++) {
				pp1 = p[k][i];
				pp2 = p[ik][i];
				p[k][i] = pp2;
				p[ik][i] = pp1;
			}
		}
		pp = p[k][k];
		for (i = 0; i < nn; i++) {
			p[k][i] = p[k][i] / pp;
			q[k][i] = q[k][i] / pp;
		}
		for (j = 0; j < nn; j++) {
			if (j == k)			continue;
			if (fabs(p[j][k]) > 1.000E-12) {
				pp = p[j][k];
				for (i = 0; i < nn; i++) {
					p[j][i] = p[j][i] - p[k][i] * pp;
					q[j][i] = q[j][i] - q[k][i] * pp;
				}
			}
		}
	}
	for (i = 0; i < nn; i++) {
		for (j = 0; j < nn; j++) {
			p[i][j] = 0.0;
			for (k = 0; k < nn; k++) {
				p[i][j] = p[i][j] + o[i][k] * q[k][j];
			}
		}
	}
	for (i = 0; i < nn; i++) {
		s[i] = 0.0;
		for (j = 0; j < nn; j++) {
			s[i] = s[i] + q[j][i] * r[j];
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** マーク間距離を算出する
 ** 引数：
 **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
 **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
 **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
 **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
 **　double* base_mark_width	対象スコープ マーク間距離(配列の先頭アドレス)
 **　double* target_mark_width	目標スコープ マーク間距離(配列の先頭アドレス)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcAlign::CalcMarkWidth(double* base_mech_x, double* base_mech_y,
							double* target_mech_x, double* target_mech_y,
							double* base_mark_width, double* target_mark_width)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	//対象スコープ マーク間距離
	//　base_mark_width[0]　視野1-2距離
	//　base_mark_width[1]　視野2-3距離
	//　base_mark_width[2]　視野3-4距離
	//　base_mark_width[3]　視野4-1距離
	for (int i = 0; i < 4; i++) {
		int view_r = i + 1;
		int view_s = i;
		if (view_r == 4) view_r = 0;
		base_mark_width[i] = sqrt(pow(base_mech_x[view_r] - base_mech_x[view_s], 2.0) + pow(base_mech_y[view_r] - base_mech_y[view_s], 2.0));
	}

	//目標スコープ マーク間距離(キャリブレーション基準の場合は基準間距離)
	//　target_mark_width[0]　視野1-2距離
	//　target_mark_width[1]　視野2-3距離
	//　target_mark_width[2]　視野3-4距離
	//　target_mark_width[3]　視野4-1距離
	for (int i = 0; i < 4; i++) {
		int view_r = i + 1;
		int view_s = i;
		if (view_r == 4) view_r = 0;
		target_mark_width[i] = sqrt(pow(target_mech_x[view_r] - target_mech_x[view_s], 2.0) + pow(target_mech_y[view_r] - target_mech_y[view_s], 2.0));
	}
}


/** ****************************************************** **
 ** 統計データセット
 ** 引数：
 **　int scope					対象スコープ
 **　double theta_x				補正結果 θX
 **　double theta_y				補正結果 θY
 **　double delta_t_rad			補正結果 ⊿T
 **　double delta_x				補正結果 ⊿X
 **　double delta_y				補正結果 ⊿Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcAlign::SetStatistics(int scope, double theta_x, double theta_y, double delta_t_rad, double delta_x, double delta_y)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	int rtn_result = 1;

	int now_index = data_statis.scope[scope].scope_align_start_index;	//現在のインデックス
	int now_value = data_statis.scope[scope].scope_align_data_value;	//現在のデータ量

	int add_index_num = -1;	//追加データインデックス
	int update_index = -1;	//更新インデックス
	int update_value = -1;	//更新データ量
	if (now_index == -1) {
		add_index_num = 0;
		update_index = 0;
		update_value = 1;
	}
	else if (now_index == 0) {
		if (now_value >= 1 && now_value <= 99) {
			add_index_num = now_value;
			update_index = 0;
			update_value = now_value + 1;
		}
		else if (now_value == 100) {
			add_index_num = 0;
			update_index = 1;
			update_value = 100;
		}
	}
	else if (now_value == 100){
		if (now_index >= 1 && now_index <= 98) {
			add_index_num = now_index;
			update_index = now_index + 1;
			update_value = 100;
		}
		else if (now_index == 99) {
			add_index_num = now_index;
			update_index = 0;
			update_value = 100;
		}
		else {
			//異常
			goto END;
		}
	}
	else {
		//異常
		goto END;
	}

	if (add_index_num == -1 || update_index == -1 || update_value == -1) goto END;	//異常

	//データ書き込み
	data_statis.scope[scope].align_data[add_index_num].theta_x = theta_x;
	data_statis.scope[scope].align_data[add_index_num].theta_y = theta_y;
	data_statis.scope[scope].align_data[add_index_num].delta_t_rad = delta_t_rad;
	data_statis.scope[scope].align_data[add_index_num].delta_x = delta_x;
	data_statis.scope[scope].align_data[add_index_num].delta_y = delta_y;
	data_statis.scope[scope].align_data[add_index_num].time = Tips0.TimeToStringSelect(":", TRUE, TRUE, TRUE, FALSE);
	data_statis.scope[scope].scope_align_start_index = update_index;
	data_statis.scope[scope].scope_align_data_value = update_value;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 演算結果データセット
 ** 引数：
 **　byte cmd					実行コマンド
 **　int scope					対象スコープ
 **　byte base_method			機能種別
 **　double theta_x				補正結果 θX
 **　double theta_y				補正結果 θY
 **　double delta_t_rad			補正結果 ⊿T
 **　double delta_x				補正結果 ⊿X
 **　double delta_y				補正結果 ⊿Y
 **　double* base_mark_width	対象スコープ マーク間距離(配列の先頭アドレス)
 **　double* target_mark_width	目標スコープ マーク間距離(配列の先頭アドレス)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcAlign::SetResult(byte cmd, int scope, byte base_method,
							double theta_x, double theta_y,
							double delta_t_rad, double delta_x, double delta_y,
							double* base_mark_width, double* target_mark_width)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	scope_param[scope].result.align.theta_x = theta_x;
	scope_param[scope].result.align.theta_y = theta_y;
	scope_param[scope].result.align.delta_t_degree = Tips0.RadianToDeg(delta_t_rad);	//degreeに変換
	scope_param[scope].result.align.delta_x = delta_x;
	scope_param[scope].result.align.delta_y = delta_y;
	for (int i = 0; i < 4; i++) {
		scope_param[scope].result.align.base_mark_width[i] = base_mark_width[i];
		scope_param[scope].result.align.target_mark_width[i] = target_mark_width[i];
	}

	scope_param[scope].result.type = base_method;	//データタイプ
	scope_param[scope].result.calc_cmd = cmd;		//実施コマンド
}


/** ****************************************************** **
 ** 書込データセット
 ** 引数：
 **　byte cmd	実行コマンド
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcAlign::SetOutData(byte cmd)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int write_type = cmd_param[cmd].write.type;				//データ書込設定

	//
	//演算結果のデータ出力
	//
	int scope = sys_template[template_number].scope;
	byte base_method = scope_param[scope].result.type;
	if (write_type == 0x00) base_method = cmd_param[cmd].base_method;	//データ無し書込の場合はコマンドパラメータを使用
	byte calc_cmd = scope_param[scope].result.calc_cmd;

	double theta_x = scope_param[scope].result.align.theta_x;
	double theta_y = scope_param[scope].result.align.theta_y;
	double delta_t_degree = scope_param[scope].result.align.delta_t_degree;
	double delta_x = scope_param[scope].result.align.delta_x;
	double delta_y = scope_param[scope].result.align.delta_y;
	double* base_mark_width = scope_param[scope].result.align.base_mark_width;
	double* target_mark_width = scope_param[scope].result.align.target_mark_width;
	calc_plc.AlignCalcDataOut(cmd, scope, base_method, calc_cmd, theta_x, theta_y, delta_t_degree, delta_x, delta_y, base_mark_width, target_mark_width);
}


/** ****************************************************** **
 ** コマンドログ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　double theta_x			補正結果 θX
 **　double theta_y			補正結果 θY
 **　double delta_t_rad		補正結果 ⊿T
 **　double delta_x			補正結果 ⊿X
 **　double delta_y			補正結果 ⊿Y
 **　double* base_mark_width	対象スコープ マーク間距離(配列の先頭アドレス)
 **　double* target_mark_width	目標スコープ マーク間距離(配列の先頭アドレス)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcAlign::CmdLogOut(byte cmd, double theta_x, double theta_y, double delta_t_rad, double delta_x, double delta_y, double* base_mark_width, double* target_mark_width)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正量θX:", "Align θX:") + Tips0.DblToString(theta_x, 4, TRUE) + "mm");	//コマンドログ 補正量θX
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正量θY:", "Align θY:") + Tips0.DblToString(theta_y, 4, TRUE) + "mm");	//コマンドログ 補正量θY
	double delta_t_degree = Tips0.RadianToDeg(delta_t_rad);	//degreeに変換
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正量⊿T:", "Align ⊿T:") + Tips0.DblToString(delta_t_degree, 4, TRUE) + "deg");	//コマンドログ 補正量⊿T
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正量⊿X:", "Align ⊿X:") + Tips0.DblToString(delta_x, 4, TRUE) + "mm");	//コマンドログ 補正量⊿X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正量⊿Y:", "Align ⊿Y:") + Tips0.DblToString(delta_y, 4, TRUE) + "mm");	//コマンドログ 補正量⊿Y

	int target_scope = cmd_param[cmd].align.target_scope;	//目標スコープ
	int parts = cmd_param[cmd].align.parts;					//アライメント視野数

	//対象スコープマーク幅
	switch (parts) {
		//視野数2
	case 2:
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "対象スコープ 視野1-2マーク幅:", "SubjectScope View1-2 Mark width:") + Tips0.DblToString(base_mark_width[0], 4, TRUE) + "mm");	//コマンドログ 補正量θX
		break;
		//視野数4
	case 4:
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "対象スコープ 視野1-2マーク幅:", "SubjectScope View1-2 MarkWidth:") + Tips0.DblToString(base_mark_width[0], 4, TRUE) + "mm");	//コマンドログ 視野1-2マーク幅
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "対象スコープ 視野2-3マーク幅:", "SubjectScope View2-3 MarkWidth:") + Tips0.DblToString(base_mark_width[1], 4, TRUE) + "mm");	//コマンドログ 視野2-3マーク幅
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "対象スコープ 視野3-4マーク幅:", "SubjectScope View3-4 MarkWidth:") + Tips0.DblToString(base_mark_width[2], 4, TRUE) + "mm");	//コマンドログ 視野3-4マーク幅
		outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "対象スコープ 視野4-1マーク幅:", "SubjectScope View4-1 MarkWidth:") + Tips0.DblToString(base_mark_width[3], 4, TRUE) + "mm");	//コマンドログ 視野4-1マーク幅
		break;
	}

	//目標スコープ
	if (target_scope >= 0 && target_scope <= 15) {
		switch (parts) {
			//視野数2
		case 2:
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "目標スコープ 視野1-2マーク幅:", "TargetScope View1-2 MarkWidth:") + Tips0.DblToString(target_mark_width[0], 4, TRUE) + "mm");	//コマンドログ 補正量θX
			break;
			//視野数4
		case 4:
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "目標スコープ 視野1-2マーク幅:", "TargetScope View1-2 MarkWidth:") + Tips0.DblToString(target_mark_width[0], 4, TRUE) + "mm");	//コマンドログ 視野1-2マーク幅
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "目標スコープ 視野2-3マーク幅:", "TargetScope View2-3 MarkWidth:") + Tips0.DblToString(target_mark_width[1], 4, TRUE) + "mm");	//コマンドログ 視野2-3マーク幅
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "目標スコープ 視野3-4マーク幅:", "TargetScope View3-4 MarkWidth:") + Tips0.DblToString(target_mark_width[2], 4, TRUE) + "mm");	//コマンドログ 視野3-4マーク幅
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "目標スコープ 視野4-1マーク幅:", "TargetScope View4-1 MarkWidth:") + Tips0.DblToString(target_mark_width[3], 4, TRUE) + "mm");	//コマンドログ 視野4-1マーク幅
			break;
		}
	}
}


/** ****************************************************** **
 ** デバックモード 視野機械座標表示
 ** 引数：
 **　double* target_mech_x		目標視野座標X
 **　double* target_mech_y		目標視野座標Y
 **　double* base_mech_x		対象視野座標X
 **　double* base_mech_y		対象視野座標Y
 **　int parts					アライメント視野数
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcAlign::DebugLogOut(
	double* target_mech_x,
	double* target_mech_y,
	double* base_mech_x,
	double* base_mech_y,
	int parts
)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	if (!sys_option.debug_mode) return;

	//目標スコープ
	for (int i = 0; i < parts; i++) {
		outlog.LogOutListCmd(0, 2, "AMx" + Tips0.IntToString(i) + ":" + Tips0.DblToString(target_mech_x[i], 4, TRUE));	//コマンドログ 視野機械座標X
		outlog.LogOutListCmd(0, 2, "AMy" + Tips0.IntToString(i) + ":" + Tips0.DblToString(target_mech_y[i], 4, TRUE));	//コマンドログ 視野機械座標Y
	}

	//対象スコープ
	for (int i = 0; i < parts; i++) {
		outlog.LogOutListCmd(0, 2, "ATx" + Tips0.IntToString(i) + ":" + Tips0.DblToString(base_mech_x[i], 4, TRUE));	//コマンドログ 視野機械座標X
		outlog.LogOutListCmd(0, 2, "ATy" + Tips0.IntToString(i) + ":" + Tips0.DblToString(base_mech_y[i], 4, TRUE));	//コマンドログ 視野機械座標Y
	}
}
