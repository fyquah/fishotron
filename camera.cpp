#include "camera.h"
using namespace std;
using namespace cv;
// #define DEBUG 1
namespace camera {
    std::string url;

    void setIpAddress(const string & ip) {
        url = "http://" + ip + "/shot.jpg";
    }

    size_t curlWriteCallBack(char* buf, size_t size, size_t nmemb, void* data){
        //callback must have this declaration
        //buf is a pointer to the data that curl has for us
        //size*nmemb is the size of the buffer

        thread_data *t = (thread_data *) data;
        for (int c = 0; c<size*nmemb; c++) {
            t->containerPtr->push_back(buf[c]);
        }
        return size*nmemb; //tell curl how many bytes we handled
    }

    void *getImage(void * threadarg) {
        CURL * curl = curl_easy_init();

#ifdef DEBUG
        thread_data * tdata = (thread_data *) threadarg;
        cerr << "Reading thread " << tdata->thread_id << endl;
#endif
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, threadarg);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &curlWriteCallBack);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
#ifdef DEBUG
        cerr << tdata->thread_id << "Contianer size from thread " << tdata->containerSize << endl;
        cerr << "Thread " << tdata->thread_id << " completed" << endl;
#endif
        pthread_exit(NULL);
    }

    void initialize() {
        curl_global_init(CURL_GLOBAL_ALL); //pretty obvious
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);        
    }

    void destroy() {
        curl_global_cleanup();
    }

    void * threadClearThreadResources(void * ptr) {
        thread_entry * te_to_be_deleted = (thread_entry *) ptr;
        delete te_to_be_deleted->tdata.containerPtr;
        delete te_to_be_deleted;
        pthread_exit(NULL);
    }

    void clearThreadResources(thread_entry * te) {
        // Spawns a new thread to clear resources
        pthread_t x;
        pthread_create(&x, NULL, threadClearThreadResources, (void *) te);
    }

    void addNewThreadToQueue(std::queue<thread_entry*> & q) {
        static int counter = 0;
        thread_entry * te = new thread_entry;

        te->tdata.thread_id = counter++;
        te->tdata.containerPtr = new vector<char>;
        pthread_create(&te->thread, NULL, camera::getImage, (void *) &(te->tdata));
        q.push(te);
    }

    thread_entry * getOldestThread(std::queue<thread_entry *> & q, void * status) {
        thread_entry * top = q.front();
        q.pop();
        pthread_join(top->thread, &status);
        return top;
    }

    void poolCamera(void (*fnc)(cv::Mat)) {
        std::queue<thread_entry*> q;
        addNewThreadToQueue(q);
        for(unsigned long long i = 0 ; waitKey(1) != 'q'; i++) {
            // get the current pool results
            void * status = 0;
            addNewThreadToQueue(q);
            thread_entry * top = getOldestThread(q, &status);
#ifdef DEBUG
            cerr << "Displaying information in thread id " << top->tdata.thread_id << endl;
            cerr << "Containe size from pooler " << top->tdata.containerSize << endl;
#endif
            // generate image matrix from char array
            cv::Mat image = imdecode(*(top->tdata.containerPtr), 1);
            fnc(image);

            // Free the memory contained by top
            clearThreadResources(top);
        }
    }
};
