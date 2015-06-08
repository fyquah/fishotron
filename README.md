# Fishotron

This is the source code of [fish-o-tron](http://fishotron.fyquah.me/), built at [Fishackathon](http://fishackathon2015.challengepost.com/) London. This source is used in the length calculation algorithm. For the code of the web-app in the link above, visit [here](https://github.com/fyquah95/fishotron-web).

This source code uses multithread operations to handle image pooling from the webcam and parallel processes to send images and data to the server. In non-technical terms, while the program is sending data to the server, the camera and length-recording subroutines runs normally without synchronous blocking. 

## Dependencies

* [OpenCV](http://opencv.org/)
* [CMake](http://www.cmake.org/)
* [Chilitags](https://github.com/chili-epfl/chilitags)

Required only if using Android's camera rather than webcam:

* [Android IP Webcam](https://play.google.com/store/apps/details?id=com.pas.webcam&hl=en_GB)
* [libcurl](http://curl.haxx.se/libcurl/)

## Setting Up

~~~bash
mkdir build
cd build
cmake ../
make
./json ip-address-of-camera
~~~

## Algorithm Overview

1. Detect the chilitags on the image
2. Warp the perspective of the image and clip the portion of image surrounded by the 4 corners
3. Convert the clipped portion into a grayscale image
4. Carry out edge detection ([Sobel Operator](https://www.wikiwand.com/en/Sobel_operator)) on the image
5. Filter out pixels which are not representative of the distribution
6. Carry out [minimum bounding rectangle algorithm](https://www.wikiwand.com/en/Minimum_bounding_rectangle) on the filtrate of the above process to determine the rectangles points
6. Calculate the length and width of the rectangle based on information obtain from several frame
7. Remove annomalies from the obtain data
8. Output the assumed length and width after annomaly detection

## Architecture Overview

1. Calculate the length and width of camera using above the algorithm
2. Take a snapshot of the image
3. Spawn a new process to upload the image to AWS S3 and data to public fish server 
4. Terminate child process