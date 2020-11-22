from shapely import wkt
import matplotlib.pyplot as plt

def printPoly(poly, ax):
    points = poly.exterior.coords.xy
    line, = ax.plot(points[0], points[1], 'go-')
    #for i in range(0, len(points[0])):
        #ax.plot(points[0][i],points[1][i])

f = open("polys.wkt")
line = f.readline()
polys = []
while line != '':
    polys.append(wkt.loads(line))
    line = f.readline()

fig  = plt.figure(1)
ax = fig.add_subplot(121)

for poly in polys:
    printPoly(poly, ax)
ax.grid()
plt.show()
