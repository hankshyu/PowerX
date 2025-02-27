#include <iostream>
#include <assert.h>
#include <unordered_map>
#include "vector"

#include "boost/polygon/polygon.hpp"

#include "isotropy.hpp"
#include "cord.hpp"
#include "fcord.hpp"
#include "line.hpp"
#include "interval.hpp"
#include "rectangle.hpp"
#include "tile.hpp"

// #include "doughnutPolygon.hpp"

namespace gtl = boost::polygon;
using namespace boost::polygon::operators;

// int main(int argc, char const *argv[]){
//     Rectangle r1(3, 9, 11, 17);
//     Tile t1(7, 9, 11, 14);

//     std::cout << boost::polygon::delta<Rectangle>(boost::polygon::rectangle_data<len_t>(3, 9, 11, 17), eOrientation2D::HORIZONTAL) << std::endl;

// }
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Create a blank white image
    cv::Mat image(500, 800, CV_8UC3, cv::Scalar(255, 255, 255));

    // Draw a rectangle (top-left corner: (50, 50), bottom-right corner: (300, 200))
    cv::rectangle(image, cv::Point(50, 50), cv::Point(300, 200), cv::Scalar(0, 0, 255), 3); // Red

    // Draw a circle (center: (500, 150), radius: 50)
    cv::circle(image, cv::Point(500, 150), 50, cv::Scalar(0, 255, 0), 3); // Green

    // Draw a rectilinear polygon (L-shape)
    std::vector<cv::Point> pts = {cv::Point(350, 250), cv::Point(450, 250), cv::Point(450, 350),
                                  cv::Point(400, 350), cv::Point(400, 300), cv::Point(350, 300)};
    cv::polylines(image, pts, true, cv::Scalar(255, 0, 0), 3); // Blue

    // Show the image
    cv::imshow("Drawing Test", image);
    cv::waitKey(0);  // Wait for a keystroke before closing
    return 0;
}