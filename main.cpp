#include "led-image-viewer.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Expected GIF filename.\n");
        return 1;
    }

    const char* gif_path = argv[1];
    Magick::InitializeMagick(argv[0]);

    rgb_matrix::RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;

    matrix_options.rows = 64;
    matrix_options.cols = 64;
    matrix_options.chain_length = 3;
    matrix_options.parallel = 3;
    matrix_options.hardware_mapping = "regular";
    runtime_opt.gpio_slowdown = 3;

    runtime_opt.drop_priv_user = getenv("SUDO_UID");
    runtime_opt.drop_priv_group = getenv("SUDO_GID");
    runtime_opt.do_gpio_init = true;

    rgb_matrix::RGBMatrix *matrix = rgb_matrix::RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL)
        return 1;

    rgb_matrix::FrameCanvas *offscreen_canvas = matrix->CreateFrameCanvas();

    ImageParams img_param;
    const bool do_center = true;
    const bool fill_width = false;
    const bool fill_height = false;

    FileInfo *file_info = NULL;
    std::string err_msg;
    std::vector<Magick::Image> image_sequence;

    if (LoadImageAndScale(gif_path, matrix->width(), matrix->height(),
                         fill_width, fill_height, &image_sequence, &err_msg)) {
        file_info = new FileInfo();
        file_info->params = img_param;
        file_info->content_stream = new rgb_matrix::MemStreamIO();
        file_info->is_multi_frame = image_sequence.size() > 1;
        
        rgb_matrix::StreamWriter out(file_info->content_stream);
        for (size_t i = 0; i < image_sequence.size(); ++i) {
            const Magick::Image &img = image_sequence[i];
            int64_t delay_time_us;
            if (file_info->is_multi_frame) {
                delay_time_us = img.animationDelay() * 10000;
            } else {
                delay_time_us = file_info->params.wait_ms * 1000;
            }
            if (delay_time_us <= 0) delay_time_us = 100 * 1000;
            StoreInStream(img, delay_time_us, do_center, offscreen_canvas, &out);
        }

        signal(SIGTERM, InterruptHandler);
        signal(SIGINT, InterruptHandler);

        while (!interrupt_received) {
            DisplayAnimation(file_info, matrix, offscreen_canvas);
        }
    } else {
        fprintf(stderr, "Failed to load image: %s\n", err_msg.c_str());
        return 1;
    }

    matrix->Clear();
    delete matrix;
    delete file_info->content_stream;
    delete file_info;

    return 0;
}