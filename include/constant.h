/* constant.h
 */

#ifndef ROBOT_IPC_CODES_H
#define ROBOT_IPC_CODES_H


/*************************
 * error code definition *
 *************************/
// Success code; 0
#define ROBOT_OK 0

// Info codes: 1 ~ 99

// Warning codes: 100 ~ 199
#define WARN_SHM_NOREAD 100   // Nothing to read
#define WARN_SHM_QUFULL 101   // The circle queue is full

// Error codes: 200 ~ 299
#define ERR_SHM_AQMETALOCK   200    // Failed to acquire meta lock
#define ERR_SHM_NOWRITETIME  201    // Failed to write timestamp due to failed ti acquire meta lock

// Fatal/exceptional errors: 300+
#define FATAL_SHM_CORRUPT 300   // Shared memory corrupted


/********************
 * global arguments *
 ********************/

/* Super variables use shared memory as backend implement.
 * To avoid directly locking the whole block, we use a queue
 * to manage multiple block, which allows multiple IO at the 
 * same time. See producer-consumer problem for help.
 * To accelerate modulo operation, this must be the power of 2. */
#define CIRCLE_QUEUE_LENGTH 4 


#endif // ROBOT_IPC_CODES_H
