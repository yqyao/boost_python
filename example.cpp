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
long get_handle()
{
    faceall_handle_t detect_and_landmark_handle;
    detect_and_landmark_handle= faceall_facesdk_detect_and_landmark_get_instance();
    std::cout << "load model" << endl;
    return (long)detect_and_landmark_handle;
}

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

BOOST_PYTHON_MODULE(detect)
{
    using namespace boost::python;
    def("Detect_landmark", &Detect_landmark);
    def("get_handle", &get_handle);

}