#Animation of simulation
#see report for details
#demonstration: https://www.youtube.com/watch?reload=9&v=2hzznVBbRWY
#if running on Windows (rather than linux) need to change system call "clear" to "cls"

import time as Time
from numpy import *
import math
import os

#parameters can be varied


n = 100
a = 0.01
e = 0.03
dt = 0.008
MAXTIME = 100
#fig,ax = subplots(2,2,figsize=(20,13))


import random
random.seed(1)

get_time = lambda: int(round(Time.time()*1000))


def update_omegas(omegas,a,flashed):
    n = len(omegas)
    orig = omegas.copy()
    for i in range(n):
        if not flashed[i]:
            for j in range(n):
                if flashed[j]:
                    omegas[i] += a*(orig[j] - orig[i])
    return omegas


def update_thetas_time(time, omegas, thetas):
    thetas = [t + o*dt for o,t in zip(omegas, thetas)]
    return thetas

def update_thetas_flash(thetas,a):
    theta_cp = thetas.copy()
    thetas = thetas-e*sin([2*pi*theta for theta in theta_cp])
    return thetas

def fireflies(n,a):
    
    time = 0
    flashed = [False for _ in range(n)]
    f_chars = [' ' for _ in range(n)]
    flash = False
    thetas = [random.uniform(0,1) for _ in range(n)]
    omegas = [random.uniform(0.1,10) for _ in range(n)]
    while (time < MAXTIME):
        t1 = get_time()
        if flash:
            thetas = update_thetas_flash(thetas,a)
            omegas = update_omegas(omegas,a,flashed)
            flash = False
        else:
            thetas = update_thetas_time(time, omegas, thetas)
        flashed = [False for _ in range(n)]
        f_chars = [' ' for _ in range(n)]
        for i in range(n):
            if thetas[i]>=1:
                flashed[i] = True
                f_chars[i] =u"\u25CF" #u"\u2b24" #
                flash = True
                thetas[i] = thetas[i]%1
        thetas = [(round(theta,4))%1 for theta in thetas]
        time = time + dt
        #s = math.floor(math.sqrt(n))
        f_2d = []
        for i in range(10):
            f_2d.append(f_chars[i*10:(1+i)*10])

        for f in f_2d:
            print(*f,sep='      ',end='\n\n\n')
        t2 = get_time()
        if (t2-t1<0.03*1000):
            Time.sleep(0.03-(t2-t1)/1000)
        else:
            print('too short')

        os.system('clear')
        print(round(time,3),end='\n')
#        for i in range(n):
#            if i%s==0:
#                print('')
#            print(f_chars[i],end=' ')


    
fireflies(n,a)
