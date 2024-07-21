#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

int main(int argc, char** argv)
{
    int mux = 0;
    int pga = 0;
    int mode = 0;
    int dr = 0;
    int c;
    while (1) {
        static struct option long_options[] = 
        {
            {"mux", required_argument, 0, 'm'},
            {"pga", required_argument, 0, 'p'},
            {"mode", required_argument, 0, 'o'},
            {"dr", required_argument, 0, 'd'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "d:p:m:o:", long_options, &option_index);

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
                    dr = 0b000;
                } else if (strcmp(optarg, "16") == 0) {
                    dr = 0b001;
                } else if (strcmp(optarg, "32") == 0) {
                    dr = 0b010;
                } else if (strcmp(optarg, "64") == 0) {
                    dr = 0b011;
                } else if (strcmp(optarg, "128") == 0) {
                    dr = 0b100;
                } else if (strcmp(optarg, "250") == 0) {
                    dr = 0b101;
                } else if (strcmp(optarg, "475") == 0) {
                    dr = 0b110;
                } else if (strcmp(optarg, "860") == 0) {
                    dr = 0b111;
                } else {
                    fprintf(stderr, "Invalid DR option: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'p':
                if (strcmp(optarg, "6p144") == 0) {
                    pga = 0b000;
                } else if (strcmp(optarg, "4p096") == 0) {
                    pga = 0b001;
                } else if (strcmp(optarg, "2p048") == 0) {
                    pga = 0b010;
                } else if (strcmp(optarg, "1p024") == 0) {
                    pga = 0b011;
                } else if (strcmp(optarg, "0p512") == 0) {
                    pga = 0b100;
                } else if (strcmp(optarg, "0p256") == 0) {
                    pga = 0b101;
                } else {
                    fprintf(stderr, "Invalid PGA option: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'm':
                if (strcmp(optarg, "P0N1") == 0) {
                    mux = 0b000;
                } else if (strcmp(optarg, "P0N3") == 0) {
                    mux = 0b001;
                } else if (strcmp(optarg, "P1N3") == 0) {
                    mux = 0b010;
                } else if (strcmp(optarg, "P2N3") == 0) {
                    mux = 0b011;
                } else if (strcmp(optarg, "P0GND") == 0) {
                    mux = 0b100;
                } else if (strcmp(optarg, "P1GND") == 0) {
                    mux = 0b101;
                } else if (strcmp(optarg, "P2GND") == 0) {
                    mux = 0b110;
                } else if (strcmp(optarg, "P3GND") == 0) {
                    mux = 0b111;
                } else {
                    fprintf(stderr, "Invalid MUX option: %s\n", optarg);
                    exit(1);
                }
                break;
            case 'o':
                if (strcmp(optarg, "continuous") == 0) {
                    mode = 0b0;
                } else if (strcmp(optarg, "singleshot") == 0) {
                    mode = 0b1;
                } else {
                    fprintf(stderr, "Invalid MODE option: %s\n", optarg);
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
    fprintf(stderr, "DR: %d\nPGA: %d\nMUX: %d\nMODE: %d\n", dr, pga, mux, mode);
    exit(0);
}
