import os
import numpy as np
import cv2
import random
import threading
import time


NEED_ORIGIN = 1

frame_lb = 1
frame_ub = 500
people_cnt = 10
col = {}
back = cv2.imread('back/' + str(1) + '_b.png', cv2.IMREAD_UNCHANGED)
sem = threading.Semaphore(16)


def get_col(v):
    if col.get(v) is None:
        col[v] = (random.randint(0, 255), random.randint(
            0, 255), random.randint(0, 255), 255)
    return col[v]


def mask(back, seg, col):
    shape = seg.shape
    result = np.full(shape, 255, dtype=np.uint8)
    # back = back.astype(np.int32)
    # seg = seg.astype(np.int32)
    c = seg[:, :, 3] != back[:, :, 3]

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
        bbox_file = open('seg/' + str(j) + '_' + str(i) + '_box.txt')
        bbox = list(map(lambda x: int(x), bbox_file.read().split()))
        lenx = bbox[2] - bbox[0] + 1
        leny = bbox[3] - bbox[1] + 1

        image = cv2.imread('seg/' + str(j) + '_' + str(i) + '_s.png', cv2.IMREAD_UNCHANGED)

        result = mask(back[bbox[1]:bbox[3] + 1, bbox[0]
                      : bbox[2] + 1, :], image, get_col(j))
        cv2.imwrite('result/segment_' + str(j).zfill(3) +
                    '_' + str(i).zfill(4) + '.png', result)
        print('result/segment_' + str(j).zfill(3) +
              '_' + str(i).zfill(4) + '.png')
        if NEED_ORIGIN:
            result = get_origin(
                result, prime[bbox[1]:bbox[3] + 1, bbox[0]: bbox[2] + 1, :])
            cv2.imwrite('result/origin_' + str(j).zfill(3) +
                        '_' + str(i).zfill(4) + '.png', result)


if not os.path.exists('result'):
    os.makedirs('result')

for i in range(frame_lb, frame_ub + 1):
    if NEED_ORIGIN:
        prime = cv2.imread('prime/image_' + str(i).zfill(4) +
                           '.png', cv2.IMREAD_UNCHANGED)
    for j in range(1, people_cnt + 1):
        if not os.path.exists('seg/' + str(j) + '_' + str(i) + '_box.txt'):
            print('no %d %d' % (i, j))
            continue
        t = threading.Thread(target=work(i, j), args=(i, j))
        t.start()
