/*
 * This uses the recommended GLib API for D-Bus: GDBus,
 * which has been distributed with GLib since version 2.26.
 *
 * For details of how to use GDBus, see:
 * https://developer.gnome.org/gio/stable/gdbus-convenience.html
 *
 * dbus-glib also exists but is deprecated.
 */
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h> /* for glib main loop */


void test_Ping(GDBusProxy *proxy)
{
	GVariant *result;
	GError *error = NULL;
	const gchar *str;

	const char* login = "WhoAmI";
	const char* password = "MySceretPassword";

	g_printf("Calling Auth()...\n");
	result = g_dbus_proxy_call_sync(proxy,
					"Auth",
					g_variant_new ("(ss)",
								login,
								password),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);
	g_assert_no_error(error);
	g_variant_get(result, "(&s)", &str);
	g_printf("The server answered: '%s'\n", str);
	g_variant_unref(result);
}


void test_CreateUser(GDBusProxy *proxy)
{

	GVariant *result;
	GError *error = NULL;
	const gchar *str;

	const char* login = "";
	const char* password = "";

	g_printf("Calling CreateUser()...\n");
	result = g_dbus_proxy_call_sync(proxy,
					"CreateUser",
					g_variant_new ("(ss)",
								login,
								password),
					G_DBUS_CALL_FLAGS_NONE,
					-1,
					NULL,
					&error);
	g_assert_no_error(error);
	g_variant_get(result, "(&s)", &str);
	g_printf("The server answered: '%s'\n", str);
	g_variant_unref(result);

}

int main(void)
{
	GDBusProxy *proxy;
	GDBusConnection *conn;
	GError *error = NULL;
	const char *version;
	GVariant *variant;

	conn = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	g_assert_no_error(error);

	proxy = g_dbus_proxy_new_sync(conn,
				      G_DBUS_PROXY_FLAGS_NONE,
				      NULL,				
				      "org.example.AuthServer",		
				      "/org/example/MyAuthObject",	
				      "org.example.SimpleLoginInterface",
				      NULL,				
				      &error);
	g_assert_no_error(error);

	variant = g_dbus_proxy_get_cached_property(proxy, "Version");
	g_assert(variant != NULL);
	g_variant_get(variant, "s", &version);
	g_variant_unref(variant);
	printf("Testing server interface v%s\n", version);

	test_Ping(proxy);
	test_CreateUser(proxy);

	g_object_unref(proxy);
	g_object_unref(conn);
	return 0;
}
