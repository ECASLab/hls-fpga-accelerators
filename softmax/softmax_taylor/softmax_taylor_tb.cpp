// En tu archivo de testbench main.cpp

#include "softmax_taylor.h"
#include <iostream>
#include <vector>

int main(int, char **) {
    const int rows = kRows;
    const int cols = kCols;
    const int total_elements = rows * cols;
    const int size_a = total_elements / kPackets;

    std::vector<RawDataT> a(size_a);
    std::vector<RawDataT> c(size_a);
    
    // --- Imprimir Información ---
    std::cout << "=================================================" << std::endl;
    if (IS_FP) {
        std::cout << "Testing Softmax with CustomFloat<" << WS << "," << MS << ">" << std::endl;
    } else {
        std::cout << "Testing Softmax with ap_fixed<" << kDataWidth << "," << kFxPDataInt << ">" << std::endl;
    }
    std::cout << "Rows: " << rows << ", Cols: " << cols << std::endl;
    std::cout << "=================================================" << std::endl;

    // --- 1. Inicialización y Empaquetado de Datos ---
    DataT current_val = 1.f;
    DataT increment = 0.00f;
    
    std::cout << "Input Data (A):" << std::endl;
    for (int i = 0; i < size_a; ++i) {
        RawDataT packet = 0;
        for (int p = 0; p < kPackets; ++p) {
            std::cout << current_val << " " << increment << " " << std::endl;
            
            
            ap_uint<kDataWidth> raw_bits = GET_RAW(current_val);
            packet.range((p + 1) * kDataWidth - 1, p * kDataWidth) = raw_bits;
            
            current_val = current_val + increment;
        }
        a[i] = packet;
    }
    
    // --- 2. Ejecutar el Kernel ---
    std::cout << "\nExecuting softmax_taylor kernel..." << std::endl;
    softmax_taylor(a.data(), c.data(), total_elements);

    // --- 3. Desempaquetado y Verificación ---
    std::cout << "\nOutput Data (C):" << std::endl;
    double total_sum = 0.0;
    for (int i = 0; i < size_a; ++i) {
        RawDataT packet = c[i];
        for (int p = 0; p < kPackets; ++p) {
            ap_uint<kDataWidth> raw_bits = packet.range((p + 1) * kDataWidth - 1, p * kDataWidth);
            
            
            DataT result_val = GET_NUMBER<DataT>(raw_bits);
            
            std::cout << result_val << " ";

            total_sum += toFloat(result_val);
        }
        std::cout << std::endl;
    }

    std::cout << "\nVerification Sum: " << total_sum << " (should be close to 1.0)" << std::endl;
    
    return 0;
    
}