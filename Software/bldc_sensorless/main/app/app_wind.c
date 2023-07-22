#include "app_wind.h"
#include "stdlib.h"
#include "math.h"

#define PI 3.14159265f

/**
 * @description: 风速生成
 * @param {int} min_speed
 * @param {int} max_speed
 * @param {double} noise_level
 * @param {double} t
 * @return {*}
 */
int app_generate_noisy_speed(int min_speed, int max_speed, double noise_level, double t)
{
    double random_noise = ((double)rand() / RAND_MAX) * 2 * noise_level - noise_level;
    double wave_speed = 200 * sin(t * 2 * PI / 10) + (max_speed + min_speed) / 2; // 增加振幅为 200
    double noisy_speed = wave_speed + random_noise;
    int speed = (int)round(fmax(min_speed, fmin(max_speed, noisy_speed))); // 限制在上下限范围内

    return speed;
}
