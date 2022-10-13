import cv2
import numpy as np
#确定座位颜色的种类
base_color = np.array(
              [[255, 204, 204],  #蓝色，GBR，下同
              [67, 163, 255],   #橙色
              [94, 111, 254],   #红色
              [203, 192, 255],  #粉色
              [226, 228, 229],  #灰色
              [50, 176, 102]]     #绿色
            )

base_color_name = ['蓝色', '橙色', '红色', '粉色', '灰色', '绿色']

#查找矩形框的颜色
def find_color(rect_img):
    color_cnt = [0]*6
    for i in range(rect_img.shape[0]):
        for j in range(rect_img.shape[1]):
            pixel = rect_img[i][j]
            # dis_list = []
            for c in range(base_color.shape[0]):
                dis = np.sqrt(np.sum(np.square(pixel-base_color[c])))
                if dis < 15:
                    color_cnt[c] += 1
    # 返回下标
    return color_cnt.index(max(color_cnt))

# 导入图片，图片放在程序所在目录
img_bgr = cv2.imread("cut.png")

#bgr转化为rgb图像
img_rgb = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2RGB)

# 转换为灰度图
gray = cv2.cvtColor(img_bgr, cv2.COLOR_BGR2GRAY)
print(type(gray))

# 使用局部阈值的自适应阈值操作进行图像二值化
# res,dst为threshold函数的返回值,res返回有没有读到图像 为true或者false，dst为返回的目标图像
res ,dst = cv2.threshold(gray, 0 ,255, cv2.THRESH_OTSU)

# 形态学去噪, getStructuringElement()函数返回指定形状和尺寸的结构元素（内核矩阵)
element = cv2.getStructuringElement(cv2.MORPH_CROSS, (3, 3))

# 开运算去噪 开运算就是先腐蚀后膨胀，可以用于降噪，消除一些小的像素点
dst = cv2.morphologyEx(dst, cv2.MORPH_OPEN, element)
print(dst.shape)
cv2.imshow('threshold',dst)

# 轮廓检测函数,第一个参数：输入的图像（要求图像为二值图） ，第二个参数：轮廓的模式，建立一个等级树结构的轮廓，第三个参数：轮廓的近似方法，
#           contours：返回的轮廓  hierarchy：每条轮廓对应的属性
contours, hierarchy = cv2.findContours(dst, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

# 绘制轮廓
cv2.drawContours(dst, contours, -1, (120, 0, 0), 2)

count = 0  # 总数
ares_avrg = 0  # 平均
seat_cnt = [0]*6 #各颜色统计
double_seat = 0 #双人座

#创建一个座位的类
class Seat:
    def __init__(self, rect, area,color_index,count):
        self.rect=rect
        self.area=area
        self.color_index=color_index
        self.count = count
        self.row = 0
        self.col = 0
        self.flag = 0

#座位数组
seats=[Seat]*0

#遍历所有轮廓
for cont in contours:
    # 计算包围形状的面积
    area = cv2.contourArea(cont)

    # 过滤面积小于50的形状
    if area < 50 or area > 2000:
        continue
    count += 1
    ares_avrg += area
    # 打印出每个轮廓对应的面积
    #print("{}-blob:{}".format(count, ares), end="  ")

    if area >1000:
        # 提取矩形坐标（x,y），根据轮廓绘制矩形边框
        rect = cv2.boundingRect(cont)
        rectl = [rect[0],rect[1],rect[2]//2,rect[3]]
        rectr = [rect[0]+rect[2]//2,rect[1],rect[2]//2,rect[3]]

        maskl = dst[rectl[1]:rectl[1] + rectl[3], rectl[0]:rectl[0] + rectl[2]]
        maskr = dst[rectr[1]:rectr[1] + rectr[3], rectr[0]:rectr[0] + rectr[2]]

        #左侧矩阵
        rect_bgrl = img_bgr[rectl[1]:rectl[1] + rectl[3], rectl[0]:rectl[0] + rectl[2]]

        color_idx = find_color(rect_bgrl)
        seat_cnt[color_idx] += 2

        #右侧矩阵
        rect_bgrr = img_bgr[rectr[1]:rectr[1] + rectr[3], rectr[0]:rectr[0] + rectr[2]]

        double_seat += 1;

        # 将座位的实例化对象加入到数组中
        seats.append(Seat(rectr, area / 2, color_idx, count))
        seats.append(Seat(rectl, area / 2, color_idx, count))
        count = count + 1

        # 绘制矩形
        cv2.rectangle(img_bgr, rectl, (0, 0, 255), 1)
        cv2.rectangle(img_bgr, rectr, (0, 0, 255), 1)

        # 在左上角写上编号
        cv2.putText(img_bgr, str(count-1), (rectr[0], rectr[1]), cv2.FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1)
        cv2.putText(img_bgr, str(count), (rectl[0], rectl[1]), cv2.FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1)
    else:
        # 提取矩形坐标（x,y），根据轮廓绘制矩形边框
        rect = cv2.boundingRect(cont)

        # 提取mask,二值图的矩形边框内的像素块
        mask = dst[rect[1]:rect[1] + rect[3], rect[0]:rect[0] + rect[2]]
        rect_bgr = img_bgr[rect[1]:rect[1] + rect[3], rect[0]:rect[0] + rect[2]]
        rect_rgb = cv2.cvtColor(rect_bgr, cv2.COLOR_BGR2RGB)

        color_idx = find_color(rect_bgr)
        seat_cnt[color_idx] += 1

        # 将座位的实例化对象加入到数组中
        seats.append(Seat(rect, area, color_idx,count))

        # 绘制矩形
        cv2.rectangle(img_bgr, rect, (0, 0, 255), 1)

        # 防止编号到图片之外（上面）,因为绘制编号写在左上角，所以让最上面的y小于10的变为10个像素
        y = 10 if rect[1] < 10 else rect[1]
        # 在左上角写上编号
        cv2.putText(img_bgr, str(count), (rect[0], y), cv2.FONT_HERSHEY_COMPLEX, 0.4, (0, 0, 0), 1)

#以y坐标进行排序
seats = sorted(seats,key=lambda x:x.rect[1],reverse=True)

row = 1
for i in range(len(seats)):
    if i>0 and seats[i-1].rect[1]-seats[i].rect[1]>20:
        row = row + 1
    seats[i].row = row

seats = sorted(seats,key=lambda x:x.rect[0],reverse=True)
col = 1
for i in range(len(seats)):
    if i>0 and seats[i-1].rect[0]-seats[i].rect[0]>20:
        col = col + 1
    seats[i].col = col

seats = sorted(seats,key=lambda x:x.count,reverse=False)

print('座位个数', count, ' 总面积', ares_avrg)
print('包含双人座', double_seat)
# print("平均面积:{}".format(round(ares_avrg / count, 2)))
for s in range(len(seat_cnt)):
    print('颜色：'+str(base_color_name[s]), '数量：'+str(seat_cnt[s]))
    for i in range(len(seats)):
        if seats[i].color_index == s:
            print('第'+str(seats[i].row)+'行',' 第'+str(seats[i].col)+'列')

cv2.imshow('imgshow', img_bgr)  # 显示原始图片（添加了外接矩形）

cv2.imshow("dst", dst)  # 显示灰度图

cv2.waitKey()