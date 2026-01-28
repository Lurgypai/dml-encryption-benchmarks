/* Adapted from the example provided at https://github.com/LifeboatLLC/HDF5-Encryption/blob/main/hdf5/hdf5-1_14_3/examples/h5ex_d_rdwr_crypt.c
*/

#include "hdf5.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILE    "h5ex_d_rdwr_crypt.h5"
#define DATASET "DS1"

static hid_t prepare_def() {
    return H5P_DEFAULT;
}

static hid_t prepare_crypt() {
    hid_t fapl_id = H5I_INVALID_HID;

    H5FD_pb_vfd_config_t     pb_vfd_config =
    {
        /* magic          = */ H5FD_PB_CONFIG_MAGIC,
        /* version        = */ H5FD_CURR_PB_VFD_CONFIG_VERSION,
        /* page_size      = */ H5FD_PB_DEFAULT_PAGE_SIZE,
        /* max_num_pages  = */ H5FD_PB_DEFAULT_MAX_NUM_PAGES,
        /* rp             = */ H5FD_PB_DEFAULT_REPLACEMENT_POLICY,
        /* fapl_id        = */ H5P_DEFAULT  /* will overwrite */
    };
    H5FD_crypt_vfd_config_t  crypt_vfd_config =
    {
        /* magic                  = */ H5FD_CRYPT_CONFIG_MAGIC,
        /* version                = */ H5FD_CURR_CRYPT_VFD_CONFIG_VERSION,
        /* plaintext_page_size    = */ 4096,
        /* ciphertext_page_size   = */ 4112,
        /* encryption_buffer_size = */ H5FD_CRYPT_DEFAULT_ENCRYPTION_BUFFER_SIZE,
        /* cipher                 = */ 0,   /* AES256 */
        /* cipher_block_size      = */ 16,
        /* key_size               = */ 32,
        /* key                    = */ H5FD_CRYPT_TEST_KEY,
        /* iv_size                = */ 16,
        /* mode                   = */ 0,
        /* fapl_id                = */ H5P_DEFAULT
    };
    hid_t                    crypt_fapl_id    = H5I_INVALID_HID;

    crypt_fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_crypt(crypt_fapl_id, &crypt_vfd_config);
    pb_vfd_config.fapl_id = crypt_fapl_id;

    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_pb(fapl_id, &pb_vfd_config);

    H5Pclose (crypt_fapl_id);
    return fapl_id;
}

// 4096
#define CHUNK_DIM   (64 / sizeof(int))
#define CHUNK_SIZE  (CHUNK_DIM * CHUNK_DIM)

int main(int argc, char** argv)
{
    hid_t   file  = H5I_INVALID_HID;
    hid_t   space = H5I_INVALID_HID;
    hid_t   dset  = H5I_INVALID_HID;
    hid_t   fapl_id = H5I_INVALID_HID;
    herr_t  status;
    hsize_t i, j;

    // parse arg type
    if (argc != 4) {
        printf("ERROR: incorrect arg count\n");
        return -1;
    }

    if (strcmp(argv[1], "def") == 0) {
        fapl_id = prepare_def();
    }
    else if (strcmp(argv[1], "crypt") == 0) {
        fapl_id = prepare_crypt();
    }
    else {
        printf("ERROR: invalid argument\n");
        return -1;
    }

    int DIM0 = atoi(argv[2]);
    int DIM1 = atoi(argv[3]);

    if(DIM0 % CHUNK_DIM != 0 || DIM1 % CHUNK_DIM != 0) {
        printf("Error, dims are not separatable into chunks (dims size %d and %d, chunk size %dx%d)\n", DIM0, DIM1, CHUNK_DIM, CHUNK_DIM);
        return -1;
    }

    int* buffer = malloc(CHUNK_SIZE * sizeof(int));
    for(int i = 0; i != CHUNK_SIZE; ++i) {
        buffer[i] = i;
    }

    hsize_t dims[2] = {DIM0, DIM1};

    // create
    file = H5Fcreate(FILE, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);
    space = H5Screate_simple(2, dims, NULL);
    dset = H5Dcreate(file, DATASET, H5T_STD_I32LE, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    // write and close

    int x_steps = DIM0 / CHUNK_DIM;
    int y_steps = DIM1 / CHUNK_DIM;

    clock_t start, end;
    double elapsed;

    start = clock();

    hsize_t ioDims[2] = {CHUNK_DIM, CHUNK_DIM};
    hid_t ioSpace = H5Screate_simple(2, ioDims, NULL);

    for(int x_pos = 0; x_pos != x_steps; ++x_steps) {
        for(int y_pos = 0; y_pos != y_steps; ++y_steps) {
            hsize_t offset[2] = {x_pos * CHUNK_DIM, y_pos * CHUNK_DIM};
            // select where to write to
            H5Sselect_hyperslab(space,
                    H5S_SELECT_SET,
                    offset,
                    NULL,
                    ioDims,
                    NULL );
            status = H5Dwrite(dset, H5T_NATIVE_INT, ioSpace, space, H5P_DEFAULT, buffer);
        }
    }

    status = H5Fclose(file);
    end = clock();

    status = H5Dclose(dset);
    status = H5Sclose(space);

    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Write: %f\n", elapsed);

    // open and read
    file = H5Fopen(FILE, H5F_ACC_RDONLY, fapl_id);
    dset = H5Dopen(file, DATASET, H5P_DEFAULT);
    space = H5Dget_space(dset);

    start = clock();

    for(int x_pos = 0; x_pos != x_steps; ++x_steps) {
        for(int y_pos = 0; y_pos != y_steps; ++y_steps) {
            hsize_t offset[2] = {x_pos * CHUNK_DIM, y_pos * CHUNK_DIM};
            // select where to write to
            H5Sselect_hyperslab(space,
                    H5S_SELECT_SET,
                    offset,
                    NULL,
                    ioDims,
                    NULL );
            status = H5Dread(dset, H5T_NATIVE_INT, ioSpace, space, H5P_DEFAULT, buffer);
        }
    }
    end = clock();

    elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Read: %f\n", elapsed);

    // cleanup
    status = H5Dclose(dset);
    status = H5Fclose(file);
    status = H5Pclose (fapl_id);

    return 0;
}
