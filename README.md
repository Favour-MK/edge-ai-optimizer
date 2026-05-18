# Edge AI Optimizer: Int8 Quantization Wrapper

[![Language: C++](https://img.shields.io/badge/Language-C++-blue.svg)](https://isocpp.org/)
[![Language: Assembly](https://img.shields.io/badge/Language-x86--64_Assembly-red.svg)](https://nasm.us/)
[![Optimization: SIMD](https://img.shields.io/badge/Optimization-AVX%2FSSE-green.svg)]()

## 📌 Overview

This project is a highly optimized, bare-metal implementation of **Asymmetric Int8 Quantization**, designed to accelerate Neural Network inference on Edge devices.

By bypassing standard compiler heuristics and directly managing CPU registers via x86-64 Assembly, this wrapper converts 32-bit floating-point (FP32) tensors into 8-bit integers (Int8). This reduces the memory footprint of AI models by **75%** and dramatically accelerates memory bandwidth and execution time.

---

## 🚀 Performance Benchmarks

In a stress-test processing a tensor of **50 Million elements** (~200MB of RAM), the Assembly wrapper achieved a **~30x speedup** over a standard C++ implementation.

| Implementation | Execution Time (ms) | Speedup Multiplier |
| :--- | :--- | :--- |
| Standard C++ (`<cmath>`) | ~781.01 ms | 1.0x (Baseline) |
| **x86 SIMD Assembly** | **~25.63 ms** | **30.47x** ⚡ |

### The 3 Pillars of Optimization

1. **SIMD Vectorization:** Utilized `DIVPS` and `XMM` registers to process four 32-bit floats simultaneously in a single CPU clock cycle.
2. **Branchless Saturation (The Secret Weapon):** Replaced expensive high-level `if/else` clamping logic (`std::max/min`) with native hardware saturation using the `PACKSSWB` instruction. This completely eliminated branch prediction penalties in the CPU pipeline.
3. **Memory Bus Optimization:** Reduced memory bottlenecks by tightly packing four 8-bit integers into a single 32-bit register and writing them to main memory in a single `MOVD` operation, reducing memory bus traffic by **75%**.

---

## 🧮 Mathematical Foundation

The quantization process maps a continuous range of floating-point values to a discrete 8-bit integer space `[-128, 127]` using the following affine mapping formula:

$$X_{q} = \text{round}\left(\frac{X}{S}\right) + Z$$

- **S (Scale):** The step size between representable integers.
- **Z (Zero-Point):** Ensures that the floating-point zero is exactly representable as an integer, which is mathematically critical for operations like zero-padding in Convolutional Neural Networks (CNNs).

---

## 🛠️ Prerequisites

To build and run this project, your environment must support the **System V AMD64 ABI** calling conventions (Linux / macOS / WSL) and require the following tools:

- `g++` — GNU Compiler Collection, for the C++ test harness.
- `nasm` — Netwide Assembler, for compiling the assembly core.

---

## ⚙️ Build and Execution

**1. Clone the repository and navigate to the directory:**

```bash
git clone 
cd edge-ai-optimizer
```

**2. Assemble the x86 code:**

```bash
nasm -f elf64 quantizer.asm -o quantizer_asm.o
```

> **Note:** On macOS, use `-f macho64` instead of `-f elf64`.

**3. Compile the C++ bridge and link the object files:**

```bash
g++ -O3 quantizer_bridge.cpp quantizer_asm.o -o quantizer_benchmark
```

**4. Run the benchmark suite:**

```bash
./quantizer_benchmark
```

---

## 📁 Project Structure

```
edge-ai-optimizer/
├── quantizer.asm           # Core assembly logic — SIMD vectorization, memory alignment, hardware packing
└── quantizer_bridge.cpp    # C++ wrapper — synthetic tensor generation, memory allocation, chrono benchmarking
```

---
