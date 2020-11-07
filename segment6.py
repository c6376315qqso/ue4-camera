import os
import numpy as np
import cv2
import random
import threading
import time


NEED_ORIGIN = 1

frame_lb = 2
frame_ub = 4
people_cnt = 10
col = {}
back = cv2.imread('panoramic_getor1/back/' + str(1) + '_b.png')
sem = threading.Semaphore(16)


def get_col(v):
    if col.get(v) is None:
        col[v] = (random.randint(0, 255), random.randint(
            0, 255), random.randint(0, 255))
    return col[v]


def mask(back, seg, col):
    shape = seg.shape + (3,)
    result = np.full(shape, 255, dtype=np.uint8)
    c = abs(seg - back) < 5

    result[c] = np.array(col)
    return result


def get_origin(msk, prime):
    shape = msk.shape
    result = np.full(shape, 255, dtype=np.uint8)
    c = msk[:, :, :3].sum(-1) != 255 * 3
    result[c] = prime[c]
    return result


prime = None


def work(i, j):
    global prime
    with sem:
        bbox_file = open('panoramic_getor1/seg/' +
                         str(j) + '_' + str(i) + '_box.txt')
        bbox = list(map(lambda x: int(x), bbox_file.read().split()))
        lenx = bbox[2] - bbox[0] + 1
        leny = bbox[3] - bbox[1] + 1
        image_d = np.load('panoramic_getor1/seg/' +
                          str(j) + '_' + str(i) + '_d.npy')
        image = cv2.imread('panoramic_getor1/seg/' +
                           str(j) + '_' + str(i) + '_s.png')
        print(image_d)
        result = mask(prime_d[bbox[1]:bbox[3] + 1, bbox[0]: bbox[2] + 1], image_d, get_col(j))
        cv2.imwrite('panoramic_getor1/result/segment_' + str(j).zfill(3) +
                    '_' + str(i).zfill(4) + '.png', result)
        print('panoramic_getor1/result/segment_' + str(j).zfill(3) +
              '_' + str(i).zfill(4) + '.png')
        if NEED_ORIGIN:
            result = get_origin(
                result, prime[bbox[1]:bbox[3] + 1, bbox[0]: bbox[2] + 1, :])
            cv2.imwrite('panoramic_getor1/result/origin_' + str(j).zfill(3) +
                        '_' + str(i).zfill(4) + '.png', result)


if not os.path.exists('panoramic_getor1/result'):
    os.makedirs('panoramic_getor1/result')

for i in range(frame_lb, frame_ub + 1):
    if not os.path.exists('panoramic_getor1/dep/' + str(i) + '_d.npy'):
        print('no %d' % (i))
        continue
    prime_d = np.load('panoramic_getor1/dep/' + str(i) + '_d.npy')
    prime = cv2.imread('panoramic_getor1/prime/image_' + str(i).zfill(4) +
                       '.png')
    for j in range(1, people_cnt + 1):
        if not os.path.exists('panoramic_getor1/seg/' + str(j) + '_' + str(i) + '_d.npy'):
            print('no %d %d' % (i, j))
            continue
        t = threading.Thread(target=work(i, j), args=(i, j))
        t.start()
