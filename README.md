# TSP Brute Force và Genetic Algorithm

Đây là dự án sử dụng phương pháp brute force và thuật toán di truyền (Genetic Algorithm) để giải bài toán TSP (Travelling Salesman Problem) theo tuần tự và song song với OpenMP.

## Hướng dẫn chạy code trên terminal

Để chạy chương trình, thực hiện các bước sau:

1. **Biên dịch**

   Sử dụng lệnh sau để biên dịch mã nguồn:

   ```bash
   gcc -fopenmp -o tsp_bruteforce main.c tsp_bruteforce.c graph.c ga_tsp_serial.c ga_tsp_parallel.c

2. **Chạy chương trình**

   Sau khi biên dịch xong, bạn có thể chạy chương trình với số đỉnh tùy ý (ví dụ: 500):

    ```bash
   ./tsp_bruteforce 500
