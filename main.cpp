#include "LedImgViewer.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <climits>

static void InterruptHandler(int signo) {
    interrupt_received = true;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Expected 4 GIF filenames for quarter display.\n");
        fprintf(stderr, "Usage: %s <gif1> <gif2> <gif3> <gif4>\n", argv[0]);
        return 1;
    }

    const char* gif_paths[4] = {argv[1], argv[2], argv[3], argv[4]};
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

    // Arrays to hold 4 GIF files
    std::vector<Magick::Image> image_sequences[4];
    std::string err_msgs[4];
    bool all_loaded = true;

    // Calculate quarter dimensions (64x64 matrix with 3x3 chain = 192x192 total)
    int quarter_width = matrix->width() / 2;
    int quarter_height = matrix->height() / 2;

    // Create a combined file info for all 4 quarters
    FileInfo *combined_file_info = new FileInfo();
    combined_file_info->params = img_param;
    combined_file_info->content_stream = new rgb_matrix::MemStreamIO();
    combined_file_info->is_multi_frame = false; // Will be determined later
    
    rgb_matrix::StreamWriter combined_out(combined_file_info->content_stream);
    
    // Load all 4 GIF files first
    for (int gif_idx = 0; gif_idx < 4; gif_idx++) {
        if (!LoadImageAndScale(gif_paths[gif_idx], quarter_width, quarter_height,
                              fill_width, fill_height, &image_sequences[gif_idx], &err_msgs[gif_idx])) {
            fprintf(stderr, "Failed to load GIF %d (%s): %s\n", gif_idx + 1, gif_paths[gif_idx], err_msgs[gif_idx].c_str());
            all_loaded = false;
        }
    }
    
    if (all_loaded) {
        // Find the maximum number of frames across all GIFs
        size_t max_frames = 0;
        for (int gif_idx = 0; gif_idx < 4; gif_idx++) {
            if (image_sequences[gif_idx].size() > max_frames) {
                max_frames = image_sequences[gif_idx].size();
            }
            if (image_sequences[gif_idx].size() > 1) {
                combined_file_info->is_multi_frame = true;
            }
        }
        
        // Create combined frames by taking one frame from each GIF simultaneously
        for (size_t frame_idx = 0; frame_idx < max_frames; frame_idx++) {
            // Create a new canvas for this combined frame
            rgb_matrix::FrameCanvas *combined_canvas = matrix->CreateFrameCanvas();
            combined_canvas->Clear();
            
            int64_t min_delay_us = INT64_MAX;
            
            // Process each quarter
            for (int gif_idx = 0; gif_idx < 4; gif_idx++) {
                if (image_sequences[gif_idx].size() > 0) {
                    // Use modulo to loop the GIF if it has fewer frames
                    size_t actual_frame_idx = frame_idx % image_sequences[gif_idx].size();
                    const Magick::Image &img = image_sequences[gif_idx][actual_frame_idx];
                    
                    // Calculate delay for this frame
                    int64_t delay_time_us;
                    if (image_sequences[gif_idx].size() > 1) {
                        delay_time_us = img.animationDelay() * 10000;
                    } else {
                        delay_time_us = combined_file_info->params.wait_ms * 1000;
                    }
                    if (delay_time_us <= 0) delay_time_us = 100 * 1000;
                    
                    // Keep track of minimum delay
                    if (delay_time_us < min_delay_us) {
                        min_delay_us = delay_time_us;
                    }
                    
                    // Calculate quarter position
                    int start_x = (gif_idx % 2) * quarter_width;
                    int start_y = (gif_idx / 2) * quarter_height;
                    
                    // Calculate offset within the quarter
                    const int x_offset = do_center ? start_x + (quarter_width - img.columns()) / 2 : start_x;
                    const int y_offset = do_center ? start_y + (quarter_height - img.rows()) / 2 : start_y;
                    
                    // Draw this quarter's image to the combined canvas
                    for (size_t y = 0; y < img.rows(); ++y) {
                        for (size_t x = 0; x < img.columns(); ++x) {
                            const Magick::Color &c = img.pixelColor(x, y);
                            if (c.alphaQuantum() < 255) {
                                combined_canvas->SetPixel(x + x_offset, y + y_offset,
                                                          ScaleQuantumToChar(c.redQuantum()),
                                                          ScaleQuantumToChar(c.greenQuantum()),
                                                          ScaleQuantumToChar(c.blueQuantum()));
                            }
                        }
                    }
                }
            }
            
            // Store the combined frame
            combined_out.Stream(*combined_canvas, min_delay_us);
        }
    }

    if (all_loaded) {
        signal(SIGTERM, InterruptHandler);
        signal(SIGINT, InterruptHandler);

        while (!interrupt_received) {
            DisplayAnimation(combined_file_info, matrix, offscreen_canvas);
        }
    } else {
        fprintf(stderr, "Failed to load one or more GIF files.\n");
        return 1;
    }

    matrix->Clear();
    delete matrix;
    
    // Clean up combined file info
    delete combined_file_info->content_stream;
    delete combined_file_info;

    return 0;
}