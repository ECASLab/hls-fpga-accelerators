/*
 * Copyright 2022-2024
 * Author: Luis G. Leon-Vega <luis.leon@ieee.org>
 */

 #include "softmax_lut.h"
 #include <iostream>

 #define BW kDataWidth
 
 int main(int, char **) {
   int rows = kRows;
   int cols = kCols;
 
   int size_a = rows * cols / kPackets;
   const int total_elements = rows * cols;
 
   RawDataT a[size_a];
   RawDataT c[size_a];
 
   

  std::cout << "Testing Softmax with " << kDataWidth << "-bit fixed point" << std::endl;
  std::cout << "Rows: " << rows << ", Cols: " << cols << std::endl;
  std::cout << "Packets: " << kPackets << ", Elements per packet: " << kPackets << std::endl;

// Fill A

  // Inicialización de los datos de entrada usando conversión explícita
  // DataT start_val = DataT(1.0f);  // Conversión explicita de -1.0 a DataT

  // DataT increment = DataT(0.000976563f); 
  
  
  // std::cout << "Incremento (float): " << start_val 
  //           << ", Incremento (DataT): " << increment << std::endl;


  //  std::cout << "A: " << std::endl;
  //  DataT current_val = (start_val);
  //  for (int elem = 0; elem < size_a; elem++) {
  //   RawDataT packet = 0;
  //    for (int p = 0; p < kPackets; ++p) {
  //      // Print the value
  //     std::cout << current_val << " ";

  //     // Pack the value into the raw packet
  //     ap_uint<BW> raw_bits;
  //     raw_bits.range(BW-1, 0) = current_val.range(BW-1, 0);
  //     packet.range((p+1)*BW-1, p*BW) = raw_bits;
  //      // Increment value
       
  //     current_val += increment;
  //    }
  //    // Store the packet in the array
  //    a[elem] = packet;
  //    std::cout << std::endl;
  //  }
   
   
DataT zero_val = DataT(-4.0f);
DataT two_val  = DataT(1.0f);


std::cout << "A: " << std::endl;
int total_elems = size_a * kPackets;
int pos_with_two = total_elems - 1;  // Última posición

int global_index = 0;
for (int elem = 0; elem < size_a; elem++) {
  RawDataT packet = 0;

  for (int p = 0; p < kPackets; ++p) {
    DataT current_val = (global_index == pos_with_two) ? two_val : zero_val;

    // Mostrar valor
    std::cout << current_val << " ";

    // Empaquetar
    ap_uint<BW> raw_bits;
    raw_bits.range(BW-1, 0) = current_val.range(BW-1, 0);
    packet.range((p+1)*BW-1, p*BW) = raw_bits;

    global_index++;
  }

  a[elem] = packet;
  std::cout << std::endl;
}
 
   softmax_lut(a, c, rows * cols);
   float sum = 0;
   std::cout << "C:" << std::endl;
   for (int elem = 0; elem < size_a; elem++) {
       RawDataT packet = c[elem];
       for (int p = 0; p < kPackets; ++p) {
           DataT tmp_val;
           tmp_val.range(BW - 1, 0) = packet.range((p + 1) * BW - 1, p * BW);
           std::cout << tmp_val << " ";
           sum += tmp_val.to_float();
       }
       std::cout << std::endl;
   }
    std::cout << "Suma total: " << sum << std::endl;
   return 0;
 }
