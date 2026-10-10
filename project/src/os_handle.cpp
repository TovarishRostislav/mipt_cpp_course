#include "os_handle.h"

#include <stdexcept>
#include <string>
namespace nano_edr{

OsHandle::OsHandle(const std::string& config_path){
    const os_status st = os_init(config_path.c_str(), &handle_);
    if (st != OS_OK){
        throw std::runtime_error(
            "os_init failed for '" + config_path + "': " + os_status_str(st));
    }
}

OsHandle::~OsHandle(){
    if (handle_ != nullptr){
        os_free(handle_);
    }
}

void OsHandle::Subscribe(os_event_cb callback, void* context){
    const os_status st = os_event_subscribe(handle_, callback, context);
    if (st != OS_OK){
        throw std::runtime_error(
            std::string("os_event_subscribe failed: ") + os_status_str(st));
    }
}

void OsHandle::Start(){
    const os_status st = os_start(handle_);
    if (st != OS_OK){
        throw std::runtime_error(
            std::string("os_start failed: ") + os_status_str(st));
    }
}

os_status OsHandle::Wait(uint32_t timeout_ms){
    return os_wait(handle_, timeout_ms);
}

void OsHandle::Stop(){
    os_stop(handle_);
}
}