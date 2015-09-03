#include <pebble.h>

#include <inttypes.h>

Window *window;
TextLayer *text_layer, *paddle_layer, *debug_text_layer;
Layer *ball_layer;

AppTimer *timer;

const int delta = 40;
int dx = 1;

int timercallback_max_duration = 0;

const uint16_t ball_radius = 10;
const int screen_width = 144;
const int screen_height = 168;
const int paddle_width = 5;
const int paddle_height = 30;

const int ball_initial_x = 10;
const int ball_initial_y = 84;

const int paddle_initial_y =84;
const int paddle_step = 15;

//game state
int millis_current = 0;
float speed;
float ball_x;
float ball_y;
float ball_velocity_x;
float ball_velocity_y;
float paddle_y;


bool game_has_started = false;


void init_game_state() {
  ball_x = ball_initial_x;
  ball_y = ball_initial_y;
  ball_velocity_x = 1.0/40;
  ball_velocity_y = 0.3/40;
  speed = 1.0;
  
  paddle_y = paddle_initial_y;
}

void ball_update_proc (Layer *layer, GContext *ctx) {
  GPoint p0 = GPoint(ball_radius, ball_radius);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_draw_circle(ctx, p0, ball_radius-1);
}

 
void timer_callback(void *data) {
  
  //Get time at start of app loop callback
  time_t time1_seconds;
  time1_seconds = time(NULL);
  uint16_t time1_millis;
  time_ms(&time1_seconds, &time1_millis);
  
  int elapsed = 0;
  if (!game_has_started) {
    init_game_state();
    game_has_started = true;
  } else {
    
    elapsed = time1_millis - millis_current;
    if (elapsed < 0) {
      elapsed = 1000 + elapsed;
    }
    static char timestamp_buffer[64];
    snprintf(timestamp_buffer, 64, "elapsed: %d\n", elapsed);
    text_layer_set_text(text_layer, timestamp_buffer);
  
  
    //move ball
    
    //Check to see if we have hit the left/right edge
    if(ball_x > screen_width - 2*ball_radius)
    {
      ball_x = screen_width - 2*ball_radius;
      if (ball_velocity_x > 0) {
        ball_velocity_x = -ball_velocity_x;    //Reverse
      }
      speed = speed * 1.05;
    }
    else if(ball_x < 0)
    {
      ball_x = 0;
      if (ball_velocity_x<0) {
        ball_velocity_x = -ball_velocity_x ; //Forwards
      }
      speed = speed * 1.05;
    }
    
    //Check to see if we have hit the bottom/top edge
    if(ball_y > screen_height - 2*ball_radius)
    {
      ball_y = screen_height - 2*ball_radius;
      if (ball_velocity_y > 0) {
        ball_velocity_y = -ball_velocity_y;    //Reverse
      }
      speed = speed * 1.05;
    }
    else if(ball_y < 0)
    {
      ball_y = 0;
      if (ball_velocity_y < 0) {
        ball_velocity_y = -ball_velocity_y; //Forwards
      }
      speed = speed * 1.05;
    }
    
    //TODO check if we hit paddle
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "new velocity: %d ",(int)ball_velocity_x*1000);
    
    ball_x = ball_x + ball_velocity_x * elapsed * speed;
    ball_y = ball_y + ball_velocity_y * elapsed * speed;
    
    //update ball position
    GRect ball_current_frame = layer_get_frame(ball_layer);
    GRect ball_next_frame = GRect(ball_x, ball_y, ball_current_frame.size.w, ball_current_frame.size.w);
    layer_set_frame(ball_layer, ball_next_frame);
  }
  
  //Get time at end of app loop callback
  time_t time_end_seconds;
  time_end_seconds = time(NULL);
  uint16_t time_end_millis;
  time_ms(&time_end_seconds, &time_end_millis);
  
  millis_current = time_end_millis;
   
  long diff = (long) (time_end_millis - time1_millis);
  if (diff < 0) {
    diff = 1000 + diff;
  }
    
  APP_LOG(APP_LOG_LEVEL_DEBUG, "game loop took %ld", diff); 
  int time_until_next_frame = 0;
  if (delta-diff > 0) {
    time_until_next_frame = delta - diff;
  }
  
  //Register next execution
  timer = app_timer_register(time_until_next_frame, (AppTimerCallback) timer_callback, NULL);
}



// Buttons
static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(debug_text_layer, "Up pressed!");
  paddle_y = paddle_y - paddle_step;
  layer_set_frame(text_layer_get_layer(paddle_layer), GRect(0,paddle_y,paddle_width,paddle_height));
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(debug_text_layer, "Select pressed!");
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(debug_text_layer, "Down pressed!");
  paddle_y = paddle_y + paddle_step;
  layer_set_frame(text_layer_get_layer(paddle_layer), GRect(0,paddle_y,paddle_width,paddle_height));
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


void window_load(Window *window)
{
  //We will add the creation of the Window's elements here soon!
   
  window_set_background_color(window, GColorWhite);
  window_set_click_config_provider(window, click_config_provider);

  text_layer = text_layer_create(GRect(0, 0, screen_width, 20));
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_color(text_layer, GColorBlack);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layer));
  debug_text_layer = text_layer_create(GRect(0, 20, screen_width, 50));
  text_layer_set_background_color(debug_text_layer, GColorClear);
  text_layer_set_text_color(debug_text_layer, GColorBlack);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(debug_text_layer));

    
  //Create layer to draw pixels
  ball_layer = layer_create(GRect(ball_initial_x, ball_initial_y, ball_radius*2, ball_radius*2));
  layer_set_update_proc(ball_layer, ball_update_proc);
  layer_add_child(window_get_root_layer(window), ball_layer);
  
  paddle_layer = text_layer_create(GRect(0, 50, paddle_width, paddle_height));
  text_layer_set_background_color(paddle_layer, GColorBlack);
  text_layer_set_text_color(paddle_layer, GColorBlack);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(paddle_layer));
  
 
  //Start app loop
  timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
}
 
void window_unload(Window *window)
{
  //We will safely destroy the Window's elements here!
  text_layer_destroy(text_layer);

  //Cancel timer
  app_timer_cancel(timer);
  
  //Destroy layers
  layer_destroy(ball_layer);
  text_layer_destroy(debug_text_layer);
  text_layer_destroy(paddle_layer);
}


void init()
{
//Initialize the app elements here!
  window = window_create();
  window_set_fullscreen(window, true);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  window_stack_push(window, true);
  
  
  
}
 
void deinit()
{
  //De-initialize elements here to save memory!
  
  //We will safely destroy the Window's elements here!
  window_destroy(window);
}  
  
  
int main(void)
{
  init();
  app_event_loop();
  deinit();
}

