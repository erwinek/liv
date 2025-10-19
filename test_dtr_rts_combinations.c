// Test all DTR/RTS combinations to find correct sequence
// Compile: gcc -o test_combinations test_dtr_rts_combinations.c
// Run: sudo ./test_combinations /dev/ttyUSB0

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

int main(int argc, char *argv[]) {
    const char *device = (argc > 1) ? argv[1] : "/dev/ttyUSB0";
    int fd;
    struct termios tty;
    int status;
    
    printf("========================================\n");
    printf("DTR/RTS Combination Test\n");
    printf("Device: %s\n", device);
    printf("========================================\n\n");
    
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("ERROR: Cannot open device");
        return 1;
    }
    
    if (tcgetattr(fd, &tty) != 0) {
        perror("ERROR: tcgetattr failed");
        close(fd);
        return 1;
    }
    
    // Configure port
    cfsetspeed(&tty, B1000000);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD;
    tty.c_cflag &= ~CLOCAL;  // IMPORTANT!
    
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_oflag &= ~OPOST;
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_cc[VTIME] = 0;
    tty.c_cc[VMIN] = 0;
    
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("ERROR: tcsetattr failed");
        close(fd);
        return 1;
    }
    
    printf("Testing all 4 combinations...\n");
    printf("Measure EN and GPIO0 for each case!\n\n");
    
    // Test 1: DTR=LOW, RTS=LOW
    printf("[1] DTR=LOW, RTS=LOW\n");
    status = 0;  // Both clear
    ioctl(fd, TIOCMSET, &status);
    printf("    Typical result: EN=HIGH, GPIO0=HIGH (Normal boot)\n");
    printf("    → Measure EN and GPIO0 now!\n");
    sleep(3);
    
    // Test 2: DTR=LOW, RTS=HIGH
    printf("\n[2] DTR=LOW, RTS=HIGH\n");
    status = TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    printf("    Typical result: EN=HIGH, GPIO0=LOW (Bootloader mode)\n");
    printf("    → Measure EN and GPIO0 now!\n");
    sleep(3);
    
    // Test 3: DTR=HIGH, RTS=LOW
    printf("\n[3] DTR=HIGH, RTS=LOW\n");
    status = TIOCM_DTR;
    ioctl(fd, TIOCMSET, &status);
    printf("    Typical result: EN=LOW, GPIO0=HIGH (Reset)\n");
    printf("    → Measure EN and GPIO0 now!\n");
    sleep(3);
    
    // Test 4: DTR=HIGH, RTS=HIGH
    printf("\n[4] DTR=HIGH, RTS=HIGH\n");
    status = TIOCM_DTR | TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    printf("    Typical result: EN=LOW, GPIO0=LOW (Reset in bootloader)\n");
    printf("    → Measure EN and GPIO0 now!\n");
    sleep(3);
    
    printf("\n========================================\n");
    printf("RESET SEQUENCE TEST\n");
    printf("========================================\n");
    printf("Testing typical ESP32 reset sequence:\n\n");
    
    // Sequence A: Standard NodeMCU/ESP32-DevKit
    printf("[Sequence A] Standard ESP32 auto-reset:\n");
    printf("  1. DTR=HIGH, RTS=LOW (Reset, GPIO0=HIGH)\n");
    status = TIOCM_DTR;
    ioctl(fd, TIOCMSET, &status);
    sleep(1);
    
    printf("  2. DTR=LOW, RTS=LOW (Release reset, GPIO0=HIGH)\n");
    status = 0;
    ioctl(fd, TIOCMSET, &status);
    printf("     → ESP32 should boot into NORMAL MODE\n");
    printf("     → Check if ESP32 boots now!\n");
    sleep(5);
    
    printf("\n[Sequence B] Alternative sequence:\n");
    printf("  1. DTR=HIGH, RTS=HIGH (Reset in bootloader)\n");
    status = TIOCM_DTR | TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    sleep(1);
    
    printf("  2. DTR=LOW, RTS=HIGH (Release reset, GPIO0=LOW)\n");
    status = TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    printf("     → ESP32 should boot into BOOTLOADER MODE\n");
    printf("     → Check if ESP32 shows 'waiting for download'\n");
    sleep(5);
    
    printf("\n========================================\n");
    printf("FINAL STATE: Normal mode\n");
    printf("========================================\n");
    printf("Setting: DTR=LOW, RTS=LOW (both OFF)\n");
    status = 0;
    ioctl(fd, TIOCMSET, &status);
    printf("EN should be HIGH, GPIO0 should be HIGH\n");
    printf("Keeping port open for 15 seconds...\n\n");
    
    for (int i = 15; i > 0; i--) {
        printf("  %2d seconds - MEASURE GPIO0 NOW! (should be HIGH)\r", i);
        fflush(stdout);
        sleep(1);
    }
    
    printf("\n\nClosing port...\n");
    close(fd);
    
    printf("\n========================================\n");
    printf("RESULTS TO REPORT:\n");
    printf("========================================\n");
    printf("1. Which combination gave GPIO0=HIGH? ______\n");
    printf("2. Which combination gave EN=HIGH? ______\n");
    printf("3. Did ESP32 boot in normal mode with Sequence A? (YES/NO) ______\n");
    printf("4. What is GPIO0 voltage with DTR=LOW, RTS=LOW? ______V\n");
    printf("========================================\n");
    
    return 0;
}

