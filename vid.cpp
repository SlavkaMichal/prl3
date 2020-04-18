#include <mpi.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>

using namespace std;

int p = -1; // verbosity level: 0 non, 1 up_sweep, 2 down_sweep, -1 measure time

/* sequential implementation of scan
   arr   - input array
   range - size of the array
   id    - mpi id of the process
*/
inline float up_sweep_seq(float *arr, int range, int id)
{
    int new_range;
    if (range == 2){ // cannot be divided anymore
        arr[1] = max(arr[1], arr[0]);
    } else if (range > 2) { // divide array and recursively evaluate
        new_range = pow(2, ceil(log2(range))-1);
        arr[range-1] = max(
            up_sweep_seq(arr          , new_range      , id),
            up_sweep_seq(arr+new_range, range-new_range, id));
    }
    return arr[range-1];
}

/* implementation of scan
   arr   - input array
   range - size of the array
   id    - mpi id of the process
   numprocs - number of process
*/
inline float up_sweep(float *arr, int range, int id, int numprocs)
{
    int pow1;
    int pow2;
    int dst;
    float n_num;
    float root;
    MPI_Status stat;

    root = up_sweep_seq(arr, range, id);
    if (numprocs == 1)
        return root;

    for (int d = 0; d < log2(numprocs); d++){
        pow1 = pow(2,d);   // if proc id is multiple of pow1 then is on the left
        pow2 = pow(2,d+1); // if proc id is multiple of pow2 then is on the right

        if (p == 1){// log
            cout << id<<":"<<d << " pow1: " << pow1 << " pow2: " << pow2 << endl;
            cout << id<<":"<<d  << " right: (id+1 \% pow2 == 0) " << (id+1) % pow2 << "==" << 0 << endl;
            cout << id<<":"<<d  << " left:  (id+1 \% pow1 == 0) " << (id+1) % pow1 << "==" << 0 << endl;
        }

        if ((id+1) % pow2 == 0){ // right proc
            if (p == 1) cout << id << ":Right:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, id - pow1, 0, MPI_COMM_WORLD, &stat);
            root = max(n_num, root);
            if (p == 1) cout << id << ":Right:"<<d<<" DONE " << endl;
        } else if ((id+1) % pow1 == 0 && id != numprocs-1){ // left proc
            dst = min(id+pow1, numprocs-1);
            if (p == 1) cout << id << ":Left:"<<d<<" send to:   " << dst << endl;
            MPI_Send(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD);
            if (p == 1) cout << id << ":Left:"<<d<<" DONE " << endl;
        } else if (id == numprocs-1 && numprocs % pow2 > pow1 ){ // rightmost proc
            if (p == 1) cout << id << ":+++:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, id - numprocs % pow1, 0, MPI_COMM_WORLD, &stat);
            if (p == 1) cout << id << ":+++:"<<d<<" DONE " << endl;
            root = max(n_num, root);
        }
    }

    arr[range-1] = root;
    return root;
}

/* seqential implementation of down sweep
   arr   - input array
   range - size of the array
   id    - mpi id of the process
*/
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
    int id;         // mpi process id
    int numprocs;   // number of processes
    int view_point; // first element of the input
    int range;      // numbers per process
    int inpsize;    // number of imputs without view position
    float *array;   // array for results
    float *angle;   // angles
    int res;        // result
    int assigned;   // assigned elements counter
    double start, end; // run time measurements

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);

    view_point = stoi(argv[1]); // altitude from which we computu visibility
    inpsize = argc - 2; // omitting view point

    // edge cases
    if (inpsize <= 1){
        if (id == 0 && inpsize == 1) cout << "_,v" << endl; // only one altitude always visible
        if (id == 0 && inpsize == 0) cout << "_" << endl;   // no altitude
        MPI_Finalize();
        return 0;
    }

    // number of elements assigned to each process
    range = 1;
    while (range*numprocs < inpsize){
        range = range*2; // always power of 2
    }

    // assign numbers to processes
    array = new float [range]{0};
    angle = new float [range]{0};
    assigned = 0;
    for (int i = id*range; i < id*range+range && i < inpsize; i++){
        array[i%range] = atan2((stoi(argv[i+2]) - view_point), (i+1));
        angle[i%range] = array[i%range];
        //cout << id <<": atan(" <<stoi(argv[i+2])- view_point  << ")/"<< i+1 << ")" << "="<<array[i%range]<< endl;
        assigned++;
    }

    // synchronize processes for measuring time
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    // stop if process if it wont be used
    if (assigned == 0){
        MPI_Finalize();
        return 0;
    }

    // change the number of processes to used number of processes
    if (inpsize > numprocs)
        numprocs = inpsize/range+(inpsize%2);

    // change the size of the last array
    if ((id+1)*range > inpsize)
        // should be equal to assigned
        range = inpsize - id*range;

    // only one cpu
    if (numprocs == 1){
        up_sweep_seq(array, range, id); // sequential scan
        array[range-1] = -3.1416/2; // set last element to minimal value
        down_sweep_seq(array, range, id); // sequential down-sweep
        // print result
        cout << "_";
        for (int i=0; i < range; i++){
            if(angle[i] > array[i])
                cout << ",v";
            else
                cout << ",u";
        }
        cout << endl;

        delete []array;
        delete []angle;
        MPI_Finalize();
        return 0;
    }

    // up-sweep
    up_sweep(array, range, id, numprocs);
    if (p == 1){
        MPI_Finalize();
        return 0;
    }

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
        pow1 = pow(2,d);   // if proc id is multiple of pow1 then is on the left
        pow2 = pow(2,d+1); // if proc id is multiple of pow2 then is on the right
        if (p == 2){
            cout << id<<":"<<d << " pow1: " << pow1 << " pow2: " << pow2 << endl;
            cout << id<<":"<<d  << " right: (id+1 \% pow2 == 0) " << (id+1) % pow2 << "==" << 0 << endl;
            cout << id<<":"<<d  << " left:  (id+1 \% pow1 == 0) " << (id+1) % pow1 << "==" << 0 << endl;
        }
        if ((id+1) % pow2 == 0){ // right proc
            dst = id - pow1;
            if (p == 2) cout << id << ":right:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD, &stat);
            if (p == 2) cout << id << ":right:"<<d<<" send to:   " <<  dst << endl;
            MPI_Send(&root  , 1, MPI_FLOAT,  dst, 0, MPI_COMM_WORLD);
            if (p == 2) cout << id << ":right:"<<d<<" DONE " << endl;
            root = max(root, n_num);
        } else if ((id+1) % pow1 == 0 && id != numprocs-1){ // left proc
            dst = min(id+pow1, numprocs-1);
            if (p == 2) cout << id << ":left:"<<d<<" send to:   " << dst << endl;
            MPI_Send(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD);
            if (p == 2) cout << id << ":left:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&root, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD, &stat);
            if (p == 2) cout << id << ":left:"<<d<<" DONE " << endl;
        } else if (id == numprocs-1 && numprocs % pow2 > pow1 ){ // rightmost proc
            dst = id - numprocs % pow1;
            if (p == 2)cout << id << ":+++:"<<d<<" recv from: " << dst << endl;
            MPI_Recv(&n_num, 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD, &stat);
            if (p == 2)cout << id << ":+++:"<<d<<" send to:   " << dst << endl;
            MPI_Send(&root  , 1, MPI_FLOAT, dst, 0, MPI_COMM_WORLD);
            if (p == 2)cout << id << ":+++:"<<d<<" DONE " << endl;
            root = max(root, n_num);
        }
    }
    array[range-1] = root;

    // sequential down sweep
    down_sweep_seq(array, range, id);

    // gather and print results
    if (id == 0){
        cout  <<"_";
        for (int i = 0; i < inpsize; i++){
            if (i < range){ // results in 0-th process
                res = angle[i] > array[i];
                if (p > 0) cout << id << ": " << angle[i] << " > " << array[i] << endl;
            } else { // results from other processes
                MPI_Recv(&res, 1, MPI_INT, i/range, 0, MPI_COMM_WORLD, &stat);
            }

            if (res)
                cout  <<",v";
            else
                cout <<",u";
        }
        cout << endl;
    } else { // send results to 0-th process
        for (int i = 0; i < range; i++){
            res = angle[i] > array[i];
            if (p > 0) cout << id << ": " << angle[i] << " > " << array[i] << endl;
            MPI_Send(&res , 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }
    MPI_Finalize();
    end = MPI_Wtime();
    if (id == 0 && p == -1){
        cout << "inpsize:"<< inpsize<<"\ttime:\t" << end-start << endl;
    }

    delete []array;
    delete []angle;
    return 0;
}
