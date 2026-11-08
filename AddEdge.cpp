/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AddEdge.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.09.12	M.Iwasaki	エッジスコア判定修正
 ** 注記：
 ** 説明：マーク探索処理の追加機能、エッジ演算プログラム
 **
 **/

#include "stdafx.h"
#include "AddEdge.h"

#include "Templates.h"			//システムテンプレートパラメータ
#include "Commands.h"			//コマンドパラメータ
#include "ScopeParam.h"			//機種毎スコープパラメータ

#include "CalcMark.h"			//サーチ結果取得

#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ

extern CCalcMark	calc_mark;		//サーチ結果取得

extern COutputLog	outlog;			//ログ出力
extern CLanguage	language;		//言語設定用

CAddEdge::CAddEdge()
{
}


CAddEdge::~CAddEdge()
{
}


/** ****************************************************** **
 ** エッジサーチメイン
 ** 引数：
 **　int template_number		テンプレート番号
 **　CTempParam* temp_param		探索パラメータ
 **　BOOL* add_ok				サーチ結果
 **　CListBox* lbox				ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗、-2:サーチエリア設定NG
 ** 特記事項：
 ** ****************************************************** **/
int CAddEdge::AddMain(int template_number, CTempParam* temp_param, BOOL* add_ok, CListBox* lbox)
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

	//縦と横エッジ実施
	for (int i = 0; i < 2; i++) {
		//サーチエリアの取得
		if (GetAddArea(&temp_param->add_edge.rect_area[i], i, mark_x, mark_y, temp_param, m_Image) == 0) {
			temp_param->add_edge.area_ok[i] = TRUE;
		}
		else {
			//コマンドログ出力
			CString str_direc = Tips0.SelStr(language.jpn, "縦", "V-");
			if (i == 1) str_direc = Tips0.SelStr(language.jpn, "横", "H-");
			CString log_str1 = Tips0.SelStr(language.jpn, "エッジサーチ(", "Edge search(") + str_direc + Tips0.SelStr(language.jpn, "エッジ) 結果:NG", "edge) Result:NG");
			CString log_str2 = Tips0.SelStr(language.jpn, "エッジサーチエリアがカメラ視野範囲外", "Area is out of camera field of view");
			if (lbox == NULL) {
				outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ エッジサーチ結果
				outlog.LogOutListCmd(0, 2, log_str2);
			}
			else {
				lbox->AddString(log_str1);
				lbox->AddString(log_str2);
			}
		}

		if (temp_param->add_edge.area_ok[i]) {
			//エッジサーチ実施
			if (ExecuteEdge(template_number, i, temp_param, &temp_param->add_edge.rect_area[i], m_Image, &temp_param->add_edge.edge_ok[i], lbox) != 0) goto END;
		}
	}
	if (!temp_param->add_edge.area_ok[0] || !temp_param->add_edge.area_ok[1]) {
		//サーチエリア設定NG
		rtn_result = -2;
		goto END;
	}

	//エッジ線からの交点座標算出
	if (temp_param->add_edge.edge_ok[0] && temp_param->add_edge.edge_ok[1]) {
		//交点計算
		if (CalcCrossPoint(template_number, temp_param, add_ok) != 0) goto END;
	}

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
int CAddEdge::AddDataSet(int template_number, CTempParam* temp_param, double* mark_x, double* mark_y)
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
		if (calc_mark.GetMarkFPM2(temp_param, mark_x, mark_y) != 0) goto END;
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
 **　int direction		方向(0:縦, 1:横)
 **　double mark_x		検出位置X
 **　double mark_y		検出位置X
 **　CTempParam* temp_param		テンプレートパラメータ
 **　FVCL::Data::CFvImage* m_Image	取込画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddEdge::GetAddArea(FVCL::Data::CFvRectangle* rect, int direction, double mark_x, double mark_y, CTempParam* temp_param, FVCL::Data::CFvImage* m_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//検査エリアを基準点追従する
	if (temp_param->additional.base_search_enable) {
		rect->st.x = mark_x + temp_param->additional.edge_area[direction].mark_dis_x;	//検出位置からの距離X
		rect->st.y = mark_y + temp_param->additional.edge_area[direction].mark_dis_y;	//検出位置からの距離Y
	}
	//検査エリアを選択範囲で固定
	else {
		rect->st.x = temp_param->additional.edge_area[direction].st_x;	//設定始点X
		rect->st.y = temp_param->additional.edge_area[direction].st_y;	//設定始点Y
	}
	rect->ed.x = rect->st.x + temp_param->additional.edge_area[direction].width - 1;
	rect->ed.y = rect->st.y + temp_param->additional.edge_area[direction].height - 1;

	//検査エリアに外接する矩形の取得
	FVCL::Data::CFvRegion region_rect(*rect);	//エリア矩形をRegion化
	int x_min = region_rect.GetXmin();
	int y_min = region_rect.GetYmin();
	int x_max = region_rect.GetXmax();
	int y_max = region_rect.GetYmax();

	//検査エリアが視野範囲内か確認
	int image_width = m_Image->GetHorzSize();
	int image_height = m_Image->GetVertSize();
	if (x_min < 0 || image_width <= x_max ||
		y_min < 0 || image_height <= y_max) {
		goto END;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** エッジ演算
 ** 引数：
 **　int template_number	テンプレート番号
 **　int direction			方向(0:縦, 1:横)
 **　CTempParam* temp_param					テンプレートパラメータ
 **　FVCL::Data::CFvRectangle* rect_area	サーチエリア矩形
 **　FVCL::Data::CFvImage* m_Image			取込画像
 **　BOOL* edge_ok		エッジサーチ結果
 **　CListBox* lbox		ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddEdge::ExecuteEdge(int template_number, int direction, CTempParam* temp_param, FVCL::Data::CFvRectangle* rect_area, FVCL::Data::CFvImage* m_Image, BOOL* edge_ok, CListBox* lbox)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	//インスタンスの準備
	FVCL::Edge::CFvBoxEdge parser;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Edge::CFvBoxEdge::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetResult(&temp_param->add_edge.result[direction]);

	//検出点
	int number = 0;
	if (direction == 0) number = rect_area->ed.x - rect_area->st.x + 1;
	if (direction == 1) number = rect_area->ed.y - rect_area->st.y + 1;

	//パラメータ設定
	parser.SetBox(*rect_area);
	parser.SetNumber(number);
	parser.SetLineWidth(temp_param->additional.edge_param[direction].line_width);
	if (temp_param->additional.edge_param[direction].edge_peak) parser.SetThreshold(-1);
	if (!temp_param->additional.edge_param[direction].edge_peak) parser.SetThreshold(temp_param->additional.edge_param[direction].threshold);
	parser.SetScanDirection(temp_param->additional.edge_param[direction].scan_direction);
	parser.SetEdgeDirection(temp_param->additional.edge_param[direction].edge_direction);
	parser.SetFoot(temp_param->additional.edge_param[direction].foot);
	parser.SetLen(temp_param->additional.edge_param[direction].len);

	//検出線数
	int box_line = parser.GetNumber();

	//検出個数
	UINT count_result = 0;
	//判定範囲内個数
	UINT count_judge = 0;

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;

		//判定範囲のパラメータ取得
		int judge_area_lower = temp_param->additional.edge_param[direction].judge_area_lower;	//判定範囲下限
		int judge_area_upper = temp_param->additional.edge_param[direction].judge_area_upper;	//判定範囲上限

		//判定範囲の矩形作成
		FVCL::Data::CFvRectangle* rect_judge = &temp_param->add_edge.rect_judge[direction];
		CreateJudgeRect(direction, temp_param, *rect_area, rect_judge);

		count_result = temp_param->add_edge.result[direction].GetCount();	//検出個数取得

		//検出成功
		if (count_result > 0) {

			//近似線作成の点群定義
			DPNT_T* pts;
			pts = new DPNT_T[count_result];	//検出個数で要素数を確保

			//結果の取得
			for (UINT i = 0; i < count_result; i++) {
				FVCL::Edge::CFvEdgeData data;
				temp_param->add_edge.result[direction].GetData(i, &data);

				bool judge_in = false;	//判定範囲内フラグ

				//判定範囲をRegion化しエッジ点が範囲内か確認する
				FVCL::Data::CFvRegion rect_region(*rect_judge);
				judge_in = rect_region.Contains(data.GetPosition().x, data.GetPosition().y);

				if (judge_in) {
					//有効なエッジ点の確保
					pts[count_judge] = data.GetPosition().ToDPNT_T();	//有効なエッジ点のみ追加する

					count_judge++;
				}
			}

			//結果判定 スコア
			double result_score = 0;
			if (count_result > 0) {
				int judge_score = temp_param->additional.edge_param[direction].judge_score;
				result_score = (double)count_judge / (double)box_line * (double)100;
				if (result_score >= (double)judge_score) {

					//線作成可条件(結果判定OK、エッジ2点以上)
					if (count_judge >= 2) {
						try {
							//ロバスト推定(M推定法)からの近似線作成
							//　FVCL::CG::FitLine()
							//	F_FIT_MESTIMATOR：ロバスト推定(M推定法)
							//	1.645：90％信頼区間内の点をインライアと判定する
							FVCL::CFvBuffer<DPNT_T> buff_pts(pts, count_judge, true);	//有効な点だけを配列で定義
							temp_param->add_edge.rslt_line[direction] = FVCL::CG::FitLine(buff_pts, F_FIT_MESTIMATOR, 1.645);	//近似線の作成
							*edge_ok = TRUE;
						}
						catch (const FVCL::CFveException &ex) {
							int code = ex.GetErrorCode();
							CString func = ex.GetFunction();
						}
					}
				}
			}
		}

		//コマンドログ出力
		CString str_ok = "NG";
		if (*edge_ok) str_ok = "OK";
		CString str_direc = Tips0.SelStr(language.jpn, "縦", "V-");
		if (direction == 1) str_direc = Tips0.SelStr(language.jpn, "横", "H-");
		CString log_str1 = Tips0.SelStr(language.jpn, "エッジサーチ(", "Edge search(") + str_direc + Tips0.SelStr(language.jpn, "エッジ) 結果:", "edge) Result:") + str_ok;
		if (lbox == NULL) {
			outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ エッジサーチ結果
		}
		else {
			lbox->AddString(log_str1);
		}
	}
	else {
		//コマンドログ出力
		CString str_direc = Tips0.SelStr(language.jpn, "縦", "V-");
		if (direction == 1) str_direc = Tips0.SelStr(language.jpn, "横", "H-");
		CString log_str1 = Tips0.SelStr(language.jpn, "エッジサーチ(", "Edge search(") + str_direc + Tips0.SelStr(language.jpn, "エッジ) 結果:NG", "edge) Result:NG");
		if (lbox == NULL) {
			outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ エッジサーチ結果
		}
		else {
			lbox->AddString(log_str1);
		}
		if (parser.GetErrorCode() == 11) {
			CString log_str2 = Tips0.SelStr(language.jpn, "エッジサーチエリアが画像の端過ぎです", "Area is too close to the image edge");
			if (lbox == NULL) {
				outlog.LogOutListCmd(0, 2, log_str2);
			}
			else {
				lbox->AddString(log_str2);
			}
			rtn_result = 0;	
		}
		if (parser.GetErrorCode() == 22) {
			CString log_str2 = Tips0.SelStr(language.jpn, "エッジがみつかりません", "Edge not found");
			if (lbox == NULL) {
				outlog.LogOutListCmd(0, 2, log_str2);
			}
			else {
				lbox->AddString(log_str2);
			}
			rtn_result = 0;
		}
		goto END;
	}

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 判定エリア矩形の作成
 ** 引数：
 **　int direction	方向(0:縦, 1:横)
 **　CTempParam* temp_param			テンプレートパラメータ
 **　FVCL::Data::CFvRectangle rect_area		サーチエリア矩形
 **　FVCL::Data::CFvRectangle* rect_judge	判定矩形
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CAddEdge::CreateJudgeRect(int direction, CTempParam* temp_param, FVCL::Data::CFvRectangle rect_area, FVCL::Data::CFvRectangle* rect_judge)
{
	// TODO: ここに実装コードを追加します.

	//判定範囲のパラメータ取得
	int judge_area_lower = temp_param->additional.edge_param[direction].judge_area_lower;	//判定範囲下限
	int judge_area_upper = temp_param->additional.edge_param[direction].judge_area_upper;	//判定範囲上限
	int scan_direction = temp_param->additional.edge_param[direction].scan_direction;		//検出方向

	double st_x, st_y;
	double ed_x, ed_y;
	int area_size;
	int offset_st, offset_ed;

	//縦エッジ
	if (direction == 0) {
		area_size = rect_area.ed.y - rect_area.st.y + 1;
		offset_st = area_size * judge_area_lower / 100;
		offset_ed = area_size * judge_area_upper / 100;

		switch (scan_direction) {
			//上から下(↓)
		case 2:
			st_x = rect_area.st.x;
			st_y = rect_area.st.y + offset_st;
			ed_x = rect_area.ed.x;
			ed_y = rect_area.st.y + (offset_ed - 1);
			break;
			//下から上(↑)
		case 3:
			st_x = rect_area.st.x;
			st_y = rect_area.ed.y - offset_st;
			ed_x = rect_area.ed.x;
			ed_y = rect_area.ed.y - (offset_ed - 1);
			break;
		}

		//判定範囲
		rect_judge->st.x = st_x;
		rect_judge->st.y = st_y;
		rect_judge->ed.x = ed_x;
		rect_judge->ed.y = ed_y;
	}
	//横エッジ
	else if (direction == 1) {
		area_size = rect_area.ed.x - rect_area.st.x + 1;
		offset_st = area_size * judge_area_lower / 100;
		offset_ed = area_size * judge_area_upper / 100;

		switch (scan_direction) {
			//左から右(→)
		case 0:
			st_x = rect_area.st.x + offset_st;
			st_y = rect_area.st.y;
			ed_x = rect_area.st.x + (offset_ed - 1);
			ed_y = rect_area.ed.y;
			break;
			//右から左(←)
		case 1:
			st_x = rect_area.ed.x - offset_st;
			st_y = rect_area.st.y;
			ed_x = rect_area.ed.x - (offset_ed - 1);
			ed_y = rect_area.ed.y;
			break;
		}

		//判定範囲
		rect_judge->st.x = st_x;
		rect_judge->st.y = st_y;
		rect_judge->ed.x = ed_x;
		rect_judge->ed.y = ed_y;
	}
}


/** ****************************************************** **
 ** 検出位置データセット
 ** 引数：
 **　int template_number	テンプレート番号
 **　CTempParam* temp_param		テンプレートパラメータ
 **　BOOL* add_ok			サーチ結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAddEdge::CalcCrossPoint(int template_number, CTempParam* temp_param, BOOL* add_ok)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Data::CFvAngle angle;

	//演算実行
	int cross_result = FVCL::Caliper::CrossPoint(temp_param->add_edge.rslt_line[0], temp_param->add_edge.rslt_line[1], &temp_param->add_edge.rslt_cross, &angle);

	//エッジ交点算出成功
	if (cross_result == 0) {
		//交点座標が視野内であれば
		if (temp_param->add_edge.rslt_cross.x >= 0 && temp_param->add_edge.rslt_cross.x < temp_param->m_Image.GetHorzSize()
			&& temp_param->add_edge.rslt_cross.y >= 0 && temp_param->add_edge.rslt_cross.y < temp_param->m_Image.GetVertSize()) {
			*add_ok = TRUE;	//成功
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}
