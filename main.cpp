#include "LedImgViewer.h"
#include "DisplayManager.h"
#include "ScreenConfig.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <climits>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

static void InterruptHandler(int signo) {
    printf("\nReceived signal %d, shutting down gracefully...\n", signo);
    interrupt_received = true;
}

int main(int argc, char *argv[]) {
    // Initialize ImageMagick
    Magick::InitializeMagick(argv[0]);

    // Load screen configuration
    ScreenConfig config;
    std::string config_file;
    bool config_loaded = false;
    
    // Parse command line arguments for --config
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            config_file = argv[i + 1];
            if (config.loadFromFile(config_file)) {
                config_loaded = true;
                printf("Configuration loaded from: %s\n", config_file.c_str());
            } else {
                fprintf(stderr, "Failed to load config file: %s\n", config_file.c_str());
                fprintf(stderr, "Using default configuration\n");
            }
            break;
        }
    }
    
    if (!config_loaded) {
        printf("No config file specified, using default configuration (192x192, ID=1)\n");
    }
    
    // Print configuration
    config.print();

    // Configure RGB matrix from config
    rgb_matrix::RGBMatrix::Options matrix_options;
    rgb_matrix::RuntimeOptions runtime_opt;

    matrix_options.rows = config.rows;
    matrix_options.cols = config.cols;
    matrix_options.chain_length = config.chain_length;
    matrix_options.parallel = config.parallel;
    matrix_options.hardware_mapping = config.hardware_mapping.c_str();
    
    // Set pixel mapper if specified (e.g., "V-mapper" for vertical screens)
    if (!config.pixel_mapper.empty()) {
        matrix_options.pixel_mapper_config = config.pixel_mapper.c_str();
    }
    
    runtime_opt.gpio_slowdown = config.gpio_slowdown;

    runtime_opt.drop_priv_user = getenv("SUDO_UID");
    runtime_opt.drop_priv_group = getenv("SUDO_GID");
    runtime_opt.do_gpio_init = true;

    rgb_matrix::RGBMatrix *matrix = rgb_matrix::RGBMatrix::CreateFromOptions(matrix_options, runtime_opt);
    if (matrix == NULL) {
        fprintf(stderr, "Failed to create RGB matrix\n");
        return 1;
    }

    // Create display manager
    // If using V-mapper, we need to swap dimensions because V-mapper rotates the display
    bool swap_dimensions = (config.pixel_mapper == "V-mapper");
    DisplayManager display_manager(matrix, swap_dimensions, config.screen_id);
    if (!display_manager.init(config.serial_port)) {
        fprintf(stderr, "Failed to initialize display manager\n");
        delete matrix;
        return 1;
    }
    
    // Check for --no-diagnostics flag or config setting
    bool show_diagnostics = config.show_diagnostics;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--no-diagnostics") == 0) {
            show_diagnostics = false;
            break;
        }
    }
    
    if (!show_diagnostics) {
        display_manager.clearScreen();
        printf("Diagnostic display disabled\n");
    }

    // Load initial GIFs if provided (backward compatibility)
    if (argc == 5) {
        const char* gif_paths[4] = {argv[1], argv[2], argv[3], argv[4]};
        int quarter_width = matrix->width() / 2;
        int quarter_height = matrix->height() / 2;
        
        // Load GIFs in quarters
        for (int i = 0; i < 4; i++) {
            int x = (i % 2) * quarter_width;
            int y = (i / 2) * quarter_height;
            display_manager.addGifElement(gif_paths[i], x, y, quarter_width, quarter_height, i);
        }
    }

    // Set up signal handlers with sigaction for better control
    struct sigaction sa;
    sa.sa_handler = InterruptHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        perror("sigaction SIGTERM");
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction SIGINT");
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("sigaction SIGHUP");
    }
    
    // Ignore SIGPIPE to prevent crashes on broken pipes
    signal(SIGPIPE, SIG_IGN);

    printf("\n=== LED Display Controller Started ===\n");
    printf("Screen ID: %d\n", config.screen_id);
    printf("Screen size: %dx%d\n", matrix->width(), matrix->height());
    printf("Protocol: Direct serial on %s at %d baud\n", config.serial_port.c_str(), config.serial_baudrate);
    printf("Commands: LOAD_GIF, DISPLAY_TEXT, CLEAR_SCREEN, SET_BRIGHTNESS, GET_STATUS\n");
    printf("Usage: %s [--config <config_file>] [--no-diagnostics] [gif1 gif2 gif3 gif4]\n", argv[0]);
    printf("Press Ctrl+C to exit\n");
    printf("======================================\n\n");

    // Main loop
    while (!interrupt_received) {
        // Process serial commands
        display_manager.processSerialCommands();
        
        // Update display
        display_manager.updateDisplay();
        
        // Small delay to prevent 100% CPU usage - 30Hz refresh rate
        usleep(33333); // ~33ms for 30Hz
    }

    printf("\nShutting down gracefully...\n");
    
    // Clear the matrix
    if (matrix) {
        matrix->Clear();
        delete matrix;
        matrix = nullptr;
    }
    
    // Cleanup ImageMagick
    // Magick::TerminateMagick(); // Not needed in modern ImageMagick
    
    printf("LED Display Controller stopped\n");
    return 0;
}