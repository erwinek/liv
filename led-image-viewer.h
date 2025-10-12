#pragma once

#include "led-matrix.h"
#include "content-streamer.h"
#include <Magick++.h>
#include <string>
#include <vector>

extern volatile bool interrupt_received;

typedef int64_t tmillis_t;

struct ImageParams {
    ImageParams() : anim_duration_ms(distant_future), wait_ms(1500),
                   anim_delay_ms(-1), loops(-1), vsync_multiple(1) {}
    tmillis_t anim_duration_ms;
    tmillis_t wait_ms;
    tmillis_t anim_delay_ms;
    int loops;
    int vsync_multiple;
    static const tmillis_t distant_future = (1LL<<40);
};

struct FileInfo {
    ImageParams params;
    bool is_multi_frame = false;
    rgb_matrix::StreamIO *content_stream = nullptr;
};

void DisplayAnimation(const FileInfo *file, 
                     rgb_matrix::RGBMatrix *matrix,
                     rgb_matrix::FrameCanvas *offscreen_canvas);

bool LoadImageAndScale(const char *filename,
                      int target_width, int target_height,
                      bool fill_width, bool fill_height,
                      std::vector<Magick::Image> *result,
                      std::string *err_msg);

void StoreInStream(const Magick::Image &img, int delay_time_us,
                  bool do_center,
                  rgb_matrix::FrameCanvas *scratch,
                  rgb_matrix::StreamWriter *output);