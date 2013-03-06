BUILD = labs

vtkinclude.cx1 = -I/apps/vtk/5.10.1/include/vtk-5.10 -I/apps/cuda/5.0.35/include/ -I./
vtkinclude.labs = -I/usr/include/vtk-5.8 -I./ -I/usr/local/cuda/include/
vtkinclude.mac = -I/usr/local/opt/vtk/include/vtk-5.10 -I/usr/local/cuda/include/ -I./

vtklibs.cx1 = -L/apps/vtk/5.10.1/lib/vtk-5.10 -lvtkCommon -lvtksys -L/apps/cuda/5.0.35/lib/ -L/apps/cuda/5.0.35/lib64/
vtklibs.labs = -L/usr/lib/vtk-5.8 -L/usr/local/cuda/lib64/ -L/usr/lib/nvidia-current
vtklibs.mac = -L/usr/local/opt/vtk/lib/vtk-5.10 -L/usr/local/cuda/lib/

CXX = g++
CXXFLAGS = -c -O3 -Wall -Wno-deprecated ${vtkinclude.${BUILD}}

OBJS = ACA2-2013.o Mesh.o

INCS = CUDATools.h Smooth.hpp Color.hpp

LIBS = ${vtklibs.${BUILD}} -lcuda -lcudart -lvtkIO -lvtkFiltering -lvtkCommon -lvtksys -ldl -lpthread

all: ACA2-2013

clean:
	rm -f $(OBJS) ACA2-2013

Mesh.o: $(INCS) Mesh.cpp Mesh.hpp
	$(CXX) $(CXXFLAGS) Mesh.cpp -o Mesh.o

#Smooth.o: $(INCS) Smooth.cpp Smooth.hpp
#	$(CXX) $(CXXFLAGS) Smooth.cpp -o Smooth.o

ACA2-2013.o: $(INCS) ACA2-2013.cpp
	$(CXX) $(CXXFLAGS) ACA2-2013.cpp -o ACA2-2013.o

ACA2-2013: $(OBJS)
	$(CXX) $(OBJS) -o ACA2-2013 $(LIBS)
