#include <gtkmm.h>

class MainWindow : public Gtk::Window {
    public:
        MainWindow() {
            set_title("Vivianite Editor");
            set_default_size(1280, 720);

            // Load custom CSS
            auto css = Gtk::CssProvider::create();
            const std::string css_data = R"(
                .panel {
                    background-color: #1e1e2e;
                    border: 1px solid #cba6f7;
                    border-radius: 4px;
                    padding: 8px;
                }
            )";
            css->load_from_data(css_data);

            // Set display
            auto display = Gdk::Display::get_default();
            if (display) {
                Gtk::StyleContext::add_provider_for_display(
                    display,
                    css,
                    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
                );
            }

            auto *main_paned = Gtk::make_managed<Gtk::Paned>(Gtk::Orientation::VERTICAL);

            auto *editor_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            auto *console_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            auto *browser_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

            editor_container->add_css_class("panel");
            console_container->add_css_class("panel");
            browser_container->add_css_class("panel");

            auto *browser_label = Gtk::make_managed<Gtk::Label>("Asset Manager");
            auto *console_label = Gtk::make_managed<Gtk::Label>("Developer Console");
            auto *editor_label = Gtk::make_managed<Gtk::Label>("Editor");

            editor_container->append(*editor_label);
            console_container->append(*console_label);
            browser_container->append(*browser_label);

            main_paned->set_start_child(*editor_container);
            main_paned->set_end_child(*console_container);

            main_paned->set_position(620);

            set_child(*main_paned);
        }
};

int main(int argc, char **argv) {
    auto app = Gtk::Application::create("org.vivianite.editor");
    return app->make_window_and_run<MainWindow>(argc, argv);
}
