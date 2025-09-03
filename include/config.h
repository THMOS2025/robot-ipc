/* config.h: 
 *      Configurations for this project.
 *      
 *      Notice that this file shouldn't be included by 
 *      external project, otherwise may caused macro
 *      confliction. 
 */

#ifndef _H_CONFIG
#define _H_CONFIG

/* The max length of paths and names */
#define NAME_MAX_LENGTH 256

/* Super variables use shared memory as backend implement.
 * To avoid directly locking the whole block, we use a queue
 * to manage multiple block, which allows multiple IO at the 
 * same time. See producer-consumer problem for help.
 * The bitwise status of buffers and the pointer referring to 
 * the lastest buffer are compressed into a uin64. So, the 
 * number of buffers should not exceed 14. Recommand to be the 
 * power of 2 ( for alignmen ). 
 */
#define SHM_BUFFER_CNT 4

/* Super funtion prefix in pipe */
#define PIPE_NAME_PREFIX "/tmp/robot_ipc/"

/* Maximum number of epoll events in a single query */
#define MAX_EPOLL_EVENTS 16

#endif // _H_CONFIG
