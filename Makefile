BUILD = cx1

vtkinclude.cx1 = -I/apps/vtk/5.10.1/include/vtk-5.10
vtkinclude.labs = -I/usr/include/vtk-5.8
vtkinclude.mac = -I/usr/local/opt/vtk/include/vtk-5.10

vtklibs.cx1 = -L/apps/vtk/5.10.1/lib/vtk-5.10 -lvtkCommon -lvtksys
vtklibs.labs = -L/usr/lib/vtk-5.8 -L/usr/lib/nvidia-current -lcuda
vtklibs.mac = -L/usr/local/opt/vtk/lib/vtk-5.10

# CXXFLAGS = -g -O3 -Wall -Wno-deprecated ${vtkinclude.${BUILD}}
CXXFLAGS = ${vtkinclude.${BUILD}}
NVCCFLAGS = ${vtkinclude.${BUILD}}

OBJS = ACA2-2013.o Mesh.o Smooth.o SVD2x2.o SmoothVector.o

LIBS = ${vtklibs.${BUILD}} -lvtkIO -lvtkFiltering -lvtkCommon -lvtksys -ldl -lpthread
#LIBS = ${vtklibs.${BUILD}} -lvtkIO -lvtkFiltering -lvtkCommon -lvtkzlib -lvtkexpat -lvtksys -ldl -lpthread


TARGET = ACA2-2013

$(TARGET):	$(OBJS)
	@$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	@$(TARGET)

SmoothVector.o:
	nvcc -arch=sm_20 -c -dlink -o SmoothVector.o SmoothVector.cu

SVD2x2.o:
	nvcc -arch=sm_20 -c -dlink -o SVD2x2.o SVD2x2.cu 

Mesh.o:
	nvcc -arch=sm_20 -c -dlink -o Mesh.o Mesh.cu
 
clean:
	@rm -f $(OBJS) $(TARGET)
