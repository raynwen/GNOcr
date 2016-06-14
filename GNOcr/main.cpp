/**************************************************************
* ȼ��������ʶ�����
* Filename:main.cpp
* @author Sunie
* Date:2016.06.08
*Description:��Ҫʵ����Ȼ�����µ�ȼ���������ʶ��
*��Ҫʹ��KNN�㷨
***************************************************************/
#include<opencv.hpp>
#include <iostream>
#include<stdio.h>
#include<ml.hpp>
#include<ml.h>

using namespace cv;
using namespace std;
using namespace ml;


Ptr<TrainData> prepare_train_data();//�������ݵ�׼��
Mat g_grayImage;//�Ҷ�ͼ
Mat g_srcImage;//ԭͼ
Mat g_bilateralImage;//˫���˲�ͼ
Mat g_blackImage;//��ֵ��ͼ
Mat g_dstImage;
RNG g_rng(12345);
vector<vector<Point>> g_vContours;//�߿�洢
vector<Vec4i> g_vHierarchy;
RotatedRect g_mr;


//Sobel��Ե�����ر���
Mat g_sobelGradient_X, g_sobelGradient_Y;
Mat g_sobelAbsGradient_X, g_sobelAbsGradient_Y;
int g_sobelKernelSize = 1;//TrackBarλ�ò���  

						  //ѵ���������
Mat t_trainData = Mat(70, 323, CV_32FC1);//ѵ������
Mat_<int>t_trainclass;//ѵ�����ݱ�ǩ


bool verifySizes(RotatedRect mr);//�ж��Ƿ�Ϊ����
void bubbleSort(int arr1[], int arr2[]);
void ShowHelpText();

int main() {

	ShowHelpText();

	//����ԭͼ
	g_srcImage = imread("01781-2.jpg");

	//������Ĥ
	g_blackImage.create(g_srcImage.rows, g_srcImage.cols, g_srcImage.type());
	//�ҶȻ�
	if (g_srcImage.channels() == 3)
		cvtColor(g_srcImage, g_grayImage, COLOR_RGB2GRAY);
	else
		g_grayImage = g_srcImage;

	bilateralFilter(g_grayImage, g_bilateralImage, 4, 8, 2);//˫���˲�
	imshow("˫���˲�", g_bilateralImage);//˫���˲���ʾ
	threshold(g_bilateralImage, g_blackImage, 115, 255, THRESH_BINARY);//��ֵ��

	namedWindow("gray_mat");
	imshow("gray_mat", g_grayImage);
	imshow("black_mat", g_blackImage);


	//=============================������������=================================
	Mat g_result;
	g_blackImage.copyTo(g_result);
	g_blackImage.copyTo(g_dstImage);
	findContours(g_blackImage, g_vContours, g_vHierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//Ѱ������(������)
																							  //==========================================================================


																							  //=============================������������=================================
	Mat drawing = Mat::zeros(g_blackImage.size(), CV_8UC3);
	for (int i = 0; i < g_vContours.size(); i++)
	{
		Scalar color = Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255));//����ֵ
		drawContours(drawing, g_vContours, i, color, 1, 8, g_vHierarchy, 0, Point());
	}
	//��ʾЧ��ͼ
	imshow("�������", drawing);
	//=============================================================================


	//����Ѱ�ҳ�������
	vector<vector<Point> >::iterator itc = g_vContours.begin();
	vector<RotatedRect> rects;


	//=============================��ɸѡ����Ҫ���������=================================
	int t = 0;
	while (itc != g_vContours.end())
	{
		//������С��Ӿ���
		RotatedRect mr = minAreaRect(Mat(*itc));
		//�����ô���
		/*	 int area = mr.size.height * mr.size.width;
		float r = (float)mr.size.width / (float)mr.size.height;
		cout << "��" << mr.size.width << "\n";
		cout << "��" << mr.size.height << "\n";
		cout << "���" << area << "\n";
		cout << "����" << r << "\n";*/

		//ȡ����Ӿ���������Ϣ
		if (!verifySizes(mr))
		{
			itc = g_vContours.erase(itc);
		}
		else
		{
			++itc;
			rects.push_back(mr);
		}
	}
	//===========================================================================


	//����������С��Ӿ���λ����Ϣ
	vector<vector<Point> > g_contours_poly(g_vContours.size());
	vector<Rect> g_boundRect(g_vContours.size());

	//Ϊ��˳��������ֵ�λ����Ϣ
	int position[6];
	int positionNumber[6] = { 0, 1, 2, 3, 4, 5 };


	//=============================�������洢λ����Ϣ�����������ָ�ͼ��=================================
	for (int i = 0; i < g_vContours.size(); i++)
	{

		g_boundRect[i] = boundingRect(Mat(g_vContours[i]));//��ȡ����ı߽����Ϻ���������

		Rect g_box;

		//ָ���ָ������Χ
		g_box.x = g_boundRect[i].tl().x;
		g_box.y = g_boundRect[i].tl().y;
		g_box.width = g_boundRect[i].br().x - g_boundRect[i].tl().x;
		g_box.height = g_boundRect[i].br().y - g_boundRect[i].tl().y;
		position[i] = g_box.x;//��ȡ���Ͻ�x����
							  //�ָ�ָ��������ָ�������ڶ�ͼ����д���
		Mat g_imageROI(g_dstImage, g_box);
		//��һ����СΪ17*19
		Mat g_imageROIResize;
		resize(g_imageROI, g_imageROIResize, cvSize(17, 19));
		Mat g_imageROIReshape = g_imageROIResize.reshape(0, 1);
		//cout << g_imageROIReshape << "\n";
		/*float response = model->predict(g_imageROIReshape);
		cout << response << "\n";*/

		char g_chFile_name[200];//��һ���������
								//��һ���������
		sprintf(g_chFile_name, "%s%d%s", "ddd_", i, ".bmp");

		String g_file_name = g_chFile_name;//��������ת��Ϊ�ַ�����
		imwrite(g_file_name, g_imageROIResize);//д���ļ����ļ���

	}
	//==========================================================================


	//ð������ó���ȷ������
	bubbleSort(position, positionNumber);

	//=============================��KNN��=================================
	Ptr<KNearest> model = KNearest::create();//����KNN
	Ptr<TrainData> tData = prepare_train_data();//���ѵ������
	model->setDefaultK(3);    //�趨kֵ
	model->setIsClassifier(true);
	model->train(tData);//ѵ��
	cout << "ȼ�����ַ�Ϊ" << endl;

	//����λ����Ϣ˳���ȡ����д���ͼƬ
	for (int i = 0; i < 6; i++)
	{
		//��һ��·����ַ
		char g_chtestfile_name[255];
		sprintf(g_chtestfile_name, "%s%d%s", "ddd_", positionNumber[i], ".bmp");
		String g_testfile_name = g_chtestfile_name;
		//��ȡ����ͼƬ(�Ҷ�ͼ)
		Mat g_testfile1 = Mat(19, 17, CV_32FC1);
		g_testfile1 = imread(g_testfile_name, 0);
		//��ֵ������ͼƬ
		Mat g_testfileTh = Mat(19, 17, CV_32FC1);
		threshold(g_testfile1, g_testfileTh, 115, 255, THRESH_BINARY);
		//��һ����������
		Mat g_testfileRe = Mat(1, 323, CV_32FC1);
		g_testfileRe = g_testfileTh.reshape(0, 1);
		Mat_<float>g_testfile(1, 323);
		g_testfileRe.copyTo(g_testfile);
		//���
		float response = model->predict(g_testfile);
		cout << response << endl;
	}
	//========================================================================

	waitKey(0);
	return 0;
}

//------------------------------------��verifySizes( )������------------------------------------
//		 �������ж��Ƿ�Ϊ��ȷ�ľ���
//----------------------------------------------------------------------------------------------
bool verifySizes(RotatedRect mr)
{
	int min = 120;//��С�������
	int max = 2000;//����������
	float rmin = 0.15;//��С�߶ȺͿ�ȵı�������
	float rmax = 1.3;//���߶ȺͿ�ȵı�������
	int area = mr.size.height * mr.size.width;//���ο������С
	float r = (float)mr.size.width / (float)mr.size.height;//���ο�߶ȺͿ�ȵı���

														   /*�����жϣ�С����С������ơ��������������ơ�
														   С����С�������ƺʹ������������ƣ�
														   �ĸ�����ֻҪ����һ���ͷ���false*/
	if ((area < min || area > max) || (r < rmin || r > rmax))
	{
		return false;
	}
	else
	{
		return true;
	}
}

//--------------------------------��prepare_train_data( )������---------------------------------
//		 ������׼��ѵ������
//----------------------------------------------------------------------------------------------
Ptr<TrainData> prepare_train_data()
{
	char t_file_path[] = "train/";//��ַ��ͷ��
	int t_train_samples = 10;//�ļ�����
	int t_classes = 10;//�ļ�������

	int t_trainclassLabels[70];//ѵ�����ݱ�ǩ
							   //��������
	Mat t_srcImage;//ԭͼ
	Mat t_blackImage;//��ֵ��ͼ
	Mat t_blackImageResize;
	Mat t_reshapeImage;//��һ������
	char t_file[255];
	int i, j;
	int t = 0;

	for (i = 0; i < t_classes; i++)
	{
		if (i == 2 || i == 5 || i == 6) {
			continue;
		}
		else {
			for (j = 0; j < t_train_samples; j++)
			{
				//��ȡ�ļ�
				sprintf(t_file, "%s%d/0%d.bmp", t_file_path, i, j);//��һ�������ַ
				string t_file_name = t_file;//�����ַת��Ϊ�ַ�������
				t_srcImage = imread(t_file_name, 0);


				//��ֵ��
				threshold(t_srcImage, t_blackImage, 115, 255, THRESH_BINARY);
				//��һ��ѵ������
				t_reshapeImage = t_blackImage.reshape(0, 1);
				//�����ݸ�����Ӧ��
				t_reshapeImage.row(0).copyTo(t_trainData.row(t));
				t_trainclassLabels[t] = i;//ѵ����ǩ��������
				t++;

			}
		}
	}
	//cout << t << "\n";
	t_trainclass = Mat(70, 1, CV_32SC1, t_trainclassLabels);//ѵ����ǩ����ת���ɾ���
	Ptr<TrainData> tData = TrainData::create(t_trainData, ROW_SAMPLE, t_trainclass);//ѵ����֤�淶��

	return tData;//����ѵ������
}

//-------------------------------------��bubbleSort( )������------------------------------------
//		 ������ð���������λ����Ϣ
//----------------------------------------------------------------------------------------------
void bubbleSort(int arr1[], int arr2[])
{
	for (int i = 0; i < 6; i++) {
		//�Ƚ��������ڵ�Ԫ��   
		for (int j = 0; j < 6 - i - 1; j++) {
			if (arr1[j] > arr1[j + 1]) {
				int t = arr1[j];
				int t2 = arr2[j];
				arr1[j] = arr1[j + 1];
				arr2[j] = arr2[j + 1];
				arr1[j + 1] = t;
				arr2[j + 1] = t2;
			}
		}
	}
}

//--------------------------------------��ShowHelpText( )������---------------------------------
//		 ���������һЩ������Ϣ
//----------------------------------------------------------------------------------------------
void ShowHelpText()
{
	//�����ӭ��Ϣ��OpenCV�汾
	printf("\n\n\t\t\t������Ŀ����Ȼ�����µ�����ʶ��\n");
	printf("\n\n\t\t\t����������\n");
	printf("\n\n\t\t\tѧ�ţ�12122326\n");
	printf("\n\n\t\t\tָ����ʦ����Ϊ\n");
	printf("\n\n\t\t\t   ��ǰʹ�õ�OpenCV�汾Ϊ��" CV_VERSION);
	printf("\n\n  ----------------------------------------------------------------------------\n");
}