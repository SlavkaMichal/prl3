#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

using namespace std;

/* sequential implementation of scan
   arr - array to be
   */
inline float scan_seq(float *arr, int range, int id)
{
    int new_range;
    if (range == 2){
        arr[1] = max(arr[1], arr[0]);
    } else if (range > 2) {
        new_range = pow(2, ceil(log2(range))-1);
        arr[range-1] = max(
            scan_seq(arr          ,new_range       , id),
            scan_seq(arr+new_range, range-new_range, id));
    }
    return arr[range-1];
}

inline float up_sweep(float *arr, int range, int id, int numprocs)
{
    int pow1;
    int pow2;
    int dst;
    float n_num;
    float root;
    MPI_Status stat;

    root = scan_seq(arr, range, id);
    //cout << id << ": " << root <<endl;
    for (int d = 0; d < log2(numprocs); d++){
        pow1 = pow(2,d);
        pow2 = pow(2,d+1);
        if (id % pow2 == pow1 - 1 && id != numprocs-1){
            dst = min(id+pow1, numprocs-1);
            //cout << id << ":"<<d<<" send to: " << dst << endl;
            MPI_Send(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD);
            //break;
        } else if (id % pow2 == pow2-1){
            //cout << id << ":oo"<<d<<" recieve from: " << id - pow1 << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, id - pow1, 0, MPI_COMM_WORLD, &stat);
            root = max(n_num, root);
        } else if (id == numprocs-1 && numprocs % pow2 > pow1 ){MPI_Recv(&n_num, 1, MPI_FLOAT, id - numprocs % pow1, 0, MPI_COMM_WORLD, &stat);
            //cout << id << ":++"<<d<<" recieve from: " << id - numprocs % pow1 << endl;
            //MPI_Recv(&n_num, 1, MPI_FLOAT, id - numprocs % pow1, 0, MPI_COMM_WORLD, &stat);
            root = max(n_num, root);
        }
    }

    arr[range-1] = root;
    return root;
}

inline float down_sweep_seq(float *arr, int range, int id)
{
    float arr0;
    int new_range;
    if (range == 2){
        arr0 = arr[0];
        arr[0] = arr[1];
        arr[1] = max(arr[1], arr0);
    } else if (range > 2) {
        new_range = pow(2, ceil(log2(range))-1);

        arr0 = arr[new_range-1];
        arr[new_range-1] = arr[range-1];
        arr[range-1] = max(arr0, arr[range-1]);

        down_sweep_seq(arr          ,new_range       , id);
        down_sweep_seq(arr+new_range, range-new_range, id);
    }
    return arr[range-1];
}

int main(int argc, char *argv[])
{
    int id;
    int numprocs;
    int view_point; // first element of the input
    int range;      // numbers per process
    int inpsize;
    float *array;      // array of numbers


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    view_point = stoi(argv[1]);
    inpsize = argc - 2; // omitting first element

    // number of elements assigned to each process
    range = 1;
    while (range*numprocs < inpsize){
        range = range*2;
    }

    // assign numbers to processes
    array = new float [range]{0};
    float *angle = new float [range]{0};
    for (int i = id*range; i < id*range+range && i < inpsize; i++){
        array[i%range] = atan2(stoi(argv[i+2]) - view_point, i+1);
        angle[i%range] = array[i%range];
        //cout << id <<": (" <<stoi(argv[i+2])<< "-" << view_point << ")/" << i << "="<<array[i%range]<< endl;
    }

    // change the size of the last array
    if ((id+1)*range > inpsize)
        range = inpsize - id*range;

    // up-sweep
    up_sweep(array, range, id, numprocs);

    // down-sweep
    if (id == numprocs-1) array[range-1] = -3.1416/2;
    int pow1;
    int pow2;
    int dst;
    float n_num;
    float root;
    MPI_Status stat;

    root = array[range-1];
    for (int d = log2(numprocs); d >= 0; d--){
        pow1 = pow(2,d);
        pow2 = pow(2,d+1);
        if (id % pow2 == pow1 - 1 && id != numprocs-1){
            dst = min(id+pow1, numprocs-1);
            //cout << id << ":l:"<<d<<" send to:   " << dst << endl;
            MPI_Send(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD);
            //cout << id << ":l:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD, &stat);
            //break;
        } else if (id % pow2 == pow2-1){
            //cout << id << ":r:"<<d<<" recv from: " << id - pow1 << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, id - pow1, 0, MPI_COMM_WORLD, &stat);
            //cout << id << ":r:"<<d<<" send to:   " << id - pow1 << endl;
            MPI_Send(&root  , 1, MPI_FLOAT, id - pow1, 0, MPI_COMM_WORLD);
            root = max(root, n_num);
        } else if (id == numprocs-1 && numprocs % pow2 > pow1 ){
            //cout << id << ":+:"<<d<<" recv from: " << id - numprocs % pow1 << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, id - numprocs % pow1, 0, MPI_COMM_WORLD, &stat);
            //cout << id << ":+:"<<d<<" send to:   " << id - numprocs % pow1 << endl;
            MPI_Send(&root  , 1, MPI_FLOAT, id - numprocs % pow1, 0, MPI_COMM_WORLD);
            root = max(root, n_num);
        }
    }
    array[range-1] = root;
    //cout << id << ": " ;
    //for (int i = 0; i < range; i++)
    //    cout << array[i] << ", ";
    //cout << endl;
    down_sweep_seq(array, range, id);

    MPI_Barrier(MPI_COMM_WORLD);
    //cout << id << ": ";
    //for (int i = 0; i < range; i++)
    //    if (angle[i] > array[i])
    //        cout << angle[i]<< " > "<<array[i] <<"v, ";
    //        //cout  <<"v,";
    //    else
    //        cout << angle[i]<< " ! "<<array[i] <<"u, ";
    //        //cout << "u,";
    //cout << endl;

    //cout << id << "----------------------------------_---------"<< endl;
    int res;
    if (id == 0){
        cout  <<"_,";
        for (int i = 0; i < inpsize; i++){
            if (i < range){
                res = angle[i] > array[i];
            } else {
                //cout << id << ":r: "<< i/range << endl;;
                MPI_Recv(&res, 1, MPI_INT, i/range, 0, MPI_COMM_WORLD, &stat);
                //cout << " val: " << res << endl;
            }

            if (res)
                cout  <<"v";
            else
                cout <<"u";
            if (i != inpsize-1)
                cout <<",";
        }
        cout << endl;
    } else {
        for (int i = 0; i < range; i++){
            res = angle[i] > array[i];
            //cout << id << ":s: "<< i << " val: " << res << endl;
            MPI_Send(&res , 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    delete []array;
    delete []angle;
    MPI_Finalize();
    return 0;
}
