#pragma once

#include "led-matrix.h"
#include "pixel-mapper.h"
#include "content-streamer.h"
#include <Magick++.h>
#include <string>
#include <vector>

extern volatile bool interrupt_received;

struct ImageParams {
    ImageParams() : anim_duration_ms(distant_future), wait_ms(1500),
                   anim_delay_ms(-1), loops(-1), vsync_multiple(1) {}
    int64_t anim_duration_ms;
    int64_t wait_ms;
    int64_t anim_delay_ms;
    int loops;
    int vsync_multiple;
    static const int64_t distant_future = (1LL<<40);
};

struct FileInfo {
    ImageParams params;
    bool is_multi_frame = false;
    rgb_matrix::StreamIO *content_stream = nullptr;
};

void DisplayAnimation(const FileInfo *file, rgb_matrix::RGBMatrix *matrix, 
                     rgb_matrix::FrameCanvas *offscreen_canvas);

bool LoadImageAndScale(const char *filename, int target_width, int target_height,
                      bool fill_width, bool fill_height,
                      std::vector<Magick::Image> *result,
                      std::string *err_msg);

void StoreInStream(const Magick::Image &img, int delay_time_us,
                  bool do_center,
                  rgb_matrix::FrameCanvas *scratch,
                  rgb_matrix::StreamWriter *output);