# 一、前言：
本项目包含c++实现和python实现，需要包含opencv
**[opencv](https://github.com/Itseez/opencv)** 
# 二、实现：
本项目通过对图像进行灰度处理，转化为单通道图像，再进行二值化处理。对于得到的二值图，
进行形态学去噪，通过开运算(先腐蚀后膨胀)去噪，得到噪声值较少的二值图。


![Image](https://user-images.githubusercontent.com/101049914/195580731-dcb72572-5356-4f69-a302-a8e7c554ebf1.png)


![Image](https://user-images.githubusercontent.com/101049914/195580796-29a85924-a9c7-425e-a81d-36483deb477a.png)



![Image](https://user-images.githubusercontent.com/101049914/195580847-0fd003f5-15f2-4f93-a968-26a6e41c7da5.png)



根据二值图绘制轮廓，并绘制外接矩阵。


![Image](https://user-images.githubusercontent.com/101049914/195580926-ce1bf432-8902-4b88-bb11-46fe1dadad64.png)



根据外接矩阵提取图像片段，识别颜色，并根据坐标相对位置，识别座位的行列。
