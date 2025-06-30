#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
    int fd;
    unsigned char buffer[32];
    ssize_t bytes_read;
    
    // Open /dev/urandom
    fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Failed to open /dev/urandom: %s\n", strerror(errno));
        return 1;
    }
    
    printf("Successfully opened /dev/urandom\n");
    
    // Read random bytes
    bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read == -1) {
        fprintf(stderr, "Failed to read from /dev/urandom: %s\n", strerror(errno));
        close(fd);
        return 1;
    }
    
    printf("Read %zd bytes from /dev/urandom:\n", bytes_read);
    
    // Print the random bytes in hex format
    for (int i = 0; i < bytes_read; i++) {
        printf("%02x ", buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    if (bytes_read % 16 != 0) {
        printf("\n");
    }
    
    // Test multiple reads to ensure different values
    printf("\nTesting multiple reads for randomness:\n");
    for (int test = 0; test < 3; test++) {
        bytes_read = read(fd, buffer, 8);
        if (bytes_read > 0) {
            printf("Read %d: ", test + 1);
            for (int i = 0; i < bytes_read; i++) {
                printf("%02x", buffer[i]);
            }
            printf("\n");
        }
    }
    
    close(fd);
    printf("Test completed successfully\n");
    return 0;
}