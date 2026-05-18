#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>
#include <iomanip>

int main() {
    // 1. Define our array size (10,000 simulates a small neural net layer)
    const int ARRAY_SIZE = 10000;
    std::vector<float> float_array(ARRAY_SIZE);
    std::vector<int8_t> int8_array(ARRAY_SIZE);

    // 2. Generate Dummy Data
    // We use a fixed seed (42) so the random numbers are the exact same every time you run it.
    // This is crucial for verifying your assembly code later.
    std::mt19937 gen(42); 
    std::uniform_real_distribution<float> dist(-5.5f, 12.8f); // Arbitrary asymmetric range

    for (int i = 0; i < ARRAY_SIZE; ++i) {
        float_array[i] = dist(gen);
    }

    // 3. Find the exact Min and Max of the tensor
    auto [min_it, max_it] = std::minmax_element(float_array.begin(), float_array.end());
    float min_val = *min_it;
    float max_val = *max_it;

    // 4. Calculate Scale and Zero-Point
    float scale = (max_val - min_val) / 255.0f;
    
    // Formula: Z = -round(min / scale) - 128
    int32_t zp_calc = static_cast<int32_t>(std::round(-min_val / scale) - 128);
    
    // Clamp to Int8 valid range [-128, 127] to prevent overflow
    zp_calc = std::max(-128, std::min(127, zp_calc));
    int8_t zero_point = static_cast<int8_t>(zp_calc);

    // 5. The Quantization Loop (This is what we will translate to Assembly)
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        // Quantize
        int32_t q_val = static_cast<int32_t>(std::round(float_array[i] / scale)) + zero_point;
        
        // Saturation logic (clamping values so they don't break the 8-bit limit)
        q_val = std::max(-128, std::min(127, q_val));
        
        // Store in our 8-bit array
        int8_array[i] = static_cast<int8_t>(q_val);
    }

    // 6. Print the Ground Truth Results
    std::cout << "--- Quantization Parameters ---\n";
    std::cout << "Min: " << min_val << ", Max: " << max_val << "\n";
    std::cout << "Scale: " << scale << "\n";
    // We cast to int for printing, otherwise cout tries to print an ASCII character
    std::cout << "Zero-Point: " << static_cast<int>(zero_point) << "\n\n";

    std::cout << "--- First 10 Elements (Ground Truth) ---\n";
    for (int i = 0; i < 10; ++i) {
        std::cout << "Float: " << std::fixed << std::setprecision(4) << float_array[i] 
                  << "  ->  Int8: " << static_cast<int>(int8_array[i]) << "\n";
    }

    return 0;
}