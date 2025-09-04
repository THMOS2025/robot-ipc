/* robot_ipc_constant.h
 *  Definition of constants
 */

#ifndef _H_ROBOT_IPC_CONSTANT
#define _H_ROBOT_IPC_CONSTANT

// Success code; 0
#define ROBOT_OK 0

// Info codes: 1 ~ 99

// Warning codes: 100 ~ 199
#define WARN_SHM_NOREAD     100   // Nothing to read
#define WARN_SHM_QUFULL     101   // The circle queue is full

// Error codes: 200 ~ 299
#define ERR_SHM_AQMETALOCK  200    // Failed to acquire meta lock
#define ERR_SHM_NOWRITETIME 201    // Failed to write timestamp due to failed ti 
#define ERR_PIPE_REQ        202    // Failed to connect to require pipeacquire meta lock
#define ERR_PIPE_CLOSE      203    // Failed to close pipe

// Fatal/exceptional errors: 300+
#define FATAL_SHM_CORRUPT   300   // Shared memory corrupted


#endif
