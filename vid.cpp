#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

using namespace std;

int prefix_sum_local(int *arr, int range, int id)
{
    int new_range;
    if (range == 2){
        arr[1] = max(arr[1], arr[0]);
    } else if (range > 2) {
        new_range = pow(2, ceil(log2(range))-1);
        arr[range-1] = max(
            prefix_sum_local(arr          ,new_range       , id),
            prefix_sum_local(arr+new_range, range-new_range, id));
    }
    return arr[range-1];
}

//int prefix_sum_local(int *arr, int range)
//{
//    int arr_left;
//    int new_range;
//    if (range == 2){
//        arr_left = arr[0];
//        arr[0] = arr[1];
//        arr[1] = max(arr[1], arr_left);
//
//    } else if (range > 2) {
//        new_range = pow(2, ceil(log2(range))-1);
//        arr_left = arr[range-1];
//        arr[range-1] = max(
//            prefix_sum_local(arr,
//    }
//    return arr[range-1];
//}


int main(int argc, char *argv[])
{
    int id;
    int numprocs;
    int view_point; // first element of the input
    int range;      // numbers per process
    int inpsize;
    int *array;      // array of numbers
    int root;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    view_point = stoi(argv[1]);
    inpsize = argc - 2; // omitting firts element
    // number of elements assigned to each process
    range = 1;
    while (range*numprocs < inpsize){
        range = range*2;
    }

    if (id==0) cout << "2^ceil(log2(inpsize)-1): " << (pow(2, ceil(log(inpsize)/log(numprocs))-1))<< endl;
    array = new int [range]{0};

    for (int i = id*range; i < id*range+range && i < inpsize; i++){
        array[i%range] = atan2(stoi(argv[i+2]) - view_point, i);
        cout << id <<": " << array[i%range] << endl;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if ((id+1)*range > inpsize)
        range = inpsize - id*range;
    cout << id << ": range: " << range << endl;

    root = prefix_sum_local(array, range, id);
    for (int d = 0; d < log2(numprocs); d++){
        int pow1 = pow(2,d);
        int pow2 = pow(2,d+1);
        if (id % pow1 == id % pow2)
            cout << id << ": send: " << root << endl;
        else if (id % pow2 == pow2-1)
            cout << id << ": reciev: " << root << endl;
    }

    //prefix_sum(array[range-1], id, numprocs);

    MPI_Barrier(MPI_COMM_WORLD);
    for (int i = 0; i < range; i++)
        cout << id << ": " << array[i] << endl;

    MPI_Finalize();
    return 0;
}
