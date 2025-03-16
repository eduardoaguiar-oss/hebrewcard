#include <allegropp/bitmap.hpp>
#include <allegropp/color.hpp>
#include <allegropp/display.hpp>
#include <allegropp/event_queue.hpp>
#include <allegropp/font.hpp>
#include <allegropp/sample.hpp>
#include <allegropp/timer.hpp>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <iomanip> // For std::setw and std::setfill
#include <sstream> // For std::stringstream
#include <fstream>

namespace
{
  constexpr int SCREEN_WIDTH = 1280;
  constexpr int SCREEN_HEIGHT = 1024;
  constexpr int CARD_WIDTH = 600;
  constexpr int CARD_HEIGHT = 300;
  constexpr int CARD_FONT_SIZE = 120;
  constexpr int BUTTONS = 8;
  constexpr int BUTTON_WIDTH = 200;
  constexpr int BUTTON_HEIGHT = 50;
  constexpr int BUTTON_MARGIN = (SCREEN_HEIGHT - BUTTON_HEIGHT * BUTTONS) / (BUTTONS + 1);
  constexpr int TIMER_START = 120; // 1 minute in seconds
} // namespace

class Button
{
public:
  Button () = default;
  Button (int, int);
  void set_text (const std::string& text){ text_ = text; }
  std::string get_text () const { return text_; }
  bool is_clicked (int, int) const;
  
  int get_ix () const { return ix_; }
  int get_iy () const { return iy_; }
  int get_fx () const { return fx_; }
  int get_fy () const { return fy_; }
  
private:
  int ix_ = 0;
  int iy_ = 0;
  int fx_ = 0;
  int fy_ = 0;
  std::string text_;
};

Button::Button (int x, int y)
 : ix_ (x),
   iy_ (y),
   fx_ (x + BUTTON_WIDTH),
   fy_ (y + BUTTON_HEIGHT)
{
}

bool
Button::is_clicked (int x, int y) const
{
  return (x >= ix_ && x <= fx_ && y >= iy_ && y <= fy_);
}
/*
class event_dispatcher
{
public:
  // GLOBAL keyboard: key.up, down, char
  // display: close, expose, resize, lost, found, switch_out, switch_in, orientation
  // timer: timer
  // GLOBAL joystick: axis, button_up, button_down, configuration
  // GLOBAL mouse: axes, button_up, button_down, warped, enter_display, leave_display
  void add_timer (timer, name="timer1");
  void add_display (display, name="display0");
  void set_callback ("keyboard.key_up", functor);
  void reset_callback (""keyboard.key_up");
  set_callback ("display0.close", f);
  set_callback ("timer1.tick", f);

  template <typename T, typename ...Args> void
  set_callback (const std::string& event_id, T& obj, bool (T::*f)(Args...))
  {
    impl_->set_callback (event_id, mobius::core::functor (obj, f));
  }


  void reset_keyup_callback ();

  void set_keydown_callback (f);
  void set_keychar_callback (f);
  void add_keyup_callback (o, t);
};*/

class App
{
public:
  App ()
    : buttons_ (BUTTONS)
  {
  }

  void load_options (const std::string&);
  void run ();

private:
   std::vector <std::pair <std::string, std::string>> options_;

   allegropp::display display_;
   allegropp::font card_font_;
   allegropp::font font_;
   allegropp::sample background_music_;
   allegropp::sample success_audio_;
   allegropp::sample fail_audio_;
   allegropp::bitmap background_;
   allegropp::bitmap speaker_bitmap_;
   allegropp::bitmap reset_bitmap_;
   allegropp::timer timer_;

   int score = 0;
   std::string currentLetter;
   std::string currentName;
   int timeRemaining = TIMER_START; // Countdown timer
   bool gameOver = false;
   
   std::vector <Button> buttons_;

   void drawGame ();
   void generateQuestion ();
};

// Function to format the timer as "MM:SS"
std::string formatTime(int seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << minutes << ":"
       << std::setw(2) << std::setfill('0') << secs;
    return ss.str();
}

void
App::load_options (const std::string& path)
{
    std::ifstream file (path);

    if (!file.is_open())
      throw std::system_error ();

    std::string line;
    while (std::getline (file, line))
      {
        std::size_t pos = line.find ('\t');

        if (pos != std::string::npos)
          {
            std::string question = line.substr (0, pos);
            std::string answer = line.substr (pos + 1);
            options_.emplace_back (question, answer);
          }
      }

    file.close();
}

// Function to generate a random Hebrew letter and options
void App::generateQuestion() {
    std::random_device rd;
    std::mt19937 g(rd());
    
    auto options = options_;

    do
      {    
        std::shuffle (options.begin(), options.end(), g);
      }
    while (options[0].first == currentLetter);

    currentLetter = options[0].first;
    currentName = options[0].second;

    // Add the correct answer
    std::vector<std::string> currentOptions;
    currentOptions.push_back(currentName);

    // Add 7 random incorrect answers
    for (int i = 1; i <= 7; ++i) {
        currentOptions.push_back(options[i].second);
    }

    // Shuffle the options
    std::shuffle(currentOptions.begin(), currentOptions.end(), g);

    for (int i = 0; i < buttons_.size (); ++i) {
        buttons_[i].set_text (currentOptions[i]);
    }
}

// Function to draw the game screen
void App::drawGame() {
    allegropp::color score_color (0, 0, 64);
    allegropp::color card_text_color (0, 0, 64);
    allegropp::color card_color (100, 100, 100, 150);
    allegropp::color button_text_color (0, 0, 0);
    allegropp::color button_color (255, 255, 100, 100);

    // Draw the background image
    background_.draw_scaled (
      0, 0, background_.get_width (), background_.get_height (),
      0, 0, SCREEN_WIDTH, SCREEN_HEIGHT
    );

    // Draw the timer
    std::string timerText = "Time: " + formatTime(timeRemaining);
    font_.draw_text_left (10, 10, timerText, score_color);

    // Draw the score
    font_.draw_text_left (10, 50, "Score: " + std::to_string(score), score_color);

    // Draw the option letter card
    al_draw_filled_rectangle(SCREEN_WIDTH / 2 - CARD_WIDTH / 2, SCREEN_HEIGHT / 2 - CARD_HEIGHT / 2,
                             SCREEN_WIDTH / 2 + CARD_WIDTH / 2, SCREEN_HEIGHT / 2 + CARD_HEIGHT / 2, card_color.get_implementation ());
    card_font_.draw_text_center (SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - CARD_FONT_SIZE / 2 - 10, currentLetter, card_text_color);

    // Draw the letter name buttons
    int buttonX = SCREEN_WIDTH - BUTTON_WIDTH - BUTTON_MARGIN;
    int buttonY = BUTTON_MARGIN;

    for (int i = 0;i < buttons_.size ();i++)
      {
        const Button& b = buttons_[i];
        const std::string text = std::to_string (i + 1) + ". " + b.get_text ();
        al_draw_filled_rectangle(b.get_ix (), b.get_iy (), b.get_fx (), b.get_fy (), button_color.get_implementation ());
        font_.draw_text_center (b.get_ix () + BUTTON_WIDTH / 2, b.get_iy () + BUTTON_HEIGHT / 2 - 10, text, button_text_color);
      }

    // Draw the reset icon
    reset_bitmap_.draw (10, SCREEN_HEIGHT - 200);

    display_.flip ();
}

void App::run ()
{
    // Initialize Allegro
    if (!al_init()) {
        std::cerr << "Failed to initialize Allegro!" << std::endl;
        return;
    }

    // Initialize display
    display_ = allegropp::display (SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!display_) {
        std::cerr << "Failed to create display!" << std::endl;
        return;
    }
    display_.set_window_title ("Home School");

    // Initialize font and addons
    al_init_primitives_addon();

    // Load font (try multiple paths)
    const char* fontPaths[] = {
        //"FreeSerif.ttf", // Common Linux path
        "/usr/share/fonts/truetype/FreeSerif.ttf", // Common Linux path
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf" // Fallback font
    };

    for (const char* path : fontPaths) {
        card_font_ = allegropp::font (path, CARD_FONT_SIZE);
        font_ = allegropp::font (path, 36);
        if (card_font_) break;
    }

    if (!card_font_) {
        std::cerr << "Failed to load font!" << std::endl;
        return;
    }

    // Load the background image
    background_ = allegropp::bitmap ("background.png");
    if (!background_) {
        std::cerr << "Failed to load background image!" << std::endl;
        return;
    }

    // Load the speaker bitmap
    speaker_bitmap_ = allegropp::bitmap ("speaker.png");
    if (!speaker_bitmap_) {
        std::cerr << "Failed to load speaker image!" << std::endl;
        return;
    }

    // Load the reset bitmap
    reset_bitmap_ = allegropp::bitmap ("reset.png");
    if (!reset_bitmap_) {
        std::cerr << "Failed to load reset image!" << std::endl;
        return;
    }

    // Load audio samples music
    background_music_ = allegropp::sample ("hebrewcard.ogg");
    if (!background_music_) {
        std::cerr << "Failed to load background music!" << std::endl;
        return;
    }

    success_audio_ = allegropp::sample ("success.wav");
    if (!success_audio_) {
        std::cerr << "Failed to load success.wav!" << std::endl;
        return;
    }

    fail_audio_ = allegropp::sample ("fail.wav");
    if (!fail_audio_) {
        std::cerr << "Failed to load fail.wav!" << std::endl;
        return;
    }

    // Play the background music in a loop
    background_music_.play_loop ();

    // Initialize timer
    timer_ = allegropp::timer (1.0);
    if (!timer_) {
        std::cerr << "Failed to create timer!" << std::endl;
        return;
    }

    // Create buttons
    int buttonX = SCREEN_WIDTH - BUTTON_WIDTH - BUTTON_MARGIN;
    int buttonY = BUTTON_MARGIN;

    for (int i = 0;i < buttons_.size ();i++)
      {
        buttons_[i] = Button (buttonX, buttonY);
        buttonY += BUTTON_HEIGHT + BUTTON_MARGIN;
      }

    // Generate the first question
    generateQuestion();

    // Main game loop
    allegropp::event_queue event_queue;

    event_queue.add_display_events (display_);
    event_queue.add_keyboard_events ();
    event_queue.add_mouse_events ();
    event_queue.add_timer_events (timer_);

    timer_.start (); // Start the countdown timer

    bool running = true;
    while (running) {
        ALLEGRO_EVENT event;
        event_queue.get_event (event);

        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
          {
            running = false;
          }

        else if (event.type == ALLEGRO_EVENT_TIMER)
          {
            // Update the countdown timer
            if (timeRemaining > 0) {
                timeRemaining--;
            } else {
                gameOver = true; // End the game when the timer reaches zero
            }
          }

        else if (!gameOver && event.type == ALLEGRO_EVENT_KEY_DOWN)
          {
            int i = -1;

            if (event.keyboard.keycode >= ALLEGRO_KEY_1 && event.keyboard.keycode <= ALLEGRO_KEY_8)            
                i = event.keyboard.keycode - ALLEGRO_KEY_1;

            else if (event.keyboard.keycode >= ALLEGRO_KEY_PAD_1 && event.keyboard.keycode <= ALLEGRO_KEY_PAD_8)
                i = event.keyboard.keycode - ALLEGRO_KEY_PAD_1;
           
            if (i == -1)
              continue;

            if (buttons_[i].get_text () == currentName) {
                        score++;
                        success_audio_.play_once ();
                    } else {
                        score = 0; // Reset score on wrong answer
                        fail_audio_.play_once ();
                    }

                    // Generate a new question
                    generateQuestion();
          }
        
        else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN)
          {
            int mouseX = event.mouse.x;
            int mouseY = event.mouse.y;
            int dx = mouseX -  74;
            int dy = mouseY - 898;

            if (dx*dx + dy*dy < 64*64)
              {
                score = 0;
                timeRemaining = TIMER_START;
                gameOver = false;
                generateQuestion();
              }
                
            else if (!gameOver)
              {
                // Check if a button was clicked
                for (const auto& button : buttons_)
                  {
                    if (button.is_clicked (mouseX, mouseY))
                      {
                        if (button.get_text () == currentName) {
                            success_audio_.play_once ();
                            score++;
                        } else {
                            score = 0; // Reset score on wrong answer
                            fail_audio_.play_once ();
                        }

                        // Generate a new question
                        generateQuestion();
                        break;
                      }
                  }
              }
        }

        drawGame();
    }
}

int main() {
    App app;
    app.load_options ("hebrew_letters.txt");
    app.run ();
    return 0;
}
