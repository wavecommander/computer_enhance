#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "types.h"

static f64 Square(f64 A);
static f64 RadiansFromDegrees(f64 Degrees);
static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius);

#include "../../perfaware/part2/listing_0065_haversine_formula.cpp"

#define X_CEIL 180.0
#define Y_CEIL 90.0
#define NUM_CLUSTERS 64

#define OUT_SZ_JSON sizeof(char) * (3+16+16) * num_pairs * 4 + 64
#define OUT_SZ_DIST sizeof(char) * (16+16) * num_pairs

#define generate_random_double(ceil) ((rand() + 1.0) / ((RAND_MAX + 1u)/(ceil)))
#define generate_random_centered_double(ceil) generate_random_double((ceil) ) - ((ceil)/2.0)

int main(int argc, char **argv) {
    u64 seed = 0;
    u64 num_pairs = 0;
    f64 sum_coeff = 0.0;

    { // parse input
        if (argc >= 3) {
            seed = atol(argv[1]);
            num_pairs = atol(argv[2]);
        } else {
            printf("Usage: %s [random seed] [number of coordinate pairs to generate]", argv[0]);
        }
    }
    srand(seed);
    sum_coeff = 1.0/(f64)num_pairs;

    char *json_out_str = 0;
    char *dist_out_str = 0;
    json_out_str = malloc(OUT_SZ_JSON);
    dist_out_str = malloc(OUT_SZ_DIST);
    long json_ct_out = 0;
    long dist_ct_out = 0;

    assert(json_out_str != 0 && dist_out_str != 0);

    f64 cluster_offsets_x[64];
    f64 cluster_offsets_y[64];

    for(u8 i = 0; i < 64; ++i) {
        cluster_offsets_x[i] = generate_random_double(X_CEIL * 2.0);
    }
    for(u8 i = 0; i < 64; ++i) {
        cluster_offsets_y[i] = generate_random_double(Y_CEIL * 2.0);
    }

    f64 final_sum = 0.0;

    for(u64 i = 0; i < num_pairs; ++i) {
        f64 x0 = generate_random_centered_double(X_CEIL * 2.0 - cluster_offsets_x[i % NUM_CLUSTERS]);
        f64 y0 = generate_random_centered_double(Y_CEIL * 2.0 - cluster_offsets_y[i % NUM_CLUSTERS]);

        f64 x1 = generate_random_centered_double(X_CEIL * 2.0 - cluster_offsets_x[i % NUM_CLUSTERS]);
        f64 y1 = generate_random_centered_double(Y_CEIL * 2.0 - cluster_offsets_y[i % NUM_CLUSTERS]);

        assert(x0 <= X_CEIL && x0 >= -X_CEIL && x1 <= X_CEIL && x1 >= -X_CEIL &&
                y0 <= Y_CEIL && y0 >= -Y_CEIL && y1 <= Y_CEIL && y1 >= -Y_CEIL);

        json_ct_out += snprintf(&json_out_str[json_ct_out], (OUT_SZ_JSON - json_ct_out), "{\"x0\":%.16f, \"y0\":%.16f,\"x1\":%.16f,\"y1\":%.16f},", x0, y0, x1, y1);

        f64 haversine_distance = ReferenceHaversine(x0, y0, x1, y1, 6372.8) * sum_coeff;
        dist_ct_out += snprintf(&dist_out_str[dist_ct_out], (OUT_SZ_DIST - dist_ct_out),  "%.16f\n", haversine_distance);
        final_sum += haversine_distance;
    }

    { // write out to files
        char filename[128];
        snprintf(filename, 128, "input-data/data_%llu_haveranswer.f64", num_pairs);
        FILE *dist_file = fopen(filename, "w+");
        assert(dist_file != 0);

        snprintf(filename, 128, "input-data/data_%llu_flex.json", num_pairs);
        FILE *json_file = fopen(filename, "w+");
        assert(json_file != 0);

        fprintf(dist_file, "%s", dist_out_str);

        json_out_str[json_ct_out - 1] = '\0'; // trim off trailing comma by terminating 1 char early
        fprintf(json_file, "{\"pairs\":[%s]}\n", json_out_str);

        { // cleanup
            free(json_out_str);
            free(dist_out_str);

            fclose(dist_file);
            fclose(json_file);
        }
    }

    printf("FINAL SUM: %.16f", final_sum);
}
