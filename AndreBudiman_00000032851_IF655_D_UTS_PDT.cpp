// AndreBudiman_00000032851_IF655_D_UTS_PDT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <mpi.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <iostream>
#include <string>

// ALGO caesar chiper // 
char caesarChiper(char msg, int k) {

    if (islower(msg)) {
        return char(int(msg + k - 65) % 26 + 65);
    }
    else {
        return char(int(msg + k - 97) % 26 + 97);
    }
}
// userText[1000] = untuk menerima inputan dari user berupa huruf dengan maks 1000 //
// slavePlaceHolder[1000] = untuk menyimpan karakter yang akan dikirim untuk di encode di setiap slave processor //
// masterPlaceHolder[1000] = untuk menyimpan karakter hasil enkripsi dari setiap slave ke master dan di kirim ke master //
// masterChiperText[1000] = untuk menampung semua hasil enkripsi yang dilakukan oleh master //
// slaveChiperText[1000] = untuk menampung semua hasil enkripsi yang dilakukan oleh slave //
char userText[1000], slavePlaceHolder[1000], masterPlaceHolder[1000], masterChiperText[1000],slaveChiperText[1000];
int n, shift;
// n = jumlah data, shift = jumlah pergeseran karakter //


//Program Utama
int main(int argc, char* argv[])
{
    // RANK = Process ID || SIZE = Banyak Process
    int rank, size, elementsPerProcess, n_elements_recieve, receiveSlave;
    double elapsed_time;

    MPI_Status stat;

    // inisialisasi MPI (step awal)
    MPI_Init(&argc, &argv);

    // mencari proses ID dan berapa banyak proses yang dimulai
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    elapsed_time = -MPI_Wtime();

    // process utama
    if (rank == 0) {
        do {
            std::cout << "Enter Words: "; // input userText //
            std::cin >> userText;
            fflush(stdout);
            std::cout << "Enter the number of shifts : "; // input shift //
            std::cin >> shift;
            fflush(stdout);

            n = strlen(userText); // mengukur panjang userText //
            elementsPerProcess = n / size; // memecah task yang harus dikerjakan per processor //

            // error handling untuk jumlah karakter kurang dari jumlah processor //
            if (n < size) {
                printf("The number of characters is less than the number of processors, please re-enter the word\n\n\n");
            }
        } while (n < size);
    }

    // broadcast ke semua processor ketika terjadi pergeseran karakter//
    MPI_Bcast(&shift, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    // broadcast panjang n (userText) ke semua processor //
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //CORE PROCESS / MASTER PROCESS
    if (rank == 0) {
        int idx, x;
        if (size > 1) {
            // membagi task ke slave
            for (x = 1; x < size - 1; x++) {
                idx = x * elementsPerProcess;
                // komunikasi antara master ke slave
                MPI_Send(&elementsPerProcess, 1, MPI_INT, x, 0, MPI_COMM_WORLD);
                MPI_Send((void*)&userText[idx], elementsPerProcess, MPI_CHAR, x, 0, MPI_COMM_WORLD);
            }

            // task yang harus dikerjakan oleh slave processor //
            idx = x * elementsPerProcess;
            int elements_left = n - idx;

            // komunikasi antara master ke slave
            MPI_Send(&elements_left, 1, MPI_INT,x, 0, MPI_COMM_WORLD);
            MPI_Send((void*)&userText[idx], elements_left, MPI_CHAR, x, 0, MPI_COMM_WORLD);
        }

        // enkripsi master process
        for (x = 0; x < elementsPerProcess; x++) {
            printf("processor %d have the data of %c\n", rank, userText[x]);
            masterChiperText[x] = caesarChiper(userText[x], shift);
        }
        printf("master chiper : %s", masterChiperText);
    }

    // slave process 
    else {
        // mendapat kiriman dari master processor
        MPI_Recv(&n_elements_recieve, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &stat);
        MPI_Recv(&slavePlaceHolder, n_elements_recieve, MPI_CHAR, 0, 0, MPI_COMM_WORLD, &stat);
       
        // enkripsi yang dilakukan oleh slave untuk setiap task yang di dapat"
        for (int x = 0; x < n_elements_recieve; x++) {
            printf("processor %d have the data of %c\n", rank, slavePlaceHolder[x]);
            slaveChiperText[x] = caesarChiper(slavePlaceHolder[x], shift);
        }

        // mengirim hasil enkripsi dari slave ke master //
        MPI_Send((void*)&slaveChiperText, n_elements_recieve, MPI_CHAR, 0, 99, MPI_COMM_WORLD);
        MPI_Send(&n_elements_recieve, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        printf("slave chiper : %s\n", slaveChiperText);
    }

    // print waktu yang dibutuhkan sistem //
    if (rank == 0) {
        elapsed_time += MPI_Wtime();
        printf("\n\nTotal elapsed time: %10.6f\n", elapsed_time);

        // menerima semua enkripsi dari setiap slave processor //
        for (int x = 1; x < size; x++) {
            MPI_Recv(&receiveSlave, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
            MPI_Recv(&masterPlaceHolder, receiveSlave, MPI_CHAR, MPI_ANY_SOURCE, 99, MPI_COMM_WORLD, &stat);

            // memisahkan hasil enkripsi yang dihasilkan oleh slave process //
            strcat_s(masterChiperText, sizeof(masterPlaceHolder), masterPlaceHolder);

        }
        // menggabungkan semua hasil enkripsi (slave + master processor) //
        printf("Encode results : %s \n", masterChiperText);
    }
    // membersihkan semua status MPI sebelum keluar dari process
    MPI_Finalize();
    return 0;
}

//referensi: Parallel Programming in C with MPI and OpenMP.pdf