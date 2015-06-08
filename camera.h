#ifndef _CAMERA_H
#define _CAMERA_H

#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <curl/curl.h>

namespace camera {

    const unsigned N_THREADS = 1;

    struct thread_data {
        int thread_id;
        std::vector<char> * containerPtr;
        int containerSize;
    };

    struct thread_entry {
        pthread_t thread;
        thread_data tdata;
    };

    void setIpAddress(const std::string & str);
    size_t curlWriteCallBack(char* buf, size_t size, size_t nmemb, void* data);
    void * poolImage(void * threadarg);
    void addNewThreadToQueue(std::queue<thread_entry*> & q);
    thread_entry * getOldestThread(std::queue<thread_entry *> & q, void * status);
    void poolCamera(void (*fnc)(cv::Mat));
}

#endif