/*
File:   tests_gtk.c
Author: Taylor Robbins
Date:   01\07\2026
Description: 
	** None
*/

#if (TARGET_IS_LINUX && BUILD_WITH_GTK)

typedef GtkApplication DemoApplication;
typedef GtkApplicationClass DemoApplicationClass;

static GType demo_application_get_type();
G_DEFINE_TYPE(DemoApplication, demo_application, GTK_TYPE_APPLICATION)

typedef struct DemoApplicationWindow DemoApplicationWindow;
struct DemoApplicationWindow
{
  GtkApplicationWindow parent_instance;

  GtkWidget* message;
  GtkWidget* infobar;
  GtkWidget* status;
  GtkWidget* menubutton;
  GMenuModel* toolmenu;
  GtkTextBuffer* buffer;
};
typedef GtkApplicationWindowClass DemoApplicationWindowClass;

static GType demo_application_window_get_type (void);
G_DEFINE_TYPE(DemoApplicationWindow, demo_application_window, GTK_TYPE_APPLICATION_WINDOW)

static DemoApplicationWindow* create_demo_window(GApplication* app, const char* contents);

static void demo_application_init(DemoApplication* app)
{
	// GSettings *settings;
	// GAction *action;
	// settings = g_settings_new ("org.gtk.Demo4.Application");
	// g_action_map_add_action_entries (G_ACTION_MAP (app),
	//                                app_entries, G_N_ELEMENTS (app_entries),
	//                                app);
	// action = g_settings_create_action (settings, "color");
	// g_action_map_add_action (G_ACTION_MAP (app), action);
	// g_object_unref (settings);
}

static void demo_application_class_init(DemoApplicationClass* class)
{
	// GApplicationClass *app_class = G_APPLICATION_CLASS (class);
	// GtkApplicationClass *gtk_app_class = GTK_APPLICATION_CLASS (class);
	// app_class->startup = startup;
	// gtk_app_class->restore_window = restore_window;
}

static void demo_application_window_init(DemoApplicationWindow* window)
{
	// GtkWidget *popover;
	// gtk_widget_init_template (GTK_WIDGET (window));
	// popover = gtk_popover_menu_new_from_model (window->toolmenu);
	// gtk_menu_button_set_popover (GTK_MENU_BUTTON (window->menubutton), popover);
	// g_action_map_add_action_entries (G_ACTION_MAP (window),
	//                                win_entries, G_N_ELEMENTS (win_entries),
	//                                window);
}

static void demo_application_window_dispose(GObject* object)
{
	// DemoApplicationWindow *window = (DemoApplicationWindow *)object;
	// gtk_widget_dispose_template (GTK_WIDGET (window), demo_application_window_get_type ());
	// G_OBJECT_CLASS (demo_application_window_parent_class)->dispose (object);
}

static void demo_application_window_class_init(DemoApplicationWindowClass* class)
{
	// GObjectClass *object_class = G_OBJECT_CLASS (class);
	// GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (class);
	// object_class->dispose = demo_application_window_dispose;
	// gtk_widget_class_set_template_from_resource (widget_class, "/application_demo/application.ui");
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, message);
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, infobar);
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, status);
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, buffer);
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, menubutton);
	// gtk_widget_class_bind_template_child (widget_class, DemoApplicationWindow, toolmenu);
	// gtk_widget_class_bind_template_callback (widget_class, clicked_cb);
	// gtk_widget_class_bind_template_callback (widget_class, update_statusbar);
	// gtk_widget_class_bind_template_callback (widget_class, mark_set_callback);
}

int RunGtkTests()
{
	GtkApplication* app = GTK_APPLICATION(g_object_new(
		demo_application_get_type(),
		"application-id", "org.gtk.Demo4.App",
		"flags", G_APPLICATION_HANDLES_OPEN,
		"register-session", TRUE,
		NULL
	));
	return g_application_run(G_APPLICATION(app), 0, NULL);
}

#endif //(TARGET_IS_LINUX && BUILD_WITH_GTK)