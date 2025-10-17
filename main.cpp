#include "LedImgViewer.h"
#include "DisplayManager.h"
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

    // Configure RGB matrix
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
    if (matrix == NULL) {
        fprintf(stderr, "Failed to create RGB matrix\n");
        return 1;
    }

    // Create display manager
    DisplayManager display_manager(matrix);
    if (!display_manager.init()) {
        fprintf(stderr, "Failed to initialize display manager\n");
        delete matrix;
        return 1;
    }
    
    // Check for --no-diagnostics flag
    bool show_diagnostics = true;
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

    printf("LED Display Controller started\n");
    printf("Screen size: %dx%d\n", matrix->width(), matrix->height());
    printf("Serial protocol: 1000000 bps on /dev/ttyUSB0\n");
    printf("Commands: LOAD_GIF, DISPLAY_TEXT, CLEAR_SCREEN, SET_BRIGHTNESS, GET_STATUS\n");
    printf("Usage: %s [gif1 gif2 gif3 gif4] [--no-diagnostics]\n", argv[0]);
    printf("Press Ctrl+C to exit\n");

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