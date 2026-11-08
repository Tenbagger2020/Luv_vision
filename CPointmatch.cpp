//#include "stdafx.h"
//#include "CPointmatch.h"
//CPointmatch  CP
//
//
//int CPointmatch::Calc()
//{
//	CP.GetPartsCenter(int parts, double* center_x, double* center_y, const double* mech_x, const double* mech_y);  //重心G（center_x,center_y）を返す
//};
//
//
///** ****************************************************** **
// **赤外カメラでのアライメントで、４点でのサーチポイントを、単点一致で他視野のポイントへの補正量を算出するために利用
// ** 対角中点座標を求める 
// ** 引数：
// **　int parts				アライメント視野数
// **　double* center_x		中心座標X
// **　double* center_y		中心座標Y
// **　const double* mech_x	視野座標X(目標視野座標X配列の先頭アドレス)　→これが、コマンド32～35で取得したx[4]
// **　const double* mech_y	視野座標Y(目標視野座標X配列の先頭アドレス)  →これが、コマンド32～35で取得したy[4]
// ** 戻り値：
// **　int 結果　0:成功、1:失敗
// ** 特記事項：
// ** ****************************************************** **/
//int CPointmatch::GetPartsCenter(int parts, double* center_x, double* center_y, const double* mech_x, const double* mech_y)
//{
//	// TODO: ここに実装コードを追加します.
//	int rtn_result = 1;
//
//	//アライメント視野数の確認
//	if (parts < 3 || parts > 4) goto END;
//
//	double a1 = 0, b1 = 0, a2 = 0, b2 = 0;
//
//	double mech_cent_x[4];	//マーク間センターX
//	double mech_cent_y[4];	//マーク間センターY
//
//	//アライメント視野数4
//	if (parts == 4) {
//
//		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
//		mech_cent_x[1] = (mech_x[1] + mech_x[2]) / 2;
//		mech_cent_x[2] = (mech_x[2] + mech_x[3]) / 2;
//		mech_cent_x[3] = (mech_x[3] + mech_x[0]) / 2;
//
//		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
//		mech_cent_y[1] = (mech_y[1] + mech_y[2]) / 2;
//		mech_cent_y[2] = (mech_y[2] + mech_y[3]) / 2;
//		mech_cent_y[3] = (mech_y[3] + mech_y[0]) / 2;
//
//		if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0 && mech_cent_x[0] != 0 && mech_cent_x[1] != 0) {
//
//			b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
//			a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];
//
//			b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
//			a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];
//
//			//交点
//			*center_y = (a1 * b2 - a2 * b1) / (a1 - a2);
//			*center_x = (*center_y - b1) / a1;
//		}
//		else {
//			if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0) {
//
//				b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
//				a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];
//
//				//交点
//				*center_x = mech_cent_x[0];
//				*center_y = a2 * (*center_x) + b2;
//			}
//			else if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {
//
//				b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
//				a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];
//
//				//交点
//				*center_x = mech_cent_x[1];
//				*center_y = a1 * (*center_x) + b1;
//			}
//			else if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {
//
//				*center_x = mech_cent_x[0];
//				*center_y = (mech_y[0] + mech_y[3]) / 2;
//			}
//		}
//	}
//	//アライメント視野数3
//	else if (parts == 3) {
//		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
//		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
//		mech_cent_x[1] = (mech_x[0] + mech_x[2]) / 2;
//		mech_cent_y[1] = (mech_y[0] + mech_y[2]) / 2;
//
//		if ((mech_x[1] - mech_cent_x[1]) != 0 && (mech_x[2] - mech_cent_x[0]) != 0 && mech_x[1] != 0 && mech_x[2] != 0) {
//
//			b1 = (mech_x[1] * mech_cent_y[1] - mech_cent_x[1] * mech_y[1]) / (mech_x[1] - mech_cent_x[1]);
//			a1 = (mech_y[1] - b1) / mech_x[1];
//
//			b2 = (mech_x[2] * mech_cent_y[0] - mech_cent_x[0] * mech_y[2]) / (mech_x[2] - mech_cent_x[0]);
//			a2 = (mech_y[2] - b2) / mech_x[2];
//
//			//交点
//			*center_y = (a1 * b2 - a2 * b1) / (a1 - a2);
//			*center_x = (*center_y - b1) / a1;
//		}
//		else {
//			goto END;
//		}
//	}
//
//	rtn_result = 0;
//
//END:
//	return rtn_result;
//}
//
