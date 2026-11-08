/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcCalib.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.09.12	M.Iwasaki	キャリブレーションデータコピー改善
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：キャリブレーション演算プログラム
 **
 **/

#include "stdafx.h"
#include "CalcCalib.h"

#include "DlgSystemMain.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Scopes.h"				//システムスコープパラメータ
#include "Commands.h"			//コマンドパラメータ
#include "CameraSettings.h"		//カメラパラメータ
#include "TempParam.h"			//機種毎テンプレートパラメータ
#include "ScopeParam.h"			//機種毎スコープパラメータ
#include "CommData.h"			//シリアル通信データクラス
#include "CurrentStatus.h"		//現状共通データ

#include "CalcMark.h"			//サーチ結果取得
#include "CalcConv.h"			//座標変換
#include "CalcPLC.h"			//PLCデータ取得

#include "AccessParamScrp.h"	//機種毎スコープパラメータファイルアクセス
#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

#define _USE_MATH_DEFINES
#include "math.h"

extern CTemplates			sys_template[256];		//システムテンプレートパラメータ
extern CScopes				sys_scope[16];			//システムスコープパラメータ
extern CCommands			cmd_param[256];			//コマンドパラメータ
extern CCameraSettings		cam_parameter[8];		//カメラパラメータ
extern CTempParam			template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam			scope_param[16];		//機種毎スコープパラメータ
extern CCurrentStatus		current;				//共有情報

extern CCalcMark calc_mark;			//サーチ結果取得
extern CCalcConv calc_conv;			//座標変換
extern CCalcPLC calc_plc;			//PLCデータ取得

extern CAccessParamScrp	access_scp;		//機種毎スコープパラメータファイルアクセス
extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

CCalcCalib::CCalcCalib()
{
}


CCalcCalib::~CCalcCalib()
{
}


/** ****************************************************** **
 ** キャリブレーションメイン
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CalcMain(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	int rtn_result = 1;

	byte cmd = dlg->run_cmd;	//実行コマンド
	byte base_method = cmd_param[cmd].base_method;	//機能種別

	//対象視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	//画素キャリブレーション実施
	if (cmd_param[cmd].search.method == 0x0307) {
		//画素キャリブレーションのデータセット
		if (PixelCalcDataSet(scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9431;	//エラーコード
			goto END;
		}
		//画素キャリブレーションの演算実行
		if (PixelCalcExec(scope, view, base_method) != 0) {
			dlg->thread_rs.calc_err = 0x9432;	//エラーコード
			goto END;
		}
	}
	//回転中心キャリブレーション実施
	else if (cmd_param[cmd].search.method == 0x0309) {
		//回転中心キャリブレーションのデータセット
		if (ThetaCalcDataSet(scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9431;	//エラーコード
			goto END;
		}
		//回転中心キャリブレーションの演算実行
		if (ThetaCalcExec(scope, view, base_method) != 0) {
			dlg->thread_rs.calc_err = 0x9433;	//エラーコード
			goto END;
		}
	}

	//経時変化基準指定
	else if (cmd_param[cmd].search.method == 0x0310) {
		//経時変化基準指定のデータセット
		if (VariationDataSetBase(template_number, scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9451;	//エラーコード
			goto END;
		}
	}

	//経時変化補正算出
	else if (cmd_param[cmd].search.method == 0x0311) {
		//経時変化補正算出のデータセット
		if (VariationDataSetMove(template_number, scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9451;	//エラーコード
			goto END;
		}
		
		//経時変化補正算出の演算実行
		if (VariationCalcExec(scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9452;	//エラーコード
			goto END;
		}
	}

	//経時変化補正リセット
	else if (cmd_param[cmd].search.method == 0x0312) {
		//経時変化基準指定のデータセット
		if (VariationCalcReset(scope, view) != 0) {
			dlg->thread_rs.calc_err = 0x9453;	//エラーコード
			goto END;
		}
	}

	//異常系(想定外コマンド)
	else {
		dlg->thread_rs.calc_err = 0x9439;	//エラーコード
		goto END;
	}

	//1スコープ1視野へのコピー
	if (cmd_param[cmd].calib.copy_scope >= 0) {
		if (CopyOne(cmd, scope, view) != 0) goto END;
	}
	//多スコープ多視野へのコピー
	else if (cmd_param[cmd].calib.copy_scope == -2) {
		if (CopyCustom(cmd, scope, view) != 0) goto END;
	}

	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "キャリブレーション演算完了", "Calibration calculation completed"));	//コマンドログ 演算完了

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** キャリブレーション結果保存
 ** 引数：
 **　int scope	スコープ番号(0～15)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::WriteResultCalib(int scope)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//Iniファイル書き込み
	if (access_scp.CheckIniFileParam(current.work_number, scope) == 0) {
		//iniファイル存在
		access_scp.WriteIniFileParam(current.work_number, scope);	//iniファイル更新
	}
	else {
		//iniファイルの作成
		if (access_scp.CreateIniFileParam(current.work_number, scope) != 0) {
			goto END;
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 経時変化結果保存
 ** 引数：
 **　int scope	スコープ番号(0～15)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::WriteResultVaria(int scope)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//Iniファイル書き込み
	if (access_scp.CheckVariationParam(current.work_number, scope) == 0) {
		//iniファイル存在
		access_scp.WriteVariationParam(current.work_number, scope);	//iniファイル更新
	}
	else {
		//iniファイルの作成
		if (access_scp.CreateVariationParam(current.work_number, scope) != 0) {
			goto END;
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 画素キャリブレーションのデータセット
 ** 引数：
 **　int scope	対象スコープ
 **　int view	対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::PixelCalcDataSet(int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//データセット ポジション1
	if (scope_param[scope].view[view].calib.pixel.enable[0]) {
		//画素Calポジション1のテンプレート番号を取得
		int template_number = scope_param[scope].view[view].calib.pixel.template_number[0];
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
		//スコープデータへセット
		scope_param[scope].view[view].calib.pixel.mark_x[0] = mark_x;
		scope_param[scope].view[view].calib.pixel.mark_y[0] = mark_y;

		//軸位置
		double axis_x;		//軸位置X
		double axis_y;		//軸位置Y
		if (calc_plc.GetAxisPosXY(template_number, &axis_x, &axis_y) != 0) goto END;
		scope_param[scope].view[view].calib.pixel.axis_x[0] = axis_x;
		scope_param[scope].view[view].calib.pixel.axis_y[0] = axis_y;
	}
	else {
		goto END;
	}

	//データセット ポジション2
	if (scope_param[scope].view[view].calib.pixel.enable[1]) {
		//画素Calポジション2のテンプレート番号を取得
		int template_number = scope_param[scope].view[view].calib.pixel.template_number[1];
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
		//スコープデータへセット
		scope_param[scope].view[view].calib.pixel.mark_x[1] = mark_x;
		scope_param[scope].view[view].calib.pixel.mark_y[1] = mark_y;

		//軸位置
		double axis_x;		//軸位置X
		double axis_y;		//軸位置Y
		if (calc_plc.GetAxisPosXY(template_number, &axis_x, &axis_y) != 0) goto END;
		scope_param[scope].view[view].calib.pixel.axis_x[1] = axis_x;
		scope_param[scope].view[view].calib.pixel.axis_y[1] = axis_y;
	}
	else {
		goto END;
	}

	//カメラ座標の構成タイプをスコープパラメータへ設定
	int cam_no = sys_scope[scope].view[view].assign_camera;
	scope_param[scope].view[view].coordinate_type = cam_parameter[cam_no].coordinate_type;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 画素キャリブレーションの演算実行
 ** 引数：
 **　int scope			対象スコープ
 **　int view			対象視野
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::PixelCalcExec(int scope, int view, byte base_method)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	//
	//Cal検出基準算出
	//
	double mark_x[2];
	double mark_y[2];
	mark_x[0] = scope_param[scope].view[view].calib.pixel.mark_x[0];
	mark_y[0] = scope_param[scope].view[view].calib.pixel.mark_y[0];
	mark_x[1] = scope_param[scope].view[view].calib.pixel.mark_x[1];
	mark_y[1] = scope_param[scope].view[view].calib.pixel.mark_y[1];
	//ポジション1とポジション2の中点を基準X,Yにする
	double cal_base_x = (mark_x[0] + mark_x[1]) / 2;
	double cal_base_y = (mark_y[0] + mark_y[1]) / 2;
	scope_param[scope].view[view].calib.pixel.cal_base_x = cal_base_x;
	scope_param[scope].view[view].calib.pixel.cal_base_y = cal_base_y;

	//
	//ポジション座標変換
	//
	double crdnt_x[2];
	double crdnt_y[2];
	int coordinate_type = scope_param[scope].view[view].coordinate_type;
	if (calc_conv.ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x[0], mark_y[0], &crdnt_x[0], &crdnt_y[0]) != 0) goto END;	//ポジション1
	scope_param[scope].view[view].calib.pixel.crdnt_x[0] = crdnt_x[0];
	scope_param[scope].view[view].calib.pixel.crdnt_y[0] = crdnt_y[0];
	if (calc_conv.ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x[1], mark_y[1], &crdnt_x[1], &crdnt_y[1]) != 0) goto END;	//ポジション2
	scope_param[scope].view[view].calib.pixel.crdnt_x[1] = crdnt_x[1];
	scope_param[scope].view[view].calib.pixel.crdnt_y[1] = crdnt_y[1];

	//
	//軸基準算出
	//
	double axis_x[2];
	double axis_y[2];
	axis_x[0] = scope_param[scope].view[view].calib.pixel.axis_x[0];
	axis_y[0] = scope_param[scope].view[view].calib.pixel.axis_y[0];
	axis_x[1] = scope_param[scope].view[view].calib.pixel.axis_x[1];
	axis_y[1] = scope_param[scope].view[view].calib.pixel.axis_y[1];
	//ポジション1とポジション2の中点を基準X,Yにする
	double axis_base_x = (axis_x[0] + axis_x[1]) / 2;
	double axis_base_y = (axis_y[0] + axis_y[1]) / 2;
	scope_param[scope].view[view].calib.pixel.axis_base_x = axis_base_x;
	scope_param[scope].view[view].calib.pixel.axis_base_y = axis_base_y;

	//
	//画素サイズと傾き算出
	//
	double pixel_size;
	double crdnt_radian;
	if (GetPixelSize(crdnt_x, crdnt_y, axis_x, axis_y, &pixel_size, &crdnt_radian) != 0) goto END;
	scope_param[scope].view[view].calib.pixel_size = pixel_size;
	scope_param[scope].view[view].calib.crdnt_radian = crdnt_radian;

	//ここから下はおそらくいらないコード
	// siya_pos_x,yの算出
	//double crdnt_siya_x;
	//double crdnt_siya_y;
	//calc_conv.ConvDirection(scope_param[scope].view[view].coordinate_type, cal_base_x, cal_base_y, 0, 0, &crdnt_siya_x, &crdnt_siya_y);	//左上(0,0)のXY座標系算出
	//double dx = crdnt_siya_x * pixel_size;
	//double dy = crdnt_siya_y * pixel_size;
	//double offset_x = cos(crdnt_radian) * dx - sin(crdnt_radian) * dy;
	//double offset_y = sin(crdnt_radian) * dx + cos(crdnt_radian) * dy;
	//scope_param[scope].view[view].calib.pixel.siya_pos_x = axis_base_x + offset_x;
	//scope_param[scope].view[view].calib.pixel.siya_pos_y = axis_base_y + offset_y;
	//ここまで

	//コマンドログ出力
	double pixel_um = pixel_size * 1000;
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "画素サイズ:", "Pixel size:") + Tips0.DblToString(pixel_um, 4, TRUE) + "um");	//コマンドログ 画素サイズ
	double crdnt_degree = Tips0.RadianToDeg(crdnt_radian);	//degreeに変換
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "視野傾き:", "View tilt:") + Tips0.DblToString(crdnt_degree, 4, TRUE) + "deg");	//コマンドログ 視野傾き

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 回転中心キャリブレーションのデータセット
 ** 引数：
 **　int scope	対象スコープ
 **　int view	対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::ThetaCalcDataSet(int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//データセット ポジション1
	if (scope_param[scope].view[view].calib.theta.enable[0]) {
		//回転中心Calポジション1のテンプレート番号を取得
		int template_number = scope_param[scope].view[view].calib.theta.template_number[0];
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
		//スコープデータへセット
		scope_param[scope].view[view].calib.theta.mark_x[0] = mark_x;
		scope_param[scope].view[view].calib.theta.mark_y[0] = mark_y;

		//軸位置
		double axis_x;		//軸位置X
		double axis_y;		//軸位置Y
		if (calc_plc.GetAxisPosXY(template_number, &axis_x, &axis_y) != 0) goto END;
		scope_param[scope].view[view].calib.theta.axis_x[0] = axis_x;
		scope_param[scope].view[view].calib.theta.axis_y[0] = axis_y;
		double axis_t;		//軸位置T
		if (calc_plc.GetAxisPosT(template_number, &axis_t) != 0) goto END;
		scope_param[scope].view[view].calib.theta.axis_t[0] = axis_t;
	}
	else {
		goto END;
	}

	//データセット ポジション2
	if (scope_param[scope].view[view].calib.theta.enable[1]) {
		//回転中心Calポジション2のテンプレート番号を取得
		int template_number = scope_param[scope].view[view].calib.theta.template_number[1];
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
		//スコープデータへセット
		scope_param[scope].view[view].calib.theta.mark_x[1] = mark_x;
		scope_param[scope].view[view].calib.theta.mark_y[1] = mark_y;

		//軸位置
		double axis_x;		//軸位置X
		double axis_y;		//軸位置Y
		if (calc_plc.GetAxisPosXY(template_number, &axis_x, &axis_y) != 0) goto END;
		scope_param[scope].view[view].calib.theta.axis_x[1] = axis_x;
		scope_param[scope].view[view].calib.theta.axis_y[1] = axis_y;
		double axis_t;		//軸位置T
		if (calc_plc.GetAxisPosT(template_number, &axis_t) != 0) goto END;
		scope_param[scope].view[view].calib.theta.axis_t[1] = axis_t;
	}
	else {
		goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 回転中心キャリブレーションの演算実行
 ** 引数：
 **　int scope	対象スコープ
 **　int view	対象視野
 **　byte base_method	機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::ThetaCalcExec(int scope, int view, byte base_method)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	//
	//Cal検出基準算出
	//
	double mark_x[2];
	double mark_y[2];
	mark_x[0] = scope_param[scope].view[view].calib.theta.mark_x[0];
	mark_y[0] = scope_param[scope].view[view].calib.theta.mark_y[0];
	mark_x[1] = scope_param[scope].view[view].calib.theta.mark_x[1];
	mark_y[1] = scope_param[scope].view[view].calib.theta.mark_y[1];

	//
	//ポジション座標変換
	//
	double crdnt_x[2];
	double crdnt_y[2];
	double cal_base_x = scope_param[scope].view[view].calib.pixel.cal_base_x;	//画素Cal基準Xを使用
	double cal_base_y = scope_param[scope].view[view].calib.pixel.cal_base_y;	//画素Cal基準Yを使用
	int coordinate_type = scope_param[scope].view[view].coordinate_type;
	if (calc_conv.ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x[0], mark_y[0], &crdnt_x[0], &crdnt_y[0]) != 0) goto END;	//ポジション1
	scope_param[scope].view[view].calib.theta.crdnt_x[0] = crdnt_x[0];
	scope_param[scope].view[view].calib.theta.crdnt_y[0] = crdnt_y[0];
	if (calc_conv.ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x[1], mark_y[1], &crdnt_x[1], &crdnt_y[1]) != 0) goto END;	//ポジション2
	scope_param[scope].view[view].calib.theta.crdnt_x[1] = crdnt_x[1];
	scope_param[scope].view[view].calib.theta.crdnt_y[1] = crdnt_y[1];

	//
	//軸基準算出
	//
	double axis_x[2];
	double axis_y[2];
	double axis_t[2];
	axis_x[0] = scope_param[scope].view[view].calib.theta.axis_x[0];	//とりあえず軸現在位置読み込み(カメラ位置読み込みの場合は違う)
	axis_y[0] = scope_param[scope].view[view].calib.theta.axis_y[0];	//とりあえず軸現在位置読み込み(カメラ位置読み込みの場合は違う)
	axis_t[0] = scope_param[scope].view[view].calib.theta.axis_t[0];
	axis_x[1] = scope_param[scope].view[view].calib.theta.axis_x[1];	//とりあえず軸現在位置読み込み(カメラ位置読み込みの場合は違う)
	axis_y[1] = scope_param[scope].view[view].calib.theta.axis_y[1];	//とりあえず軸現在位置読み込み(カメラ位置読み込みの場合は違う)
	axis_t[1] = scope_param[scope].view[view].calib.theta.axis_t[1];

	//
	//メカ座標変換
	//
	double mech_x[2];
	double mech_y[2];
	double pixel_size = scope_param[scope].view[view].calib.pixel_size;
	double crdnt_radian = scope_param[scope].view[view].calib.crdnt_radian;
	if (calc_conv.ConvMechCoord(crdnt_x[0], crdnt_y[0], axis_x[0], axis_y[0], pixel_size, crdnt_radian, &mech_x[0], &mech_y[0]) != 0) goto END;
	scope_param[scope].view[view].calib.theta.mech_x[0] = mech_x[0];
	scope_param[scope].view[view].calib.theta.mech_y[0] = mech_y[0];
	if (calc_conv.ConvMechCoord(crdnt_x[1], crdnt_y[1], axis_x[1], axis_y[1], pixel_size, crdnt_radian, &mech_x[1], &mech_y[1]) != 0) goto END;
	scope_param[scope].view[view].calib.theta.mech_x[1] = mech_x[1];
	scope_param[scope].view[view].calib.theta.mech_y[1] = mech_y[1];

	//
	//回転中心算出
	//
	double relative_degree;
	double rotation_center_x;
	double rotation_center_y;
	relative_degree = fabs(axis_t[1] - axis_t[0]);
	if (GetRotCenter(mech_x, mech_y, relative_degree, &rotation_center_x, &rotation_center_y) != 0) goto END;
	scope_param[scope].view[view].calib.theta.rotation_center_x = rotation_center_x;
	scope_param[scope].view[view].calib.theta.rotation_center_y = rotation_center_y;
	scope_param[scope].conf_rotcent_x = rotation_center_x;	//回転中心X 確定データ
	scope_param[scope].conf_rotcent_y = rotation_center_y;	//回転中心Y 確定データ
	scope_param[scope].conf_rotcent_view = view;			//回転中心Cal 実行視野


	//ここから下はおそらくいらないコード
	// siya_pos_x,yの算出
//	double crdnt_siya_x;
//	double crdnt_siya_y;
//	ConvDirection(scope_param[scope].view[view].coordinate_type, 0, 0, 0, 0, &crdnt_siya_x, &crdnt_siya_y);	//左上(0,0)のXY座標系算出
//	double dx = crdnt_siya_x * pixel_size;
//	double dy = crdnt_siya_y * pixel_size;
//	double offset_x = cos(crdnt_degree) * dx - sin(crdnt_degree) * dy;
//	double offset_y = sin(crdnt_degree) * dx + cos(crdnt_degree) * dy;
//	scope_param[scope].view[view].calib.pixel.siya_pos_x = axis_base_x + offset_x;
//	scope_param[scope].view[view].calib.pixel.siya_pos_y = axis_base_y + offset_y;
	//ここまで

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "回転中心X:", "Rotation center X:") + Tips0.DblToString(rotation_center_x, 4, TRUE) + "mm");	//コマンドログ 回転中心X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "回転中心Y:", "Rotation center Y:") + Tips0.DblToString(rotation_center_y, 4, TRUE) + "mm");	//コマンドログ 回転中心X

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 経時変化基準指定のデータセット
 ** 引数：
 **　int template_number	テンプレートNo
 **　int scope	対象スコープ
 **　int view	対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::VariationDataSetBase(int template_number, int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

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

	//経時変化補正パラメータへセット
	scope_param[scope].view[view].move.base_mark_x = mark_x;
	scope_param[scope].view[view].move.base_mark_y = mark_y;
	scope_param[scope].view[view].move.move_mark_x = 0;
	scope_param[scope].view[view].move.move_mark_y = 0;
	scope_param[scope].view[view].move.displacement_x = 0;
	scope_param[scope].view[view].move.displacement_y = 0;

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "経時変化基準点指定 X:", "BasePos X:") + Tips0.DblToString(mark_x, 1, TRUE));	//コマンドログ 基準点X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "経時変化基準点指定 Y:", "BasePos Y:") + Tips0.DblToString(mark_y, 1, TRUE));	//コマンドログ 基準点X

	rtn_result = 0;

END:
	return rtn_result;
}




/** ****************************************************** **
 ** 経時変化補正算出のデータセット
 ** 引数：
 **　int template_number	テンプレートNo
 **　int scope	対象スコープ
 **　int view	対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::VariationDataSetMove(int template_number, int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

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

	//経時変化補正パラメータへセット
	scope_param[scope].view[view].move.move_mark_x = mark_x;
	scope_param[scope].view[view].move.move_mark_y = mark_y;
	scope_param[scope].view[view].move.displacement_x = 0;
	scope_param[scope].view[view].move.displacement_y = 0;

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "経時変化点 X:", "VariationPos X:") + Tips0.DblToString(mark_x, 1, TRUE));	//コマンドログ 変化点X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "経時変化点 Y:", "VariationPos Y:") + Tips0.DblToString(mark_y, 1, TRUE));	//コマンドログ 変化点X

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 経時変化補正算出の演算実行
 ** 引数：
 **　int scope			対象スコープ
 **　int view			対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::VariationCalcExec(int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	//
	//変化量算出
	//
	double displacement_x;
	double displacement_y;
	displacement_x = scope_param[scope].view[view].move.move_mark_x - scope_param[scope].view[view].move.base_mark_x;
	displacement_y = scope_param[scope].view[view].move.move_mark_y - scope_param[scope].view[view].move.base_mark_y;

	//変化量セット
	scope_param[scope].view[view].move.displacement_x = displacement_x;
	scope_param[scope].view[view].move.displacement_y = displacement_y;

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "変化量X:", ":Variation X:") + Tips0.DblToString(displacement_x, 1, TRUE) + "pix");	//コマンドログ 変化量X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "変化量Y:", ":Variation Y:") + Tips0.DblToString(displacement_y, 1, TRUE) + "pix");	//コマンドログ 変化量Y

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 経時変化補正算出値のリセット
 ** 引数：
 **　int scope			対象スコープ
 **　int view			対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::VariationCalcReset(int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int rtn_result = 1;

	//
	//変化量算出
	//
	double displacement_x;
	double displacement_y;
	displacement_x = 0;
	displacement_y = 0;

	//変化量セット
	scope_param[scope].view[view].move.displacement_x = displacement_x;
	scope_param[scope].view[view].move.displacement_y = displacement_y;

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正値リセット X:", ":Reset X:") + Tips0.DblToString(displacement_x, 1, TRUE) + "pix");	//コマンドログ リセット X
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "補正値リセット Y:", ":Reset Y:") + Tips0.DblToString(displacement_y, 1, TRUE) + "pix");	//コマンドログ リセット Y

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 画素サイズと傾き算出
 ** 引数：
 **　double* crdnt_x		座標変換X(座標変換X配列の先頭アドレス)
 **　double* crdnt_y		座標変換Y(座標変換Y配列の先頭アドレス)
 **　double* axis_x			軸位置X(軸位置X配列の先頭アドレス)
 **　double* axis_y			軸位置Y(軸位置Y配列の先頭アドレス)
 **　double* pixel_size		画素サイズ
 **　double* crdnt_radian	座標系傾き
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::GetPixelSize(double* crdnt_x, double* crdnt_y, double* axis_x, double* axis_y, double* pixel_size, double* crdnt_radian)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	double axis_width;
	double x_width;
	double y_width;

	if (crdnt_x[0] == crdnt_x[1] && crdnt_y[0] == crdnt_y[1]) {
		//異常系
		goto END;
	}

	//X軸(視野位置X)のみ移動
	if (axis_x[0] != axis_x[1] && axis_y[0] == axis_y[1]) {
		//軸移動量
		axis_width = fabs(axis_x[0] - axis_x[1]);
		//検出位置移動量
		x_width = fabs(crdnt_x[0] - crdnt_x[1]);
		y_width = fabs(crdnt_y[0] - crdnt_y[1]);

		//画素サイズ算出
		*pixel_size = axis_width / sqrt(x_width * x_width + y_width * y_width);

		//傾き算出
		//*crdnt_radian = atan(((crdnt_y[1] - crdnt_y[0]) * -1) / (crdnt_x[1] - crdnt_x[0]));	//確認すべし(Y軸のみ移動と算出違う)
		*crdnt_radian = atan(((crdnt_y[1] - crdnt_y[0])) / (crdnt_x[1] - crdnt_x[0]));
	}
	//Y軸(視野位置Y)のみ移動
	else if (axis_x[0] == axis_x[1] && axis_y[0] != axis_y[1]) {
		//軸移動量
		axis_width = fabs(axis_y[0] - axis_y[1]);

		//検出位置移動量
		x_width = fabs(crdnt_x[0] - crdnt_x[1]);
		y_width = fabs(crdnt_y[0] - crdnt_y[1]);

		//画素サイズ算出
		*pixel_size = axis_width / sqrt(x_width * x_width + y_width * y_width);

		//傾き算出
		*crdnt_radian = atan((crdnt_x[0] - crdnt_x[1]) / (crdnt_y[1] - crdnt_y[0]));
	}
	else {
		//異常系
		goto END;
	}

	rtn_result = 0;	//算出成功

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 回転中心算出
 ** 引数：
 **　double* mech_x				メカ座標X(メカ座標X配列の先頭アドレス)
 **　double* mech_y				メカ座標Y(メカ座標Y配列の先頭アドレス)
 **　double* deg				移動量T
 **　double* rotation_center_x	回転中心X
 **　double* rotation_center_y	回転中心Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::GetRotCenter(double* mech_x, double* mech_y, double deg, double* rotation_center_x, double* rotation_center_y)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	int rtn_result = 1;
	if (deg == 0) goto END;

	double relative_radian;
	double sita2;
	double sita3;
	double pd;
	double pq;
	double pr;
	double dr;
	double temp1;
	double RX;
	double RY;
	double No1, No2, No3, No4;

	//Nolb
	relative_radian = Tips0.DegToRadian(deg);
	//sita2 = (3.1415926535 - relative_radian) / 2;
	sita2 = (M_PI - relative_radian) / 2;
	sita3 = fabs(atan((mech_y[1] - mech_y[0]) / (mech_x[1] - mech_x[0])));

	pq = sqrt((mech_x[1] - mech_x[0]) * (mech_x[1] - mech_x[0]) + (mech_y[1] - mech_y[0]) * (mech_y[1] - mech_y[0]));
	pd = pq / 2;
	pr = pd / cos(sita2);

	temp1 = fabs(pr * pr - pd * pd);
	dr = sqrt(temp1);

	No1 = dr * sin(sita3);
	No2 = pd * cos(sita3);
	No3 = dr * cos(sita3);
	No4 = pd * sin(sita3);

	if ((mech_x[0] < mech_x[1]) && (mech_y[0] < mech_y[1])) {
		//第1象限
		RX = No1 - No2;
		RY = No3 + No4;

		*rotation_center_x = mech_x[0] - RX;
		*rotation_center_y = mech_y[0] + RY;
	}
	else if ((mech_x[0] < mech_x[1]) && (mech_y[0] > mech_y[1])) {
		//第2象限
		RX = No1 + No2;
		RY = No3 - No4;

		*rotation_center_x = mech_x[0] + RX;
		*rotation_center_y = mech_y[0] + RY;
	}
	else if ((mech_x[0] > mech_x[1]) && (mech_y[0] < mech_y[1])) {
		//第3象限
		RX = No1 + No2;
		RY = No3 - No4;

		*rotation_center_x = mech_x[0] - RX;
		*rotation_center_y = mech_y[0] - RY;
	}
	else if ((mech_x[0] > mech_x[1]) && (mech_y[0] > mech_y[1])) {
		//第4象限
		RX = No1 - No2;
		RY = No3 + No4;

		*rotation_center_x = mech_x[0] + RX;
		*rotation_center_y = mech_y[0] - RY;
	}
	else {
		AfxMessageBox("Wrong result in Rot Center Cal");	//回転中心演算にて予期せぬ演算結果
		goto END;
	}

	rtn_result = 0;	//算出成功

	//デバック 20200409
	double a = sqrt(pow(mech_x[1] - mech_x[0], 2.0) + pow(mech_y[1] - mech_y[0], 2.0)) / (double)2;
	double r = a / sin(relative_radian / (double)2);
	double alpha_rad = atan(fabs(mech_x[1] - mech_x[0]) / fabs(mech_y[1] - mech_y[0]));
	double beta_rad = (relative_radian / (double)2) + alpha_rad;
	double x1_dash = r * cos(beta_rad);
	double y1_dash = r * sin(beta_rad);

	//double x_center = mech_x[0] - x1_dash;
	//double y_center = mech_y[0] + y1_dash;
	double x_center = mech_x[1] + x1_dash;
	double y_center = mech_y[1] + y1_dash;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 1スコープ1視野へコピー
 ** 引数：
 **　byte cmd	実行コマンド
 **　int scope	対象スコープ
 **　int View	対象視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CopyOne(byte cmd, int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int scope_copy = cmd_param[cmd].calib.copy_scope;
	int view_copy = cmd_param[cmd].calib.copy_view;
	if (scope_copy < 0 && scope_copy > 15) return 1;
	if (view_copy < 0 && view_copy > 3) return 1;

	//画素キャリブレーションデータコピー
	if (cmd_param[cmd].search.method == 0x0307) {
		if (CopyPixelCalc(scope, view, scope_copy, view_copy) == 0) {
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
				Tips0.IntToString(scope_copy + 1) +
				Tips0.SelStr(language.jpn, "、視野", ", View") +
				Tips0.IntToString(view_copy + 1));	//コマンドログ データコピー
		}
	}
	//回転中心キャリブレーションデータコピー
	else if (cmd_param[cmd].search.method == 0x0309) {
		if (CopyThetaCalc(scope, view, scope_copy, view_copy) == 0) {
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
				Tips0.IntToString(scope_copy + 1) +
				Tips0.SelStr(language.jpn, "、視野", ", View") +
				Tips0.IntToString(view_copy + 1));	//コマンドログ データコピー
		}
	}
	//経時変化補正データコピー
	else if (cmd_param[cmd].search.method == 0x0310 || cmd_param[cmd].search.method == 0x0311 || cmd_param[cmd].search.method == 0x0312) {
		if (CopyMoveCalc(scope, view, scope_copy, view_copy) == 0) {
			outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
				Tips0.IntToString(scope_copy + 1) +
				Tips0.SelStr(language.jpn, "、視野", ", View") +
				Tips0.IntToString(view_copy + 1));	//コマンドログ データコピー
		}
	}

	return 0;
}


/** ****************************************************** **
 ** 多スコープ多視野へコピー
 ** 引数：
 **　byte cmd	実行コマンド
 **　int scope	対象スコープ
 **　int View	対象視野
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CopyCustom(byte cmd, int scope, int view)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			//コピー実施
			if (cmd_param[cmd].calib.custom_copy_scope[i].view[j]) {

				//画素キャリブレーションデータコピー
				if (cmd_param[cmd].search.method == 0x0307) {
					if (CopyPixelCalc(scope, view, i, j) == 0) {
						outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
							Tips0.IntToString(i + 1) +
							Tips0.SelStr(language.jpn, "、視野", ", View") +
							Tips0.IntToString(j + 1));	//コマンドログ データコピー
					}
				}
				//回転中心キャリブレーションデータコピー
				else if (cmd_param[cmd].search.method == 0x0309) {
					if (CopyThetaCalc(scope, view, i, j) == 0) {
						outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
							Tips0.IntToString(i + 1) +
							Tips0.SelStr(language.jpn, "、視野", ", View") +
							Tips0.IntToString(j + 1));	//コマンドログ データコピー
					}
				}
				//経時変化補正データコピー
				else if (cmd_param[cmd].search.method == 0x0310 || cmd_param[cmd].search.method == 0x0311 || cmd_param[cmd].search.method == 0x0312) {
					if (CopyMoveCalc(scope, view, i, j) == 0) {
						outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "データコピー:スコープ", "Data copy:Scope") +
							Tips0.IntToString(i + 1) +
							Tips0.SelStr(language.jpn, "、視野", ", View") +
							Tips0.IntToString(j + 1));	//コマンドログ データコピー
					}
				}
			}
		}
	}

	return 0;
}


/** ****************************************************** **
 ** 画素キャリブレーションコピー
 ** 引数：
 **　int scope_org		コピー元スコープ
 **　int view_org		コピー元視野
 **　int scope_copy		コピー先スコープ
 **　int view_copy		コピー先視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CopyPixelCalc(int scope_org, int view_org, int scope_copy, int view_copy)
{
	// TODO: ここに実装コードを追加します.
	if ((scope_org == scope_copy) && (view_org == view_copy)) return 1;

	CScopeParam::PixelCalibParam *param_org = &scope_param[scope_org].view[view_org].calib.pixel;		//コピー元
	CScopeParam::PixelCalibParam *param_copy = &scope_param[scope_copy].view[view_copy].calib.pixel;	//コピー先

	//ポジション1,2データコピー
	for (int i = 0; i < 2; i++) {
		param_copy->enable[i] = TRUE;
		param_copy->template_number[i] = param_org->template_number[i];

		param_copy->axis_x[i] = param_org->axis_x[i];	//軸位置X
		param_copy->axis_y[i] = param_org->axis_y[i];	//軸位置Y
		param_copy->mark_x[i] = param_org->mark_x[i];	//検出位置X
		param_copy->mark_y[i] = param_org->mark_y[i];	//検出位置Y
		param_copy->crdnt_x[i] = param_org->crdnt_x[i];	//座標変換X Cal基準を0としカメラXY座標系
		param_copy->crdnt_y[i] = param_org->crdnt_y[i];	//座標変換Y Cal基準を0としカメラXY座標系
	}
	param_copy->axis_base_x = param_org->axis_base_x;	//軸基準X(Cal基準での軸X)
	param_copy->axis_base_y = param_org->axis_base_y;	//軸基準Y(Cal基準での軸Y)
	param_copy->cal_base_x = param_org->cal_base_x;		//Cal基準X(画像内のCal中心X)
	param_copy->cal_base_y = param_org->cal_base_y;		//Cal基準Y(画像内のCal中心Y)

	//画素サイズ
	scope_param[scope_copy].view[view_copy].calib.pixel_size = scope_param[scope_org].view[view_org].calib.pixel_size;
	//座標系傾き
	scope_param[scope_copy].view[view_copy].calib.crdnt_radian = scope_param[scope_org].view[view_org].calib.crdnt_radian;

	//カメラ座標の構成タイプ
	scope_param[scope_copy].view[view_copy].coordinate_type = scope_param[scope_org].view[view_org].coordinate_type;

	return 0;
}


/** ****************************************************** **
 ** 回転中心キャリブレーションコピー
 ** 引数：
 **　int scope_org		コピー元スコープ
 **　int view_org		コピー元視野
 **　int scope_copy		コピー先スコープ
 **　int view_copy		コピー先視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CopyThetaCalc(int scope_org, int view_org, int scope_copy, int view_copy)
{
	// TODO: ここに実装コードを追加します.
	if ((scope_org == scope_copy) && (view_org == view_copy)) return 1;

	CScopeParam::ThetalCalibParam *param_org = &scope_param[scope_org].view[view_org].calib.theta;		//コピー元
	CScopeParam::ThetalCalibParam *param_copy = &scope_param[scope_copy].view[view_copy].calib.theta;	//コピー先

	//ポジション1,2データコピー
	for (int i = 0; i < 2; i++) {
		param_copy->enable[i] = TRUE;
		param_copy->template_number[i] = param_org->template_number[i];

		param_copy->axis_x[i] = param_org->axis_x[i];	//軸位置X
		param_copy->axis_y[i] = param_org->axis_y[i];	//軸位置Y
		param_copy->axis_t[i] = param_org->axis_t[i];	//軸位置T
		param_copy->mark_x[i] = param_org->mark_x[i];	//検出位置X
		param_copy->mark_y[i] = param_org->mark_y[i];	//検出位置Y
		param_copy->crdnt_x[i] = param_org->crdnt_x[i];	//座標変換X カメラXY座標系
		param_copy->crdnt_y[i] = param_org->crdnt_y[i];	//座標変換Y カメラXY座標系
		param_copy->mech_x[i] = param_org->mech_x[i];	//メカ座標X
		param_copy->mech_y[i] = param_org->mech_y[i];	//メカ座標Y
	}
	param_copy->rotation_center_x = param_org->rotation_center_x;	//回転中心X
	param_copy->rotation_center_y = param_org->rotation_center_y;	//回転中心Y

	scope_param[scope_copy].conf_rotcent_x = scope_param[scope_org].conf_rotcent_x;			//回転中心X 確定データ
	scope_param[scope_copy].conf_rotcent_y = scope_param[scope_org].conf_rotcent_y;			//回転中心Y 確定データ
	scope_param[scope_copy].conf_rotcent_view = view_copy;		//回転中心Cal 実行視野 ※コピー元の視野ではなくコピー先の視野とする

	return 0;
}


/** ****************************************************** **
 ** 経時変化補正コピー
 ** 引数：
 **　int scope_org		コピー元スコープ
 **　int view_org		コピー元視野
 **　int scope_copy		コピー先スコープ
 **　int view_copy		コピー先視野
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcCalib::CopyMoveCalc(int scope_org, int view_org, int scope_copy, int view_copy)
{
	// TODO: ここに実装コードを追加します.
	if ((scope_org == scope_copy) && (view_org == view_copy)) return 1;

	CScopeParam::MoveParameter* param_org = &scope_param[scope_org].view[view_org].move;		//コピー元
	CScopeParam::MoveParameter* param_copy = &scope_param[scope_copy].view[view_copy].move;		//コピー先

	//経時変化補正データコピー
	param_copy->base_mark_x = param_org->base_mark_x;
	param_copy->base_mark_y = param_org->base_mark_y;
	param_copy->move_mark_x = param_org->move_mark_x;
	param_copy->move_mark_y = param_org->move_mark_y;
	param_copy->displacement_x = param_org->displacement_x;
	param_copy->displacement_y = param_org->displacement_y;

	return 0;
}


/** ****************************************************** **
 ** 書込データセット
 ** 引数：
 **　byte cmd	実行コマンド
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcCalib::SetOutData(byte cmd)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int write_type = cmd_param[cmd].write.type;				//データ書込設定

	//
	//演算結果のデータ出力
	//
	byte base_method = cmd_param[cmd].base_method;
	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	WORD method = cmd_param[cmd].search.method;

	//画素キャリブレーション結果のデータ出力
	if (method == 0x0306 || method == 0x0307) {
		double pixel_size = scope_param[scope].view[view].calib.pixel_size;
		double crdnt_radian = scope_param[scope].view[view].calib.crdnt_radian;

		calc_plc.PixelCalcDataOut(cmd, scope, base_method, pixel_size, crdnt_radian);
	}
	//回転中心キャリブレーション結果のデータ出力
	else if (method == 0x0308 || method == 0x0309) {
		double rotation_center_x = scope_param[scope].conf_rotcent_x;
		double rotation_center_y = scope_param[scope].conf_rotcent_y;

		calc_plc.ThetaCalcDataOut(cmd, scope, base_method, rotation_center_x, rotation_center_y);
	}
	//経時変化結果のデータ出力
	else if (method == 0x0310 || method == 0x0311 || method == 0x0312) {
		double displacement_x = scope_param[scope].view[view].move.displacement_x;
		double displacement_y = scope_param[scope].view[view].move.displacement_y;

		calc_plc.VariationDataOut(cmd, scope, base_method, displacement_x, displacement_y);
	}
}
