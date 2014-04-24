#include <v8.h>
#include <node.h>

#include <iostream>
#include <string.h>
#include <libudev.h>

using namespace v8;

static struct udev *udev;

static const char* GetMediaType(struct udev_device* dev){
    //Check if the device is probably an external storage device (but not SATA)
    const char *id_bus = udev_device_get_property_value(dev, "ID_BUS");
    const char *id_fs_type = udev_device_get_property_value(dev, "ID_FS_TYPE");
    const char *id_usb_driver = udev_device_get_property_value(dev, "ID_USB_DRIVER");
    const char *id_model = udev_device_get_property_value(dev, "ID_MODEL");
    const char *id_type = udev_device_get_property_value(dev, "ID_TYPE");

    bool isSata = id_bus != NULL && strcmp(id_bus, "ata") == 0;

    bool isUsb = id_fs_type != NULL 
                    && id_usb_driver != NULL
                    && strcmp(id_usb_driver, "usb-storage") == 0; 

    bool isMMC = id_fs_type != NULL 
                    && id_model != NULL 
                    && strcmp(id_model, "SD_MMC") == 0;

    bool isDVD = id_type != NULL
                    && strcmp(id_type, "cd") == 0;

    bool isUnknown = id_fs_type != NULL;


    if(isDVD){
        return "dvd";
    }

    //DVD is also ata driver, so needs to be after isDVD check
    if(isSata){
        return NULL;
    }

    if(isMMC){
        return "sdcard";
    }

    if(isUsb){
        return "usb";
    }

    if(isUnknown){
        return "unknown";
    }
    
    return NULL;
}

static const char* ConvertChangeAction(struct udev_device* dev){
    const char *action = udev_device_get_property_value(dev, "ACTION");

    if(action == NULL){
        return NULL;
    }

    //If action is add or remove, do nothing and return that action instead 
    if(strcmp(action, "change") != 0){
        return action;
    }

    //Otherwise try to convert the change either to add or remove
    const char* media_type = GetMediaType(dev);

    //If there is no media-type, then there is something totally wrong
    if(media_type == NULL){
        return NULL;
    }

    const char *id_fs_type = udev_device_get_property_value(dev, "ID_FS_TYPE");
    const char *disk_eject_request = udev_device_get_property_value(dev, "DISK_EJECT_REQUEST");
    

    //DVD and change
    if(strcmp(media_type, "dvd") == 0){
        //Add
        if(id_fs_type != NULL){
            return "add";
        }

        //Remove
        if(disk_eject_request != NULL && strcmp(disk_eject_request, "1") == 0){
            return "remove";
        }
    }

    return NULL;
}

static void PushProperties(Local<Object> obj, struct udev_device* dev) {
    struct udev_list_entry* sysattrs;
    struct udev_list_entry* entry;

    //Retrieve all properties of the given device
    sysattrs = udev_device_get_properties_list_entry(dev);

    udev_list_entry_foreach(entry, sysattrs) {
        const char *name, *value;
        name = udev_list_entry_get_name(entry);
        value = udev_list_entry_get_value(entry);
        if (value != NULL) {
            obj->Set(String::New(name), String::New(value));
        } else {
            obj->Set(String::New(name), Null());
        }
    }

    //Add an additional property to distinct the media type
    const char* media_type = GetMediaType(dev);

    if(media_type != NULL){
        obj->Set(String::New("MEDIA_TYPE"), String::New(media_type)); 
    }
}

class Monitor : public node::ObjectWrap {
    struct poll_struct {
        Persistent<Object> monitor;
    };
    
    public:
    static void Init(Handle<Object> target) {
        Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
       
        tpl->SetClassName(String::NewSymbol("Monitor"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        tpl->PrototypeTemplate()->Set(String::NewSymbol("close"),
                                      FunctionTemplate::New(Close)->GetFunction());

        Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());

        target->Set(String::NewSymbol("Monitor"), constructor);
    };

    private:
    static void on_handle_close(uv_handle_t *handle) {
        poll_struct* data = (poll_struct*)handle->data;
        data->monitor.Dispose();
        delete data;
        delete handle;
    }

    static void on_handle_event(uv_poll_t* handle, int status, int events) {
        HandleScope scope;

        poll_struct* data = (poll_struct*)handle->data;

        Monitor* wrapper = ObjectWrap::Unwrap<Monitor>(data->monitor);
        udev_device* dev = udev_monitor_receive_device(wrapper->mon);

        const char *media_type = GetMediaType(dev);

        TryCatch tc;

        //Only emit if there is an actual device
        if(media_type != NULL){
            //Convert Change Actions correctly, also only emit, if there was an actual action
            const char* action = ConvertChangeAction(dev);
            if(action){
                Local<Object> obj = Object::New();
                obj->Set(String::NewSymbol("syspath"), String::New(udev_device_get_syspath(dev)));
                PushProperties(obj, dev);

                Local<Value> emit_v = data->monitor->Get(String::NewSymbol("emit"));
                Local<Function> emit = Local<Function>::Cast(emit_v);
                Local<Value> emitArgs[2];

                obj->Set(String::New("ACTION"), String::New(action));

                //TODO: Probably add some additional routines to transform values  

                //Emit add or remove event
                emitArgs[0] = String::NewSymbol(action);
                emitArgs[1] = obj;
                emit->Call(data->monitor, 2, emitArgs);
                
                //Experimental unified emit for all types
                //emitArgs[0] = String::NewSymbol("device_event");
                //emitArgs[1] = obj;
                //emit->Call(data->monitor, 2, emitArgs);
            }
        }

        udev_device_unref(dev);
        if (tc.HasCaught()) node::FatalException(tc);
    };

    static Handle<Value> New(const Arguments& args) {
        HandleScope scope;

        //For handling udev-events
        uv_poll_t* handle;

        //Convenient Monitor wrapper for managing the C udev monior 
        Monitor* obj = new Monitor();

        //Create the monitor struct, generated by libudev and connect to event-source "udev" 
        obj->mon = udev_monitor_new_from_netlink(udev, "udev");
        
		//Experimental: Adding type-matching for kernel filter
		udev_monitor_filter_add_match_subsystem_devtype(obj->mon, "block", "partition");
		udev_monitor_filter_add_match_subsystem_devtype(obj->mon, "block", "disk");
	
        //Starting the event polling
		udev_monitor_enable_receiving(obj->mon);
	
        obj->fd = udev_monitor_get_fd(obj->mon);
        obj->poll_handle = handle = new uv_poll_t;
        obj->Wrap(args.This());

        poll_struct* data = new poll_struct;
        data->monitor = Persistent<Object>::New(args.This());

        //Additional parameters for this handle
        handle->data = data;

        uv_poll_init(uv_default_loop(), obj->poll_handle, obj->fd);
        uv_poll_start(obj->poll_handle, UV_READABLE, on_handle_event);

        return args.This();
    };

    static Handle<Value> Close(const Arguments& args) {
        HandleScope scope;

        Monitor* obj = ObjectWrap::Unwrap<Monitor>(args.This());
        uv_poll_stop(obj->poll_handle);
        uv_close((uv_handle_t*)obj->poll_handle, on_handle_close);
        udev_monitor_unref(obj->mon);

        return scope.Close(Undefined());
    };

    uv_poll_t* poll_handle;
    udev_monitor* mon;
    int fd;
};

static Handle<Value> List(const Arguments& args) {
    HandleScope scope;
    Local<Array> list = Array::New();

    struct udev_enumerate* enumerate;
    struct udev_list_entry* devices;
    struct udev_list_entry* entry;
    struct udev_device *dev;

    enumerate = udev_enumerate_new(udev);

    // add match etc. stuff.
    udev_enumerate_add_match_subsystem(enumerate, "block");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);

    int i = 0;
    udev_list_entry_foreach(entry, devices) {
        const char *path;
        path = udev_list_entry_get_name(entry);
        dev = udev_device_new_from_syspath(udev, path);

        const char *media_type = GetMediaType(dev);

        //Only take non-sata devices
        if(media_type != NULL){
            //Create new javascript object and push 
            Local<Object> obj = Object::New();
            PushProperties(obj, dev);
            obj->Set(String::NewSymbol("syspath"), String::New(path));
            list->Set(i++, obj);
        }
        
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);

    return scope.Close(list);
}

static void Init(Handle<Object> target) {
    udev = udev_new();
    if (!udev) {
        ThrowException(String::New("Can't create udev\n"));
    }

    //Publish function "list"
    target->Set(String::NewSymbol("list"),
                FunctionTemplate::New(List)->GetFunction());

    Monitor::Init(target);
}

NODE_MODULE(udev, Init)
