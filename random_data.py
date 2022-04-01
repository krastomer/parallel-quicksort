import random

size = 100
filename = 'data.txt'

arr = [random.random() for _ in range(size)]

with open('data.txt', 'w') as f:
    f.write(str(size) + '\n')
    for a in arr:
        f.write(str(a) + '\n')


f.close()
