#include <iostream>
#include <face/faceall_sdk.h>
#include <omp.h>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <errno.h>
#include <memory.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <face/cJSON.h>
#include <boost/python.hpp>

using namespace std;
/*
这是释放python GIL的类，如果要利用多核，必须释放python的GIL
 */
class releaseGIL{
public:
    inline releaseGIL(){
        save_state = PyEval_SaveThread();
    }

    inline ~releaseGIL(){
        PyEval_RestoreThread(save_state);
        save_state = NULL;
    }
private:
    PyThreadState *save_state;
};
/*
big circle for test
 */
float big_circle(float lon1,float lat1,float lon2,float lat2)
{
    float radius = 3956.0;
    float pi = 3.14159265;
    float x = pi/180.0;  
    float a,b,theta,c;  
  
    a = (90.0-lat1)*(x);  
    b = (90.0-lat2)*(x);  
    theta = (lon2-lon1)*(x); 
    c = acosf((cosf(a)*cosf(b)) + (sinf(a)*sinf(b)*cosf(theta)));   
    return radius*c ; 
}
/*
do the big_circle about 1000000 
 */
void test()
{
    releaseGIL unlockGIL = releaseGIL();
    float lon1 = -72.345;
    float lat1 = 34.243;
    float lon2 = -61.823;
    float lat2 = 54.826;
    float result = 0; 
    for(int i = 0; i < 1000000; i++)
        result = big_circle(lon1, lat1, lon2, lat2);
}
/*
获取人脸检测的句柄
此部分在后来的多线程容易出错，申请的内存会常常被python的GC回收
 */
long get_handle()
{
    faceall_handle_t detect_and_landmark_handle;
    detect_and_landmark_handle= faceall_facesdk_detect_and_landmark_get_instance();
    std::cout << "load model" << endl;
    return (long)detect_and_landmark_handle;
}
/*
人脸检测，boost::python使用，可以看出boost::python和python一样类似写法
 */
boost::python::list Detect_landmark(long detect_and_landmark_handle_i, std::string img_path)
{
        releaseGIL unlockGIL = releaseGIL();
        using namespace boost::python;
       
        void* detect_and_landmark_handle = (void*)detect_and_landmark_handle_i;

        cv::Mat input_image = cv::imread(img_path);
        faceall_image_t image;


        image.width = input_image.cols;
        image.height = input_image.rows;
        image.channels = input_image.channels();


        int total = input_image.cols * input_image.rows * input_image.channels();
        image.data = (unsigned char*)malloc(sizeof(unsigned char)*total);
        memcpy(image.data,input_image.data,total);

        faceall_rect_t *face_rects;
        faceall_landmark_t *face_landmark_array;

        unsigned int counts=0 ;

        faceall_facesdk_detect_and_landmark(detect_and_landmark_handle, image, &face_rects, &face_landmark_array, &counts);
        boost::python::list result;
        for (int face_idx = 0; face_idx < counts; ++face_idx)
        {
            boost::python::dict d;
            boost::python::list rect,landmark;
            rect.append(face_rects[face_idx].left);
            rect.append(face_rects[face_idx].top);
            rect.append(face_rects[face_idx].right);
            rect.append(face_rects[face_idx].bottom);

            d["rect"] = rect;
           
            for (int j = 0; j < face_landmark_array[face_idx].n_points; j++)
            {
                landmark.append(make_tuple(face_landmark_array[face_idx].points[j].x, face_landmark_array[face_idx].points[j].y));
            }
            d["landmark"] = landmark;
            result.append(d);

            d["score"] = face_rects[face_idx].score;
        }
        free(image.data);
        faceall_facesdk_detect_and_landmark_release_result(face_rects,face_landmark_array,counts);
        std::cout << "mult2" << std::endl;
        return result;
}
/*
生成pyhton可调用库的关键字
 */
BOOST_PYTHON_MODULE(detect)
{
    using namespace boost::python;
    def("Detect_landmark", &Detect_landmark);
    def("get_handle", &get_handle);
    def("test", &test);

}