/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcBcrCtrl.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：バーコードリーダー演算プログラム制御
 **
 **/

#include "stdafx.h"
#include "CalcBcrCtrl.h"

#include "DlgSystemMain.h"
#include "ExcParamAlignBarcode.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Commands.h"			//コマンドパラメータ

#include "CalcMark.h"			//サーチ結果取得
#include "CalcPLC.h"			//PLCデータ取得
#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

//DLL
#include "AlignBarcode.h"	//AlignBarcode DLL Header

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ

extern CCalcMark	calc_mark;		//サーチ結果取得
extern CCalcPLC		calc_plc;		//PLCデータ取得
extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

//DLL
extern AlignBarcode::Param::CBarcodeParam bcr_param[256];		//機種毎バーコードリーダーパラメータ
extern AlignBarcode::Proc::CCalcBarcode calc_bcr;				//バーコードリーダー演算

CCalcBcrCtrl::CCalcBcrCtrl()
{
}


CCalcBcrCtrl::~CCalcBcrCtrl()
{
}


/** ****************************************************** **
 ** バーコードリーダー制御メイン
 ** 引数：
 **　LPVOID pParam
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBcrCtrl::CalcMain(LPVOID pParam)
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam));

	CToolTips0 Tips0;
	int rtn_result = 1;

	byte cmd = dlg->run_cmd;	//実行コマンド
	byte base_method = cmd_param[cmd].base_method;	//機能種別

	//対象視野
	int template_number = cmd_param[cmd].template_number;	//テンプレートNo
	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	//使用するパラメータ
	CTempParam* temp_param = &template_param[template_number];		//テンプレートパラメータ
	AlignBarcode::Param::CBarcodeParam* param = &bcr_param[template_number];	//バーコードリーダーパラメータ
	CScopeParam* scp_param = &scope_param[scope];			//スコープパラメータ
	AlignBarcode::Data::CComDataBarcode com_data_bcr;		//読取文字列データ

	//結果をリセット
	BOOL bcr_ok = FALSE;

	//バーコードリーダー実施
	if (cmd_param[cmd].search.method == 0x0401) {

		//バーコードのデータセット
		if (param->base_search_enable) {
			int parts = cmd_param[cmd].align.parts;	//アライメント視野数

			if (CalcDataSet(temp_param, scp_param, parts) != 0) {
				dlg->thread_rs.calc_err = 0x9441;	//エラーコード
				goto END;
			}
		}

		FVCL::Barcode::CFvBarcodeResult* bc_result = &temp_param->barcode.bc_result;
		FVCL::QR::CFvQRResult* qr_result = &temp_param->barcode.qr_result;
		FVCL::DM::CFvDMResult* dm_result = &temp_param->barcode.dm_result;
		int* base_search_result = &temp_param->barcode.base_search_result;
		FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

		double base_mark_x = scp_param->view[0].align.mark_x;
		double base_mark_y = scp_param->view[0].align.mark_y;

		//バーコードリーダーの演算実行
		if (calc_bcr.CalcMain(param, &com_data_bcr, m_Image, bc_result, qr_result, dm_result, base_mark_x, base_mark_y, &bcr_ok, base_search_result, NULL) == 1) {
			dlg->thread_rs.calc_err = 0x9442;	//エラーコード
			goto END;
		}
	}
	//異常系(想定外コマンド)
	else {
		dlg->thread_rs.calc_err = 0x9449;	//エラーコード
		goto END;
	}

	//
	//検査結果のデータセット
	//
	WORD w_data[15] = { 0x0000 };
	for (int i = 0; i < 15; i++) {
		Tips0.conv3.val_b[0] = com_data_bcr.Ascii_Code_Data[0 + i * 2];
		Tips0.conv3.val_b[1] = com_data_bcr.Ascii_Code_Data[1 + i * 2];
		w_data[i] = Tips0.conv3.val_w;	//byte30文字データをWORDデータに
	}
	SetResult(scope, base_method, bcr_ok, w_data);

	//結果をセット
	dlg->thread_rs.bcr_ok = bcr_ok;

	//コマンドログ出力
	outlog.LogOutListCmd(0, 2, Tips0.SelStr(language.jpn, "バーコードリーダー演算完了", "Barcode reader calculation completed"));	//コマンドログ 演算完了

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** バーコードリーダーのデータセット
 ** 引数：
 **　CTempParam* temp_param	テンプレートパラメータ
 **　CScopeParam* scp_param	スコープパラメータ
 **　int parts				アライメント視野数
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBcrCtrl::CalcDataSet(CTempParam* temp_param, CScopeParam* scp_param, int parts)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (parts != 1) goto END;	//視野数は"1"固定のため1以外はエラー

	for (int i = 0; i < parts; i++) {
		//対象スコープのテンプレート番号を取得
		//　バーコードリーダーでは視野数1でスコープパラメータに入るテンプレート番号も自身と同じなので
		//　スコープパラメータからもらうのをやめる
		//int template_number = scope_param[scope].view[i].align.template_number;

		//検出位置
		double mark_x;	//検出位置X
		double mark_y;	//検出位置Y
		//サーチ成功
		if (!temp_param->manu.do_point) {
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
 **　BOOL bcr_ok			バーコードリーダー結果
 **　WORD* w_data			読取文字データ(ASCIIコード)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcBcrCtrl::SetResult(int scope, byte base_method, BOOL bcr_ok, WORD* w_data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;	//機能ツール

	scope_param[scope].result.bcr.bcr_ok = bcr_ok;

	//15ワード(30文字)を格納
	for (int i = 0; i < 15; i++) {
		scope_param[scope].result.bcr.w_data[i] = w_data[i];
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
void CCalcBcrCtrl::SetOutData(byte cmd)
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

	calc_plc.BcrCalcDataOut(cmd, scope, base_method, scope_param[scope].result.bcr.w_data);
}
