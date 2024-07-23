#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdint.h>
#include <errno.h>
#include <math.h>

#include <getopt.h>
#include <string.h>

#define ADS1115_ADDRESS        0x48  // Default I2C address of ADS1115
#define ADS1115_CONFIG_REG     0x01
#define ADS1115_CONVERSION_REG 0x00

// These features are bundled in one byte
const int OS        = 0b1;
int MUX             = 0b000;
int PGA             = 0b001;
const int MODE      = 0b0;

// These features are bundled in one byte
int DR              = 0b100; int dr_sps;
const int COMP_MODE = 0b0;
const int COMP_POL  = 0b0;
const int COMP_LAT  = 0b0;
const int COMP_QUE  = 0b11;

const char READCODE[1] = {ADS1115_CONVERSION_REG};


void parse(int argc, char** argv)
{
    int c;
    while (1) {
        static struct option long_options[] =
        {
            {"mux", required_argument, 0, 'm'},
            {"pga", required_argument, 0, 'p'},
            {"dr", required_argument, 0, 'd'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "d:p:m:", long_options, &option_index);

        if (c == -1) break;
        switch (c)
        {
            case 0:
                if (long_options[option_index].flag != 0) break;
                printf("option %s", long_options[option_index].name);
                if (optarg) printf(" with arg %s", optarg);
                printf("\n");
                break;
            case 'd':
                if (strcmp(optarg, "8") == 0) {
                    DR = 0b000; dr_sps = 8;
                } else if (strcmp(optarg, "16") == 0) {
                    DR = 0b001; dr_sps = 16;
                } else if (strcmp(optarg, "32") == 0) {
                    DR = 0b010; dr_sps = 32;
                } else if (strcmp(optarg, "64") == 0) {
                    DR = 0b011; dr_sps = 64;
                } else if (strcmp(optarg, "128") == 0) {
                    DR = 0b100; dr_sps = 128;
                } else if (strcmp(optarg, "250") == 0) {
                    DR = 0b101; dr_sps = 250;
                } else if (strcmp(optarg, "475") == 0) {
                    DR = 0b110; dr_sps = 475;
                } else if (strcmp(optarg, "860") == 0) {
                    DR = 0b111; dr_sps = 860;
                } else {
                    fprintf(stderr, "Invalid DR option: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'p':
                if (strcmp(optarg, "6p144") == 0) {
                    PGA = 0b000;
                } else if (strcmp(optarg, "4p096") == 0) {
                    PGA = 0b001;
                } else if (strcmp(optarg, "2p048") == 0) {
                    PGA = 0b010;
                } else if (strcmp(optarg, "1p024") == 0) {
                    PGA = 0b011;
                } else if (strcmp(optarg, "0p512") == 0) {
                    PGA = 0b100;
                } else if (strcmp(optarg, "0p256") == 0) {
                    PGA = 0b101;
                } else {
                    fprintf(stderr, "Invalid PGA option: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'm':
                if (strcmp(optarg, "P0N1") == 0) {
                    MUX = 0b000;
                } else if (strcmp(optarg, "P0N3") == 0) {
                    MUX = 0b001;
                } else if (strcmp(optarg, "P1N3") == 0) {
                    MUX = 0b010;
                } else if (strcmp(optarg, "P2N3") == 0) {
                    MUX = 0b011;
                } else if (strcmp(optarg, "P0GND") == 0) {
                    MUX = 0b100;
                } else if (strcmp(optarg, "P1GND") == 0) {
                    MUX = 0b101;
                } else if (strcmp(optarg, "P2GND") == 0) {
                    MUX = 0b110;
                } else if (strcmp(optarg, "P3GND") == 0) {
                    MUX = 0b111;
                } else {
                    fprintf(stderr, "Invalid MUX option: %s\n", optarg);
                    exit(1);
                }
                break;
            case '?':
                /* getopt_long already printed an error message. */
                break;
            default:
                abort();
        }
    }
    if (optind < argc) {
        fprintf(stderr, "non-option ARGV-elements: ");
        while (optind < argc) fprintf(stderr, "%s ", argv[optind++]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "DR: %d\nPGA: %d\nMUX: %d\n", DR, PGA, MUX);
}

int setup(const char* path)
{
    int file;
    if ((file = open(path, O_RDWR)) < 0) {
        perror("Failed to open the bus");
        exit(1);
    }
    if (ioctl(file, I2C_SLAVE, ADS1115_ADDRESS) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
    return file;
}

void configurate(int file, char configcode[3])
{
    write(file, configcode, 3);  // 3 is just the length of CONFIGCODE
    sleep(1);
    write(file, READCODE, 1);    // 1 is just the length of READCODE
}

int convertADCOutputToInt16(char* data)
{
    int raw_adc = (data[0] << 8) | data[1];
    if (raw_adc > 32767) raw_adc -= 65535;
    return raw_adc;
}

int readADCOutputAsInt16From(int file)
{
    char data[2] = {0};
    if (read(file, data, 2) != 2) {
        perror("I/O error");
        exit(5);
    };
    return convertADCOutputToInt16(data);
}

int main(int argc, char** argv)
{
    int file = setup("/dev/i2c-1");

    parse(argc, argv);  // update MUX, PGA and DR

    // COMMAND1 needs to be sent as a byte so we shift right by 1 byte here
    int command1 = (OS << 15 | MUX << 12 | PGA << 9 | MODE << 8) >> 8;
    int command2 = DR << 5 | COMP_MODE << 4 | COMP_POL << 3 | COMP_LAT << 2 | COMP_QUE;
    char configcode[3] = {ADS1115_CONFIG_REG, command1, command2};
    configurate(file, configcode);

    int sleep_time_microsecond = (int)ceil(1.0/dr_sps*1000000);

    int value = 0;
    while (1) {
        value = readADCOutputAsInt16From(file);
        fprintf(stdout, "%d\n", value); fflush(stdout);

        usleep(sleep_time_microsecond);
    }

    return 0;
}
