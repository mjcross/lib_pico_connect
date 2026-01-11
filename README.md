# A library for simpler MQTT and network time on the RPi Pico-W
A 'C' library to make it easier to use MQTT and network time in Pico projects.

The Pico SDK provides excellent networking support via the widely-used 3rd party lightweight IP stack 'lwIP'. This provides built-in clients for various [applications](https://www.nongnu.org/lwip/2_0_x/group__apps.html) including MQTT and the simple network time protocol SNTP.

However using these clients in your application involves quite a lot of boilerplate and familiarity with the workings of lwIP.

The purpose of this library is therefore to provide a simple way to get your application off the ground without having to learn all the details. And while it may not expose every feature of the protocols it should at least provide a framework to build on.

In case you are asking why this all seems to be so difficult, have a quick look at the [Behind the scenes](#behind-the-scenes) section below.

## How do I use it?
Start by cloning the repository. It contains a simple VS Code example project that connects to WiFi, synchronises to [pool.ntp.org](pool.ntp.org) and posts timestamps to an MQTT server

> Note: if your MQTT server needs a login, certificates or SSL then you'll have to add those yourself (e.g. see [here](https://github.com/raspberrypi/pico-examples/tree/master/pico_w/wifi/mqtt)). 

To use the example you must **create the file** `lib_connect/private_settings.h` that contains your WiFi login and the hostname of your MQTT server. It should look something like this:

```
#define MQTT_SERVER "your.mqtt.server.address"
#define WIFI_SSID "your WiFi network hostname or IP"
#define WIFI_PASSWORD "your WiFi password"
```

### To use the library in your own code:

1. copy the `lib_connect` folder into your project
2. create the `private_settings.h` file as described above, or define the corresponding settings in your environment
3. copy the `lwipopts.h` file into your project folder, making any customisations you require
    
    *Note: if you use a 'stock' `lwipopts.h` file then add `#include "lib_connect/extra_lwipopts.h"` to it somewhere near the top or manually add the extra settings from that file.*
4. add `lib_connect` to the list of target_link_libraries in your top level `CMakeLists.txt` file or equivalent

    *Note: if you are targeting the RP2040, add `add_compile_definitions(DISABLE_SNTP_COMP_ROUNDTRIP)` (the CMakeLists.txt in the example shows how to do this automatically)*
5. add `#include "lib_connect/connect.h"` to the top of your main source file.

## What functions are provided?
The public API provided by the library is currently:

| Name                  | Type          | Description |
| --------------------- | ------------- | ---------- |
| `void connect()`      | function      | the entry point: start associating with the WiFi network and connecting to the servers |
| `bool network_is_up`  | flag          | indicates whether the network is currently up (i.e. we have a valid IP address) |
| `bool mqtt_is_up`     | flag          | indicates whether we are currently connected to the MQTT server |
| `void publish_mqtt(const char *topic, const void *payload, uint16_t payload_length)` | function | publish to the given topic on the MQTT server |
| `bool aon_timer_is_initialised` | flag  | indicates whether the always-on *(aon)* timer has been successfully initialised from an SNTP server |
| `const char *get_timestamp()` | function | return a string representing the current time of day |
| `int get_time_utc(struct timespec *ts_ptr)` | function | fill in a `timespec` structure with the current UTC time from the always on *(aon)* timer

More functions (for example *mqtt_subscribe*) will be added when I get around to it. In the meantime have a look at the source because the function you're looking for may already be there.

## Behind the scenes
So why is all this so difficult? 

A significant factor is that the lightweight IP stack, a third party component, is *non-reentrant*. This means that if you call a lwIP function while another is in progress, bad things will sooner or later happen.

To work around this the Pico SDK uses a mechanism called an `async_context`. This guarantees that lwIP functions are only called one-by-one and never concurrently. It also (normally) looks after stuff that goes on in the background to keep lwIP and the WiFi driver happy and talking to each other.

You can read more about asynchronous contexts in the [pico_arch_cyw43](https://www.raspberrypi.com/documentation/pico-sdk/networking.html#group_pico_cyw43_arch) and [pico_async_context](https://www.raspberrypi.com/documentation/pico-sdk/high_level.html#group_pico_async_context) sections of the SDK docs; but from a practical perspective it means that to call a lwIP function you add a *worker* structure containing a callback.

A simple example might look like this:
```
static void start_sntp_cb(async_context_t *ctx, async_at_time_worker_t *p_worker) {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();    
}
void start_sntp() {
    static async_at_time_worker_t start_sntp_worker = { .do_work = start_sntp_cb };
    async_context_add_at_time_worker_in_ms(ctx, &start_sntp_worker, 0);
}
```
Here `start_sntp()` is a user-facing function that might be called at any time, i.e. *asynchronously*. It creates a worker that references the callback function `start_sntp_cb()` and schedules it to be run by the async context. The two functions in the callback are the lwIP API calls themselves.

Another part of the story is that lwIP functions tend to be non-blocking calls that fire a user-provided callback when they have something to report. This tends to lead to state-machine type code that isn't always easy to follow.