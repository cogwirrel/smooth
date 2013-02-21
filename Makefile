BUILD = cx1

vtkinclude.cx1 = -I/apps/vtk/5.10.1/include/vtk-5.10
vtkinclude.labs = -I/usr/include/vtk-5.8
vtkinclude.mac = -I/usr/local/opt/vtk/include/vtk-5.10

vtklibs.cx1 = -L/apps/vtk/5.10.1/lib/vtk-5.10 -lvtkCommon -lvtksys
vtklibs.labs = -L/usr/lib/vtk-5.8 -L/usr/lib/nvidia-current -lcuda
vtklibs.mac = -L/usr/local/opt/vtk/lib/vtk-5.10

CXXFLAGS = -g -O3 -Wall -Wno-deprecated ${vtkinclude.${BUILD}}


OBJS = ACA2-2013.o Mesh.o Smooth.o SVD2x2.o SmoothVector.o

LIBS = ${vtklibs.${BUILD}} -lvtkIO -lvtkFiltering -lvtkCommon -lvtksys -ldl -lpthread
#LIBS = ${vtklibs.${BUILD}} -lvtkIO -lvtkFiltering -lvtkCommon -lvtkzlib -lvtkexpat -lvtksys -ldl -lpthread


TARGET = ACA2-2013

$(TARGET):	$(OBJS)
	@$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	@$(TARGET)

SmoothVector.o:
	nvcc -c -o SmoothVector.o SmoothVector.cu 

clean:
	@rm -f $(OBJS) $(TARGET)
