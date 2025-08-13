#ifndef ROBOT_IPC_CODES_H
#define ROBOT_IPC_CODES_H

// Success (normal): 
#define ROBOT_OK 0   // Nothing to log

// Info codes: 1 ~ 99


// Warning codes: 100 ~ 199
#define WARN_SHM_NOREAD 100   // Nothing to read
#define WARN_SHM_QUFULL 101   // The circle queue is full


// Error codes: 200 ~ 299
#define ERR_SHM_AQMETALOCK   200    // Failed to acquire meta lock
#define ERR_SHM_NOWRITETIME  201    // Failed to write timestamp due to failed ti acquire meta lock

// Fatal/exceptional errors: 300+
#define FATAL_SHM_CORRUPT 300   // Shared memory corrupted



#endif // ROBOT_IPC_CODES_H