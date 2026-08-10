#include <gtkmm.h>
#include <thread>
#include <string>
#include "process.hpp"

class MainWindow : public Gtk::Window {
    public:
        Gtk::TextView console_view;
        Glib::RefPtr<Gtk::TextBuffer> console_buffer;

        Gtk::Button build_button{"Build"};

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

            Gtk::ScrolledWindow console_scroll;

            auto *editor_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            auto *console_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
            auto *browser_container = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

            // Set up console
            this->console_buffer = Gtk::TextBuffer::create();
            this->console_view.set_buffer(this->console_buffer);
            this->console_view.set_editable(false);
            this->console_view.set_cursor_visible(false);
            this->console_view.set_wrap_mode(Gtk::WrapMode::WORD_CHAR);
            this->console_view.set_vexpand(true);
            this->console_view.set_hexpand(true);
            
            // Set up Editor
            editor_container->append(build_button);
            this->build_button.signal_clicked().connect(
                sigc::bind(
                    sigc::mem_fun(*this, &MainWindow::run_console),
                    "./build/VivianiteRuntime"
                )
            );
            build_button.set_hexpand(false);
            build_button.set_halign(Gtk::Align::START);

            console_scroll.set_child(*console_container);
            console_scroll.set_vexpand(true);
            console_scroll.set_hexpand(true);

            editor_container->add_css_class("panel");
            console_container->add_css_class("panel");
            browser_container->add_css_class("panel");

            auto *browser_label = Gtk::make_managed<Gtk::Label>("Asset Manager");
            auto *console_label = Gtk::make_managed<Gtk::Label>("Developer Console");
            auto *editor_label = Gtk::make_managed<Gtk::Label>("Editor");

            editor_container->append(*editor_label);
            console_container->append(*console_label);
            console_container->append(this->console_view);
            browser_container->append(*browser_label);

            main_paned->set_start_child(*editor_container);
            main_paned->set_end_child(console_scroll);

            main_paned->set_position(620);

            set_child(*main_paned);
        }

        void append_console_text(const std::string& text) {
            if (text.empty())
                return;

            console_buffer->insert(console_buffer->end(), text);

            auto end = console_buffer->end();
            console_view.scroll_to(end);
        }

        void run_console(const std::string command) {
            Process process = process_create(command);

            std::thread([this, process = std::move(process)]() mutable {
                std::string chunk;

                while (process_read_chunk(process, chunk)) {
                    if (!chunk.empty()) {
                        Glib::MainContext::get_default()->invoke(
                            [this, text = std::move(chunk)]() mutable {
                                this->append_console_text(text);
                                return false;
                            }
                        );
                    }
                }

                Glib::MainContext::get_default()->invoke(
                    [this]() {
                        this->append_console_text("\n[process exited]\n");
                        return false;
                    }
                );
            }).detach();
        }
};

int main(int argc, char **argv) {
    auto app = Gtk::Application::create("org.vivianite.editor");
    return app->make_window_and_run<MainWindow>(argc, argv);
}
