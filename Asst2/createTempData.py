import random
import os
import shutil
import filecmp
import glob
FILE_NUM=500
DIR_NUM=100

FILE_PER_DIR=FILE_NUM/DIR_NUM

extension = [".tmp"]



size = [1,2,4,8,16,32,64,128,256,512]

for dirnum in range(0,DIR_NUM):
    if not os.path.exists("./toencode/"+str(dirnum)):
        os.makedirs("./toencode/"+str(dirnum))
    for a in range(0,int(FILE_PER_DIR)):
        with open("./toencode/"+str(dirnum)+"/"+str(random.randint(0,10000))+extension[random.randint(0,6)], 'wb') as fout:
            fout.write(os.urandom(102400*size[random.randint(0,9)]))

shutil.copytree("./toencode","./tocheck")
