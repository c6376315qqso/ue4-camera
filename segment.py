import os
import numpy as np
import cv2
import random

frame_lb = 1
frame_ub = 100
people_cnt = 10
col = {}


def get_col(v):
    if col.get(v) is None:
        col[v] = (random.randint(0, 255), random.randint(
            0, 255), random.randint(0, 255))
    return col[v]


def mask(back, seg, col):
    shape = seg.shape
    result = np.full(shape, 255, dtype=np.uint8)
    back = back.astype(np.int32)
    seg = seg.astype(np.int32)
    c = ((seg - back) ** 2).sum(axis=-1)
    result[c > 50] = np.array(col)
    # print(c > 50)
    return result


if not os.path.exists('result'):
    os.makedirs('result')

for i in range(frame_lb, frame_ub + 1):
    for j in range(1, people_cnt + 1):
        if not os.path.exists('seg/' + str(j) + '_' + str(i) + '_box.txt'):
            print('no %d %d' % (i, j))
            continue
        bbox_file = open('seg/' + str(j) + '_' + str(i) + '_box.txt')
        bbox = list(map(lambda x: int(x), bbox_file.read().split()))
        lenx = bbox[2] - bbox[0] + 1
        leny = bbox[3] - bbox[1] + 1
        file = open('seg/' + str(j) + '_' + str(i) + '_num.txt')
        num = list(map(lambda x: int(x), file.read()[:-1].split(' ')))
        image = np.zeros((leny, lenx, 3), dtype=np.uint8)
        for x in range(0, leny):
            for y in range(0, lenx):
                binary = bin(num[x * lenx + y]).replace('0b', '').zfill(32)
                image[x][y][0] = int(binary[0:8], 2)
                image[x][y][1] = int(binary[8:16], 2)
                image[x][y][2] = int(binary[16:24], 2)
                # image[x][y][3] = int(binary[24:32], 2)
        back = cv2.imread('back/' + str(i) + '_b.png')
        result = mask(back[bbox[1]:bbox[3] + 1, bbox[0]:bbox[2] + 1, :], image, get_col(j))
        cv2.imwrite('result/segment_' + str(j).zfill(3) +
                    '_' + str(i).zfill(4) + '.png', result)
        print('result/segment_' + str(j).zfill(3) +
              '_' + str(i).zfill(4) + '.png')
