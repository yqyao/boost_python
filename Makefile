CXX = g++
CXXFLAGS = -g -Wall
INCLUDE = -I ./include -I/home/zhaoyu/software/boost-1.58.0-mt/include \
-I/home/yaoyongqiang/anaconda2/include/python2.7
LDFLAGS = -L ./lib_faceall -lfaceall_face -lopencv_core \
-lopencv_imgproc -lopencv_highgui -L/home/zhaoyu/software/boost-1.58.0-mt/lib -lboost_python-mt

TARGET = detect
SRC = test_detect
all:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LDFLAGS) $(SRC).cpp \
	-shared -fPIC -o $(TARGET).so 