#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib/gprintf.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

const char *version = "0.1";
GMainLoop *mainloop;

const char *server_introspection_xml =
	DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
	"<node>\n"

	"  <interface name='org.freedesktop.DBus.Introspectable'>\n"
	"		<method name='Introspect'>\n"
	"			<arg name='data' type='s' direction='out' />\n"
	"		</method>\n"
	"  </interface>\n"
	
	"  <interface name='org.freedesktop.DBus.Properties'>\n"
	"    	<method name='Get'>\n"
	"      		<arg name='interface' type='s' direction='in' />\n"
	"      		<arg name='property'  type='s' direction='in' />\n"
	"      		<arg name='value'     type='s' direction='out' />\n"
	"    	</method>\n"

	"    	<method name='GetAll'>\n"
	"      		<arg name='interface'  type='s'     direction='in'/>\n"
	"      		<arg name='properties' type='a{sv}' direction='out'/>\n"
	"    	</method>\n"
	"  </interface>\n"

	"  <interface name='org.example.SimpleLoginInterface'>\n"

	"		<property name='Version' type='s' access='read' />\n"

	"		<method name='Auth' >\n"
	"      		<arg name='login'  type='s' direction='in' />\n"
	"      		<arg name='password'  type='s' direction='in' />\n"
	"      		<arg type='s' direction='out' />\n"
	"    	</method>\n"

	"    	<method name='CreateUser' >\n"
	"      		<arg name='login'  type='s' direction='in' />\n"
	"      		<arg name='password'  type='s' direction='in' />\n"
	"      		<arg type='s' direction='out' />\n"
	"    	</method>\n"

	"  </interface>\n"
	"</node>\n";

DBusHandlerResult server_get_properties_handler(const char *property, DBusConnection *conn, DBusMessage *reply) {

	if (!strcmp(property, "Version")) {

		dbus_message_append_args(reply,
					 DBUS_TYPE_STRING, &version,
					 DBUS_TYPE_INVALID);
	} else {

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	if (!dbus_connection_send(conn, reply, NULL)) {

		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult server_get_all_properties_handler(DBusConnection *conn, DBusMessage *reply) {

	DBusHandlerResult result;
	DBusMessageIter array, dict, iter, variant;
	const char *property = "Version";

	result = DBUS_HANDLER_RESULT_NEED_MEMORY;

	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &array);

	property = "Version";
	dbus_message_iter_open_container(&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
	dbus_message_iter_append_basic(&dict, DBUS_TYPE_STRING, &property);
	dbus_message_iter_open_container(&dict, DBUS_TYPE_VARIANT, "s", &variant);
	dbus_message_iter_append_basic(&variant, DBUS_TYPE_STRING, &version);
	dbus_message_iter_close_container(&dict, &variant);
	dbus_message_iter_close_container(&array, &dict);

	dbus_message_iter_close_container(&iter, &array);

	if (dbus_connection_send(conn, reply, NULL))
		result = DBUS_HANDLER_RESULT_HANDLED;
	return result;
}

DBusHandlerResult server_message_handler(DBusConnection *conn, DBusMessage *message, void *data) {

	DBusHandlerResult result;
    DBusMessage *reply = NULL;
	DBusError err;
	bool quit = false;

	fprintf(stderr, "Got D-Bus request: %s.%s on %s\n",
		dbus_message_get_interface(message),
		dbus_message_get_member(message),
		dbus_message_get_path(message));

	dbus_error_init(&err);

	if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect")) {

		if (!(reply = dbus_message_new_method_return(message))) {

			dbus_message_append_args(reply,
						DBUS_TYPE_STRING, &server_introspection_xml,
						DBUS_TYPE_INVALID);
		}

	}  else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "Get")) {

		const char *interface, *property;

		if (dbus_message_get_args(message, &err,
					   DBUS_TYPE_STRING, &interface,
					   DBUS_TYPE_STRING, &property,
					   DBUS_TYPE_INVALID)) {

			if ((reply = dbus_message_new_method_return(message)))
			{

				result = server_get_properties_handler(property, conn, reply);
				dbus_message_unref(reply);
				return result;
			}
		}

	}  
	else if (dbus_message_is_method_call(message, DBUS_INTERFACE_PROPERTIES, "GetAll"))  {

		if ((reply = dbus_message_new_method_return(message))) {

			result = server_get_all_properties_handler(conn, reply);
			dbus_message_unref(reply);
			return result;
		}

	}  else if (dbus_message_is_method_call(message, "org.example.SimpleLoginInterface", "Auth"))  {

		const char *auth_result = "Access is denied";

		if ((reply = dbus_message_new_method_return(message))) {

			const char* login;
			const char* password;

			if (!dbus_message_get_args(message, &err,
								DBUS_TYPE_STRING, &login,
								DBUS_TYPE_STRING, &password,
								DBUS_TYPE_INVALID))
			g_printf("\n");
			g_printf("Login: %s Password: %s\n", login, password);


			dbus_message_append_args(reply,
						DBUS_TYPE_STRING, &auth_result,
						DBUS_TYPE_INVALID);	

		}

	}   else if (dbus_message_is_method_call(message, "org.example.SimpleLoginInterface", "CreateUser"))  {

		const char* craete_user_result = "This functionality has not yet been implemented";
	
		if ((reply = dbus_message_new_method_return(message)))
		{
			dbus_message_append_args(reply,
				DBUS_TYPE_STRING, &craete_user_result,
				DBUS_TYPE_INVALID);	
		}

	}  else {	

		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	}

	if (dbus_error_is_set(&err)) {

		if (reply)
			dbus_message_unref(reply);
		reply = dbus_message_new_error(message, err.name, err.message);
		dbus_error_free(&err);
	}
	
	if (!reply) {

		return DBUS_HANDLER_RESULT_NEED_MEMORY;
	}

	result = DBUS_HANDLER_RESULT_HANDLED;
	if (!dbus_connection_send(conn, reply, NULL)) {

		result = DBUS_HANDLER_RESULT_NEED_MEMORY;
	}
	dbus_message_unref(reply);

	if (quit) {

		fprintf(stderr, "Server exiting...\n");
		g_main_loop_quit(mainloop);
	}
	return result;
}


const DBusObjectPathVTable server_vtable = {
	.message_function = server_message_handler
};


int main(void)
{
	DBusConnection *conn;
	DBusError err;
	int rv;

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
	if (!conn) 
	{
		fprintf(stderr, "Failed to get a session DBus connection: %s\n", err.message);
		dbus_error_free(&err);
		return EXIT_FAILURE;
	}

	rv = dbus_bus_request_name(conn, "org.example.AuthServer", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (rv != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) 
	{
		fprintf(stderr, "Failed to request name on bus: %s\n", err.message);
		dbus_error_free(&err);
		return EXIT_FAILURE;
	}

	if (!dbus_connection_register_object_path(conn, "/org/example/MyAuthObject", &server_vtable, NULL)) 
	{
		fprintf(stderr, "Failed to register a object path for 'TestObject'\n");
		dbus_error_free(&err);
		return EXIT_FAILURE;
	}

	printf("Starting dbus tiny server v%s\n", version);
	mainloop = g_main_loop_new(NULL, false);
	dbus_connection_setup_with_g_main(conn, NULL);
	g_main_loop_run(mainloop);

	return EXIT_SUCCESS;
}

