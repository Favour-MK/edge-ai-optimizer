#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <chrono> // NEW: For high-precision timing

extern "C" void quantize_asm(const float* input, int8_t* output, int length, float scale, int32_t zero_point);

int main() {
    // 1. CRANK UP THE VOLUME: 50 Million Elements (approx 200MB of RAM)
    // Must be divisible by 4 for our SIMD loop to work perfectly!
    const int ARRAY_SIZE = 50000000; 
    
    std::cout << "Allocating memory for " << ARRAY_SIZE << " elements...\n";
    std::vector<float> float_array(ARRAY_SIZE);
    std::vector<int8_t> cpp_output_array(ARRAY_SIZE);
    std::vector<int8_t> asm_output_array(ARRAY_SIZE);

    // 2. Generate Dummy Data
    std::mt19937 gen(42); 
    std::uniform_real_distribution<float> dist(-5.5f, 12.8f); 

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        float_array[i] = dist(gen);
    }

    // 3. Calculate Scale and Zero-Point (Using hardcoded min/max to save setup time)
    float min_val = -5.49979f;
    float max_val = 12.7948f;
    float scale = (max_val - min_val) / 255.0f;
    
    int32_t zp_calc = static_cast<int32_t>(std::round(-min_val / scale) - 128);
    zp_calc = std::max(-128, std::min(127, zp_calc));
    int8_t zero_point = static_cast<int8_t>(zp_calc);

    std::cout << "Data ready. Starting Benchmark...\n\n";

    // ==========================================
    // BENCHMARK 1: Standard C++ Loop
    // ==========================================
    auto start_cpp = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        int32_t q_val = static_cast<int32_t>(std::round(float_array[i] / scale)) + zero_point;
        q_val = std::max(-128, std::min(127, q_val)); // Saturation
        cpp_output_array[i] = static_cast<int8_t>(q_val);
    }

    auto end_cpp = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> cpp_ms = end_cpp - start_cpp;


    // ==========================================
    // BENCHMARK 2: Your Custom Assembly
    // ==========================================
    auto start_asm = std::chrono::high_resolution_clock::now();

    quantize_asm(float_array.data(), asm_output_array.data(), ARRAY_SIZE, scale, zp_calc);

    auto end_asm = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> asm_ms = end_asm - start_asm;

    // ==========================================
    // THE RESULTS
    // ==========================================
    std::cout << "--- PERFORMANCE RESULTS ---\n";
    std::cout << "Standard C++ Execution:   " << cpp_ms.count() << " ms\n";
    std::cout << "SIMD Assembly Execution:  " << asm_ms.count() << " ms\n";
    
    double speedup = cpp_ms.count() / asm_ms.count();
    std::cout << "\nYour Assembly is " << speedup << "x faster than standard C++!\n";

    return 0;
}