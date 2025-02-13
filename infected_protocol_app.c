#include <furi.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <input/input.h>
#include <stdio.h>
#include <string.h>

// Color definitions (adjust hex values as needed)
#define COLOR_BLACK         0x000000
#define COLOR_NEON_GREEN    0x39FF14   // For progress and highlights
#define COLOR_NEON_PINK     0xFF6EC7   // For title text
#define COLOR_NEON_PURPLE   0x800080   // For non-selected menu items
#define COLOR_ELECTRIC_BLUE 0x7DF9FF   // (Reserved for future use)

#define FONT_PRIMARY        FontPrimary  // Use the appropriate font from your SDK

// App modes for managing UI state
typedef enum {
    MENU_MAIN,
    PROCESS_UNLOCKING,
    SHOW_MESSAGE
} AppMode;

// Application state structure
typedef struct {
    AppMode mode;
    uint8_t progress;       // Unlocking progress (0-100)
    uint8_t selected_index; // Selected index for the drop-down menu
    char message[32];       // Message for SHOW_MESSAGE mode
} InfectedProtocolApp;

// Menu items for the drop-down list
static const char* menu_items[] = {
    "Unlock Phone",
    "Diagnostic Check",
    "System Reset"
};
static const uint8_t menu_items_count = sizeof(menu_items) / sizeof(menu_items[0]);

// Draw callback: renders UI elements on the screen
static void infected_protocol_draw_callback(Canvas* canvas, void* ctx) {
    InfectedProtocolApp* app = ctx;
    // Clear the screen with a dark base
    canvas_clear(canvas, COLOR_BLACK);
    
    // Draw the full app name at the top
    canvas_draw_str(canvas, 10, 20, "Infected Protocol", FONT_PRIMARY, COLOR_NEON_PINK);
    
    // Render based on current mode
    if(app->mode == MENU_MAIN) {
        // Draw the drop-down menu below the title
        int start_y = 40;
        for(uint8_t i = 0; i < menu_items_count; i++) {
            // Highlight the selected item with neon green; others use neon purple
            uint32_t color = (i == app->selected_index) ? COLOR_NEON_GREEN : COLOR_NEON_PURPLE;
            canvas_draw_str(canvas, 10, start_y + (i * 20), menu_items[i], FONT_PRIMARY, color);
        }
    } 
    else if(app->mode == PROCESS_UNLOCKING) {
        // Draw a progress bar and percentage for the unlocking process
        int progress_bar_width = (canvas->width - 20) * app->progress / 100;
        canvas_draw_box(canvas, 10, canvas->height - 40, progress_bar_width, 10, COLOR_NEON_GREEN);
        
        char progress_text[32];
        snprintf(progress_text, sizeof(progress_text), "Unlocking: %d%%", app->progress);
        canvas_draw_str(canvas, 10, canvas->height - 60, progress_text, FONT_PRIMARY, COLOR_NEON_GREEN);
    } 
    else if(app->mode == SHOW_MESSAGE) {
        // Display a message (e.g., "Not implemented" or "Unlock Complete")
        canvas_draw_str(canvas, 10, canvas->height / 2, app->message, FONT_PRIMARY, COLOR_NEON_GREEN);
    }
}

// Input callback: handles button presses and other input events
static void infected_protocol_input_callback(InputEvent* event, void* ctx) {
    InfectedProtocolApp* app = ctx;
    
    if(event->type == InputEventTypeShort) {
        if(event->key == InputKeyOk) {
            if(app->mode == MENU_MAIN) {
                // If "Unlock Phone" is selected, start the unlocking process;
                // otherwise, show a "Not implemented" message
                if(app->selected_index == 0) {
                    app->mode = PROCESS_UNLOCKING;
                    app->progress = 0;
                } else {
                    app->mode = SHOW_MESSAGE;
                    strcpy(app->message, "Not implemented");
                }
            }
            else if(app->mode == SHOW_MESSAGE) {
                // Return to the main menu on OK press in message mode
                app->mode = MENU_MAIN;
            }
        }
        else if(event->key == InputKeyUp && app->mode == MENU_MAIN) {
            if(app->selected_index > 0) {
                app->selected_index--;
            }
        }
        else if(event->key == InputKeyDown && app->mode == MENU_MAIN) {
            if(app->selected_index < menu_items_count - 1) {
                app->selected_index++;
            }
        }
    }
}

// Main function: sets up the UI and runs the application loop
int infected_protocol_app(void* p) {
    InfectedProtocolApp app = { .mode = MENU_MAIN, .progress = 0, .selected_index = 0 };
    
    Gui* gui = furi_record_open(RECORD_GUI);
    ViewPort* view_port = view_port_alloc();
    view_port_set_draw_callback(view_port, infected_protocol_draw_callback, &app);
    view_port_set_input_callback(view_port, infected_protocol_input_callback, &app);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    
    gui_update(gui);
    
    while(1) {
        // Simulate the unlocking process when active
        if(app.mode == PROCESS_UNLOCKING) {
            if(app.progress < 100) {
                furi_delay_ms(100);  // Simulate processing delay
                app.progress++;
                gui_update(gui);
            } else {
                // Once complete, display a confirmation message briefly before returning to the menu
                app.mode = SHOW_MESSAGE;
                strcpy(app.message, "Unlock Complete");
                gui_update(gui);
                furi_delay_ms(2000);
                app.mode = MENU_MAIN;
                gui_update(gui);
            }
        }
        furi_yield();  // Allow other system tasks to run
    }
    
    // Cleanup (this part is normally not reached due to the infinite loop)
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    
    return 0;
}
