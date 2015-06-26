#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <time.h>
 
##ifndef MAX_QUEUE_LENGTH
#define MAX_QUEUE_LENGTH 50
#endif
 
 
##ifndef MAX_PACKET_DATA_LENGTH
#define MAX_PACKET_DATA_LENGTH 2000
#endif
 
##ifndef MAX_DEVICE_HASH_SIZE
#define MAX_DEVICE_HASH_SIZE 10000
#endif
 
##ifndef MAX_DEVICE_HASH_SIZE
#define MAX_SERVICE_HASH_SIZE 5000
#endif
 
 
#define ERR_NO_QUEUE_SPACE 20
#define ERR_NOTHING_IN_QUEUE 21
 
 
typedef struct queue_message {
 
    char* message_type;
 
    char* requesting_service_TAG;
 
    char* requesting_user_TAG;
 
    char* requesting_device_TAG;
 
    void* data;
 
    bool* is_open_stream;
    char* data_stream
 
}queue_message;
 
typedef struct in_device_info {
 
    int device_id_no;
    char* device_TAG;
 
    char* deviceState;
 
    bool isPendingDisconnect;
    bool isDisconnected;
    bool isConnected;
    bool isDiscovered;
    device_info_linked_list_node* device_node;
 
 
    bool is_record_available;
    char* record[];
 
    bool is_incomming_request;
    queue_message* incomming_request_queue[];
 
    bool is_outgoing_request;
    queue_message* outgoing_request_queue[];
 
 
}in_device_info;
 
 
typedef struct device_info_linked_list_node {
    device_info_linked_list* previous;
    device_info_linked_list* next;
 
    in_device_info*  device_info;
}
 
typedef struct locally_networked_device_struct {
//[device id no. | device TAG | is_record_available | record | is_incomming_request | incomming requests |is_outgoing |outgoing requests]
 
    //has using the device id then can strcmp(deviceTAG, deviceTAG)
    in_device_info* in_devices_hash_table[];
    int numberOfEntries;
 
    device_info_linked_list_node* disocvered_list;
    device_info_linked_list_node* connected_list;
 
    device_info_linked_list_node* pending_disconnect_list;
    device_info_linked_list_node* disconnected_list;
 
}device_tag_hash_table;
 
 
 
 
 
typedef struct service_info_struct {
 
}service_info_struct;
 
 
 
 
 
typedef struct services_struct {
 
    struct registeredServices {
        service_info_struct* service_tag_hash_list[];
    }
 
    struct onServices {
        service_info_struct* service_tag_hash_list[];
    }
 
    struct all_connected_services {
        service_info_struct* service_tag_hash_list[];
 
    }
 
}services_struct;
 
 
 
 
 
typedef struct thread_args{
 
    char *thread_type;
 
    locally_networked_device_struct* locally_networked_devices;
 
    services_struct* services_struct;
 
    int *inQueueAddIndex;
    queue_message* inQueue[]
 
    int *outQueueAddIndex;
    queue_message* outQueue[];
 
}thread_args;
 
 
bool initialized = false;
services_struct* service_struct_;
 
 
//these variables used by the broadcast thread
 
int broadcast_in_queue_add_index = 0;
int broadcast_in_queue_remove_index = 0;
int amount_in_broadcast_in_queue = 0;
queue_message* broadcast_in_queue[];
 
 
int broadcast_out_queue_add_index = 0;
int broadcast_out_queue_remove_index = 0;
int amount_out_broadcast_out_queue = 0;
queue_message* broadcast_out_queue[];
 
 
//all networked
locally_networked_device_struct* locally_networked_devices;
 
bool initialize_all(){
 
    if(initialized == true) {
        return false;
    }
     
    //SERVICE INFOS INCLUDING QUEUES AND REGISTERED AND WHAT NOT 
    service_struct_ = (services_struct*)malloc(sizeof(services_struct));
 
    service_struct_->registeredServices->service_tag_hash_list = (services_struct*)malloc(sizeof(services_struct*MAX_SERVICE_HASH_SIZE));
    service_struct_->onServices->service_tag_hash_list = (services_struct*)malloc(sizeof(services_struct*MAX_SERVICE_HASH_SIZE));
    service_struct_->all_connected_services->service_tag_hash_list = (services_struct*)malloc(sizeof(services_struct*MAX_SERVICE_HASH_SIZE));
 
 
 
    //CREATE THE QUEUES FOR THE THREADS
    broadcast_in_queue_add_index = 0;
    broadcast_in_queue_remove_index = 0;
    amount_in_broadcast_in_queue = 0;
 
    broadcast_in_queue = (queue_message**)malloc(sizeof((queue_message*)*MAX_QUEUE_LENGTH));
 
 
    broadcast_out_queue_add_index = 0;
    broadcast_out_queue_remove_index = 0;
    amount_out_broadcast_out_queue = 0;
 
    broadcast_out_queue = (queue_message**)malloc(sizeof((queue_message*)*MAX_QUEUE_LENGTH));
 
 
 
//LOCALLY NOTWORKED DEVICES LISTS AND HASH TABLE
    locally_networked_devices = (locally_networked_device_struct*)malloc(sizeof(locally_networked_device_struct));
 
    locally_networked_devices->in_devices_hash_table = (in_device_info**)malloc(sizeof( (in_device_info*)*MAX_DEVICE_HASH_SIZE ));
 
    locally_networked_devices->numberOfEntries = 0;
 
    locally_networked_devices->disocvered_list = NULL;
    locally_networked_devices->connected_list = NULL;
 
    locally_networked_devices->pending_disconnect_list = NULL;
    locally_networked_devices->disconnected_list = NULL;
 
}
 
 
static pthread_mutex_t in_lock; 
static pthread_mutex_t out_lock;
 
 
pthread_t broadcast_thread;
 
void main(int arc, char *arv[]) {
 
 
    in_lock = PTHREAD_MUTEX_INITIALIZER;
    out_lock = PTHREAD_MUTEX_INITIALIZER;
 
 
 
 
 
 
    thread_args* broadcast_thread_args = (thread_args*)malloc(sizeof(thread_args));
    broadcast_thread_args->thread_type = (char*)malloc(sizeof(char*20));
 
    strcpy(broadcast_thread_args->thread_type, "BROADCAST");
 
    broadcast_thread_args->locally_networked_devices = locally_networked_devices;
 
    broadcast_thread_args->services_struct = service_struct_;
 
    broadcast_thread_args->inQueueAddIndex = &broadcast_in_queue_add_index;
    broadcast_thread_args->inQueue = broadcast_in_queue;
 
 
    broadcast_thread_args->inQueueAddIndex = &broadcast_in_queue_add_index;
    broadcast_thread_args->outQueue = broadcast_out_queue;
 
 
    broadcast_out_queue = (queue_message*)malloc(sizeof(queue_message*MAX_QUEUE_LENGTH));
 
 
    pthread_create(&broadcast_thread, NULL, run_thread, broadcast_thread_args);
 
}
 
void *run_thread(void *args) {
 
    struct sockaddr_in this_device_udp_addr;
 
    if(strcmp(args->thread_type , "BROADCAST")==0){
 
 
 
    }
 
}
 
 
int error_queue_index;
int error_queue[100];
 
 
bool remove_from_queue(*queue_message, queue_message* queue[], int* remove_index, int* amount_in_queue, queue_message *message) {
    if(*amount_in_queue == 0){
        error_queue[error_queue_index] = ERR_NOTHING_IN_QUEUE;
        error_queue_index++;
        return false;
    }
    else{
        *message = *queue[*remove_index];
        if(*remove_index == MAX_QUEUE_LENGTH-1) {
            *remove_index = 0;
        }
        else{
            *remove_index++;
        }
        return true;
    }
}
 
 
//could cause race condition, use semaphores
bool add_to_queue(*queue_message, queue_message* queue[], int* add_index, int* amount_in_queue) {
 
    if(*amount_in_queue >= MAX_QUEUE_LENGTH){
        error_queue[error_queue_index] = ERR_NO_QUEUE_SPACE;
        error_queue_index++;
        return false;
    }
    else{
        *queue[*add_index] = queue_message;
        if(*add_index == MAX_QUEUE_LENGTH-1){
            *add_index = 0;
        }
        else{
            *add_index++;
        }
        return true;
    }
 
}
 
// bool test_add_to_queue(int* add_index, int* remove_index, int* amount_in_queue) {
//  //either add index > remove_index
 
//  if(add_index > remove_index) {
//      if(add_index == MAX_QUEUE_LENGTH -1){
//          if(remove_index > 0) {
//              return true;
//          }
//          else {
//              return false;
//          }
//      }
//      else{
//          return true;
//      }
//  }
//  else {
//      if(add_index == remove_index)
//  }
// }