#pragma once

/**
 * @description: 风速生成
 * @param {int} min_speed
 * @param {int} max_speed
 * @param {double} noise_level
 * @param {double} t
 * @return {*}
 */
int app_generate_noisy_speed(int min_speed, int max_speed, double noise_level, double t);