// Direct ioctl test for RTS control
// Compile: gcc -o test_ioctl_rts test_ioctl_rts.c
// Run: sudo ./test_ioctl_rts /dev/ttyUSB0

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

void print_modem_bits(int status) {
    printf("  Modem status: 0x%04X\n", status);
    printf("    TIOCM_RTS  (0x004): %s\n", (status & TIOCM_RTS) ? "SET (HIGH)" : "clear (LOW)");
    printf("    TIOCM_DTR  (0x002): %s\n", (status & TIOCM_DTR) ? "SET (HIGH)" : "clear (LOW)");
    printf("    TIOCM_CTS  (0x020): %s\n", (status & TIOCM_CTS) ? "SET" : "clear");
    printf("    TIOCM_DSR  (0x100): %s\n", (status & TIOCM_DSR) ? "SET" : "clear");
    printf("    TIOCM_CD   (0x040): %s\n", (status & TIOCM_CD) ? "SET" : "clear");
}

int main(int argc, char *argv[]) {
    const char *device = (argc > 1) ? argv[1] : "/dev/ttyUSB0";
    int fd;
    struct termios tty;
    int status;
    
    printf("========================================\n");
    printf("Direct ioctl() RTS Test\n");
    printf("Device: %s\n", device);
    printf("========================================\n\n");
    
    // Open device
    printf("[1] Opening device...\n");
    fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("ERROR: Cannot open device");
        return 1;
    }
    printf("  Device opened successfully (fd=%d)\n\n", fd);
    
    // Get current termios settings
    if (tcgetattr(fd, &tty) != 0) {
        perror("ERROR: tcgetattr failed");
        close(fd);
        return 1;
    }
    
    // Configure port (NO CLOCAL!)
    printf("[2] Configuring port (NO CLOCAL)...\n");
    cfsetspeed(&tty, B1000000);
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;       // NO hardware flow control
    tty.c_cflag |= CREAD;
    tty.c_cflag &= ~CLOCAL;        // IMPORTANT: Don't ignore modem control lines!
    
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
    
    printf("  CLOCAL: %s (GOOD: should be DISABLED)\n", 
           (tty.c_cflag & CLOCAL) ? "ENABLED (BAD!)" : "DISABLED");
    printf("  CRTSCTS: %s\n\n", 
           (tty.c_cflag & CRTSCTS) ? "ENABLED" : "DISABLED");
    
    // Get initial modem status
    printf("[3] Reading initial modem status...\n");
    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("ERROR: TIOCMGET failed");
        close(fd);
        return 1;
    }
    print_modem_bits(status);
    printf("\n");
    
    // Test 1: Set RTS=LOW
    printf("[4] Setting RTS=LOW...\n");
    status &= ~TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("ERROR: TIOCMSET failed");
        close(fd);
        return 1;
    }
    usleep(100000);
    
    // Verify
    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("ERROR: TIOCMGET failed");
        close(fd);
        return 1;
    }
    print_modem_bits(status);
    
    if (status & TIOCM_RTS) {
        printf("  ❌ FAILED: RTS is still HIGH!\n");
    } else {
        printf("  ✅ SUCCESS: RTS is LOW\n");
    }
    printf("  → GPIO0 should be LOW now (~0V)\n\n");
    sleep(2);
    
    // Test 2: Set RTS=HIGH
    printf("[5] Setting RTS=HIGH...\n");
    status |= TIOCM_RTS;
    if (ioctl(fd, TIOCMSET, &status) == -1) {
        perror("ERROR: TIOCMSET failed");
        close(fd);
        return 1;
    }
    usleep(100000);
    
    // Verify
    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("ERROR: TIOCMGET failed");
        close(fd);
        return 1;
    }
    print_modem_bits(status);
    
    if (status & TIOCM_RTS) {
        printf("  ✅ SUCCESS: RTS is HIGH\n");
    } else {
        printf("  ❌ FAILED: RTS is still LOW!\n");
    }
    printf("  → GPIO0 should be HIGH now (~3.3V)\n\n");
    
    // Test 3: Toggle test
    printf("[6] Toggling RTS 5 times...\n");
    for (int i = 0; i < 5; i++) {
        status &= ~TIOCM_RTS;
        ioctl(fd, TIOCMSET, &status);
        printf("  %d. RTS=LOW", i+1);
        usleep(500000);
        
        status |= TIOCM_RTS;
        ioctl(fd, TIOCMSET, &status);
        printf(" → RTS=HIGH\n");
        usleep(500000);
    }
    printf("\n");
    
    // Final state with measurement time
    printf("[7] Setting final state: DTR=HIGH, RTS=HIGH\n");
    status |= TIOCM_DTR | TIOCM_RTS;
    ioctl(fd, TIOCMSET, &status);
    usleep(100000);
    
    if (ioctl(fd, TIOCMGET, &status) == -1) {
        perror("ERROR: TIOCMGET failed");
        close(fd);
        return 1;
    }
    print_modem_bits(status);
    
    printf("\n========================================\n");
    printf("MEASUREMENT TIME!\n");
    printf("========================================\n");
    printf("Port is being held open for 10 seconds.\n");
    printf("→ Measure GPIO0 with multimeter NOW!\n");
    printf("   Expected: ~3.3V (if DIRECT logic)\n");
    printf("   Or: ~0V (if INVERTED logic)\n\n");
    
    for (int i = 10; i > 0; i--) {
        printf("  %2d seconds remaining...\r", i);
        fflush(stdout);
        sleep(1);
    }
    
    printf("\n\n");
    printf("[8] Closing port...\n");
    close(fd);
    printf("  Port closed (DTR/RTS will reset to LOW)\n\n");
    
    printf("========================================\n");
    printf("Results:\n");
    printf("========================================\n");
    printf("If ioctl() showed SUCCESS but GPIO0 didn't change:\n");
    printf("  → Check physical connection RTS pin to GPIO0\n");
    printf("  → Check if USB-Serial converter has RTS output\n");
    printf("  → Check for pull-down resistors on GPIO0\n");
    printf("\nIf ioctl() FAILED:\n");
    printf("  → Driver or hardware issue\n");
    printf("  → Try different USB port or converter\n");
    printf("========================================\n");
    
    return 0;
}

