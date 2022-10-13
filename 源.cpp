#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include<vector>
#include<math.h>
#include<algorithm>

using namespace cv;
using namespace std;

class Seat
{
public:
	Seat(Rect rect, float area, int color_index,int count) {
		this->rect = rect;
		this->area = area;
		this->color_index = color_index;
		this->count = count;
	}
	int count;
	float area;
	Rect rect;
	int color_index;
	int row = 0;
	int col;
	bool falg = 0;
};

bool seat_compare_x(const Seat &A, const Seat &B) {
	return A.rect.tl().x > B.rect.tl().x;
}

bool seat_compare_y(const Seat &A, const Seat &B) {
	return A.rect.tl().y > B.rect.tl().y;
}

bool seat_compare_count(const Seat &A, const Seat &B) {
	return A.count < B.count;
}

vector<vector <int>>base_color = vector<vector <int>>(6,vector<int>(3)) = {
		{255, 204, 204},	//蓝色
		{67, 163, 255},		//橙色
		{94, 111, 254},		//红色
		{203, 192, 255},  //粉色
		{ 226, 228, 229 }, //灰色
		{ 50, 176, 102 }    //绿色
	};

string base_color_name[6] = { "蓝色", "橙色", "红色", "粉色", "灰色", "绿色" };

//查找矩形框的颜色
int find_color(const Mat rect_img) {
	int color_cnt[6] = { 0 };
	for (int i = 0; i < rect_img.rows; i++) {
		for (int j = 0; j < rect_img.cols; j++) {
			Vec3b pixel = rect_img.at<Vec3b>(i, j);
			for (int c = 0; c < 6; c++) {
				double dis = sqrt(pow(pixel[0] - base_color[c][0], 2) +
					pow(pixel[1] - base_color[c][1], 2) +
					pow(pixel[2] - base_color[c][2], 2));
				if (dis < 15) {
					color_cnt[c]++;
				}
			}
		}
	}
	int max = color_cnt[0];
	int index = 0;
	for (int i = 1; i < 6; i++) {
		if (color_cnt[i] > max) {
			max = color_cnt[i];
			index = i;
		}
	}
	//返回矩形框的颜色下标
	return index;
}

int main(){
	//导入图片，图片放在程序所在目录
	//imread读入图片为 bgr图像
	Mat img_bgr = imread("./cut.png");

	//bgr转化为rgb图像
	Mat img_rgb;
	cvtColor(img_bgr, img_rgb, COLOR_BGR2RGB);

	//转换为灰度图
	Mat gray;
	cvtColor(img_bgr, gray, COLOR_BGR2GRAY);
	imshow("gray", gray);

	//使用局部阈值的自适应阈值操作进行图像二值化
	//gray为函数的输入图像，dst为threshold函数的输出图像, 0 为二值化的阈值， 255为最大值，THRESH_OTSU确定生成二值化图像的算法
	Mat dst;
	threshold(gray, dst, 0, 255, THRESH_OTSU);
	imshow("dst1", dst);

	//形态学去噪, getStructuringElement()函数返回指定形状和尺寸的结构元素（内核矩阵) (3*3大小的十字矩阵结构元素)
	Mat element = getStructuringElement(MORPH_CROSS, Size(3, 3));

	//开运算去噪 开运算就是先腐蚀后膨胀，可以用于降噪，消除一些小的像素点
	morphologyEx(dst, dst, MORPH_OPEN, element);
	imshow("dst2", dst);

	//轮廓检测函数,第一个参数：输入的图像（要求图像为单通道图像，最好为二值图） ，第二个参数：输出的轮廓，为vector<vector<Point>>类型
	vector<vector<Point>> contours;

	/*第三个参数：每条轮廓对应的属性;定义为 vector<Vec4i>hierarchy中的每个元素都是一个由4个int型组成的集合。直观的表示可以参考列数为4，行数为n的二维矩阵。
	这四个int型数hierarchy[i][0]~hierarchy[i][3]分别表示后一个轮廓，前一个轮廓，父轮廓，内嵌轮廓的索引编号，如果当前轮廓所对应的这四个轮廓之一有缺失，
	比如说容器内的第一个轮廓为没有前一个轮廓，则相应位置hierarchy[i][1]=-1。*/
	vector<Vec4i>hierarchy; //Vec4i 表示为四通道图像  即int[4];
	
	//第四个参数：轮廓的模式，建立一个等级树结构的轮廓，第五个参数：轮廓的近似方法，
	findContours(dst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

	//绘制轮廓
	drawContours(dst, contours, -1, (120, 0, 0), 2);


	int count = 0;						//总数
	double area_avrg = 0;				//平均
	int	seat_cnt[6] = { 0 };		   //各颜色统计
	int	double_seat = 0;				//双人座

	//创建座位数组
	vector<Seat> seats;

	//遍历所有轮廓
	for (int i = 0; i < contours.size(); i++) {
		double area = contourArea(contours[i]);
		if (area < 50 or area > 2000)
			continue;
		count++;
		area_avrg += area;

		if (area > 1000) {
			//提取矩形坐标（x,y），根据轮廓绘制矩形边框
			//将双人坐进行拆分
			Rect rect = boundingRect(contours[i]);
			Rect rectl = Rect(rect.tl(), Size(rect.width / 2, rect.height));
			Rect rectr = Rect(rect.tl().x + rect.width / 2, rect.tl().y , rect.width / 2, rect.height);

			Mat maskl = dst(rectl);
			Mat maskr = dst(rectr);

			//左侧矩阵
			Mat rect_bgrl = img_bgr(rectl);

			int color_idx = find_color(rect_bgrl);
			seat_cnt[color_idx] += 2;

			//右侧矩阵 
			Mat rect_bgrr = img_bgr(rectr);

			double_seat += 1;

			//将座位的实例化对象加入到数组中
			seats.push_back(Seat(rectr, area / 2, color_idx, count));
			seats.push_back(Seat(rectl, area / 2, color_idx, ++count));

			//绘制矩形
			rectangle(img_bgr, rectr, (0, 0, 255), 1);
			rectangle(img_bgr, rectl, (0, 0, 255), 1);

			// 在左上角写上编号
			putText(img_bgr, to_string(count-1), rectr.tl(), FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1);
			putText(img_bgr, to_string(count), rectl.tl(), FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1);
		}
		else
		{
			//提取矩形坐标（x,y），根据轮廓绘制矩形边框
			Rect rect = boundingRect(contours[i]);

			//rect.area();     //返回rect的面积 5000
			//rect.size();     //返回rect的尺寸 [50 × 100]
			//rect.tl();       //返回rect的左上顶点的坐标 [100, 50]
			//rect.br();       //返回rect的右下顶点的坐标 [150, 150]
			//rect.width();    //返回rect的宽度 50
			//rect.height();   //返回rect的高度 100
			//rect.contains(Point(x, y));  //返回布尔变量，判断rect是否包含Point(x, y)点
			//提取mask,二值图的矩形边框内的像素块,用Rect来截取图片
			Mat mask = dst(rect);

			Mat rect_bgr = img_bgr(rect);

			int color_idx = find_color(rect_bgr);
			seat_cnt[color_idx] += 1;

			//将座位的实例化对象加入到数组中
			seats.push_back(Seat(rect, area, color_idx, count));

			//绘制矩形
			rectangle(img_bgr, rect, (0, 0, 255), 1);

			//防止编号到图片之外（上面）,因为绘制编号写在左上角，所以让最上面的y小于10的变为10个像素
			int y;
			if (rect.tl().y < 10)
				y = 10;
			else
			{
				y = rect.tl().y;
			}
			// 在左上角写上编号
			putText(img_bgr, to_string(count), rect.tl(), FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1);
		}
	}
	
	//以y坐标进行排序
	sort(seats.begin(), seats.end(), seat_compare_y);

	int row = 1;
	for (int i = 0; i < seats.size(); i++) {
		if (i > 0 && seats[i - 1].rect.tl().y - seats[i].rect.tl().y > 20)
			row++;
		seats[i].row = row;
	}

	//以x坐标进行排序
	sort(seats.begin(), seats.end(), seat_compare_x);

	int col = 1;
	for (int i = 0; i < seats.size(); i++) {
		if (i > 0 && seats[i - 1].rect.tl().x - seats[i].rect.tl().x > 20)
			col++;
		seats[i].col = col;
	}
	
	sort(seats.begin(), seats.end(), seat_compare_count);

	cout << "座位个数：" << count << "\t" << "总面积为：" << area_avrg << endl;
	cout << "包含双人座位：" << double_seat << "个" << endl;
	for (int i = 0; i < 6; i++) {
		cout << "座位颜色：" << base_color_name[i] << "\t" << "数量：" << seat_cnt[i] << endl;
		for (int j = 0; j < seats.size(); j++) {
			if (seats[j].color_index == i) {
				cout << "\t" << "第" << seats[j].row << "行  "
					<< "第" << seats[j].col << "列  " << endl;
			}
		}
	}


	imshow("bgr", img_bgr);
	waitKey(0);
	return 0;
}


