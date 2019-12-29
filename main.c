#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <dbus/dbus.h>

enum Orientation { Undefined, Normal, RightUp, LeftUp, BottomUp };

DBusError error;
atomic_bool running;
char* output = "eDP-1";

void dbus_disconnect(DBusConnection* connection) {
    dbus_connection_close(connection);
    dbus_connection_unref(connection);
    dbus_error_free(&error);
}

DBusConnection* dbus_connect() {
    dbus_error_init(&error);
    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection != NULL) {
        DBusMessage* msg = dbus_message_new_method_call(
            "net.hadess.SensorProxy", "/net/hadess/SensorProxy",
            "net.hadess.SensorProxy", "ClaimAccelerometer");
        char ok = dbus_connection_send_with_reply_and_block(
                      connection, msg, 200, &error) != NULL;
        dbus_message_unref(msg);
        if (ok) {
            return connection;
        }
    }
    return NULL;
}

enum Orientation parse_orientation(DBusMessage* msg) {
    DBusMessageIter args, iter_array, iter_dict, iter_v;
    char *iface, *property, *orientation;
    if (dbus_message_iter_init(msg, &args)) {
        dbus_message_iter_get_basic(&args, &iface);
        if (!strcmp("net.hadess.SensorProxy", iface)) {
            dbus_message_iter_next(&args);
            dbus_message_iter_recurse(&args, &iter_array);
            dbus_message_iter_recurse(&iter_array, &iter_dict);
            dbus_message_iter_get_basic(&iter_dict, &property);
            if (!strcmp(property, "AccelerometerOrientation")) {
                dbus_message_iter_next(&iter_dict);
                dbus_message_iter_recurse(&iter_dict, &iter_v);
                dbus_message_iter_get_basic(&iter_v, &orientation);

                if (!strcmp(orientation, "normal")) {
                    return Normal;
                }
                if (!strcmp(orientation, "bottom-up")) {
                    return BottomUp;
                }
                if (!strcmp(orientation, "left-up")) {
                    return LeftUp;
                }

                if (!strcmp(orientation, "right-up")) {
                    return RightUp;
                }
            }
        }
    }
    return Undefined;
}

void system_fmt(char* format, ...) {
    char command[128];
    va_list args;
    va_start(args, format);
    vsnprintf(command, sizeof(command), format, args);
    system(command);
    va_end(args);
}

void handle_orientation(enum Orientation orientation) {
    switch (orientation) {
        case Normal:
            system_fmt("swaymsg \"output %s transform 0\"", output);
            break;

        case BottomUp:
            system_fmt("swaymsg \"output %s transform 180\"", output);
            break;

        default:
            break;
    }
}

int dbus_loop(DBusConnection* connection) {
    DBusMessage* msg;
    dbus_bus_add_match(connection,
        "type='signal',interface='org.freedesktop.DBus.Properties'", &error);
    dbus_connection_flush(connection);
    atomic_store(&running, 1);
    while (atomic_load(&running)) {
        dbus_connection_read_write(connection, 0);
        msg = dbus_connection_pop_message(connection);

        if (msg == NULL) {
            usleep(100000);
            continue;
        }

        if (dbus_message_is_signal(
                msg, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
            handle_orientation(parse_orientation(msg));
        }

        dbus_message_unref(msg);
    }
    dbus_disconnect(connection);
    return 0;
}

void sig_handler(int s) {
    atomic_store(&running, 0);
}

int main(int argc, char* argv[]) {
    DBusConnection* connection = dbus_connect();
    if (connection == NULL) {
        printf("error: cannot open dbus connection\n");
        return 1;
    }
    if (argc > 1) {
        output = argv[1];
    }
    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    return dbus_loop(connection);
}
