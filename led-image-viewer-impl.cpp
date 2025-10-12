#include "led-image-viewer.h"
#include <sys/time.h>
#include <unistd.h>
#include <string.h>

// Global variable definition
volatile bool interrupt_received = false;

static int64_t GetTimeInMillis() {
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

void StoreInStream(const Magick::Image &img, int delay_time_us,
                   bool do_center,
                   rgb_matrix::FrameCanvas *scratch,
                   rgb_matrix::StreamWriter *output) {
    // ...existing implementation from led-image-viewer.cc...
}

bool LoadImageAndScale(const char *filename,
                       int target_width, int target_height,
                       bool fill_width, bool fill_height,
                       std::vector<Magick::Image> *result,
                       std::string *err_msg) {
    // ...existing implementation from led-image-viewer.cc...
}

void DisplayAnimation(const FileInfo *file,
                      rgb_matrix::RGBMatrix *matrix, 
                      rgb_matrix::FrameCanvas *offscreen_canvas) {
    // ...existing implementation from led-image-viewer.cc...
}