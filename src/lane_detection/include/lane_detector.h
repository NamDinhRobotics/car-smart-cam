#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <Python.h>
#include <opencv2/opencv.hpp>
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <iostream>

using namespace cv;
namespace py = boost::python;
namespace np = boost::python::numpy;

class LaneDetector {
   private:
        py::object process_img;
   public:

    void Init() {
        // Set your python location.
        // wchar_t str[] = L"/home/vietanhdev/miniconda3/envs/example_env";
        // Py_SetPythonHome(str);

        setenv("PYTHONPATH", "./python_lane_detector", 1);

        Py_Initialize();
        np::initialize();
    }

    LaneDetector() {

        try {
            // Initialize boost python and numpy
            Init();

            // Import module
            py::object main_module = py::import("__main__");

            // Load the dictionary for the namespace
            py::object mn = main_module.attr("__dict__");

            // Import the ConfigParser module into the namespace
            py::exec("import init_lane_detector", mn);

            // Create the locally-held RawConfigParser object
            py::object image_processor =
                py::eval("init_lane_detector.get_lane_finder()", mn);
            this->process_img = image_processor.attr("process_image");

        } catch (py::error_already_set&) {
            PyErr_Print();
            exit(1);
        }

    }

    cv::Mat detect_lane(const cv::Mat& img) {

        cv::Mat clone_img;
        cv::resize(img, clone_img, cv::Size(1280, 720));
        np::ndarray nd_img = ConvertMatToNDArray(clone_img);
        np::ndarray output_img = py::extract<np::ndarray>(process_img(nd_img, false));
        cv::Mat mat_img = ConvertNDArrayToMat(output_img);

        cv::imshow("Debug Lane", mat_img);
        cv::waitKey(1);

        return mat_img;

    }

    // Function to convert from cv::Mat to numpy array
    np::ndarray ConvertMatToNDArray(const cv::Mat& mat) {
        py::tuple shape = py::make_tuple(mat.rows, mat.cols, mat.channels());
        py::tuple stride =
            py::make_tuple(mat.channels() * mat.cols * sizeof(uchar),
                        mat.channels() * sizeof(uchar), sizeof(uchar));
        np::dtype dt = np::dtype::get_builtin<uchar>();
        np::ndarray ndImg =
            np::from_data(mat.data, dt, shape, stride, py::object());

        return ndImg;
    }


    // Function to convert from numpy array to cv::Mat
    cv::Mat ConvertNDArrayToMat(const np::ndarray& ndarr) {
        int length =
            ndarr.get_nd();  // get_nd() returns num of dimensions. this is used as
                            // a length, but we don't need to use in this case.
                            // because we know that image has 3 dimensions.
        const Py_intptr_t* shape =
            ndarr.get_shape();  // get_shape() returns Py_intptr_t* which we can get
                                // the size of n-th dimension of the ndarray.
        char* dtype_str = py::extract<char*>(py::str(ndarr.get_dtype()));

        // Variables for creating Mat object
        int rows = shape[0];
        int cols = shape[1];
        int channel = length == 3 ? shape[2] : 1;
        int depth;

        // Find corresponding datatype in C++
        if (!strcmp(dtype_str, "uint8")) {
            depth = CV_8U;
        } else if (!strcmp(dtype_str, "int8")) {
            depth = CV_8S;
        } else if (!strcmp(dtype_str, "uint16")) {
            depth = CV_16U;
        } else if (!strcmp(dtype_str, "int16")) {
            depth = CV_16S;
        } else if (!strcmp(dtype_str, "int32")) {
            depth = CV_32S;
        } else if (!strcmp(dtype_str, "float32")) {
            depth = CV_32F;
        } else if (!strcmp(dtype_str, "float64")) {
            depth = CV_64F;
        } else {
            std::cout << "Wrong dtype error" << std::endl;
            return cv::Mat();
        }

        int type = CV_MAKETYPE(
            depth, channel);  // Create specific datatype using channel information

        cv::Mat mat = cv::Mat(rows, cols, type);
        memcpy(mat.data, ndarr.get_data(), sizeof(uchar) * rows * cols * channel);

        return mat;
    }
};

#endif