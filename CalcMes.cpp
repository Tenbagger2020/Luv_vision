/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcMes.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	検査結果出力追加、言語切替修正
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2023.11.08	M.Iwasaki	10.15.1.1	画像合成機能追加
 **			2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 **			2024.07.16	M.Iwasaki	10.18.4.6	ブロブ機能拡張
 ** 注記：
 ** 説明：計測モード演算プログラム
 **
 **/

#include "stdafx.h"
#include "CalcMes.h"

#include "DlgSystemMain.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Scopes.h"				//システムスコープパラメータ
#include "CameraSettings.h"		//カメラパラメータ
#include "Commands.h"			//コマンドパラメータ
#include "MesParam.h"			//機種毎検査パラメータ
#include "ExcParamAlignBlob.h"		//AlignBlobパラメータ変換
#include "ExcParamAlignEdge.h"		//AlignEdgeパラメータ変換
#include "ExcParamAlignMeas.h"		//AlignMeasパラメータ変換
#include "ExcParamAlignMultiSrh.h"	//AlignMultiSrhパラメータ変換
#include "ExcParamAlignDisEdge.h"	//AlignDisEdgeパラメータ変換
#include "ExcParamAlignDisEdgeToEdge.h"	//AlignDisEdgeToEdgeパラメータ変換

#include "CalcMark.h"			//サーチ結果取得
#include "CalcPLC.h"			//PLCデータ取得
#include "ProcOutResult.h"		//結果出力処理クラス
#include "ProcCopyImage.h"		//画像コピー処理クラス
#include "CalcFormula.h"			//計算演算クラス
#include "ProcOutResultFormula.h"	//計算結果出力処理クラス

#include "CurrentStatus.h"		//現状共通データ
#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

//DLL
#include "AlignBlob.h"	//AlignBlob DLL Header
#include "AlignEdge.h"	//AlignEdge DLL Header
#include "AlignMeas.h"	//AlignMeas DLL Header
#include "AlignMultiSrh.h"	//AlignMultiSrh DLL Header
#include "AlignDisEdge.h"	//AlignDisEdge DLL Header
#include "AlignDisEdgeToEdge.h"	//AlignDisEdgeToEdge DLL Header

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CScopes			sys_scope[16];			//システムスコープパラメータ
extern CCameraSettings	cam_parameter[8];		//カメラパラメータ
extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ

extern CCalcMark		calc_mark;		//サーチ結果取得
extern CCalcPLC			calc_plc;		//PLCデータ取得
extern CProcOutResult*	proc_out_rlt;	//結果出力処理クラス
extern CProcCopyImage*	proc_cpyimg;	//画像コピー処理クラス
extern CCalcFormula		calc_formula;	//計算演算

extern CCurrentStatus	current;		//共有情報
extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

//DLL
extern AlignBlob::Proc::CCalcBlob	calc_blob;		//ブロブ演算
extern AlignEdge::Proc::CCalcEdge	calc_edge;		//エッジ演算
extern AlignMeas::Proc::CCalcMeas	calc_meas;		//濃度演算
extern AlignMultiSrh::Proc::CCalcMultiSrh	calc_multisrh;		//マルチ探索演算
extern AlignDisEdge::Proc::CCalcDisEdge		calc_disedge;		//基準-エッジ距離計測演算
extern AlignDisEdgeToEdge::Proc::CCalcDisEdgeToEdge	calc_disetoe;	//エッジ-エッジ距離計測演算

CCalcMes::CCalcMes()
{
}


CCalcMes::~CCalcMes()
{
}


/** ****************************************************** **
 ** 計測モードメイン
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcMes::CalcMain(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	int rtn_result = 1;
	CString str_ok;

	byte cmd = dlg->run_cmd;	//実行コマンド
	byte base_method = cmd_param[cmd].base_method;	//機能種別

	//対象視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	//使用するパラメータ
	CTempParam* temp_param = &template_param[template_number];	//テンプレートパラメータ
	CMesParam* param = &mes_param[template_number];				//検査パラメータ
	CScopeParam* scp_param = &scope_param[scope];				//スコープパラメータ

	//結果をリセット
	BOOL mes_ok[8];
	for (int i = 0; i < 8; i++) {
		mes_ok[i] = FALSE;

		//画像コピー結果を初期化
		temp_param->copy_result[i].st_x = 0;
		temp_param->copy_result[i].st_y = 0;
		temp_param->copy_result[i].width = 0;
		temp_param->copy_result[i].height = 0;
	}

	//計測モード実施
	if (cmd_param[cmd].search.method == 0x0201) {

		//計測モードのデータセット
		if (param->base_search_enable) {
			int parts = cmd_param[cmd].align.parts;	//アライメント視野数

			if (CalcDataSet(temp_param, scp_param, parts) != 0) {
				dlg->thread_rs.calc_err = 0x9421;	//エラーコード
				goto END;
			}
		}

		//計測モードの演算実行
		for (int i = 0; i < 8; i++) {
			//検査機能を確認
			int mes_type = param->sheet[i].mes_type;

			//検査を実行
			switch (mes_type) {
				//無し
			case 0x00:
				mes_ok[i] = TRUE;
				break;
				//ブロブ
			case 0x01:
			{
				FVCL::Blob::CFvBlobResult* blob_result = &temp_param->blob_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignBlob exc_param;
				AlignBlob::Param::CMesParamBlob dll_param_mes;		//検査パラメータ
				exc_param.SetBlobMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
				BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
				//キャリブレーション未完了の場合
				if (!pixel_complete) {
					pixel_size = 0;		//画素サイズ0
				}

				//ブロブ検査の演算実行
				if (calc_blob.CalcMain(m_Image, blob_result, &dll_param_mes, base_mark_x, base_mark_y, i, pixel_size, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9422;	//エラーコード
					goto END;
				}
			}
				break;
				//エッジ
			case 0x02:
			{
				FVCL::Edge::CFvEdgeResult* edge_result = &temp_param->edge_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignEdge exc_param;
				AlignEdge::Param::CMesParamEdge dll_param_mes;
				exc_param.SetEdgeMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				//エッジ検査の演算実行
				if (calc_edge.CalcMain(m_Image, edge_result, &dll_param_mes, base_mark_x, base_mark_y, i, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9423;	//エラーコード
					goto END;
				}
			}
				break;
				//濃度
			case 0x03:
			{
				double* meas_result = &temp_param->meas_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignMeas exc_param;
				AlignMeas::Param::CMesParamMeas dll_param_mes;
				exc_param.SetMeasMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				//濃度検査の演算実行
				if (calc_meas.CalcMain(m_Image, meas_result, &dll_param_mes, base_mark_x, base_mark_y, i, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9424;	//エラーコード
					goto END;
				}
			}
				break;
				//マルチ探索
			case 0x04:
			{
				AlignMultiSrh::Param::CMultiSrhResult* multisrh_result = &temp_param->multisrh_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignMultiSrh exc_param;
				AlignMultiSrh::Param::CMesParamMultiSrh dll_param_mes;
				exc_param.SetMultiSrhMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				//マルチ探索検査の演算実行
				if (calc_multisrh.CalcMain(m_Image, multisrh_result, &dll_param_mes, base_mark_x, base_mark_y, i, template_number, current.work_number, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9425;	//エラーコード
					goto END;
				}
			}
				break;
				//基準-エッジ距離計測
			case 0x05:
			{
				AlignDisEdge::Param::CDisEdgeResult* disedge_result = &temp_param->disedge_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignDisEdge exc_param;
				AlignDisEdge::Param::CMesParamDisEdge dll_param_mes;
				exc_param.SetDisEdgeMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				int assign_camera = sys_scope[scope].view[view].assign_camera;		//割り当てカメラ番号 0～7
				int coordinate_type = cam_parameter[assign_camera].coordinate_type;	//視野座標の構成タイプ
				double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
				double crdnt_radian = scp_param->view[view].calib.crdnt_radian;		//座標系傾き(radian)
				BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
				//キャリブレーション未完了の場合
				if (!pixel_complete) {
					pixel_size = 0;		//画素サイズ0
					crdnt_radian = 0;	//座標系傾き0
				}

				//基準-エッジ距離計測検査の演算実行
				if (calc_disedge.CalcMain(m_Image, disedge_result, &dll_param_mes, base_mark_x, base_mark_y, i, coordinate_type, pixel_size, crdnt_radian, pixel_complete, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9426;	//エラーコード
					goto END;
				}
			}
				break;
				//エッジ-エッジ距離計測
			case 0x06:
			{
				AlignDisEdgeToEdge::Param::CDisEdgeToEdgeResult* disetoe_result = &temp_param->disetoe_result[i];
				FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

				CExcParamAlignDisEdgeToEdge exc_param;
				AlignDisEdgeToEdge::Param::CMesParamDisEdgeToEdge dll_param_mes;
				exc_param.SetDisEdgeToEdgeMes(param, &dll_param_mes, i);

				double base_mark_x = scp_param->view[0].align.mark_x;
				double base_mark_y = scp_param->view[0].align.mark_y;

				int assign_camera = sys_scope[scope].view[view].assign_camera;		//割り当てカメラ番号 0～7
				int coordinate_type = cam_parameter[assign_camera].coordinate_type;	//視野座標の構成タイプ
				double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
				double crdnt_radian = scp_param->view[view].calib.crdnt_radian;		//座標系傾き(radian)
				BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
				//キャリブレーション未完了の場合
				if (!pixel_complete) {
					pixel_size = 0;		//画素サイズ0
					crdnt_radian = 0;	//座標系傾き0
				}

				//エッジ-エッジ距離計測検査の演算実行
				if (calc_disetoe.CalcMain(m_Image, disetoe_result, &dll_param_mes, base_mark_x, base_mark_y, i, coordinate_type, pixel_size, crdnt_radian, pixel_complete, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9427;	//エラーコード
					goto END;
				}
			}
			break;
			//画像コピー(矩形)
			case 0x11:
			{
				//コピー先サイズリセット
				proc_cpyimg->SizeReset(temp_param, &temp_param->m_Image, i);

				//内部メモリへコピー
				proc_cpyimg->CopyToMem(temp_param, &temp_param->m_Image, param, i, &mes_ok[i], NULL);
			}
				break;
			//計算
			case 0x21:
			{
				//計算の演算実行
				if (calc_formula.CalcMain(param, temp_param, i, &mes_ok[i], NULL) == 1) {
					dlg->thread_rs.calc_err = 0x9429;	//エラーコード
					goto END;
				}
			}
			break;
			//その他(想定外)
			default:
				dlg->thread_rs.calc_err = 0x9429;	//エラーコード
				break;
			}
		}
	}
	//異常系(想定外コマンド)
	else {
		dlg->thread_rs.calc_err = 0x9429;	//エラーコード
		goto END;
	}

	//
	//検査結果のデータセット
	//
	SetResult(scope, base_method, mes_ok);

	//結果をセット
	dlg->thread_rs.mes_all = TRUE;
	for (int i = 0; i < 8; i++) {
		dlg->thread_rs.mes_ok[i] = mes_ok[i];
		if (!dlg->thread_rs.mes_ok[i]) dlg->thread_rs.mes_all = FALSE;	//総合判定
	}

	//検査結果出力
	if (temp_param->out_result.enable) OutResult(pParam);

	//コマンドログ出力
	str_ok = "NG";
	if (dlg->thread_rs.mes_all) str_ok = "OK";
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "総合判定 結果:", "Comprehensive Result:") + str_ok);	//コマンドログ 総合判定

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 計測モードのデータセット
 ** 引数：
 **　CTempParam* temp_param	テンプレートパラメータ
 **　CScopeParam* scp_param	スコープパラメータ
 **　int parts				アライメント視野数
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcMes::CalcDataSet(CTempParam* temp_param, CScopeParam* scp_param, int parts)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (parts != 1) goto END;	//視野数は"1"固定のため1以外はエラー

	for (int i = 0; i < parts; i++) {
		//対象スコープのテンプレート番号を取得
		//　検査では視野数1でスコープパラメータに入るテンプレート番号も自身と同じなので
		//　スコープパラメータからもらうのをやめる
		//int template_number = scope_param[scope].view[i].align.template_number;

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
		scp_param->view[i].align.mark_x = mark_x;
		scp_param->view[i].align.mark_y = mark_y;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 演算結果データセット
 ** 引数：
 **　int scope				対象スコープ
 **　byte base_method		機能種別
 **　BOOL* mes_ok			シート0～7の検査結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcMes::SetResult(int scope, byte base_method, BOOL* mes_ok)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	for (int i = 0; i < 8; i++) {
		scope_param[scope].result.mes.mes_ok[i] = mes_ok[i];
	}

	scope_param[scope].result.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** 書込データセット
 ** 引数：
 **　byte cmd	実行コマンド
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcMes::SetOutData(byte cmd)
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

	calc_plc.MesCalcDataOut(cmd, scope, base_method, scope_param[scope].result.mes.mes_ok);
}


/** ****************************************************** **
 ** 検査結果出力
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcMes::OutResult(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	byte cmd = dlg->run_cmd;	//実行コマンド
	byte base_method = cmd_param[cmd].base_method;	//機能種別

	//対象視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	//使用するパラメータ
	CTempParam* temp_param = &template_param[template_number];	//テンプレートパラメータ
	CMesParam* param = &mes_param[template_number];				//検査パラメータ
	CScopeParam* scp_param = &scope_param[scope];				//スコープパラメータ

	//結果出力パラメータ
	CTempParam::OutResult* out_result = &temp_param->out_result;

	//データ保存しない確認
	if (!out_result->enable_result) {	//データ保存無効
		return;
	}

	//総合判定
	BOOL judge = dlg->thread_rs.mes_all;

	//基準点探索結果保存
	BOOL srch_ok = scp_param->view[view].align.srch_enable;
	if (param->base_search_enable) proc_out_rlt->SaveDataMesSearch(pParam, judge, srch_ok);

	//結果出力ディレクトリの作成
	proc_out_rlt->CreateResultFolder(pParam, judge);

	//結果出力パスの作成
	char data_file_path[MAX_PATH] = { '\0' };	//データファイルパス
	char id_name[20] = { '\0' };				//ID文字列
	proc_out_rlt->CreatePath(pParam, judge, NULL, data_file_path, NULL, id_name);

	//計測モードのデータ保存
	for (int i = 0; i < 8; i++) {
		//検査機能を確認
		int mes_type = param->sheet[i].mes_type;

		//検査毎のデータ保存
		switch (mes_type) {
			//無し
		case 0x00:
			break;
			//ブロブ
		case 0x01:
		{
			CExcParamAlignBlob exc_param;
			AlignBlob::Param::CTempParamBlob dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			FVCL::Blob::CFvBlobResult* blob_result = &temp_param->blob_result[i];

			AlignBlob::Param::CMesParamBlob dll_param_mes;		//検査パラメータ
			exc_param.SetBlobMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);	//テンプレート結果セット

			double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
			BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
			//キャリブレーション未完了の場合
			if (!pixel_complete) {
				pixel_size = 0;		//画素サイズ0
			}

			//ブロブ検査データ保存
			AlignBlob::Proc::CProcOutResultBlob proc_outrslt_blob;
			proc_outrslt_blob.SaveDataMesBlob(data_file_path, id_name, blob_result, &dll_param_temp, &dll_param_mes, template_number, i, pixel_size, dlg->thread_rs.mes_ok[i]);
		}
			break;
		//エッジ
		case 0x02:
		{
			CExcParamAlignEdge exc_param;
			AlignEdge::Param::CTempParamEdge dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			FVCL::Edge::CFvEdgeResult* edge_result = &temp_param->edge_result[i];

			AlignEdge::Param::CMesParamEdge dll_param_mes;		//検査パラメータ
			exc_param.SetEdgeMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);	//テンプレート結果セット

			//エッジ検査データ保存
			AlignEdge::Proc::CProcOutResultEdge proc_outrslt_edge;
			proc_outrslt_edge.SaveDataMesEdge(data_file_path, id_name, edge_result, &dll_param_temp, &dll_param_mes, template_number, i, dlg->thread_rs.mes_ok[i]);
		}
			break;
		//濃度
		case 0x03:
		{
			CExcParamAlignMeas exc_param;
			AlignMeas::Param::CTempParamMeas dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			double* meas_result = &temp_param->meas_result[i];

			AlignMeas::Param::CMesParamMeas dll_param_mes;		//検査パラメータ
			exc_param.SetMeasMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);	//テンプレート結果セット

			//濃度検査データ保存
			AlignMeas::Proc::CProcOutResultMeas proc_outrslt_meas;
			proc_outrslt_meas.SaveDataMesMeas(data_file_path, id_name, meas_result, &dll_param_temp, &dll_param_mes, template_number, i, dlg->thread_rs.mes_ok[i]);
		}
			break;
			//マルチ探索
		case 0x04:
		{
			CExcParamAlignMultiSrh exc_param;
			AlignMultiSrh::Param::CTempParamMultiSrh dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			AlignMultiSrh::Param::CMultiSrhResult* multisrh_result = &temp_param->multisrh_result[i];

			AlignMultiSrh::Param::CMesParamMultiSrh dll_param_mes;		//検査パラメータ
			exc_param.SetMultiSrhMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);	//テンプレート結果セット

			//マルチ探索検査データ保存
			AlignMultiSrh::Proc::CProcOutResultMultiSrh proc_outrslt_multisrh;
			proc_outrslt_multisrh.SaveDataMesMultiSrh(data_file_path, id_name, multisrh_result, &dll_param_temp, &dll_param_mes, template_number, i, dlg->thread_rs.mes_ok[i]);
		}
			break;
			//基準-エッジ距離計測
		case 0x05:
		{
			CExcParamAlignDisEdge exc_param;
			AlignDisEdge::Param::CTempParamDisEdge dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			AlignDisEdge::Param::CDisEdgeResult* disedge_result = &temp_param->disedge_result[i];

			AlignDisEdge::Param::CMesParamDisEdge dll_param_mes;		//検査パラメータ
			exc_param.SetDisEdgeMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);	//テンプレート結果セット

			//基準-エッジ距離計測検査データ保存
			AlignDisEdge::Proc::CProcOutResultDisEdge proc_outrslt_disedge;
			proc_outrslt_disedge.SaveDataMesDisEdge(data_file_path, id_name, disedge_result, &dll_param_temp, &dll_param_mes, template_number, i, dlg->thread_rs.mes_ok[i]);
		}
			break;
			//エッジ-エッジ距離計測
		case 0x06:
		{
			CExcParamAlignDisEdgeToEdge exc_param;
			AlignDisEdgeToEdge::Param::CTempParamDisEdgeToEdge dll_param_temp;
			exc_param.SetTempParam(temp_param, &dll_param_temp);

			AlignDisEdgeToEdge::Param::CDisEdgeToEdgeResult* disetoe_result = &temp_param->disetoe_result[i];

			AlignDisEdgeToEdge::Param::CMesParamDisEdgeToEdge dll_param_mes;	//検査パラメータ
			exc_param.SetDisEdgeToEdgeMes(param, &dll_param_mes, i);
			exc_param.SetTempResult(temp_param, &dll_param_temp, i);			//テンプレート結果セット

			//エッジ-エッジ距離計測検査データ保存
			AlignDisEdgeToEdge::Proc::CProcOutResultDisEdgeToEdge proc_outrslt_disetoe;
			proc_outrslt_disetoe.SaveDataMesDisEdgeToEdge(data_file_path, id_name, disetoe_result, &dll_param_temp, &dll_param_mes, template_number, i, dlg->thread_rs.mes_ok[i]);
		}
			break;
			//計算
		case 0x21:
		{
			CFormulaResult* formula_result = &temp_param->formula_result[i];

			//計算検査データ保存
			CProcOutResultFormula proc_outrslt_formula;
			proc_outrslt_formula.SaveDataMesFormula(
				data_file_path,
				id_name,
				formula_result,
				temp_param,
				param,
				template_number,
				i,
				dlg->thread_rs.mes_ok[i]
			);
		}
			break;
		}
	}

	//総合判定保存
	proc_out_rlt->SaveDataMesJudge(pParam, judge);
}
