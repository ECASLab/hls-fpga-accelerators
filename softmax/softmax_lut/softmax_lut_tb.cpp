/*
 * Copyright 2025
 * Author: Anthony Leiva V <anleva1720@gmail.com>
 * 
 */

#include "softmax_lut.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <iomanip>

#define BW kDataWidth
#define NUM_TEST_VECTORS 20  // Requisito: 1000 vectores
#define MIN_VAL -4.0f
#define MAX_VAL  4.0f          

// --------------------------------------------------------------------------
// Golden Reference
// --------------------------------------------------------------------------
void softmax_golden(const std::vector<float>& input, std::vector<float>& output) {
  float sum = 0.0f;


  
  for (size_t i = 0; i < input.size(); ++i) {
      output[i] = std::exp(input[i]);
      sum += output[i];
  }


  for (size_t i = 0; i < input.size(); ++i) {
      output[i] /= sum;
  }
  
}

// --------------------------------------------------------------------------
// Main Testbench
// --------------------------------------------------------------------------
int main(int, char **) {
    int rows = kRows;
    int cols = kCols;
    const int vector_length = rows * cols;
    
    
    int size_a = vector_length / kPackets; 
    
    RawDataT* a = new RawDataT[size_a]; 
    RawDataT* c = new RawDataT[size_a];

    // Random generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(MIN_VAL, MAX_VAL);

    double total_squared_error = 0.0;
    int total_elements_processed = 0;

    std::cout << "==================================================" << std::endl;
    std::cout << " Iniciando Test de Error Numerico (RMSE)" << std::endl;
    std::cout << " Config: " << BW << "-bit fixed point" << std::endl;
    std::cout << " Vector Length: " << vector_length << " | Vectores de prueba: " << NUM_TEST_VECTORS << std::endl;
    std::cout << "==================================================" << std::endl;

    
    for (int iter = 0; iter < NUM_TEST_VECTORS; ++iter) {
        
        std::vector<float> input_float(vector_length);
        std::vector<float> output_golden(vector_length);
        std::vector<float> output_hls(vector_length);

        // 1. Generar vector aleatorio y empaquetar para HLS
        int global_idx = 0;
        for (int elem = 0; elem < size_a; elem++) {
            RawDataT packet = 0;
            for (int p = 0; p < kPackets; ++p) {
                // Generar valor float aleatorio
                float val_f = dist(gen);
                input_float[global_idx] = val_f;

                // Convertir a Punto Fijo (DataT)
                DataT val_fixed = DataT(val_f);

                // Empaquetar bits
                ap_uint<BW> raw_bits;
                raw_bits.range(BW-1, 0) = val_fixed.range(BW-1, 0);
                packet.range((p+1)*BW-1, p*BW) = raw_bits;

                global_idx++;
            }
            a[elem] = packet;
        }

        // 2. Ejecutar DUT (HLS Softmax)
        softmax_lut(a, c, vector_length);

        // 3. Ejecutar Golden Reference (Software Softmax)
        softmax_golden(input_float, output_golden);

        // 4. Desempaquetar resultados HLS y Calcular Error
        global_idx = 0;
        float verification_sum = 0;
        float verification_sum_exact = 0;
        for (int elem = 0; elem < size_a; elem++) {
            RawDataT packet = c[elem];
            for (int p = 0; p < kPackets; ++p) {
                // Extraer y convertir a float para comparar
                DataT tmp_val;
                tmp_val.range(BW - 1, 0) = packet.range((p + 1) * BW - 1, p * BW);
                
                output_hls[global_idx] = tmp_val.to_float();

                // Calcular error cuadrático puntual
                float diff = output_hls[global_idx] - output_golden[global_idx];
                /*std::cout << "Vector " << (iter + 1) << ", Index " << global_idx 
                          << ": HLS = " << std::scientific << std::setprecision(6) << output_hls[global_idx]
                          << ", Golden = " << std::scientific << std::setprecision(6) << output_golden[global_idx]
                          << ", Diff = " << std::scientific << std::setprecision(6) << diff << std::endl;*/
                total_squared_error += (diff * diff);
                verification_sum += output_hls[global_idx];
                verification_sum_exact += output_golden[global_idx];


                global_idx++;
            }
        }
        // Verificación de que la suma es aproximadamente 1.0
        std::cout << "Vector " << (iter + 1) << ": Suma verificación Exacta = " << verification_sum_exact << std::endl;
        
        std::cout << "Vector " << (iter + 1) << ": Suma verificación = " << verification_sum << std::endl;
        total_elements_processed += vector_length;

        // Opcional: Imprimir progreso cada 100 iteraciones
        if ((iter + 1) % 100 == 0) {
            std::cout << "Procesados " << (iter + 1) << " vectores..." << std::endl;
        }
    }

    // 5. Calcular RMSE Final
    double mse = total_squared_error / total_elements_processed;
    double rmse = std::sqrt(mse);

    std::cout << "==================================================" << std::endl;
    std::cout << " RESULTADOS FINALES " << std::endl;
    std::cout << " Precisión (Bits): " << BW << std::endl;
    std::cout << " Longitud Vector:  " << vector_length << std::endl;
    std::cout << " RMSE Global:      " << std::scientific << std::setprecision(6) << rmse << std::endl;
    std::cout << "==================================================" << std::endl;

    // Limpieza de memoria
    delete[] a;
    delete[] c;

    return 0;
}