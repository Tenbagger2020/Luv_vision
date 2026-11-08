/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AddBlob.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：マーク探索処理の追加機能、2値化重心演算プログラム
 **
 **/

#include "stdafx.h"
#include "AddBlob.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Commands.h"			//コマンドパラメータ
#include "ScopeParam.h"			//機種毎スコープパラメータ

#include "CalcMark.h"			//サーチ結果取得
#include "CalcBinz.h"			//2値化演算

#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ

extern CCalcMark	calc_mark;		//サーチ結果取得
extern CCalcBinz	calc_binz;		//2値化演算

extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

CAddBlob::CAddBlob()
{
}


CAddBlob::~CAddBlob()
{
}


/** ****************************************************** **
 ** 2値化重心サーチメイン
 ** 引数：
 **　int template_number		テンプレート番号
 **　CTempParam* temp_param		探索パラメータ
 **　BOOL* add_ok				サーチ結果
 **　CListBox* lbox				ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗、-2:サーチエリア設定NG
 ** 特記事項：
 ** ****************************************************** **/
int CAddBlob::AddMain(int template_number, CTempParam* temp_param, BOOL* add_ok, CListBox* lbox)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;
	*add_ok = FALSE;

	//取込画像
	FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

	//検出位置データセット(基準点をサーチする場合)
	double mark_x = 0;	//検出位置X
	double mark_y = 0;	//検出位置Y
	if (temp_param->additional.base_search_enable) {
		if (AddDataSet(template_number, temp_param, &mark_x, &mark_y) != 0) goto END;
	}

	//2値化重心サーチ実施

	//サーチエリアの取得
	if (GetAddArea(&temp_param->add_blob.rect_area, mark_x, mark_y, temp_param, m_Image) == 0) {
		temp_param->add_blob.area_ok = TRUE;
	}
	else {
		//コマンドログ出力
		CString log_str1 = Tips0.SelStr(language.jpn, "2値化重心サーチ 結果:NG", "Gravity search  Result:NG");
		CString log_str2 = Tips0.SelStr(language.jpn, "2値化重心サーチエリアがカメラ視野範囲外", "Area is out of camera field of view");
		if (lbox == NULL) {
			outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ 2値化重心サーチ結果
			outlog.LogOutListCmd(0, 2, log_str2);
		}
		else {
			lbox->AddString(log_str1);
			lbox->AddString(log_str2);
		}
	}

	if (temp_param->add_blob.area_ok) {

		//
		// 2値化処理実行
		//

		//固定閾値パラメータ
		int threshold1 = temp_param->additional.blob_param.fix_threshoid[0];	//閾値1
		int threshold2 = temp_param->additional.blob_param.fix_threshoid[1];	//閾値2

		WORD type = temp_param->additional.blob_param.type;
		int xs = temp_param->add_blob.rect_area.st.x;
		int ys = temp_param->add_blob.rect_area.st.y;
		int x_size = temp_param->add_blob.rect_area.GetWidth() + 1;
		int y_size = temp_param->add_blob.rect_area.GetHeight() + 1;

		//2値化画像
		FVCL::Data::CFvImage binz_Image;

		switch (type) {
			//単一閾値指定
		case 0x0101:
			if (calc_binz.CalcBinzFix(xs, ys, x_size, y_size, threshold1, m_Image, &binz_Image) != 0) goto END;
			break;
			//バンド閾値指定
		case 0x0102:
			if (calc_binz.CalcBinzFixBand(xs, ys, x_size, y_size, threshold1, threshold2, m_Image, &binz_Image) != 0) goto END;
			break;
		}

		//2値化重心サーチ実施
		if (ExecuteBlob(temp_param, &temp_param->add_blob.rect_area, &binz_Image, &temp_param->add_blob.blob_ok, lbox) != 0) goto END;
	}
	else {
		//サーチエリア設定NG
		rtn_result = -2;
		goto END;
	}

	if (temp_param->add_blob.blob_ok) *add_ok = TRUE;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 検出位置データセット
 ** 引数：
 **　int template_number		テンプレート番号
 **　CTempParam* temp_param		探索パラメータ
 **　double* mark_x				検出位置X
 **　double* mark_y				検出位置Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddBlob::AddDataSet(int template_number, CTempParam* temp_param, double* mark_x, double* mark_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	byte matching = temp_param->basic.matching;	//探索手法 0x00:グレイサーチ、0x01:形状サーチ

	//グレイサーチ実行
	if (matching == 0x00) {
		if (calc_mark.GetMarkGS2(temp_param, mark_x, mark_y) != 0) goto END;
	}
	//形状サーチ実行
	else if (matching == 0x01) {
		if (calc_mark.GetMarkFPM2(temp_param , mark_x, mark_y) != 0) goto END;
	}
	else {
		goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** サーチエリアの取得
 ** 引数：
 **　FVCL::Data::CFvRectangle* rect		サーチエリア矩形
 **　double mark_x		検出位置X
 **　double mark_y		検出位置X
 **　CTempParam* temp_param		テンプレートパラメータ
 **　FVCL::Data::CFvImage* m_Image	取込画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddBlob::GetAddArea(FVCL::Data::CFvRectangle* rect, double mark_x, double mark_y, CTempParam* temp_param, FVCL::Data::CFvImage* m_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//検査エリアを基準点追従する
	if (temp_param->additional.base_search_enable) {
		rect->st.x = mark_x + temp_param->additional.blob_area.mark_dis_x;	//検出位置からの距離X
		rect->st.y = mark_y + temp_param->additional.blob_area.mark_dis_y;	//検出位置からの距離Y
	}
	//検査エリアを選択範囲で固定
	else {
		rect->st.x = temp_param->additional.blob_area.st_x;	//設定始点X
		rect->st.y = temp_param->additional.blob_area.st_y;	//設定始点Y
	}
	rect->ed.x = rect->st.x + temp_param->additional.blob_area.width - 1;
	rect->ed.y = rect->st.y + temp_param->additional.blob_area.height - 1;

	//検査エリアに外接する矩形の取得
	FVCL::Data::CFvRegion region_rect(*rect);	//エリア矩形をRegion化
	int x_min = region_rect.GetXmin();
	int y_min = region_rect.GetYmin();
	int x_max = region_rect.GetXmax();
	int y_max = region_rect.GetYmax();

	//検査エリアが視野範囲内か確認
	//※始点(rect->st.x/rect->st.y)が0未満(負数)の場合はx_min/y_minが0になることがあることから始点も確認する
	int image_width = m_Image->GetHorzSize();
	int image_height = m_Image->GetVertSize();
	if (x_min < 0 || image_width <= x_max ||
		y_min < 0 || image_height <= y_max ||
		rect->st.x < 0 || rect->st.y < 0) {
		goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 2値化重心演算
 ** 引数：
 **　CTempParam* temp_param				テンプレートパラメータ
 **　FVCL::Data::CFvRectangle* rect		サーチエリア矩形
 **　FVCL::Data::CFvImage* binz_Image	2値化画像
 **　BOOL* blob_ok		2値化重心サーチ結果
 **　CListBox* lbox		ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddBlob::ExecuteBlob(CTempParam* temp_param, FVCL::Data::CFvRectangle* rect_area, FVCL::Data::CFvImage* binz_Image, BOOL* blob_ok, CListBox* lbox)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//インスタンスの準備
	FVCL::Blob::CFvBlob	parser;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Blob::CFvBlob::CheckValidity(*binz_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, binz_Image);
	parser.SetResult(&temp_param->add_blob.result);

	//パラメータ設定
	FVCL::Blob::CFvBlobParam& param = parser.GetParam();
	byte color_mode = temp_param->additional.blob_param.color_mode;	//抽出色
	if (color_mode == 0x00) param.SetColorMode(FVCL::Blob::ObjectColor::BlackFG);	//前景:黒、背景:計測しない
	if (color_mode == 0x01) param.SetColorMode(FVCL::Blob::ObjectColor::WhiteFG);	//前景:白、背景:計測しない
	param.SetMaxBlobs(0); // 0:上限なし
	param.SetMaxRows(0);  // 0:上限なし
	param.SetMaxRuns(0);  // 0:上限なし
	param.SetNeighborhood(FVCL::Blob::Neighborhood::Eight);   // 8近傍
	param.SetPrecalcFeatures(0);

	//抽出面積
	UINT select_blob_area = 0;
	//抽出ブロブNo
	UINT select_blob_no = 0;
	temp_param->add_blob.blob_number = 0;	//結果ブロブ番号リセット

	//画像処理実行
	if (parser.Execute()) {

		//検出個数
		//　ブローブ数は背景ブローブが必ずあるので1以上になる
		//　0の場合は異常終了
		UINT count_result = temp_param->add_blob.result.GetCount();
		//検出成功
		if (count_result > 0) {
			rtn_result = 0;

			//結果の取得
			for (UINT i = 0; i < count_result; i++) {
				FVCL::Blob::CFvBlobData data;
				temp_param->add_blob.result.GetData(i, &data);

				if (data.GetBlobNo() == 0) goto LOOP_END;	//背景ブローブ(ブローブ番号0)の結果は取得しない

				UINT select_area = data.GetArea();	//ブロブ面積取得

				UINT area_min = temp_param->additional.blob_param.area_min;	//抽出サイズmin
				UINT area_max = temp_param->additional.blob_param.area_max;	//抽出サイズmax
				BOOL enable_area_unlimit = temp_param->additional.blob_param.enable_area_unlimit;	//抽出サイズmax上限なし

				//面積を抽出サイズで判別
				if (select_area >= area_min) {
					if (select_area <= area_max || enable_area_unlimit) {

						//ブロブ最大面積を選択
						if (select_blob_area < select_area) {
							select_blob_no = i;
							select_blob_area = select_area;	//最大面積を更新

							*blob_ok = TRUE;
						}
					}
				}
			LOOP_END:;
			}
		}
	}

	//最大面積のブロブ検出ができた場合は重心を取得
	if (select_blob_no > 0) {

		//重心取得
		FVCL::Blob::CFvBlobData data;
		temp_param->add_blob.result.GetData(select_blob_no, &data);
		FVCL::Data::CFvPoint m_point_center;	//ブロブ重心位置
		data.GetCenter(&m_point_center);		//ブロブ重心取得
		temp_param->add_blob.rslt_center.x = rect_area->st.x + m_point_center.x;
		temp_param->add_blob.rslt_center.y = rect_area->st.y + m_point_center.y;

		temp_param->add_blob.blob_number = select_blob_no;		//ブロブ番号を保存
	}

	//コマンドログ出力
	CString str_ok = "NG";
	if (*blob_ok) str_ok = "OK";
	CString log_str1 = Tips0.SelStr(language.jpn, "2値化重心サーチ 結果:", "Gravity search  Result:") + str_ok;
	if (lbox == NULL) {
		outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ 2値化重心サーチ結果
	}
	else {
		lbox->AddString(log_str1);
	}

END:
	return rtn_result;
}
