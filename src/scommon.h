
#ifndef SCOMMON_H
#define SCOMMON_H

#include <QDebug>
#include <QImage>
#include <opencv2/opencv.hpp>
#include <vector>

/*******常用的转换函数********/



cv::Mat qimage_to_mat(QImage image);

QImage mat_to_qimage(const cv::Mat& mat);

QByteArray mat_to_byteArray(const cv::Mat &image);

cv::Mat byteArray_to_mat(const QByteArray & byteArray);

QByteArray floatArray_to_byteArray(std::vector<float>& float_array);

std::vector<float> byteArray_to_floatArray(QByteArray& array);

std::string Mat2Base64(const cv::Mat &img, std::string imgType);

cv::Mat Base2Mat(std::string &base64_data);

std::string base64Encode(const unsigned char* Data, int DataByte);

std::string base64Decode(const char* Data, int DataByte);


#endif // SCOMMON_H
