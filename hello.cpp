
#include <boost/python.hpp>
//#include "test_detect.cpp"
char const* test()
{
    return "hello";
}
BOOST_PYTHON_MODULE(detect)
{
    using namespace boost::python;
    def("greet", test);
}