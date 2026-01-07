/*
File:   tests_gtk.c
Author: Taylor Robbins
Date:   01\07\2026
Description: 
	** None
*/

#if (TARGET_IS_LINUX && BUILD_WITH_GTK)

static void OpenFileDialogCallback(GObject* source, GAsyncResult* result, gpointer user_data)
{
	WriteLine_I("Got OpenFileDialog callback!");
	GError* error = nullptr;
	GFile* file = gtk_file_dialog_open_finish((GtkFileDialog*)source, result, &error);
	UNUSED(file);
	UNUSED(error);
}

static void GtkApp_PrintHello(GtkWidget* widget, gpointer userData)
{
	ScratchBegin(scratch);
	PrintLine_I("Button pressed!");
	// GtkFileDialog* dialog = gtk_file_dialog_new();
	// gtk_file_dialog_open(dialog, NULL, NULL, OpenFileDialogCallback, nullptr);
	FilePath chosenPath = FilePath_Empty;
	Result dialogResult = OsDoOpenFileDialog(scratch, &chosenPath);
	if (dialogResult == Result_Success)
	{
		PrintLine_I("Chose file: \"%.*s\"", StrPrint(chosenPath));
	}
	else
	{
		PrintLine_E("Dialog error: %s", GetResultStr(dialogResult));
	}
	ScratchEnd(scratch);
}

static void GtkApp_Activate(GtkApplication* app, gpointer userData)
{
	GtkWidget* window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(window), "GTK Window");
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	
	GtkWidget* button = gtk_button_new_with_label("Hello World");
	gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(button, GTK_ALIGN_CENTER);
	g_signal_connect(button, "clicked", G_CALLBACK(GtkApp_PrintHello), nullptr);
	gtk_window_set_child(GTK_WINDOW(window), button);
	
	gtk_window_present(GTK_WINDOW(window));
}

int RunGtkTests(int argc, char** argv)
{
	GtkApplication* app = gtk_application_new("org.gtk.example", G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app, "activate", G_CALLBACK(GtkApp_Activate), nullptr);
	
	int exitCode = g_application_run(G_APPLICATION(app), argc, argv);
	
	g_object_unref(app);
	return exitCode;
}

#endif //(TARGET_IS_LINUX && BUILD_WITH_GTK)